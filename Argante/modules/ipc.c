/*

   Argante virtual OS
   ------------------

   Remote IPC connectivity

   Status: done, needs testing

   Author:     Bulba <bulba@intelcom.pl>
   Maintainer: Bulba <bulba@intelcom.pl>
   Patched:    Michal Zalewski <lcamtuf@ids.pl> - fioasync

*/


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/un.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#ifndef O_ASYNC
// Solaris?!
#include <sys/file.h>
#define O_ASYNC FASYNC
#endif

#include "config.h"
#include "acman.h"
#include "console.h"
#include "syscall.h"
#include "task.h"
#include "bcode.h"
#include "memory.h"
#include "module.h"

extern void 	(*task_ripc_handler)(void);
extern void	(*task_ipc_timeouter)(void);
static int network_handler_semafor = 0;


// wanna see timestamped debug messages?
#if 0
# define RIPC_LOG(x...) { \
        struct timeval tv; \
	gettimeofday(&tv,NULL); \
        printk("LOG %d (%d) ",(int)tv.tv_sec,(int)tv.tv_usec); \
	printk(x); \
    }
#else
# define RIPC_LOG(x...)
#endif

#if 0
# define RIPC_DEBUG(x...) printk(x)
#else 
# define RIPC_DEBUG(x...)
#endif


#ifndef AF_LOCAL
// Solaris
#define AF_LOCAL AF_UNIX
#endif

#ifndef PF_LOCAL
// Solaris
#define PF_LOCAL AF_UNIX
#endif



#define MEM_FLAG_IPC			4
#define MEM_FLAG_LOCKED			8	// not used yet...

// configurable variables
static unsigned int	system_max_streams = 1024;
#define GLOBAL_MAX_STREAMS	system_max_streams
static unsigned int	max_interfaces = 16;
#define MAX_INTERFACES max_interfaces
static unsigned int	max_streams = 16;
#define MAX_STREAMS	max_streams
static unsigned int	max_blocks = 16;
#define MAX_BLOCKS	max_blocks
static unsigned int	max_stream_buffers = 512;
#define MAX_STREAM_BUFFERS max_stream_buffers
static unsigned int	max_buckets = 512;
#define BUCKET_MAX	max_buckets
static unsigned int	default_ttl = 32;
#define DEFAULT_TTL	default_ttl

static int	listen_counter = 0;
#define NETWORK_INTERVAL	5000
static int	network_counter = 0;
static int	network_force = 0;

#define MAX_CONFIG_CONNECTS	100
#define ACCEPT_INTERVAL		10000
#define RECONNECT_INTERVAL	100000

struct config_connect {
    char 	* to;
    int		interface;
    int		status;
};
struct config_connect	config_connects[MAX_CONFIG_CONNECTS];
static int		reconnect_counter = 0;


#define IOWAIT_ID_MSG_RECV		1
#define IOWAIT_ID_MSG_SEND		2
#define IOWAIT_ID_STREAM_SENT_REQ	3
#define IOWAIT_ID_STREAM_REQ		4
#define IOWAIT_ID_STREAM_ACK		5
#define IOWAIT_ID_STREAM_READ		6
#define IOWAIT_ID_STREAM_WRITE		7
#define IOWAIT_ID_BLOCK_READ		8
#define IOWAIT_ID_BLOCK_WRITE		9
#define IOWAIT_ID_BLOCK_REQ		10


// s0 return values:
#define IPC_EOK				1
#define IPC_ETRYAGAIN			0
#define IPC_ERROR			-1

// request status
#define IPC_RSTATUS_ERROR		-1
#define IPC_RSTATUS_WAITING		0
#define IPC_RSTATUS_ACCEPTED		1
#define IPC_RSTATUS_COMPLETED		2

#define HEAD_SIZE	sizeof(struct pckt_header)
#define LEN_MAX		sizeof(struct pckt_resp)+PAYLOAD

// najwiekszy pckt_ (pckt_resp 14*4b) plus 4096 na ladunek
#define BUCKET_SIZE	PAYLOAD+sizeof(struct pckt_bucket)-sizeof(struct pckt_header)\
			+sizeof(struct pckt_resp)


struct pckt_header {
    unsigned char	dst;
    unsigned char	src;
    unsigned char	type;
    unsigned char	ttl;
    unsigned int	id;
};

struct pckt_bucket {
    struct pckt_bucket	*next;
    unsigned char	from_interface;
    unsigned int	len;
    unsigned char	data[sizeof(struct pckt_header)];
#define pckt_dst	data[0]
#define pckt_src	data[1]
#define pckt_type	data[2]
#define pckt_ttl	data[3]
#define pckt_id(x)	((int *)x->data)[1]
#define pckt_load	data[sizeof(struct pckt_header)]
};

struct ipc_address {
    signed int		vcpu;
    signed int		vs;
    unsigned int	ipc_reg;
};

struct ipc_request {
    struct ipc_address	s;
    struct ipc_address	d;
    struct ipc_address	f;	// first one who accepted request
    unsigned int	type;
#define IPC_MSG			1
#define IPC_STREAM		2
#define IPC_BLOCK_READ		3
#define IPC_BLOCK_WRITE		4
    unsigned int	id;
    unsigned int	data[4];
#define msg_data1	data[0]
#define msg_data2	data[1]
#define str_req_str	data[0]
#define str_dst_str	data[1]
#define blk_block	data[0]
#define blk_offset	data[1]
#define blk_count	data[2]
    unsigned int	extra[4];
#define blk_current	extra[0]
#define blk_next	extra[1]
#define blk_adres	extra[2]
#define str_local_src	extra[0]
#define str_local_dst	extra[1]
    int			errcode;
#define IPC_ERR_OK		0		// order matters...
#define IPC_ERR_NORESOURCES	1
#define IPC_ERR_NOTARGET	2
#define IPC_ERR_TIMEOUT		3
#define IPC_ERR_NACK		4
#define IPC_ERR_BADMEM		5
#define IPC_ERR_DEAD		6		// other side is dead...
    unsigned int	flags;
#define IPC_FLAG_NONBLOCK	1		// if call is nonblocking (we keep report)
#define IPC_FLAG_MULTICAST	2		// completed only if each target gets this
#define IPC_FLAG_SENDERSLEEP	4		// need to wake up sender
#define IPC_FLAG_ACK_SENT	8		// when send net ack
#define IPC_FLAG_ACCEPTED	16		// someone has accepted alredy
#define IPC_FLAG_CLOSING	32		// when finishing net requset
#define IPC_FLAG_COMPLETED	64		// request is done...
#define IPC_FLAG_ERROR		128
#define IPC_FLAG_HAD_TARGETS	512		// when request had local targets

#define IPC_FLAG_NETMASK	(IPC_FLAG_MULTICAST|IPC_FLAG_ACCEPTED|\
				 IPC_FLAG_COMPLETED|IPC_FLAG_ERROR)
				
    struct timeval		timestamp;		
    int				ripc_host_count;
    int				retries;	// retransmision count...
    struct ipc_queue		*targets;
    struct ipc_request 		*next;
    struct ipc_request		*prev; // linking...
};

struct ipc_queue {
    unsigned int	vcpu;
    struct ipc_request	* req;
    struct ipc_queue	* next;		// link in this process queue
    struct ipc_queue	* prev;
    struct ipc_queue	* next_r;	// link in request targets
    struct ipc_queue	* prev_r;
};

struct buffer {
    unsigned int 	len;
    unsigned int	head; 		// next written byte place
    unsigned int	tail; 		// first byte to read
    unsigned int	net_tail;	// amount of data sent but not acked.
    char		buf[STREAM_BUFFER_LEN];
};

struct stream {
    struct buffer	* in;
    struct buffer	* out;
    struct ipc_request	*req;
    unsigned int	peer_vcpu;
    unsigned int	peer_vs;
    unsigned int	peer_stream;
    unsigned int	my_vcpu;
    unsigned int	my_stream;
    unsigned int	my_global_nr;
    unsigned int	id;
    unsigned int	flags;
#define STR_READSLEEP		1	// waiting for data
#define STR_WRITESLEEP		2	// sleeping for write
#define STR_INUSE		4	// stream in use
#define STR_PEERDEAD		8	// peer is dead, no writing, but may read
#define STR_PEERREADY		16	// peer is ready for data (NETWORKING)
#define STR_NOTREADY		32	// in handshake mode (NETWORKING)
#define STR_CLOSING		64	// cute way to end network stream..
#define STR_PEERCLOSING		128	// peer is closing, no writing, but may read
    // when sleeping for read/write
    unsigned char *	ptr;		// data ptr, updated every time
    unsigned int	count;		// how much we have to transfer
    unsigned int	transfered;	// how much we've alredy transfered
    // networking stuff..
    struct timeval	timestamp;	// last time we sent something
    unsigned int	retries;	// count of retransmits
    struct timeval	pong;		// last time we got anything from peer
    struct pckt_bucket	* data;		// data packet...
    unsigned int	peer_count;	// # of last data pckt acked by us...
    unsigned int	my_count;	// # of last data pckt we've sent...
};

#define PCKT_ROUTE		1
#define PCKT_REQUEST		2
#define PCKT_RESP		3
#define PCKT_HOST_ACK		4
#define PCKT_STREAM_DATA 	5
#define PCKT_STREAM_ACK_ACK	6
#define PCKT_STREAM_RESET	7
#define PCKT_BLOCK_DATA		8
#define PCKT_BLOCK_ACK_ACK	9
#define PCKT_BLOCK_RESET	10

struct pckt_stream_data {
    struct pckt_header	h;
    int		s_vcpu;
    int		s_vs;
    int		s_stream;
    int		d_vcpu;
    int		d_vs;
    int		d_stream;
    int		id;
    int		pckt_number;	// to be acked further..
    int		flags;
#define STR_DATA_DATA		1 // stream data in packet (load of bytes)
#define STR_DATA_READY		2 // ready for more data
#define STR_DATA_CLOSING	4 // announce stream close
#define STR_DATA_ACK		8 // ack==# of pckt we're acking
#define STR_DATA_DEAD		16 // when no such stream/peer is dead
    int		load;		// data bytes in this packet...    
    int		ack;
};

struct pckt_stream_ack_ack {
    struct pckt_header	h;
    int		s_vcpu;
    int		s_vs;
    int		s_stream;
    int		d_vcpu;
    int		d_vs;
    int		d_stream;
    int		id;
};

struct pckt_stream_reset {
    struct pckt_header	h;
    int		s_vcpu;
    int		s_vs;
    int		s_stream;
    int		d_vcpu;
    int		d_vs;
    int		d_stream;
    int		id;
};

struct pckt_block_reset {
    struct pckt_header	h;
    int		id;
    int		s_vcpu;
    int		s_vs;
    int		block;    
};


struct pckt_block_data {
    struct pckt_header	h;
    int		id;
    int		r_vcpu;
    int		r_vs;
    int		direction;	// to or from
    int		block;
    int		pckt_number;
    int		flags;
#define BLK_ACK		1
#define BLK_NACK	2
#define BLK_DONE	4
    int 	load;
};

struct pckt_block_ack_ack {
    struct pckt_header	h;
    int		s_vcpu;
    int		s_vs;
    int		s_ipc_reg;
    int		f_vcpu;
    int		f_vs;
    int		f_ipc_reg;
    int		id;
    int		block;
    int		len;
};

struct pckt_request {
    struct pckt_header	h;
    int		type;
    int		s_vcpu;
    int		s_vs;
    int		s_ipc_reg;
    int		d_vcpu;
    int		d_vs;
    int		d_ipc_reg;
    int		data[4];
    int		flags;
    int		id;
};

struct pckt_resp {
    struct pckt_header	h;
    int		type;
    int		s_vcpu;
    int		s_vs;
    int		s_ipc_reg;
    int		f_vcpu;
    int		f_vs;
    int		f_ipc_reg;
    int		id;
    int		data[4];
    int		flags;
    int		errcode;
};

struct pckt_host_ack {
    struct pckt_header	h;
    int		type;
    int		vcpu;
    int		vs;
    int		ipc_reg;
    int		id;
};

struct interface {
    int			fd;
    int			flags;
#define IF_FLAG_CONNECTED	1
#define IF_FLAG_IDENTIFIED	2
#define IF_FLAG_NEED_UPDATE	4
    struct pckt_bucket	*in_pckt;
    struct pckt_bucket	*out_pckt;
    struct pckt_bucket	*queue_pckt;
    struct pckt_bucket	*zapas;		// in case we need one route packet
    unsigned char 	*in_ptr;
    unsigned char	*out_ptr;
    int			in_cnt;
    int			out_cnt;
    char		route[MAX_HOSTS]; // hops, 0 = unreach
    char		send[MAX_HOSTS]; 
    char		neue[MAX_HOSTS];
};

struct ipc_block {
    unsigned int	flags;
#define BLOCK_INUSE	1
#define BLOCK_BUSY	2
    unsigned int	block;
    unsigned int	len;
    struct ipc_request	*req;	// current request..
};

struct msg_sleeper {
    unsigned int	vcpu;
    struct msg_sleeper	*next;
    struct msg_sleeper	*prev;
};

struct buffer_bucket {
    struct buffer_bucket *next;
};


// macrosy...
#define IPC_UNLINK_NET_REQUEST(x) if (x->prev) x->prev->next = x->next;\
				    else network_requests = x->next; \
				if (x->next) x->next->prev = x->prev;
#define IPC_UNLINK_REQUEST(x,c) if (x->prev) x->prev->next = x->next; \
				    else proces_requests[c] = x->next; \
				if (x->next) x->next->prev = x->prev; 
#define IPC_UNLINK_QUEUE(x,r)   if (x->prev) x->prev->next = x->next; \
    				    else proces_incoming[(x)->vcpu] = (x)->next; \
				if ((x)->next) (x)->next->prev = (x)->prev; \
				if ((x)->prev_r) (x)->prev_r->next_r = (x)->next_r; \
    				    else (r)->targets = (x)->next_r; \
				if ((x)->next_r) (x)->next_r->prev_r = (x)->prev_r; 

static struct ipc_request	*request_recycler = NULL;
static void			*request_block[REQUESTS_MAX];

static struct ipc_queue		*queue_recycler = NULL;
static void			*queue_block[QUEUES_MAX];

static struct ipc_request 	*proces_requests[MAX_VCPUS];
static struct ipc_queue		*proces_incoming[MAX_VCPUS];
static struct ipc_request	*network_requests = NULL;

static int			buffers_count = 0;
static struct buffer_bucket	*buffers = NULL;
static struct stream		**proces_streams[MAX_VCPUS];
static struct stream		*streams = NULL;
static int			streams_count = 0;

static struct ipc_block		*proces_blocks[MAX_VCPUS];

static struct msg_sleeper	*msg_sleepers = NULL;
static struct msg_sleeper	sleepers[MAX_VCPUS];

static unsigned int		ipc_vs_number = 1;
static unsigned int		ipc_id = 0;
static unsigned int		networking = 0;

static unsigned int
ipc_new_id () {
    return ipc_id++;
}

inline void
ipc_queue_init (void) {
    memset(queue_block, 0, sizeof(queue_block));
}

inline void
ipc_queue_destroy (void) {
    int i;
    for (i=0; i<QUEUES_MAX; i++)
	if (queue_block[i]) free(queue_block[i]);
}

static void
ipc_queue_malloc (void) {
    struct ipc_queue 	*p;
    int i;
    for (i=0; ((i<QUEUES_MAX) && queue_block[i]); i++) ;
    if (i>=QUEUES_MAX) return;
    queue_block[i] = (void *) malloc(sizeof (struct ipc_queue) * QUEUES_AT_ONCE);
    if (!queue_block[i]) return;
    p = queue_block[i];
    memset(p, 0, sizeof (struct ipc_queue) * QUEUES_AT_ONCE);
    RIPC_DEBUG("debug: new queue block\n");
    for (i=0;i<QUEUES_AT_ONCE;i++) {
	p[i].next = queue_recycler;
	queue_recycler = &p[i];
    }
}

static struct ipc_queue *
ipc_queue_new (void) {
    struct ipc_queue *p;
    if (!queue_recycler) ipc_queue_malloc();
    if (!queue_recycler) return NULL;
    p = queue_recycler;
    queue_recycler = p->next;
    p->next = NULL;
    RIPC_DEBUG("debug: new queue element\n");
    return p;
}

static void
ipc_queue_done (struct ipc_queue *p) {
    memset(p, 0, sizeof(struct ipc_queue));
    RIPC_DEBUG("debug: queue element done\n");
    p->next = queue_recycler;
    queue_recycler = p;
}

static void
ipc_queue_done_linked (struct ipc_queue *p) {
    struct ipc_queue *q;
    if (!p) return;
    q = p;
    RIPC_DEBUG("debug: queue list done\n");
    for (q=p;;q=q->next_r) {
	if (q->prev) q->prev->next = q->next;
	    else proces_incoming[q->vcpu] = q->next;
	if (q->next) q->next->prev = q->prev;
	q->prev = NULL;
	q->next = NULL;
	q->prev_r = NULL;
	q->req = NULL;
	q->vcpu = 0;
	if (!q->next_r) break;
    } 
    q->next_r = queue_recycler;
    queue_recycler = p;
}

inline void
ipc_request_init (void) {
    memset(request_block, 0, sizeof(request_block));
}

inline void
ipc_request_destroy (void) {
    int i;
    for (i=0;i<REQUESTS_MAX;i++) 
	if (request_block[i]) free(request_block[i]);
}

static void
ipc_request_malloc (void) {
    struct ipc_request *p;
    int i;
    for (i=0; ((i<REQUESTS_MAX) && (request_block[i])); i++);
    if (i>=REQUESTS_MAX) return;
    request_block[i] = (void *)malloc(sizeof(struct ipc_request) * REQUESTS_AT_ONCE);
    if (!request_block[i]) return;
    RIPC_DEBUG("debug: new block of requests\n");
    memset(request_block[i], 0, sizeof(struct ipc_request) * REQUESTS_AT_ONCE);
    p = (struct ipc_request*) request_block[i];
    for (i=0; i<REQUESTS_AT_ONCE; i++) {
	p[i].next = request_recycler;
	request_recycler = &p[i];
    }
}


static struct ipc_request *
ipc_request_new (void) {
    struct ipc_request *p;
    if (!request_recycler) ipc_request_malloc();
    if (!request_recycler) return NULL;
    RIPC_DEBUG("debug: new request\n");
    p = request_recycler;
    request_recycler = p->next;
    p->next = NULL;
    return p;    
}

static void
ipc_request_done (struct ipc_request *p) {
    RIPC_DEBUG("debug: request done\n");
    memset(p, 0, sizeof(struct ipc_request));
    p->next = request_recycler;
    request_recycler = p;
}

static int
ipc_make_targets (int c, struct ipc_request *p) {
    struct ipc_queue *q, *t;
    int i;
    p->targets = NULL;
    for (i=0;i<MAX_VCPUS;i++) {
	if (i==c) continue;		// disallow sending req to ourself
	if (!(cpu[i].flags & VCPU_FLAG_USED)) continue;
	if (!cpu[i].ipc_reg) continue;
	if ((p->d.vcpu != -1) && (p->d.vcpu != i)) continue;
	if ((p->d.ipc_reg != 0) && (cpu[i].ipc_reg != p->d.ipc_reg)) continue;
	// ok, so we've got potential target...
	q = ipc_queue_new();
	if (!q) {
	    if (p->targets) ipc_queue_done_linked (p->targets);
	    p->targets = NULL;
	    return 0;
	}
	q->next_r = p->targets;
	if (q->next_r) (q->next_r->prev_r = q);	
	q->prev_r = NULL;
	p->targets = q;
	q->req = p;
	q->vcpu = i;
	if (proces_incoming[i]) {
	    for (t=proces_incoming[i];t->next;t=t->next);
	    t->next = q;
	    q->prev = t;
	    q->next = NULL;
	} else {
	    proces_incoming[i] = q;
	    q->prev = NULL;
	    q->next = NULL;
	}
    }
    if (p->targets) p->flags |= IPC_FLAG_HAD_TARGETS;
    return 1;
}

static unsigned int		bucket_count = 0;
static struct pckt_bucket	*bucket_recycling = NULL;
static struct pckt_bucket	*postprocess_queue = NULL;
static struct pckt_bucket	*postprocess_last = NULL;
static struct pckt_bucket	*lost_queue = NULL;
static struct pckt_bucket	*lost_last = NULL;
static unsigned int		known_hosts = 0;
static unsigned int		broadcast_table[MAX_HOSTS];
static int			ripc_listen_socket = -1;
static struct interface		*interfaces;
static unsigned int		broadcast_id = 0;

static unsigned int
ripc_give_id () {
    return broadcast_id++;
}

static void *
ripc_bucket_new (void) {
    struct pckt_bucket *p = bucket_recycling;
    if (p) bucket_recycling = p->next;
    else if (bucket_count<BUCKET_MAX) {
	p = (struct pckt_bucket*) malloc (BUCKET_SIZE);
	if (p) bucket_count++;
    }
    return p;
}

static void
ripc_bucket_done (struct pckt_bucket * p) {
    p->next = bucket_recycling;
    bucket_recycling = p;
}

static void
ripc_route_drop_interface ( int i ) {
    int iff;
    int best;
    char best_h, second_h;
    int c;
    for (c=1;c<MAX_HOSTS;c++) {
	if (c==ipc_vs_number) continue;
	if (!interfaces[i].route[c]) continue;
	interfaces[i].route[c] = 0;
	second_h = 0;
	best = i; best_h = 0;
	for (iff=0;iff<MAX_INTERFACES;iff++) {
	    char r;
	    if ((iff==i) || (!interfaces[iff].flags)) continue;
	    if (!(r=interfaces[iff].route[c])) continue;
	    if (!best_h) { best_h = r; best = iff; }
	    else if (best_h>r) { second_h = best_h; best_h = r; best = iff; }
	    else if ((!second_h) || (second_h>r)) second_h = r;
	}
	if (!best_h) known_hosts--;
	// update neues
	for (iff=0;iff<MAX_INTERFACES;iff++) {
	    if ((iff==i) || (!interfaces[iff].flags)) continue;
	    if (iff==best) interfaces[iff].neue[c] = second_h;
	    else interfaces[iff].neue[c] = best_h;
	}    
    }
    for (iff=0;iff<MAX_INTERFACES;iff++) {
	if ((iff==i) || (!interfaces[iff].flags)) continue;
	interfaces[iff].flags |= IF_FLAG_NEED_UPDATE;
    }
    RIPC_DEBUG("debug: known_hosts %d\n", known_hosts);
}

static void
ripc_route_request (struct pckt_bucket *p) {
    int i;
    int iff;
    int best;
    char best_h, second_h;
    int c;
    int change = 0;
    unsigned char *d;
    int len;
    RIPC_DEBUG("debug: got route update packet.\n");
    i = p->from_interface;
    len = ntohl(p->len) - sizeof(struct pckt_header);
    d = p->data + sizeof(struct pckt_header);
    while (len>1) {
	len -= 2;
	c = *d++;
	if ((c==ipc_vs_number) && (*d == 1)) {
	    printk("Warning: Am I connected to myself???\n");
	}
	if (interfaces[i].route[c]) change=1;
	interfaces[i].route[c] = *d;
	second_h = 0;
	best = i; best_h = *d++;
	for (iff=0;iff<MAX_INTERFACES;iff++) {
	    char r;
	    if ((iff==i) || (!interfaces[iff].flags)) continue;
	    if (!(r=interfaces[iff].route[c])) continue;
	    if (!best_h) { best_h = r; best = iff; }
	    else if (best_h>r) { second_h = best_h; best_h = r; best = iff; }
	    else if ((!second_h) || (second_h>r)) second_h = r;
	}
	if (change && !best_h) known_hosts--;
	else if (!change && best == i && !second_h) known_hosts++;
	// update neues
	for (iff=0;iff<MAX_INTERFACES;iff++) {
	    if ((iff==i) || (!interfaces[iff].flags)) continue;
	    if (iff==best) interfaces[iff].neue[c] = second_h;
	    else interfaces[iff].neue[c] = best_h;
	}    
    }
    for (iff=0;iff<MAX_INTERFACES;iff++) {
	if ((iff==i) || (!interfaces[iff].flags)) continue;
	interfaces[iff].flags |= IF_FLAG_NEED_UPDATE;
    }
    printk("debug: known_hosts %d\n", known_hosts);
}

static void
ripc_route_new_interface (int i) {
    int iff;
    char best_h;
    int c;
    for (c=1;c<MAX_HOSTS;c++) {
	if (c==ipc_vs_number) continue;
	interfaces[i].route[c] = 0;
	interfaces[i].send[c] = 0;
	best_h = 0;
	for (iff=0;iff<MAX_INTERFACES;iff++) {
	    char r;
	    if ((iff==i) || (!interfaces[iff].flags)) continue;
	    if (!(r=interfaces[iff].route[c])) continue;
	    if ((!best_h) || (best_h>r)) best_h = r; 
	}
	interfaces[i].neue[c] = best_h;
    }
    interfaces[i].flags |= IF_FLAG_NEED_UPDATE;
    RIPC_DEBUG("debug: updated route to send\n");
}

static int
ripc_create_route_pckt (struct pckt_bucket *p, int iff) {
    struct pckt_header	*h;
    unsigned char 	*d;
    int			i;
    int			cos = 0;
    p->next = NULL;
    h = (struct pckt_header *)p->data;
    h->src = ipc_vs_number;
    h->dst = 0;
    h->type = PCKT_ROUTE;
    h->ttl = DEFAULT_TTL;    
    d = (unsigned char *)h + sizeof(struct pckt_header);
    if (!(interfaces[iff].flags & IF_FLAG_IDENTIFIED)) {
	*d++ = ipc_vs_number;
	*d++ = 1;
	interfaces[iff].flags |= IF_FLAG_IDENTIFIED;
	cos++;
    }
    for (i=1;i<MAX_HOSTS;i++) {
	if (i==ipc_vs_number) continue;
	if (interfaces[iff].send[i] != interfaces[iff].neue[i]) {
	    interfaces[iff].send[i] = interfaces[iff].neue[i];
	    *d++ = i;
	    if (interfaces[iff].neue[i]) *d++ = interfaces[iff].neue[i] + 1;
	    else *d++ = interfaces[iff].neue[i];
	    cos++;
	}
    }
    interfaces[iff].flags -= IF_FLAG_NEED_UPDATE;
    p->len = htonl(d - (unsigned char*)h);
    // max packet len is MAX_HOSTS * 2 .. about 512b + pckt_header + 4b
    if (cos) RIPC_DEBUG("debug: created route update packet.\n");
    return cos;
}

static int
ripc_interface_new (int fd) {
    struct interface *p;
    int i;
//    network_handler_semafor++;
    for (i=0,p=interfaces;(i<MAX_INTERFACES && p->flags);i++,p++);
    if (i<MAX_INTERFACES) {
	p->zapas = ripc_bucket_new();
	if (!p->zapas) {
	    printk("FUCKUP: couldn't alloc zapas.\n");
	    close(fd);
//	    network_handler_semafor--;
	    return -1;
	}
	p->fd = fd;
	p->flags = IF_FLAG_CONNECTED;
	p->in_pckt = p->out_pckt = p->queue_pckt = NULL;
	p->in_ptr = p->out_ptr = NULL;
	p->in_cnt = p->out_cnt = 0;
	ripc_route_new_interface(i);
	RIPC_DEBUG("debug: new interface\n");
	networking++;
//	network_handler_semafor--;
	return i;
    } else {
	printk("FUCKUP: too many connections\n");
	close(fd);
//	network_handler_semafor--;
	return -1;
    }
}

static void
config_down_interface (int iff) {
    int i;
    for (i=0;i<MAX_CONFIG_CONNECTS;i++) {
	if (!config_connects[i].to) break;
	if (!config_connects[i].status) continue;
	if (config_connects[i].interface == iff) {
	    config_connects[i].interface = -1;
	    config_connects[i].status = 0;
	    break;
	}
    }
}

static void
ripc_interface_close (int i) {
    struct interface *p = &interfaces[i];
    if (!p->flags) return;
    config_down_interface(i);
    broadcast_table[i] = 0;
    p->flags = 0;
    if (p->in_pckt) ripc_bucket_done(p->in_pckt);
    if (p->zapas) ripc_bucket_done(p->zapas);
    if (p->out_pckt) {
	RIPC_DEBUG("debug: recovering out packet\n");
	if (lost_last) lost_last->next = p->out_pckt;
	else lost_queue = p->out_pckt;
	lost_last = p->out_pckt;
	p->out_pckt = NULL;
	lost_last->next = NULL;
    }
    if (p->queue_pckt) {
	RIPC_DEBUG("debug: recovering queue packets\n");
	if (lost_last) lost_last->next = p->queue_pckt;
	else {
	    lost_queue = p->queue_pckt;
	    lost_last = lost_queue;
	}
	while (lost_last->next) lost_last = lost_last->next;
    }
    close (p->fd);
    networking--;
    ripc_route_drop_interface(i);
    RIPC_DEBUG("debug: interface closed\n");
}

static int
ripc_route_unicast (struct pckt_bucket *p) {
    struct pckt_bucket *b;
    int to = p->pckt_dst;
    char min = 0;
    int iff = 0;
    int i;
    for (i=0;i<MAX_INTERFACES;i++) {
	if (!interfaces[i].flags) continue;
    	if (!interfaces[i].route[to]) continue;
    	if (!min || min>interfaces[i].route[to]) { min = interfaces[i].route[to]; iff = i; }
    }
    if (!min) {
        RIPC_DEBUG("FUCKUP: no route to target\n");
        ripc_bucket_done(p);
        return 0;
    }
    b = interfaces[iff].queue_pckt;
    if (!b) interfaces[iff].queue_pckt = p;
    else {
        while (b->next) b = b->next;
        b->next = p;
    }
    network_force = 1;
    return 1;
}

static int
ripc_route_broadcast (struct pckt_bucket *p) {
    int i, err=0;
    int one = -1;
    p->next = NULL;
    for (i=0;i<MAX_INTERFACES;i++) {
	if (i==p->from_interface) continue;
	if (interfaces[i].flags) {
	    struct pckt_bucket *b, *h;
	    b = ripc_bucket_new();
	    if (!b) {
		if (one<0) { one = i; continue; }
		printk("FUCKUP: not enough memory to make full broadcast\n");
		err = 1;
		break;
	    }	
	    memcpy(b, p, BUCKET_SIZE);
	    h = interfaces[i].queue_pckt;
	    if (!h) interfaces[i].queue_pckt = b;
	    else {
		while (h->next) h=h->next;
		h->next = b;
	    }
	}
    }
    if (one<0) ripc_bucket_done(p);
    else {
	struct pckt_bucket *h = interfaces[one].queue_pckt;
	if (!h) interfaces[one].queue_pckt = p;
	else {
	    while (h->next) h=h->next;
	    h->next = p;
	}
    }
    network_force = 1;
    return 1;
}


static void
ripc_lost_packet_handler (void) {
    unsigned char to;
    struct pckt_bucket *b;
    int i;
    // lost packets handling...
    while (lost_queue) {
	char min = 0;
	int iff = 0;
	struct pckt_bucket *p = lost_queue;
	lost_queue = p->next;
	p->next = NULL;
	to = p->pckt_dst;
	if (!to) {
	    ripc_bucket_done(p);
	    continue;
	}
	for (i=0;i<MAX_INTERFACES;i++) {
	    if (!interfaces[i].flags) continue;
	    if (!interfaces[i].route[to]) continue;
	    if (!min || interfaces[i].route[to]<min) { min = interfaces[i].route[to]; iff = i; }
	}
	if (!min) {
	    RIPC_DEBUG("FUCKUP: lost_packet_handler no route to target\n");
	    ripc_bucket_done(p);
	    continue;
	}
	b = interfaces[iff].queue_pckt;
	if (!b) interfaces[iff].queue_pckt = p;
	else {
	    while (b->next) b = b->next;
	    b->next = p;
	}
    }
    lost_last = NULL;
}

void fioasync_handler(int x);


static int
ripc_make_server_socket (char * name) {
    long flags;
    int fd;
    struct sockaddr_un	un;
    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) return fd;
    un.sun_family = AF_LOCAL;
    strncpy(un.sun_path, name, 108);
    un.sun_path[107] = '\0';
    unlink(un.sun_path);
    if (bind (fd, (struct sockaddr*)&un, sizeof(un)) == -1) { close (fd); return -1; }
    if ((flags = fcntl(fd, F_GETFL)) == -1) { close(fd); return -1; }
    flags |= O_NONBLOCK|O_ASYNC;
    if (fcntl(fd, F_SETFL, flags) == -1) { close(fd); return -1; }
    if (listen(fd, 5) < 0) { close (fd); return -1; }
    fcntl(fd,F_SETOWN,getpid());
//    signal(SIGIO,fioasync_handler);
    return fd;
}


static int
ripc_make_nonblock (int fd) {
    long flags;
    if ((flags = fcntl(fd, F_GETFL)) == -1) { close(fd); return -1; }
    flags |= O_NONBLOCK | O_ASYNC;
    if (fcntl(fd, F_SETFL, flags) == -1) { close(fd); return -1; }
    flags = 32;
    fcntl(fd,F_SETOWN,getpid());
    flags = 10;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &flags, sizeof(flags));
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &flags, sizeof(flags));
//    signal(SIGIO,fioasync_handler);
    return fd;
}


static int
ripc_make_client_socket (char * name) {
    long flags;
    int fd;
    struct sockaddr_un	un;
    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) return fd;
    un.sun_family = AF_LOCAL;
    strncpy(un.sun_path, name, 108);
    un.sun_path[107] = '\0';
    if (connect (fd, (struct sockaddr*)&un, sizeof(un)) < 0) { close(fd); return -1; }
    if ((flags = fcntl(fd, F_GETFL)) == -1) { close(fd); return -1; }
    flags |= O_NONBLOCK | O_ASYNC;
    if (fcntl(fd, F_SETFL, flags) == -1) { close(fd); return -1; }
    flags = 10;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &flags, sizeof(flags));
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &flags, sizeof(flags));
    fcntl(fd,F_SETOWN,getpid());
//    signal(SIGIO,fioasync_handler);
    return fd;
}

static int
ripc_init_interfaces (void) {
    if (max_interfaces && !interfaces) {
	interfaces = (void *)calloc(max_interfaces, sizeof(struct interface));
    }
    if (interfaces) return 1; else return 0;
}

static void
config_new_interface (char *name) {
    int i;
    int fd, iff=-1;
    fd = ripc_make_client_socket(name);
    if (fd>=0) {
        iff = ripc_interface_new(fd);
        if (iff<0) {
    	    close(fd);
	    fd = 0;
	} else fd = 1;
    } else {
        printk("Warning: couldn't connect to %s\n",name);
	fd = 0;
    }
    for (i=0;i<MAX_CONFIG_CONNECTS;i++) {
	if (config_connects[i].to) continue;
	if ((config_connects[i].to = strdup(name)) == NULL) {
	    printk("Warning: memory allocation failure, no reconnect for this interface.\n");
	} else {
	    config_connects[i].status = fd;
	    config_connects[i].interface = iff;
	}
	break;
    }
    if (i>=MAX_CONFIG_CONNECTS) {
	printk("Warning: too many 'connect' commands in your config file...\n");
	printk("         change MAX_CONFIG_CONNECTS in ipc.c and recompile,\n");
	printk("         or remove unnecesary 'connect' commands.\n");
	printk("Warning: no reconnecting for this interface.\n");
    }
}

static void
config_try_to_reconnect (void) {
    int fd;
    int iff;
    int i;
    for (i=0;i<MAX_CONFIG_CONNECTS;i++) {
	if (!config_connects[i].to) break;
	if (config_connects[i].status) continue;
	fd = ripc_make_client_socket(config_connects[i].to);
	if (fd<0) {
	    printk("Warning: couldn't connect to %s\n", config_connects[i].to);
	} else {
	    iff = ripc_interface_new(fd);
	    if (iff != -1) {
		config_connects[i].status = 1;
		config_connects[i].interface = iff;
		printk("Log: conected to %s\n", config_connects[i].to);
	    } else {
		close (fd);
	    }
	}
    }    
}





static void
ripc_read_config (void) {
    char linia[1024];
    char command[20];
    char path[110];
    int	net = 0;
    FILE *f;
    int got = 0;
    if ((f = fopen(RIPC_CONFIG, "r")) == NULL) {
	printk("-- FUCKUP: couldn't open config file\n");
    }
    while (fgets(linia, 1023, f)) {
	if (sscanf(linia, "%20s %109s\n", command, path) != 2) continue;
	if (linia[0] == '#' || linia[0] == '!' || linia[0] == '/') continue;
	printk("-- IPC config: requested '%s' to '%s'\n",command, path);
	if (!strcmp(command, "listen")) {
	    int fd;
	    net = 1;
	    if (!ripc_init_interfaces()) {
		printk("-- FUCKUP: network interfaces disabled or memory allocation failure.\n");
	    } else if (!got) {
		fd = ripc_make_server_socket(path);
		if (fd>0) {
		    ripc_listen_socket = fd; got++;
		} else {
		    printk("-- FUCKUP: error creating listening socket\n");
		}
	    } else {
		printk("-- FUCKUP: only one listening socket allowed\n");
	    }
	} else if (!strcmp(command, "connect")) {
	    net = 1;
	    if (!ripc_init_interfaces()) {
		printk("-- FUCKUP: network interfaces disabled or memory allocation failure.\n");
	    } else config_new_interface(path);
	} else if (!strcmp(command, "vs_number")) {
	    int vs = atoi(path);
	    if (net) {
		printk("-- FUCKUP: setting vs_number should occure before connect and listen.\n");
	    } else if (!vs) {
		printk("-- FUCKUP: bad vs_number argument '%s'\n", path);
	    } else {
		ipc_vs_number = vs;
	    }
	} else if (!strcmp(command, "system_max_streams")) {
	    int i = atoi(path);
	    if (i<1) {
		printk("-- FUCKUP: you should give at least 1 stream on system\n");
	    } else {
		system_max_streams = i;
	    }
	} else if (!strcmp(command, "max_interfaces")) {
	    int i = atoi(path);
	    if (net) {
		printk("-- WARNING: you cannot change max_interfaces after listen or connect\n");
	    } else if (i<1) {
		printk("-- WARNING: you have just disabled IPC networking\n");
		max_interfaces = 0;
	    } else {
		max_interfaces = i;
	    }
	} else if (!strcmp(command, "max_streams")) {
	    int i = atoi(path);
	    if (i<1) {
		printk("-- WARNING: do you want me to disable IPC streams?\n");
		max_streams = 0;
	    } else {
		max_streams = i;
	    }
	} else if (!strcmp(command, "max_blocks")) {
	    int i = atoi(path);
	    if (i<1) {
		printk("-- WARNING: do you want me to disable IPC blocks?\n");
		max_blocks = 0;
	    } else {
		max_blocks = i;
	    }
	} else if (!strcmp(command, "max_stream_buffers")) {
	    int i = atoi(path);
	    if (i<2) {
		printk("-- FUCKUP: you should give at least 2 stream buffers\n");
	    } else {
		max_stream_buffers = i;
	    }
	} else if (!strcmp(command, "bucket_max")) {
	    int i = atoi(path);
	    if (net) {
		printk("-- WARNING: you cannot change bucket_max after connect or listen.\n");
	    } else if (i<max_interfaces) {
		printk("-- FUCKUP: you should give at least 1 packet per interface\n");
	    } else {
		max_buckets = i;
	    }
	} else if (!strcmp(command, "default_ttl")) {
	    int i = atoi(path);
	    if (i<1 || i > 255) {
		printk("-- FUCKUP: illegal default ttl (should be 1..255)\n");
	    } else {
		default_ttl = i;
	    }
	} else {
	    printk("-- FUCKUP: bad config command '%s'\n", command);
	}
    }
    fclose(f);
}

static struct ipc_request *
ripc_pckt_to_request (struct pckt_bucket *p) {
    struct pckt_request	*d;
    struct ipc_request	*r;
    r = ipc_request_new();
    if (!r) return r;
    d = (void *)p->data;
    r->s.vcpu = ntohl(d->s_vcpu);
    r->s.vs = ntohl(d->s_vs);
    r->s.ipc_reg = ntohl(d->s_ipc_reg);
    r->d.vcpu = ntohl(d->d_vcpu);
    r->d.vs = ntohl(d->d_vs);
    r->d.ipc_reg = ntohl(d->d_ipc_reg);
    r->type = ntohl(d->type);
    r->id = ntohl(d->id);
    r->data[0] = ntohl(d->data[0]);
    r->data[1] = ntohl(d->data[1]);
    r->data[2] = ntohl(d->data[2]);
    r->data[3] = ntohl(d->data[3]);
    r->flags = ntohl(d->flags);
    r->next =  r->prev = NULL;
    r->targets = NULL;
    r->errcode = 0;
    gettimeofday (&r->timestamp, NULL);
    return r;    
}

// sending out request...
static int 
ripc_send_request (struct ipc_request *r) {
    struct pckt_bucket	*p;
    struct pckt_request *h;
    p = ripc_bucket_new ();
    if (!p) return -1;
    RIPC_DEBUG("debug: sending request\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.type = PCKT_REQUEST;
    h->h.ttl = DEFAULT_TTL;
    h->h.src = ipc_vs_number;
    if (r->d.vs>0) h->h.dst = r->d.vs;
    else { 
	h->h.dst = 0;
	h->h.id = htonl(ripc_give_id());
    }
    h->type = htonl(r->type);
    h->s_vcpu = htonl(r->s.vcpu);
    h->s_vs = htonl(r->s.vs);
    h->s_ipc_reg = htonl(r->s.ipc_reg);
    h->d_vcpu = htonl(r->d.vcpu);
    h->d_vs = htonl(r->d.vs);
    h->d_ipc_reg = htonl(r->d.ipc_reg);
    h->data[0] = htonl(r->data[0]);
    h->data[1] = htonl(r->data[1]);
    h->data[2] = htonl(r->data[2]);
    h->data[3] = htonl(r->data[3]);
    h->flags = htonl(r->flags & IPC_FLAG_NETMASK);
    h->id = htonl(r->id);
    p->len = htonl(sizeof(struct pckt_request));
    // ok we've got real packet.. so now route it...
    if (h->h.dst) {
	r->ripc_host_count = 1;
	return ripc_route_unicast(p);
    } else {
	r->ripc_host_count = known_hosts;
	p->from_interface = MAX_INTERFACES;
	return ripc_route_broadcast(p);
    }
}

static void
ripc_send_pckt_nack (struct pckt_bucket *r) {
    struct pckt_bucket	*p;
    struct pckt_resp	*h;
    struct pckt_request		*s;
    p = ripc_bucket_new ();
    if (!p) return;
    RIPC_DEBUG("debug: sending host NACK (packet)\n");
    p->next = NULL;
    h = (void *)p->data;
    s = (void *)r->data;
    h->h.dst = s->h.src;
    h->h.src = ipc_vs_number;
    h->h.type = PCKT_RESP;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->s_vcpu = s->s_vcpu;
    h->s_vs = s->s_vs;
    h->s_ipc_reg = s->s_ipc_reg;
    h->id = s->id;
    h->flags = htonl(IPC_FLAG_ERROR);
    h->errcode = htonl(IPC_ERR_NORESOURCES);
    p->len = htonl(sizeof(struct pckt_resp));
    ripc_route_unicast(p);    
}

static int
ripc_send_host_nack (struct ipc_request *r) {
    struct pckt_bucket	*p;
    struct pckt_resp	*h;
    p = ripc_bucket_new ();
    if (!p) return -1;
    RIPC_DEBUG("debug: sending host NACK|DONE\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.dst = r->s.vs;
    h->h.src = ipc_vs_number;
    h->h.type = PCKT_RESP;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->s_vcpu = htonl(r->s.vcpu);
    h->s_vs = htonl(r->s.vs);
    h->s_ipc_reg = htonl(r->s.ipc_reg);
    h->type = htonl(r->type);
    h->id = htonl(r->id);
    h->data[0] = htonl(r->data[0]);
    h->data[1] = htonl(r->data[1]);
    h->data[2] = htonl(r->data[2]);
    h->data[3] = htonl(r->data[3]);
    h->flags = htonl(IPC_FLAG_ERROR);
    h->errcode = htonl(r->errcode);
    p->len = htonl(sizeof(struct pckt_resp));
    return ripc_route_unicast(p);
}

// someone accepted this request...
static int
ripc_send_ack (struct ipc_request *r) {
    struct pckt_bucket	*p;
    struct pckt_resp	*h;
    p = ripc_bucket_new ();
    if (!p) return -1;
    RIPC_DEBUG("debug: sending ACK\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.dst = r->s.vs;
    h->h.src = ipc_vs_number;
    h->h.type = PCKT_RESP;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->s_vcpu = htonl(r->s.vcpu);
    h->s_vs = htonl(r->s.vs);
    h->s_ipc_reg = htonl(r->s.ipc_reg);
    h->f_vcpu = htonl(r->f.vcpu);
    h->f_vs = htonl(r->f.vs);
    h->f_ipc_reg = htonl(r->f.ipc_reg);
    h->type = htonl(r->type);
    h->id = htonl(r->id);
    h->data[0] = htonl(r->data[0]);
    h->data[1] = htonl(r->data[1]);
    h->data[2] = htonl(r->data[2]);
    h->data[3] = htonl(r->data[3]);
    h->flags = htonl(IPC_FLAG_ACCEPTED);
    p->len = htonl(sizeof(struct pckt_resp));
    return ripc_route_unicast(p);
}

static int
ripc_send_host_ack (struct ipc_request *r) {
    struct pckt_bucket		*p;
    struct pckt_host_ack	*h;
    p = ripc_bucket_new ();
    if (!p) return -1;
    RIPC_DEBUG("debug: sending host ACK\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.dst = r->s.vs;
    h->h.src = ipc_vs_number;
    h->h.type = PCKT_HOST_ACK;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0; // we don't need this
    h->type = htonl(r->type);
    h->vcpu = htonl(r->s.vcpu);
    h->vs = htonl(r->s.vs);
    h->ipc_reg = htonl(r->s.ipc_reg);
    h->id = htonl(r->id);
    p->len = htonl(sizeof(struct pckt_host_ack));
    return ripc_route_unicast(p);
}

static int
ripc_send_host_done (struct ipc_request *r) {
    struct pckt_bucket		*p;
    struct pckt_resp		*h;
    p = ripc_bucket_new ();
    if (!p) return -1;
    RIPC_DEBUG("debug: sending host DONE\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.dst = r->s.vs;
    h->h.src = ipc_vs_number;
    h->h.type = PCKT_RESP;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->s_vcpu = htonl(r->s.vcpu);
    h->s_vs = htonl(r->s.vs);
    h->s_ipc_reg = htonl(r->s.ipc_reg);
    h->f_vcpu = htonl(r->f.vcpu);
    h->f_vs = htonl(r->f.vs);
    h->f_ipc_reg = htonl(r->f.ipc_reg);
    h->type = htonl(r->type);
    h->id = htonl(r->id);
    h->data[0] = htonl(r->data[0]);
    h->data[1] = htonl(r->data[1]);
    h->data[2] = htonl(r->data[2]);
    h->data[3] = htonl(r->data[3]);
    h->flags = htonl(r->flags & IPC_FLAG_NETMASK);
    h->errcode = htonl(r->errcode);
    p->len = htonl(sizeof(struct pckt_resp));
    return ripc_route_unicast(p);
}

static void
ripc_host_ack (struct pckt_bucket *p) {
    struct pckt_host_ack *h;
    struct ipc_request *r;
    int c, id;
    h = (void *)p->data;
    c = ntohl(h->vcpu);
    id = ntohl(h->id);
    if (!(cpu[c].flags & VCPU_FLAG_USED)) return;
    for (r=proces_requests[c];((r) && (r->id!=id));r=r->next);
    if (!r) return;
    gettimeofday(&r->timestamp, NULL);	// touch, so it can stay alive some more
}

static int
ripc_interface_read (struct interface *p, int i) {
    int ile;
    if (!p->in_pckt) {
	// starting to read new packet
	p->in_pckt = ripc_bucket_new();
	if (!p->in_pckt) return 0;
	p->in_pckt->from_interface = i;
	p->in_ptr = (char *)&p->in_pckt->len;
	p->in_cnt = sizeof(unsigned int);	
	RIPC_LOG("starting to read new packet\n");
    }
    if (p->in_ptr < p->in_pckt->data) {
	ile = recv(p->fd, p->in_ptr, p->in_cnt, 0);
	if ((ile<0)&&(errno==EWOULDBLOCK)) return 0;
        if (ile<1) {
	    RIPC_DEBUG("debug: socket reading error or EOF\n");
	    ripc_interface_close(i);
	    return 0;
	}
	p->in_cnt -= ile;
	p->in_ptr += ile;
	if (p->in_cnt) return 0;
	p->in_cnt = ntohl(p->in_pckt->len);
	if ((p->in_cnt > LEN_MAX) || (p->in_cnt < HEAD_SIZE)) {
	    RIPC_LOG("FUCKUP: illegal packet recived\n");
	    ripc_interface_close(i);
	    return 0;
	}
    }
    ile = recv(p->fd, p->in_ptr, p->in_cnt, 0);
    if ((ile<0)&&(errno==EWOULDBLOCK)) return 0;
    if (ile<1) {
	RIPC_DEBUG("debug: socket reading error or EOF\n");
	ripc_interface_close(i);
	return 0;
    }
    p->in_cnt -= ile;
    p->in_ptr += ile;
    if (p->in_cnt) return 0;
	RIPC_LOG("got full packet\n");

    // route updates are special!!
    if (p->in_pckt->pckt_type == PCKT_ROUTE) {
	ripc_route_request(p->in_pckt);
	ripc_bucket_done(p->in_pckt);
	p->in_pckt = NULL;
	return 1;
    }
    if (postprocess_last) postprocess_last->next = p->in_pckt;
	else postprocess_queue = p->in_pckt;
    p->in_pckt->next = NULL;
    postprocess_last = p->in_pckt;
    p->in_pckt = NULL;
    return 1;
}

static int
ripc_interface_write (struct interface *p, int i) {
    int hmm = 0;
    int ile;
    if (!p->out_pckt) {
	if (p->flags & IF_FLAG_NEED_UPDATE) {
	    hmm = ripc_create_route_pckt(p->zapas, i);
	}
	if (hmm) {
	    RIPC_DEBUG("debug: begining to send route update\n");
	    p->out_pckt = p->zapas;
	    p->zapas = NULL;
	} else {
	    if (!p->queue_pckt) return 0;
	    RIPC_LOG("begining to send new packet\n");
	    p->out_pckt = p->queue_pckt;
	    p->queue_pckt = p->queue_pckt->next;
	}
	p->out_cnt = ntohl(p->out_pckt->len) + sizeof(p->out_pckt->len);
	p->out_ptr = (char *)&p->out_pckt->len;
    }
    if (!p->out_pckt) return 0;
    ile = send(p->fd, p->out_ptr, p->out_cnt, 0);
    if (ile<0) {
	if (errno == EPIPE) {
	    RIPC_DEBUG("FUCKUP: broken pipe\n");
	    ripc_interface_close(i);
	}
	return 0;
    }
    p->out_ptr += ile;
    p->out_cnt -= ile;
    if (p->out_cnt) return 0;
    RIPC_LOG("sent full packet\n");
    if (!p->zapas) p->zapas = p->out_pckt;
	else ripc_bucket_done(p->out_pckt);
    p->out_pckt = NULL;
    p->out_ptr = NULL;
    p->out_cnt = 0;
    return 1;
}

static void
ipc_register (int c) {
    char buf[512];
    int t_ipcreg = cpu[c].uregs[0];
    if (!t_ipcreg) { 
	cpu[c].ipc_reg = 0;
	return;
    }
    snprintf(buf, 511, "ipc/ipcreg/%d",t_ipcreg);
    buf[sizeof(buf) - 1] = 0;		// to be sure...
    VALIDATE(c, buf, "ipc/register");
    cpu[c].ipc_reg = t_ipcreg;
    return;
}

// !!!!!!!!!!!!!!!!!!!! IPC_MSG !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


static void
ipc_msg_send (int c) {
    char 		buffer[100];
    struct ipc_request	* r;
    struct ipc_queue	* q;
    struct msg_sleeper	* s;
    unsigned int	data1, data2, ipc_reg, flags;
    signed int		vcpu, vs;
    // first we check if registered...
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC", c);
	return;
    }
    flags = cpu[c].uregs[0];
    if (flags & ~(IPC_FLAG_NONBLOCK|IPC_FLAG_MULTICAST)) {
	non_fatal (ERROR_IPC_BAD_FLAGS, "Bad flags passed", c);
	return;
    }
    ipc_reg = cpu[c].uregs[1];
    data1 = cpu[c].uregs[2];
    data2 = cpu[c].uregs[3];
    vcpu = cpu[c].sregs[0];
    vs = cpu[c].sregs[1];		
    if (!networking) {
	if ((vs>0) && (vs != ipc_vs_number)) {
	    non_fatal (ERROR_IPC_NO_TARGET, "No such target (no networking)",c);
	    return;
	}
	if ((vcpu < -1) || (vcpu > MAX_VCPUS)) {
	    non_fatal (ERROR_IPC_BAD_TARGET, "Illegal target specified",c);
	    return;
	}
    } 
    if (vs != -1  && vcpu != -1  && ipc_reg != 0) {
	sprintf(buffer, "ipc/target/unicast/%d/%d/%d", vs,vcpu,ipc_reg);
    } else {
	strcpy(buffer, "ipc/target/broadcast");
    }
    VALIDATE(c, buffer, "ipc/msg/send");
    r = ipc_request_new();
    if (!r) {
	non_fatal (ERROR_IPC_NOMEM, "Memory allocation failure",c);
	return;
    }
    r->s.vcpu = c;
    r->s.vs   = ipc_vs_number;
    r->s.ipc_reg = cpu[c].ipc_reg;
    r->d.vcpu = vcpu;
    r->d.vs = vs;
    r->d.ipc_reg = ipc_reg;
    r->type = IPC_MSG;
    r->id = ipc_new_id();
    r->msg_data1 = data1;
    r->msg_data2 = data2;
    r->errcode = 0;
    r->flags = flags;
    gettimeofday(&r->timestamp, NULL);
    if (vs<1 || vs==ipc_vs_number) {
	if (!ipc_make_targets(c, r)) {
	    ipc_request_done(r);
	    non_fatal (ERROR_IPC_NOMEM, "Memory allocation failure",c);
	    return;
	}
    }
    // now check if we can deliver this to someone...
    s = msg_sleepers;
    while (s && r->targets) {
	struct msg_sleeper *ms = s->next;
	for (q=r->targets; ((q->vcpu!=s->vcpu) && q); q=q->next_r);
	if (!q) { s=ms; continue; }
	if (!(r->flags & IPC_FLAG_ACCEPTED)) {
	    r->f.vcpu = s->vcpu;
	    r->f.vs   = ipc_vs_number;
	    r->f.ipc_reg = cpu[s->vcpu].ipc_reg;
	    r->flags |= IPC_FLAG_ACCEPTED;
	}
	if (s->prev) s->prev->next = s->next;
	    else msg_sleepers = s->next;
	if (s->next) s->next->prev = s->prev;
	s->next = s->prev = NULL;
	IPC_UNLINK_QUEUE(q,r);
	cpu[s->vcpu].state -= VCPU_STATE_IPCWAIT;
	cpu[s->vcpu].iowait_id = 0;
	cpu[s->vcpu].uregs[0] = r->s.vcpu;
	cpu[s->vcpu].uregs[1] = r->s.vs;
	cpu[s->vcpu].uregs[2] = r->s.ipc_reg;
	cpu[s->vcpu].uregs[3] = r->msg_data1;
	cpu[s->vcpu].uregs[4] = r->msg_data2;
	s = ms;
    }
    if (networking && vs!=0 && vs!=ipc_vs_number) {
	int ret = ripc_send_request(r); 
	if (!ret) { // unicast msg route failure..
	    r->ripc_host_count = 0;
	}
    }
    
    if (!r->targets && !r->ripc_host_count && !(r->flags & IPC_FLAG_HAD_TARGETS)) {
	ipc_request_done(r);
	non_fatal (ERROR_IPC_NO_TARGET, "No such target, or no route to target",c);
	return;    
    }
    if (!r->targets && !r->ripc_host_count) r->flags |= IPC_FLAG_COMPLETED;
    if (flags & IPC_FLAG_NONBLOCK) {
	struct ipc_request *tr = proces_requests[c];
	if (!tr) {
		r->next = r->prev = NULL;
		proces_requests[c] = r;
    	} else {
	    while (tr->next) tr=tr->next;
	    r->next = NULL;
	    r->prev = tr;
	    tr->next = r;
	}
	cpu[c].uregs[0] = r->id;
	return;
    } else {
	struct ipc_request *tr = proces_requests[c];
	if (r->flags & IPC_FLAG_COMPLETED)  {
	    cpu[c].uregs[0] = r->f.vcpu;
	    cpu[c].uregs[1] = r->f.vs;
	    cpu[c].uregs[2] = r->f.ipc_reg;
	    ipc_request_done(r);
	    return;
	}
	if (!tr) {
		r->next = r->prev = NULL;
		proces_requests[c] = r;
    	} else {
	    while (tr->next) tr=tr->next;
	    r->next = NULL;
	    r->prev = tr;
	    tr->next = r;
	}
	if ((r->flags & IPC_FLAG_ACCEPTED) && !(flags & IPC_FLAG_MULTICAST)) {
	    cpu[c].uregs[0] = r->f.vcpu;
	    cpu[c].uregs[1] = r->f.vs;
	    cpu[c].uregs[2] = r->f.ipc_reg;
	} else {
	    cpu[c].state |= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = IOWAIT_ID_MSG_SEND;
	    r->flags |= IPC_FLAG_SENDERSLEEP;
	}
	return;
    }
}

static void
ripc_msg_send (struct ipc_request *r) {
    struct ipc_request 	*tr;
    struct ipc_queue	*q;
    struct msg_sleeper	*s;
    if (!ipc_make_targets(MAX_VCPUS+1, r)) {
	r->flags = IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NORESOURCES;
	ripc_send_host_nack(r);
	ipc_request_done(r);
	return;
    }
    if (!r->targets) {
	r->flags = IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NOTARGET;
	ripc_send_host_nack(r);
	ipc_request_done(r);
	return;
    }
    if (r->flags & IPC_FLAG_ACCEPTED) r->flags |= IPC_FLAG_ACK_SENT;
    s = msg_sleepers;
    while (s && r->targets) {
	struct msg_sleeper *ms = s->next;
	for (q=r->targets; ((q->vcpu!=s->vcpu) && q); q=q->next_r);
	if (!q) { s=ms; continue; }
	if (!(r->flags & IPC_FLAG_ACCEPTED)) {
	    r->f.vcpu = s->vcpu;
	    r->f.vs   = ipc_vs_number;
	    r->f.ipc_reg = cpu[s->vcpu].ipc_reg;
	    r->flags |= IPC_FLAG_ACCEPTED;
	}
	// unlink from sleepers and from request
	if (s->prev) s->prev->next = s->next;
	    else msg_sleepers = s->next;
	if (s->next) s->next->prev = s->prev;
	s->next = s->prev = NULL;
	IPC_UNLINK_QUEUE(q,r);
	cpu[s->vcpu].state -= VCPU_STATE_IPCWAIT;
	cpu[s->vcpu].iowait_id = 0;
	cpu[s->vcpu].uregs[0] = r->s.vcpu;
	cpu[s->vcpu].uregs[1] = r->s.vs;
	cpu[s->vcpu].uregs[2] = r->s.ipc_reg;
	cpu[s->vcpu].uregs[3] = r->msg_data1;
	cpu[s->vcpu].uregs[4] = r->msg_data2;
	s = ms;
    }
    if (!r->targets) {
	r->flags |= IPC_FLAG_COMPLETED;
	ripc_send_host_done(r);
	ipc_request_done(r);
	return;
    } else if ((r->flags & IPC_FLAG_ACCEPTED) && !(r->flags & IPC_FLAG_ACK_SENT)) {
	int ret = ripc_send_ack(r);
	if (ret>0) r->flags |= IPC_FLAG_ACK_SENT;
    } else {
	ripc_send_host_ack(r);
    }
    tr = network_requests;
    if (!tr) {
	r->next = r->prev = NULL;
	network_requests = r;
    } else {
	while (tr->next) tr=tr->next;
	tr->next = r;
	r->prev = tr;
	r->next = NULL;
    }
    return;
}

static void
ipc_msg_recv (int c) {
    char		buffer[100];
    struct ipc_request	*r;
    struct ipc_queue	*q;
    struct msg_sleeper	*s;
    unsigned int	flags;
    int			send_ack = 0;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC", c);
	return;
    }
    flags = cpu[c].uregs[0];
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Bad flags passed", c);
	return;
    }

restart:
    for (q=proces_incoming[c];q;q=q->next) {
	if (q->req->type != IPC_MSG) continue;
	sprintf(buffer, "ipc/source/%d/%d/%d", q->req->s.vs, q->req->s.vcpu, q->req->s.ipc_reg);
	if (is_permitted(c, buffer, "ipc/msg/recv")) break;
	r = q->req;
	IPC_UNLINK_QUEUE(q,r);
	ipc_queue_done(q);
	if (!r->ripc_host_count && !r->targets) {
	    if (!r->flags & IPC_FLAG_ACCEPTED) {
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NACK;
	    } else r->flags |= IPC_FLAG_COMPLETED;
	    if (r->s.vs != ipc_vs_number) {
		ripc_send_host_done(r);
		IPC_UNLINK_NET_REQUEST(r);
		ipc_request_done(r);
	    } else {
		if (r->flags & IPC_FLAG_NONBLOCK) {
		    gettimeofday(&r->timestamp, NULL);
		} else {
		    if (r->flags & IPC_FLAG_SENDERSLEEP) {
			if (r->flags & IPC_FLAG_ERROR) {
			    IPC_UNLINK_REQUEST(r, r->s.vcpu);
			    non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped", r->s.vcpu);
			    ipc_request_done(r);
			    // fuck, we must restart
			    goto restart;
			} else {
			    IPC_UNLINK_REQUEST(r, r->s.vcpu);
			    cpu[r->s.vcpu].uregs[0] = r->f.vcpu;
			    cpu[r->s.vcpu].uregs[1] = r->f.vs;
			    cpu[r->s.vcpu].uregs[2] = r->f.ipc_reg;
			    cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
			    cpu[r->s.vcpu].iowait_id = 0;
			    r->flags -= IPC_FLAG_SENDERSLEEP;
			    ipc_request_done(r);
			}
		    } else {
			IPC_UNLINK_REQUEST(r, r->s.vcpu);
			ipc_request_done(r);
		    }
		}
	    }
	}
    };
    if (q) {
	r = q->req;
	gettimeofday(&r->timestamp, NULL); // touching it..
	if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_EOK;
	cpu[c].uregs[0] = r->s.vcpu;
	cpu[c].uregs[1] = r->s.vs;
	cpu[c].uregs[2] = r->s.ipc_reg;
	cpu[c].uregs[3] = r->msg_data1;
	cpu[c].uregs[4] = r->msg_data2;
	if (!(r->flags & IPC_FLAG_ACCEPTED)) {
	    r->flags |= IPC_FLAG_ACCEPTED;
	    r->f.vcpu = c;
	    r->f.vs = ipc_vs_number;
	    r->f.ipc_reg = cpu[c].ipc_reg;
	}
	if ((r->s.vs != ipc_vs_number) && (r->flags & IPC_FLAG_ACCEPTED) && !(r->flags & IPC_FLAG_ACK_SENT))
	    send_ack = 1;	
	IPC_UNLINK_QUEUE(q,r);
	ipc_queue_done(q);
	if (!r->ripc_host_count && !r->targets) r->flags |= IPC_FLAG_COMPLETED;
	if (send_ack && !(r->flags & IPC_FLAG_COMPLETED)) {
	    int ret = ripc_send_ack(r);
	    if (ret==1) r->flags |= IPC_FLAG_ACK_SENT;
	}
	if (r->flags & IPC_FLAG_NONBLOCK) {
	    return;
	}
	if (r->flags & IPC_FLAG_SENDERSLEEP)
	    if (((r->flags & IPC_FLAG_MULTICAST) && (r->flags & IPC_FLAG_COMPLETED))
		|| !(r->flags & IPC_FLAG_MULTICAST)) {
		cpu[r->s.vcpu].uregs[0] = r->f.vcpu;
		cpu[r->s.vcpu].uregs[1] = r->f.vs;
		cpu[r->s.vcpu].uregs[2] = r->f.ipc_reg;
		cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[r->s.vcpu].iowait_id = 0;
		r->flags -= IPC_FLAG_SENDERSLEEP;
	    }
	if (r->flags & IPC_FLAG_COMPLETED) {
	    if (r->s.vs != ipc_vs_number) {
	    	ripc_send_host_done (r);
	    	IPC_UNLINK_NET_REQUEST(r);
		ipc_request_done(r);
	    } else {
		IPC_UNLINK_REQUEST(r, r->s.vcpu);
		ipc_request_done(r);
	    }
	}
    } else {
	if (flags & IPC_FLAG_NONBLOCK) {
	    cpu[c].sregs[0] = IPC_ETRYAGAIN;
	    return;
	}
	if (!msg_sleepers) {
	    msg_sleepers = &sleepers[c];
	    sleepers[c].next = NULL;
	    sleepers[c].prev = NULL;
	} else {
	    s = msg_sleepers;
	    while (s->next) s=s->next;
	    s->next = &sleepers[c];
	    sleepers[c].prev = s;
	    sleepers[c].next = NULL;
	}
	// go asleep
	cpu[c].state |= VCPU_STATE_IPCWAIT;
	cpu[c].iowait_id = IOWAIT_ID_MSG_RECV;
    }
}

static void
ripc_msg_recv (struct pckt_bucket *p) {
    struct pckt_resp		*h;
    struct ipc_request		*r;
    int c, id, vcpu, vs, ipc_reg, flags, errcode;
    h = (void *)p->data;
    c = ntohl(h->s_vcpu);
    id = ntohl(h->id);
    vcpu = ntohl(h->f_vcpu);
    vs = ntohl(h->f_vs);
    ipc_reg = ntohl(h->f_ipc_reg);
    flags = ntohl(h->flags);
    errcode = ntohl(h->errcode);

    ripc_bucket_done(p);

    if (!(cpu[c].flags & VCPU_FLAG_USED)) return;
    for (r=proces_requests[c];((r) && (r->id!=id));r=r->next);
    if (!r) return;
    if (r->flags & (IPC_FLAG_ERROR|IPC_FLAG_COMPLETED)) return;
    if (!(flags & IPC_FLAG_ERROR)) gettimeofday(&r->timestamp, NULL);	// touch, so it can stay alive some more
    if (flags & (IPC_FLAG_ERROR|IPC_FLAG_COMPLETED)) r->ripc_host_count--;
    if (!(r->flags & IPC_FLAG_ACCEPTED) && (flags & IPC_FLAG_ACCEPTED)) {
	r->flags |= (IPC_FLAG_HAD_TARGETS|IPC_FLAG_ACCEPTED);
	r->f.vcpu = vcpu;
	r->f.vs = vs;
	r->f.ipc_reg = ipc_reg;
    }    
    if (!r->ripc_host_count & !r->targets) {
	if (r->flags & IPC_FLAG_ACCEPTED) {
	    r->flags |= IPC_FLAG_COMPLETED;
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[c].uregs[0] = r->f.vcpu;
		cpu[c].uregs[1] = r->f.vs;
		cpu[c].uregs[2] = r->f.ipc_reg;
		cpu[c].state -= VCPU_STATE_IPCWAIT;
		cpu[c].iowait_id = 0;
		r->flags -= IPC_FLAG_SENDERSLEEP;
	    }
	    if (r->flags & IPC_FLAG_NONBLOCK) return;
	    IPC_UNLINK_REQUEST(r, c);
	    ipc_request_done(r);
	    return;
	}
	r->flags |= IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NOTARGET;
	if (r->flags & IPC_FLAG_SENDERSLEEP) {
	    IPC_UNLINK_REQUEST(r, c);
	    cpu[c].state -= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = 0;
	    non_fatal(ERROR_IPC_NO_TARGET, "No such target",c);
	    ipc_request_done(r);
	    return;
	}
	if (r->flags & IPC_FLAG_NONBLOCK) return;
	IPC_UNLINK_REQUEST(r, c);
	ipc_request_done(r);
	return;
    }
    // request not done.. wake up if needed... and go on...
    if (r->flags & IPC_FLAG_SENDERSLEEP)
	if (((r->flags & IPC_FLAG_MULTICAST) && (r->flags & IPC_FLAG_COMPLETED))
	    || !(r->flags & IPC_FLAG_MULTICAST)) {
	    cpu[c].uregs[0] = r->f.vcpu;
	    cpu[c].uregs[1] = r->f.vs;
	    cpu[c].uregs[2] = r->f.ipc_reg;
	    cpu[c].state -= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = 0;
	    r->flags -= IPC_FLAG_SENDERSLEEP;
	} 
}


static void
ipc_msg_stat (int c) {
    struct ipc_request	*r;
    unsigned int 	id;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC", c);
	return;
    }
    id = cpu[c].uregs[0];
    for (r=proces_requests[c];(r && (r->id != id)); r=r->next);
    if ((!r) || (!(r->flags & IPC_FLAG_NONBLOCK))) {
	non_fatal (ERROR_IPC_NO_REQUEST, "Bad request id provided",c);
	return;
    }

    if (r->flags & IPC_FLAG_ERROR) { 
	cpu[c].sregs[0] = IPC_RSTATUS_ERROR;
	cpu[c].uregs[0] = r->errcode;
    } else if (r->flags & IPC_FLAG_COMPLETED) {
	cpu[c].sregs[0] = IPC_RSTATUS_COMPLETED;
        cpu[c].uregs[0] = r->f.vcpu;
        cpu[c].uregs[1] = r->f.vs;
        cpu[c].uregs[2] = r->f.ipc_reg;
    } else if (r->flags & IPC_FLAG_ACCEPTED) {
	cpu[c].sregs[0] = IPC_RSTATUS_ACCEPTED;
	if (!(r->flags & IPC_FLAG_MULTICAST)) {
    	    cpu[c].uregs[0] = r->f.vcpu;
	    cpu[c].uregs[1] = r->f.vs;
    	    cpu[c].uregs[2] = r->f.ipc_reg;
	}
	return; // do not destroy request...
    } else {
	cpu[c].sregs[0] = IPC_RSTATUS_WAITING;
	return; // do not destroy request..
    }
    IPC_UNLINK_REQUEST(r, c);
    ipc_request_done (r);
}

static void
ipc_msg_init (void) {
    int i;
    for (i=0;i<MAX_VCPUS;i++) {
	sleepers[i].vcpu = i;
	sleepers[i].prev = sleepers[i].next = NULL;
    }
}

static void
ipc_msg_task_cleanup (int c) {
    struct msg_sleeper 	*ms;
    // someone else will destroy our requests....
    // we don't have to wakeup our client, coz he's going to die right now
    ms = &sleepers[c];
    if ((ms->next) || (ms->prev) || (msg_sleepers==ms)) {
	if (ms->prev) ms->prev->next = ms->next;
	    else msg_sleepers = ms->next;
	if (ms->next) ms->next->prev = ms->prev;
	ms->next = ms->prev = NULL;
    }
}

static void
ipc_msg_module_unload (void) {
    // someone else will wake up our senders/readers...
}


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!! STREAM !!!!!!!!!!!!!!!!!!!!!!!!!!!1


static void *
ipc_buffer_new (void) {
    void *p;
    p = buffers;
    if (p) {
	buffers = buffers->next;
    } else {
	if (buffers_count < MAX_STREAM_BUFFERS) {
	    p = (void *) malloc (sizeof(struct buffer));
	    if (p) buffers_count++;
	}
    }
    return p;
}

static void
ipc_buffer_done (void *p) {
    struct buffer_bucket *k = p;
    k->next = buffers;
    buffers = k;
}

static void
ipc_stream_init (void) {
    int i;
    struct stream	**p;
    streams = (void *) calloc(GLOBAL_MAX_STREAMS, sizeof(struct stream));
    p = (void *) calloc(MAX_STREAMS * MAX_VCPUS, sizeof(struct stream*));
    if (!p || !streams) {
	if (p) free(p);
	if (streams) free(streams);
	proces_streams[0] = NULL;
	streams = NULL;
	MAX_STREAMS = 0;
	GLOBAL_MAX_STREAMS = 0;
	printk("-- WARINIG: memory failure when initialising streams, streams disabled.\n");
    } else {
	for (i=0; i<MAX_VCPUS; i++)
	    proces_streams[i] = &p[i*MAX_STREAMS];
    }
}

static void
ripc_stream_send_ack_nack (struct pckt_bucket *s) {
    struct pckt_bucket		*p;
    struct pckt_stream_ack_ack	*h;
    struct pckt_resp		*r;
    
    p = ripc_bucket_new();
    if (!p) return;
    p->next = NULL;
    h = (void *)p->data;
    r = (void *)s->data;

    RIPC_DEBUG("debug: sending stream ACK NACK\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.type = PCKT_STREAM_ACK_ACK;
    h->h.ttl = DEFAULT_TTL;
    h->h.src = ipc_vs_number;
    h->h.dst = r->h.src;
    h->h.id = 0;
    h->s_vcpu = r->s_vcpu;
    h->s_vs = r->s_vs;
    h->s_stream = r->str_req_str;
    h->d_vcpu = r->f_vcpu;
    h->d_vs = ipc_vs_number;	// pertend that someone else accepted this stream
    h->d_stream = r->str_dst_str;
    h->id = htonl(r->id);
    p->len = htonl(sizeof(struct pckt_stream_ack_ack));
}

static void
ripc_stream_send_reset (struct pckt_bucket *s) {
    struct pckt_bucket		*p;
    struct pckt_stream_data	*d;
    struct pckt_stream_reset	*h;
    
    p = ripc_bucket_new();
    if (!p) return;
    p->next = NULL;
    h = (void *)p->data;
    d = (void *)s->data;

    RIPC_DEBUG("debug: sending stream RESET.\n");    
    h->h.dst = d->h.src;
    h->h.src = d->h.dst;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->h.type = PCKT_STREAM_RESET;
    h->s_vcpu = d->d_vcpu;
    h->s_vs = d->d_vs;
    h->s_stream = d->d_stream;
    h->d_vcpu = d->s_vcpu;
    h->d_vs = d->s_vs;
    h->d_stream = d->s_stream;
    h->id = d->id;
    p->len = htonl(sizeof(struct pckt_stream_reset));
    ripc_route_unicast(p);
}

static void
ripc_stream_send_ack_ack (struct stream *s) {
    struct pckt_bucket		*p;
    struct pckt_stream_ack_ack	*h;
    p = ripc_bucket_new();
    if (!p) return;
    RIPC_DEBUG("debug: sending stream ACK ACK.\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.type = PCKT_STREAM_ACK_ACK;
    h->h.ttl = DEFAULT_TTL;
    h->h.src = ipc_vs_number;
    h->h.dst = s->peer_vs;
    h->s_vcpu = htonl(s->my_vcpu);
    h->s_vs = htonl(ipc_vs_number);
    h->s_stream = htonl(s->my_global_nr);
    h->d_vcpu = htonl(s->peer_vcpu);
    h->d_vs = htonl(s->peer_vs);
    h->d_stream = htonl(s->peer_stream);
    h->id = htonl(s->id);
    p->len = htonl(sizeof(struct pckt_stream_ack_ack));
    ripc_route_unicast(p);
}

static void
ripc_stream_send_data (struct stream *s) {
    struct pckt_bucket		*p;
    struct pckt_stream_data	*h;
    char 			*d;
    int				flags=0;

    if (s->flags & STR_PEERDEAD) return; // do nothing when peer is dead

    p = ripc_bucket_new();
    if (!p) return;
    p->next = NULL;
    h = (void *)p->data;
    h->h.type = PCKT_STREAM_DATA;
    h->h.src = ipc_vs_number;
    h->h.dst = s->peer_vs;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->s_vcpu = htonl(s->my_vcpu);
    h->s_vs = htonl(ipc_vs_number);
    h->s_stream = htonl(s->my_global_nr);
    h->d_vs = htonl(s->peer_vs);
    h->d_vcpu = htonl(s->peer_vcpu);
    h->d_stream = htonl(s->peer_stream);
    h->id = htonl(s->id);
    flags |= STR_DATA_ACK; 
    h->ack = htonl(s->peer_count);
    if (!s->data) flags |= STR_DATA_READY;
    if ((s->out->head != s->out->tail) && !(s->flags & STR_PEERCLOSING)) {
	// got some data to send..
	h->pckt_number = htonl(s->my_count);
	flags |= STR_DATA_DATA;
	d = (char *)h + sizeof(struct pckt_stream_data);
	if (s->out->net_tail != s->out->tail) {
	    // we retransmit packet..
	    // copy copy...
	    int cnt=0;
	    int i = s->out->tail;
	    RIPC_DEBUG("debug: stream DATA retransmission\n");
	    while (i!=s->out->net_tail) {
		*(d++) = s->out->buf[i++];
		if (i==s->out->len) i = 0;
		cnt++;
	    }
	    h->load = htonl(cnt);
	    p->len = htonl(sizeof(struct pckt_stream_data) + cnt);
	} else {
	    // transmit packet...
	    int cnt = 0;
	    int i = s->out->tail;
	    RIPC_DEBUG("debug: stream DATA transmission\n");
	    while (i!=s->out->head) {
		*(d++) = s->out->buf[i++];
		if (i==s->out->len) i=0;
		cnt++;
		if (cnt==MAX_STREAM_PAYLOAD) break;
	    }
	    if (s->flags & STR_CLOSING) flags |= STR_DATA_CLOSING;
	    s->out->net_tail = i;
	    h->load = htonl(cnt);
	    p->len = htonl(sizeof(struct pckt_stream_data) + cnt);
	}
    } else {
	if (s->flags & STR_CLOSING) { 
	    RIPC_DEBUG("debug: stream sending closing packet\n");
	    flags |= STR_DATA_CLOSING;
	    h->pckt_number = htonl(s->my_count);
	} else {
	    RIPC_DEBUG("debug: stream sending ping packet\n");
	    h->pckt_number = 0;	
	}
	// no data, just communication/closing
	h->load = 0;
	p->len = htonl(sizeof(struct pckt_stream_data));
    }
    h->flags = htonl(flags);
    RIPC_DEBUG("debug: sending packet acking %d with number %d\n", s->peer_count, (int)ntohl(h->pckt_number));
    gettimeofday(&s->pong, NULL);    
    if (!ripc_route_unicast(p)) {
	// no route to target host...
	s->flags |= STR_PEERDEAD;
    } else RIPC_LOG("enqueued new stream packet\n");
}

static void
ripc_stream_update (struct stream *s,int sendsth) {
    struct pckt_stream_data	*h;
    char 			*d;
    int				load;
    int				change = sendsth;

    if (s->flags & STR_NOTREADY) return;
    RIPC_DEBUG("debug: stream update in progress\n");
    if ((s->data) && (s->in->head == s->in->tail)) {
	RIPC_DEBUG("debug: moving data packet to buffer.\n");
	// try to move some data from packet... but only when buffer empty
	h = (void *)s->data->data;
	load = ntohl(h->load);
	d = (char *)h + sizeof(struct pckt_stream_data);
	memcpy(s->in->buf, d, load);
	s->in->head = load;
	ripc_bucket_done(s->data);
	s->data = NULL;
	change = 1;
    }
    if (s->flags & STR_READSLEEP) {
	if (s->in->head == s->in->tail) {
	    if (s->flags & STR_PEERDEAD) {
		RIPC_DEBUG("debug: EOF in stream update\n");
		// EOF...
		cpu[s->my_vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[s->my_vcpu].iowait_id = 0;
		cpu[s->my_vcpu].uregs[0] = s->transfered;
		s->flags -= STR_READSLEEP;
	    }	
	} else {
	    int i, j, cnt;
	    cnt = s->transfered;
	    i = s->in->tail;
	    j = s->in->head;
	    while (cnt<s->count) {
		*(s->ptr++) = s->in->buf[i++];
		cnt++;
		if (i==s->in->len) i=0;
		if (i==j) { i = j = 0; break; }
	    }
	    s->transfered = cnt;
	    s->in->tail = i;
	    s->in->head = j;
	    if (cnt == s->count) {
		cpu[s->my_vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[s->my_vcpu].iowait_id = 0;
		cpu[s->my_vcpu].uregs[0] = cnt;
		s->flags -= STR_READSLEEP;
	    } else if (s->flags & STR_PEERDEAD) {
		RIPC_DEBUG("debug: EOF in stream update\n");
		// EOF...
		cpu[s->my_vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[s->my_vcpu].iowait_id = 0;
		cpu[s->my_vcpu].uregs[0] = s->transfered;
		s->flags -= STR_READSLEEP;
	    }	
	}
    } else if (s->flags & STR_WRITESLEEP) {
	int i, j, cnt;
	if (s->flags & (STR_PEERDEAD|STR_PEERCLOSING)) {
	    cpu[s->my_vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[s->my_vcpu].iowait_id = 0;
	    ripc_stream_send_data(s);
	    non_fatal(ERROR_IPC_STREAM_CLOSED, "Trying to write to closed stream",s->my_vcpu);	    	
	    return;
	} 
	cnt = s->transfered;
	i = s->out->head;
	j = s->out->tail;
	if (i==j) change = 1;
	while (cnt<s->count) {
	    if ((i+1 == j) || ((i+1 == s->out->len) && j==0)) break; // no room 
	    s->out->buf[i++] = *(s->ptr++);
	    if (i==s->out->len) i=0;
	    cnt++;
	}
	s->out->head = i;
	s->transfered = cnt;
	if (cnt == s->count) {
	    cpu[s->my_vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[s->my_vcpu].iowait_id = 0;
	    cpu[s->my_vcpu].uregs[0] = cnt;
	    s->flags -= STR_WRITESLEEP;
	}	
    } else if (s->flags & STR_PEERDEAD) {
	// our peer is dead, so we do nothing, don't try to send a thing..
	return;    
    }
    // may seem strange, but... again try to fill stuff from packets...
    if ((s->data) && (s->in->head == s->in->tail)) {
	// try to move some data from packet... but only when buffer empty
	RIPC_DEBUG("debug: moving data packet to buffer.\n");
	h = (void *)s->data->data;
	load = ntohl(h->load);
	d = (char *)h + sizeof(struct pckt_stream_data);
	memcpy(s->in->buf, d, load);
	s->in->head = load;
	ripc_bucket_done(s->data);
	s->data = NULL;
	change = 1;
    }
    if ((s->out->net_tail == s->out->tail) && (s->out->tail != s->out->head)) change = 1;
    if (change) ripc_stream_send_data(s);
}

static void
ipc_stream_req (int c) {
    unsigned int	flags, ipc_reg;
    char		buffer[200];
    signed int		vs, vcpu;
    struct ipc_request 	*r;
    struct ipc_request 	*tr;
    struct ipc_queue	*q;
    struct stream	*s;
    struct buffer	*b1, *b2;
    int			i, j;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    flags = cpu[c].uregs[0];
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Bad flags passed",c);
	return;
    }
    ipc_reg = cpu[c].uregs[1];
    vcpu = cpu[c].sregs[0];
    vs = cpu[c].sregs[1];
    if (!networking) {
	if ((vs>0) && (vs!=ipc_vs_number)) {
	    non_fatal(ERROR_IPC_NO_TARGET, "No such target (no networking)",c);
	    return;
	}
	if ((vcpu < -1) || (vcpu >= MAX_VCPUS)) {
	    non_fatal(ERROR_IPC_BAD_TARGET, "Illegal target specified", c);
	    return;
	}
    }
    if (vs != -1 && vcpu != -1 && ipc_reg!=0) {
	sprintf(buffer, "ipc/target/unicast/%d/%d/%d", vs, vcpu, ipc_reg);
    } else {
	strcpy(buffer, "ipc/target/broadcast");
    }
    VALIDATE(c, buffer, "ipc/stream/req");
    // create one side stream... so we can assure that there is one free
    for (i=0;(i<MAX_STREAMS && proces_streams[c][i]);i++);
    if (i>=MAX_STREAMS) {
	non_fatal(ERROR_IPC_NO_RESOURCES, "No resources for stream request",c);
	return;
    }
    for (j=0,s=streams;(j<GLOBAL_MAX_STREAMS && s->flags);j++,s++);
    if (j>=GLOBAL_MAX_STREAMS) {
	non_fatal(ERROR_IPC_NO_RESOURCES, "No resources for stream request",c);
	return;
    }
    if ((b1 = (struct buffer *) ipc_buffer_new()) == NULL) {
	non_fatal(ERROR_IPC_NO_RESOURCES, "No resources for stream request",c);
	return;
    }
    if ((b2 = (struct buffer *) ipc_buffer_new()) == NULL) {
	ipc_buffer_done(b1);
	non_fatal(ERROR_IPC_NO_RESOURCES, "No resources for stream request",c);
	return;
    }
    r = ipc_request_new();
    if (!r) {
	ipc_buffer_done(b1);
	ipc_buffer_done(b2);
	non_fatal(ERROR_IPC_NOMEM, "Memory allocation failure", c);
	return;
    }
    r->s.vcpu = c;
    r->s.vs = ipc_vs_number;
    r->s.ipc_reg = cpu[c].ipc_reg;
    r->d.vcpu = vcpu;
    r->d.vs = vs;
    r->d.ipc_reg = ipc_reg;
    r->type = IPC_STREAM;
    r->id = ipc_new_id();
    r->flags = flags;
    // initialize this stream..
    b2->len = b1->len = STREAM_BUFFER_LEN;
    b2->head = b1->head = 0;
    b2->tail = b1->tail = 0;
    b2->net_tail = b1->net_tail = 0;
    proces_streams[c][i] = s;
    s->req = r;
    s->peer_vcpu = 0;
    s->peer_vs = 0;
    s->peer_stream = 0;
    s->my_vcpu = c;
    s->my_stream = i;
    s->my_global_nr = j;
    s->id = r->id;
    s->flags = STR_INUSE|STR_NOTREADY;
    s->peer_count = 0;
    s->my_count = 1;
    s->in = b1;
    s->out = b2;
    s->data = NULL;
    r->str_req_str = j;
    r->str_local_src = i;
    gettimeofday(&r->timestamp, NULL);
    if (vs<1 || vs==ipc_vs_number) {
	if (!ipc_make_targets(c, r)) {
	    ipc_buffer_done(s->in);
	    ipc_buffer_done(s->out);
	    proces_streams[c][i] = NULL;
	    s->in = s->out = NULL;
	    s->flags = 0;
	    ipc_request_done(r);
	    non_fatal (ERROR_IPC_NOMEM, "Memory allocation failure",c);
	    return;
	}
    }
    if (networking && vs!=0 && vs!=ipc_vs_number) {
	int ret = ripc_send_request(r);
	if (!ret) {
	    ipc_buffer_done(s->in);
	    ipc_buffer_done(s->out);
	    proces_streams[c][i] = NULL;
	    s->in = s->out = NULL;
	    s->flags = 0;
	    ipc_request_done(r);
	    non_fatal(ERROR_IPC_NO_TARGET, "No route to target host",c);
	    return;
	} else if (ret<0) {
	    // for now we're letting this request time out, because
	    // we cannot say if we sent this request to anyone...
	    // but someone may get this and respond, or even local targets..
	}
    }
    if (!r->targets && !r->ripc_host_count) {
	ipc_buffer_done(s->in);
	ipc_buffer_done(s->out);
	proces_streams[c][i] = NULL;
	s->in = s->out = NULL;
	s->flags = 0;
	ipc_request_done(r);
	non_fatal(ERROR_IPC_NO_TARGET, "No such target", c);
	return;
    }
    // wake up local sleepers...
    for (q=r->targets;q;q=q->next_r) {
	if ((cpu[q->vcpu].state & VCPU_STATE_IPCWAIT) && (cpu[q->vcpu].iowait_id == IOWAIT_ID_STREAM_REQ)) {
	    cpu[q->vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[q->vcpu].iowait_id = 0;
	    cpu[q->vcpu].uregs[0] = r->s.vcpu;
	    cpu[q->vcpu].uregs[1] = r->s.vs;
	    cpu[q->vcpu].uregs[2] = r->s.ipc_reg;
	    cpu[q->vcpu].uregs[3] = r->id;
	}
    }    
    tr = proces_requests[c];
    if (!tr) {
    	r->next = r->prev = NULL;
    	proces_requests[c] = r;
    } else {
        while (tr->next) tr=tr->next;
        r->next = NULL;
        r->prev = tr;
        tr->next = r;
    }
    streams_count++;
    if (flags & IPC_FLAG_NONBLOCK) {
	cpu[c].uregs[0] = r->id;
	return;
    }
    cpu[c].state |= VCPU_STATE_IPCWAIT;
    cpu[c].iowait_id = IOWAIT_ID_STREAM_SENT_REQ;
    r->flags |= IPC_FLAG_SENDERSLEEP;
}    

static void
ripc_stream_req (struct ipc_request *r) {
    struct ipc_request	*tr;
    struct ipc_queue	*q;
    if (!ipc_make_targets(MAX_VCPUS+1, r)) {
	r->flags = IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NORESOURCES;
	ripc_send_host_nack(r);
        ipc_request_done(r);
        return;
    }
    if (!r->targets) {
	r->flags = IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NOTARGET;
	ripc_send_host_nack(r);
	ipc_request_done(r);
	return;
    }
    for (q=r->targets;q;q=q->next_r) {
	if ((cpu[q->vcpu].state & VCPU_STATE_IPCWAIT) && (cpu[q->vcpu].iowait_id == IOWAIT_ID_STREAM_REQ)) {
	    cpu[q->vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[q->vcpu].iowait_id = 0;
	    cpu[q->vcpu].sregs[0] = IPC_EOK;
	    cpu[q->vcpu].uregs[0] = r->s.vcpu;
	    cpu[q->vcpu].uregs[1] = r->s.vs;
	    cpu[q->vcpu].uregs[2] = r->s.ipc_reg;
	    cpu[q->vcpu].uregs[3] = r->id;
	}
    }    
    tr = network_requests;
    if (!tr) {
    	r->next = r->prev = NULL;
    	network_requests = r;
    } else {
        while (tr->next) tr=tr->next;
        r->next = NULL;
        r->prev = tr;
        tr->next = r;
    }
    ripc_send_host_ack (r);
}

static void
ipc_stream_stat (int c) {
    unsigned int	id;
    struct ipc_request 	*r;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC", c);
	return;
    }
    id = cpu[c].uregs[0];
    for (r=proces_requests[c];(r && (r->id != id));r=r->next);
    if (!r || !(r->flags & IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_NO_REQUEST, "No such request",c);
	return;
    }
    // ok so we've got an request.. copy data that we need..
    if (r->flags & IPC_FLAG_ERROR) {
	cpu[c].sregs[0] = IPC_RSTATUS_ERROR;
	cpu[c].uregs[0] = r->errcode;
	// destroy this request..
	IPC_UNLINK_REQUEST(r, c);
	ipc_request_done(r);
	return;
    } else if (r->flags & IPC_FLAG_COMPLETED) {
	cpu[c].sregs[0] = IPC_RSTATUS_COMPLETED;
	cpu[c].uregs[0] = r->f.vcpu;
	cpu[c].uregs[1] = r->f.vs;
	cpu[c].uregs[2] = r->f.ipc_reg;
	cpu[c].uregs[3] = r->str_local_src;
	IPC_UNLINK_REQUEST(r,c);
	ipc_request_done(r);
    } else if (r->flags & IPC_FLAG_ACCEPTED) {
	cpu[c].sregs[0] = IPC_RSTATUS_ACCEPTED;
    } else {
	cpu[c].sregs[0] = IPC_RSTATUS_WAITING;
    }
}

static void
ipc_stream_chck (int c) {
    char	buffer[200];
    unsigned int	flags;
    struct ipc_queue	*q;
    struct ipc_request	*r;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC", c);
	return;
    }
    flags = cpu[c].uregs[0];
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Bad flags passed", c);
	return;
    }
restart:
    for (q=proces_incoming[c];q;q=q->next) {
	if (q->req->type != IPC_STREAM) continue;
	r = q->req;
	sprintf(buffer, "ipc/source/%d/%d/%d", r->s.vs, r->s.vcpu, r->s.ipc_reg);
	if (is_permitted(c, buffer, "ipc/stream/recv")) break;
	IPC_UNLINK_QUEUE(q,r);
	ipc_queue_done(q);
	if (!r->targets && !r->ripc_host_count) {
	    r->flags |= IPC_FLAG_ERROR;
	    r->errcode = IPC_ERR_NACK;
	    if (r->s.vs != ipc_vs_number) {
		ripc_send_host_nack(r);
		IPC_UNLINK_NET_REQUEST(r);
		ipc_request_done(r);	    
	    } else {
		struct stream *s;
		gettimeofday(&r->timestamp, NULL);
		if (r->flags & IPC_FLAG_NONBLOCK) continue;
		streams_count--;
		s = proces_streams[r->s.vcpu][r->str_local_src];
		s->flags = 0;
		proces_streams[r->s.vcpu][r->str_local_src] = NULL;
		ipc_buffer_done(s->in);
		ipc_buffer_done(s->out);
		s->in = s->out = NULL;
		cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[r->s.vcpu].iowait_id = 0;
		IPC_UNLINK_REQUEST(r, r->s.vcpu);
		non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped", r->s.vcpu);
		ipc_request_done(r);
		goto restart;
	    }
	}
    };
    if (q) {
	gettimeofday(&q->req->timestamp, NULL); // touch it... so it can stay alive
	if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_EOK;
	cpu[c].uregs[0] = q->req->s.vcpu;
	cpu[c].uregs[1] = q->req->s.vs;
	cpu[c].uregs[2] = q->req->s.ipc_reg;
	cpu[c].uregs[3] = q->req->id;
	return;
    }
    if (flags & IPC_FLAG_NONBLOCK) {
	cpu[c].sregs[0] = IPC_ETRYAGAIN;
	return;
    }
    cpu[c].state |= VCPU_STATE_IPCWAIT;
    cpu[c].iowait_id = IOWAIT_ID_STREAM_REQ;
    return;
}

static void
ipc_stream_nack (int c) {
    unsigned int	id;
    struct ipc_queue	*q;
    struct ipc_request	*r;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    id = cpu[c].uregs[0];
    for (q=proces_incoming[c];(q && (q->req->id != id));q=q->next);
    if (!q) {			
	non_fatal (ERROR_IPC_NO_REQUEST, "No such stream request",c);
	return;	
    }
    r = q->req;
    gettimeofday(&r->timestamp, NULL);	// touching it
    IPC_UNLINK_QUEUE(q, r);
    ipc_queue_done(q);
    if (!r->targets && !r->ripc_host_count) { 
	r->flags |= IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NACK;
	if (r->s.vs != ipc_vs_number) {
	    ripc_send_host_nack(r);
	    IPC_UNLINK_NET_REQUEST(r);
	} else {
	    int h;
	    struct stream *s;
	    if (r->flags & IPC_FLAG_NONBLOCK) return;
	    h = r->s.vcpu;
	    streams_count--;
	    s = proces_streams[h][r->str_local_src];
	    proces_streams[h][r->str_local_src] = NULL;
	    ipc_buffer_done(s->in);
	    ipc_buffer_done(s->out);
	    s->in = s->out = NULL;
	    s->flags = 0;
	    cpu[h].state -= VCPU_STATE_IPCWAIT;
	    cpu[h].iowait_id = 0;
	    IPC_UNLINK_REQUEST(r, h);
	    non_fatal(ERROR_IPC_REQUEST_NACKED,"Request dropped",h);
	}
	ipc_request_done(r);
    }
}

static void
ipc_stream_ack (int c) {
    struct ipc_queue	*q;
    struct ipc_request	*r;
    unsigned int	id;
    unsigned int	flags;

    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    id = cpu[c].uregs[1];
    flags = cpu[c].uregs[0];
    for (q=proces_incoming[c];((q) && (q->req->id != id)); q=q->next);
    if (!q) {
	non_fatal(ERROR_IPC_NO_REQUEST, "No such request", c);
	return;
    }
    r = q->req;
    IPC_UNLINK_QUEUE(q, r);
    ipc_queue_done(q);
    gettimeofday(&r->timestamp, NULL); // touching it...
    if (r->s.vs != ipc_vs_number) {
	int i, j;
	struct stream *s;
	struct buffer *b1, *b2;
	for (i=0;(i<MAX_STREAMS && proces_streams[c][i]);i++);
	for (j=0,s=streams;(j<GLOBAL_MAX_STREAMS && s->flags);j++,s++);
	b1 = (struct buffer *) ipc_buffer_new();
	b2 = (struct buffer *) ipc_buffer_new();
	if (i<MAX_STREAMS && j<GLOBAL_MAX_STREAMS && b1 && b2) {
	    proces_streams[c][i] = s;
	    s->req = r;
	    s->peer_vcpu = r->s.vcpu;
	    s->peer_vs = r->s.vs;
	    s->peer_stream = r->str_req_str;
	    s->my_vcpu = c;
	    s->my_stream = i;
	    s->my_global_nr = j;
	    s->id = r->id;
	    s->flags = STR_INUSE|STR_NOTREADY;
	    s->peer_count = 1;
	    s->my_count = 0;
	    s->data = NULL;
	    b1->net_tail = b2->net_tail = 0;
	    b1->tail = b2->tail = 0;
	    b1->head = b2->head = 0;
	    b2->len = b1->len = STREAM_BUFFER_LEN;
	    s->in = b1;
	    s->out = b2;
	    r->f.vs = ipc_vs_number;
	    r->f.vcpu = c;
	    r->f.ipc_reg = cpu[c].ipc_reg;
	    r->str_dst_str = j;
	    r->str_local_dst = i;
	    r->flags |= IPC_FLAG_ACK_SENT;
	    r->flags |= IPC_FLAG_ACCEPTED;
	    ripc_send_host_done(r);	// don't care, timeouter will do its job
	    streams_count++;
	    ipc_queue_done_linked(r->targets);
	    r->targets = NULL;
	    if (flags & IPC_FLAG_NONBLOCK) {
		cpu[c].sregs[0] = IPC_EOK;
		cpu[c].uregs[0] = r->str_local_src;
	    } else {
		cpu[c].state |= VCPU_STATE_IPCWAIT;
		cpu[c].iowait_id = IOWAIT_ID_STREAM_ACK;
		r->flags |= IPC_FLAG_SENDERSLEEP;	    
	    }
	    return;
	} else {
	    if (b1) ipc_buffer_done(b1);
	    if (b2) ipc_buffer_done(b2);
	    if (!r->targets) {
		r->flags = IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NORESOURCES;
		ripc_send_host_nack(r);
		IPC_UNLINK_NET_REQUEST(r);
		ipc_request_done(r);
	    }	
	    non_fatal(ERROR_IPC_NO_RESOURCES, "No resources for acking stream request",c);
	    return;
	}
    } else {
	// local request...
	int i,j,h,m;
	struct stream *s, *s2;
	for (i=0;(i<MAX_STREAMS && proces_streams[c][i]);i++);
	for (j=0,s=streams;(j<GLOBAL_MAX_STREAMS && s->flags);j++,s++);
	if (i<MAX_STREAMS && j<GLOBAL_MAX_STREAMS) {
	    proces_streams[c][i] = s;
	    s->my_stream = i;
	    r->str_local_dst = i;
	    s->my_global_nr = j;
	    r->str_dst_str = j;
	    h = r->s.vcpu;
	    m = r->str_req_str;
	    s2 = &streams[m];
	    s2->peer_vcpu = c;
	    s2->peer_vs = ipc_vs_number;
	    s2->peer_stream = j;
	    s2->req = NULL;
	    s->req = NULL;
	    s->peer_vcpu = h;
	    s->peer_vs = ipc_vs_number;
	    s->peer_stream = m;
	    s->id = r->id;
	    s->in = s2->out;
	    s->out = s2->in;
	    s->flags = STR_INUSE|STR_PEERREADY;
	    s2->flags = STR_INUSE|STR_PEERREADY;
	    ipc_queue_done_linked(r->targets);
	    streams_count++;
	    if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_EOK;
	    cpu[c].uregs[0] = r->str_local_dst;
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[h].state -= VCPU_STATE_IPCWAIT;
		cpu[h].iowait_id = 0;
		cpu[h].uregs[0] = c;
		cpu[h].uregs[1] = ipc_vs_number;
		cpu[h].uregs[2] = cpu[c].ipc_reg;
		cpu[h].uregs[3] = r->str_local_src;
		IPC_UNLINK_REQUEST(r, h);
		ipc_request_done(r);
	    } else {
		r->flags |= IPC_FLAG_COMPLETED;
		r->ripc_host_count = 0;
		r->targets = NULL;
	    }
	    return;
	} else {
	     // sth has gone wrong...
	    int i,h;
	    struct stream *s;
	    if (!r->targets && !r->ripc_host_count) {
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NORESOURCES;
		i = r->str_local_src;
		h = r->s.vcpu;
		s = proces_streams[h][i];	
		if (s->in) ipc_buffer_done(s->in);
		if (s->out) ipc_buffer_done(s->out);
		proces_streams[h][i] = NULL;
		streams_count--;
		s->in = s->out = NULL;
		s->flags = 0;
		if (!(r->flags & IPC_FLAG_NONBLOCK)) {
		    cpu[h].state -= VCPU_STATE_IPCWAIT;
		    cpu[h].iowait_id = 0;
		    IPC_UNLINK_REQUEST(r, h);
		    ipc_request_done(r);
		    non_fatal(ERROR_IPC_NO_RESOURCES, "No resources when acking stream",c);
		}		
	    }
	    non_fatal(ERROR_IPC_NO_RESOURCES, "No resources for acking stream requset",c);
	    return;
	}
    }
}

static void
ripc_stream_ack_ack (struct pckt_bucket *p) {
    struct pckt_stream_ack_ack	*h;
    struct ipc_request		*r;
    struct stream		*s;
    int id, stream, vcpu, vs, s_vs, s_vcpu, s_stream;
    int he;
    
    h = (void *)p->data;
    id = ntohl(h->id);
    stream = ntohl(h->d_stream);
    vcpu = ntohl(h->d_vcpu);
    vs = ntohl(h->d_vs);
    s_vs = ntohl(h->s_vs);
    s_vcpu = ntohl(h->s_vcpu);
    s_stream = ntohl(h->s_stream);

    RIPC_DEBUG("debug: got stream ACK ACK\n");

    s = NULL;
    if (stream <GLOBAL_MAX_STREAMS) {
	s = &streams[stream];
	if (s->peer_stream!=s_stream || s->peer_vcpu!=s_vcpu 
	    || s->peer_vs!=s_vs || s->id!=id || s->my_vcpu!=vcpu || !s->flags) {
	    s = NULL;	
	}
    }
    if (!s || (s->flags & STR_PEERDEAD)) {
	RIPC_DEBUG("debug: stream ACK ACK to dead or nonexistant stream.\n");
	ripc_stream_send_reset(p);
	ripc_bucket_done(p);
	return;
    }

    ripc_bucket_done(p);
    if (!s->req) return;	// alredy got ack_ack

    r = s->req;
    if (vs != ipc_vs_number) {
	// ack_ack that's nacking
	RIPC_DEBUG("debug: bad stream ACK\n");
	ipc_queue_done_linked(r->targets);
	r->targets = NULL;
	r->flags |= IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NACK;
	if (proces_streams[vcpu][r->str_local_dst] == s) 
	    proces_streams[vcpu][r->str_local_dst] = NULL; 
	s->req = NULL;
	if (s->in) ipc_buffer_done(s->in);
	if (s->out) ipc_buffer_done(s->out);
	s->in = s->out = NULL;
	s->flags = 0;
	he = r->f.vcpu;
	if (r->flags & IPC_FLAG_SENDERSLEEP) {
	    IPC_UNLINK_NET_REQUEST(r);
	    cpu[he].state -= VCPU_STATE_IPCWAIT;
	    cpu[he].iowait_id = 0;
	    non_fatal(ERROR_IPC_NO_REQUEST, "No such request",he);
	}	
	ipc_request_done(r);
    } else {
	RIPC_DEBUG("debug: good stream ACK\n");
	// ack ack that's acking
	he = vcpu;
	s->flags -= STR_NOTREADY;	// BULBA IS FUCKIN LAME!!!
	s->flags |= STR_PEERREADY;
	gettimeofday(&s->timestamp, NULL);
	gettimeofday(&s->pong, NULL);
	s->retries = 0;
	s->req = NULL;
	IPC_UNLINK_NET_REQUEST(r);
	if (r->flags & IPC_FLAG_SENDERSLEEP) {
	    cpu[he].state -= VCPU_STATE_IPCWAIT;
	    cpu[he].iowait_id = 0;
	    cpu[he].uregs[0] = s->my_stream;
	}
	ipc_request_done(r);
	ripc_stream_send_data(s);
	return;
    }
}


static void
ripc_stream_resp (struct pckt_bucket *p) {
    struct pckt_resp	*h;
    struct ipc_request	*r;
    struct stream	*s;
    int s_vcpu, id, vcpu, vs, ipc_reg, flags, errcode, number, stream;
    h = (void *)p->data;
    s_vcpu = ntohl(h->s_vcpu);
    id = ntohl(h->id);
    vcpu = ntohl(h->f_vcpu);
    vs = ntohl(h->f_vs);
    ipc_reg = ntohl(h->f_ipc_reg);
    flags = ntohl(h->flags);
    errcode = ntohl(h->errcode);
    stream = ntohl(h->str_req_str);
    number = ntohl(h->str_dst_str);

    s = NULL;
    if (stream < GLOBAL_MAX_STREAMS) {
	s = &streams[stream];
	if (!s->flags || s->id!=id || s->my_vcpu!=s_vcpu) 
	    s = NULL;
    }
    
    if (!s) {
	if (flags & IPC_FLAG_ACCEPTED) ripc_stream_send_ack_nack(p);	
	ripc_bucket_done(p);
	return;
    }
    
    if (flags & IPC_FLAG_ACCEPTED) {
	r = s->req;
	if (r && !(r->flags & IPC_FLAG_ACCEPTED)) {
	    r->ripc_host_count = 0;
	    ipc_queue_done_linked(r->targets);
	    r->targets = NULL;
	    r->flags |= (IPC_FLAG_ACCEPTED|IPC_FLAG_COMPLETED);
	    r->f.vcpu = vcpu;
	    r->f.vs = vs;
	    r->f.ipc_reg = ipc_reg;
	    s->peer_vcpu = vcpu;
	    s->peer_vs = vs;
	    s->peer_stream = number;
	    s->req = NULL;
	    s->flags = (STR_INUSE|STR_NOTREADY);
	    if (r->d.vs <0) vs = 0;
	    ripc_stream_send_ack_ack(s);
	    gettimeofday(&r->timestamp, NULL);
	    gettimeofday(&s->pong, NULL);
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[s_vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[s_vcpu].iowait_id = 0;
		cpu[s_vcpu].uregs[0] = r->f.vcpu;
		cpu[s_vcpu].uregs[1] = r->f.vs;
		cpu[s_vcpu].uregs[2] = r->f.ipc_reg;
		cpu[s_vcpu].uregs[3] = r->str_local_src;
		IPC_UNLINK_REQUEST(r, s_vcpu);
		ipc_request_done(r);
	    }
	} else {
	    if (vcpu!=s->peer_vcpu || vs!=s->peer_vs || number!=s->peer_stream || id!=s->id) {
		ripc_stream_send_ack_nack(p);
	    } else {
		gettimeofday(&s->pong, NULL);
		ripc_stream_send_ack_ack(s);
	    }
	}
	ripc_bucket_done(p);
	return;
    } 
    r = s->req;
    ripc_bucket_done(p);
    if (!r) return;	// just ignore bad nacks...
    r->ripc_host_count--;
    if (flags & IPC_FLAG_COMPLETED) r->flags |= IPC_FLAG_HAD_TARGETS;
    if (!r->ripc_host_count && !r->targets) {    
	gettimeofday(&r->timestamp, NULL);
	r->flags |= IPC_FLAG_ERROR;
	if (r->flags & IPC_FLAG_HAD_TARGETS) r->errcode = IPC_ERR_NACK;
	else r->errcode = IPC_ERR_NOTARGET;
	// destroy stream...
	if (s->in) ipc_buffer_done(s->in);
	if (s->out) ipc_buffer_done(s->out);
	streams_count--;
	s->in = s->out = NULL;
	s->flags = 0;
	s->req = NULL;
	if (proces_streams[s_vcpu][s->my_stream] == s) 
	    proces_streams[s_vcpu][s->my_stream] = NULL;
	if (r->flags & IPC_FLAG_SENDERSLEEP) {
	    IPC_UNLINK_REQUEST(r, s_vcpu);
	    cpu[s_vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[s_vcpu].iowait_id = 0;
	    if (r->errcode == IPC_ERR_NACK) 
		non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped",s_vcpu);
	    else non_fatal(ERROR_IPC_NO_TARGET, "No such target", s_vcpu);
	    ipc_request_done(r);
	}
	return;
    }
}

static void
close_this_stream (struct stream *s, int hard) {
    if (s->peer_vs != ipc_vs_number) {
	if ((s->flags & (STR_PEERDEAD|STR_PEERCLOSING)) || hard) {
	    RIPC_DEBUG("debug: hard stream shutdown\n");
	    streams_count--;
	    s->flags = 0;
	    if (s->in) ipc_buffer_done(s->in);
	    if (s->out) ipc_buffer_done(s->out);
	    if (s->data) ripc_bucket_done(s->data);
	    s->data = NULL;
	    s->in = s->out = NULL;
	    s->ptr = NULL;
	    s->count = 0;
	    if (proces_streams[s->my_vcpu][s->my_stream] == s) {
	        proces_streams[s->my_vcpu][s->my_stream] = NULL;
	    }
	    return;
	}
	RIPC_DEBUG("debug: soft stream shutdown\n");
	gettimeofday(&s->timestamp, NULL);
	s->retries = 0;
	s->flags |= STR_CLOSING;
	if (s->out->head == s->out->tail) ripc_stream_send_data(s);
	// now wait till ack/timeout... 
    } else {
	int h;
	struct stream *d;
	if (s->flags & STR_PEERDEAD) {
	    RIPC_DEBUG("debug: hard stream shutdown\n");
	    streams_count--;
	    s->flags = 0;
	    if (s->in) ipc_buffer_done(s->in);
	    if (s->out) ipc_buffer_done(s->out);
	    if (s->data) ripc_bucket_done(s->data);
	    s->data = NULL;
	    s->in = s->out = NULL;
	    s->ptr = NULL;
	    s->count = 0;
	    if (proces_streams[s->my_vcpu][s->my_stream] == s) {
	        proces_streams[s->my_vcpu][s->my_stream] = NULL;
	    }
	    return;
	}
	RIPC_DEBUG("debug: soft stream shutdown\n");
	h = s->peer_vcpu;
	d = &streams[s->peer_stream];
	d->flags |= STR_PEERDEAD;
	s->in = s->out = NULL;
	s->ptr = NULL;
	s->count = 0;
	s->flags = 0;
	streams_count--;
	proces_streams[s->my_vcpu][s->my_stream] = NULL;
	if (d->flags & STR_WRITESLEEP) {
	    cpu[h].state -= VCPU_STATE_IPCWAIT;
	    cpu[h].iowait_id = 0;
	    d->flags -= STR_WRITESLEEP;
	    non_fatal(ERROR_IPC_STREAM_CLOSED, "Writing to closed stream",h);
    	} else if (d->flags & STR_READSLEEP) {
	    cpu[h].state -= VCPU_STATE_IPCWAIT;
	    cpu[h].iowait_id = 0;
	    d->flags -= STR_READSLEEP;
	    cpu[h].uregs[0] = d->transfered;
	}
    }
}

static void
ipc_stream_close (int c) {
    unsigned int	id;
    struct stream	*s;
    id = cpu[c].uregs[0];
    if ((id>=MAX_STREAMS) || !(proces_streams[c][id])) {
	non_fatal(ERROR_IPC_STREAM_ID_INVALID, "Invalid stream id",c);
	return;
    }
    s = proces_streams[c][id];
    close_this_stream(s,0);
}

static void
ripc_stream_reset(struct pckt_bucket *p) {
    struct pckt_stream_reset	*h;
    struct stream 		*s;
    int	vcpu, vs, stream, id;
    int s_vcpu, s_vs, s_stream;
    h = (void *)p->data;
    stream = ntohl(h->d_stream);
    vs = ntohl(h->d_vs);
    vcpu = ntohl(h->d_vcpu);
    id = ntohl(h->id);
    s_vcpu = ntohl(h->s_vcpu);
    s_vs = ntohl(h->s_vs);
    s_stream = ntohl(h->s_stream);
    
    ripc_bucket_done(p);
    if ((vs != ipc_vs_number)||(vcpu >= MAX_VCPUS)) return;
    if (stream >= GLOBAL_MAX_STREAMS) return;
    if (!(cpu[vcpu].flags & VCPU_FLAG_USED)) return;
    s = &streams[stream];
    if (!s || !s->flags || (s->flags & STR_PEERDEAD) || s->peer_vs != s_vs || s->peer_vcpu != s_vcpu || s->peer_stream != s_stream) return;

    RIPC_DEBUG("debug: in stream_reset\n");

    if (s->flags & STR_NOTREADY) s->flags -= STR_NOTREADY;
    s->flags |= STR_PEERDEAD;
    if (s->flags & STR_CLOSING) close_this_stream(s,1);
    else ripc_stream_update(s,0);
}

static void
ipc_stream_status (int c) {
    unsigned int	id;
    unsigned int	flags = 0;
    struct stream	*s;

    id = cpu[c].uregs[0];
    if (id >= MAX_STREAMS || !proces_streams[c][id]) {
	non_fatal(ERROR_IPC_STREAM_ID_INVALID, "No such stream",c);
	return;
    }
    s = proces_streams[c][id];
    
    if (s->in->tail != s->in->head) flags |= 0x1; 		// can read something
    if (s->flags & STR_PEERDEAD) flags |= 0x4;			// half_closed
	else if ((s->out->head + 1 != s->out->tail) 		// can write
		&& !((s->out->head+1==s->out->len)&&(s->out->tail==0))) 
		    flags |= 0x2; 
    if (s->flags & STR_NOTREADY) flags &= 0x4;			// not acked yet
    cpu[c].uregs[0] = flags;
    return;
}

static void
ipc_stream_write (int c) {
    unsigned int	id, src, count, flags;
    char 		*ptr;
    struct stream	*s;
    int			i, j, cnt;
    id = cpu[c].uregs[0];
    src = cpu[c].uregs[1];
    count = cpu[c].uregs[2];
    flags = cpu[c].uregs[3];
    if ((id >= MAX_STREAMS) || !proces_streams[c][id]) {
	non_fatal(ERROR_IPC_STREAM_ID_INVALID, "No such stream", c);
	return;
    }
    ptr = verify_access(c, src, (count+3)>>2, MEM_FLAG_READ);
    if (!ptr) {
	non_fatal(ERROR_OUTSIDE_MEM, "Invalid destination buffer",c);
	return;
    }
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Invalid flags passed",c);
	return;
    }
    s = proces_streams[c][id];

    if (s->flags & STR_PEERDEAD) {
	non_fatal(ERROR_IPC_STREAM_CLOSED, "Trying to write to dead stream",c);
	return;
    }

    RIPC_LOG("stream write operation\n");
    if (s->flags & STR_NOTREADY) {
	if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_ETRYAGAIN;
	else {
	    s->transfered = 0;
	    s->count = count;
	    s->ptr = ptr;
	    cpu[c].state |= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = IOWAIT_ID_STREAM_WRITE;
	    s->flags |= STR_WRITESLEEP;
	}
	return;
    }

    if (s->peer_vs != ipc_vs_number) {
	cnt = 0;
	i = s->out->head;
	j = s->out->tail;
	// if nonblock, just copy and go away..
	if (flags & IPC_FLAG_NONBLOCK) {
	    while (cnt<count) {
		if ((i+1==j) || ((i+1==s->out->len) && !j)) break;
		s->out->buf[i++] = *(ptr++);
		cnt++;
		if (i==s->out->len) i=0;
	    }
	    s->out->head = i;
	    if (!cnt) cpu[c].sregs[0] = IPC_ETRYAGAIN;
	    else {
		cpu[c].sregs[0] = IPC_EOK;
		cpu[c].uregs[0] = cnt;
	    }
	} else { 
	    // if blocking :) we're happy, we can just mark us sleeping and update..
	    s->transfered = 0;
	    s->count = count;
	    s->ptr = ptr;
	    cpu[c].state |= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = IOWAIT_ID_STREAM_WRITE;
	    s->flags |= STR_WRITESLEEP;
	}
	ripc_stream_update(s,0);
    } else {
	struct stream	*p;
	cnt = 0;
	p = &streams[s->peer_stream];		// I'LL HATE YOU BETTER!
	// nakarmimy swiniaka najpierw...
	if (p->flags & STR_READSLEEP) {
	    int left = p->transfered;
	    while ((left<p->count) && (cnt!=count)) {
		*(p->ptr++) = *(ptr++);
		cnt++;
		left++;
	    }
	    p->transfered = left;
	    if (left==p->count) {
		cpu[s->peer_vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[s->peer_vcpu].iowait_id = 0;
		cpu[s->peer_vcpu].uregs[0] = left;
		p->flags -= STR_READSLEEP;
	    
	    }
	}
	i = s->out->head;
	j = s->out->tail;
	while (cnt<count) {
	    if ((i+1==j) || ((i+1==s->out->len) && !j)) break;
	    s->out->buf[i++] = *(ptr++);
	    cnt++;
	    if (i==s->out->len) i=0;
	}
	s->out->head = i;
	s->ptr = ptr;
	s->count = count;
	s->transfered = cnt;
	if (cnt == count) {
	    if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_EOK;
	    cpu[c].uregs[0] = cnt;
	} else if (flags & IPC_FLAG_NONBLOCK) {
	    if (!cnt) cpu[c].sregs[0] = IPC_ETRYAGAIN;
	    else {
		cpu[c].sregs[0] = IPC_EOK;
		cpu[c].uregs[0] = cnt;
	    }
	} else {
	    if (p->flags & STR_WRITESLEEP) {
		// deadlock
		if (cnt) cpu[c].uregs[0] = cnt;
		else non_fatal(ERROR_IPC_STREAM_DEADLOCK, "Stream write deadlock",c);
		return;
	    }
	    // go asleep...
	    cpu[c].state |= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = IOWAIT_ID_STREAM_WRITE;
	    s->flags |= STR_WRITESLEEP;
	}
    }
}

static void
ipc_stream_read (int c) {
    unsigned int 	id, dst, count, flags;
    char *		ptr;
    struct stream	*s;
    int			i, j, cnt;
    id = cpu[c].uregs[0];
    dst = cpu[c].uregs[1];
    count = cpu[c].uregs[2];
    flags = cpu[c].uregs[3];
    if ((id >= MAX_STREAMS) || !proces_streams[c][id]) {
	non_fatal(ERROR_IPC_STREAM_ID_INVALID, "No such stream", c);
	return;
    }
    ptr = verify_access(c, dst, (count+3)>>2, MEM_FLAG_WRITE);
    if (!ptr) {
	non_fatal(ERROR_OUTSIDE_MEM, "Invalid destination buffer",c);
	return;
    }
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Invalid flags passed",c);
	return;
    }
    s = proces_streams[c][id];
    RIPC_LOG("stream read operation\n");
    // check if there is anything to read...
    cnt = 0;
    i = s->in->tail;
    j = s->in->head;
    while ((i!=j) && (cnt != count)) {
	*(ptr++) = s->in->buf[i++];
	cnt++;
	if (i==s->in->len) i=0; // wrapping buffer...
    }    
    if (i==j) i=j=0;	// make it look new...
    s->in->tail = i;
    s->in->head = j;
    RIPC_DEBUG("debug: after first read stage %d\n", cnt);
    if (s->flags & STR_PEERDEAD) {
	RIPC_DEBUG("debug: stream EOF in stream read\n");
	if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_EOK;
	cpu[c].uregs[0] = cnt;
	return;
    }    
    if (s->peer_vs == ipc_vs_number) { // it's local so we can try more magic...
        struct stream	*p;
        p = &streams[s->peer_stream];
        if (p->flags & STR_WRITESLEEP) {	// zjedzmy z koryta swiniaka
    	    int left = p->transfered;
    	    while ((cnt!=count) && (left!=p->count)) {
    		*(ptr++) = *(p->ptr++);
    		cnt++;
    	        left++;
    	    }
	    if (left==p->count) {
	        cpu[s->peer_vcpu].state -= VCPU_STATE_IPCWAIT;
	        cpu[s->peer_vcpu].iowait_id = 0;
	        cpu[s->peer_vcpu].uregs[0] = left;
	        p->flags -= STR_WRITESLEEP;
	    } else {
	        // dump another portion od data from sleeper.. 
	        while (left<p->count) {
	    	    if ((j+1==i) || ((j+1==p->out->len) && !i)) break;
		    p->out->buf[j++] = *(p->ptr++);
		    left++;
		    if (j==p->out->len) j = 0;
		}
		p->out->head = j;
	        p->out->tail = i;
		p->transfered = left;
	        if (left==p->count) {
		    cpu[s->peer_vcpu].state -= VCPU_STATE_IPCWAIT;
		    cpu[s->peer_vcpu].iowait_id = 0;
		    cpu[s->peer_vcpu].uregs[0] = left;
		    p->flags -= STR_WRITESLEEP;
		}
	    }
	}
	if (cnt == count) {
	    if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_EOK;
	    cpu[c].uregs[0] = cnt;
	    return;
	}
	if (flags & IPC_FLAG_NONBLOCK) {
	    if (!cnt) cpu[c].sregs[0] = IPC_ETRYAGAIN;
	    else {
		cpu[c].sregs[0] = IPC_EOK;
		cpu[c].uregs[0] = cnt;
	    }
	    return;
	}
	// go asleeep...
	if (p->flags & STR_READSLEEP) {
	    if (cnt) cpu[c].uregs[0] = cnt;
	    else non_fatal(ERROR_IPC_STREAM_DEADLOCK, "Stream read deadlock",c);
	    return;		
	}
	s->ptr = ptr;
        s->transfered = cnt;
        s->count = count;
	cpu[c].state |= VCPU_STATE_IPCWAIT;
        cpu[c].iowait_id = IOWAIT_ID_STREAM_READ;
        s->flags |= STR_READSLEEP;
        return;
    } else {
	if (cnt == count) {
	    if (flags & IPC_FLAG_NONBLOCK) cpu[c].sregs[0] = IPC_EOK;
	    cpu[c].uregs[0] = cnt;
	} else if (flags & IPC_FLAG_NONBLOCK) {
	    if (!cnt) cpu[c].sregs[0] = IPC_ETRYAGAIN;
	    else {
	        cpu[c].uregs[0] = cnt;
	        cpu[c].sregs[0] = IPC_EOK;
	    }
	} else {
	    s->ptr = ptr;
	    s->transfered = cnt;
	    s->count = count;
	    cpu[c].state |= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = IOWAIT_ID_STREAM_READ;
	    s->flags |= STR_READSLEEP;
	}
	RIPC_DEBUG("debug: read driven update\n");
	ripc_stream_update(s,0);
    }
}

static void
ripc_stream_recv_data (struct pckt_bucket *p) {
    struct pckt_stream_data	*h;
    struct stream		*s;
    unsigned int	id, stream, vcpu, p_vcpu, p_vs, p_stream, flags, number, ack;
    h = (void *)p->data;
    id = ntohl(h->id);
    stream = ntohl(h->d_stream);
    vcpu = ntohl(h->d_vcpu);
    p_vcpu = ntohl(h->s_vcpu);
    p_vs = ntohl(h->s_vs);
    p_stream = ntohl(h->s_stream);    
    flags = ntohl(h->flags);
    number = ntohl(h->pckt_number);
    ack = ntohl(h->ack);

    RIPC_DEBUG("debug: got stream DATA number %d ack %d\n", number, ack);
    if (stream<GLOBAL_MAX_STREAMS) {
	s = &streams[stream];
	if (s->id!=id || s->peer_vcpu!=p_vcpu || s->peer_vs!=p_vs ||
	    s->peer_stream!=p_stream || !(s->flags & STR_INUSE) || s->req) s = NULL;
    } else s = NULL;

    if (!s || (s->flags & STR_PEERDEAD)) {
	RIPC_DEBUG("debug: stream data talking to dead man\n");
	ripc_stream_send_reset(p);
	ripc_bucket_done(p);
	return;
    }
    
    if (s->flags & STR_NOTREADY) {
	// we're the request sender and this is response to our ack_ack
	s->flags -= STR_NOTREADY;
	s->flags |= STR_PEERREADY;
	s->retries = 0;
    }	

    if (flags & STR_DATA_CLOSING) RIPC_DEBUG("debug: stream DATA CLOSING packet\n");

    if (!number) {	// not data packet...
	// mark stream as alive...
	RIPC_DEBUG("debug: stream data unnumbered packet.\n");
	s->retries = 0;
	if (flags & STR_DATA_ACK) {
	    RIPC_DEBUG("debug: stream DATA acking %d\n", ack);
	    if (ack != s->my_count) {
		if (ack == 0) {
		    if (flags & STR_DATA_CLOSING) s->flags |= STR_PEERCLOSING;
		    if (flags & STR_DATA_READY) s->flags |= STR_PEERREADY;
		    // status update.. ???
		}
		RIPC_DEBUG("debug: stream unnumbered DATA ack of packet from past\n");
		ripc_bucket_done(p);
		return;
	    }
	    if (s->out->tail != s->out->net_tail) RIPC_LOG("got ACK of sent packet\n");
	    s->my_count++;
	    if (!s->my_count) s->my_count = 1;
	    if ((s->flags & STR_CLOSING) && s->out->head==s->out->tail) {
		ripc_stream_send_reset(p);
		ripc_bucket_done(p);
		close_this_stream(s,1);
		return;
	    } 
	    s->out->tail = s->out->net_tail;
	    if (s->out->tail == s->out->head) s->out->tail = s->out->head = s->out->net_tail = 0;
	    if (flags & STR_DATA_READY) s->flags |= STR_PEERREADY;
	    gettimeofday(&s->timestamp, NULL);
	    ripc_bucket_done(p);
	    ripc_stream_update(s,1); 
	    return;
	} else {
	    RIPC_DEBUG("FUCKUP: UNUSUAL!\n");
	    if (flags & (STR_DATA_CLOSING|STR_DATA_DEAD)) {
		RIPC_DEBUG("debug: closing packet\n");
		s->flags |= STR_PEERDEAD;
		if (s->flags & STR_WRITESLEEP) {
		    gettimeofday(&s->timestamp, NULL);
		    ripc_stream_update(s,1);
		}
		ripc_bucket_done(p);
		return;
	    }
	    // ping or ready... do not ack it...
	    if (flags & STR_DATA_READY) s->flags |= STR_PEERREADY;
	    ripc_bucket_done(p);
	    return;
	}
    } else if (number == s->peer_count) {
	// well we got this packet alredy...
	RIPC_DEBUG("debug: stream data old packet\n");
	gettimeofday(&s->timestamp, NULL);
	if (flags & STR_DATA_CLOSING) {
	    if (s->flags & STR_CLOSING) {
		s->flags |= STR_PEERDEAD;
		ripc_stream_send_reset(p);
		ripc_bucket_done(p);
		ripc_stream_update(s,1);
		return;		
	    }
	    s->flags |= STR_PEERCLOSING;
	}
	if (ack == s->my_count) {
	    // he may be ancient, but he's got good news..
	    if (s->out->tail != s->out->net_tail) RIPC_LOG("got ACK of sent packet\n");
	    RIPC_DEBUG("debug: but he's acking our counter\n");
	    s->my_count++;
	    if (!s->my_count) s->my_count = 1;
	    if ((s->flags & STR_CLOSING) && s->out->head==s->out->tail) {
		ripc_stream_send_reset(p);
		ripc_bucket_done(p);
		close_this_stream(s,1);
		return;
	    } 
	    s->out->tail = s->out->net_tail;
	    if (s->out->tail == s->out->head) s->out->tail = s->out->head = s->out->net_tail = 0;
	    if (flags & STR_DATA_READY) s->flags |= STR_PEERREADY;
	}
	s->retries = 0;
	ripc_bucket_done(p);
	ripc_stream_update(s,1);
	return;	
    } else if ((number == s->peer_count+1) || (number==1 && s->peer_count==-1)) {
	// new packet
	int change = 0;
	RIPC_DEBUG("debug: stram data new packet\n");
	gettimeofday(&s->timestamp, NULL);
	s->retries = 0;
	if (flags & STR_DATA_CLOSING) s->flags |= STR_PEERCLOSING;
	if (flags & STR_DATA_READY) s->flags |= STR_PEERREADY;
	if ((flags & STR_DATA_ACK) && (ack == s->my_count)) {
	    // he's acking something...
	    RIPC_DEBUG("debug: stream DATA acking %d\n", ack);
	    if (s->out->tail != s->out->net_tail) RIPC_LOG("got ACK of sent packet\n");
	    s->my_count++;
	    if ((s->out->tail == s->out->head) && (s->flags & STR_CLOSING)) {
		s->flags |= STR_PEERDEAD;
		ripc_stream_send_reset(p);
		ripc_bucket_done(p);
		close_this_stream(s, 1);
		return;		
	    }
	    if (!s->my_count) s->my_count = 1;
	    s->out->tail = s->out->net_tail;	    
	    if (s->out->tail == s->out->head) s->out->tail = s->out->head = s->out->net_tail = 0;
	    change = 1;
	}
	if (!s->data) {
	    if (flags & STR_DATA_DATA) { 
		RIPC_DEBUG("debug: linking data packet.\n");
		s->peer_count = number;
		s->data = p;
		change = 1;
	    } else {
		RIPC_DEBUG("debug: acking empty closing packet.\n");
		ripc_bucket_done(p);
		s->peer_count = number;
		change = 1;
	    }
	} else {
	    ripc_bucket_done(p);
	}
	if (change) ripc_stream_update(s,1);
    } else {
	RIPC_DEBUG("debug: stream data packet from outer space\n");
	ripc_bucket_done(p);	// back to the future?.. drop this packet..
    }
}

static void
ipc_stream_module_unload (void) {
    struct stream	*s;
    struct buffer_bucket	*p, *np;
    int i;

    for (i=0,s=streams;i<GLOBAL_MAX_STREAMS;i++,s++) {
	if (!(s->flags)) continue;
	if ((s->flags & STR_PEERDEAD) || (s->peer_vs != ipc_vs_number)) {
	    if (s->in) ipc_buffer_done(s->in);
	    if (s->out) ipc_buffer_done(s->out);
	}
	streams_count--;
    }

    if (streams) free(streams);
    if (proces_streams[0]) free(proces_streams[0]);
    p = buffers;
    while (p) {
	np = p->next;
	free(p);
	p = np;
	buffers_count--;
    }
    if (buffers_count) {
	printk("-- FUCKUP: some buffers are lost in space\n");
    }
}

static void
ipc_stream_task_cleanup (int c) {
    struct stream	*s;
    int 		i;
    
    for (i=0;i<MAX_STREAMS;i++) {
	if ((s=proces_streams[c][i])==NULL) continue;
	if (s->req) {
	    IPC_UNLINK_NET_REQUEST(s->req);
	    ipc_request_done(s->req);
	    s->req = NULL;
	}
	if (s->flags & STR_READSLEEP) s->flags -= STR_READSLEEP;
	if (s->flags & STR_WRITESLEEP) s->flags -= STR_WRITESLEEP;
	close_this_stream(s, 0);
	proces_streams[c][i] = NULL;
    }
}


// sent stream requests timeouter...
static int
ipc_stream_timeouter (int c, struct ipc_request *r, struct timeval *tv) {
    // local stream requests timeouter
    struct stream *s;
    if (r->flags & (IPC_FLAG_COMPLETED|IPC_FLAG_ERROR)) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return 1;
	IPC_UNLINK_REQUEST(r, c);
	ipc_request_done(r);
	return 1;
    }
    // waiting and not accepted yet...
    if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return 1;
    s = proces_streams[c][r->str_local_src];
    if (s) close_this_stream(s, 1);
    ipc_queue_done_linked(r->targets);
    if (r->flags & IPC_FLAG_SENDERSLEEP) {
	IPC_UNLINK_REQUEST(r, c);
	ipc_request_done(r);
	non_fatal(ERROR_IPC_REQUEST_TIMEOUTED, "Request timeout",c);
	return 0;
    } else {
	memcpy(&r->timestamp, tv, sizeof(struct timeval));
	r->flags |= IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_TIMEOUT;
	return 1;
    }
}

// recived stream requests timeouter
static int
ripc_stream_timeouter (struct ipc_request *r, struct timeval *tv) {
    struct stream *s;
    if (r->flags & (IPC_FLAG_COMPLETED|IPC_FLAG_ERROR)) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return 0;
	IPC_UNLINK_NET_REQUEST(r);
	ipc_request_done(r);
	return 0;
    }
    // not acked yet
    if (!(r->flags & IPC_FLAG_ACK_SENT)) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return 0;
	IPC_UNLINK_NET_REQUEST(r);
	r->flags |= IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_TIMEOUT;
	ipc_queue_done_linked(r->targets);
	RIPC_DEBUG("debug: in ripc_stream_timeouter hostdone.\n");
	ripc_send_host_done(r);
	ipc_request_done(r);
	return 0;
    }
    // sent ack but no response yet...
    if (tv->tv_sec - r->timestamp.tv_sec < IPC_RESEND_TIME) return 0;
    r->retries++;
    if (r->retries<IPC_MAX_RESEND) {
	memcpy(&r->timestamp, tv, sizeof(struct timeval));
	RIPC_DEBUG("debug: in ripc_stream_timeouter resend.\n");
	ripc_send_host_done(r);
    } else {
	s = proces_streams[r->f.vcpu][r->str_local_dst];
	if (s) close_this_stream(s, 0);
	IPC_UNLINK_NET_REQUEST(r);
	if (r->flags & IPC_FLAG_SENDERSLEEP) {
	    non_fatal(ERROR_IPC_DEAD, "Peer died while accepting his request",r->f.vcpu);
	    ipc_request_done(r);
	    return 1;
	}
	ipc_request_done(r);
    }
    return 0;
}

#define STREAM_PEER_TIME	10
#define STREAM_PING_TIME	2

static void
ripc_stream_ping_timeouter (struct stream *s, struct timeval *tv) {
    if (s->peer_vs == ipc_vs_number) return;
    if (s->req) {
	// not acked yet, or acked but no ack_ack...
	// ignore (r)ipc_stream_timeouter should treat this one well
	return;    
    }
    if (s->flags & STR_PEERDEAD) return;
    if (tv->tv_sec - s->pong.tv_sec >= STREAM_PEER_TIME) {
	// peer is dead ???    
	RIPC_DEBUG("debug: ripc_stream_ping_timeout PEERDIED.\n");
	s->flags |= STR_PEERDEAD;
	if (s->flags & STR_CLOSING) {
	    close_this_stream(s, 1);
	    return;
	}
	ripc_stream_update(s,1);
	return;
    }
    if (tv->tv_sec - s->timestamp.tv_sec >= STREAM_PING_TIME) {
	memcpy(&s->timestamp, tv, sizeof(struct timeval));
	ripc_stream_update(s,1);	
	return;
    }
}



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! BLOCK !!!!!!!!!!!!!!!!!!!!!!!1

static void
ipc_block_init (void) {
    struct ipc_block 	*p;
    int i;
    p = (void *)calloc (MAX_BLOCKS * MAX_VCPUS, sizeof(struct ipc_block));
    if (!p) {
	MAX_BLOCKS = 0;
	proces_blocks[0] = p;
	printk("-- WARNING: memory failure while initialising blocks, blocks disabled.\n");
    } else 
    for (i=0;i<MAX_VCPUS;i++) 
	proces_blocks[i] = &p[i*MAX_BLOCKS];
}

static void
ipc_block_is_ready (int c) {
    unsigned int	block = cpu[c].uregs[0];
    if (!(cpu[c].ipc_reg)) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    if (block>=MAX_BLOCKS || !(proces_blocks[c][block].flags & BLOCK_INUSE)) {
	non_fatal(ERROR_IPC_BLOCK_ID_INVALID, "Invalid block id",c);
	return;
    }
    cpu[c].uregs[0] = (proces_blocks[c][block].flags==BLOCK_BUSY);
}

static void
ipc_block_create (int c) {
    unsigned int	size;
    int 		i;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    for (i=0;((i<MAX_BLOCKS) && (proces_blocks[c][i].flags & BLOCK_INUSE));i++);
    VALIDATE(c, "none", "ipc/block/create");
    if (i<MAX_BLOCKS) {
	int y;
	size = cpu[c].uregs[0];
	if (size>MAX_ALLOC_MEMBLK) {
	    non_fatal(ERROR_TOOBIG, "Excessive alloc attempt",c);
	    return;
	}
	y = mem_alloc(c, size, (MEM_FLAG_READ|MEM_FLAG_WRITE|MEM_FLAG_IPC));
	if (y<0) {
	    non_fatal(ERROR_IPC_NOMEM, "Memory allocation failure", c);
	    return;
	}
	proces_blocks[c][i].flags = BLOCK_INUSE;
	proces_blocks[c][i].block = y;
	proces_blocks[c][i].len = size;
	proces_blocks[c][i].req = NULL;
	cpu[c].uregs[0] = i;
	cpu[c].uregs[1] = y * (2+MAX_ALLOC_MEMBLK);
    } else {
	non_fatal(ERROR_IPC_NO_RESOURCES, "IPC blocks limit reached",c);
	return;
    }
}


static int
ipc_make_targets_block (int c, struct ipc_request *r) {
    struct ipc_queue	*q, *t;
    int i;
    int block = r->blk_block;
    int offset = r->blk_offset;
    r->targets = NULL;
    for (i=0;i<MAX_VCPUS;i++) {
	if (i==c) continue;	// we don't want requests to ourself...
	if (!(cpu[i].flags & VCPU_FLAG_USED)) continue;
	if (!cpu[i].ipc_reg) continue;
	if ((r->d.vcpu != -1) && (i != r->d.vcpu)) continue;
	if ((r->d.ipc_reg) && (cpu[i].ipc_reg != r->d.ipc_reg)) continue;
	if (block != -1) {
	    if (!(proces_blocks[i][block].flags & BLOCK_INUSE)) continue;
	    if (offset >= proces_blocks[i][block].len) continue;
	} else {
	    int y;
	    for (y=0;((y<MAX_BLOCKS) && !(proces_blocks[i][y].flags & BLOCK_INUSE));y++);
	    if (y>=MAX_BLOCKS) continue;
	}
	// this is a potential target...
	q = ipc_queue_new();
	if (!q) {
	    if (r->targets) ipc_queue_done_linked(r->targets);
	    r->targets = NULL;
	    return 0;
	}    
	q->next_r = r->targets;
	if (q->next_r) q->next_r->prev_r = q;
	q->prev_r = NULL;
	r->targets = q;
	q->req = r;
	q->vcpu = i;
	if (proces_incoming[i]) {
	    for (t=proces_incoming[i];t->next;t=t->next);
	    t->next = q;
	    q->prev = t;
	    q->next = NULL;
	} else {
	    proces_incoming[i] = q;
	    q->next = q->prev = NULL;
	}	    
    }
    if (r->targets) r->flags |= IPC_FLAG_HAD_TARGETS;
    return 1;
}

static void
ripc_block_send_ack_nack (struct pckt_bucket *q) {
    struct pckt_bucket		*p;
    struct pckt_block_ack_ack	*h;
    struct pckt_resp		*r;
    p = ripc_bucket_new();
    if (!p) return;
    RIPC_DEBUG("Sending block ACK NACK.\n");
    p->next = NULL;
    h = (void *)p->data;
    r = (void *)q->data;
    h->h.dst = r->h.src;
    h->h.src = ipc_vs_number;
    h->h.type = PCKT_BLOCK_ACK_ACK;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->s_vcpu = r->s_vcpu;
    h->s_vs = r->s_vs;
    h->s_ipc_reg = r->s_ipc_reg;
    h->id = r->id;
    h->f_vs = htonl(ipc_vs_number);	// pretend that we've accepted our own requset..
    h->f_vcpu = 0;		// doesn't matter...
    h->f_ipc_reg = r->s_ipc_reg;	// doesn't matter...
    h->block = 0;		// doesn't matter...
    h->len = 0;			// doesn't matter...
    p->len = htonl(PCKT_BLOCK_ACK_ACK);
    ripc_route_unicast(p);
}

static int
ripc_block_send_ack_ack (struct ipc_request *r, int to) {
    struct pckt_bucket		*p;
    struct pckt_block_ack_ack	*h;
    p = ripc_bucket_new();
    if (!p) return -1;
    RIPC_DEBUG("Sending block ACK ACK.\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.type = PCKT_BLOCK_ACK_ACK;
    h->h.ttl = DEFAULT_TTL;
    h->h.src = ipc_vs_number;
    if (to!=-1) h->h.dst = to;
    else {
	h->h.dst = 0;
	h->h.id = htonl(ripc_give_id());
    }
    h->s_vcpu = htonl(r->s.vcpu);
    h->s_vs = htonl(r->s.vs);
    h->s_ipc_reg = htonl(r->s.ipc_reg);
    h->f_vcpu = htonl(r->f.vcpu);
    h->f_vs = htonl(r->f.vs);
    h->f_ipc_reg = htonl(r->f.ipc_reg);
    h->id = htonl(r->id);
    h->block = htonl(r->blk_block);
    h->len = htonl(r->blk_count);
    p->len = htonl(sizeof(struct pckt_block_ack_ack));
    p->from_interface = MAX_INTERFACES;
    if (to!=-1) return ripc_route_unicast(p);
    else return ripc_route_broadcast(p);
}

static void
ripc_block_send_reset (int to, int id, int vcpu, int vs, int block) {
    struct pckt_bucket		*p;
    struct pckt_block_reset	*h;
    p = ripc_bucket_new();
    if (!p) return;
    RIPC_DEBUG("debug: sending BLOCK RESET.\n");
    h = (void *)p->data;
    h->h.src = ipc_vs_number;
    h->h.dst = to;
    h->h.ttl = DEFAULT_TTL;
    h->h.type = PCKT_BLOCK_RESET;
    h->h.id = 0;
    h->id = htonl(id);
    h->s_vcpu = htonl(vcpu);
    h->s_vs = htonl(vs);
    h->block = htonl(block);
    p->len = htonl(sizeof(struct pckt_block_reset));
    ripc_route_unicast(p);
}

static void
ripc_block_reset (struct pckt_bucket *p) {
    struct pckt_block_reset	*h;
    struct ipc_request		*r;
    int id, vcpu, vs, block;
    RIPC_DEBUG("debug: got BLOCK RESET.\n");
    h = (void *)p->data;
    id = ntohl(h->id);
    vcpu = ntohl(h->s_vcpu);
    vs = ntohl(h->s_vs);
    block = ntohl(h->block);

    ripc_bucket_done(p);    

    if (vs != ipc_vs_number) {
	struct ipc_block	*b;	
	for (r=network_requests;r;r=r->next)
	    if (r->id==id && r->s.vcpu==vcpu && r->s.vs==vs && r->blk_block==block) break;
	if (!r) return; 
	if (r->flags & (IPC_FLAG_COMPLETED|IPC_FLAG_ERROR)) return;
	// we cannot get reset without accepting...
	b = &proces_blocks[r->f.vcpu][block];
	b->req = NULL;
	b->flags -= BLOCK_BUSY;
	IPC_UNLINK_NET_REQUEST(r);
	if (r->flags & IPC_FLAG_SENDERSLEEP) {
	    cpu[r->f.vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[r->f.vcpu].iowait_id = 0;
	}
	ipc_request_done(r);
    } else {
	struct ipc_block	*b;	
	for (r=proces_requests[vcpu];r;r=r->next)
	    if (r->id==id && r->s.vcpu==vcpu && r->s.vs==vs && r->blk_block==block) break;
	if (!r) return; 
	if (r->flags & (IPC_FLAG_COMPLETED|IPC_FLAG_ERROR)) return;
	b = &proces_blocks[vcpu][block];
	b->req = NULL;
	b->flags -= BLOCK_BUSY;
	r->flags |= IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_DEAD;
	if (r->flags & IPC_FLAG_SENDERSLEEP) {
	    IPC_UNLINK_REQUEST(r, vcpu);
	    ipc_request_done(r);
	    cpu[vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[vcpu].iowait_id = 0;
	    non_fatal(ERROR_IPC_DEAD, "Block transmision died",vcpu);
	    return;
	}    
	gettimeofday(&r->timestamp, NULL);
    }
}




static void
ripc_block_send_data_ack (struct ipc_request *r) {
    struct pckt_bucket		*p;
    struct pckt_block_data	*h;
    p = ripc_bucket_new();
    if (!p) return;
    RIPC_DEBUG("debug: sending DATA ACK.\n");
    p->next = NULL;
    h = (void *)p->data;
    h->h.type = PCKT_BLOCK_DATA;
    h->h.src = ipc_vs_number;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    if (r->type==IPC_BLOCK_WRITE) {
	h->h.dst = r->s.vs;
	h->direction = htonl(IPC_BLOCK_WRITE);
    } else {
	h->h.dst = r->f.vs;
	h->direction = htonl(IPC_BLOCK_READ);
    }
    h->id = htonl(r->id);
    h->r_vcpu = htonl(r->s.vcpu);
    h->r_vs = htonl(r->s.vs);
    h->block = htonl(r->blk_block);
    h->flags = htonl(BLK_ACK);
    h->pckt_number = htonl(r->blk_current); 
    h->load = 0;
    p->len = htonl(sizeof(struct pckt_block_data));
    ripc_route_unicast(p);
    return;
}

static void
ripc_block_send_data_nack (struct pckt_bucket *b) {
    struct pckt_bucket		*p;
    struct pckt_block_data	*h;
    struct pckt_block_data	*r;
    p = ripc_bucket_new();
    if (!p) return;
    RIPC_DEBUG("debug: sending DATA NACK.\n");
    p->next = NULL;
    r = (void *)b->data;
    h = (void *)p->data;
    h->h.type = PCKT_BLOCK_DATA;
    h->h.src = ipc_vs_number;
    h->h.ttl = DEFAULT_TTL;
    h->h.id = 0;
    h->h.dst = r->h.src;
    h->direction = r->direction;
    h->id = r->id;
    h->r_vcpu = r->r_vcpu;
    h->r_vs = r->r_vs;
    h->block = r->block;
    h->flags = htonl(BLK_NACK);
    h->pckt_number = r->pckt_number;
    h->load = 0;
    p->len = htonl(sizeof(struct pckt_block_data));
    ripc_route_unicast(p);
    return;
}

static void
ripc_block_send_data (struct ipc_request *r) {
    struct pckt_bucket		*p;
    struct pckt_block_data	*h;
    char 			*dst, *src;
    
    int he;
    int load;
    
    p = ripc_bucket_new();
    if (!p) return;
    RIPC_DEBUG("debug: sending BLOCK DATA.\n");
    h = (void *)p->data;
    dst = (char *)h + sizeof(struct pckt_block_data);
    
    h->direction = htonl(r->type);
    h->r_vs = htonl(r->s.vs);
    h->r_vcpu = htonl(r->s.vcpu);
    h->id = htonl(r->id);
    h->block = htonl(r->blk_block);
    h->h.ttl = DEFAULT_TTL;
    h->h.src = ipc_vs_number;
    h->h.type = PCKT_BLOCK_DATA;
    h->h.id = 0;

    load = r->blk_next - r->blk_current;
    if (!load) load = r->blk_count - r->blk_current;
    if (load > MAX_BLOCK_PAYLOAD) load = MAX_BLOCK_PAYLOAD;
    
    if (r->s.vs != ipc_vs_number) {
	he = r->f.vcpu;
	h->h.dst = r->s.vs;
	// are there any data to send?
	if (!load) {
	    p->len = htonl(sizeof(struct pckt_block_data));
	    h->flags = htonl(BLK_DONE);
	    // unlink request
	    proces_blocks[he][r->blk_block].flags -= BLOCK_BUSY;
	    proces_blocks[he][r->blk_block].req = NULL;
	    IPC_UNLINK_NET_REQUEST(r);
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[he].state -= VCPU_STATE_IPCWAIT;
		cpu[he].iowait_id = 0;
	    }
	    ipc_request_done(r);
	} else {
	    src = verify_access(he, r->blk_adres, load, MEM_FLAG_READ);
	    if (!src) {
		proces_blocks[he][r->blk_block].flags -= BLOCK_BUSY;
		proces_blocks[he][r->blk_block].req = NULL;
		IPC_UNLINK_NET_REQUEST(r);
	        if (r->flags & IPC_FLAG_SENDERSLEEP) {
		    cpu[he].state -= VCPU_STATE_IPCWAIT;
		    cpu[he].iowait_id = 0;
		    // (CHECKIT) should we send exception to block owner?
		}
		p->len = htonl(sizeof(struct pckt_block_data));
		h->flags = htonl(BLK_DONE);
		ipc_request_done(r);
	    } else {
		r->blk_next = r->blk_current + load;
		memcpy(dst, src, load<<2);
		p->len = htonl(sizeof(struct pckt_block_data) + (load<<2));
		h->pckt_number = htonl(r->blk_next);
		h->load = htonl(load);
	        h->flags = 0;
	    }
	}
    } else {
	he = r->s.vcpu;
	h->h.dst = r->f.vs;
	if (!load) {
	    p->len = ntohl(sizeof(struct pckt_block_data));
	    h->flags = ntohl(BLK_DONE);
	    r->flags |= IPC_FLAG_COMPLETED;
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[he].state -= VCPU_STATE_IPCWAIT;
		cpu[he].iowait_id = 0;
		cpu[he].uregs[0] = r->f.vcpu;
		cpu[he].uregs[1] = r->f.vs;
		cpu[he].uregs[2] = r->f.ipc_reg;
		cpu[he].uregs[3] = r->blk_block;
		cpu[he].uregs[4] = r->blk_count;
		IPC_UNLINK_REQUEST(r, he);
		ipc_request_done(r);
	    }
	} else {
	    src = verify_access(he, r->blk_adres, load, MEM_FLAG_READ);
	    if (!src) {
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_BADMEM;
		if (r->flags & IPC_FLAG_SENDERSLEEP) {
		    cpu[he].state -= VCPU_STATE_IPCWAIT;
		    cpu[he].iowait_id = 0;
		    IPC_UNLINK_REQUEST(r, he);
		    ipc_request_done(r);
		    non_fatal(ERROR_OUTSIDE_MEM, "Invalid memory block", he);
    		}
		p->len = htonl(sizeof(struct pckt_block_data));
		h->flags = htonl(BLK_DONE);
	    } else {
		r->blk_next = r->blk_current + load;
		memcpy(dst, src, load<<2);
		h->pckt_number = htonl(r->blk_next);
		h->load = htonl(load);
		p->len = htonl( sizeof(struct pckt_block_data) + (load<<2));
		h->flags = 0;
	    }
	}
    }
    ripc_route_unicast(p);
}

static void
ripc_block_data (struct pckt_bucket *p) {
    struct pckt_block_data	*h;
    struct ipc_request		*r;
    int flags, id, vcpu, vs, type, block, number, load;
    
    h = (void *)p->data;
    vcpu = ntohl(h->r_vcpu);
    vs = ntohl(h->r_vs);
    block = ntohl(h->block);
    number = ntohl(h->pckt_number);
    id = ntohl(h->id);
    type = ntohl(h->direction);
    flags = ntohl(h->flags);
    load = ntohl(h->load);

    // find this request...    
    if (vs != ipc_vs_number) {
	for (r=network_requests;r;r=r->next) {
	    if (r->id==id && r->type==type && r->s.vcpu==vcpu && r->s.vs==vs) break;
	}    
    } else {
	for (r=proces_requests[vcpu];r;r=r->next) {
	    if (r->id==id && r->type==type) break;
	}
    }

    if (!r) {
	RIPC_DEBUG("debug: got BAD block DATA.\n");    
	if ((flags & BLK_ACK) || load) {
	    ripc_block_send_data_nack(p);
	}
	ripc_bucket_done(p);
	return;
    }

    if (r->flags & (IPC_FLAG_COMPLETED|IPC_FLAG_ERROR)) {
	RIPC_DEBUG("debug: got too late block DATA.\n");    
	ripc_bucket_done(p);
	return;
    }

    if (flags & BLK_NACK) {
	int c;
	RIPC_DEBUG("debug: got block NACK DATA.\n");    
	ripc_bucket_done(p);
	if (vs != ipc_vs_number) {
	    struct ipc_block	*b;
	    c = r->f.vcpu;
	    b = &proces_blocks[c][block];
	    b->req = NULL;
	    b->flags -= BLOCK_BUSY;
	    IPC_UNLINK_NET_REQUEST(r);
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[c].state -= VCPU_STATE_IPCWAIT;
		cpu[c].iowait_id = 0;
	    }
	    ipc_request_done(r);
	} else {
	    c = vcpu;
	    r->flags |= IPC_FLAG_ERROR;
	    r->errcode = IPC_ERR_NOTARGET;
	    gettimeofday(&r->timestamp, NULL);
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		IPC_UNLINK_REQUEST(r, c);
		cpu[c].flags -= VCPU_STATE_IPCWAIT;
		cpu[c].iowait_id = 0;
		non_fatal(ERROR_IPC_NO_TARGET, "Request target died",c);
		ipc_request_done(r);
	    }
	}
	return;	
    }

    if (flags & BLK_DONE) {
	RIPC_DEBUG("debug: got block DATA CLOSE.\n");    
	ripc_bucket_done(p);
	if (vs != ipc_vs_number) {
	    int c;
	    struct ipc_block	*b;
	    c = r->f.vcpu;
	    b = &proces_blocks[c][block];
	    b->req = NULL;
	    if (b->flags & BLOCK_BUSY) b->flags -= BLOCK_BUSY;
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[c].state -= VCPU_STATE_IPCWAIT;
		cpu[c].iowait_id = 0;
	    }    
	    IPC_UNLINK_NET_REQUEST(r);
	    ipc_request_done(r);
	    return;	
	} else {
	    // we're the sender...
	    gettimeofday(&r->timestamp, NULL);
	    r->flags |= IPC_FLAG_COMPLETED;
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[vcpu].iowait_id = 0;
		cpu[vcpu].uregs[0] = r->f.vcpu;
		cpu[vcpu].uregs[1] = r->f.vs;
		cpu[vcpu].uregs[2] = r->f.ipc_reg;
		cpu[vcpu].uregs[3] = block;
		cpu[vcpu].uregs[4] = r->blk_count;
		IPC_UNLINK_REQUEST(r, vcpu);
		ipc_request_done(r);
	    }
	    return;
	}
    }
    if (load) {
	RIPC_DEBUG("debug: got block DATA with load.\n");    
	if (number>r->blk_current) {
	    char 	* src;
	    char	* dst;
	    int		he;
	    RIPC_DEBUG("debug: good stuff...\n");
	    load = (r->blk_current+load > r->blk_count) ? r->blk_count-r->blk_current : load;

	    src = (char *)h + sizeof(struct pckt_block_data);
	    if (vs != ipc_vs_number) {
		he = r->f.vcpu;
    	    	dst = verify_access(he, r->blk_adres + r->blk_current, load, MEM_FLAG_WRITE);
	    } else {
		he = r->s.vcpu;
		dst = verify_access(he, r->blk_adres + r->blk_current, load, MEM_FLAG_WRITE);
	    }
	    if (!dst) {
		ripc_block_send_data_nack(p);
		ripc_bucket_done(p);
		if (vs != ipc_vs_number) {
		    proces_blocks[he][block].flags -= BLOCK_BUSY;
		    IPC_UNLINK_NET_REQUEST(r);
		    if (r->flags & IPC_FLAG_SENDERSLEEP) {
			cpu[he].state -= VCPU_STATE_IPCWAIT;
			cpu[he].iowait_id = 0;
		    }
		    ipc_request_done(r);
		} else {
		    gettimeofday(&r->timestamp, NULL);
		    r->flags |= IPC_FLAG_ERROR;
		    r->errcode = IPC_ERR_BADMEM;
		    if (r->flags & IPC_FLAG_SENDERSLEEP) {
			IPC_UNLINK_REQUEST(r, he);
			cpu[he].state -= VCPU_STATE_IPCWAIT;
			cpu[he].iowait_id = 0;
			non_fatal(ERROR_OUTSIDE_MEM, "Invalid target memory",he);
			ipc_request_done(r);
		    }
		}
		return;
	    }
	    r->blk_current += load;
	    memcpy(dst, src, load<<2);
	    gettimeofday(&r->timestamp, NULL);
	    r->retries = 0;
	    if (r->blk_current == r->blk_count) r->flags |= IPC_FLAG_CLOSING;
	}
	ripc_bucket_done(p);
	ripc_block_send_data_ack(r);
	return;
    }
    
    if (flags & BLK_ACK) {
	RIPC_DEBUG("debug: got block ACK DATA.\n");    
	if (number==r->blk_next) {
	    gettimeofday(&r->timestamp, NULL);
	    RIPC_DEBUG("debug: good ACK 'n stuff\n");
	    r->retries = 0;
	    r->blk_current = r->blk_next;
	    ripc_block_send_data(r);
	}
	ripc_bucket_done(p);	
    }
}

static void
ripc_block_req (struct ipc_request *r) {
    struct ipc_request	*tr;
    struct ipc_queue	*q;
    RIPC_DEBUG("debug: got block request.\n");
    if (!ipc_make_targets_block(MAX_VCPUS, r)) {
	r->flags = IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NORESOURCES;
	ripc_send_host_nack(r);
	ipc_request_done(r);
	return;
    }
    if (!r->targets) {
	r->flags = IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_NOTARGET;
	ripc_send_host_nack(r);
	ipc_request_done(r);
	return;
    }
    for (q=r->targets;q;q=q->next_r) {
	if ((cpu[q->vcpu].state & VCPU_STATE_IPCWAIT) && cpu[q->vcpu].iowait_id==IOWAIT_ID_BLOCK_REQ) {
	    cpu[q->vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[q->vcpu].iowait_id = 0;
	    cpu[q->vcpu].sregs[1] = (r->type ==IPC_BLOCK_READ) ? 0 : 1;
	    cpu[q->vcpu].uregs[0] = r->s.vcpu;
	    cpu[q->vcpu].uregs[1] = r->s.vs;
	    cpu[q->vcpu].uregs[2] = r->s.ipc_reg;
	    cpu[q->vcpu].uregs[3] = r->id;
	    cpu[q->vcpu].uregs[4] = r->blk_block;	// block id
	    cpu[q->vcpu].uregs[5] = r->blk_offset; // offset
	    cpu[q->vcpu].uregs[6] = r->blk_count; // count
	}
    }
    tr = network_requests;
    if (!tr) {
	r->next = r->prev = NULL;
	network_requests = r;
    } else {
	while (tr->next) tr=tr->next;
	r->next = NULL;
	tr->next = r;
	r->prev = tr;
    }
    ripc_send_host_ack(r);
}


static void
ipc_block_read (int c) {
    unsigned int	dst, offset, id, ipc_reg, flags, count;
    char		buffer[200];
    signed int		vcpu, vs;
    struct ipc_request 	*r, *tr;
    struct ipc_queue	*q;
    if (!(cpu[c].ipc_reg)) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    vcpu = cpu[c].sregs[0];
    vs = cpu[c].sregs[1];
    if (!networking) {
	if (vs>0 && vs!=ipc_vs_number) {
	    non_fatal(ERROR_IPC_NO_TARGET, "No such target (networking)",c);
	    return;
	}
	if (vcpu<-1 || vcpu>MAX_VCPUS) {
	    non_fatal(ERROR_IPC_BAD_TARGET, "Illegal target specified",c);
	    return;
	}
    }
    ipc_reg = cpu[c].uregs[1];
    id = cpu[c].sregs[2];
    flags = cpu[c].uregs[0];
    dst = cpu[c].uregs[2];
    offset = cpu[c].uregs[3];
    count = cpu[c].uregs[4];
    if (vs!=-1 && vcpu!=-1 && ipc_reg!=0) {
	sprintf(buffer, "ipc/target/unicast/%d/%d/%d", vs, vcpu, ipc_reg);
    } else {
	strcpy(buffer, "ipc/target/broadcast");
    }
    VALIDATE(c, buffer, "ipc/block/read");
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Illegal flags",c);
	return;
    }
    if (!verify_access(c, dst, count, MEM_FLAG_WRITE)) {
	non_fatal(ERROR_OUTSIDE_MEM, "Outside lagal memory block",c);
	return;
    }
    r = ipc_request_new();
    if (!r) {
	non_fatal(ERROR_IPC_NOMEM, "Memory allocation error",c);
	return;
    }
    r->s.vcpu = c;
    r->s.vs = ipc_vs_number;
    r->s.ipc_reg = cpu[c].ipc_reg;
    r->d.vcpu = vcpu;
    r->d.vs = vs;
    r->d.ipc_reg = ipc_reg;
    r->type = IPC_BLOCK_READ;
    r->id = ipc_new_id();
    r->flags = flags;
    r->blk_block = id;
    r->blk_adres = dst;
    r->blk_offset = offset;
    r->blk_count = count;
    r->blk_current = 0;
    r->blk_next = 0;
    gettimeofday(&r->timestamp, NULL);
    if (vs<1 || vs==ipc_vs_number) {
	if (!ipc_make_targets_block(c,r)) {
	    ipc_request_done(r);
	    non_fatal(ERROR_IPC_NOMEM, "Memory allocation failure",c);
	    return;
	}
    }
    if ((vs==-1) || (vs && vs!=ipc_vs_number)) {
	int ret = ripc_send_request(r);
	if (!ret) {
	    ipc_request_done(r);
	    non_fatal(ERROR_IPC_NO_TARGET, "No route to target host",c);
	    return;
	} else {
	    // ignore...  :)
	}
    }    
    if (!r->targets && !r->ripc_host_count) {
	ipc_request_done(r);
	non_fatal(ERROR_IPC_NO_TARGET, "No such target",c);
	return;
    }
    for (q=r->targets;q;q=q->next_r) {
	if ((cpu[q->vcpu].state & VCPU_STATE_IPCWAIT) && (cpu[q->vcpu].iowait_id == IOWAIT_ID_BLOCK_REQ)) {
	    cpu[q->vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[q->vcpu].iowait_id = 0;
	    cpu[q->vcpu].sregs[1] = 0;	// type block_read 
	    cpu[q->vcpu].uregs[0] = r->s.vcpu;
	    cpu[q->vcpu].uregs[1] = r->s.vs;
	    cpu[q->vcpu].uregs[2] = r->s.ipc_reg;
	    cpu[q->vcpu].uregs[3] = r->id;
	    cpu[q->vcpu].uregs[4] = r->blk_block;	// block id
	    cpu[q->vcpu].uregs[5] = r->blk_offset; // offset
	    cpu[q->vcpu].uregs[6] = r->blk_count; // count
	}
    }
    tr = proces_requests[c];
    if (!tr) {
	r->next = r->prev = NULL;
	proces_requests[c] = r;
    } else {
	while (tr->next) tr=tr->next;
	r->next = NULL;
	tr->next = r;
	r->prev = tr;
    }
    if (flags & IPC_FLAG_NONBLOCK) {
	cpu[c].uregs[0] = r->id;
	return;
    }
    cpu[c].state |= VCPU_STATE_IPCWAIT;
    cpu[c].iowait_id = IOWAIT_ID_BLOCK_READ;
    r->flags |= IPC_FLAG_SENDERSLEEP;
}
        
static void
ipc_block_write (int c) {
    unsigned int	src, offset, id, ipc_reg, flags, count;
    char		buffer[200];
    signed int		vcpu, vs;
    struct ipc_request 	*r, *tr;
    struct ipc_queue	*q;
    if (!(cpu[c].ipc_reg)) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    vcpu = cpu[c].sregs[0];
    vs = cpu[c].sregs[1];
    if (!networking) {
	if (vs>0 && vs!=ipc_vs_number) {
	    non_fatal(ERROR_IPC_NO_TARGET, "No such target (networking)",c);
	    return;
	}
	if (vcpu<-1 || vcpu>MAX_VCPUS) {
	    non_fatal(ERROR_IPC_BAD_TARGET, "Illegal target specified",c);
	    return;
	}
    }
    id = cpu[c].sregs[2];
    flags = cpu[c].uregs[0];
    ipc_reg = cpu[c].uregs[1];
    src = cpu[c].uregs[2];
    offset = cpu[c].uregs[3];
    count = cpu[c].uregs[4];
    if (vs!=-1 && vcpu!=-1 && ipc_reg!=0) {
	sprintf(buffer, "ipc/target/unicast/%d/%d/%d", vs, vcpu, ipc_reg);
    } else {
	strcpy(buffer, "ipc/target/broadcast");
    }
    VALIDATE(c, buffer, "ipc/block/write");
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Illegal flags",c);
	return;
    }
    if (!verify_access(c, src, count, MEM_FLAG_READ)) {
	non_fatal(ERROR_OUTSIDE_MEM, "Outside lagal memory block",c);
	return;
    }
    r = ipc_request_new();
    if (!r) {
	non_fatal(ERROR_IPC_NOMEM, "Memory allocation error",c);
	return;
    }
    r->s.vcpu = c;
    r->s.vs = ipc_vs_number;
    r->s.ipc_reg = cpu[c].ipc_reg;
    r->d.vcpu = vcpu;
    r->d.vs = vs;
    r->d.ipc_reg = ipc_reg;
    r->type = IPC_BLOCK_WRITE;
    r->id = ipc_new_id();
    r->flags = flags;
    r->blk_block = id;
    r->blk_adres = src;
    r->blk_offset = offset;
    r->blk_count = count;
    r->blk_current = 0;
    r->blk_next = 0;
    gettimeofday(&r->timestamp, NULL);
    if (vs<1 || vs==ipc_vs_number) {
	if (!ipc_make_targets_block(c,r)) {
	    ipc_request_done(r);
	    non_fatal(ERROR_IPC_NOMEM, "Memory allocation failure",c);
	    return;
	}
    }
    if ((vs==-1) || (vs && vs!=ipc_vs_number)) {
	int ret = ripc_send_request(r);
	if (!ret) {
	    ipc_request_done(r);
	    non_fatal(ERROR_IPC_NO_TARGET, "No route to target host",c);
	    return;
	} else {
	    // ignore...  :)
	}
    }    
    if (!r->targets && !r->ripc_host_count) {
	ipc_request_done(r);
	non_fatal(ERROR_IPC_NO_TARGET, "No such target",c);
	return;
    }
    for (q=r->targets;q;q=q->next_r) {
	if ((cpu[q->vcpu].state & VCPU_STATE_IPCWAIT) && (cpu[q->vcpu].iowait_id == IOWAIT_ID_BLOCK_REQ)) {
	    cpu[q->vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[q->vcpu].iowait_id = 0;
	    cpu[q->vcpu].sregs[1] = 1;	// type block_write 
	    cpu[q->vcpu].uregs[0] = r->s.vcpu;
	    cpu[q->vcpu].uregs[1] = r->s.vs;
	    cpu[q->vcpu].uregs[2] = r->s.ipc_reg;
	    cpu[q->vcpu].uregs[3] = r->id;
	    cpu[q->vcpu].uregs[4] = r->blk_block;	// block id
	    cpu[q->vcpu].uregs[5] = r->blk_offset; // offset
	    cpu[q->vcpu].uregs[6] = r->blk_count; // count
	}
    }
    tr = proces_requests[c];
    if (!tr) {
	r->next = r->prev = NULL;
	proces_requests[c] = r;
    } else {
	while (tr->next) tr=tr->next;
	r->next = NULL;
	tr->next = r;
	r->prev = tr;
    }
    if (flags & IPC_FLAG_NONBLOCK) {
	cpu[c].uregs[0] = r->id;
	return;
    }
    cpu[c].state |= VCPU_STATE_IPCWAIT;
    cpu[c].iowait_id = IOWAIT_ID_BLOCK_WRITE;
    r->flags |= IPC_FLAG_SENDERSLEEP;
}
        
static void
ipc_block_chck (int c) {
    char 		buffer[100];
    unsigned int	flags;
    struct ipc_request	*r;
    struct ipc_queue	*q;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    flags = cpu[c].uregs[0];
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Bad flags passed",c);
	return;
    }
restart:
    for (q= proces_incoming[c];q;q=q->next) {
	if ((q->req->type != IPC_BLOCK_WRITE) && (q->req->type != IPC_BLOCK_READ)) continue;
	r = q->req;
	sprintf(buffer, "ipc/source/%d/%d/%d", r->s.vs, r->s.vcpu, r->s.ipc_reg);
	if (is_permitted(c, buffer, "ipc/block/recv")) break;
	IPC_UNLINK_QUEUE(q,r)
	ipc_queue_done(q);
        if (!r->targets && !r->ripc_host_count) {
    	    if (r->s.vs!=ipc_vs_number) {
	        r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NACK;
		ripc_send_host_done(r);
		IPC_UNLINK_NET_REQUEST(r);
		ipc_request_done(r);
	    } else {
	        r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NACK;
		if (r->flags & IPC_FLAG_NONBLOCK) {
		    gettimeofday(&r->timestamp, NULL);
		    continue;
		}
		cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[r->s.vcpu].iowait_id = 0;
		IPC_UNLINK_REQUEST(r, r->s.vcpu);
		non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped", r->s.vcpu);
		ipc_request_done(r);
		goto restart;
    	    }
	}	
    }
    if (q) {
	gettimeofday(&q->req->timestamp, NULL); // touch it... so it can stay alive
	if (flags & IPC_FLAG_NONBLOCK)	cpu[c].sregs[0] = IPC_EOK;
	cpu[c].sregs[1] = (q->req->type == IPC_BLOCK_READ) ? 0 : 1;
	cpu[c].uregs[0] = q->req->s.vcpu;
	cpu[c].uregs[1] = q->req->s.vs;
	cpu[c].uregs[2] = q->req->s.ipc_reg;
	cpu[c].uregs[3] = q->req->id;
	cpu[c].uregs[4] = q->req->blk_block; // block id
	cpu[c].uregs[5] = q->req->blk_offset; // offset
	cpu[c].uregs[6] = q->req->blk_count; // count
	return;
    } 
    if (flags & IPC_FLAG_NONBLOCK) {
	cpu[c].sregs[0] = IPC_ETRYAGAIN;
	return;
    }
    cpu[c].state |= VCPU_STATE_IPCWAIT;
    cpu[c].iowait_id = IOWAIT_ID_BLOCK_REQ;
    return;
}

static void
ipc_block_nack (int c) {
    unsigned int	id;
    struct ipc_queue	*q;
    struct ipc_request	*r;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    id = cpu[c].uregs[0];
    for (q=proces_incoming[c];(q && (q->req->id != id));q=q->next);
    if (!q) {
	non_fatal(ERROR_IPC_NO_REQUEST, "No such block request", c);
	return;
    }
    r = q->req;
    gettimeofday(&r->timestamp, NULL);
    IPC_UNLINK_QUEUE(q, r);
    ipc_queue_done(q);
    if (!r->targets && !r->ripc_host_count) {
	if (r->s.vs!=ipc_vs_number) {
	    r->flags |= IPC_FLAG_ERROR;
	    r->errcode = IPC_ERR_NACK;
	    ripc_send_host_done(r);
	    IPC_UNLINK_NET_REQUEST(r);
	} else {
	    r->flags |= IPC_FLAG_ERROR;
	    r->errcode = IPC_ERR_NACK;
	    if (r->flags & IPC_FLAG_NONBLOCK) return;
	    cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
	    cpu[r->s.vcpu].iowait_id = 0;
	    IPC_UNLINK_REQUEST(r, r->s.vcpu);
	    non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped", r->s.vcpu);
	}
	ipc_request_done(r);
    }
}    

static void
ipc_block_stat (int c) {
    unsigned int	id;
    struct ipc_request	*r;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    id = cpu[c].uregs[0];
    for (r=proces_requests[c];r;r=r->next) {
	if (r->id==id && (r->flags & IPC_FLAG_NONBLOCK) && (r->type==IPC_BLOCK_WRITE || r->type== IPC_BLOCK_READ)) break;
    }
    if (!r) {
	non_fatal(ERROR_IPC_NO_REQUEST, "No such request",c);
	return;
    }
    if (r->flags & IPC_FLAG_ERROR) {
	cpu[c].sregs[0] = IPC_RSTATUS_ERROR;
	cpu[c].uregs[0] = r->errcode;
	IPC_UNLINK_REQUEST(r,c); 
	ipc_request_done(r);
	return;
    }
    if (r->flags & IPC_FLAG_COMPLETED) { 
	cpu[c].sregs[0] = IPC_RSTATUS_COMPLETED;
	cpu[c].uregs[0] = r->f.vcpu;
	cpu[c].uregs[1] = r->f.vs;
	cpu[c].uregs[2] = r->f.ipc_reg;
	cpu[c].uregs[3] = r->blk_block; // ... block id
	cpu[c].uregs[4] = r->blk_count; // amount of data read/written
	IPC_UNLINK_REQUEST(r,c); 
	ipc_request_done(r);
    } else if (r->flags & IPC_FLAG_ACCEPTED) {
	cpu[c].sregs[0] = IPC_RSTATUS_ACCEPTED;
	cpu[c].uregs[0] = r->f.vcpu;
	cpu[c].uregs[1] = r->f.vs;
	cpu[c].uregs[2] = r->f.ipc_reg;
	cpu[c].uregs[3] = r->blk_block; // ... block id
    } else {
	cpu[c].sregs[0] = IPC_RSTATUS_WAITING;
    }
}

static void
ripc_block_ack_ack (struct pckt_bucket *p) {
    struct pckt_block_ack_ack	*h;
    struct ipc_request		*r;
    int s_vcpu, s_vs, id, vcpu, vs, ipc_reg, block;
    h = (void *)p->data;
    s_vcpu = ntohl(h->s_vcpu);
    s_vs = ntohl(h->s_vs);
    id = ntohl(h->id);
    vcpu = ntohl(h->f_vcpu);
    vs = ntohl(h->f_vs);
    ipc_reg = ntohl(h->f_ipc_reg);
    block = ntohl(h->block);
    RIPC_DEBUG("debug: got BLOCK ACK ACK.\n");

    if (vs != ipc_vs_number) {
        for (r=network_requests;r;r=r->next)
		if (r->id==id && r->s.vcpu==s_vcpu && r->s.vs==s_vs && (r->type==IPC_BLOCK_WRITE || r->type==IPC_BLOCK_READ)) break;
        if (r) {
	    if (r->flags & IPC_FLAG_ACK_SENT) {
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NOTARGET;	
		if (r->flags & IPC_FLAG_SENDERSLEEP) {
		    vcpu = r->f.vcpu;
		    cpu[vcpu].state -= VCPU_STATE_IPCWAIT;
		    cpu[vcpu].iowait_id = 0;
		    IPC_UNLINK_NET_REQUEST(r);
		    ipc_request_done(r);
		    non_fatal(ERROR_IPC_NO_TARGET, "Too late",vcpu);
		}
	    } else {
		ipc_queue_done_linked(r->targets);
		r->targets = NULL;
		IPC_UNLINK_NET_REQUEST(r);
		ipc_request_done(r);
	    }
	} 
	return;
    }
    for (r=network_requests;r;r=r->next)
    	if (r->id==id && r->s.vcpu==s_vcpu && r->s.vs==s_vs && (r->type==IPC_BLOCK_WRITE || r->type==IPC_BLOCK_READ)) break;
    if (!r) {
	ripc_block_send_reset(s_vs, id, s_vcpu, s_vs, block);
	return;
    }    
    gettimeofday(&r->timestamp, NULL);	// to make timeouter work OK
    r->retries = 0;
    r->flags -= IPC_FLAG_ACK_SENT;
    r->flags |= IPC_FLAG_ACCEPTED;
    if (r->type == IPC_BLOCK_READ) {
	RIPC_DEBUG("debug: in ack ack, sending data.\n");
	ripc_block_send_data(r);
    } else {
	RIPC_DEBUG("debug: in ack ack, sending ack.\n");
	ripc_block_send_data_ack(r);
    }
}

static void
ripc_block_resp (struct pckt_bucket *p) {
    struct pckt_resp	*h;
    struct ipc_request	*r;
    int	c, id, vcpu, vs, ipc_reg, flags, errcode, block, len;
    h = (void *)p->data;
    c = ntohl(h->s_vcpu);
    id = ntohl(h->id);
    vcpu = ntohl(h->f_vcpu);
    vs = ntohl(h->f_vs);
    ipc_reg = ntohl(h->f_ipc_reg);
    flags = ntohl(h->flags);
    errcode = ntohl(h->errcode);
    block = ntohl(h->blk_block);
    len = ntohl(h->blk_count);
    
    RIPC_DEBUG("debug: got BLOCK RESP.\n");
    if (!(cpu[c].flags & VCPU_FLAG_USED)) {
	ripc_bucket_done(p);
	return;
    }
    for (r=proces_requests[c];r;r=r->next) 
	if (r->id==id && (r->type==IPC_BLOCK_WRITE || r->type==IPC_BLOCK_READ)) break;
    if (!r) {
	if (flags & IPC_FLAG_ACCEPTED) {
	    ripc_block_send_ack_nack(p);
	}
	ripc_bucket_done(p);
	return;
    }
    ripc_bucket_done(p);
    if (r->flags & IPC_FLAG_ACCEPTED) {
	if (flags & IPC_FLAG_ACCEPTED) {
	    // resend ack_ack
	    ripc_block_send_ack_ack(r, vs);
	}
	return;
    }
    gettimeofday(&r->timestamp, NULL);
    if (flags & IPC_FLAG_ACCEPTED) {
	r->ripc_host_count = 0;
	if (r->targets) ipc_queue_done_linked(r->targets);
	r->targets = NULL;
	r->flags |= IPC_FLAG_ACCEPTED;
	r->f.vcpu = vcpu;
	r->f.vs = vs;
	r->f.ipc_reg = ipc_reg;
	r->blk_block = block;
	r->blk_count = len;
	ripc_block_send_ack_ack(r, r->d.vs);
	return;
    } else {
	r->ripc_host_count--;
	if (flags & IPC_FLAG_COMPLETED) r->flags |= IPC_FLAG_HAD_TARGETS;
	if (!r->targets && !r->ripc_host_count) {
	    r->flags |= IPC_FLAG_ERROR;
	    if (r->flags & IPC_FLAG_HAD_TARGETS) r->errcode = IPC_ERR_NACK;
	    else r->errcode = IPC_ERR_NOTARGET;
	    if (r->flags & IPC_FLAG_NONBLOCK) return;
	    cpu[c].state -= VCPU_STATE_IPCWAIT;
	    cpu[c].iowait_id = 0;
	    IPC_UNLINK_REQUEST(r, c);
	    if (r->errcode == IPC_ERR_NOTARGET) non_fatal(ERROR_IPC_NO_TARGET, "No such target",c);
	    else non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped",c );
	    ipc_request_done(r);
	}
    }
}

static void
ipc_block_ack (int c) {
    unsigned int	id, block, flags;
    struct ipc_request	*r;
    struct ipc_queue	*q;
    struct ipc_block	*b;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    flags = cpu[c].uregs[0];
    id = cpu[c].uregs[1];
    block = cpu[c].uregs[2];
    if (flags & ~(IPC_FLAG_NONBLOCK)) {
	non_fatal(ERROR_IPC_BAD_FLAGS, "Bad flags passed",c );
	return;	
    }
    for (q=proces_incoming[c];(q && (q->req->id != id));q=q->next);
    if (!q) {
	non_fatal(ERROR_IPC_NO_REQUEST, "No such request",c);
	return;
    }
    r = q->req;
    IPC_UNLINK_QUEUE(q, r);
    ipc_queue_done(q);
    gettimeofday(&r->timestamp, NULL); // touching it so it can stay alive...

    if (r->blk_block != -1) {
	block = r->blk_block;
    } else {
	int ok = 1;
//	char buffer[200];
//	sprintf(buffer, "ipc/block/source/%d/%d/%d/%d", r->s.vs, r->s.vcpu, r->s.ipc_reg, block);
//	ok = is_permitted(c, buffer, (r->type==IPC_BLOCK_READ ? "ipc/block/read" : "ipc/block/write"));
	if (block > MAX_BLOCKS || !(proces_blocks[c][block].flags & BLOCK_INUSE) || !ok) {
	    if (!ok) non_fatal(ERROR_NOPERM, "HAC didn't allow you to ACK request with this block ID",c );
	    non_fatal(ERROR_IPC_BLOCK_ID_INVALID, "Passed invalid block ID",c );
	    if (!r->targets && !r->ripc_host_count) {
		if (r->s.vs!=ipc_vs_number) {
		    r->flags |= IPC_FLAG_ERROR;
		    r->errcode = IPC_ERR_NACK;
		    ripc_send_host_done(r);
		    IPC_UNLINK_NET_REQUEST(r);
		} else {
		    r->flags |= IPC_FLAG_ERROR;
		    r->errcode = IPC_ERR_NACK;
		    if (r->flags & IPC_FLAG_NONBLOCK) return;
	    	    cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
		    cpu[r->s.vcpu].iowait_id = 0;
		    IPC_UNLINK_REQUEST(r, r->s.vcpu);
		    non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped", r->s.vcpu);
		}
	        ipc_request_done(r);
		return;
	    }
	}
    }
    b = &proces_blocks[c][block];
    if (b->flags & BLOCK_BUSY) {
	// hmm... detach.. we're busy...
	gettimeofday(&r->timestamp, NULL);
	IPC_UNLINK_QUEUE(q, r);
	ipc_queue_done(q);
	if (!r->targets && !r->ripc_host_count) {
	    if (r->s.vs!=ipc_vs_number) {
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NACK;
		ripc_send_host_done(r);
		IPC_UNLINK_NET_REQUEST(r);
	    } else {
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_NACK;
		if (r->flags & IPC_FLAG_NONBLOCK) return;
		cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[r->s.vcpu].iowait_id = 0;
		IPC_UNLINK_REQUEST(r, r->s.vcpu);
	        non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped", r->s.vcpu);
	    }
	ipc_request_done(r);
	}
	return;
    }
    r->f.vcpu = c;
    r->f.vs = ipc_vs_number;
    r->f.ipc_reg = cpu[c].ipc_reg;
    r->blk_block = block;
    ipc_queue_done_linked(r->targets); // we don't need this anymore
    r->targets = NULL;


    if (r->s.vs != ipc_vs_number) {
	int len;
	r->flags |= (IPC_FLAG_ACK_SENT|IPC_FLAG_ACCEPTED);
	b->flags |= BLOCK_BUSY;
	b->req = r;
	len = b->len - r->blk_offset;
	r->blk_adres = b->block * (2 + MAX_ALLOC_MEMBLK) + r->blk_offset;
	r->blk_current = 0;
	r->blk_next = 0;
	r->blk_count = (len<r->blk_count) ? len : r->blk_count;
	r->retries = 0;
	ripc_send_host_done(r);		// we ignore errors, retransmit will do it's job
	if (flags & IPC_FLAG_NONBLOCK) return;
	cpu[c].state |= VCPU_STATE_IPCWAIT;
	cpu[c].iowait_id = IOWAIT_ID_BLOCK_REQ;
	r->flags |= IPC_FLAG_SENDERSLEEP;
	return;	
    } else {
	int he;
	char *src, *dst;
	int count, offset;
	unsigned int	adres;

	RIPC_DEBUG("debug: sending local data.\n");
	he = r->s.vcpu;
	b = &proces_blocks[c][block];
	adres = b->block * (2 + MAX_ALLOC_MEMBLK);
	offset = r->blk_offset;
	count = b->len;
	count -= offset;
	count = (count <= r->blk_count) ? count : r->blk_count;
	adres += offset;
	if (r->type == IPC_BLOCK_READ) {
	    src = verify_access(c, adres, count, MEM_FLAG_READ);
	    dst = verify_access(he, r->blk_adres, count, MEM_FLAG_WRITE);
	    if (!dst) { 
		RIPC_DEBUG("error: dst.\n");
		// can happen only in nonblocking requests..
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_BADMEM;
		return;
	    }
	} else {
	    dst = verify_access(c, adres, count, MEM_FLAG_WRITE);
	    src = verify_access(he, r->blk_adres, count, MEM_FLAG_READ);
	    if (!src) {
		RIPC_DEBUG("error: src.\n");
		r->flags |= IPC_FLAG_ERROR;
		r->errcode = IPC_ERR_BADMEM;
		return;
	    }
	}
	if (src && dst) memcpy (dst, src, (count<<2));
	else count = 0;
	r->blk_count = count;
	r->flags |= IPC_FLAG_COMPLETED;
	if (r->flags & IPC_FLAG_NONBLOCK) return;
	cpu[he].state -= VCPU_STATE_IPCWAIT;
	cpu[he].iowait_id = 0;
	cpu[he].uregs[0] = r->f.vcpu;
	cpu[he].uregs[1] = r->f.vs;
	cpu[he].uregs[2] = r->f.ipc_reg;
	cpu[he].uregs[3] = block; // block id...
	cpu[he].uregs[4] = r->blk_count; // data amount 
	IPC_UNLINK_REQUEST(r, he);
	ipc_request_done(r);
    }
}

static void
ipc_block_destroy (int c) {
    unsigned int	i;
    struct ipc_block	*b;
    struct ipc_request	*r;
    struct ipc_queue	*q;
    int			other=0, y;
    if (!cpu[c].ipc_reg) {
	non_fatal(ERROR_IPC_NOT_REGISTERED, "Not registered for IPC",c);
	return;
    }
    i = cpu[c].uregs[0];
    if (i>=MAX_BLOCKS && !(proces_blocks[c][i].flags & BLOCK_INUSE)) {
	non_fatal(ERROR_IPC_BLOCK_ID_INVALID, "Invalid block id",c);
	return;
    }
    b = &proces_blocks[c][i];
    b->flags = 0;
    // dealloc mem block

    for (y=0;y<MAX_BLOCKS;y++) {
	if (proces_blocks[c][y].flags & BLOCK_INUSE) { other++; break; }
    }
    if ((r=b->req)!=NULL) {
	ripc_block_send_reset(r->s.vs, r->id, r->s.vcpu, r->s.vs, r->blk_block);
	IPC_UNLINK_REQUEST(r, c);
	ipc_request_done(r);
	b->req = NULL;
    }
    q = proces_incoming[c];
    while (q) {
	struct ipc_queue *nq = q->next;
	r = q->req;
	if (r->type!=IPC_BLOCK_WRITE || r->type!=IPC_BLOCK_READ) {
	    q = nq;
	    continue;
	}
	if (r->blk_block==i || (r->blk_block==-1 && !other)) {
	    gettimeofday(&r->timestamp, NULL);
	    IPC_UNLINK_QUEUE(q, r);
	    ipc_queue_done(q);
	    if (!r->targets && !r->ripc_host_count) {
		r->flags |= IPC_FLAG_ERROR;
		if (r->flags & IPC_FLAG_HAD_TARGETS) r->errcode = IPC_ERR_NACK;
		else r->errcode = IPC_ERR_NOTARGET;
		if (r->s.vs != ipc_vs_number) {
		    ripc_send_host_done(r);
		    IPC_UNLINK_NET_REQUEST(r);
		    ipc_request_done(r);		
		} else {
		    r->flags |= IPC_FLAG_ERROR;
		    r->errcode = IPC_ERR_NACK;
		    if (r->flags & IPC_FLAG_SENDERSLEEP) {
			cpu[r->s.vcpu].state -= VCPU_STATE_IPCWAIT;
			cpu[r->s.vcpu].iowait_id = 0;
			IPC_UNLINK_REQUEST(r, r->s.vcpu);
			non_fatal(ERROR_IPC_REQUEST_NACKED, "Request dropped", r->s.vcpu);
			ipc_request_done(r);
			q = proces_incoming[c];
			continue;
		    }
		
		}
	    }
	}
    }
    y = b->block;
    b->block = 0;
    if (y) mem_dealloc(c, y);
}

static void
ripc_block_timeouter (struct ipc_request *r, struct timeval *tv) {
    if (r->flags & (IPC_FLAG_COMPLETED|IPC_FLAG_ERROR|IPC_FLAG_CLOSING)) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return;
	IPC_UNLINK_NET_REQUEST(r);
	ipc_request_done(r);
	return;
    }
    if (r->flags & IPC_FLAG_ACK_SENT) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_RESEND_TIME) return;
	r->retries++;
	if (r->retries<IPC_MAX_RESEND) {
	    memcpy(&r->timestamp, tv, sizeof(struct timeval));
	    ripc_send_host_done(r);
	} else {
	    struct ipc_block *b = &proces_blocks[r->f.vcpu][r->blk_block];
	    b->flags -= BLOCK_BUSY;
	    b->req = NULL;
	    IPC_UNLINK_NET_REQUEST(r);
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[r->f.vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[r->f.vcpu].iowait_id = 0;
	    }
	    // should we send NACK ??? 
	    ipc_request_done(r);
	}
	return;
    }
    if (r->flags & IPC_FLAG_ACCEPTED) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_RESEND_TIME) return;
	r->retries++;
	if (r->retries>=IPC_MAX_RESEND) {
	    struct ipc_block *b = &proces_blocks[r->f.vcpu][r->blk_block];
	    b->flags -= BLOCK_BUSY;
	    b->req = NULL;
	    IPC_UNLINK_NET_REQUEST(r);
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[r->f.vcpu].state -= VCPU_STATE_IPCWAIT;
		cpu[r->f.vcpu].iowait_id = 0;
	    }
	    ipc_request_done(r);
	    return;
	}
	memcpy(&r->timestamp, tv, sizeof(struct timeval));
	if (r->type==IPC_BLOCK_WRITE) {
	    if (!r->blk_current) ripc_block_send_data_ack(r);
	} else {
	    ripc_block_send_data(r);
	}
	return;	
    }
    if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return;
    // woah, waiting request...
    ipc_queue_done_linked(r->targets);
    IPC_UNLINK_NET_REQUEST(r);
    r->flags |= IPC_FLAG_ERROR;
    r->errcode = IPC_ERR_TIMEOUT;
    ripc_send_host_done(r);
    ipc_request_done(r);
}


static int
ipc_block_timeouter (int c, struct ipc_request *r, struct timeval *tv) {
    if (r->flags & (IPC_FLAG_COMPLETED|IPC_FLAG_ERROR)) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return 1;
	IPC_UNLINK_NET_REQUEST(r);
	ipc_request_done(r);
	return 1;
    }
    if (r->flags & IPC_FLAG_CLOSING) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_RESEND_TIME) return 1;
	memcpy(&r->timestamp, tv, sizeof(struct timeval));
	r->retries++;
	if (r->retries>=IPC_MAX_RESEND) {
	    r->flags |= IPC_FLAG_COMPLETED;
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		IPC_UNLINK_REQUEST(r, c);
		cpu[c].state -= VCPU_STATE_IPCWAIT;
		cpu[c].iowait_id = 0;
		cpu[c].uregs[0] = r->f.vcpu;
		cpu[c].uregs[1] = r->f.vs;
		cpu[c].uregs[2] = r->f.ipc_reg;
		cpu[c].uregs[3] = r->blk_block;
		cpu[c].uregs[4] = r->blk_count;
		ipc_request_done(r);	    
	    }	
	}
	return 1;
    }
    if (r->flags & IPC_FLAG_ACCEPTED) {
	if (tv->tv_sec - r->timestamp.tv_sec < IPC_RESEND_TIME) return 1;
	r->retries++;
	memcpy(&r->timestamp, tv, sizeof(struct timeval));
	if (r->retries>=IPC_MAX_RESEND) {
	    r->flags |= IPC_FLAG_ERROR;
	    r->errcode = IPC_ERR_DEAD;
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		IPC_UNLINK_REQUEST(r, c);
		cpu[c].state -= VCPU_STATE_IPCWAIT;
		cpu[c].iowait_id = 0;
		non_fatal(ERROR_IPC_DEAD, "Peer died",c);
		ipc_request_done(r);
		return 0;
	    }
	    return 1;    
	} 
	if (r->type==IPC_BLOCK_WRITE) {
	    ripc_block_send_data(r);
	}
	return 1;
    }

    if (tv->tv_sec - r->timestamp.tv_sec < IPC_REQUEST_TTL) return 1;
    ipc_queue_done_linked(r->targets);
    r->flags |= IPC_FLAG_ERROR;
    r->errcode = IPC_ERR_TIMEOUT;
    if (r->flags & IPC_FLAG_SENDERSLEEP) {
        IPC_UNLINK_REQUEST(r,c);
	cpu[c].state -= VCPU_STATE_IPCWAIT;
	cpu[c].iowait_id = 0;
	non_fatal(ERROR_IPC_REQUEST_TIMEOUTED, "Timeout",c);
	ipc_request_done(r);
	return 0;
    }
    return 1;
}

static void
ipc_block_task_cleanup (int c) {
    struct ipc_block	*b;
    int i;
    
    b = proces_blocks[c];
    for (i=0;i<MAX_BLOCKS;i++,b++) {
	if (!b->flags) continue;
	if (b->flags & BLOCK_BUSY) {
	    IPC_UNLINK_NET_REQUEST(b->req);
	    ipc_request_done(b->req);
	    b->req = NULL;
	}
	b->flags = 0;
	if (b->block) mem_dealloc(i, b->block);
    }
}

static void
ipc_block_module_unload (void) {
    struct ipc_block	*b;
    int i, y;
    for (i=0;i<MAX_VCPUS;i++) {
	if (!(cpu[i].flags & VCPU_FLAG_USED)) continue;
	b = proces_blocks[i];
	for (y=0;y<MAX_BLOCKS;y++,b++) {
	    if (!(b->flags & BLOCK_INUSE)) continue;
	    if (b->block) mem_dealloc(i, b->block);
	    b->flags = 0;
	}	
    }
    if (proces_blocks[0]) free(proces_blocks[0]);
}



static void ripc_network_handler (void);

void fioasync_handler(int x) {
  signal(SIGIO,SIG_IGN); // Slowaris is stupid!
  RIPC_LOG("debug: SIGIO, data pending, calling handler.\n");
  network_force=1;
  listen_counter=ACCEPT_INTERVAL+1;
  reconnect_counter=RECONNECT_INTERVAL+1;
  ripc_network_handler();
  RIPC_LOG("debug: out of SIGIO handler.\n");
//  usleep(1);	// to allow task switch
  signal(SIGIO,fioasync_handler);
}

#define semafor_return  { network_handler_semafor--; return; }


static void ripc_network_handler (void) {
    fd_set		in;
    struct timeval	tv = { 0, 0};
    int 		max = -1;
    int 		i, type;
    struct interface	*p;

    network_handler_semafor++;
    if (network_handler_semafor>1) {
	network_handler_semafor--;
	network_force++;
	RIPC_DEBUG("debug: alredy in network_handler\n");
	return;
    }

    if (!network_force && (++network_counter<NETWORK_INTERVAL) ) semafor_return;

    if (ripc_listen_socket != -1) {
	int fd;
	if (++listen_counter >= ACCEPT_INTERVAL) {
	    if ((fd = accept(ripc_listen_socket, NULL, NULL)) != -1) {
		RIPC_DEBUG("New connection accepted...\n");
		if (ripc_make_nonblock(fd) != -1) ripc_interface_new(fd);
		else close(fd);
	    }
	    listen_counter = 0;
	}
    }

    if (++reconnect_counter >= RECONNECT_INTERVAL) {
	config_try_to_reconnect();
	reconnect_counter = 0;
    }

    network_force = network_counter = 0;

    if (!networking) semafor_return;
    FD_ZERO(&in);
    for (i=0, p=interfaces;i<MAX_INTERFACES;i++, p++) {
	if (!p->flags) continue;
	FD_SET(p->fd, &in);
	max = (max > p->fd) ? max : p->fd;
    }
    // try to read all we can..
    i = select(max+1, &in, NULL, NULL, &tv);
    if (i<0) {
	// should we do RIPc system shutdown (CHEKIT)
	RIPC_DEBUG("Hmm.. select error...\n");
	semafor_return;
    } else if (i)
	for (i=0, p=interfaces;i<MAX_INTERFACES;i++, p++) {
	    if (!p->flags) continue;
	    if (!FD_ISSET(p->fd, &in)) continue;
	    while (ripc_interface_read (p, i));
	}    

    ripc_lost_packet_handler();

    while (postprocess_queue) {
	unsigned int	id;
	struct pckt_bucket *p = postprocess_queue;
	postprocess_queue = p->next;
	p->next = NULL;
	if (!(p->pckt_ttl--)) {
	    RIPC_DEBUG("Net event: Packet ttl expired.\n");
	    ripc_bucket_done (p);	
	    continue;
	}
	if (!p->pckt_dst) {
	    int hgw;
	    hgw = broadcast_table[p->pckt_src];
	    id = htonl(pckt_id(p));
	    if (hgw > id) {
		ripc_bucket_done(p);
		continue;
	    }
	    broadcast_table[p->pckt_src] = id;
	}
	if (!p->pckt_dst || p->pckt_dst==ipc_vs_number) {
	    int clear = 1;
	    struct ipc_request *r;
	    // remember to free this packet if we are the destination
	    switch (p->pckt_type) {
	    
	    case PCKT_REQUEST:
		r = ripc_pckt_to_request (p);
		if (!r) {
		    RIPC_DEBUG("Network request memory allocation error.\n");
		    ripc_send_pckt_nack(p);
		} else if (r->type == IPC_MSG) ripc_msg_send(r);
		else if (r->type == IPC_STREAM) ripc_stream_req(r);
		else if (r->type==IPC_BLOCK_WRITE || r->type==IPC_BLOCK_READ) ripc_block_req(r);
		else {
		    RIPC_DEBUG("Invalid network request type.\n");
		    ipc_request_done(r);
		}
		if (p->pckt_dst) { ripc_bucket_done(p); clear=0; }
		break;
	    case PCKT_HOST_ACK:
		RIPC_DEBUG("debug: host_ack.\n");
		ripc_host_ack(p);
		ripc_bucket_done(p);
		clear=0;
		break;
	    case PCKT_RESP:
		type = ntohl(*((int*)&p->pckt_load));
		RIPC_DEBUG("debug: pckt_req_ack.\n");
		if (type == IPC_MSG) {
		    ripc_msg_recv(p);
		} else if (type == IPC_STREAM) {
		    ripc_stream_resp(p);
		} else if (type==IPC_BLOCK_READ || type==IPC_BLOCK_WRITE) {
		    ripc_block_resp(p);
		} else {
		    RIPC_DEBUG("Invalid req_ack type.\n");
		    ripc_bucket_done(p);
		}
		clear = 0;
		break;
	    case PCKT_STREAM_ACK_ACK:
		ripc_stream_ack_ack(p);
		RIPC_DEBUG("debug: after ripc_straem_ack_ack\n");
		clear = 0;
		break;
	    case PCKT_STREAM_RESET:
		ripc_stream_reset(p);
		RIPC_DEBUG("debug: got stream reset\n");
		clear = 0;
		break;
	    case PCKT_BLOCK_ACK_ACK:
		ripc_block_ack_ack(p);
		if (p->pckt_dst == ipc_vs_number) {
		    ripc_bucket_done(p);
		    clear = 0;
		}
		break;
	    case PCKT_BLOCK_DATA:
		ripc_block_data(p);
		clear = 0;
		break;
	    case PCKT_BLOCK_RESET:
		ripc_block_reset(p);
		clear = 0;
		break;
	    case PCKT_STREAM_DATA:
		ripc_stream_recv_data(p);
		clear = 0;
		break;
	    default:
		RIPC_DEBUG("Packet type not supported.\n");
		ripc_bucket_done(p);
		clear = 0;
		break;
	    }
	    if (!clear) continue;

	}
	if (!p->pckt_dst) {
	    ripc_route_broadcast(p);
	} else if (p->pckt_dst != ipc_vs_number) {
	    if (!ripc_route_unicast(p)) {
		RIPC_DEBUG("No route to destination.\n");
		ripc_bucket_done(p);
	    }
	} 
    }
    postprocess_last = NULL;

    // try to write all we can...
    for (i=0, p=interfaces;i<MAX_INTERFACES;i++, p++) {
	if (!p->flags) continue;
	while (ripc_interface_write(p,i));
    }    
    ripc_lost_packet_handler();
    network_handler_semafor--;
}

static void
ripc_shutdown (void) {
    struct pckt_bucket	*p, *np;
    p = bucket_recycling;
    bucket_recycling = NULL;
    while (p) {
	np = p->next;
	free(p);
	p = np;
	bucket_count--;
    }
    if (bucket_count) {
	printk("-- FUCKUP: some buckets are lost somewhere in space\n");
    }
    // need to destroy all buckets... 
    task_ripc_handler = NULL;

    if (interfaces) free(interfaces);
    interfaces = NULL;
    printk(">> rIPC system shutdown complete.\n");
}

static void
ripc_init (void) {
    printk(">> Initializing rIPC system.\n");
    ripc_read_config();
    if (networking || ripc_listen_socket!=-1 || config_connects[0].to) {
	task_ripc_handler = ripc_network_handler;
	printk(">> rIPC system initialization completed.\n");
	network_force++;
	signal(SIGIO,fioasync_handler);
    } else {
	printk(">> rIPC system not needed.\n");
	ripc_shutdown ();
    }
}

static void
ripc_task_cleanup (int c) {
    // do nothing...
    return;
}

static void
ripc_module_unload (void) {
    if (networking || ripc_listen_socket!=-1 || config_connects[0].to) ripc_shutdown();
    return;
}

void
ipc_timeouter (void) {
    struct ipc_request	*r;
    struct ipc_request 	*nr;
    struct stream	*s;
    int 		i;
    struct timeval	tv;
    gettimeofday (&tv, NULL);

    // first requests from network..
    r = network_requests;
    while (r) {
	nr = r->next;
	if ((r->type==IPC_BLOCK_READ) || (r->type==IPC_BLOCK_WRITE)) {
	    ripc_block_timeouter(r, &tv);
	    r = nr;
	    continue;
	}
	if (r->type==IPC_STREAM) {
	    if (ripc_stream_timeouter(r, &tv)) nr = network_requests;
	    r = nr;
	    continue;
	}
        if (r->timestamp.tv_sec + IPC_REQUEST_TTL > tv.tv_sec) {
    	    r = nr;
	    continue;
	}
	r->flags |= IPC_FLAG_ERROR;
	r->errcode = IPC_ERR_TIMEOUT;
	ipc_queue_done_linked(r->targets);
	IPC_UNLINK_NET_REQUEST(r);
	RIPC_DEBUG("debug: in ipc_timeouter hostdone.\n");
	ripc_send_host_done(r);
	ipc_request_done(r);
	r = nr;
    }
    for (i=0;i<MAX_VCPUS;i++) {
	if (!(cpu[i].flags & VCPU_FLAG_USED)) continue;
	r = proces_requests[i];
	while (r) {
	    nr = r->next;
	    if ((r->type==IPC_BLOCK_READ) || (r->type==IPC_BLOCK_WRITE)) {
		if (ipc_block_timeouter(i, r, &tv)) r = nr;
		else r = proces_requests[i];
		continue;
	    }
	    if (r->type==IPC_STREAM) {
		if (ipc_stream_timeouter(i, r, &tv)) r = nr;
		else r = proces_requests[i];
		continue;	
	    }
	    if (r->timestamp.tv_sec + IPC_REQUEST_TTL > tv.tv_sec) {
		r = nr;
		continue;
	    }
	    if (r->flags & (IPC_FLAG_ERROR|IPC_FLAG_COMPLETED)) {
		IPC_UNLINK_REQUEST(r, i);
		ipc_request_done(r);
		r = nr;
		continue;
	    }
	    if (r->flags & IPC_FLAG_SENDERSLEEP) {
		cpu[i].state -= VCPU_STATE_IPCWAIT;
		cpu[i].iowait_id = 0;
		// destroy request..
		ipc_queue_done_linked(r->targets);
		IPC_UNLINK_REQUEST(r, i);
		ipc_request_done(r);
		non_fatal(ERROR_IPC_REQUEST_TIMEOUTED, "Request timed out",i);
		r = proces_requests[i];
		continue;
	    } else {
		if ((r->flags & IPC_FLAG_NONBLOCK)) {
		    gettimeofday(&r->timestamp, NULL);
		    r->flags |= IPC_FLAG_ERROR;
		    r->errcode = IPC_ERR_TIMEOUT;
		} else {
		    ipc_queue_done_linked(r->targets);
		    IPC_UNLINK_REQUEST(r, i);
		    ipc_request_done(r);
		}
	    }
	    r = nr;
	}
    }
    for (i=0,s=streams;i<GLOBAL_MAX_STREAMS;i++,s++) {
	if (s->flags) ripc_stream_ping_timeouter (s, &tv);
    }
}

void
syscall_load (int *x) {
    memset(proces_requests, 0, sizeof(proces_requests));
    memset(proces_incoming, 0, sizeof(proces_incoming));
    printk(">> starting IPC module.\n");
    ripc_init();
    ipc_stream_init();
    ipc_block_init();
    ipc_msg_init();
    ipc_queue_init();
    ipc_request_init();
    task_ipc_timeouter = ipc_timeouter;
    *(x++) = SYSCALL_IPC_REGISTER;
    *(x++) = SYSCALL_IPC_MSG_SEND;
    *(x++) = SYSCALL_IPC_MSG_RECV;
    *(x++) = SYSCALL_IPC_MSG_STAT;
    *(x++) = SYSCALL_IPC_STREAM_REQ;
    *(x++) = SYSCALL_IPC_STREAM_QUEUE;
    *(x++) = SYSCALL_IPC_STREAM_NACK;
    *(x++) = SYSCALL_IPC_STREAM_ACK;
    *(x++) = SYSCALL_IPC_STREAM_WRITE;
    *(x++) = SYSCALL_IPC_STREAM_READ;
    *(x++) = SYSCALL_IPC_STREAM_CLOSE;
    *(x++) = SYSCALL_IPC_STREAM_STAT;
    *(x++) = SYSCALL_IPC_STREAM_INFO;
    *(x++) = SYSCALL_IPC_BLOCK_CREATE;
    *(x++) = SYSCALL_IPC_BLOCK_DESTROY;
    *(x++) = SYSCALL_IPC_BLOCK_READ;
    *(x++) = SYSCALL_IPC_BLOCK_WRITE;
    *(x++) = SYSCALL_IPC_BLOCK_QUEUE;
    *(x++) = SYSCALL_IPC_BLOCK_ACK;
    *(x++) = SYSCALL_IPC_BLOCK_NACK;
    *(x++) = SYSCALL_IPC_BLOCK_STAT;
    *(x++) = SYSCALL_IPC_BLOCK_IS_READY;
    *(x) = SYSCALL_ENDLIST;
    printk(">> IPC module loaded... <<\n");
}

void
syscall_handler (int c,int num) {
    switch (num) {
	case SYSCALL_IPC_REGISTER: ipc_register(c); break;
	case SYSCALL_IPC_MSG_STAT: ipc_msg_stat(c); break;    
	case SYSCALL_IPC_MSG_RECV: ipc_msg_recv(c); break;    
	case SYSCALL_IPC_MSG_SEND: ipc_msg_send(c); break;    
	case SYSCALL_IPC_STREAM_REQ: 	ipc_stream_req(c); break;
	case SYSCALL_IPC_STREAM_STAT:	ipc_stream_stat(c); break;
	case SYSCALL_IPC_STREAM_QUEUE: 	ipc_stream_chck(c); break;
	case SYSCALL_IPC_STREAM_NACK: 	ipc_stream_nack(c); break;
	case SYSCALL_IPC_STREAM_ACK: 	ipc_stream_ack(c); break;
	case SYSCALL_IPC_STREAM_WRITE: 	ipc_stream_write(c); break;
	case SYSCALL_IPC_STREAM_READ: 	ipc_stream_read(c); break;
	case SYSCALL_IPC_STREAM_CLOSE: 	ipc_stream_close(c); break;
	case SYSCALL_IPC_STREAM_INFO:	ipc_stream_status(c); break;
	case SYSCALL_IPC_BLOCK_CREATE:	ipc_block_create(c); break;
	case SYSCALL_IPC_BLOCK_DESTROY:	ipc_block_destroy(c); break;
	case SYSCALL_IPC_BLOCK_WRITE:	ipc_block_write(c); break;
	case SYSCALL_IPC_BLOCK_READ:	ipc_block_read(c); break;
	case SYSCALL_IPC_BLOCK_QUEUE:	ipc_block_chck(c); break;
	case SYSCALL_IPC_BLOCK_ACK:	ipc_block_ack(c); break;
	case SYSCALL_IPC_BLOCK_NACK:	ipc_block_nack(c); break;
	case SYSCALL_IPC_BLOCK_STAT:	ipc_block_stat(c); break;
	case SYSCALL_IPC_BLOCK_IS_READY: ipc_block_is_ready(c); break;
    }
}

void
syscall_unload (void) {
    int i;
    task_ipc_timeouter = NULL;
    signal(SIGIO,SIG_IGN);
    ipc_msg_module_unload();
    ipc_stream_module_unload();
    ipc_block_module_unload();
    for (i=0;i<MAX_VCPUS;i++) {
	if ((cpu[i].flags & VCPU_FLAG_USED) && (cpu[i].state & VCPU_STATE_IPCWAIT)) {
	    cpu[i].state -= VCPU_STATE_IPCWAIT;
	    cpu[i].iowait_id = 0;
	    non_fatal(ERROR_NOMODULE, "IPC module unloading", i);
	}
    }
    ipc_queue_destroy();
    ipc_request_destroy();
    ripc_module_unload ();
}

void
syscall_task_cleanup (int c) {
    struct ipc_request	*r = proces_requests[c];
    struct ipc_queue	*q;
    proces_requests[c] = NULL;
    ipc_msg_task_cleanup(c);
    ipc_stream_task_cleanup(c);
    ipc_block_task_cleanup(c);
    while (r) {
	struct ipc_request *nr = r->next;
	if (r->targets) ipc_queue_done_linked(r->targets);
	r->next = NULL;
	ipc_request_done(r);
	r = nr;
    }
    q = proces_incoming[c];
    while (q) {
	struct ipc_queue *qn = q->next;
	r = q->req;
	IPC_UNLINK_QUEUE(q, q->req);
	ipc_queue_done(q);
	if (!r->targets) {
	    if (!(r->flags & IPC_FLAG_ACCEPTED)) {
		r->flags |= IPC_FLAG_ERROR;	    
		r->errcode = IPC_ERR_NACK;
	    } else {
		r->flags |= IPC_FLAG_COMPLETED;
	    }
	    if (r->s.vs != ipc_vs_number) {
		IPC_UNLINK_NET_REQUEST(r);
		if (r->flags & IPC_FLAG_ACCEPTED) ripc_send_host_done(r);
		else ripc_send_host_nack(r);
		ipc_request_done(r);
	    } else {
		if (r->flags & IPC_FLAG_NONBLOCK) {
			gettimeofday(&r->timestamp, NULL);
		} else {
		    int he = r->s.vcpu;
		    IPC_UNLINK_REQUEST(r, he);
		    if (r->flags & IPC_FLAG_SENDERSLEEP) {
			cpu[he].state -= VCPU_STATE_IPCWAIT;
			cpu[he].iowait_id = 0;
		    	non_fatal(ERROR_IPC_REQUEST_TIMEOUTED, "Request timeout",he);
			ipc_request_done(r);
			qn = proces_incoming[c]; // rerun queue...
		    }			
		    ipc_request_done(r);
		}
	    }
	}
	q = qn;
    }
    ripc_task_cleanup(c);
}


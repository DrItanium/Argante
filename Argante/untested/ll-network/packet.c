/*

   Argante virtual OS
   ------------------

   Low-level socket interface - send / receive, firewalling

   Status: untested

   Author:     bikappa <bikappa@itapac.net>
   Maintainer: bikappa <bikappa@itapac.net>

*/

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <limits.h>
#include <sys/un.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <net/bpf.h>

#include "config.h"
#include "module.h"
#include "console.h"
#include "syscall.h"
#include "task.h"
#include "bcode.h"
#include "exception.h"
#include "acman.h"

#ifndef MSG_NOSIGNAL
// IRIX64!
#define MSG_NOSIGNAL 0
#endif

#ifndef MSG_DONTWAIT
// In-case...
#define MSG_DONTWAIT 0
#endif


#ifndef AF_LOCAL
// Solaris
#define AF_LOCAL AF_UNIX
#endif

#ifndef PF_LOCAL
// Solaris
#define PF_LOCAL AF_UNIX
#endif

#ifndef SOCK_PACKET
// Solaris...
#define SOCK_PACKET SOCK_SEQPACKET
#endif


#define LOW_NET_SOCK_FLAG_NONE                  0x00
#define RAW_SENDER				0x1
#define RAW_LISTENER				0x2
#define RAW_BPF			                0x3
#define RAW_LIVE                                0x4
#define RAW_BPF_LISTENER                        0x5


#define NET_LOW_MODULE_VERSION "0.33b"
#define VBUF_SIZE 	512


#define CHECKSD(d) if (d >= MAX_LOW_SD || !lowdata[c].sd[d]) { \
    non_fatal (ERROR_NET_BAD_SD, "lownetwork: bad socket descriptor", c); \
    failure=1; \
    return; \
  }

#define CHECKSDNOERR(d) if (d >= MAX_LOW_SD || !lowdata[c].sd[d]) { \
    return; \
  }


#define INTERROR(e,str) { \
    non_fatal(ERROR_NETERROR, "lownetwork->" str ": internal error",c); \
    failure=1; \
    return e; \
  }

#define PROTERROR(e,str) { \
    non_fatal(ERROR_PROTFAULT, "lownetwork->" str ": attempt to access protected \
                                memory",c); \
    failure=1; \
    return e; \
  }

#define CUSTEXC(e,str,err,msg) { \
    non_fatal(err, "lownetwork->" str ": " msg ,c); \
    failure=1; \
    return e; \
  }


/* shit */
#define SYSCALL_LOW_NET_INITDEV                 701
#define SYSCALL_LOW_NET_RAW                     702
#define SYSCALL_LOW_NET_BPF                     703
#define SYSCALL_LOW_NET_SEND                    704
#define SYSCALL_LOW_NET_RECV                    705
#define SYSCALL_LOW_NET_CLOSE                   706
#define SYSCALL_LOW_NET_LIVE                    707

#define MAX_LOW_SD MAX_NET_SD
/* eof shit */

struct process_low_entry
{
  int sd[MAX_LOW_SD];
  int flags[MAX_LOW_SD];
  char *pkt;
  int plen;
};


struct process_low_entry lowdata[MAX_VCPUS];


void syscall_load (int *x) {
  *(x++) = SYSCALL_LOW_NET_RECV;
  *(x++) = SYSCALL_LOW_NET_SEND;
  *(x++) = SYSCALL_LOW_NET_INITDEV;
  *(x++) = SYSCALL_LOW_NET_RAW;
  *(x++) = SYSCALL_LOW_NET_BPF;
  *(x++) = SYSCALL_LOW_NET_LIVE;
  *(x++) = SYSCALL_LOW_NET_CLOSE;
  *(x)   = SYSCALL_ENDLIST;
  printk (">> l0wlevel-net module v%s loaded.\n", NET_LOW_MODULE_VERSION);
  printk ("+> Memory usage: %d fd/VCPU (%d bytes)\n", MAX_LOW_SD, sizeof (lowdata));
}



int make_sock_nonblock (int c, int d) {
  long fl;
  if ((fl = fcntl (d, F_GETFL)) == -1)  return -1;
  fl |= O_NONBLOCK;
  if (fcntl (d, F_SETFL, fl) == -1) return -1;
  return 0;
}


// looks for free descriptors in lowdata[].sd[]
int look_for_sd (int c) {
  int n;
  for (n = 0; n < MAX_LOW_SD; n++)
    if (!lowdata[c].sd[n]) break;

  if (n == MAX_LOW_SD)  return -1;

  return (n);
}


// slurp
int
wait_lowrecv (int c)
{
  int k, fd, len;
  char *pkt;

  len = cpu[c].uregs[2];
  fd = cpu[c].iowait_id;

  if (!
      (pkt =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) / 4,
		      MEM_FLAG_WRITE))) PROTERROR (1, "recv");

  k = recv (fd, pkt, len, MSG_DONTWAIT | MSG_NOSIGNAL);

  switch (k)
  {
    case -1:
                if (errno == EAGAIN)
                        return 0;
                else
                if (errno == EPIPE)
	                CUSTEXC (1
                                 , "recv"
                                 , ERROR_NET_EPIPE
                                 , "broken pipe")
	        else
	                INTERROR (1, "recv");
    case 0:
                lowdata[c].flags[cpu[c].uregs[0]] = LOW_NET_SOCK_FLAG_NONE;
                CUSTEXC (1
                         , "trans"
                         , ERROR_NET_EOF
                         , "remote party disconnected");
    default:
                cpu[c].sregs[0] = k;
                return 1; // wake up
    }
}


// slurp 2
int
wait_lowsend (int c)
{
  int k, fd, len;
  char *pkt;

  len = cpu[c].uregs[2];
  fd  = cpu[c].iowait_id;

  if (!
      (pkt =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) / 4,
		      MEM_FLAG_READ))) PROTERROR (1, "send");

  k = send (fd, pkt, len, MSG_NOSIGNAL | MSG_DONTWAIT);

  switch (k)
  {
        case -1:
                if (errno == EAGAIN)
	                return 0;
                else
                if (errno == EPIPE)
	                CUSTEXC (1
                                 , "send"
                                 , ERROR_NET_EPIPE
                                 , "broken pipe")
	        else
	                INTERROR (1, "send");
        default:
                cpu[c].sregs[0] = k;
                return 1; // wake up
    }
}

void exiton (void) {
  struct ifreq saved_ifr;
  static int c, fd;
  if((fd = socket(PF_INET, SOCK_PACKET, htons(0x0003))) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "pf inet")

  else if (ioctl(fd, SIOCSIFFLAGS, &saved_ifr) < 0)
                                CUSTEXC (;
                                         , "socket"
                                         , ERROR_NET_SOCK
                                         , "SIOCSIFFLAGS");}


// syscall SYSCALL_LOW_NET_INITDEV

void open_gap (int c) {
  struct ifreq ifr;
  struct sockaddr sa;
  unsigned char *device;
  static int fd, n;
  char buf[100],b2[200];

  device=verify_access(c,cpu[c].uregs[0], (cpu[c].uregs[1] + 3) / 4,
         MEM_FLAG_READ);

  if (!device) PROTERROR (;, "initraw");
  if (cpu[c].uregs[1]>=sizeof(buf)) PROTERROR (;, "baddev");
  if (cpu[c].uregs[1]>=sizeof(ifr.ifr_name)) PROTERROR (;, "baddev2");
  bzero(buf,sizeof(buf));
  memcpy(buf,device,cpu[c].uregs[1]);
  sprintf(b2,"net/dev/phys/%s",buf);

  VALIDATE(c,b2,"net/raw/open/listener");

  if ((fd = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ALL))) == -1)
        CUSTEXC (;
                , "connect",
                ERROR_NET_SOCK,
                "unable to create new socket");

  memset(&ifr, 0, sizeof(ifr));
  memcpy(ifr.ifr_name, device, cpu[c].uregs[1]);
  if(ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
        CUSTEXC (;
                , "socket",
                ERROR_NET_SOCK,
                "SIOCGIFHWADDR");

  memset(&sa, 0, sizeof(sa));
  strncpy(sa.sa_data, device, sizeof(sa.sa_data));

  if ((n = look_for_sd (c)) == -1)
        CUSTEXC (;, "connect", ERROR_NET_NO_FREE_SD, "no free s-descriptors");

  lowdata[c].sd[n] = fd;
  lowdata[c].flags[n] = RAW_LISTENER;
  cpu[c].sregs[0] = n;
  make_sock_nonblock(c,fd);
}


// syscall SYSCALL_LOW_NET_BPF
// u2 - read only/read write 1/0

void open_bpf (int c) {
  struct ifreq ifr;
  struct bpf_version bv;
  static int fd = -1, n, ro;
  char device[sizeof "/dev/bpf000"];

  VALIDATE(c,"none","net/raw/open/bpf");

  ro = cpu[c].uregs[1]; // Read only

  do {
  (void)sprintf(device, "/dev/bpf%d", n++);
  if (ro) fd = open(device, O_RDONLY);
  else fd = open(device, O_RDWR);
  } while (fd < 0 && errno == EBUSY);

  if (fd == -1)
        CUSTEXC (;
                 , "open bpf"
                 ,  ERROR_NET_SOCK
                 ,  "error connecting socket");

  if (ioctl(fd, BIOCVERSION, (caddr_t)&bv) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "BIOCVERSION");

  if (bv.bv_major != BPF_MAJOR_VERSION || bv.bv_minor < BPF_MINOR_VERSION)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "kernel bpf filter out of date");

  (void)strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
  if (ioctl(fd, BIOCSETIF, (caddr_t)&ifr) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "BIOCSETIF");

  if ((n = look_for_sd (c)) == -1)
        CUSTEXC (;, "connect", ERROR_NET_NO_FREE_SD, "no free s-descriptors");

  lowdata[c].sd[n] = fd;
  if (ro)
        lowdata[c].flags[n] = RAW_BPF_LISTENER;
  else
        lowdata[c].flags[n] = RAW_BPF;
  cpu[c].sregs[0] = n;
  make_sock_nonblock(c,fd);
}


// syscall SYSCALL_LOW_NET_RAW

void open_raw (int c) {
  static int fd, n, one = 1;
  void *onptr = &one; //lamez

  VALIDATE(c,"none","net/raw/open/sender");

  if ((fd = socket(AF_INET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
        CUSTEXC (;
                , "connect",
                ERROR_NET_SOCK,
                "unable to create new raw socket");

  if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, onptr, sizeof(onptr)) == -1)
        CUSTEXC (;
                , "connect",
                ERROR_NET_SOCK,
                "unable to set socket option");

  if ((n = look_for_sd (c)) == -1)
        CUSTEXC (;, "connect", ERROR_NET_NO_FREE_SD, "no free s-descriptors");

  lowdata[c].sd[n] = fd;
  lowdata[c].flags[n] = RAW_SENDER;
  cpu[c].sregs[0] = n;
  make_sock_nonblock(c,fd);
}


// syscall SYSCALL_LOW_NET_SEND
// I:   u0 - descriptor
//      u1 - address
//      u2 - packet length
//      u4 - blocking/non-blocking (1/0)
// O:   s0 - bytes sent
//      s1 - ret code (1 == OK, 0 == wouldblock)


void
low_send (int c)
{
  static int n, len, k, sock, block, rawb = 0, lbp = 0;
  static char *pkt;

  n = cpu[c].uregs[0];          /* virtual descriptor   */

  CHECKSD (n);

  sock = lowdata[c].sd[n];     /* real descriptor      */
  len = cpu[c].uregs[2];
  block = cpu[c].uregs[4];

  if (!
      (pkt =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3)/4,
                                                MEM_FLAG_READ)))
       PROTERROR (;, "lowsend");

  switch(lowdata[c].flags[n])
  {
        case RAW_BPF            :
        case RAW_SENDER         :       rawb = 1; break;
        case RAW_LISTENER       :       lbp  = 1; break;
        case RAW_BPF_LISTENER   :
                CUSTEXC (;
                         , "lowsend"
                         , ERROR_NET_BAD_SD
                         , "bpf socket is read-only");
                break;
        default:
                CUSTEXC (;
                         , "lowsend"
                         , ERROR_NET_BAD_SD
                         , "not a bpf RAW socket");
                break;}
  while ( (k>=0) && (k<len) ) {
  if (lbp)
    k = send(   sock
                , pkt
                , len
                , MSG_DONTWAIT | MSG_NOSIGNAL);
  if (rawb)
    k = write(    sock
                , pkt
                , len);


    if ( (k>=0) && (k<len) ) {
      pkt += k;
      len -= k;
      cpu[c].sregs[2] = len;
    }
  }

  switch (k) {
        case -1:
                if (errno == EAGAIN) {
                  if (!block) {
                        cpu[c].sregs[1] = 0;
                        return;
                        }
                  ENTER_IOWAIT (c, sock, wait_lowsend);

                  cpu[c].sregs[1] = 0;
                  printk("packet.so: unable to send whole data in one cycle.\n");
                  // Bytes left in s2
  	        return;
              } else if (errno == EPIPE)
                        CUSTEXC (;
                                 , "lowsend"
                                 , ERROR_NET_EPIPE
                                 , "broken pipe")

  	        else    INTERROR (;, "lowsend");
	case 0:
                INTERROR (;, "lowsend");
	default:
                cpu[c].sregs[0] = k;
                cpu[c].sregs[1] = 1;
   }
}



// syscall SYSCALL_LOW_NET_RECV
// I:   u0 - descriptor
//      u1 - address
//      u2 - packet length
//      u4 - blocking/non-blocking (1/0)
// O:   s0 - bytes received
//      s1 - ret code (1 == OK, 0 == wouldblock)


void low_recv (int c) {
  static int n, len, k, sock, block, rawb = 0, lbp = 0;
  static char *pkt;

  n = cpu[c].uregs[0];          /* virtual descriptor   */

  CHECKSD (n);

  if (!
      (pkt =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) /4, MEM_FLAG_WRITE)))
    PROTERROR (;, "recv");


  sock = lowdata[c].sd[n];     /* real descriptor      */
  len = cpu[c].uregs[2];
  block = cpu[c].uregs[4];

  switch(lowdata[c].flags[n])
  {
        case RAW_BPF_LISTENER   :
        case RAW_BPF            :       rawb = 1; break;
        case RAW_LISTENER       :       lbp  = 1; break;
        case RAW_SENDER         :
                CUSTEXC (;
                         , "lowrecv"
                         , ERROR_NET_BAD_SD
                         , "bpf socket is read-only");
                break;
        default:
                CUSTEXC (;
                         , "lowrecv"
                         , ERROR_NET_BAD_SD
                         , "not a Listen socket");
                break;}

  if (lbp) k = recv(sock, pkt, len, MSG_DONTWAIT|MSG_NOSIGNAL);
  if (rawb) k = read(sock, pkt, len);

  switch (k) {
        case -1:
                if (errno == EAGAIN) {
                  if (!block) {
                        cpu[c].sregs[1] = 0;
                        return;
                        }
                  ENTER_IOWAIT (c, sock, wait_lowrecv);
                  cpu[c].sregs[1] = 0;
                  printk("packet.so: unable to receive whole data in one cycle.\n");
  	        return;
              } else  if (errno == EPIPE)
                        CUSTEXC (;
                                 , "recv"
                                 , ERROR_NET_EPIPE
                                 , "broken pipe")
  	        else    INTERROR (;, "recv");
	case 0:
                INTERROR (;, "recv");
	default:
                cpu[c].sregs[0] = k;
                cpu[c].sregs[1] = 1;
   }
}



// syscall SYSCALL_LOW_NET_LIVE
// I:   u0 - descriptor
//      u1 - address
//      u2 - packet length
//      u3 - promisc
//      u4 - ms timeout
//      u5 - bsize
//
// O:   s1 - linktype
//      s2 - buffersize


void open_live (int c) {
  struct ifreq ifr;
  unsigned char *device;
  static int len, promisc, fd, ms, n, linktype, bsize, rawb = 0, lbp = 0;
  char *pkt, buf[100],b2[200];

  device=verify_access(c,cpu[c].uregs[0], (cpu[c].uregs[1] + 3) / 4,
         MEM_FLAG_READ);

  if (!device) PROTERROR (;, "initraw");
  if (cpu[c].uregs[1]>=sizeof(buf)) PROTERROR (;, "baddev");
  if (cpu[c].uregs[1]>=sizeof(ifr.ifr_name)) PROTERROR (;, "baddev2");

  bzero(buf,sizeof(buf));
  memcpy(buf,device,cpu[c].uregs[1]);
  sprintf(b2,"net/dev/phys/%s",buf);

  VALIDATE(c,b2,"net/raw/open/live");

  n = cpu[c].uregs[0];          /* virtual descriptor   */

  CHECKSD (n);

  if (!
      (pkt =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) /4, MEM_FLAG_WRITE)))
    PROTERROR (;, "live");


  fd = lowdata[c].sd[n];     /* real descriptor      */
  len = cpu[c].uregs[2];
  promisc = cpu[c].uregs[3];
  ms = cpu[c].uregs[4];

  switch(lowdata[c].flags[n]) {
        case RAW_BPF            :       rawb = 1; break;
        case RAW_LISTENER       :       lbp  = 1; break;
        default:
                CUSTEXC (;
                         , "live"
                         , ERROR_NET_BAD_SD
                         , "not a bpf RAW socket");
                break; }

  if (lbp) {
  static struct ifreq saved_ifr;
  struct sockaddr sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_family = AF_INET;
  (void)strncpy(sa.sa_data, device, sizeof(sa.sa_data));
  if (bind(fd, &sa, sizeof(sa)))
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "bind");

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, device, cpu[c].uregs[5]);
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "SIOCGIFHWADDR");

   switch (ifr.ifr_hwaddr.sa_family) {
        case ARPHRD_ETHER:
	case ARPHRD_METRICOM:   linktype = DLT_EN10MB;  break;
	case ARPHRD_EETHER:     linktype = DLT_EN3MB;   break;
	case ARPHRD_AX25:       linktype = DLT_AX25;    break;
	case ARPHRD_PRONET:     linktype = DLT_PRONET;  break;
	case ARPHRD_CHAOS:      linktype = DLT_CHAOS;   break;
	case ARPHRD_IEEE802:    linktype = DLT_IEEE802; break;
	case ARPHRD_ARCNET:     linktype = DLT_ARCNET;	break;
	case ARPHRD_SLIP:
	case ARPHRD_CSLIP:
	case ARPHRD_SLIP6:
	case ARPHRD_CSLIP6:
	case ARPHRD_PPP:        linktype = DLT_RAW;     break;
	case ARPHRD_LOOPBACK:   linktype = DLT_NULL;    break;
	default:
                CUSTEXC (;
                         , "socket"
                         , ERROR_NET_SOCK
                         , "unknown physical layer type");
                break; }

  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, device, cpu[c].uregs[5]);
  if (ioctl(fd, SIOCGIFMTU, &ifr) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "SIOCGIFMTU");

  bsize = ifr.ifr_mtu + 64;
  if (linktype == DLT_EN10MB) bsize+=2;

  // Set promisc
  if (promisc) {
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, device, cpu[c].uregs[5]);
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0 )
               CUSTEXC (;
                        , "socket"
                        , ERROR_NET_SOCK
                        , "SIOCGIFFLAGS");

		        saved_ifr = ifr;
	                ifr.ifr_flags |= IFF_PROMISC;
                        if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0 )
                                CUSTEXC (;
                                         , "socket"
                                         , ERROR_NET_SOCK
                                         , "SIOCSIFFLAGS");
		        ifr.ifr_flags &= ~IFF_PROMISC;
                        atexit(exiton); }
        }
  if (rawb) {
  bsize = 32768;    // Standar buffersize
  if (ioctl(fd, BIOCSBLEN, (caddr_t)&bsize) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "BIOCSBLEN");

  // Data link type
  if (ioctl(fd, BIOCGDLT, (caddr_t)&linktype) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "BIOCGDLT");

  switch (linktype) {
        case DLT_SLIP:  linktype = DLT_SLIP_BSDOS;      break;
	case DLT_PPP:   linktype = DLT_PPP_BSDOS;       break;
	case 11:        linktype = DLT_RAW;             break;
	case 12:	linktype = DLT_CHDLC;           break;}

  // Set promisc
  if (promisc)
        (void)ioctl(fd, BIOCPROMISC, NULL);

  // Get buffersize
  if (ioctl(fd, BIOCGBLEN, (caddr_t)&bsize) < 0)
        CUSTEXC (;
                 , "socket"
                 , ERROR_NET_SOCK
                 , "BIOCGBLEN");}


  // Set the timeout
  if (ms != 0) {
        struct timeval to;
        // Time conver
        to.tv_sec = ms
                   / 1000;
        to.tv_usec = (ms * 1000)
                     % 1000000;
        if (ioctl(fd, BIOCSRTIMEOUT, (caddr_t)&to) < 0)
                CUSTEXC (;
                        , "socket"
                        , ERROR_NET_SOCK
                        , "BIOCGDLT"); }

  cpu[c].sregs[1] = linktype; // Linktype
  if ( (pkt = (u_char *)malloc(bsize)) == NULL) PROTERROR (;, "malloc");
  if (linktype != DLT_EN10MB) bsize-=2;
  cpu[c].sregs[2] = bsize; // Buffersize
  lowdata[c].sd[n] = fd;
  lowdata[c].flags[n] = RAW_LIVE;
  make_sock_nonblock(c,fd);
}


// syscall SYSCALL_LOW_NET_CLOSE
// I: u0 - descriptor, u1 - how

void close_sock (int c) {
  static int n, how, sock;

  n = cpu[c].uregs[0];
  how = cpu[c].uregs[1];

  CHECKSDNOERR (n);

  sock = lowdata[c].sd[n];

  if ( close(sock) == 0)
                  CUSTEXC (;
                        , "closesock",
                        ERROR_NET_SOCK,
                        "closing sock");

  lowdata[c].sd[n] = 0;
  lowdata[c].flags[n] = LOW_NET_SOCK_FLAG_NONE;
}

void
syscall_handler (int c, int nr)
{
  switch (nr)
  {
    case SYSCALL_LOW_NET_INITDEV:
        open_gap (c);
      break;
    case SYSCALL_LOW_NET_RAW:
        open_raw (c);
      break;
    case SYSCALL_LOW_NET_CLOSE:
        close_sock (c);
      break;
    case SYSCALL_LOW_NET_SEND:
        low_send (c);
      break;
    case SYSCALL_LOW_NET_RECV:
        low_recv (c);
      break;
    case SYSCALL_LOW_NET_BPF:
        open_bpf (c);
      break;
    case SYSCALL_LOW_NET_LIVE:
        open_live (c);
      break;
    }
}

void
syscall_unload (void)
{
  int n, m, k = 0;
  for (m = 0; m < MAX_VCPUS; m++)
    for (n = 0; n < MAX_LOW_SD; n++)
      if (lowdata[m].sd[n]) {
	  k++;
	  close (lowdata[m].sd[n]);
	}

  if (k) printk ("<< l0wlevel-net: shutdown: closed %d open socket descriptors.\n", k);
}


void
syscall_task_cleanup (int c)
{
  int n, k = 0;
  for (n = 0; n < MAX_LOW_SD; n++)
    if (lowdata[c].sd[n])
      {
	k++;
	close (lowdata[c].sd[n]);
	lowdata[c].sd[n] = 0;
	lowdata[c].flags[n] = LOW_NET_SOCK_FLAG_NONE;
      }

  if (k)
    printk("=> l0wlevel-net: task_cleanup: closed %d VCPU's owned socket descriptors.\n", k);
}



/*

   Argante virtual OS
   ------------------

   Low-level socket interface - send / receive, firewalling

   Status: done, but not portable; libpcap will be implemented

   Author:     bikappa <bikappa@itapac.net>
   Maintainer: bikappa <bikappa@itapac.net>
   Patched:    Bulba <bulba@intelcom.pl>
               Michal Zalewski <lcamtuf@ids.pl>

*/

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if 0
#include <netinet/if_ether.h>
#else
#include <linux/if_ether.h>
#endif
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <limits.h>
#include <sys/un.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/uio.h>
#include <linux/socket.h>
#include <linux/sockios.h>
//#include <linux/in.h>
//#include <linux/if.h>

// #include <net/bpf.h>

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

#define NET_LOW_MODULE_VERSION "0.21b"
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
  *(x++) = SYSCALL_LOW_NET_CLOSE;
  *(x++) = SYSCALL_LOW_NET_GETHWADDR;
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



// syscall SYSCALL_LOW_NET_INITDEV

void open_gap (int c) {
  struct ifreq ifr;
  struct sockaddr sa;
  unsigned char *device;
  static int fd, n; // , block, len;
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



// syscall SYSCALL_LOW_NET_RAW

void open_raw (int c) {
  static int fd, n, /* block, len, */ one = 1;
  void *onptr = &one; //lamez

  VALIDATE(c,"none","net/raw/open/sender");

  // by Bulba, and we don't need IP_HDRINCL because of IPPROTO_RAW
  if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1)
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

void
low_send (int c)
{
  static int n, /* block, */ len, k, sock;
  static char *pkt;
  struct sockaddr_in   sain;
  int                  *addr;
  struct iovec         iov;
  struct msghdr                msg;

  n = cpu[c].uregs[0];          /* virtual descriptor   */

  CHECKSD (n);

  if (lowdata[c].flags[n]!=RAW_SENDER) 
     CUSTEXC (;, "send", ERROR_NET_BAD_SD, "not a writable RAW socket");

  sock = lowdata[c].sd[n];     /* real descriptor      */
  len = cpu[c].uregs[2];

  if (!
      (pkt =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) /4, MEM_FLAG_READ)))
    PROTERROR (;, "lowsend");

  // Shouldn't block? -- lcamtuf

  addr = (void *)pkt;
  addr +=4;

  sain.sin_family = AF_INET;
  sain.sin_addr.s_addr = *(unsigned int*)addr;

  memset(&msg, 0, sizeof(msg));
  msg.msg_name = &sain;
  msg.msg_namelen = sizeof (sain);
  msg.msg_iovlen = 1;
  msg.msg_iov = &iov;
  iov.iov_base = pkt;
  iov.iov_len = len;

  k = sendmsg(sock, &msg, MSG_DONTWAIT);

  switch (k) {
    case -1:  if (errno == EAGAIN) {
                cpu[c].sregs[1] = 0;
                printk("packet.so: unable to send whole data in one cycle.\n");
                // Bytes left in s2
  	        return;
              } else
              if (errno == EPIPE)
                CUSTEXC (;, "lowsend",ERROR_NET_EPIPE,"broken pipe")
  	        else INTERROR (;, "lowsend");
	case 0: INTERROR (;, "lowsend");
       default: if (k!=len) {
            cpu[c].sregs[1] = 0;
            printk("packet.so: packet size bigger than interface MTU.\n");
            return;
           }
           cpu[c].sregs[0] = k; cpu[c].sregs[1] = 1;
   }
}


// syscall SYSCALL_LOW_NET_RECV
// I:   u0 - descriptor
//      u1 - address
//      u2 - packet length

void low_recv (int c) {
  static int n, /* block, */ len, k, sock;
  static char *pkt;

  n = cpu[c].uregs[0];          /* virtual descriptor   */

  CHECKSD (n);

  if (lowdata[c].flags[n]!=RAW_LISTENER)
     CUSTEXC (;, "recv", ERROR_NET_BAD_SD, "not a readable RAW socket");

  if (!
      (pkt =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) /4, MEM_FLAG_WRITE)))
    PROTERROR (;, "recv");


  sock = lowdata[c].sd[n];     /* real descriptor      */
  len = cpu[c].uregs[2];

  {
     fd_set rf;
     struct timeval tv = { 0, 0};
     FD_ZERO(&rf);
     FD_SET(sock,&rf);
     if (select(sock+1, &rf, NULL, NULL, &tv)==0) {
       cpu[c].sregs[1] = 0;
       return;
     }
  }

  k = recv(sock, pkt, len, MSG_DONTWAIT|MSG_NOSIGNAL);

  switch (k) {
    case -1:  if (errno == EAGAIN) {
                cpu[c].sregs[1] = 0;
  	        return;
              } else
              if (errno == EPIPE)
                CUSTEXC (;, "recv",ERROR_NET_EPIPE,"broken pipe")
  	        else INTERROR (;, "recv");
	case 0: INTERROR (;, "recv");
	default: cpu[c].sregs[0] = k; cpu[c].sregs[1] = 1;
   }
}

// SYSCALL_LOW_NET_GETHWADDR
// Parameters: u0, u1 - interface name
// Return: u0:u1:u2:u3:u4:u5 - hardware address
//
// HAC: net/raw/hwaddr/get on net/dev/phys/<iface>
// Exceptions: standard memory access, HAC, internal error (if unable to
// create temp socket), ERROR_BAD_SYS_PARAM (unknown interface)

void low_gethwaddr(int c) {
  int x;
  struct ifreq blah;
  char inameX[64];
  char b2[200];
  char* tmpin;
  if (cpu[c].uregs[1]>12)
    CUSTEXC (;, "gethwaddr",ERROR_BAD_SYS_PARAM,"iface name too long")

  tmpin=verify_access(c,cpu[c].uregs[0], (cpu[c].uregs[1] + 3) / 4,
                      MEM_FLAG_READ);
  if (!tmpin) PROTERROR (;, "gethwaddr");

  bzero(inameX,sizeof(inameX)); 
  memcpy(inameX,tmpin,cpu[c].uregs[1]);
  sprintf(b2,"net/dev/phys/%s",inameX);
  VALIDATE(c,b2,"net/raw/hwaddr/get");

  x=socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (x<0) INTERROR (;, "hwaddr_sock");
  strcpy(blah.ifr_ifrn.ifrn_name,inameX);
  if (ioctl(x, SIOCGIFHWADDR, &blah))
    CUSTEXC (;, "gethwaddr",ERROR_BAD_SYS_PARAM,"unknown interface")
  close(x);

  printf("packet: (%s) Running with hwaddr %x:%x:%x:%x:%x:%x\n",inameX,
  blah.ifr_ifru.ifru_hwaddr.sa_data[0] & 0xff,
  blah.ifr_ifru.ifru_hwaddr.sa_data[1] & 0xff,
  blah.ifr_ifru.ifru_hwaddr.sa_data[2] & 0xff,
  blah.ifr_ifru.ifru_hwaddr.sa_data[3] & 0xff,
  blah.ifr_ifru.ifru_hwaddr.sa_data[4] & 0xff,
  blah.ifr_ifru.ifru_hwaddr.sa_data[5] & 0xff);

  cpu[c].uregs[0] = blah.ifr_ifru.ifru_hwaddr.sa_data[0] & 0xff;
  cpu[c].uregs[1] = blah.ifr_ifru.ifru_hwaddr.sa_data[1] & 0xff;
  cpu[c].uregs[2] = blah.ifr_ifru.ifru_hwaddr.sa_data[2] & 0xff;
  cpu[c].uregs[3] = blah.ifr_ifru.ifru_hwaddr.sa_data[3] & 0xff;
  cpu[c].uregs[4] = blah.ifr_ifru.ifru_hwaddr.sa_data[4] & 0xff;
  cpu[c].uregs[5] = blah.ifr_ifru.ifru_hwaddr.sa_data[5] & 0xff;
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
    case SYSCALL_LOW_NET_GETHWADDR:
        low_gethwaddr (c);
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



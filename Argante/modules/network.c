/*

   Argante virtual OS
   ------------------

   Network layer support.

   Status: done

   Author:     Marcin Dawcewicz <marcel@linux.com.pl>
   Maintainer: Marcin Dawcewicz <marcel@linux.com.pl>
   Patched:    Michal Zalewski <lcamtuf@tpi.pl> - unix socket support, ports

*/

#define FS_FLAG_USED            1

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

#ifndef MAP_FAILED
// linux libc?
#define MAP_FAILED      (void*)-1
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



#ifndef SUN_LEN
// Solaris
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)         \
                      + strlen ((ptr)->sun_path))
#endif

#ifndef SHUT_RD
#define SHUT_RD 0
#endif

#ifndef SHUT_WR
#define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#define NET_SOCK_FLAG_NONE              0x00
#define NET_SOCK_FLAG_LISTEN_TCP        0x01
#define NET_SOCK_FLAG_LISTEN_UDP        0x02
#define NET_SOCK_FLAG_CONNECTED_TCP     0x04
#define NET_SOCK_FLAG_CONNECTED_UDP     0x08
#define NET_SOCK_FLAG_UNIX_SOCKET       0x100

#define NETMODULE_VERSION "0.6"
#define SWITCH_TCP	0
#define SWITCH_UDP	1
#define SWITCH_RCV	0
#define SWITCH_SND	1
#define VBUF_SIZE 	512
#define MAX_BACKLOG	64	

#define CHECKSD(d) if (d >= MAX_NET_SD || !netdata[c].sd[d]) { \
    non_fatal (ERROR_NET_BAD_SD, "network: bad socket descriptor", c); \
    failure=1; \
    return; \
  }

#define CHECKSDNOERR(d) if (d >= MAX_NET_SD || !netdata[c].sd[d]) { \
    return; \
  }


#define INTERROR(e,str) { \
    non_fatal(ERROR_NETERROR, "network->" str ": internal error",c); \
    failure=1; \
    return e; \
  }

#define PROTERROR(e,str) { \
    non_fatal(ERROR_PROTFAULT, "network->" str ": attempt to access protected \
                                memory",c); \
    failure=1; \
    return e; \
  }

#define CUSTEXC(e,str,err,msg) { \
    non_fatal(err, "network->" str ": " msg ,c); \
    failure=1; \
    return e; \
  }


struct der_s { // Directory entries...
  char name[257];
  char type;
};


struct process_fs_entry {
  int ders;
  struct der_s* der[MAX_DIRENTS];
  char cwd[MAX_FS_PATH+10];
  char waitfile[MAX_FS_PATH*2+10];
  int ino[MAX_FS_FD];
  int dev[MAX_FS_FD];
  int fd[MAX_FS_FD];
  int flag[MAX_FS_FD];
  int errmode;
};

extern struct process_fs_entry fsdata[MAX_VCPUS];


struct process_net_entry
{
  int sd[MAX_NET_SD];
  int flags[MAX_NET_SD];
  char* saddr;
  char* map, *morig;
  int mlen, mlo;
  fd_set fds;
  int slen;
};

struct process_net_entry netdata[MAX_VCPUS];


void
syscall_load (int *x)
{
  *(x++) = SYSCALL_NET_CONNECT;
  *(x++) = SYSCALL_NET_LISTEN;
  *(x++) = SYSCALL_NET_ACCEPT;
  *(x++) = SYSCALL_NET_SUN_LISTEN;
  *(x++) = SYSCALL_NET_SUN_CONNECT;
  *(x++) = SYSCALL_NET_RECV;
  *(x++) = SYSCALL_NET_SEND;
  *(x++) = SYSCALL_NET_SHUTDOWN;
  *(x++) = SYSCALL_NET_ISWAITING;
  *(x++) = SYSCALL_NET_EVENT;
  *(x++) = SYSCALL_NET_SENDFILE;
  *(x) = SYSCALL_ENDLIST;
  printk (">> Network module v%s loaded.\n", NETMODULE_VERSION);
  printk ("+> Memory usage: %d fd/VCPU (%d bytes)\n", MAX_NET_SD,
	  sizeof (netdata));
}

// make socket non-blocking

int
make_sock_nonblock (int c, int d)
{
  long fl;

  if ((fl = fcntl (d, F_GETFL)) == -1)
    return -1;
  fl |= O_NONBLOCK;
  if (fcntl (d, F_SETFL, fl) == -1)
    return -1;

  return 0;
}


// looks for free descriptors in netdata[].sd[]

int
look_for_sd (int c)
{
  int n;

  for (n = 0; n < MAX_NET_SD; n++)
    if (!netdata[c].sd[n])
      break;

  if (n == MAX_NET_SD)
    return -1;

  return (n);
}


// io_handler for do_connect() (TCP olny)

int
wait_for_conn (int c)
{
  int ret, sd1, s, n, len = sizeof (s);
  struct timeval timeo = { 0, 0 };
  fd_set fds;

  sd1 = cpu[c].iowait_id;	/* real descriptor */

  FD_ZERO (&fds);
  FD_SET (sd1, &fds);

  if ((ret = select (sd1 + 1, NULL, &fds, NULL, &timeo)) == -1)
    INTERROR (1, "connect")
    else
  if (!ret)
    {
      if (cpu[c].uregs[4])	/* is timeout set ? */
	{
	  struct timeval ctime, endtime;

	  if (gettimeofday (&ctime, NULL) == -1)
	    INTERROR (1, "connect");
	  endtime.tv_sec = cpu[c].uregs[3];
	  endtime.tv_usec = cpu[c].uregs[4];
	  if (((ctime.tv_sec == endtime.tv_sec) ?
	       (ctime.tv_usec > endtime.tv_usec) :
	       (ctime.tv_sec > endtime.tv_sec)))
	    {
	      close (sd1);
	      CUSTEXC (1, "connect", ERROR_NET_TIMEO, "connect timeout");
	    }
	}
      return 0;			/* not ready for writing */
    }
  // mmkey, sd1 is ready for writing 

  if ((ret = getsockopt (sd1, SOL_SOCKET, SO_ERROR, &s, &len)) == -1)
    INTERROR (1, "connect");

  if (s)
    switch (s)
      {
      case ECONNREFUSED:
	close (sd1);
	CUSTEXC (1, "connect", ERROR_NET_CONN_REFUSED, "connection refused");
      case ETIMEDOUT:
	close (sd1);
	CUSTEXC (1, "connect", ERROR_NET_TIMEO, "connect timeout");
      default:
	INTERROR (1, "connect");
      }

  if ((n = look_for_sd (c)) == -1)
    CUSTEXC (1, "connect", ERROR_NET_NO_FREE_SD, "no free s-descriptors");
  /* this exception should _never_ occur */

  netdata[c].sd[n] = sd1;
  cpu[c].sregs[0] = n;
  netdata[c].flags[cpu[c].sregs[0]] |= NET_SOCK_FLAG_CONNECTED_TCP;
  FD_SET(sd1,&netdata[c].fds);

  return 1;			/* hey, wake up honey */
}


// syscall NET_CONNECT
// I: u0 - dst addr, u1 - dst port, u2 - src addr, u3 - src port
//    u4 - timeo (usecs), u5 - TCP/UDP (0,1)
// O: s0 - descriptor
// WARNING: if timeo is set then u3 is overwritten by syscall !

// Added by lcamtuf:
// CONNECT for local unix sockets:
// Connect to socket owned by process u0, sockid u1, u4 - timeout,
// u5 - stream / datagram. If u0 is excessive (>65535), external
// program is assumed.

void
do_connect (int c, int unixsock)
{
  int ret, n, sd1, t = cpu[c].uregs[5];
  unsigned int srcaddr = 0, dstaddr = 0;
  unsigned short int srcport = 0, dstport = 0;
  struct sockaddr_in daddr;
  struct sockaddr_un uaddr;
  char vbuf[VBUF_SIZE];
  struct in_addr addr;
  char waitsun[VBUF_SIZE];

  if (unixsock)
    {

      if (cpu[c].uregs[0] > 65535)
	{

	  sprintf (vbuf, "net/address/dest/unix/external/%d",
		   cpu[c].uregs[1]);
	  sprintf (waitsun, UNIXPATH "/external/%d", cpu[c].uregs[1]);


	}
      else
	{

	  sprintf (vbuf, "net/address/dest/unix/%d/%d", cpu[c].uregs[0],
		   cpu[c].uregs[1]);

	  sprintf (waitsun, UNIXPATH "/%d-%d", cpu[c].uregs[0],
		   cpu[c].uregs[1]);

	}

      VALIDATE (c, vbuf, "net/sock/connect");

    }
  else
    {

      if (cpu[c].uregs[1] > 65535 || cpu[c].uregs[3] > 65535)
	CUSTEXC (;
		 , "connect", ERROR_NET_PORT_OOR, "port out of range");

      dstaddr = cpu[c].uregs[0];
      dstport = cpu[c].uregs[1];
      srcaddr = cpu[c].uregs[2];
      srcport = cpu[c].uregs[3];

      bzero (vbuf, VBUF_SIZE);
      addr.s_addr = htonl (dstaddr);
      ret = snprintf (vbuf, VBUF_SIZE - 1, "net/address/dest/%s/%s/%u",
		      (t) ? "udp" : "tcp", inet_ntoa (addr), dstport);
      vbuf[ret] = 0;
      VALIDATE (c, vbuf, "net/sock/connect");

      addr.s_addr = htonl (srcaddr);

      if (srcaddr && srcport)
	ret =
	  snprintf (vbuf, VBUF_SIZE - 1, "net/address/source/%s/%s/%u",
		    (t) ? "udp" : "tcp", inet_ntoa (addr), srcport);
      else if (srcaddr)
	ret =
	  snprintf (vbuf, VBUF_SIZE - 1, "net/address/source/%s/%s",
		    (t) ? "udp" : "tcp", inet_ntoa (addr));
      else if (srcport)
	ret =
	  snprintf (vbuf, VBUF_SIZE, "net/address/source/%s/default/%u",
		    (t) ? "udp" : "tcp", srcport);
      else
	ret =
	  snprintf (vbuf, VBUF_SIZE, "net/address/source/%s/default",
		    (t) ? "udp" : "tcp");
      /* always be paranoid ;P */

      vbuf[ret] = 0;
      VALIDATE (c, vbuf, "net/sock/connect");

      // uff, looks like we have permission to connect

    }

// first of all let's try to acquire a socket from native OS

  if (
      (ret =
       socket ((unixsock) ? PF_UNIX : PF_INET, (t) ? SOCK_DGRAM : SOCK_STREAM,
	       +0)) == -1)
    CUSTEXC (;
	     , "connect-sun", ERROR_NET_SOCK, "unable to create new socket");

// find free slot for new descriptor

  if ((n = look_for_sd (c)) == -1)
    CUSTEXC (;
	     , "connect", ERROR_NET_NO_FREE_SD, "no free s-descriptors");

  sd1 = ret;

// we have a socket, let's make it non-blocking

  if (make_sock_nonblock (c, sd1))
    CUSTEXC (;, "connect", ERROR_NET_NONBLOCK,
	     "unable to switch socket to non-blocking mode");

// we need to do bind() if user wishes to have specific srcaddr/srcport

  if (!unixsock)
    {

      if (srcaddr || srcport)
	{
	  struct sockaddr_in saddr;

	  saddr.sin_family = PF_INET;
	  saddr.sin_port = htons (srcport);
	  saddr.sin_addr.s_addr = htonl (srcaddr);

	  if ((ret = bind (sd1, (struct sockaddr *) &saddr, sizeof (saddr)))
	      == -1)
	    CUSTEXC (;, "connect", ERROR_NET_BIND, "unable to bind socket");
	}

      daddr.sin_family = PF_INET;
      daddr.sin_port = htons (dstport);
      daddr.sin_addr.s_addr = htonl (dstaddr);
      ret = connect (sd1, (struct sockaddr *) &daddr, sizeof (daddr));

    }
  else
    {

      uaddr.sun_family = AF_LOCAL;
      strcpy (uaddr.sun_path, waitsun);
      ret = connect (sd1, (struct sockaddr *) &uaddr, SUN_LEN (&uaddr));

    }


  if (!t)
    {
if (ret !=-1)
{
  netdata[c].sd[n] = sd1;
  cpu[c].sregs[0] = n;
  netdata[c].flags[cpu[c].sregs[0]] |= NET_SOCK_FLAG_CONNECTED_TCP;
  FD_SET(sd1,&netdata[c].fds);
return;
}
     else if (ret == -1 && errno != EINPROGRESS)
	INTERROR (;
		  , "connect");
    }
  else if (t && ret == -1)
    INTERROR (;, "connect");

  if (!t)
    {
      if (cpu[c].uregs[4])
	{
	  struct timeval startime;

	  if (gettimeofday (&startime, NULL) == -1)
	    INTERROR (;
		      , "connect");
	  cpu[c].uregs[3] = startime.tv_sec;
	  cpu[c].uregs[4] += startime.tv_usec;
	}
    ENTER_IOWAIT (c, sd1, wait_for_conn)}
  else				/* datagram sockets only */
    {
      cpu[c].sregs[0] = n;
      netdata[c].sd[n] = sd1;
      FD_SET(sd1,&netdata[c].fds);
      netdata[c].flags[n] |= NET_SOCK_FLAG_CONNECTED_UDP;
    }
}


// syscall NET_LISTEN.
// I: u0 - local addr, u1 - local port, u2 - backlog (TCP only)
//    u5 - TCP/UDP (0/1)
// O: s0 - descriptor

// Added by lcamtuf:
// LISTEN for local unix sockets:
// Listen at sockid u1, u2 - backlog for stream sockets.
// u5 - stream / datagram.


void
do_listen (int c, int unixsock)
{
  int blog = 0, b = 1, ret, n, sd1, t = cpu[c].uregs[5];
  struct sockaddr_in laddr;
  struct sockaddr_un uaddr;
  unsigned long locaddr = 0;
  unsigned int locport = 0;
  char vbuf[VBUF_SIZE];
  struct in_addr addr;
  char waitsun[VBUF_SIZE];

  if (!unixsock)
    {

      if (cpu[c].uregs[1] > 65535)
	CUSTEXC (;, "listen", ERROR_NET_PORT_OOR, "port out of range");

      locaddr = cpu[c].uregs[0];
      locport = cpu[c].uregs[1];

      bzero (vbuf, VBUF_SIZE);
      addr.s_addr = htonl (locaddr);

      if (!locaddr)
	{
	  ret = snprintf (vbuf, VBUF_SIZE - 1, "net/address/source/%s/all/%d",
			  (t) ? "udp" : "tcp", locport);
	}
      else
	{

	  ret = snprintf (vbuf, VBUF_SIZE - 1, "net/address/source/%s/%s/%d",
			  (t) ? "udp" : "tcp", inet_ntoa (addr), locport);
	}

      vbuf[ret] = 0;
      VALIDATE (c, vbuf, "net/sock/listen");

    }
  else
    {

      sprintf (vbuf, "net/address/source/unix/%s/self/%d", (t) ?
	       "dgram" : "stream", cpu[c].uregs[1]);
      VALIDATE (c, vbuf, "net/sock/listen");

      sprintf (waitsun, UNIXPATH "/%d-%d", c, cpu[c].uregs[1]);
      unlink (waitsun);
    }

  if (!t)
    {
      blog = cpu[c].uregs[2];
      if (blog > MAX_BACKLOG)
	CUSTEXC (;
		 , "listen", ERROR_NET_BAD_BLOG,
		 "backlog paramater too high");
    }


// doors wide open

// first of all let's try to acquire a socket from native OS

  if (
      (ret =
       socket ((unixsock) ? PF_LOCAL : PF_INET,
	       (t) ? SOCK_DGRAM : SOCK_STREAM, 0)) == -1)
    CUSTEXC (;
	     , "listen", ERROR_NET_SOCK, "unable to create new socket");

// find free slot for new descriptor

  if ((n = look_for_sd (c)) == -1)
    CUSTEXC (;
	     , "listen", ERROR_NET_NO_FREE_SD, "no free s-descriptors");

  sd1 = ret;

// we have a socket, let's make it non-blocking

  if (make_sock_nonblock (c, sd1))
    CUSTEXC (;, "listen", ERROR_NET_NONBLOCK,
	     "unable to switch socket to non-blocking mode");

  if (setsockopt (sd1, SOL_SOCKET, SO_REUSEADDR, &b, sizeof (b)) == -1)
    INTERROR (;, "listen");

  // time to bind()

  if (!unixsock)
    {

      laddr.sin_family = PF_INET;
      laddr.sin_port = htons (locport);
      laddr.sin_addr.s_addr = htonl (locaddr);

      if ((ret = bind (sd1, (struct sockaddr *) &laddr, sizeof (laddr))) ==
	  -1)
	CUSTEXC (;, "connect", ERROR_NET_BIND, "unable to bind socket");

    }
  else
    {

      uaddr.sun_family = AF_LOCAL;
      strcpy (uaddr.sun_path, waitsun);

      if ((ret = bind (sd1, (struct sockaddr *) &uaddr, SUN_LEN (&uaddr))) ==
	  -1)
	CUSTEXC (;, "connect-sun", ERROR_NET_BIND, "unable to bind socket");

    }

  if (!t && (ret = listen (sd1, blog)) == -1)
    INTERROR (;
	      , "listen");

  netdata[c].sd[n] = sd1;
  cpu[c].sregs[0] = n;

  FD_SET(sd1,&netdata[c].fds);

  if (t)
    netdata[c].flags[n] |= NET_SOCK_FLAG_LISTEN_UDP;
  else
    netdata[c].flags[n] |= NET_SOCK_FLAG_LISTEN_TCP;
}


// io_handler for do_accept()

int
wait_for_client (int c)
{
  int n1, ret, sd1;

  sd1 = cpu[c].iowait_id;

// find free slot for new descriptor

  if ((n1 = look_for_sd (c)) == -1)
    CUSTEXC (1, "accept", ERROR_NET_NO_FREE_SD, "no free s-descriptors");

  if ((ret = accept (sd1, NULL, NULL)) == -1)
    switch (errno)
      {
      case EAGAIN:
	return 0;
      default:
	INTERROR (1, "accept");
      }

  if ((n1 = look_for_sd (c)) == -1)
    CUSTEXC (1, "accept", ERROR_NET_NO_FREE_SD, "no free s-descriptors");
  /* should _never_ occur */

  if (make_sock_nonblock (c, ret))
    CUSTEXC (1, "accept", ERROR_NET_NONBLOCK,
	     "unable to switch socket to non-blocking mode");

  cpu[c].sregs[0] = n1;
  netdata[c].sd[n1] = ret;
  netdata[c].flags[n1] |= NET_SOCK_FLAG_CONNECTED_TCP;
  FD_SET(ret,&netdata[c].fds);

  return 1;			/* wake up */
}


// syscall NET_ACCEPT
// I: u0 - descriptor, u4 - blocking/non-blocking
// O: s0 - new descriptor, s1 - ret code (1 == OK, 0 == wouldblock)

void
do_accept (int c)
{
  int n, n1, block, ret, sd1;

  n = cpu[c].uregs[0];

  CHECKSD (n);

  sd1 = netdata[c].sd[n];
  block = cpu[c].uregs[4];

  if (!(netdata[c].flags[n] & NET_SOCK_FLAG_LISTEN_TCP))
    CUSTEXC (;, "accept", ERROR_NET_SOCK_NON_LISTEN, "non-listening socket");

// find free slot for new descriptor

  if ((n1 = look_for_sd (c)) == -1)
    CUSTEXC (;, "accept", ERROR_NET_NO_FREE_SD, "no free s-descriptors");

  if ((ret = accept (sd1, NULL, NULL)) == -1)
    switch (errno)
      {
      case EAGAIN:		/* would block ...                            */
	if (!block)		/* ... but we dont want to ...                */
	  {
	    cpu[c].sregs[1] = 0;
	    return;
	  }
	/* ... or do we ?     */
	ENTER_IOWAIT (c, sd1, wait_for_client);
	return;
      default:
	INTERROR (;
		  , "accept");
      }

  if (make_sock_nonblock (c, ret))
    CUSTEXC (;, "accept", ERROR_NET_NONBLOCK,
	     "unable to switch socket to non-blocking mode");


  cpu[c].sregs[0] = n1;
  cpu[c].sregs[1] = 1;
  netdata[c].sd[n1] = ret;
  FD_SET(ret,&netdata[c].fds);
  netdata[c].flags[n1] |= NET_SOCK_FLAG_CONNECTED_TCP;
}

// 1st io_handler's  for do_transm()

int
wait_for_recv (int c)
{
  int k, sd1, len = cpu[c].uregs[2];
  char *buf;

  sd1 = cpu[c].iowait_id;

  if (!
      (buf =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) / 4,
		      MEM_FLAG_WRITE))) PROTERROR (1, "recv");

  k = recv (sd1, buf, len, MSG_DONTWAIT | MSG_NOSIGNAL);

  switch (k)
    {
    case -1:
      if (errno == EAGAIN)
	return 0;
      else if (errno == EPIPE)
	CUSTEXC (1, "recv", ERROR_NET_EPIPE, "broken pipe")
	else
	INTERROR (1, "recv");
    case 0:			/* remote end closed the connection */
      netdata[c].flags[cpu[c].uregs[0]] = NET_SOCK_FLAG_NONE;
      CUSTEXC (1, "trans", ERROR_NET_EOF, "remote party disconnected");
    default:
      cpu[c].sregs[0] = k;
      return 1;			/* wake up */
    }
}


// 2nd io_handler for do_transm()

int
wait_for_send (int c)
{
  int k, sd1, len = netdata[c].slen;
  char *buf;

  sd1 = cpu[c].iowait_id;
  buf=netdata[c].saddr;

sendlup1:

  k = send (sd1, buf, len, MSG_NOSIGNAL | MSG_DONTWAIT);

  if ( (k>=0) && (k<len) ) {
    buf += k;
    len -= k;
    netdata[c].slen -= k;
    netdata[c].saddr += k;
    goto sendlup1;
  }

  switch (k)
    {
    case -1:
      if (errno == EAGAIN)
	return 0;
      else if (errno == EPIPE)
	CUSTEXC (1, "send", ERROR_NET_EPIPE, "broken pipe")
	else
	INTERROR (1, "send");
    default:
      cpu[c].sregs[0] = k;
      return 1;			/* wake up */
    }
}

#define MAX_SEND_UNIT 32768


int wait_for_sendfile (int c) {
  int n=cpu[c].iowait_id,k;

sendlup3:

  k = send (n, netdata[c].map, (netdata[c].mlen>MAX_SEND_UNIT) ? MAX_SEND_UNIT
            : netdata[c].mlen, MSG_NOSIGNAL | MSG_DONTWAIT);

  if ( (k>=0) && (k<netdata[c].mlen) ) {
    netdata[c].map += k;
    netdata[c].mlen -= k;
    goto sendlup3;
  }

  switch (k) {
    case -1:
      if (errno == EAGAIN) return 0;
      else if (errno == EPIPE) {
        munmap(netdata[c].morig,netdata[c].mlo);
        CUSTEXC (1, "sendfile", ERROR_NET_EPIPE, "broken pipe");
      } else {
        munmap(netdata[c].morig,netdata[c].mlo);
	INTERROR (1, "sendfile");
      }
    default:
      munmap(netdata[c].morig,netdata[c].mlo);
      return 1;
  }
}



// Quadruple combo ;) - services NET_{RECV,SEND} (TCP & UDP)
// I: u0 - descriptor, u1 - buffer address, u2 - buffer length
//    u4 - blocking/non-blocking (1/0)
// O: s0 - bytes sent/received , s1 - ret code (1 == OK, 0 == wouldblock)

void
do_transm (int c, int t)
{
  int n, sd1, block, len, k;
  char *buf;

  n = cpu[c].uregs[0];		/* virtual descriptor */

  CHECKSD (n);

  sd1 = netdata[c].sd[n];	/* real descriptor    */
  block = cpu[c].uregs[4];
  len = cpu[c].uregs[2];

  if (!
      (buf =
       verify_access (c, cpu[c].uregs[1], (cpu[c].uregs[2] + 3) / 4,
		      (t) ? MEM_FLAG_READ : MEM_FLAG_WRITE)))
    PROTERROR (;, "transm");

  cpu[c].sregs[2]=0;

  if (netdata[c].flags[n] & NET_SOCK_FLAG_CONNECTED_UDP
      || netdata[c].flags[n] & NET_SOCK_FLAG_CONNECTED_TCP)
    {

sendlup2:

     if (t) {
       k = send (sd1, buf, len, MSG_DONTWAIT | MSG_NOSIGNAL);
       if ( (k>=0) && (k<len) ) {
         buf += k;
         len -= k;
         cpu[c].sregs[2] = len;
         goto sendlup2;
       }
     } else {
       k = recv (sd1,buf,len,MSG_DONTWAIT  | MSG_NOSIGNAL);
     }


      switch (k)
	{
	case -1:
	  if (errno == EAGAIN)	/* unfortunately would block ...              */
	    {
	      if (!block)	/* ... but we dont want to ...                */
		{
		  cpu[c].sregs[1] = 0;
		  return;
		}
	      /* ... or do we ?     */
              netdata[c].saddr=buf; netdata[c].slen=len;
	      ENTER_IOWAIT (c, sd1, (t) ? wait_for_send : wait_for_recv);
	      return;
	    }
	  else if (errno == EPIPE)
	    CUSTEXC (;
		     , "transm", ERROR_NET_EPIPE, "broken pipe")
	  else
	    INTERROR (;
		      , "transm");
	case 0:		/* remote end closed the connection */
	  if (!t)
	    {
	      netdata[c].flags[cpu[c].uregs[0]] = NET_SOCK_FLAG_NONE;
	      CUSTEXC (;
		       , "trans", ERROR_NET_EOF, "remote party disconnected");
	    }
	  else
	    INTERROR (;
		      , "transm");
	default:
	  cpu[c].sregs[0] = k;
	  cpu[c].sregs[1] = 1;
	}
    }
  else
    CUSTEXC (;
	     , "transm", ERROR_NET_SOCK_NOT_CONN, "socket not connected");
}

// syscall NET_SHUTDOWN
// I: u0 - descriptor, u1 - how

void
do_shutdown (int c)
{
  int /* ret, */ n, how, sd1;
  n = cpu[c].uregs[0];		/* virtual descriptor */
  how = cpu[c].uregs[1];

  CHECKSDNOERR (n);

  sd1 = netdata[c].sd[n];

  if (how != SHUT_RD && how != SHUT_WR && how != SHUT_RDWR)
    CUSTEXC (;, "shutdown", ERROR_NET_BAD_HOW, "invalid 'how' parameter");

//  if (netdata[c].flags[n] & NET_SOCK_FLAG_CONNECTED_TCP)
//    {
//      if ((ret = shutdown (sd1, how)) == -1) 
//	INTERROR (;
//		  , "shutdown");
//    shutdown (sd1, how);
//    }

  close (sd1);

  if (FD_ISSET(sd1,&netdata[c].fds)) FD_CLR(sd1,&netdata[c].fds);

  netdata[c].sd[n] = 0;
  netdata[c].flags[n] = NET_SOCK_FLAG_NONE;
}


// syscall NET_ISWAITING
// I: u0 - descriptor (listening socket)
// O: s0 - client/no_clients (1/0)

void
do_iswaiting (int c)
{
  int n, sd1, ret;
  fd_set fds;
  struct timeval timeo = {
    0, 0
  };
  n = cpu[c].uregs[0];

  CHECKSD (n);

  sd1 = netdata[c].sd[n];

  if (!(netdata[c].flags[n] & NET_SOCK_FLAG_LISTEN_TCP))
    CUSTEXC (;, "iswaiting", ERROR_NET_SOCK_NON_LISTEN,
	     "non-listening socket");

  FD_ZERO (&fds);
  FD_SET (sd1, &fds);

  ret = select (sd1 + 1, &fds, NULL, NULL, &timeo);

  switch (ret)
    {
    case -1:
      INTERROR (;, "iswaiting");
    case 0:
      cpu[c].sregs[0] = 0;	/* no clients */
      break;
    default:
      cpu[c].sregs[0] = 1;	/* client */
    }
}


int wait_event (int c) {
  int ret;
  struct timeval timeo = { 0, 0 };
  fd_set copy;
  memcpy(&copy,&netdata[c].fds,sizeof(fd_set));
  ret = select (MAX_NET_SD + 1, &copy, NULL, &copy, &timeo);
  switch (ret) {
    case -1: INTERROR (1, "event-select");
    case 0: return 0;
    default: return 1;
  }
}



void do_event (int c) {
  int ret;
  struct timeval timeo = {  0, 0  };
  fd_set copy;
  memcpy(&copy,&netdata[c].fds,sizeof(fd_set));
  ret = select (MAX_NET_SD+1, &copy, NULL, &copy, &timeo);

  switch (ret) {
    case -1: INTERROR (;, "event-select");
    case 0: ENTER_IOWAIT(c,0,wait_event); return;
    default: return ;
  }
}



void do_sendfile (int c) {
  struct stat x;
  int fd,n=cpu[c].uregs[3];
  if (cpu[c].uregs[0]>=MAX_FS_FD) {
    non_fatal(ERROR_FS_BAD_VFD,"sendfile: VFD number too big",c);
    failure=1;
    return;
  }
  fd=fsdata[c].fd[cpu[c].uregs[0]];
  if (!(fsdata[c].flag[cpu[c].uregs[0]] & FS_FLAG_USED)) {
    non_fatal(ERROR_FS_BAD_VFD,"sendfile: VFD not open",c);
    failure=1;
    return;
  }
  if (fstat(fd,&x)) {
    non_fatal(ERROR_FSERROR,"sendfile: fstat failed",c);
    failure=1;
    return;
  }
  if (!cpu[c].uregs[1]) cpu[c].uregs[1]=x.st_size;
  if (cpu[c].uregs[2]+cpu[c].uregs[1]>x.st_size) {
    non_fatal(ERROR_BAD_SYS_PARAM,"sendfile: offset + length > size",c);
    failure=1;
    return;
  }

  if (!(netdata[c].flags[n] & NET_SOCK_FLAG_CONNECTED_UDP) && 
      !(netdata[c].flags[n] & NET_SOCK_FLAG_CONNECTED_TCP)) {
    non_fatal(ERROR_NETERROR,"sendfile: not a connected socket",c);
    failure=1;
    return;
  }

  netdata[c].map=mmap(0,cpu[c].uregs[1],PROT_READ,MAP_PRIVATE,fd,cpu[c].uregs[2]);
  netdata[c].mlen=cpu[c].uregs[1];
  if (netdata[c].map==MAP_FAILED) {
    netdata[c].map=0;
    non_fatal(ERROR_FSERROR,"sendfile: mmap() failure",c);
    failure=1;
    return;
  }
  netdata[c].morig=netdata[c].map;
  netdata[c].mlo=cpu[c].uregs[1];
  ENTER_CRITWAIT(c,netdata[c].sd[n],wait_for_sendfile);

}




void
syscall_handler (int c, int nr)
{
  switch (nr)
    {
    case SYSCALL_NET_CONNECT:
      do_connect (c, 0);
      break;
    case SYSCALL_NET_SUN_CONNECT:
      do_connect (c, 1);
      break;
    case SYSCALL_NET_LISTEN:
      do_listen (c, 0);
      break;
    case SYSCALL_NET_SUN_LISTEN:
      do_listen (c, 1);
      break;
    case SYSCALL_NET_ACCEPT:
      do_accept (c);
      break;
    case SYSCALL_NET_RECV:
      do_transm (c, SWITCH_RCV);
      break;
    case SYSCALL_NET_SEND:
      do_transm (c, SWITCH_SND);
      break;
    case SYSCALL_NET_SHUTDOWN:
      do_shutdown (c);
      break;
    case SYSCALL_NET_ISWAITING:
      do_iswaiting (c);
      break;
    case SYSCALL_NET_EVENT:
      do_event (c);
      break;
    case SYSCALL_NET_SENDFILE:
      do_sendfile (c);
      break;
    }
}

void
syscall_unload (void)
{
  int n, m, k = 0;
  for (m = 0; m < MAX_VCPUS; m++)
    for (n = 0; n < MAX_NET_SD; n++)
      if (netdata[m].sd[n])
	{
	  k++;
	  close (netdata[m].sd[n]);
	}

  if (k)
    printk ("<< net: shutdown: closed %d open socket descriptors.\n", k);
}

void
syscall_task_cleanup (int c)
{
  int n, k = 0;
  for (n = 0; n < MAX_NET_SD; n++)
    if (netdata[c].sd[n])
      {
	k++;
	close (netdata[c].sd[n]);
	netdata[c].sd[n] = 0;
	netdata[c].flags[n] = NET_SOCK_FLAG_NONE;
      }

  if (k)
    printk
      ("=> net: task_cleanup: closed %d VCPU's owned socket descriptors.\n",
       k);}

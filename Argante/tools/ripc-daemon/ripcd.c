
/*
Argante rIPC daemon v0.5

todo:
- primary: make this code understandable (rewrite this from scratch ;P)
- getopt()
- strenghten config file parsing function
- add some security features (DoS protections especially)
- clean up awful error handling

Marcin Dawcewicz

*/

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>

char* CONFIG=CFGFILE;

#ifdef USE_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>


// customize this
#define CERTFILE	"/var/state/openssl/cacert.pem"
#define KEYFILE		"/var/state/openssl/private/cakey.pem"

#define CHK_NULL(x) if ((x)==NULL) {fprintf(stderr,"SSL Error\n");ERR_print_errors_fp(stderr);exit (1);}
#define CHK_SSL(err) if ((err)==-1) { fprintf(stderr,"SSL Error\n");ERR_print_errors_fp(stderr); exit(1); }

SSL_METHOD *meth;
SSL_CTX *ctx;
#endif

#ifndef UNIX_PATH_MAX		/* max unix socket name length */
#define UNIX_PATH_MAX   108
#endif

#ifndef NOFILE			/* max opened files per process */
#define NOFILE 128
#endif

//#define CONFIG "ripcd.conf"

// MAX_ENTS + MAX_CLIS should be << NOFILE
#define MAX_ENTS	16	/* max entries (lines) in config file */
#define MAX_CLIS	32	/* max concurrent clients serviced */

// MAX_KEYLEN must be > BUFSIZE
#define MAX_KEYLEN	4096	/* max key length */
#define BUFSIZE		1024

#define CONF_ELEMS	5	/* elements in one line of config file */
#define BLOG		5	/* backlog for stream sockets */

#define INTERROR(sd,msg,func) { int back = errno; if (send (sd, "ERR!\n", 5, 0) == -1 && errno != EAGAIN) syslog (LOG_ERR, "can't send message to remote client (send(): %m"); errno = back; syslog (LOG_ERR, "%s (%s(): %m)", msg, func);  close (sd);  return -1; }

#ifdef USE_SSL
#define SSLERROR(sd,func,ret,ssl) {int ret2; if ((ret2=SSL_write (ssl, "ERR!\n", 5)) == -1 && ((SSL_get_error(ssl,ret2)!=SSL_ERROR_WANT_READ) || (SSL_get_error(ssl,ret2)!=SSL_ERROR_WANT_WRITE))) syslog (LOG_ERR, "can't send message to remote client (SSL_write(): %i",SSL_get_error(ssl,ret2));syslog(LOG_ERR,"SSL error (%s(): %i)",(func),ret);SSL_free(ssl);close(sd);return -1; }
#endif

#define STAT_CONNECTING		1
#define STAT_WAITFORKEY		2
#define STAT_RELAY		4
#define STAT_KEYSEND		8
#define STAT_WAITFORSSL		16

#ifndef AF_LOCAL
// Solaris
#define AF_LOCAL AF_UNIX
#endif

#ifndef PF_LOCAL
// Solaris
#define PF_LOCAL AF_UNIX
#endif

#undef sun

typedef struct
{
  int prot;			/* 0 - plain, 1 - ssl   */
  int lstn;			/* 0 - inet , 1 - local */
  struct in_addr ipaddr;
  unsigned short int port;
  char sun[UNIX_PATH_MAX];
  char key[MAXPATHLEN];
  int sd;			/* listening socket descriptor */
}
cfile_entry;			/* one line in config file */

typedef struct
{
  int l, r;			/* local/remote side */
  int key_len;			/* key length */
  int stat;			/* status */
  int ref;			/* cf[] index */
  int offs;
  char keyb[MAX_KEYLEN];	/* buffer for key */

#ifdef USE_SSL
  SSL *ssl;
#endif

}
connsd;				/* single junction */

cfile_entry cf[MAX_ENTS];	/* all lines */
int ent;			/* number of loaded lines */
connsd csd[MAX_CLIS];		/* connected sockets */
char gbuf[1024];		/* all-purpose buffer ;) */


// SIG_INT handler

void
int_handle (int c)
{

#ifdef USE_SSL
  SSL_CTX_free (ctx);
#endif

  exit (1);
}

int
do_relay (int m);


// Load configuration

int
parse_cfile (char *path)
{
  int i = 0;
  FILE *fd;
  char buf[CONF_ELEMS][256];
  char *p;

  if (!(fd = fopen (path, "r")))
    {
      fprintf (stderr, "Can't open config file '%s' (fopen(): %s)\n", CONFIG,
	       strerror (errno));
      perror ("fopen");
      exit (1);
    }

  fprintf (stdout, "\nParsing config file '%s' ...\n\n", CONFIG);

  while (fscanf
	 (fd, "%s %s %s %s %s", buf[0], buf[1], buf[2], buf[3],
	  buf[4]) == CONF_ELEMS && i < MAX_ENTS)
    {
      p = strchr (buf[2], ':');
      *p = 0;
      inet_aton (buf[2], &cf[i].ipaddr);
      cf[i].port = atoi (p + 1);

      cf[i].prot = (strcmp (buf[1], "ssl")) ? 0 : 1;
      cf[i].lstn = (strcmp (buf[0], "inet")) ? 1 : 0;

      strncpy (cf[i].sun, buf[3], UNIX_PATH_MAX);
      cf[i].sun[UNIX_PATH_MAX - 1] = 0;
      strncpy (cf[i].key, buf[4], MAXPATHLEN);
      cf[i].key[MAXPATHLEN - 1] = 0;

      printf ("#%i (%s)\t%s:%i  %s\t<=>\t%s  %s\n", i,
	      (cf[i].prot) ? "plain" : "ssl  ", inet_ntoa (cf[i].ipaddr),
	      cf[i].port, (cf[i].lstn) ? "" : "L", cf[i].sun,
	      (cf[i].lstn) ? "L" : "");

      i++;
    }

  printf ("\n%i entries loaded.\n", i);

  fclose (fd);
  return i;
}

// Make socket non-blocking

int
make_sock_nonblock (int d)
{
  long fl;

  if ((fl = fcntl (d, F_GETFL)) == -1)
    return -1;
  fl |= O_NONBLOCK;
  if (fcntl (d, F_SETFL, fl) == -1)
    return -1;

  return 0;
}

// No comment

void
daemonize (char *name)
{
  int i, pid;

  fprintf (stdout, "\nDaemonizing ...\n\n");

  pid = fork ();

  if (pid == -1)
    {
      fprintf (stderr, "Can't daemonize (fork(): %s)\n", strerror (errno));
      exit (1);
    }
  else if (pid != 0)
    exit (0);

  if (setsid () == -1)
    {
      fprintf (stderr, "Can't daemonize (setsid(): %s)\n", strerror (errno));
      exit (1);
    }

  if (signal (SIGHUP, SIG_IGN) == SIG_ERR)
    {
      fprintf (stderr, "Can't daemonize (signal(): %s)\n", strerror (errno));
      exit (1);
    }

  if (signal (SIGPIPE, SIG_IGN) == SIG_ERR)
    {
      perror ("signal");
      exit (1);
    }

  if (signal (SIGINT, int_handle) == SIG_ERR)
    {
      perror ("signal");
      exit (1);
    }

//  chdir ("/");
//  umask (0);

  for (i = 0; i < NOFILE; i++)
    close (i);

  openlog (name, LOG_PID, LOG_DAEMON);
  syslog (0, "Argante rIPC daemon started");
}

// Activate entries from config file (make listening endpoints)

int
activate_ents ()
{
  int n, sd=0, ret = 0;
  struct sockaddr_in saddrin;
  struct sockaddr_un saddrun;

  fprintf (stdout, "\nActivating entries ... \n\n");

  for (n = 0; n < ent; n++)
    {
      fprintf (stdout, "#%i ...", n);
      switch (cf[n].lstn)
	{

	case 0:		/* inet  */
	  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	    {
	      fprintf (stdout, "ERROR (socket(): %s)\n", strerror (errno));
	      continue;
	    }

	  saddrin.sin_family = AF_INET;
	  saddrin.sin_addr = cf[n].ipaddr;
	  saddrin.sin_port = htons (cf[n].port);

	  {
	    int b = 1;
	    if (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, &b, sizeof (b)) ==
		-1)
	      {
		fprintf (stdout, "ERROR (setsockopt(): %s)\n",
			 strerror (errno));
		close (sd);
		continue;
	      }
	  }

	  if (bind (sd, (struct sockaddr *) &saddrin, sizeof (saddrin)) == -1)
	    {
	      fprintf (stdout, "ERROR (bind(): %s)\n", strerror (errno));
	      close (sd);
	      continue;
	    }
	  break;

	case 1:		/* local */
	  if ((sd = socket (AF_LOCAL, SOCK_STREAM, 0)) == -1)
	    {
	      fprintf (stdout, "ERROR (socket(): %s)\n", strerror (errno));
	      continue;
	    }

	  saddrun.sun_family = AF_LOCAL;
	  strncpy (saddrun.sun_path, cf[n].sun, UNIX_PATH_MAX);
	  saddrun.sun_path[UNIX_PATH_MAX - 1] = 0;

	  unlink (cf[n].sun);

	  if (bind (sd, (struct sockaddr *) &saddrun, sizeof (saddrun)) == -1)
	    {
	      fprintf (stdout, "ERROR (bind(): %s)\n", strerror (errno));
	      close (sd);
	      continue;
	    }
	  break;
	}

      if (make_sock_nonblock (sd) == -1)
	{
	  fprintf (stdout, "ERROR (fcntl(): %s)\n", strerror (errno));
	  close (sd);
	  continue;
	}
      else
	{
	  if (listen (sd, BLOG) == -1)
	    {
	      fprintf (stdout, "ERROR (listen(): %s)\n", strerror (errno));
	      close (sd);
	      continue;
	    }
	  cf[n].sd = sd;
	  printf ("OK.\n");
	  ret++;
	}
    }

  return ret;
}

// Find entry in cf[].sd or csd[] which has associated given socket descriptor

int
find_ibysd (int s, int set)
{
  int n;

  switch (set)
    {
    case 0:
      for (n = 0; n < MAX_ENTS; n++)
	if (cf[n].sd == s)
	  return n;
      break;
    case 1:
      for (n = 0; n < MAX_CLIS; n++)
	if (csd[n].l == s || csd[n].r == s)
	  return n;
      break;
    }

  return -1;
}

// Check if descriptor belongs to cf[].sd or csd[]

int
belongs_to (int s)
{
  int n;

  for (n = 0; n < MAX_ENTS; n++)
    if (cf[n].sd == s)
      return 0;

  for (n = 0; n < MAX_CLIS; n++)
    if (csd[n].l == s || csd[n].r == s)
      return 1;

  return -1;
}

// Find free slot in csd[]

int
find_free_slot ()
{
  int n;
  for (n = 0; n < MAX_CLIS; n++)

    if (!csd[n].l)
      return n;

  return -1;
}

// Connect to unix socket

int
connect_sun (int i)
{
  struct sockaddr_un sun;
  int sd1, m, j, ret;

  m = csd[i].l;
  j = csd[i].ref;

  if ((sd1 = socket (AF_LOCAL, SOCK_STREAM, 0)) == -1)
    {
      csd[i].l = 0;

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      INTERROR (m, "can't create new socket", "socket");
    }

  if (make_sock_nonblock (sd1) == -1)
    {
      int back = errno;

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      csd[i].l = 0;
      close (sd1);
      errno = back;
      INTERROR (m, "can't make socket non-blocking", "fcntl()");
    }

  sun.sun_family = AF_LOCAL;
  strncpy (sun.sun_path, cf[j].sun, UNIX_PATH_MAX);

  if ((ret = connect (sd1, (struct sockaddr *) &sun, sizeof (sun))) == -1
      && errno != EINPROGRESS)
    {
      csd[i].l = 0;

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      close (sd1);
      INTERROR (m, "connect to local socket", "connect");
    }

  csd[i].r = sd1;
  csd[i].stat = STAT_CONNECTING;

  if (ret != -1)
    do_relay (m);

  return 0;
}

// Initiate sending key to remote daemon

int
send_key (int i)
{
  struct sockaddr_in sin;
  int sd1, m, j, ret;

  m = csd[i].l;
  j = csd[i].ref;

  if ((sd1 = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      csd[i].l = 0;

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      INTERROR (m, "can't create new socket", "socket");
    }

  if (make_sock_nonblock (sd1) == -1)
    {
      int back = errno;

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      csd[i].l = 0;
      close (sd1);
      errno = back;
      INTERROR (m, "can't make socket non-blocking", "fcntl()");
    }

  sin.sin_family = AF_INET;
  sin.sin_addr = cf[j].ipaddr;
  sin.sin_port = htons (cf[j].port);

  if ((ret = connect (sd1, (struct sockaddr *) &sin, sizeof (sin))) == -1
      && errno != EINPROGRESS)
    {
      csd[i].l = 0;

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      close (sd1);
      INTERROR (m, "connect to remote daemon", "connect");
    }

  csd[i].r = sd1;
  csd[i].stat = STAT_CONNECTING;

  if (ret != -1)
    do_relay (m);

  return 0;
}

// Compare keys from client and from file (or just load the key - sw=1)

int
verify_key (int m, int i, int sw)
{
  int fd, ret;
  struct stat statf;
  char *p;

  if ((fd = open (cf[csd[i].ref].key, O_RDONLY)) == -1)
    {
      csd[i].l = 0;

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      INTERROR (m, "can't open key file", "open");
    }

  if (fstat (fd, &statf) == -1)
    {
      int back = errno;
      csd[i].l = 0;
      close (fd);

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      errno = back;
      INTERROR (m, "can't stat key file", "fstat");
    }

  if (!sw)
    {
      if (statf.st_size != csd[i].key_len)
	{
	  csd[i].l = 0;
	  close (fd);

#ifdef USE_SSL
	  if (csd[i].ssl)
	    SSL_free (csd[i].ssl);
#endif

	  errno = EACCES;
	  INTERROR (m, "key length mismatch", "verify_key");
	}
    }

  if (!(p = malloc ((sw) ? MAX_KEYLEN : csd[i].key_len)))
    {
      int back = errno;
      csd[i].l = 0;
      close (fd);

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      errno = back;
      INTERROR (m, "internal error", "malloc");
    }


  if ((ret = read (fd, p, (sw) ? MAX_KEYLEN : csd[i].key_len)) <= 0)
    {
      int back = errno;
      csd[i].l = 0;
      free (p);
      close (fd);

#ifdef USE_SSL
      if (csd[i].ssl)
	SSL_free (csd[i].ssl);
#endif

      errno = back;
      INTERROR (m, "can't read key file", "read");
    }

  if (!sw)
    {
      if (memcmp (csd[i].keyb, p, csd[i].key_len))
	{

	  csd[i].l = 0;
	  free (p);
	  close (fd);

#ifdef USE_SSL
	  if (csd[i].ssl)
	    SSL_free (csd[i].ssl);
#endif

	  errno = EACCES;
	  INTERROR (m, "invalid key", "verify_key");
	}
    }


// verified/loaded

  if (sw)
    {
      csd[i].key_len = ret;
      memcpy (csd[i].keyb, p, MAX_KEYLEN);
    }

  free (p);
  close (fd);

  return 0;
}

// Main relaying function

int
do_relay (int m)
{
  int ret, i;

  i = find_ibysd (m, 1);

  bzero (gbuf, sizeof (gbuf));

#ifdef USE_SSL
  if (csd[i].stat == STAT_WAITFORSSL)
    {
      ret =
	(cf[csd[i].ref].lstn) ? SSL_connect (csd[i].
					     ssl) : SSL_accept (csd[i].ssl);

      switch ((SSL_get_error (csd[i].ssl, ret)))
	{
	case SSL_ERROR_NONE:
	  switch (cf[csd[i].ref].lstn)
	    {
	    case 1:
	      csd[i].stat = STAT_KEYSEND;
	      do_relay (m);
	      break;
	    case 0:
	      csd[i].stat = STAT_WAITFORKEY;
	      break;
	    }
	case SSL_ERROR_WANT_READ:
	  return 0;
	case SSL_ERROR_WANT_WRITE:
	  return 0;
	default:
	  csd[i].stat = 0;
	  ret = SSL_get_error (csd[i].ssl, ret);
	  if (cf[csd[i].ref].lstn)
	    {
	      if (send (csd[i].l, "ERR!\n", 5, 0) == -1 && errno != EAGAIN)
		syslog (LOG_ERR,
			"can't send message to remote client (send(): %m");
	    }
	  syslog (LOG_ERR, "SSL error (%s(): %i)",
		  (cf[csd[i].ref].lstn) ? "SSL_connect" : "SSL_accept", ret);
	  SSL_free (csd[i].ssl);
	  close (m);
	  csd[i].l = 0;
	  return -1;
	}
    }
  else
#endif

  if (csd[i].stat == STAT_WAITFORKEY)
    {

      if (cf[csd[i].ref].lstn)	//listening on local
	{

	  if (!verify_key (m, i, 1))
	    send_key (i);
	  return 0;
	}

// listening on inet
      while (1)
	{
#ifdef USE_SSL
	  int sslr = (cf[csd[i].ref].prot) ? 1 : 0;
#endif

	  bzero (gbuf, sizeof (gbuf));

#ifdef USE_SSL
	  ret =
	    (!sslr) ? recv (m, gbuf, sizeof (gbuf), 0) : SSL_read (csd[i].ssl,
								   gbuf,
								   sizeof
								   (gbuf));
#else
	  ret = recv (m, gbuf, sizeof (gbuf), 0);
#endif

	  if (ret > 0)
	    {
	      int len = 0;

	      if (!csd[i].key_len)
		{
		  char *p;
		  if (sscanf (gbuf, "%i", &csd[i].key_len) <= 0
		      || csd[i].key_len > MAX_KEYLEN
		      || !(p = strchr (gbuf, '\n')))
		    {
		      csd[i].l = 0;

#ifdef USE_SSL
		      if (sslr)
			{
			  SSLERROR (m, "SSL_write",
				    SSL_get_error (csd[i].ssl, ret),
				    csd[i].ssl);
			}
		      else
			{
			  INTERROR (m, "protocol error", "sscanf");
			}

		      if (csd[i].ssl)
			SSL_free (csd[i].ssl);
#else
		      INTERROR (m, "protocol error", "sscanf");
#endif
		    }

		  csd[i].offs = csd[i].key_len;

		  if (*(p + 1))
		    {
		      ret = &gbuf[ret - 1] - p;
		      memmove (gbuf, p + 1, ret);
		    }
		  else
		    {
		      ret = 0;
		    }
		}


	      len = (ret <= csd[i].offs) ? ret : csd[i].offs;
	      memcpy (&csd[i].keyb[csd[i].key_len - csd[i].offs], gbuf, len);

	      if (!(csd[i].offs -= len))	// whole key received
		{
		  if (!verify_key (m, i, 0))
		    connect_sun (i);
		  return 0;
		}
	      continue;
	    }
	  else
	    {
	      switch (ret)
		{
		case 0:
		  csd[i].l = 0;

#ifdef USE_SSL
		  if (csd[i].ssl)
		    SSL_free (csd[i].ssl);
#endif

		  close (m);
		  return -1;
		  break;
		case -1:
		  if (errno != EAGAIN
#ifdef USE_SSL
		      && SSL_get_error (csd[i].ssl,
					ret) != SSL_ERROR_WANT_READ
		      && SSL_get_error (csd[i].ssl,
					ret) != SSL_ERROR_WANT_WRITE
#endif
		    )
		    {
		      csd[i].l = 0;

#ifdef USE_SSL
		      if (sslr)
			{
			  SSLERROR (m, "SSL_read",
				    SSL_get_error (csd[i].ssl, ret),
				    csd[i].ssl);}
		      else
			{
			  INTERROR (m, "can't read from client", "recv");
			}
#else
		      INTERROR (m, "can't read from client", "recv");
#endif
		    }
		}
	      return 0;
	    }
	}
    }
  else if (csd[i].stat == STAT_CONNECTING)
    {
      int err, len = sizeof (err);

      if (getsockopt (m, SOL_SOCKET, SO_ERROR, &err, &len) == -1 || err)
	{
	  int d = csd[i].l;

	  close (csd[i].r);
	  csd[i].l = 0;

#ifdef USE_SSL
	  if (csd[i].ssl)
	    SSL_free (csd[i].ssl);
#endif

	  errno = err;
	  INTERROR (d, "can't connect to remote party", "connect");
	}

#ifdef USE_SSL
      if (cf[csd[i].ref].lstn && cf[csd[i].ref].prot)
	{
	  if ((csd[i].ssl = SSL_new (ctx)) == NULL)
	    {
	      if (send (csd[i].l, "ERR!\n", 5, 0) == -1 && errno != EAGAIN)
		syslog (LOG_WARNING,
			"can't connect to remote party (SSL error)");
	      close (csd[i].l);
	      return -1;
	    }

	  SSL_set_fd (csd[i].ssl, m);
	  ret = SSL_connect (csd[i].ssl);

	  switch ((SSL_get_error (csd[i].ssl, ret)))
	    {
	    case SSL_ERROR_WANT_READ:
	      csd[i].stat = STAT_WAITFORSSL;
	      return 0;
	    case SSL_ERROR_WANT_WRITE:
	      csd[i].stat = STAT_WAITFORSSL;
	      return 0;
	    default:
	      {
		close (csd[i].r);
		ret = SSL_get_error (csd[i].ssl, ret);
		SSLERROR (m, "SSL_connect", ret, csd[i].ssl);
		return -1;
	      }
	    }
	}
#endif

      csd[i].stat = (cf[csd[i].ref].lstn) ? STAT_KEYSEND : STAT_RELAY;
      if (csd[i].stat == STAT_KEYSEND)
	do_relay (m);
      return 0;
    }
  else if (csd[i].stat == STAT_KEYSEND)
    {
      char kbuf[64];
#ifdef USE_SSL
      int ssls = (cf[csd[i].ref].prot) ? 1 : 0;
#endif
      int ret1;

      snprintf (kbuf, sizeof (kbuf), "%i\n", csd[i].key_len);

#ifdef USE_SSL
      ret1 =
	(!ssls) ? send (csd[i].r, kbuf, strlen (kbuf),
			0) : SSL_write (csd[i].ssl, kbuf, csd[i].key_len);
#else
      ret1 = send (csd[i].r, kbuf, strlen (kbuf), 0);
#endif

      if (ret1 == -1)
	{
	  close (csd[i].r);
	  csd[i].l = 0;

#ifdef USE_SSL
	  if (ssls)
	    {
	      SSLERROR (m, "SSL_write",
			SSL_get_error (csd[i].ssl, ret1), csd[i].ssl);
	    }
	  else
	    {
	      INTERROR (m, "can't send to remote daemon", "send");
	    }

	  if (csd[i].ssl)
	    SSL_free (csd[i].ssl);

#else
	  INTERROR (m, "can't send to remote daemon", "send");
#endif

	}


#ifdef USE_SSL
      ret1 =
	(!ssls) ? send (csd[i].r, csd[i].keyb, csd[i].key_len,
			0) : SSL_write (csd[i].ssl, csd[i].keyb,
					csd[i].key_len);
#else
      ret1 = send (csd[i].r, csd[i].keyb, csd[i].key_len, 0);
#endif

      if (ret1 == -1)
	{
	  close (csd[i].r);
	  csd[i].l = 0;

#ifdef USE_SSL
	  if (csd[i].ssl)
	    SSL_free (csd[i].ssl);
#endif

	  INTERROR (m, "can't send key to remote daemon", "send");
	}
      csd[i].stat = STAT_RELAY;
    }
  else if (csd[i].stat == STAT_RELAY)
    {
      while (1)
	{
#ifdef USE_SSL
	  int sslr = 0, ssls = 0;

/* recv via SSL ? */
	  if ((m == csd[i].l && !cf[csd[i].ref].lstn && cf[csd[i].ref].prot)
	      || (m == csd[i].r && cf[csd[i].ref].lstn
		  && cf[csd[i].ref].prot)) sslr = 1;

/* send via SSL ? */
	  if (!sslr && cf[csd[i].ref].prot)
	    ssls = 1;

	  ret =
	    (!sslr) ? recv (m, gbuf, sizeof (gbuf), 0) : SSL_read (csd[i].ssl,
								   gbuf,
								   sizeof
								   (gbuf));
#else
	  ret = recv (m, gbuf, sizeof (gbuf), 0);
#endif

	  if (ret > 0)
	    {
	      int ret1;

#ifdef USE_SSL
	      ret1 =
		(!ssls) ? send ((m == csd[i].l) ? csd[i].r : csd[i].l, gbuf,
				ret, 0) : SSL_write (csd[i].ssl, gbuf, ret);
#else
	      ret1 =
		send ((m == csd[i].l) ? csd[i].r : csd[i].l, gbuf, ret, 0);
#endif

	      if (ret1 == -1)
		{
		  close ((m == csd[i].l) ? csd[i].r : csd[i].l);
		  csd[i].l = 0;

#ifdef USE_SSL
		  if (sslr)
		    {
		      SSLERROR (m, "SSL_write",
				SSL_get_error (csd[i].ssl, ret1), csd[i].ssl);
		    }
		  else
		    {
		      INTERROR (m, "can't send to client", "send");
		    }
#else
		  INTERROR (m, "can't send to client", "send");
#endif

		}
	    }
	  else
	    {
	      switch (ret)
		{
		case 0:
		  close (csd[i].r);
		  close (csd[i].l);
		  csd[i].l = 0;

#ifdef USE_SSL
		  if (csd[i].ssl)
		    SSL_free (csd[i].ssl);
#endif

		  break;
		case -1:
		  if (errno != EAGAIN
#ifdef USE_SSL
		      && SSL_get_error (csd[i].ssl,
					ret) != SSL_ERROR_WANT_READ
		      && SSL_get_error (csd[i].ssl,
					ret) != SSL_ERROR_WANT_WRITE
#endif
		    )
		    {
		      int d = (m == csd[i].l) ? csd[i].r : csd[i].l;
		      close ((m == csd[i].l) ? csd[i].l : csd[i].r);
		      csd[i].l = 0;

#ifdef USE_SSL
		      if (!sslr && m == csd[i].l && cf[csd[i].ref].lstn)
			{
			  SSLERROR (d, "SSL_read",
				    SSL_get_error (csd[i].ssl, ret),
				    csd[i].ssl);}
		      else
			{
			  INTERROR (d, "can't read from client", "recv");
			}
#else
		      INTERROR (d, "can't read from client", "recv");
#endif

		    }
		}
	    }
	  return 0;
	}
    }

  return 0;
}

int
set_new_csd (int s, int sd, int m)
{
  csd[s].l = sd;
  csd[s].r = 0;
  csd[s].key_len = 0;
  csd[s].offs = 0;
  csd[s].ref = find_ibysd (m, 0);
  csd[s].stat = STAT_WAITFORKEY;

#ifdef USE_SSL
  csd[s].ssl = NULL;
#endif

  bzero (csd[s].keyb, sizeof (csd[s].keyb));

  return 0;
}

// Accept clients on listening sockets

int
do_accept (int m)
{
  int sd, s;
#ifdef USE_SSL
  int ret;
#endif

  if ((sd = accept (m, NULL, NULL)) == -1)
    {
      syslog (LOG_WARNING, "connection dropped (accept(): %m)");
      return -1;
    }

  if (make_sock_nonblock (sd) == -1)
    {
      syslog (LOG_ERR, "can't make socket non-blocking (fcntl(): %m)");
      close (sd);
      return -1;
    }

  if ((s = find_free_slot ()) == -1)
    {
      if (send (sd, "Too many clients\n", 17, 0) == -1 && errno != EAGAIN)
	syslog (LOG_WARNING,
		"can't send message to remote client (send(): %m");
      close (sd);
      return -1;
    }

  set_new_csd (s, sd, m);

#ifdef USE_SSL
  if (!cf[csd[s].ref].lstn && cf[csd[s].ref].prot)
    {
      if ((csd[s].ssl = SSL_new (ctx)) == NULL)
	{
	  if (send (sd, "ERR!\n", 5, 0) == -1 && errno != EAGAIN)
	    syslog (LOG_WARNING,
		    "can't send message to remote client (send(): %m");
	  close (sd);
	  return -1;

	}
      SSL_set_fd (csd[s].ssl, sd);
      ret = SSL_accept (csd[s].ssl);

      switch ((SSL_get_error (csd[s].ssl, ret)))
	{
	case SSL_ERROR_WANT_READ:
	  csd[s].stat = STAT_WAITFORSSL;
	  return 0;
	case SSL_ERROR_WANT_WRITE:
	  csd[s].stat = STAT_WAITFORSSL;
	  return 0;
	default:
	  close (sd);
	  csd[s].l = 0;
	  SSL_free (csd[s].ssl);
	  syslog (LOG_ERR, "SSL error (%s(): %i)", "SSL_accept",
		  SSL_get_error (csd[s].ssl, ret));
	  return -1;
	}
    }
#endif

  if (cf[csd[s].ref].lstn)
    do_relay (sd);

  return 0;
}

char* suicide_reborn;
char* suicide_file;

void am_i_suicidal(void) {
  int i;
  if (!suicide_file) return;
  if (!access(suicide_file,F_OK)) {
    unlink(suicide_file);
    syslog (LOG_WARNING, "caught rehash request at %s (executing '%s')",suicide_file,suicide_reborn);
    for (i=2;i<256;i++) close(i);
    execl(suicide_reborn,suicide_reborn,CONFIG,suicide_file,0);
    perror("execl");
    syslog (LOG_ERR, "rehash failure!");
    exit(1);
  }
}

// Whole magic starts here ...

int
do_your_job ()
{
  int n, m, max = 0, ret;
  fd_set fds, wds;

  while (1)
    {
      FD_ZERO (&fds);
      FD_ZERO (&wds);

      am_i_suicidal();

      for (n = 0; n < ent; n++)
	{
	  FD_SET (cf[n].sd, &fds);
	  max = (cf[n].sd > max) ? cf[n].sd : max;
	}

      for (n = 0; n < MAX_CLIS; n++)
	if (csd[n].l)
	  {
	    FD_SET (csd[n].l, &fds);
	    max = (csd[n].l > max) ? csd[n].l : max;
	    if (csd[n].r)
	      {
		FD_SET (csd[n].r, &fds);
		max = (csd[n].r > max) ? csd[n].r : max;
	      }
	    if (csd[n].stat == STAT_WAITFORSSL)
	      {
		FD_SET (csd[n].l, &wds);
		max = (csd[n].l > max) ? csd[n].l : max;
	      }
	    if (csd[n].stat == STAT_CONNECTING)
	      {
		FD_SET (csd[n].r, &wds);
		max = (csd[n].r > max) ? csd[n].r : max;
	      }
	  }

      if ((ret = select (max + 1, &fds, &wds, NULL, NULL)) == -1)
	{
	  syslog (LOG_ERR, "fatal error (select(): %m)");
	  exit (1);
	}

      for (m = 0; m < max + 1 && ret > 0; m++)
	{
	  if (FD_ISSET (m, &fds) || FD_ISSET (m, &wds))
	    {
	      if (FD_ISSET (m, &fds) && FD_ISSET (m, &wds))
		{
		  ret -= 2;
		}
	      else
		{
		  ret--;
		}

	      switch (belongs_to (m))
		{
		case 0:	/* ... to cf[].sd - accepting */
		  do_accept (m);
		  break;
		case 1:	/* ... to csd[] - relaying */
		  do_relay (m);
		  break;
		default:
		  syslog (LOG_ERR, "fatal error (orphan socket)");
		  exit (1);
		}
	    }
	}
    }

  return 0;
}

#ifdef USE_SSL
int
init_ssl ()
{
  int ret;

  printf ("\nInitializing SSL ...\n");

  OpenSSL_add_all_algorithms ();

  SSL_load_error_strings ();

  meth = SSLv3_method ();
  CHK_NULL (meth);

  ctx = SSL_CTX_new (meth);
  CHK_NULL (ctx);

  ret = SSL_CTX_use_RSAPrivateKey_file (ctx, KEYFILE, SSL_FILETYPE_PEM);
  CHK_SSL (ret);

  ret = SSL_CTX_use_certificate_file (ctx, CERTFILE, SSL_FILETYPE_PEM);
  CHK_SSL (ret);

  printf ("SSL initialized\n");

  return 0;
}
#endif

int
main (int argc,char* argv[])
{


  suicide_reborn=argv[0];
  if (argc<2) {
    fprintf(stderr,"Usage: ripcd config_file [ suicide_file ]\n");
    exit(1);
  }

  CONFIG=argv[1];
  if (argc==3) suicide_file=argv[2];

  if (!(ent = parse_cfile (CONFIG)))
    {
      fprintf (stderr, "No entries loaded.\nQuitting ...\n");
      exit (1);
    }

  if (!(activate_ents ()))
    {
      fprintf (stderr, "No entries activated.\nQuitting ...\n");
      exit (1);
    }

//  daemonize (argv[0]);

  if (signal (SIGPIPE, SIG_IGN) == SIG_ERR)
    {
      perror ("signal");
      exit (1);
    }

  if (signal (SIGINT, int_handle) == SIG_ERR)
    {
      perror ("signal");
      exit (1);
    }

#ifdef USE_SSL
  init_ssl ();
#endif

  do_your_job ();
  return 0;
}


/* AF_LOCAL/SSL telnet ;) 

v0.2
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef USE_SSL
#include <openssl/ssl.h>

#define CERTFILE        "/var/state/openssl/cacert.pem"
#define KEYFILE         "/var/state/openssl/private/cakey.pem"

#define CHK_NULL(x) if ((x)==NULL) {fprintf(stderr,"SSL Error\n");ERR_print_errors_fp(stderr);exit (1);}
#define CHK_SSL(err) if ((err)==-1) { fprintf(stderr,"SSL Error\n");ERR_print_errors_fp(stderr); exit(1); }

SSL_METHOD *meth;
SSL_CTX *ctx;
SSL* ssl;
#endif

#ifdef USE_SSL
int
init_ssl (int i)
{
  int ret;

  OpenSSL_add_all_algorithms ();
  SSL_load_error_strings ();
  meth = SSLv3_method ();
  CHK_NULL (meth);
  ctx = SSL_CTX_new (meth);
  CHK_NULL (ctx);

  if (i) /* server */
    {
      ret = SSL_CTX_use_RSAPrivateKey_file (ctx, KEYFILE, SSL_FILETYPE_PEM);
      CHK_SSL (ret);

      ret = SSL_CTX_use_certificate_file (ctx, CERTFILE, SSL_FILETYPE_PEM);
      CHK_SSL (ret);
    }

  printf ("SSL initialized.\n");

  return 0;
}
#endif

main (int argc, char **argv)
{
  int sd, sd1, ret;
  struct sockaddr_un saddr;
  char buf[1024];
  fd_set fds;

  if (argc < 2)
    {
      printf ("%s [-l] addr\n", argv[0]);
      exit (1);
    }

#ifdef USE_SSL
  (strcmp (argv[1], "-l")) ? init_ssl (0) : init_ssl (1);
#endif

  sd = socket (AF_LOCAL, SOCK_STREAM, 0);

  saddr.sun_family = AF_LOCAL;

  if (!strcmp (argv[1], "-l"))
    {
      unlink (argv[2]);
      sd1 = sd;
      strncpy (saddr.sun_path, argv[2], sizeof (saddr.sun_path));
      if (bind (sd1, (struct sockaddr *) &saddr, sizeof (saddr)) == -1)
	{
	  perror ("bind");
	  exit (1);
	}

      printf ("Listening.\n");

      listen (sd1, 5);

      sd = accept (sd1, NULL, NULL);

#ifdef USE_SSL
      ssl = SSL_new (ctx);
      CHK_NULL (ssl);
      SSL_set_fd (ssl, sd);
      ret = SSL_accept (ssl);
      CHK_SSL (ret);
      printf ("SSL connection using %s\n", SSL_get_cipher (ssl));

#endif
    }
  else
    {
      strncpy (saddr.sun_path, argv[1], sizeof (saddr.sun_path));

      if (connect (sd, (struct sockaddr *) &saddr, sizeof (saddr)) == -1)
	{
	  perror ("connect");
	  exit (1);
	}

#ifdef USE_SSL
      ssl = SSL_new (ctx);
      CHK_NULL (ssl);
      SSL_set_fd (ssl, sd);
      ret = SSL_connect (ssl);
      CHK_SSL (ret);
      printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
#endif

      printf ("Connected.\n");
    }

  while (1)
    {
      FD_ZERO (&fds);

      FD_SET (0, &fds);
      FD_SET (sd, &fds);

      select (sd + 1, &fds, NULL, NULL, NULL);

      if (FD_ISSET (0, &fds))
	{
	  if (!fgets (buf, sizeof (buf), stdin))
	    exit (0);
	  send (sd, buf, strlen (buf), 0);
	}
      if (FD_ISSET (sd, &fds))
	{
	  if (!(ret = recv (sd, buf, sizeof (buf), 0)))
	    exit (1);
	  buf[ret] = 0;
	  printf ("%s", buf);
	}
    }
}

/*

   Argante virtual OS
   ------------------

   VCPU console connectivity

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <sys/un.h>

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

#ifndef PF_LOCAL
// Solaris
#define PF_LOCAL AF_UNIX
#endif

#ifndef SUN_LEN
// Solaris
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)         \
                      + strlen ((ptr)->sun_path))
#endif



int saved,s,a,looping;
struct termios nope, orig;
struct sockaddr_un x;

void termset(int i) {
  if (i) {
    tcgetattr(0,&nope);
    if (!saved) tcgetattr(0,&orig);
    saved=1;
    if ( nope.c_lflag & ECHO ) nope.c_lflag-=ECHO;
    if ( nope.c_lflag & ICANON ) nope.c_lflag-=ICANON;
    tcsetattr(0,TCSANOW,&nope);
  } else if (saved) tcsetattr(0,TCSANOW,&orig);
}


void sigexit(int x) {
  shutdown(s,2);
  close(s);
  termset(0);
  if (x) fprintf(stderr,"\nvcpucons: caught signal %d, exiting.\n",x);
  else fprintf(stderr,"\nvcpucons: exiting gracefully.\n");
  exit(0);
}

char buf[100000];
char sname[64];

int main(int argc,char* argv[]) {
  if (argc<2) {
    fprintf(stderr,"\nvcpcons - Argante OS virtual console connectivity (lcamtuf@ids.pl)\n");
    fprintf(stderr,"usage: %s [ -l ] /path/to/sockets/vcpuid-sockid\n",argv[0]);
    exit(1);
  }
  signal(SIGINT,sigexit); 
  signal(SIGTERM,sigexit); signal(SIGPIPE,sigexit);
  s=socket(PF_UNIX,SOCK_STREAM,0);
  x.sun_family=AF_UNIX;
  if (!strcmp(argv[1],"-l")) { looping=1; strncpy(sname,argv[2],60); }
    else { strncpy(sname,argv[1],60); }

  termset(1);
  strcpy(x.sun_path,sname);

reconn:

  fprintf(stderr,"vcpucons: connecting to %s...\n",sname);
  if (connect(s,&x,SUN_LEN(&x))) { 
    perror(sname); 
    if (looping) {
      sleep(1);
      goto reconn;
    }
    sigexit(0); 
  }
  fprintf(stderr,"vcpucons: connected successfully.\n");
  write(1,"\033[2J\033[0H",8);  // clear the screen... hmm :)
  fcntl(0,F_SETFL,O_NONBLOCK);
  fcntl(s,F_SETFL,O_NONBLOCK);
  while (1) {

    usleep(10000);

fastback:

    // Read from console, send to socket...

    a=read(0,buf,100000);
    if (a<0 && errno!=EAGAIN) {
      sigexit(0);
    }

    if (a>0) { 
      fcntl(s,F_SETFL,0); // Be blocking here.
      a=send(s,buf,a,MSG_NOSIGNAL);  
      fcntl(s,F_SETFL,O_NONBLOCK);
      if (a<=0) {
        fprintf(stderr,"\nvcpucons: send() failure, connection closed.\n");
        if (looping) goto reconn;
        sigexit(0);
      }
    }

    // Read from socket, send to console...

    a=recv(s,buf,100000,MSG_NOSIGNAL);
    if (a<0 && errno!=EAGAIN) {
      fprintf(stderr,"\nvcpucons: recv() failure, connection closed.\n");
      if (looping) goto reconn;
      sigexit(0);
    }

    if (a<=0) goto fastback;

    if (a>0) { 
      fcntl(0,F_SETFL,0); // Be blocking here.
      a=write(1,buf,a);  
      fcntl(0,F_SETFL,O_NONBLOCK);
      if (a<=0) {
        sigexit(0);
      }
      goto fastback;
    }
  }
}

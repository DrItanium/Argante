/*

   Argante virtual OS
   ------------------

   Argante command-line -> VCPU execution

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

int vcpuid=-1;
int processpid=-1;
int verbose=0;
int doconsole=0;
int waitproc=1;

FILE* ain;
FILE* aout;

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

int pozz;
char oname[100];

void exec_process(char* what) {
  char buf[100];
  char b2[1024];
  char b3[1024];
  int rulez=0,cnter=0;
  sprintf(buf,"/proc/%d",processpid);
  if (chdir(buf)) {
    if (verbose) fprintf(stderr,"agtexe: couldn't chdir to %s.\n",buf);
    exit(4);
  }
  sprintf(buf,"/proc/%d/fd/0",processpid);
  ain=fopen(buf,"a");
  sprintf(buf,"/var/argante/ses-%d.log",processpid-2);
  strcpy(oname,buf);
  aout=fopen(buf,"r");
  if (!ain || !aout) {
    if (verbose) fprintf(stderr,"agtexe: couldn't attach to AOS fds.\n");
    exit(4);
  }
  fseek(aout,0,SEEK_END);
  pozz=ftell(aout);
  if (verbose) fprintf(stderr,"agtexe: executing %s...\n",what);
  fprintf(ain,"$%s\n",what);
  fflush(ain);

loopa:

  fclose(aout);
  aout=fopen(buf,"r");
  fseek(aout,pozz,SEEK_SET);

  while (fgets(b2,1024,aout)) {
    sscanf(b2,"+> Free VCPU #%d, binary image from: %s",&vcpuid,b3);
    if (!strncmp(b3,what,strlen(what))) rulez=1;
    if (!rulez) continue;
    if (!strncmp(b2,"+> Process launched successfully",30)) {
      if (verbose) fprintf(stderr,"agtexe: loaded successfully, vcpu #%d...\n",vcpuid);
      return;
      pozz=ftell(aout);
      fclose(aout);
    }
  }
  if (!rulez) { usleep(1000); if (++cnter<50) goto loopa; }

  if (!rulez) {
    if (verbose)
      fprintf(stderr,"agtexe: strangely, our request was ignored.\n");
    exit(2);
  }

  if (verbose)
    fprintf(stderr,"agtexe: load failure, use agtses for debugging.\n");

  exit(2);
}


void check_process(void) {
  char buf[1000];
  int id,num;
  char sth[100];
  aout=fopen(oname,"r");
  fseek(aout,pozz,SEEK_SET);
  while (fgets(buf,1000,aout)) { 
    if (sscanf(buf,"-> VCPU #%d (%*[A-z._-/]): Unhandled non-fatal exception %d",&id,&num)==2) {
      if (id==vcpuid) {
        if (verbose) fprintf(stderr,"\nagtexe: process killed (exception %d)\n",num);
        termset(0);
        exit(3);
      }
    }
    if (sscanf(buf,"+> VCPU #%d (%*[A-z._-/]) %s\n",&id,sth)>1) {
      if (strstr(sth,"killed") || strstr(sth,"terminated"))
        if (id==vcpuid) {
          if (verbose) fprintf(stderr,"\nagtexe: process terminated.\n");
          termset(0);
          exit(0);
        }
    }
  } 
  pozz=ftell(aout);
  fclose(aout);
}



void sigexit(int x) {
  shutdown(s,2);
  close(s);
  termset(0);
  if (x) {
   if (verbose) fprintf(stderr,"\nagtexe: caught signal %d, exiting.\n",x);
  } else if (verbose) fprintf(stderr,"\nagtexe: exiting gracefully.\n");
  if (x) exit(1);
  exit(0);
}

char buf[100000];
char sname[64];

int main(int argc,char* argv[]) {

  if (argc!=4) {
    fprintf(stderr,"\nagtexe - Argante OS command-line -> VCPU execution (lcamtuf@ids.pl)\n");
    fprintf(stderr,"usage: %s /path/to/binary f[cwm] pid\n",argv[0]);
    exit(100);
  }

  signal(SIGINT,sigexit); 
  signal(SIGTERM,sigexit); 
  signal(SIGHUP,sigexit); 
  signal(SIGPIPE,sigexit);
  processpid=atoi(argv[3]);

  waitproc=!(strchr(argv[2],'w'));
  doconsole=!(strchr(argv[2],'c'));
  verbose=!(strchr(argv[2],'m'));

  exec_process(argv[1]);

  if (!waitproc) {
    if (verbose) fprintf(stderr,"Process launched in background, exiting...\n");
    exit(0);
  }

nonetwork:

  if (doconsole) {

    s=socket(PF_UNIX,SOCK_STREAM,0);
    x.sun_family=AF_UNIX;
    sprintf(sname,"/proc/%d/cwd/fs/unix-sock/%d-100",processpid,vcpuid);
    termset(1);
    strcpy(x.sun_path,sname);
  
    reconn:
    check_process();

    if (connect(s,&x,SUN_LEN(&x))) { usleep(10000); goto reconn; }

    if (verbose) fprintf(stderr,"agtexe: entering console session.\n\n");
    fcntl(0,F_SETFL,O_NONBLOCK);
    fcntl(s,F_SETFL,O_NONBLOCK);

    while (1) {

      usleep(10000);
      check_process();

    fastback:

      a=read(0,buf,100000);
      if (a<0 && errno!=EAGAIN) { sigexit(0); }
  
      if (a>0) { 
        fcntl(s,F_SETFL,0);
        a=send(s,buf,a,MSG_NOSIGNAL);  
        fcntl(s,F_SETFL,O_NONBLOCK);
        if (a<=0) {
          if (verbose) fprintf(stderr,"\nagtexe: console session terminated.\n");
          termset(0);
          goto nonetwork;
        }
      }
  
      a=recv(s,buf,100000,MSG_NOSIGNAL);
      if (a<0 && errno!=EAGAIN) {
        if (verbose) fprintf(stderr,"\nagtexe: console session terminated.\n");
        termset(0);
        goto nonetwork;
      }

      if (a<=0) goto fastback;

      if (a>0) { 
        fcntl(0,F_SETFL,0); // Be blocking here.
        a=write(1,buf,a);  
        fcntl(0,F_SETFL,O_NONBLOCK);
        if (a<=0) { sigexit(0); }
        goto fastback;
      }
    }
  }

  while (1) { usleep(10000); check_process(); }

  return 0;

}

/*

   Argante virtual OS
   ------------------

   Filesystem access routines.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>

#include "config.h"
#include "task.h"
#include "bcode.h"
#include "module.h"
#include "memory.h"
#include "console.h"
#include "syscall.h"
#include "acman.h"

#define FS_FLAG_USED		1
#define FS_FLAG_READ		2
#define FS_FLAG_WRITE		4
#define FS_FLAG_APPEND		8
#define FS_FLAG_NONBLOCK	16

#define MASK 			(FS_FLAG_READ|FS_FLAG_WRITE|FS_FLAG_APPEND)

#define MAX_DIRENT MAX_DIRENTS // mistype

char tmppath[MAX_FS_PATH+10];
char pbuf[MAX_FS_PATH+10];
char pelem[MAX_FS_PATH+10];

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


struct path_conv_entry {
  char real[MAX_FS_PATH];
  char virt[MAX_FS_PATH];
};


struct process_fs_entry fsdata[MAX_VCPUS];
struct path_conv_entry  fsconv[MAX_FS_CONV];

int top_fsconv=0;

// Do strong path validation. Return: ptr to path buffer or
// null on failure (exception already reported).


char* verify_and_clean(int c,char* p,int cnt) {
  char sav;
  char *next=p,*cur;
  int l;
  sav=p[cnt];
  p[cnt]=0;

  for (l=0;l<strlen(p);l++) { 
    if (!isprint(p[l])) {
      p[cnt]=sav;
      non_fatal(ERROR_FS_BAD_PATH,"non-printable path",c);
      failure=1;
      return 0;
    }
    if (p[l]=='*') {
      p[cnt]=sav;
      non_fatal(ERROR_FS_BAD_PATH,"wildcard in path",c);
      failure=1;
      return 0;
    }
  }

  pbuf[0]=0;
  pbuf[MAX_FS_PATH+3]=0;
  if (*p!='/') {
    if (fsdata[c].cwd[0])
      strncat(pbuf,fsdata[c].cwd,MAX_FS_PATH+5); else {
        p[cnt]=sav;
        non_fatal(ERROR_FS_BAD_PATH,"relative path but no cwd",c);
        failure=1;
        return 0;
      }
  }
  while (next && *next) {
    cur=next;
    next=strchr(cur+1,'/');
    if (!next) next=&p[strlen(p)];
    if (next-cur>MAX_FS_PATH) {
      p[cnt]=sav;
      non_fatal(ERROR_FS_BAD_PATH,"fs path element too long",c);
      failure=1;
      return 0;
    }
    if (*cur=='/') cur++;
    strncpy(pelem,cur,next-cur);
    pelem[next-cur]=0;
    if (!strlen(pelem)) continue;
    if (!strcmp(pelem,".")) continue;
    if (!strcmp(pelem,"..")) {
      char* n;
      n=strrchr(pbuf,'/');
      if (n) {
        *n=0;
        if (!(*(n+1))) { n=strrchr(pbuf,'/'); if (n) *n=0; }
      }
      continue; 
    }

    strcat(pbuf,"/");
    strncat(pbuf,pelem,MAX_FS_PATH+5);

    if (pbuf[MAX_FS_PATH+3]) {
      p[cnt]=sav;
      non_fatal(ERROR_FS_BAD_PATH,"fs path too long",c);
      failure=1;
      return 0;
    }
  }
  p[cnt]=sav;
  if (pbuf[0]=='/') return &pbuf[1];
  return pbuf;
}


char rpath[MAX_FS_PATH*2+10];
char wpath[MAX_FS_PATH+10];


char* get_realpath(int c,char* x) {
  int i;
  wpath[MAX_FS_PATH+8]=0;

  if (*x=='/') x++;
  strncpy(wpath,x,MAX_FS_PATH+5);

  if (wpath[MAX_FS_PATH+3]) {
    non_fatal(ERROR_FS_BAD_PATH,"fs path too long",c);
    failure=1;
    return 0;
  }

  strcat(wpath,"/");

  for (i=0;i<top_fsconv;i++) {
    if (strlen(wpath)<strlen(fsconv[i].virt)) continue;
    if (strncmp(wpath,fsconv[i].virt,strlen(fsconv[i].virt))) continue;

    strcpy(rpath,fsconv[i].real);
    strcat(rpath,"/");
    strcat(rpath,&wpath[strlen(fsconv[i].virt)]);
    rpath[strlen(rpath)-1]=0;

    return rpath;

  }

  non_fatal(ERROR_NOOBJECT,"object does not exist",c);
  failure=1;

  return 0;

};


void load_ctables(void) {
  FILE* f;
  char* x;
  int i;
  char buf[1000];
  f=fopen(FSCONV,"r");
  if (!f) {
    printk("-> ERROR: Cannot open " FSCONV " fsconv file.\n");
    return;
  }
  printk("=> Loading fs path conversion rules (" FSCONV ")...\n");

  printk("+> Memory usage: %d bytes, %d entries.\n",sizeof(fsconv),MAX_FS_CONV);

  top_fsconv=0;

  while (fgets(buf,1000,f)) {
    if ((x=strrchr(buf,'#'))) *x=0;
    if ((x=strrchr(buf,'\n'))) *x=0;
    if (!strlen(buf)) continue;
    while (isspace(buf[strlen(buf)-1])) buf[strlen(buf)-1]=0;
    if (!strlen(buf)) continue;
    for (i=0;i<strlen(buf);i++) buf[i]=tolower(buf[i]);
    if (sscanf(buf,"%s %s",fsconv[top_fsconv].virt,fsconv[top_fsconv].real)==2)
      top_fsconv++;
    if (top_fsconv>=MAX_FS_CONV) {
      printk("-> ERROR: no space left for rulesets, aborting load.\n");
      fclose(f); 
      return;
    }
  }
  if (top_fsconv)
    printk("=> fs: loaded %d conversion rules.\n",top_fsconv);
    else printk("=> fs: warning - loaded empty fsconv set.\n");
  fclose(f);

}


void syscall_load(int* x) {

  // fs/open/file/{write|read|append}
  // write/append: blocking, non-blocking
  *(x++)=SYSCALL_FS_OPEN_FILE;
  // fs/fops/create/file
  *(x++)=SYSCALL_FS_CREATE_FILE;
  // fs/fops/write/file
  *(x++)=SYSCALL_FS_WRITE_FILE;
  // fs/fops/read/file
  *(x++)=SYSCALL_FS_READ_FILE;
  // fs/fops/seek/file
  *(x++)=SYSCALL_FS_SEEK_FILE;
  // fs/fops/close/file
  *(x++)=SYSCALL_FS_CLOSE_FILE;
  // fs/fops/read/directory
  *(x++)=SYSCALL_FS_LIST_DIR;
  // fs/stat/{file|directory}
  *(x++)=SYSCALL_FS_STAT;
  // fs/fops/create/directory
  *(x++)=SYSCALL_FS_MAKE_DIR;
  // fs/fops/rename/{file|directory}
  *(x++)=SYSCALL_FS_RENAME;
  // fs/fops/delete/{file|directory}
  *(x++)=SYSCALL_FS_DELETE;
  // fs/setup/error_mode
  *(x++)=SYSCALL_FS_ERRORMODE;
  // fs/wdops/cwd
  *(x++)=SYSCALL_FS_CWD;
  // fs/wdops/pwd
  *(x++)=SYSCALL_FS_PWD;
  *(x++)=SYSCALL_FS_END_DIR;

  // Go away :)
  *(x)=SYSCALL_ENDLIST;

  printk(">> Filesystem module is starting...\n");
  printk("+> Open files per VCPU limit: %d, maxpath %d.\n",MAX_FS_FD,MAX_FS_PATH);
  load_ctables();
}


int wait_for_write_access(int c) {
  int fin;
  int h,ll;
  char* fn;
  int mode,st_ino,st_dev;
  fin=cpu[c].iowait_id;
  mode=fsdata[c].flag[fin];
  st_ino=fsdata[c].ino[fin];
  st_dev=fsdata[c].dev[fin];
  fn=fsdata[c].waitfile;

  for (h=0;h<MAX_VCPUS;h++)
    for (ll=0;ll<MAX_FS_FD;ll++) 
      if (fsdata[h].flag[ll] & FS_FLAG_USED)
        if (fsdata[h].flag[ll] & (FS_FLAG_APPEND|FS_FLAG_WRITE))
          if ((fsdata[h].ino[ll]==st_ino) && (fsdata[h].dev[ll]==st_dev)) {
            if (cpu[h].state & VCPU_STATE_IOWAIT) continue;
            return 0;
          }

  if (mode & FS_FLAG_READ) fsdata[c].fd[fin]=open(fn,O_RDONLY); else
  if (mode & FS_FLAG_APPEND) fsdata[c].fd[fin]=open(fn,O_WRONLY|O_APPEND); else
  if (mode & FS_FLAG_WRITE) fsdata[c].fd[fin]=open(fn,O_WRONLY|O_TRUNC);

  if (fsdata[c].fd[fin]<0) {
    fsdata[c].flag[fin]=0;
    non_fatal(ERROR_FS_OPEN_ERROR,"open() failed",c);
    failure=1;
    return 0;
  }

  printk("+> fs: VCPU #%d running again, write access granted.\n",c);

  cpu[c].sregs[0]=fin;

  return 1;
}

int buildcache_cpu;
int buildfail=0;
int broken_scandir=0;

void broken_implementation(void) {
  if (!broken_scandir)
    printk("-> WARNING: YOUR scandir() IMPLEMENTATION IS BROKEN!\n");
  broken_scandir=1;
}


int buildcache(const struct dirent* x) {
  int c=buildcache_cpu;
//  struct stat b;
  int d;
  if (!strcmp(x->d_name,"..")) return 0;
  if (!strcmp(x->d_name,".")) return 0;
  d=fsdata[c].ders;
  if (d+1>=MAX_DIRENT) {
    if (!buildfail)
      printk("-> fs warning: VCPU %d listing too long directory.\n",c);
    buildfail=1;
    return 0;
  }
  fsdata[c].ders++;
  buildfail=0;
  fsdata[c].der[d]=malloc(sizeof(struct der_s));
  strncpy(fsdata[c].der[d]->name,x->d_name,256); // Just-in-case, shouldn't 
                                                // happen.
// Won't be filled for now - fix it!
//  fsdata[c].der[d]->type=
  return 0;
}



void syscall_handler(int c,int nr) {
  char* oper=0,*oper2=0,*fiction;
  int fin,got,l;
  int mode;
  char *sth,*sth2,*fn,*fn2,*fiction2;
  struct stat b;

  failure=0;

  switch (nr) {

    case SYSCALL_FS_OPEN_FILE:

    // Input:  u0 - name addr, u1 - name len
    //         u2 - mode
    // Return: s0 - VFD handle

      if ((cpu[c].uregs[2] & MASK) == FS_FLAG_READ) oper="fs/fops/open/file/read"; else
      if ((cpu[c].uregs[2] & MASK) == FS_FLAG_WRITE) oper="fs/fops/open/file/write"; else
      if ((cpu[c].uregs[2] & MASK) == FS_FLAG_APPEND) oper="fs/fops/open/file/write/append";

      mode=FS_FLAG_USED+(cpu[c].uregs[2] & MASK);

      if (!oper) {
        non_fatal(ERROR_FS_BAD_OPEN_MODE,"invalid OPEN_FILE mode",c);
        failure=1;
        return;
      }

      if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_READ))) {
        non_fatal(ERROR_PROTFAULT,"open: Attempt to access protected memory",c);
        failure=1;
        return;
      }

      fn=verify_and_clean(c,sth,cpu[c].uregs[1]);
      check_status();

      VALIDATE(c,fn,oper);

      fn=get_realpath(c,fn);
      check_status();
      strcpy(fsdata[c].waitfile,fn);

      if (stat(fn,&b)) {
        non_fatal(ERROR_FS_OPEN_ERROR,"Cannot open requested file",c);
        failure=1;
        return;
      }

      if (!S_ISREG(b.st_mode)) {
        non_fatal(ERROR_FS_NOFILE,"cannot open object as a file",c);
        failure=1;
        return;
      }

      got=0;
      for (fin=0;fin<MAX_FS_FD;fin++) {
        if (!fsdata[c].flag[fin]) { got=1; break; } 
      }
     
      if (!got) {
        non_fatal(ERROR_FSERROR,"No free v-descriptors",c);
        failure=1;
        return;
      }

      fsdata[c].ino[fin]=b.st_ino;
      fsdata[c].dev[fin]=b.st_dev;

      if (cpu[c].uregs[2] & (FS_FLAG_APPEND|FS_FLAG_WRITE)) {
        int h,ll;
        for (h=0;h<MAX_VCPUS;h++)
          for (ll=0;ll<MAX_FS_FD;ll++) 
            if (fsdata[h].flag[ll] & FS_FLAG_USED)
              if (fsdata[h].flag[ll] & (FS_FLAG_APPEND|FS_FLAG_WRITE))
                if ((fsdata[h].ino[ll]==b.st_ino) && (fsdata[h].dev[ll]==b.st_dev)) {

                  if (cpu[h].state & VCPU_STATE_IOWAIT) continue;

                  // Locking needed, dude.
                  if (!(cpu[c].uregs[2] & FS_FLAG_NONBLOCK)) {
                    fsdata[c].flag[fin]=mode;

                    printk("=> fs: VCPU #%d (%s) entering IOWAIT state (blocking write access).\n",c,cpu[c].name);
                    printk("=> fs: filename=%s [locked by #%d]\n",fn,h);

                    cpu[c].state|=VCPU_STATE_IOWAIT;
                    cpu[c].iohandler=wait_for_write_access;
                    cpu[c].iowait_id=fin;
                    return;
                  }

                  cpu[c].sregs[0]=-1;
                  return;
                }
      }

      if (mode & FS_FLAG_READ) fsdata[c].fd[fin]=open(fn,O_RDONLY); else
      if (mode & FS_FLAG_APPEND) fsdata[c].fd[fin]=open(fn,O_WRONLY|O_APPEND); else
      if (mode & FS_FLAG_WRITE) fsdata[c].fd[fin]=open(fn,O_WRONLY|O_TRUNC);

      if (fsdata[c].fd[fin]<0) {
         non_fatal(ERROR_FS_OPEN_ERROR,"open() failed",c);
         failure=1;
         return;
      }

      cpu[c].sregs[0]=fin;
      fsdata[c].flag[fin]=mode;

      break;


    case SYSCALL_FS_CREATE_FILE:

    // Input:  u0 - name addr, u1 - name len
    //         u2 - mode
    // Return: s0 - VFD handle

      if (cpu[c].uregs[2]==FS_FLAG_WRITE) 
        oper="fs/fops/create/file/write"; else
        oper="fs/fops/create/file/append";

      if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_READ))) {
        non_fatal(ERROR_PROTFAULT,"create: Attempt to access protected memory",c);
        failure=1;
        return;
      }

      fn=verify_and_clean(c,sth,cpu[c].uregs[1]);
      check_status();

      VALIDATE(c,fn,oper);

      fn=get_realpath(c,fn);
      check_status();
      strcpy(fsdata[c].waitfile,fn);

      got=0;
      for (fin=0;fin<MAX_FS_FD;fin++) {
        if (!fsdata[c].flag[fin]) { got=1; break; } 
      }
     
      if (!got) {
        non_fatal(ERROR_FSERROR,"No free v-descriptors",c);
        failure=1;
        return;
      }

      if (cpu[c].uregs[2]==FS_FLAG_WRITE) 
        fsdata[c].fd[fin]=open(fn,O_WRONLY|O_CREAT|O_EXCL,0600);
      else fsdata[c].fd[fin]=open(fn,O_WRONLY|O_CREAT|O_APPEND|O_EXCL,0600);

      if (fsdata[c].fd[fin]<0) {
        non_fatal(ERROR_FS_CREATE_ERROR,"Cannot create requested file",c);
        failure=1;
        return;
      }

      // Oh stop kidding! Couldn't fail ;)
      fstat(fsdata[c].fd[fin],&b);

      fsdata[c].ino[fin]=b.st_ino;
      fsdata[c].dev[fin]=b.st_dev;

      if (cpu[c].uregs[2]==FS_FLAG_WRITE) fsdata[c].flag[fin]=FS_FLAG_WRITE;
        else fsdata[c].flag[fin]=FS_FLAG_APPEND;

      cpu[c].sregs[0]=fin;

      break;


    case SYSCALL_FS_CLOSE_FILE:

      // Input:  u0 - VFD handle

      if (cpu[c].uregs[0]>=MAX_FS_FD) {
        non_fatal(ERROR_FS_BAD_VFD,"close: VFD number too big",c);
        failure=1;
        return;
      }
    
      if (fsdata[c].flag[cpu[c].uregs[0]] & FS_FLAG_USED) {
        if (fsdata[c].flag[cpu[c].uregs[0]] & FS_FLAG_WRITE) 
          ftruncate(fsdata[c].fd[cpu[c].uregs[0]],
                    lseek(fsdata[c].fd[cpu[c].uregs[0]],0,SEEK_CUR));
        close(fsdata[c].fd[cpu[c].uregs[0]]);
        fsdata[c].flag[cpu[c].uregs[0]]=0;
      } else {
        non_fatal(ERROR_FS_BAD_VFD,"close: VFD is not open",c);
        failure=1;
        return;
      }

      break;


    case SYSCALL_FS_WRITE_FILE:

    // Input:  u0 - VFD handle
    //	       u1 - mem addr, u2 - mem size

      if (cpu[c].uregs[0]>=MAX_FS_FD) {
        non_fatal(ERROR_FS_BAD_VFD,"write: VFD number too big",c);
        failure=1;
        return;
      }
    
      if (fsdata[c].flag[cpu[c].uregs[0]] & FS_FLAG_USED) {
        char* mem;
        mem=verify_access(c,cpu[c].uregs[1],(cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);
        if (!mem) {
          non_fatal(ERROR_PROTFAULT,"write: Attempt to access protected memory",c);
          failure=1;
          return;
        }
        write(fsdata[c].fd[cpu[c].uregs[0]],mem,cpu[c].uregs[2]);
      } else {
        non_fatal(ERROR_FS_BAD_VFD,"write: VFD is not open",c);
        failure=1;
        return;
      }

      break;


    case SYSCALL_FS_SEEK_FILE:

    // Input:  u0 - VFD handle
    //	       u1 - offset, u2 - whence

      if (cpu[c].uregs[0]>=MAX_FS_FD) {
        non_fatal(ERROR_FS_BAD_VFD,"seek: VFD number too big",c);
        failure=1;
        return;
      }
    
      if (!(fsdata[c].flag[cpu[c].uregs[0]] & FS_FLAG_USED)) {
        non_fatal(ERROR_FS_BAD_VFD,"seek: invalid VFD",c);
        failure=1;
        return;
      }


      if ((fsdata[c].flag[cpu[c].uregs[0]] & FS_FLAG_APPEND)) {
        if ((cpu[c].uregs[1]==0) && (cpu[c].uregs[2]==1)) goto lilly;
        non_fatal(ERROR_FS_NOSEEK,"seek: VFD append-only",c);
        failure=1;
        return;
      }

lilly:

      cpu[c].sregs[0]=lseek(fsdata[c].fd[cpu[c].uregs[0]],cpu[c].uregs[1],cpu[c].uregs[2]);

      break;



    case SYSCALL_FS_READ_FILE:

    // Input:  u0 - VFD handle
    //	       u1 - mem addr, u2 - mem size

      if (cpu[c].uregs[0]>=MAX_FS_FD) {
        non_fatal(ERROR_FS_BAD_VFD,"read: VFD number too big",c);
        failure=1;
        return;
      }

    
      if (fsdata[c].flag[cpu[c].uregs[0]] & FS_FLAG_USED) {
        char* mem;
        mem=verify_access(c,cpu[c].uregs[1],(cpu[c].uregs[2]+3)/4,MEM_FLAG_WRITE);
        if (!mem) {
          non_fatal(ERROR_PROTFAULT,"read: Attempt to access protected memory",c);
          failure=1;
          return;
        }
        cpu[c].sregs[0]=read(fsdata[c].fd[cpu[c].uregs[0]],mem,cpu[c].uregs[2]);
        if (cpu[c].sregs[0]<0) {
          non_fatal(ERROR_DEADLOCK,"read: negative read() result",c);
          failure=1;
          break;
        }
      } else {
        non_fatal(ERROR_FS_BAD_VFD,"read: VFD is not open",c);
        failure=1;
        return;
      }

      break;


    case SYSCALL_FS_MAKE_DIR:

    // Input:  u0 - name addr, u1 - name len

    oper="fs/fops/create/directory";

    if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_READ))) {
      non_fatal(ERROR_PROTFAULT,"mkdir: Attempt to access protected memory",c);
      failure=1;
      return;
    }

    fn=verify_and_clean(c,sth,cpu[c].uregs[1]);
    check_status();

    VALIDATE(c,fn,oper);

    fn=get_realpath(c,fn);
    check_status();

    if (mkdir(fn,0700)) {
      non_fatal(ERROR_FS_EXISTS,"mkdir: cannot create directory",c);
      failure=1;
      return;
    }

    break;

    case SYSCALL_FS_DELETE:

    // Input:  u0 - name addr, u1 - name len

    if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_READ))) {
      non_fatal(ERROR_PROTFAULT,"delete: Attempt to access protected memory",c);
      failure=1;
      return;
    }

    fn=verify_and_clean(c,sth,cpu[c].uregs[1]);
    check_status();

    fiction=fn;
    fn=get_realpath(c,fn);
    check_status();

    if (stat(fn,&b)) {
      non_fatal(ERROR_FS_NOFILE,"delete: cannot access object",c);
      failure=1;
      return;
    }

    if (S_ISDIR(b.st_mode)) oper="fs/fops/delete/directory";
      else oper="fs/fops/delete/file";

    VALIDATE(c,fiction,oper);

    if (unlink(fn)) {
      non_fatal(ERROR_FSERROR,"delete: cannot unlink object?",c);
      failure=1;
      return;
    }

    break;


    case SYSCALL_FS_RENAME:

    // Input:  u0 - name addr, u1 - name len
    //         u2 - dst name addr, u3 - dst name len

    if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_READ))) {
      non_fatal(ERROR_PROTFAULT,"rename[1]: Attempt to access protected memory",c);
      failure=1;
      return;
    }

    if (!(sth2=verify_access(c,cpu[c].uregs[2],(cpu[c].uregs[3]+3)/4,MEM_FLAG_READ))) {
      non_fatal(ERROR_PROTFAULT,"rename[2]: Attempt to access protected memory",c);
      failure=1;
      return;
    }

    fn=verify_and_clean(c,sth,cpu[c].uregs[1]);
    check_status();

    fn2=verify_and_clean(c,sth2,cpu[c].uregs[3]);
    check_status();

    fiction=fn;
    fiction2=fn2;

    fn=get_realpath(c,fn);
    check_status();
    fn2=get_realpath(c,fn2);
    check_status();

    if (stat(fn,&b)) {
      non_fatal(ERROR_FS_NOFILE,"rename: cannot access source object",c);
      failure=1;
      return;
    }

    if (S_ISDIR(b.st_mode)) {
      oper="fs/fops/delete/directory";
      oper2="fs/fops/create/directory";
    } else {
      oper="fs/fops/delete/file";
      oper="fs/fops/create/file";
    }

    if (!stat(fn2,&b)) {
      non_fatal(ERROR_FS_EXISTS,"rename: destination object exists",c);
      failure=1;
      return;
    }

    VALIDATE(c,fiction,oper);
    VALIDATE(c,fiction2,oper2);

    if (rename(fn,fn2)) {
      non_fatal(ERROR_FSERROR,"rename: cannot rename object?",c);
      failure=1;
      return;
    }

    break;


    case SYSCALL_FS_PWD:

    // Input:  u0 - mem addr, u1 - mem size
    // Output: s0 - number of characters stored.

        sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_WRITE);
        if (!sth) {
          non_fatal(ERROR_PROTFAULT,"pwd: Attempt to access protected memory",c);
          failure=1;
          return;
        }

        if (strlen(fsdata[c].cwd)>cpu[c].uregs[1]) {
          non_fatal(ERROR_FS_BAD_PATH,"pwd: current working directory longer than buffer",c);
          failure=1;
          return;
        }

        if (fsdata[c].cwd[0])
          memcpy(sth,fsdata[c].cwd,strlen(fsdata[c].cwd));

        cpu[c].sregs[0]=strlen(fsdata[c].cwd);

      break;

    case SYSCALL_FS_CWD:

      // Input:  u0 - mem addr, u1 - mem size

      sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_WRITE);

      if (!sth) {
        non_fatal(ERROR_PROTFAULT,"pwd: Attempt to access protected memory",c);
        failure=1;
        return;
      }

      fn=verify_and_clean(c,sth,cpu[c].uregs[1]);
      check_status();

      memcpy(fsdata[c].cwd,fn,strlen(fsdata[c].cwd));
      fsdata[c].cwd[strlen(fsdata[c].cwd)]=0;

      break;

  // Stat information:
  // Input: u0, u1 - string
  //
  // u0 - last modification time
  // u1 - 0 = non-accessible, 1 = file, 2 = directory 

    case SYSCALL_FS_STAT:

    if (!(sth=verify_access(c,cpu[c].uregs[0],(cpu[c].uregs[1]+3)/4,MEM_FLAG_READ))) {
      non_fatal(ERROR_PROTFAULT,"stat: Attempt to access protected memory",c);
      failure=1;
      return;
    }

    fn=verify_and_clean(c,sth,cpu[c].uregs[1]);
    check_status();

    VALIDATE(c,fn,"fs/fops/stat");

    fiction=fn;
    fn=get_realpath(c,fn);
    check_status();

    if (stat(fn,&b)) {
      cpu[c].uregs[1]=0;
      return;
    }

    cpu[c].uregs[2]=b.st_size;

    if (S_ISDIR(b.st_mode)) cpu[c].uregs[1]=2; else cpu[c].uregs[1]=1;
    cpu[c].uregs[0]=b.st_mtime;

    break;

    case SYSCALL_FS_END_DIR:

      for (l=0;l<fsdata[c].ders;l++)
        if (fsdata[c].der[l]) free(fsdata[c].der[l]); else break;
      fsdata[c].ders=0;
      
      break;

    case SYSCALL_FS_LIST_DIR:

      // u0 != 0 - start new session, u0 == 0 - continue session
      // u0 != 0 --> u1 - dirname ptr, u2 - dirname len
      // u3 - offset
      // Returns:
      // u0 == 0:
      //   u2 - real length
      //   s0 - entries left
      //   s1 - 0 == file, 1 == directory
      // u0 != 0 -- s0 - number of entries cached
      // ERROR_FSERROR == can't read here ;>

      if (cpu[c].uregs[0]) {
        struct dirent** dent;

        // create cache...

        for (l=0;l<fsdata[c].ders;l++)
          if (fsdata[c].der[l]) free(fsdata[c].der[l]); else break;

        bzero(fsdata[c].der,sizeof(fsdata[c].der));
        fsdata[c].ders=0;

        sth=verify_access(c,cpu[c].uregs[1],(cpu[c].uregs[2]+3)/4,MEM_FLAG_READ);

        if (!sth) {
          non_fatal(ERROR_PROTFAULT,"dirlist: Attempt to access protected memory",c);
          failure=1;
          return;
        }
	
        fn=verify_and_clean(c,sth,cpu[c].uregs[2]);
        check_status();
        
        VALIDATE(c,fn,"fs/fops/list/directory");

        fiction=fn;
        fn=get_realpath(c,fn);
        check_status();

        buildcache_cpu=c;
        broken_scandir=0;

        if (scandir(fn,&dent,buildcache,(void*)broken_implementation)<0) {
          non_fatal(ERROR_FS_NOFILE,"Non-existing directory",c);
          failure=1;
          break;
        }

        cpu[c].sregs[0]=fsdata[c].ders;

      } else {

        // access cache...
        if (cpu[c].uregs[3]>=MAX_DIRENT) {
          non_fatal(ERROR_FS_NODIRENT,"Excessive offset for listdir",c);
          failure=1;
          break;
        }

        if (cpu[c].uregs[3]>=fsdata[c].ders) {
          non_fatal(ERROR_FS_NODIRENT,"Read beyond past of directory",c);
          failure=1;
          break;
        }

        sth=verify_access(c,cpu[c].uregs[1],(cpu[c].uregs[2]+3)/4,MEM_FLAG_WRITE);
        if (!sth) {
          non_fatal(ERROR_PROTFAULT,"dirlist: Attempt to access protected memory",c);
          failure=1;
          return;
        }

        if (strlen(fsdata[c].der[cpu[c].uregs[3]]->name)>cpu[c].uregs[2]) {
          non_fatal(ERROR_RESULT_TOOLONG,"dirlist: object name longer than buffer",c);
          failure=1;
          return;
        }

        memcpy(sth,fsdata[c].der[cpu[c].uregs[3]]->name,strlen(fsdata[c].der[cpu[c].uregs[3]]->name));

        cpu[c].uregs[2]=strlen(fsdata[c].der[cpu[c].uregs[3]]->name);
        cpu[c].sregs[0]=fsdata[c].ders-cpu[c].uregs[3]-1;
        cpu[c].sregs[1]=fsdata[c].der[cpu[c].uregs[3]]->type;

      }

      break;
     
  }

}


void syscall_unload(void) {
  int i,j,k=0,l;
  for (i=0;i<MAX_VCPUS;i++) {
    for (j=0;j<MAX_FS_FD;j++)
      if (fsdata[i].flag[j] & FS_FLAG_USED) { k++; close(fsdata[i].fd[j]); fsdata[i].flag[j]=0; }
    for (l=0;l<fsdata[i].ders;l++)
      if (fsdata[i].der[l]) free(fsdata[i].der[l]); else break;
    fsdata[i].ders=0;
  }
  if (k)
    printk("<< fs: Shutdown: closed %d active file descriptors.\n",k);
}


void syscall_task_cleanup(int c) {
  int j,l,k=0;
  for (j=0;j<MAX_FS_FD;j++) 
    if (fsdata[c].flag[j] & FS_FLAG_USED) { k++; close(fsdata[c].fd[j]); fsdata[c].flag[j]=0; }
  for (l=0;l<fsdata[c].ders;l++)
    if (fsdata[c].der[l]) free(fsdata[c].der[l]); else break;
  fsdata[c].ders=0;
  if (k)
    printk("=> fs: task_cleanup: Closed %d VCPU's owned file descriptors.\n",k);
}


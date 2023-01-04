/*

   Argante virtual OS
   ------------------

   Source language -> bytecode compiler. Actually, it's pretty
   rough code written within 4 hours. You'll notice lack of several
   useful "real compiler" features, it will be implemented in AHLL.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>
   Patched:    Bulba <bulba@intelcom.pl> - .code / .data switching
               Bulba <bulba@intelcom.pl> - .defines support

*/

#ifndef BUILD
#  define BUILD 1
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "console.h"
#include "bcode.h"
#include "bformat.h"
#include "language.h"
#include "syscall.h"


struct sdes {
  char* name;
  int num;
};

struct defines {
    char 		*name;
    char 		*value;
    struct defines	* next;
};

struct defines 		*defined = NULL;


struct sdes sys[]={
#include "autogen.h"
};

#define SCNUM (sizeof(sys)/sizeof(struct sdes))



char* nam;

struct sym_desc {
  char name[64];
  char in_code;
  int  addr;
  int  blength;
  char* data;
};

struct sym_desc sym[COMPILER_MAXSYM];
struct bformat bfmt;

int top_sym=0;

FILE* in;
FILE* out;

int line=0;


void error(char* x,char* lin) {
  if (nam) unlink(nam);
  printk("ERROR: line %d: %s\n",line,x);
  printk("Offending code: '%s'.\n",lin);
  exit(100);
}

struct defines *
find_define (char *name) {
    struct defines *d = defined;
    while (d) {
	if (!strcmp(d->name, name)) break;
    } 
    return d;
}

void 
add_define(char *name, char *value) {
    int got = 0;
    struct defines *d = defined;
    if (d) {
	while (d->next) {
	    if (!strcmp(d->name,name)) { got= 1; break; }
	    d = d->next;
	}
	if (got) {
	    free (d->value);
	} else {
	    d->next = (struct defines *) malloc (sizeof(struct defines));
	    if (!d) error("malloc failure","<<system>>");
	    d = d->next;
	    d->next = NULL;
	    d->name = strdup(name);
	}
	d->value = strdup (value);
    } else {
	defined = (struct defines *) malloc (sizeof (struct defines));
	if (!defined) error("malloc failure","<<system>>");
	defined->next = NULL;
	defined->name = strdup(name);
	defined->value = strdup(value);
	d = defined;
    }
    if (!d->value || !d->name) error("strup troubles","<<system>>");
}

void
parse_define (char *b) {
    char *d, *p=b+8;
    char name[1000], value[1000];
    while (*p && (*p==' ' || *p=='\t')) p++;
    if (!*p || *p=='\n') error("broken .define statement",b);
    d = name;
    while (*p && *p!=' ' && *p!='\t' && *p!='\n') *(d++) = *(p++);
    if (!*p || *p=='\n') error("broken .define statement",b);
    *d = '\0';
    d = value;
    while (*p && (*p==' ' || *p=='\t')) p++;
    while (*p && *p!='\n') *(d++) = *(p++);
    *d = '\0';    
    add_define(name, value);
}

void
substitute_define (char *b) {
    char *p;
    struct defines *d = defined;
    char buf[1000];
    while (d) {
	p = strstr(b, d->name);
	if (p) {
	    int l = p - b;
	    memcpy(buf, b, p-b);
	    memcpy(buf+l, d->value, strlen(d->value));
	    strncpy(buf+l+strlen(d->value), p+strlen(d->name), 999-l-strlen(d->value));
	    buf[999] = '\0';
	    strcpy(b,buf);
	    break;	
	}
	d=d->next;
    }
}

void fix_line(char* b) {
  char outbuf[1010];
  int x,c=0;
  int quoting=0,nospace=0,left_quote=0;
  if (b[strlen(b)-1]=='\n') b[strlen(b)-1]=0;

  for (x=0;x<strlen(b);x++) {
    if (quoting) {
      if (b[x]=='"') {
        quoting=0;
        left_quote=1; 
        nospace=0;
      }
      if (b[x]=='\\') {
        x++;
        switch (b[x]) {
          case 'n': outbuf[c++]='\n'; break;
          case 'r': outbuf[c++]='\r'; break;
          case 'e': outbuf[c++]=27;  break;
          case 'b': outbuf[c++]='\b'; break;
          default: printk("Warning: line %d: unknown escape sequence %c.\n",line,b[x]);
          // dorobic \x itp... kiedys ;>
        }
      } else outbuf[c++]=b[x];
    } else {
      if (b[x]=='"') {
        quoting=1; outbuf[c++]=b[x]; continue;
      }

      if (!isspace(b[x])) {
        if (b[x]=='#') break;
        nospace=1;
        if (left_quote) {
          error("closing '\"' followed by garbage.",b);
          exit(1);
        }
        outbuf[c++]=tolower(b[x]);
      } else {
        if (nospace) {
          nospace=0; outbuf[c++]=' '; 
        }
      }
    }
  }
  outbuf[c]=0;
  if (quoting) {
    error("unmatched '\"'",b);
    exit(1);
  }
  strncpy(b,outbuf,1000);
}

int validate_name(char* b) {
  int x;
  for (x=0;x<strlen(b);x++) 
    if ((!isalnum(b[x])) && (b[x]!='_')) return 0;
  return 1;
}

int current_code;
int current_data;


void collect_symbols(void) {

  int x;
  char buf[1000];
  int in_code=0;
  int in_data=0;
  int beyond_end=0;

  printk("Phase #1: indexing symbols...\n");

  while (fgets(buf,1000,in)) {
    char suck[1000]; int dupa,zenek;
    line++;
    fix_line(buf);
    if (!strlen(buf)) continue;
    while (buf[0] && (buf[strlen(buf)-1]==' ')) buf[strlen(buf)-1]=0;
    if (beyond_end) error("data beyond .end statement.",buf);
    if (buf[0]=='.') {
      if (!strcmp(buf,".data")) { in_code=0; in_data=1; } else
      if (!strcmp(buf,".code")) { in_data=0; in_code=1; } else
      if (!strcmp(buf,".end")) { beyond_end=1; in_data=in_code=0; } else
      if (strncmp(buf,".define ", 8)) 
          error("unrecognized . statement.",buf);
      continue;
    }
    if (buf[0]=='!') {
      // OPTION PARSING
      if (sscanf(buf,"!signature \"%[ -~]",suck)) {
        if (strrchr(suck,'"')) *strrchr(suck,'"')=0;
        strncpy(bfmt.signature,suck,60);
        continue;
      }

      if (sscanf(buf,"!priority %i",&dupa)) {
        bfmt.priority=dupa;
        continue;
      }

      if (sscanf(buf,"!ipcreg %i",&dupa)) {
        bfmt.ipc_reg=dupa;
        continue;
      }

      if (sscanf(buf,"!initdomain %i",&bfmt.current_domain)) continue;

      if (sscanf(buf,"!initdomain %i",&bfmt.current_domain)) continue;

      if (sscanf(buf,"!inituid %i",&bfmt.domain_uid)) continue;

      for (dupa=0;dupa<MAX_EXEC_DOMAINS;dupa++) 
        bfmt.domains[dupa]=0;
      zenek=0;

      if (sscanf(buf,"!domains %i",&dupa)) {
        char* blu=buf;
        int lo;
        for (lo=0;lo<MAX_EXEC_DOMAINS;lo++) {
          blu=strchr(blu,' ');
          if (!blu) break; 
          blu++;
          bfmt.domains[zenek++]=atoi(blu);
        } 
        
        continue;
      }

      error("bad or unparsable ! option.",buf);
      continue;
    }
    if (buf[0]==':') {

      if (!(in_code|in_data)) {
        error("symbol declared outside data or code block.",buf);
        exit(1);
      }

      if (strlen(buf)>60) {
        error("symbol name too long.",buf);
        exit(1);
      }

      if (!validate_name(&buf[1])) {
        error("incorrect symbol name or trailing garbage.",buf);
        exit(1);
      }

      strcpy(sym[top_sym].name,&buf[1]);
      sym[top_sym].in_code=in_code;

      // fgetsujemy nastêpn± liniê.
      line++;

loop:

      if (!fgets(buf,1000,in)) {
        error("unexpected end of file while item expected.","<EOF>");
        exit(1);
      }
      line++;

      fix_line(buf);

      if (strlen(buf)<1) goto loop;

      { 
        int b,a,i; float c; int* q;
        if (sscanf(buf,"%*s repeat %i",&b)>0) {
          if (in_code) {
            error("'repeat' statement not allowed in code.",buf);
            exit(1);
          }
          if (b<1 || b>MAX_ALLOC_MEMBLK) {
            error("senseless 'repeat' count.",buf);
            exit(1);
          }
          sym[top_sym].addr=current_data;
          sym[top_sym].data=malloc(b*4);
          q=(void*)sym[top_sym].data;

          if (strchr(buf,'.')) {
            if (sscanf(buf,"%f repeat",&c)) {
              a=*(int*)(&c);
            } else {
              error("unparsable repeat parameter.",buf);
            }
          } else
	    if (sscanf(buf,"0x%x repeat",&a)!=1) 
	     if (sscanf(buf,"%u repeat", &a)!=1) 
	       error("unparsable repeat parameter.",buf);

//	  printf("repeat member: %d %#x\n",a, a);
          for (i=0;i<b;i++) *(q++)=a;
          
          sym[top_sym].blength=b*4;
          current_data+=b;
          top_sym++;
          continue;
        }
      }


      { 
        int b,a; float c; int* q;
        int kount=0;
        if (sscanf(buf,"block %i",&b)>0) {
          if (in_code) {
            error("'block' statement not allowed in code.",buf);
            exit(1);
          }
          if (b<1 || b>MAX_ALLOC_MEMBLK) {
            error("senseless 'block' count.",buf);
            exit(1);
          }
          sym[top_sym].addr=current_data;
          sym[top_sym].data=malloc(b*4);
          q=(void*)sym[top_sym].data;

          for (kount=0;kount<b;kount++) {

            if (!fgets(buf,1000,in)) error("unexpected end of block statement.",buf);
            line++;
            fix_line(buf);
            if (strlen(buf)<1) continue;

            if (strchr(buf,'.')) {
              if (sscanf(buf,"%f",&c)) {
                a=*(int*)(&c);
              } else {
                error("unparsable block member.",buf);
              }
            } else
              if (sscanf(buf,"0x%x",&a)<1)
	        if (sscanf(buf,"%u",&a)<1) 
	         error("unparsable block member.",buf);

//	     printf("block member: %d %#x\n",a, a);
            *(q++)=a;

          }
          
          sym[top_sym].blength=b*4;
          current_data+=b;
          top_sym++;
          continue;
        }
      }

      if (strchr(buf,'"')) {
        if (in_code) {
          error("string statement not allowed in code.",buf);
          exit(1);
        }
        sym[top_sym].addr=current_data;
        sym[top_sym].blength=strlen(buf)-2;
        sym[top_sym].data=malloc(strlen(buf)-2);

        memcpy(sym[top_sym].data,buf+1,strlen(buf)-2);
        
        current_data+=(strlen(buf)+1)/4;
        top_sym++;
        continue;
      }

      if (in_data) {
        float c; int a;
        sym[top_sym].addr=current_data;
        sym[top_sym].data=malloc(4);

        if (strchr(buf,'.')) {
          if (sscanf(buf,"%f",&c)) {
            a=*(int*)(&c);
           } else {
              error("unparsable variable.",buf);
            }
          } else
	    if (sscanf(buf,"0x%x",&a)<1)
             if (sscanf(buf,"%i",&a)<1)
	      error("unparsable variable.",buf);

        memcpy(sym[top_sym].data,&a,4);
        sym[top_sym].blength=4;
        current_data+=1;
        top_sym++;      
      } else if (in_code) {
        sym[top_sym].addr=current_code;
        sym[top_sym].blength=0;
        top_sym++;
        current_code++;
      }
    } else if (strlen(buf)) {
      if (in_data) {
        error("unnamed .data item.",buf);
      } else {
        current_code++;
      }
    }
  }

  printk("Symbol table:\n");

  for (x=0;x<top_sym;x++) {
    printk("+ %03d: %16s | addr=0x%08x segment=%s blength=%d\n",
           x,sym[x].name,sym[x].addr,
           sym[x].in_code?"code":"data",sym[x].blength);
  }

  printk("Binary parameters:\n");

  bfmt.magic1= BFMT_MAGIC1;
  bfmt.magic2= BFMT_MAGIC2;

  printk("+ Execution domains: ");

  { 
    int got=0;

    for (x=0;x<MAX_EXEC_DOMAINS && bfmt.domains[x]>0;x++) {
      printk("%d ",bfmt.domains[x]); got=1;
    }

    if (!got) printk("<none>");
  }
  printk("\n");

  printk("+ Priority:          %d\n",bfmt.priority); 
  printk("+ IPC id:            %d\n",bfmt.ipc_reg); 
  printk("+ Initial IP:        %d\n",bfmt.init_IP); 
  printk("+ Current domain:    %d:%d\n",bfmt.current_domain,bfmt.domain_uid);
  printk("+ Data size:         %d bytes\n",current_data*4);
  printk("+ Code size:         %d bytes\n",current_code*12);
  printk("+ Signature:         %s\n",bfmt.signature[0]?bfmt.signature:"<none>");

  bfmt.bytesize=current_code;
  bfmt.datasize=current_data;
  bfmt.memflags=3;


}


void compile_binary(void) {
  int i=0;
  char buf[1000];
  int got;
  int written=0;
  line=0;

  printk("Phase #2: Writing headers...\n");
  fwrite(&bfmt,1,sizeof(bfmt),out);
  printk("Phase #3: Compiling code...\n");

  // compile and dump code

  fseek(in,0,0);

  got=0;
  while (fgets(buf,1000,in)) {
    if (!strncmp(buf,".define ", 8)) {
	line++;
	parse_define(buf);
	continue;
    }
    fix_line(buf);
    line++;
    if (!strcmp(buf,".code")) { got=1; break; }
  }

  if (!got) error("cannot find .code segment.","<<EOF>>");

  while (fgets(buf,1000,in)) {
    int gotme;
    char mnem[100],arg1[100],arg2[100],temp[100];
    int i,j,to1,to2;
    char t1,t2,t1f=0,t2f=0,zero=0;
    if (!strncmp(buf,".define ",8)) {
	line++;
	parse_define(buf);
	continue;
    }
    substitute_define(buf);
    fix_line(buf);
    line++;
    if (!strlen(buf)) continue;
    if (buf[0]=='.') {
      if (!strcmp(buf,".end")) break; 
/* eru beg 2000.12.10 */ 
	if (!strcmp(buf,".code")) continue; /* .code\n.code\n.code accepted */
/* eru end 2000.12.10 */
      if (!strcmp(buf,".data")) {
        int ender = 0;
        got = 0;
        while (fgets(buf,1000,in)) {
	  if (!strncmp(buf,".define ",8)) {
	    line++;
	    parse_define(buf);
	    continue;
          }
   	  fix_line(buf);
          line++;
          if (!strlen(buf)) continue;
  	  if (!strcmp(buf,".code")) { got = 1; break; }
	  if (!strcmp(buf,".end")) { ender = 1; break; }
        }
	if (ender) break;
	if (got) continue;
      } else error("found . statement within code block.",buf);
    }
    if (buf[0]=='!') error("found ! statement within code block.",buf);
    if (buf[0]==':') continue; // skip symbol.
    if (strlen(buf)>90) error("line too long.",buf);
    // code partytime!
    i=sscanf(buf,"%[a-z0-9] %[:*a-z0-9_.^%$]%*[ ,]%[-:*a-z0-9_.^%$] %s",
             mnem,arg1,arg2,temp);

    if (i<1) error("unparsable mnemonic.",buf);
    if (i>3) error("trailing garbage at the end of line.",buf);

    gotme=0;
    for (j=0;j<OPS;j++)
      if (!strcmp(op[j].name,mnem)) {
        gotme=1; break;
      }

    if (!gotme) error("unknown instruction.",buf);

    if (op[j].params!=(i-1)) error("incorrect number of parameters.",buf);

//    printk("DEBUG %d:\t%s\n",written,buf);

    if (!op[j].params) {
      to1=0; to2=0; t1=0; t2=0;
      fwrite(&op[j].bcode,1,1,out);
      fwrite(&t1,1,1,out);
      fwrite(&t2,1,1,out);
      fwrite(&zero,1,1,out);
      fwrite(&to1,4,1,out);
      fwrite(&to2,4,1,out);
      written++;
      continue;
    }

    to2=0;
    t2=0;

    if (arg1[0]==':') {
      int q;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg1[1],sym[q].name)) {
        gotme=1;
        sprintf(arg1,"%d",sym[q].addr);
      }
      if (!gotme) error("unknown symbol.",buf);
    }

    if (arg1[1]==':') {
      int q;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg1[2],sym[q].name)) {
        gotme=1;
        sprintf(arg1,"*%d",sym[q].addr);
      }
      if (!gotme) error("unknown symbol.",buf);
    }

    if (arg1[0]=='^') {
      int q;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg1[1],sym[q].name)) {
        gotme=1;
        if (sym[q].in_code) error("cannot predict size of .code symbols.",buf);
        sprintf(arg1,"%d",sym[q].blength);
      }
      if (!gotme) error("unknown symbol.",buf);
    }


    if (arg1[0]=='$') {
      int q;
      gotme=0;
      for (q=0;q<SCNUM;q++) {
        if (!strcasecmp(&arg1[1],sys[q].name)) {
          gotme=1;
          sprintf(arg1,"%u",sys[q].num);
        }
      }
      if (!gotme) error("unknown syscall / exception.",buf);
    }



    if (arg1[0]=='%') {
      int q,l;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg1[1],sym[q].name)) {
        gotme=1; l=sym[q].blength;
        if (!sym[q].in_code) l=(sym[q].blength+3)/4; else
          error("cannot predict size of .code symbols.",buf);
        sprintf(arg1,"%d",l);
      }
      if (!gotme) error("unknown symbol.",buf);
    }

    if (arg1[0]=='*') {
      if (!isdigit(arg1[1])) {
        if (arg1[1]=='u') {
          t1=TYPE_UPTR;
          strcpy(temp,&arg1[2]);
          strcpy(arg1,temp);
        } else error("unknown register or syntax error.",buf);
      } else {
        t1=TYPE_IMMPTR;
        strcpy(temp,&arg1[1]);
        strcpy(arg1,temp);
      }
    } else {
      if (strchr(arg1,'.')) {
        t1=TYPE_IMMEDIATE;
	t1f=1;
      } else {
        if (!isdigit(arg1[0])) {
          if (arg1[0]=='u') {
            t1=TYPE_UREG;
            strcpy(temp,&arg1[1]);
            strcpy(arg1,temp);
          } else if (arg1[0]=='s') {
            t1=TYPE_SREG;
            strcpy(temp,&arg1[1]);
            strcpy(arg1,temp);
          } else if (arg1[0]=='f') {
            t1=TYPE_FREG;
            strcpy(temp,&arg1[1]);
            strcpy(arg1,temp);
          } else error("unknown register or syntax error.",buf);
        } else {
          t1=TYPE_IMMEDIATE;
        }
      }
    }


    if (op[j].params==2) {

    if (arg2[0]=='$') {
      int q;
      gotme=0;
      for (q=0;q<SCNUM;q++) if (!strcasecmp(&arg2[1],sys[q].name)) {
        gotme=1;
        sprintf(arg2,"%u",sys[q].num);
      }
      if (!gotme) error("unknown syscall / exception.",buf);
    }


    if (arg2[0]==':') {
      int q;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg2[1],sym[q].name)) {
        gotme=1;
        sprintf(arg2,"%d",sym[q].addr);
      }
      if (!gotme) error("unknown symbol.",buf);
    }

    if (arg2[1]==':') {
      int q;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg2[2],sym[q].name)) {
        gotme=1;
        sprintf(arg2,"*%d",sym[q].addr);
      }
      if (!gotme) error("unknown symbol.",buf);
    }

    if (arg2[0]=='^') {
      int q;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg2[1],sym[q].name)) {
        gotme=1;
        if (sym[q].in_code) error("cannot predict size of .code symbols.",buf);
        sprintf(arg2,"%d",sym[q].blength);
      }
      if (!gotme) error("unknown symbol.",buf);
    }


    if (arg2[0]=='%') {
      int q,l;
      gotme=0;
      for (q=0;q<top_sym;q++) if (!strcmp(&arg2[1],sym[q].name)) {
        gotme=1; l=sym[q].blength;
        if (!sym[q].in_code) l=(sym[q].blength+3)/4; else
          error("cannot predict size of .code symbols.",buf);
        sprintf(arg2,"%d",l);
      }
      if (!gotme) error("unknown symbol.",buf);
    }

    if (arg2[0]=='*') {
      if (!isdigit(arg2[1])) {
        if (arg2[1]=='u') {
          t2=TYPE_UPTR;
          strcpy(temp,&arg2[2]);
          strcpy(arg2,temp);
        } else error("unknown register or syntax error.",buf);
      } else {
        t2=TYPE_IMMPTR;
        strcpy(temp,&arg2[1]);
        strcpy(arg2,temp);
      }
    } else {
      if (strchr(arg2,'.')) {
        t2=TYPE_IMMEDIATE;
	t2f=1;
      } else {
        if (!isdigit(arg2[0]) && arg2[0]!='-') {
          if (arg2[0]=='u') {
            t2=TYPE_UREG;
            strcpy(temp,&arg2[1]);
            strcpy(arg2,temp);
          } else if (arg2[0]=='s') {
            t2=TYPE_SREG;
            strcpy(temp,&arg2[1]);
            strcpy(arg2,temp);
          } else if (arg2[0]=='f') {
            t2=TYPE_FREG;
            strcpy(temp,&arg2[1]);
            strcpy(arg2,temp);
          } else error("unknown register or syntax error.",buf);
        } else {
          t2=TYPE_IMMEDIATE;
        }
      }
    }

    }

    // siu...

    if (t1f) sscanf(arg1,"%f",(float*)&to1); 
    else {
	if (sscanf(arg1,"0x%x",&to1)<1) sscanf(arg1,"%u",&to1);
    }
    
    if (op[j].params==2) {
      if (t2f) sscanf(arg2,"%f",(float*)&to2); 
      else 
          if (sscanf(arg2,"0x%x",&to2)<1) sscanf(arg2,"%u",&to2);
    }
    t1f=0;t2f=0;
    // verify param1

    if (t1==TYPE_IMMEDIATE && !op[j].imm1) error("value not allowed as 1st parameter",buf);
    if (t1==TYPE_UREG && !op[j].ureg1) error("ureg not allowed as 1st parameter",buf);
    if (t1==TYPE_SREG && !op[j].sreg1) error("sreg not allowed as 1st parameter",buf);
    if (t1==TYPE_FREG && !op[j].freg1) error("freg not allowed as 1st parameter",buf);
    if (t1==TYPE_IMMPTR && !op[j].immptr1) error("immptr not allowed as 1st parameter",buf);
    if (t1==TYPE_UPTR && !op[j].uptr1) error("uptr not allowed as 1st parameter",buf);

    if (op[j].params==2) {
      if (t2==TYPE_IMMEDIATE && !op[j].imm2) error("value not allowed as 2st parameter",buf);
      if (t2==TYPE_UREG && !op[j].ureg2) error("ureg not allowed as 2st parameter",buf);
      if (t2==TYPE_SREG && !op[j].sreg2) error("sreg not allowed as 2st parameter",buf);
      if (t2==TYPE_FREG && !op[j].freg2) error("freg not allowed as 2st parameter",buf);
      if (t2==TYPE_IMMPTR && !op[j].immptr2) error("immptr not allowed as 2st parameter",buf);
      if (t2==TYPE_UPTR && !op[j].uptr2) error("uptr not allowed as 2st parameter",buf);
    }

//   printk("Writing instruction %s 0x%x,0x%x t1=%d t2=%d\n",op[j].name,
//          to1,to2,t1,t2);

    {
      fwrite(&op[j].bcode,1,1,out);
      fwrite(&t1,1,1,out);
      fwrite(&t2,1,1,out);
      fwrite(&zero,1,1,out);
      fwrite(&to1,4,1,out);
      fwrite(&to2,4,1,out);
      written++;
    }

  }

  // compile and dump data

  printk("Compiled %d instructions...\n",written);
  printk("Phase #4: Writing data segment...\n");

  for (i=0;i<top_sym;i++) {
    char zeros[]={0,0,0,0};
    if (!sym[top_sym].in_code) {
      fwrite(sym[i].data,1,sym[i].blength,out);
      if (sym[i].blength % 4) 
        fwrite(zeros,1,4 - sym[i].blength % 4,out);
    }
  }

}


int main(int argc,char* argv[]) {
  char* x;
  printk("%s OS version %d.%03d.%04d *BETA* bytecode compiler.\n",SYSNAME,SYS_MAJOR,SYS_MINOR,BUILD);
  printk("(C) 2000 Michal Zalewski <lcamtuf@ids.pl>\n\n");
  if (argc-2) {
    printk("Usage: %s filename.agt\n"
           "This will produce filename.img.\n\n",argv[0]);
    exit(1);
  }
  in=fopen(argv[1],"r");
  if (!in) {
    perror("Cannot open input file");
    exit(2);
  }
  x=strrchr(argv[1],'.');
  if (x) *x=0;
  strcat(argv[1],".img");
  out=fopen(argv[1],"w");
  if (!out) {
    perror("Cannot create output file");
    exit(3);
  }
  nam=argv[1];
  collect_symbols();
  compile_binary();
  fclose(out);
  printk("Compiled successfully, output file is %s.\n",argv[1]);
  return 0;
}

/*

   Argante virtual OS
   ------------------

   High-level language translator. Procudes .agt output to be parsed
   with agtc (Argante low-level compiler). It's really nasty code, don't
   even look at it - just... it works ;)

   Status: YOU REALLY DON'T WANT TO LOOK AT THIS CODE. IT SUCKS.

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

   The bank called to tell me that I'm overdrawn,
   Some freaks are burning crosses out on my front lawn,
   And I *can't*believe* it, all the Cheetos are gone,
           It's just ONE OF THOSE DAYS!



*/

//      ------------------------------------------------------
//      N E X T   3 0 0 0   L I N E S   A R E    C R A P ! ! !
//      ------------------------------------------------------

#ifndef BUILD
#define BUILD 1
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

#include "config.h"
#include "ahlt.h"

#include "console.h"
#include "bcode.h"
#include "syscall.h"
#include "bformat.h"

char* tfile;

FILE *in,*out;
char* nam;

char* pre_src[MAX_HLL_DEF];
char* pre_dst[MAX_HLL_DEF];
int top_def=0;
int fc=0;

int nodo=0;
int unused=1000;

#define LINEBUFFER 10000

char buf[LINEBUFFER+10];
int line=0;
int isitwrit=0,inswitch=0;

void error(char* x,char* lin) {
  if (nam) unlink(nam);
  printk("ERROR: line %d: %s\n",line,x);
  if (tfile) printk("Working file: %s\n",tfile);
  printk("Offending code: '%s'.\n",lin);
  exit(100);
}

int inside_comment=0;
int nesting_level=0;
int com_blk=0,cmp_dir=0;

void fix_line(char* b) {
  char outbuf[LINEBUFFER+100];
  int x,c=0;
  int quoting=0,nospace=0,left_quote=0;
  if (b[strlen(b)-1]=='\n') b[strlen(b)-1]=0;

  if (inside_comment)
    for (x=0;x<strlen(b);x++) {
      if (!strncmp(&b[x],"*/",2)) { 
//        printk("Removing comment, skipping %d characters [%s]\n",x,b);
        inside_comment=0; 
        strcpy(b,b+x+2); 
        break; 
      }
    }

  if (inside_comment) b[0]=0;

  for (x=0;x<strlen(b);x++) {
    if (quoting) {
      if (b[x]=='"') {
        quoting=0;
        left_quote=1;
        nospace=0;
      }
      outbuf[c++]=b[x];
    } else {
      if (b[x]=='"') {
        quoting=1; outbuf[c++]=b[x]; continue;
      }

      if (!isspace(b[x])) {
        if (!strncmp(&b[x],"//",2)) { com_blk++; break; }
        if (!strncmp(&b[x],"/*",2)) { com_blk++; inside_comment=1; break; }
        nospace=1;
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

int depth=0;

FILE *ftmp;

char lineenders[]="{}(;,";
char spacers[]="{};(),:";
int rep_made=0;
int swcnt=0;
int inc_files=0;
int sub_blk=0;

void fputs_lineender(char* buf,FILE* sth) {
  int i;
  int quotyn=0;
  for (i=0;i<strlen(buf);i++) {
    if (buf[i]=='"') quotyn=!quotyn;
    if (!quotyn) if (strchr(spacers,buf[i])) fputc(' ',sth);
    fputc(buf[i],sth);
    if (!quotyn) if (strchr(lineenders,buf[i])) fputc('\n',sth);
    if (!quotyn) if (buf[i]=='{') { sub_blk++; nesting_level++; }
    if (!quotyn) if (buf[i]=='}') nesting_level--;
    if (!quotyn) if (nesting_level<0) error("Found '}' without '{'.",buf);
  }
  if (strchr(buf,'#')) fputc('\n',sth); else fputc(' ',sth);
}


int cstr=0;

void parse_file(void) {
  FILE* inc;
  char tmp[256]; // ,tmp2[256];
  char wk[LINEBUFFER],wk2[LINEBUFFER];
  int have_includes=0;
  sprintf(tmp,".ahlt-%d-%d-%d.tmp1",getpid(),(int)time(0),fc++);
  if (!depth) printk("* Phase #1: Precompilation / parsing.\n"); else
    printk("+ Includes - pass %d, working file: %s...\n",depth,tmp);
  buf[LINEBUFFER-1]=0;
  line=0;
  ftmp=fopen(tmp,"w+");
  if ((!ftmp)) error("Cannot create temporary files","<<NULL>>");
  while (fgets(buf,LINEBUFFER,in)) {
    line++;
    if (buf[LINEBUFFER-1]) error("Line too long.","<<cropped>>");
    fix_line(buf);
    if (!strlen(buf)) continue;
//    if (buf[0]=='#')
//      if (nesting_level) error("# directives can have global scope only.",buf);

    if (!strncmp(buf,"#include",8)) {
      if (sscanf(buf,"#include \"%[0-9A-z._-/]\"",wk)!=1) error("Malformed #include.",buf);
      inc=fopen(wk,"r");
      if (!inc) error("Cannot open include file.",buf);
      have_includes=1; inc_files++;
      while (fgets(buf,LINEBUFFER,inc)) {
        if (buf[LINEBUFFER-1]) error("Line too long.","<<cropped>>");
        fix_line(buf);
        if (!strlen(buf)) continue;
        fputs_lineender(buf,ftmp);
      }
      fclose(inc);
    } else if (!strncmp(buf,"#define",7)) {
      if (sscanf(buf,"#define %[0-9A-z_] %[ -~]",wk,wk2)!=2) 
        error("Malformed #define statement.",buf);
      if (!isalpha(wk[0])) error("#define symbol must begin with character.",buf);
      pre_src[top_def]=strdup(wk);
      pre_dst[top_def]=strdup(wk2);
      top_def++;
    } else if (!strncmp(buf,"#compiler",9)) {
      fputs(buf+10,out);
      cmp_dir++;
      fputc('\n',out);
    } else if (!strncmp(buf,"#cstring",8)) {
      int len=0;
      if (sscanf(buf,"#cstring %s \"%[^\"]",wk,wk2)!=2)
        error("Malformed #cstring",buf);
      fprintf(ftmp,"type cstr_%d is bytechunk 0 .. %d ;\n",
                  cstr,strlen(wk2));
      { char* x=wk2;
        while ((x=strchr(x,'\\'))) { len++; x++; }
      }
      fprintf(ftmp,"%s_bytelength : unsigned := %d ;\n",wk,strlen(wk2)-len);
      fprintf(ftmp,"%s : cstr_%d := \"%s\" ;\n",wk,cstr++,wk2);
    } else if (buf[0]=='#') {
      error("Unknown # directive.",buf);  
    }  else {
      fputs_lineender(buf,ftmp);
    }


  }




  if (depth>HLL_MAX_DEPTH) error("#include recursion too deep.","<<?>>");

  if (have_includes) {
    if (tfile) unlink(tfile); // :)
    fclose(in);
    depth++;
    in=ftmp;
    tfile=strdup(tmp);
    fseek(in,0,0);
    parse_file();
    return;
  }

  if (tfile) unlink(tfile); // :)
  fclose(in);
  in=ftmp;
  tfile=strdup(tmp);
  fseek(in,0,0);

  printk("+ Substitutions - resolving precompiler #defs...\n");
  sprintf(tmp,".ahlt-%d-%d-%d.tmp2",getpid(),(int)time(0),fc++);

  buf[LINEBUFFER-1]=0;
  line=0;
  ftmp=fopen(tmp,"w+");
  if ((!ftmp)) error("Cannot create temporary files","<<NULL>>");

  while (fgets(buf,LINEBUFFER,in)) {
    int x,y;
    line++;
    if (buf[LINEBUFFER-1]) error("Line too long.","<<cropped>>");
    fix_line(buf);
    if (!strlen(buf)) continue;
    for (x=0;x<strlen(buf);x++) {
      int got=0;
      for (y=0;y<top_def;y++) {
        if (!strncmp(&buf[x],pre_src[y],strlen(pre_src[y]))) {
           if ((x>0) && isalnum(buf[x-1])) continue;
           if (isalnum(buf[x+strlen(pre_src[y])])) continue;
           x+=strlen(pre_src[y])-1;
           got=1;
           rep_made++;
           fputs(pre_dst[y],ftmp);
           continue;
        }
      }
      if (!got) fputc(buf[x],ftmp);
    }
    fputc('\n',ftmp);
  }

  // here it goes.

  if (tfile) unlink(tfile); // :)
  fclose(in);
  in=ftmp;
  tfile=strdup(tmp);
  fseek(in,0,0);

  printk("+ String parsing...\n");
  sprintf(tmp,".ahlt-%d-%d-%d.tmp2",getpid(),(int)time(0),fc++);

  buf[LINEBUFFER-1]=0;
  line=0;
  ftmp=fopen(tmp,"w+");
  if ((!ftmp)) error("Cannot create temporary files","<<NULL>>");

  while (fgets(buf,LINEBUFFER,in)) {
    int x,y=0;
    line++;
    if (buf[LINEBUFFER-1]) error("Line too long.","<<cropped>>");
    fix_line(buf);
    for (x=0;x<strlen(buf);x++) {
      if (buf[x]=='"') {
//        if ((!y) && (!strchr("({=<>",buf[x-1])))        
//          error("Standalone string constant 1",buf);
        y=!y;
        if ((!y) && !strncmp(&buf[x],"\" \"",3)) { x+=2; y=1; continue; }
        if ((!y) && !strncmp(&buf[x],"\"\"",2)) { x+=1; y=1; continue; }
//        if ((!y) && (!strchr(")};,",buf[x+1])))
//          error("Standalone string constant 2",buf);
      }
      fputc(buf[x],ftmp);
    }
    fputc('\n',ftmp);
  }

  // Final validation:
  // { == nesting_level++
  // } == nesting_level--
  //  lines should end with ;, { or }.
  // nesting level 0: procedure, subtype, type, "somename :"
  // nesting level >0: ignore, will be verified later :)

  nesting_level=0;

  if (tfile) unlink(tfile); // :)
  fclose(in);
  in=ftmp;
  fseek(in,0,0);

  printk("+ Final syntax sanity checks...\n");

  line=0;

  while (fgets(buf,LINEBUFFER,in)) {
    line++;
    if (buf[LINEBUFFER-1]) error("Line too long.","<<cropped>>");
    fix_line(buf);
    if (buf[strlen(buf)-1]=='}') nesting_level--; else
      if (buf[strlen(buf)-1]=='{') nesting_level++; else
        if (buf[strlen(buf)-1]==')') nesting_level--; else
          if (buf[strlen(buf)-1]=='(') nesting_level++; else
            if (buf[strlen(buf)-1]!=',') {
              if (buf[strlen(buf)-1]!=';') error("Unparsable statement",buf); else {

                if (!nesting_level)
                if (strncmp(buf,"procedure ",10) && strncmp(buf,"subtype ",8)
                  && strncmp(buf,"type ",5) && !strchr(buf,':'))
                error("Unrecognized statement",buf);

              }
            }
  }

  fseek(in,0,0);

  printk("+ Precompilation results: %d files included (passes: %d), %d defines\n",inc_files,depth,top_def);
  printk("+ found, %d replacements made, %d compiler directives, %d comment blocks,\n"
         "+ %d code / struct blocks. Output: %s...\n",rep_made,cmp_dir,com_blk,sub_blk,tmp);

//  fclose(ftmp);
}


struct typedesc typ[MAX_TYPES];
struct symdesc  sym[MAX_SYMBOLS];
struct fndesc   fn [MAX_FUNCTIONS];

struct blockdesc blk[MAX_HLL_NEST];

int top_fn=0,
    top_sym=0,
    top_typ=0,
    top_nest=0,
    top_blk=0;

int extyp=0,exsiz=0;

int validate_name(char* x) {
  int n;

  n = (strcmp(x,"type")!=0) *        (strcmp(x,"subtype")!=0) *
      (strcmp(x,"do")!=0) *          (strcmp(x,"while")!=0) *
      (strcmp(x,"ignore")!=0) *      (strcmp(x,"raise")!=0) *
      (strcmp(x,"halt")!=0) *        (strcmp(x,"twait")!=0) *
      (strcmp(x,"cwait")!=0) *       (strcmp(x,"goto")!=0) *
      (strcmp(x,"exception")!=0) *   (strcmp(x,"guarded")!=0) *
      (strcmp(x,"continue")!=0) *    (strcmp(x,"break")!=0) *
      (strcmp(x,"procedure")!=0) *   (strcmp(x,"unsigned")!=0) *
      (strcmp(x,"signed")!=0) *      (strcmp(x,"bytechunk")!=0) *
      (strcmp(x,"pointer")!=0) *     (strcmp(x,"to")!=0) *
      (strcmp(x,"addressable")!=0) * (strcmp(x,"writable")!=0) *
      (strcmp(x,"float")!=0) *       (strcmp(x,"create")!=0) *
      (strcmp(x,"bind")!=0) *        (strcmp(x,"unbind")!=0) *
      (strcmp(x,"destroy")!=0) *     (strcmp(x,"structure")!=0) *
      (strcmp(x,"array")!=0) *       (strcmp(x,"and")!=0) *
      (strcmp(x,"or")!=0) *          (strcmp(x,"not")!=0) *
      (strcmp(x,"xor")!=0) *         (strcmp(x,"syscall")!=0) *
      (strcmp(x,"__assembler__")!=0);


  if (!n) return 0;
  if (!isalpha(x[0])) return 0;
  for (n=0;n<strlen(x);n++) if ((!isalnum(x[n])) && (x[n]!='_')) return 0;
  return 1;

}


int lookup_type(char* name) {
  int n;
  for (n=0;n<top_typ;n++) if (!strcmp(name,typ[n].name)) return n;
  return -1;
}

int nest_cnt=0;
int param_already=0;
int sosen=-1;

int lookup_field(int type,int upto,char* name) {
  int n;
  for (n=0;n<upto;n++) if (!strcmp(name,typ[type].s_field[n])) return n;
  return -1;
}


int lookup_par(int now,int upto,char* name) {
  int n;
  for (n=0;n<upto;n++) if (!strcmp(name,fn[now].s_name[n])) return n;
  return -1;

}



int lookup_fn(char* name) {
  int n;
  for (n=0;n<top_fn;n++) if (!strcmp(name,fn[n].name)) return n;
  return -1;
}


int lookup_sym(char* name) {
  int n;
  for (n=0;n<top_sym;n++) if (!strcmp(name,sym[n].name)) return n;
  return -1;
}

char real_name[LINEBUFFER];
char  is_pointer, is_addressable;
int dest_size;
int base_type;
int its_type, symno;
int is_simple;
int is_immed;
int last_val_flags;
int hadcase,jakisint;

// Lookup SOME symbol in current context :)

#define isitwri isitwrit

int lookup_var(char* ident,char* whereami) {
  int nn;
  char dummy[LINEBUFFER];
  last_val_flags=0;
  is_simple=0;
  nn=lookup_par(top_fn-1,fn[top_fn-1].params,ident);
  if (nn>-1) {
    if (fn[top_fn-1].s_issim[nn]) {
      its_type=fn[top_fn-1].s_type[nn];
      dest_size=1;
      is_pointer=1;
      is_simple=1;
      isitwri=is_addressable=fn[top_fn-1].writ[nn];
    } else {
      base_type=fn[top_fn-1].s_type[nn];
      its_type=typ[fn[top_fn-1].s_type[nn]].type;
      is_pointer=1;
      dest_size=typ[fn[top_fn-1].s_type[nn]].size;
      if (its_type!=TYPE_STRUCT)
        if (its_type!=TYPE_BCHUNK)
          if (its_type!=TYPE_ARRAY) is_simple=1;
      isitwri=is_addressable=fn[top_fn-1].writ[nn];
    }
    sprintf(real_name,"PPtr_FN_%s_%s",whereami,ident);
    symno=nn;
    return 1;
  }

  sprintf(dummy,"LV_%s_%s",whereami,ident);
  nn=lookup_sym(dummy);

  if (nn>-1) {
    if (sym[nn].issim) {
      its_type=sym[nn].type;
      isitwri=1;
      dest_size=is_simple=1;
      is_pointer=sym[nn].flag & V_POINTER;
      last_val_flags=sym[nn].flag;
      is_addressable=sym[nn].flag & V_ADDRESSABLE;
    } else {
      base_type=sym[nn].type;
      its_type=typ[sym[nn].type].type;
      dest_size=typ[sym[nn].type].size;

      last_val_flags=sym[nn].flag;
      if (its_type!=TYPE_STRUCT)
        if (its_type!=TYPE_BCHUNK)
          if (its_type!=TYPE_ARRAY) is_simple=1;
      is_pointer=sym[nn].flag & V_POINTER;
      isitwri=1;
      is_addressable=sym[nn].flag & V_ADDRESSABLE;
    }
    strcpy(real_name,dummy);
    symno=nn;
    return 1;
  }

  sprintf(dummy,"GV_%s",ident);
  nn=lookup_sym(dummy);

  if (nn>-1) {
    if (sym[nn].issim) {
      its_type=sym[nn].type;
      isitwri=1;
      dest_size=is_simple=1;
      last_val_flags=sym[nn].flag;
      is_pointer=sym[nn].flag & V_POINTER;
      is_addressable=sym[nn].flag & V_ADDRESSABLE;
    } else {
      isitwri=1;
      base_type=sym[nn].type;
      its_type=typ[sym[nn].type].type;
      dest_size=typ[sym[nn].type].size;
      if (its_type!=TYPE_STRUCT)
        if (its_type!=TYPE_BCHUNK)
          if (its_type!=TYPE_ARRAY) is_simple=1;
      last_val_flags=sym[nn].flag;
      is_pointer=sym[nn].flag & V_POINTER;
      is_addressable=sym[nn].flag & V_ADDRESSABLE;
    }
    strcpy(real_name,dummy);
    symno=nn;
    return 1;
  }

  return 0;

}


#define oXt obuf+strlen(obuf)

int DO_NOT_FOLLOW_POINTERS=0;

int put_value_in_reg(char* obuf,char* statement,char* whereami,char* reg) {
//  char txt[1000];
  char* isanything=0,*more;
  int atsiz,want_addr=0;
  more=strchr(statement,'.');

  is_immed=0;
  if (statement[0]=='@') { want_addr=1; statement++; }

  sprintf(oXt,"# Accessing %s...\n",statement);

  if (!(isanything=strchr(statement,'['))) {
    if (isdigit(statement[0]) || (statement[0]=='-')) {
      is_immed=1;
      sprintf(oXt,"  MOV %s,%s\n",reg,statement);
//      if (want_addr) error("Cannot get address of immediate expression",buf);
      return 1;
    } else {
      if (more) *more=0;

      if (!lookup_var(statement,whereami)) error("Cannot resolve symbol",buf);
      if ((!more) && (DO_NOT_FOLLOW_POINTERS)) {
          sprintf(oXt,"  MOV %s,:%s\n",reg,real_name);
          goto LuLu;
      }
      if ((more) || (want_addr)) {
        if (!is_pointer) {
          sprintf(oXt,"  MOV %s,:%s\n",reg,real_name);
        } else {
          sprintf(oXt,"  MOV %s,*:%s\n",reg,real_name);
          sprintf(oXt,"  IFEQ %s,0\n",reg);
          sprintf(oXt,"    RAISE 10001 # Pointer not initialized\n");
        }
      } else {
        if (!is_pointer) {
          sprintf(oXt,"  MOV %s,*:%s\n",reg,real_name);
        } else {
            sprintf(oXt,"  MOV %s,*:%s\n",reg,real_name);
            sprintf(oXt,"  IFEQ %s,0\n",reg);
            sprintf(oXt,"    RAISE 10001 # Pointer not initialized\n");
            sprintf(oXt,"  MOV %s,*%s\n",reg,reg);
        }
      }
LuLu:
    }
    if (!more) {
      if (its_type == TYPE_SIGNED) return 1;
      if (its_type == TYPE_UNSIGNED) return 1;
      if (its_type == TYPE_FLOAT) return 1;
      if (its_type == TYPE_IPADDR) return 1;
      if (want_addr) return 1;
      return 0;
    }
  } else {
    char* x, bufik[1000];
    int typik;
    strcpy(bufik,isanything+1);
    x=strchr(bufik,']');
    if (!x) error("No closing bracket",buf);
    *x=0;
    if (!put_value_in_reg(obuf,bufik,whereami,reg)) 
      error("Only simple type can be used for addressing",buf);
    strcpy(bufik,statement);
    x=strchr(bufik,'[');
    *x=0;
    if (!lookup_var(bufik,whereami)) error("Cannot resolve table name",buf);
    if (its_type != TYPE_ARRAY)
//      if (its_type != TYPE_BCHUNK)
        error("Non-table symbol used as table identifier",buf);

    sprintf(oXt,"  IFBEL %s,%d\n",reg,typ[sym[symno].type].a_start);
    sprintf(oXt,"    RAISE 10002 # Array index too low\n");
    sprintf(oXt,"  IFABO %s,%d\n",reg,typ[sym[symno].type].a_end);
    sprintf(oXt,"    RAISE 10003 # Array index too high\n");
    sprintf(oXt,"  SUB %s,%d\n",reg,typ[sym[symno].type].a_start);

    if (typ[sym[symno].type].a_simple) atsiz=1;
      else atsiz=typ[typ[sym[symno].type].a_type].size;

    if (typ[its_type].a_simple) typik=typ[its_type].a_type;
      else typik=typ[typ[its_type].a_type].type;

    its_type=typik;
    dest_size=atsiz;

    if (typik!=TYPE_ARRAY)
      if (typik!=TYPE_BCHUNK)
        if (typik!=TYPE_STRUCT) is_simple=1;

    if (!is_pointer) {
      sprintf(oXt,"  MUL %s,%d\n",reg,atsiz);
      sprintf(oXt,"  MOV u15,:%s\n",real_name);
      sprintf(oXt,"  ADD %s,u15\n",reg);
      if (!want_addr || more)
        sprintf(oXt,"  MOV %s,*%s\n",reg,reg);
    } else {
      sprintf(oXt,"  MUL %s,%d\n",reg,atsiz);
      sprintf(oXt,"  MOV u15,*:%s\n",real_name);
      sprintf(oXt,"  IFEQ u15,0\n");
      sprintf(oXt,"    RAISE 10001 # Pointer not initialized\n");
      sprintf(oXt,"  ADD %s,u15\n",reg);
      if (!want_addr || more)
        sprintf(oXt,"  MOV %s,*%s\n",reg,reg);
    }

    if (!more) {
      if (want_addr) return 1;
      if (typ[sym[symno].type].a_simple) return 1;
      if (typ[typ[sym[symno].type].a_type].type == TYPE_UNSIG) return 1;
      if (typ[typ[sym[symno].type].a_type].type == TYPE_SIGNED) return 1;
      if (typ[typ[sym[symno].type].a_type].type == TYPE_FLOAT) return 1;
      if (typ[typ[sym[symno].type].a_type].type == TYPE_IPADDR) return 1;
      return 0;
    }

  }

  if (more) {
    int off=0,x;
    if (its_type!=TYPE_STRUCT) 
      error("Cannot access fields of non-structural type",buf);
    for (x=0;x<typ[base_type].s_fields;x++) {
      if (!strcmp(typ[base_type].s_field[x],more+1)) {
        sprintf(oXt,"  ADD %s,%d\n",reg,off);

        last_val_flags=typ[base_type].s_flag[x];

        if (!(is_simple=typ[base_type].issim[x]))
          its_type=typ[typ[base_type].s_type[x]].type;
        if (is_simple) dest_size=1;
          else dest_size=typ[typ[its_type].s_type[x]].size;

        if (!DO_NOT_FOLLOW_POINTERS)
        if (typ[its_type].s_flag[x] & V_POINTER) {
          sprintf(oXt,"  MOV %s,*%s\n",reg,reg);
          sprintf(oXt,"  IFEQ %s,0\n",reg);
          sprintf(oXt,"    RAISE 10001  # Pointer uninitialized\n");
        }

        if (!want_addr)
          sprintf(oXt,"  MOV %s,*%s\n",reg,reg);

        {
          int tyfek;
          if (typ[its_type].issim[x]) tyfek=typ[its_type].s_type[x];
          else tyfek=typ[typ[its_type].s_type[x]].type;

        is_pointer=(typ[its_type].s_flag[x] & V_POINTER);
        is_addressable=(typ[its_type].s_flag[x] & V_ADDRESSABLE);
        its_type=tyfek;
        is_simple=0;
        if (tyfek == TYPE_UNSIG)  is_simple=1;
        if (tyfek == TYPE_SIGNED) is_simple=1;
        if (tyfek == TYPE_FLOAT)  is_simple=1;
        if (tyfek == TYPE_IPADDR) is_simple=1;

        }


        if (want_addr) return 1;
        if (typ[sym[symno].type].issim[x]) return 1;
        if (typ[typ[sym[symno].type].s_type[x]].type == TYPE_UNSIG) return 1;
        if (typ[typ[sym[symno].type].s_type[x]].type == TYPE_SIGNED) return 1;
        if (typ[typ[sym[symno].type].s_type[x]].type == TYPE_FLOAT) return 1;
        if (typ[typ[sym[symno].type].s_type[x]].type == TYPE_IPADDR) return 1;
        return 0;
      }
      if (typ[sym[symno].type].issim[x]) off++; else
        off+=typ[typ[sym[symno].type].s_type[x]].size;

    }
    error("Unknown field",buf);
  }

  return 0;

}


int write_value_in_u0(char* sym,char* whereami) {
  char buf[LINEBUFFER];
  int x;
  buf[0]=0;
  x=put_value_in_reg(buf,sym,whereami,"u0");
  fputs(buf,out);
  return x;
}


int isanexception=0;

int immthing=1;

void write_code(void) {
  char wai[LINEBUFFER],dummy[LINEBUFFER],w1[LINEBUFFER],w2[LINEBUFFER],
       wtc[LINEBUFFER];
  int state=STATE_NONE,a,b,xxx=0;
  int now_at=0;
  fseek(in,0,0);
  line=0;

  printk("Phase #2: writing code and data segments...\n");

  fprintf(out,"\n# Some very-first data items, sorry...\n");
  fprintf(out,".DATA\n\n");
  fprintf(out,":NULL\n# To avoid problems\n");
  fprintf(out,"  0\n");
  fprintf(out,":TempFloat\n");
  fprintf(out,"  0.001\n");
  fprintf(out,":Switch_Value\n");
  fprintf(out,"  0\n");
  fprintf(out,"\n.CODE\n\n");
  fprintf(out,"JMP :FN_Main\n\n");
  fprintf(out,"# AHLL compiler dynamic code:\n\n");

  while (fgets(buf,LINEBUFFER,in)) {
    line++;
    fix_line(buf);
    if (!strlen(buf)) continue;
    switch (state) {

      case STATE_NONE:

        if (sscanf(buf,"type %s is structure %[{]",wai,dummy)==2) {
          state=STATE_STRUCT;
          now_at=0;
          if (!validate_name(wai)) error("Incorrect type name",buf);
          if (lookup_type(wai)>-1) error("Type already declared",buf);
          strcpy(typ[top_typ].name,wai);
          typ[top_typ].type=TYPE_STRUCT;
          continue;
        }

        if ( (sscanf(buf,"type %s is array %i..%i of %s %[;]",w1,&a,&b,w2,dummy)==5) ||
             (sscanf(buf,"type %s is array %i .. %i of %s %[;]",w1,&a,&b,w2,dummy)==5)) {
          if (!validate_name(w1)) error("Incorrect type name",buf);
          if (lookup_type(w1)>-1) error("Type already declared",buf);
          xxx=lookup_type(w2);
          typ[top_typ].type=TYPE_ARRAY;
          typ[top_typ].a_simple=1;
          if (xxx>-1) {
            typ[top_typ].a_type=xxx;
            typ[top_typ].a_simple=0;
            typ[top_typ].size=(b-a)*typ[xxx].size;
          } else {
            if (!strcmp(w2,"unsigned")) typ[top_typ].a_type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) typ[top_typ].a_type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) typ[top_typ].a_type=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) typ[top_typ].a_type=TYPE_IPADDR; else
              error("Unknown type for array elements",buf);
            typ[top_typ].size=(b-a);
          }
          strcpy(typ[top_typ].name,w1);
          typ[top_typ].a_start=a;
          typ[top_typ].a_end=b;
          if (b-a<1) error("Zero or negative size array",buf);
          top_typ++;
          continue;
        }


        if ((sscanf(buf,"type %s is bytechunk %i..%i %[;]",w1,&a,&b,dummy)==4) ||
            (sscanf(buf,"type %s is bytechunk %i .. %i %[;]",w1,&a,&b,dummy)==4)) {
          if (!validate_name(w1)) error("Incorrect type name",buf);
          if (lookup_type(w1)>-1) error("Type already declared",buf);
          typ[top_typ].type=TYPE_BCHUNK;
          typ[top_typ].size=(b-a+3)/4;
          strcpy(typ[top_typ].name,w1);
          typ[top_typ].a_start=a;
          typ[top_typ].a_end=b;
          if (b-a<1) error("Zero or negative size bytechunk",buf);
          top_typ++;
          continue;
        }


        if (sscanf(buf,"subtype %s is %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect type name",buf);
          if (lookup_type(w1)>-1) error("Type already declared",buf);
          xxx=lookup_type(w2);
          if (xxx>-1) {
            memcpy(&typ[top_typ],&typ[xxx],sizeof(struct typedesc));
            if (typ[top_typ].type!=TYPE_UNSIGNED)
              if (typ[top_typ].type!=TYPE_SIGNED)
                if (typ[top_typ].type!=TYPE_FLOAT)
                  error("Subtype of complex types not allowed",buf);
          } else {
            if (!strcmp(w2,"unsigned")) typ[top_typ].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) typ[top_typ].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) typ[top_typ].type=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) typ[top_typ].type=TYPE_IPADDR; else
              error("Unknown type for subtype declaration",buf);
            typ[top_typ].size=1;
          }

          typ[top_typ].btype_comp=1;
          strcpy(typ[top_typ].name,w1);

          top_typ++;
          continue;
        }

        if (sscanf(buf,"type %s is %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect type name",buf);
          if (lookup_type(w1)>-1) error("Type already declared",buf);
          xxx=lookup_type(w2);
          if (xxx>-1) {
            memcpy(&typ[top_typ],&typ[xxx],sizeof(struct typedesc));
          } else {
            if (!strcmp(w2,"unsigned")) typ[top_typ].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) typ[top_typ].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) typ[top_typ].type=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) typ[top_typ].type=TYPE_IPADDR; else
              error("Unknown base type for type declaration",buf);
            typ[top_typ].size=1;
          }

          strcpy(typ[top_typ].name,w1);

          top_typ++;
          continue;
        }

        if (sscanf(buf,"procedure %s %[(]",wai,dummy)==2) {
          state=STATE_FNDEF;
          printk("+ Parsing function %s declaration...\n",wai);
          now_at=0;
          if (!validate_name(wai)) error("Incorrect function name",buf);
          strcpy(fn[top_fn].name,"FN_");
          strcat(fn[top_fn].name,wai);
          if (lookup_fn(fn[top_fn].name)>-1) error("Function already declared",buf);
//          fprintf(out,".CODE\n");
          fprintf(out,":%s\n",fn[top_fn].name);
          continue;
        }


        if (sscanf(buf,"%s : %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          strcpy(sym[top_sym].name,"GV_");
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);

          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { sym[top_sym].type=xxx; sym[top_sym].issim=0;
            sprintf(w1,"0 repeat %d",typ[xxx].size);
            sym[top_sym].in=strdup(w1);
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) sym[top_sym].type=TYPE_IPADDR; else
              error("Unknown type for subtype declaration",buf);
            sprintf(w1,"0");
            sym[top_sym].in=strdup(w1);

          }

          top_sym++;

          continue;
        }


        if (sscanf(buf,"%s : addressable %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          strcpy(sym[top_sym].name,"GV_");
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_ADDRESSABLE;
          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { 
            sym[top_sym].type=xxx; sym[top_sym].issim=0;
            sprintf(w1,"0 repeat %d",typ[xxx].size);
            sym[top_sym].in=strdup(w1);
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) sym[top_sym].type=TYPE_IPADDR; else
              error("Unknown type for subtype declaration",buf);
            sprintf(w1,"0");
            sym[top_sym].in=strdup(w1);
          }

          top_sym++;

          continue;
        }


        if (sscanf(buf,"%s : %s := %[{]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          strcpy(sym[top_sym].name,"GV_");
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].in=strdup("");
          xxx=lookup_type(w2);
          if (xxx>-1) sym[top_sym].type=xxx; else
            error("Unknown structural type",buf);

          if (typ[xxx].type!=TYPE_ARRAY)
            if (typ[xxx].type!=TYPE_STRUCT)
              error("Structural initializer for simple type",buf);

          if (typ[xxx].type==TYPE_ARRAY) now_at=typ[xxx].a_start; else
            now_at=0;

          state=STATE_GDEF;
          printk("+ Parsing global initializer for variable %s...\n",w1);


          continue;
        }


        if (sscanf(buf,"%s : addressable %s := %[{]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          strcpy(sym[top_sym].name,"GV_");
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_ADDRESSABLE;
          sym[top_sym].in=strdup("");
          xxx=lookup_type(w2);
          if (xxx>-1) sym[top_sym].type=xxx; else
            error("Unknown structural type",buf);

          if (typ[xxx].type!=TYPE_ARRAY)
            if (typ[xxx].type!=TYPE_STRUCT)
              error("Structural initializer for simple type",buf);

          if (typ[xxx].type==TYPE_ARRAY)
            if (!typ[xxx].a_simple)
              if ((typ[typ[xxx].a_type].type==TYPE_ARRAY) ||
                  (typ[typ[xxx].a_type].type==TYPE_STRUCT))
                    error("Multi-dimensional initialisers not allowed",buf);

          if (typ[xxx].type==TYPE_ARRAY) now_at=typ[xxx].a_start; else
            now_at=0;

          state=STATE_GDEF;
          printk("+ Parsing global initializer for variable %s...\n",w1);

          continue;
        }

        if (sscanf(buf,"%s : %s := %[ -~]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          strcpy(sym[top_sym].name,"GV_");
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);

          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { 
            sym[top_sym].type=xxx; sym[top_sym].issim=0;
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
              error("Unknown type for subtype declaration",buf);
          }

          if (sym[top_sym].issim)
            xxx=sym[top_sym].type;
          else xxx=typ[sym[top_sym].type].type;

          if (isdigit(dummy[0]) || dummy[0]=='-') {
            if (xxx!=TYPE_UNSIGNED)
              if (xxx!=TYPE_SIGNED)
                if (xxx!=TYPE_FLOAT) error("Senseless initializer (complex type)",buf);
          } else {
            if ((dummy[0]=='"') && (xxx==TYPE_BCHUNK)) {
            // ok
            } else error("Senseless initializer (must be simple)",buf);
          }

          dummy[strlen(dummy)-1]=0;
          sym[top_sym].in=strdup(dummy);

          top_sym++;
          continue;
        }


        if (sscanf(buf,"%s : addressable %s := %[ -~]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          strcpy(sym[top_sym].name,"GV_");
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_ADDRESSABLE;
          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { sym[top_sym].type=xxx; sym[top_sym].issim=0;
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
              error("Unknown type for subtype declaration",buf);
          }

          if (sym[top_sym].issim)
            xxx=sym[top_sym].type;
          else xxx=typ[sym[top_sym].type].type;

          if (isdigit(dummy[0]) || dummy[0]=='-') {
            if (xxx!=TYPE_UNSIGNED)
              if (xxx!=TYPE_SIGNED)
                if (xxx!=TYPE_FLOAT) error("Senseless initializer (complex type)",buf);
          } else {
            if ((dummy[0]=='"') && (xxx==TYPE_BCHUNK)) {
              if ((strlen(dummy)-2)!=(typ[xxx].size)) error("Incorrect size of bchunk initializer",buf);
            } else error("Senseless initializer (must be simple)",buf);
          }

          dummy[strlen(dummy)-1]=0;
          sym[top_sym].in=strdup(dummy);

          top_sym++;

          continue;
        }



        if (sscanf(buf,"%s : pointer to %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          strcpy(sym[top_sym].name,"GV_");
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_POINTER;
          xxx=lookup_type(w2);
          sym[top_sym].issim=1;
          if (xxx>-1) { sym[top_sym].type=xxx; sym[top_sym].issim=0;

          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
            if (!strcmp(w2,"bytechunk")) sym[top_sym].type=TYPE_BCHUNK; else
            if (!strcmp(w2,"proc_addr")) sym[top_sym].type=TYPE_IPADDR; else
              error("Unknown type for subtype declaration",buf);
          }

          sym[top_sym].in="0";

          top_sym++;

          continue;
        }

        error("Unparsable statement within main code",buf);        

        break;

      case STATE_STRUCT:

        if (buf[0]=='}') {
          if (!now_at) error("Empty structure",buf);
          typ[top_typ].s_fields=now_at;
          state=STATE_NONE;
          printk("+ End of structure.\n");
          top_typ++;
          continue;
        }

        if (sscanf(buf,"%s : %s %[;]",w1,w2,dummy)==3) {

          if (!validate_name(w1)) error("Incorrect field name",buf);
          if (lookup_field(top_typ,now_at,w1)>-1) error("Field already declared",buf);
          strcpy(typ[top_typ].s_field[now_at],w1);

          xxx=lookup_type(w2);
          typ[top_typ].issim[now_at]=1;

          if (xxx>-1) { typ[top_typ].s_type[now_at]=xxx; 
            typ[top_typ].issim[now_at]=0;
            typ[top_typ].size+=typ[xxx].size;
          } else {
            if (!strcmp(w2,"unsigned")) typ[top_typ].s_type[now_at]=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) typ[top_typ].s_type[now_at]=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) typ[top_typ].s_type[now_at]=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) typ[top_typ].s_type[now_at]=TYPE_IPADDR; else
              error("Unknown type for field declaration",buf);
            typ[top_typ].size+=1;
          }

          typ[top_typ].s_flag[now_at]=V_NORMAL;

          now_at++;

          continue;
        }


        if (sscanf(buf,"%s : addressable %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect field name",buf);
          if (lookup_field(top_typ,now_at,w1)>-1) error("Field already declared",buf);
          strcpy(typ[top_typ].s_field[now_at],w1);

          xxx=lookup_type(w2);
          typ[top_typ].issim[now_at]=1;

          if (xxx>-1) { typ[top_typ].s_type[now_at]=xxx; 
            typ[top_typ].issim[now_at]=0;
            typ[top_typ].size+=typ[xxx].size;
          } else {
            if (!strcmp(w2,"unsigned")) typ[top_typ].s_type[now_at]=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) typ[top_typ].s_type[now_at]=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) typ[top_typ].s_type[now_at]=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) typ[top_typ].s_type[now_at]=TYPE_IPADDR; else
              error("Unknown type for field declaration",buf);
            typ[top_typ].size+=1;
          }

          typ[top_typ].s_flag[now_at]=V_ADDRESSABLE;

          now_at++;
          continue;
        }


        if (sscanf(buf,"%s : pointer to %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect field name",buf);
          if (lookup_field(top_typ,now_at,w1)>-1) error("Field already declared",buf);
          strcpy(typ[top_typ].s_field[now_at],w1);

          xxx=lookup_type(w2);
          typ[top_typ].issim[now_at]=1;

          if (xxx>-1) { 

            typ[top_typ].s_type[now_at]=xxx; 
            typ[top_typ].issim[now_at]=0;
          } else {
            if (!strcmp(w2,"unsigned")) typ[top_typ].s_type[now_at]=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) typ[top_typ].s_type[now_at]=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) typ[top_typ].s_type[now_at]=TYPE_FLOAT; else
            if (!strcmp(w2,"bytechunk")) typ[top_typ].s_type[now_at]=TYPE_BCHUNK; else
            if (!strcmp(w2,"proc_addr")) typ[top_typ].s_type[now_at]=TYPE_IPADDR; else
              error("Unknown type for field declaration",buf);
          }

          typ[top_typ].size+=1;
          typ[top_typ].s_flag[now_at]=V_POINTER;
          now_at++;
          continue;
        }

        error("Unparsable struct entry",buf);

        break;

      case STATE_GDEF:

        if (buf[0]=='}') {
          if (typ[xxx].type==TYPE_ARRAY) {
            if ((now_at-1)!=typ[xxx].a_end) error("Incorrect number of fields",buf);
          } else if (now_at!=typ[xxx].s_fields) error("Incorrect number of fields",buf);
          top_sym++;
          state=STATE_NONE;
          printk("+ End of initializer.\n");
          continue;
        }

        if (buf[strlen(buf)-1]==',') buf[strlen(buf)-1]=0;
        fix_line(buf);
        if (buf[strlen(buf)-1]==' ') buf[strlen(buf)-1]=0;

        if (typ[xxx].type==TYPE_ARRAY) {
          if (typ[xxx].a_simple) { extyp=typ[xxx].a_type; exsiz=1; }
            else { extyp=typ[typ[xxx].a_type].type; exsiz=typ[typ[xxx].a_type].size; }
        } else {
          if (typ[xxx].s_flag[now_at] & V_POINTER) exsiz=1;
          if (typ[xxx].issim[now_at]) { extyp=typ[xxx].s_type[now_at]; exsiz=1; }
            else { extyp=typ[typ[xxx].s_type[now_at]].type; exsiz=typ[typ[xxx].s_type[now_at]].size; }
        }

        if (!strcmp(buf,"none")) {
          sprintf(dummy,"%s\n:unused_%d\n  0 repeat %d",sym[top_sym].in,unused++,exsiz);
          if (sym[top_sym].in) free(sym[top_sym].in);
          sym[top_sym].in=strdup(dummy);
          now_at++;
          continue;
        }

        if (typ[xxx].type==TYPE_ARRAY) {
          if (typ[xxx].a_simple) { extyp=typ[xxx].a_type; exsiz=1; }
            else { extyp=typ[typ[xxx].a_type].type; exsiz=typ[typ[xxx].a_type].size; }
        } else {
          if (typ[xxx].s_flag[now_at] & V_POINTER) {
             error("Cannot initialize pointers, use 'none' to skip",buf);
          }
          if (typ[xxx].issim[now_at]) { extyp=typ[xxx].s_type[now_at]; exsiz=1; }
            else { extyp=typ[typ[xxx].s_type[now_at]].type; exsiz=typ[typ[xxx].s_type[now_at]].size; }
        }

        if ( (extyp==TYPE_UNSIGNED) || (extyp==TYPE_FLOAT) || (extyp==TYPE_SIGNED) ) {
           if (!(isdigit(buf[0]) || buf[0]=='-')) error("Non-numeric initializer",buf);
           sprintf(dummy,"%s\n:unused_%d\n  %s",sym[top_sym].in,unused++,buf);
           if (sym[top_sym].in) free(sym[top_sym].in);
           sym[top_sym].in=strdup(dummy);
          now_at++;
          continue;
        }

        if (extyp==TYPE_BCHUNK) {
           if (buf[0]!='"') error("Non-text initializer",buf);
           if (exsiz!=((strlen(buf)+1)/4)) error("Incorrect initializer length",buf);
           sprintf(dummy,"%s\n:unused_%d\n  %s",sym[top_sym].in,unused++,buf);
           if (sym[top_sym].in) free(sym[top_sym].in);
           sym[top_sym].in=strdup(dummy);
          now_at++;
          continue;
        }

        error("Cannot initialize complex types, use 'none' to skip",buf);
        continue;


      case STATE_FNDEF:

        // Yo!

        if (sscanf(buf,"%s : %s %[,)]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect parameter name",buf);
          strcpy(fn[top_fn].s_name[now_at],w1);
          if (lookup_par(top_fn,now_at,fn[top_fn].s_name[now_at])>-1) error("Parameter already declared",buf);
          xxx=lookup_type(w2);
          if (xxx>-1) {
            fn[top_fn].s_type[now_at]=xxx;
            fn[top_fn].s_issim[now_at]=0;
          } else {
            fn[top_fn].s_issim[now_at]=1;
            if (!strcmp(w2,"unsigned")) fn[top_fn].s_type[now_at]=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) fn[top_fn].s_type[now_at]=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) fn[top_fn].s_type[now_at]=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) fn[top_fn].s_type[now_at]=TYPE_IPADDR; else
              error("Unknown type for parameter declaration",buf);
          }
          fn[top_fn].writ[now_at]=0;
        } else

        if (sscanf(buf,"writable %s : %s %[),]",w1,w2,dummy)==3) {

          if (!validate_name(w1)) error("Incorrect parameter name",buf);
          strcpy(fn[top_fn].s_name[now_at],w1);
          if (lookup_par(top_fn,now_at,fn[top_fn].s_name[now_at])>-1) error("Parameter already declared",buf);
          xxx=lookup_type(w2);
          if (xxx>-1) {
            fn[top_fn].s_type[now_at]=xxx;
            fn[top_fn].s_issim[now_at]=0;
          } else {
            fn[top_fn].s_issim[now_at]=1;
            if (!strcmp(w2,"unsigned")) fn[top_fn].s_type[now_at]=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) fn[top_fn].s_type[now_at]=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) fn[top_fn].s_type[now_at]=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) fn[top_fn].s_type[now_at]=TYPE_IPADDR; else
              error("Unknown type for parameter declaration",buf);
          }
          fn[top_fn].writ[now_at]=1;

        } else {
          if (!strcmp(buf,") {")) {
             fn[top_fn].params=0; top_fn++; state=STATE_INIT;
             continue;
          } else error("Unparsable parameter declaration",buf);
        }

        now_at++;

        if (buf[strlen(buf)-1]=='{') {
          if (!strcmp(wai,"main"))
            error("Procedure Main() should have no parameters.",buf);
          fn[top_fn].params=now_at;
          top_fn++;
          state=STATE_INIT;
        }

        break;

      case STATE_INIT:

        if (!strcmp(buf,"local {")) {
          nesting_level=1;
          state=STATE_LDEF;
          printk("+   Parsing local variables.\n");

          continue;
        }

        state=STATE_CODE;
        printk("+   Parsing code.\n");

        nesting_level=1;

      case STATE_EXCEPT:

      case STATE_CODE:

        if (state==STATE_EXCEPT) isanexception=1;
          else isanexception=0;

      // Phun starts here!

      if (buf[0]==':') {
        sscanf(buf,": %s %[ -~]",dummy,w1);
        strcpy(buf,w1);
        fprintf(out,":CLabel_%s_%s\n",wai,dummy);
      }


      if (!strcmp(buf,"halt ;")) {
        fprintf(out,"  HALT\n");
        continue;
      }

      if (!strcmp(buf,"return ;")) {
        fprintf(out,"  RET %d\n",1+isanexception);
        continue;
      }

      if (!strcmp(buf,"ignore ;")) {
        if (isanexception) fprintf(out,"  RET 1\n");
        continue;
      }

      if (buf[strlen(buf)-1]=='{') nesting_level++;
      if (buf[strlen(buf)-1]=='}') nesting_level--;

        if (!strcmp(buf,"exception {")) {
          fprintf(out,"  JMP :ExAfter_%s\n",wai);
          fprintf(out,":ExHndlr_%s\n",wai);
          state=STATE_EXCEPT;
          printk("+   Parsing exception handler.\n");

          strcpy(buf,"switch internalexception {");
        }

//      printk("BUF: [%s] nesting=%d tblk=%d\n",buf,nesting_level,top_blk);

      if (top_blk>0)
        while (blk[top_blk-1].nest>nesting_level) {
          if (top_blk<=0) break;
          top_blk--;
          fprintf(out,"  NOP\n");
          switch (blk[top_blk].type) {
            case P_LOOP:

              fprintf(out,":Loop_%d_Check\n",blk[top_blk].no);
              fprintf(out,"%s\n",blk[top_blk].checkc);
              fprintf(out,"    JMP :Loop_%d_Start\n",blk[top_blk].no);

              fprintf(out,":Loop_%d_Exit\n",blk[top_blk].no);
              fprintf(out,"  NOP\n");
              break;

            case P_IF:

              fprintf(out,":If_%d_Exit\n",blk[top_blk].no);
              fprintf(out,"  NOP\n");
              break;

            case P_CASE:

              inswitch=0;
              fprintf(out,":Case_%d_Entry_%d\n",blk[top_blk].no,hadcase+1);
              fprintf(out,"  NOP\n");
              fprintf(out,":Case_%d_Exit\n",blk[top_blk].no);
              fprintf(out,"  NOP\n");
              break;

            case P_GUARD:

              fprintf(out,"  NOFAIL\n");

          }
        }

      if (!nesting_level) {
        if (isanexception) {
          fprintf(out,":Case_%d_Entry_%d\n",blk[top_blk].no,hadcase);
          fprintf(out,"  NOP\n");
          fprintf(out,":Case_%d_Exit\n",blk[top_blk].no);
          fprintf(out,"  NOP\n");
          fprintf(out,"  RAISE *:Switch_Value\n");
          fprintf(out,":ExAfter_%s\n",wai);
        }
        if (!strcmp(wai,"main")) fprintf(out,"  HALT\n"); else
        fprintf(out,"  RET 1\n");
        state=STATE_NONE;
        printk("+ End of function.\n");

        continue;
      }

      if (sscanf(buf,"raise %s %[;]",w1,dummy)==2) {
        if (!write_value_in_u0(w1,wai))
          error("Simple variable expected",buf);
        fprintf(out,"  RAISE u0\n");
        continue;
      }

      if (!strcmp(buf,"continue ;")) {
        int n=top_blk,got=0;
        while ((--n)>=0) {
          switch (blk[n].type) {

            case P_LOOP:

              fprintf(out,"  JMP :Loop_%d_Check\n",blk[n].no);
              n=-10; got=1;
              break;

            case P_IF: break;
            case P_CASE: fprintf(out,"  JMP :Case_%d_Exit\n",blk[n].no); n=-10; got=1; break;
            case P_GUARD: break;
         }
       }
       if (!got) error("'continue' outside loop",buf);
     }

      if (!strcmp(buf,"break ;")) {
        int n=top_blk,got=0;
        while ((--n)>=0) {
          switch (blk[n].type) {

            case P_LOOP:

              fprintf(out,"  JMP :Loop_%d_Exit\n",blk[n].no);
              n=-10; got=1;
              break;

            case P_IF: break;
            case P_CASE: fprintf(out,"  JMP :Case_%d_Exit\n",blk[n].no); n=-10; got=1; break;
            case P_GUARD: break;
         }
       }
       if (!got) error("'break' outside loop",buf);
        continue;
     }


      if (sscanf(buf,"twait %s %[;]",w1,dummy)==2) {
        if (!write_value_in_u0(w1,wai))
          error("Simple variable expected",buf);
        fprintf(out,"  TWAIT u0\n");
        continue;
      }

      if (sscanf(buf,"cwait %s %[;]",w1,dummy)==2) {
        if (!write_value_in_u0(w1,wai))
          error("Simple variable expected",buf);
        fprintf(out,"  CWAIT u0\n");
        continue;
      }

      if (sscanf(buf,"goto %s %[;]",w1,dummy)==2) {
        fprintf(out,"  JMP :CLabel_%s_%s\n",wai,w1);
        continue;
      }


      if (sscanf(buf,"__assembler__ \"%[^\"]",w1)==1) {
        fprintf(out,"  %s     # Implicit assembly\n",w1);
        continue;
      }

      nodo=0;
      if (!strncmp(buf,"while ",6)) {
        nodo=1;
        memcpy(buf,"loop  ",6);
      }

      if (sscanf(buf,"loop %s %[{]",w1,dummy)==2) {
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_LOOP;
        if (nodo) fprintf(out,"  JMP :Loop_%d_Check\n",nest_cnt);
          else fprintf(out,"  NOP\n");
        fprintf(out,":Loop_%d_Start\n",nest_cnt);
        dummy[0]=0;

        if (!put_value_in_reg(dummy,w1,wai,"u0"))
          error("Complex type used as loop condition",buf);

        sprintf(dummy+strlen(dummy),"  IFNEQ u0,0\n");

        blk[top_blk].checkc=strdup(dummy);

        nest_cnt++;
        top_blk++;
        continue;
      }

      if (sscanf(buf,"loop not %s %[{]",w1,dummy)==2) {
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_LOOP;
        if (nodo) fprintf(out,"  JMP :Loop_%d_Check\n",nest_cnt);
          else fprintf(out,"  NOP\n");
        fprintf(out,":Loop_%d_Start\n",nest_cnt);
        dummy[0]=0;

        if (!put_value_in_reg(dummy,w1,wai,"u0"))
          error("Complex type used as loop condition",buf);

        sprintf(dummy+strlen(dummy),"  IFEQ u0,0\n");

        blk[top_blk].checkc=strdup(dummy);

        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"loop %s = %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_LOOP;

        if (nodo) fprintf(out,"  JMP :Loop_%d_Check\n",nest_cnt);
          else fprintf(out,"  NOP\n");
        fprintf(out,":Loop_%d_Start\n",nest_cnt);

        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        sprintf(blk[top_blk].checkc,"@%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        sprintf(dummy+strlen(dummy),"  MOV s0,%d\n",dest_size);
        if (!is_immed) {
          sprintf(dummy+strlen(dummy),"  CMPCNT u2,u1\n");
          sprintf(dummy+strlen(dummy),"  IFEQ s0,0\n");
        } else {
          sprintf(dummy+strlen(dummy),"  IFEQ *u2,u1\n");
        }
        blk[top_blk].checkc=strdup(dummy);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"loop %s < %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_LOOP;

        if (nodo) fprintf(out,"  JMP :Loop_%d_Check\n",nest_cnt);
          else fprintf(out,"  NOP\n");
        fprintf(out,":Loop_%d_Start\n",nest_cnt);

        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        sprintf(dummy+strlen(dummy),"  IFBEL u1,u2\n");

        blk[top_blk].checkc=strdup(dummy);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"loop %s > %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_LOOP;

        if (nodo) fprintf(out,"  JMP :Loop_%d_Check\n",nest_cnt);
          else fprintf(out,"  NOP\n");
        fprintf(out,":Loop_%d_Start\n",nest_cnt);


        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        sprintf(dummy+strlen(dummy),"  IFABO u1,u2\n");

        blk[top_blk].checkc=strdup(dummy);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"loop %s >= %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_LOOP;

        if (nodo) fprintf(out,"  JMP :Loop_%d_Check\n",nest_cnt);
          else fprintf(out,"  NOP\n");
        fprintf(out,":Loop_%d_Start\n",nest_cnt);


        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        sprintf(dummy+strlen(dummy),"  IFABO u1,u2\n");
        sprintf(dummy+strlen(dummy),"    JMP :Loop_%d_Start\n",nest_cnt);
        sprintf(dummy+strlen(dummy),"  IFEQ u1,u2\n");

        blk[top_blk].checkc=strdup(dummy);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"loop %s <= %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_LOOP;

        if (nodo) fprintf(out,"  JMP :Loop_%d_Check\n",nest_cnt);
          else fprintf(out,"  NOP\n");
        fprintf(out,":Loop_%d_Start\n",nest_cnt);


        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        sprintf(dummy+strlen(dummy),"  IFBEL u1,u2\n");
        sprintf(dummy+strlen(dummy),"    JMP :Loop_%d_Start\n",nest_cnt);
        sprintf(dummy+strlen(dummy),"  IFEQ u1,u2\n");

        blk[top_blk].checkc=strdup(dummy);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"if %s %[{]",w1,dummy)==2) {
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        if (!write_value_in_u0(w1,wai))
          error("Simple variable expected",buf);
        fprintf(out,"  IFEQ u0,0\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"if %s = %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        if (is_immed) error("First parameter shoudn't be an immediate value",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"@%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  MOV s0,%d\n",dest_size);
        if (!is_immed) {
          fprintf(out,"  CMPCNT u2,u1\n");
          fprintf(out,"  IFNEQ s0,0\n");
        } else {
          fprintf(out,"  IFNEQ *u2,u1\n");
        }
        fprintf(out,  "    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }

      if (sscanf(buf,"if %s < %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  IFABO u1,u2\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        fprintf(out,"  IFEQ u1,u2\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }

      if (sscanf(buf,"if %s > %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  IFBEL u1,u2\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        fprintf(out,"  IFEQ u1,u2\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }

      if (sscanf(buf,"if %s <= %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  IFABO u1,u2\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }

      if (sscanf(buf,"if %s >= %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  IFBEL u1,u2\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"if %s & %s %[{]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic comparsion",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  AND u1,u2\n");
        fprintf(out,"  IFEQ u1,0\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }



      if (sscanf(buf,"if not %s %[{]",w1,dummy)==2) {
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_IF;
        if (!write_value_in_u0(w1,wai))
          error("Simple variable expected",buf);
        fprintf(out,"  IFNEQ u0,0\n");
        fprintf(out,"    JMP :If_%d_Exit\n",nest_cnt);
        nest_cnt++;
        top_blk++;
        continue;
      }

      if (sscanf(buf,"switch %s %[{]",w1,dummy)==2) {
        if (inswitch) error("No switch {} nesting allowed",buf);
        inswitch=nest_cnt;
        hadcase=0;
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_CASE;
        if (strcmp(w1,"internalexception"))
          if (!write_value_in_u0(w1,wai))
            error("Simple variable expected",buf);
        fprintf(out,"  MOV *:Switch_Value,u0\n");
        fprintf(out,"  JMP :Case_%d_Entry_1\n",inswitch);
        nest_cnt++;
        top_blk++;
        continue;
      }


      if (sscanf(buf,"case %i %[;]",&jakisint,dummy)==2) {
        if (!inswitch) error("case outside switch {}",buf);
        fprintf(out,"  JMP :Case_%d_Exit\n",inswitch);
        hadcase++;
        fprintf(out,":Case_%d_Entry_%d\n",inswitch,hadcase);
        fprintf(out,"  IFNEQ *:Switch_Value,%d\n",jakisint);
        fprintf(out,"    JMP :Case_%d_Entry_%d\n",inswitch,hadcase+1);
        continue;
      }

      if (sscanf(buf,"case default %[;]",dummy)==1) {
        if (!inswitch) error("case outside switch {}",buf);
        fprintf(out,"  JMP :Case_%d_Exit\n",inswitch);
        hadcase++;
        fprintf(out,":Case_%d_Entry_%d\n",inswitch,hadcase);
        continue;
      }


      if (!strcmp(buf,"guard {")) {
//        if (nesting_level!=2) 
//          error("guard {} block allowed only at base function level",buf);
        blk[top_blk].nest=nesting_level;
        blk[top_blk].no=nest_cnt;
        blk[top_blk].type=P_GUARD;
        fprintf(out,"  ONFAIL :ExHndlr_%s\n",wai);
        continue;
      }

      if (!strcmp(buf,"syscall (")) {
        state=STATE_SYSCALL;
        param_already=0; sosen=-1;
        continue;
      }

      if (sscanf(buf,"%s %[(]",w1,dummy)==2) {
        sprintf(wtc,"FN_%s",w1);
        if ((xxx=lookup_fn(wtc))<0) error("Undefined function",buf);
        state=STATE_FNPAR;
        sosen=0;
        continue;
      }

      if (sscanf(buf,"bind %s to %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w2);
        DO_NOT_FOLLOW_POINTERS=1;
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        DO_NOT_FOLLOW_POINTERS=0;
        if (!is_addressable) error("Cannot bind non-addressable variable",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"@%s",w1);
        DO_NOT_FOLLOW_POINTERS=1;
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        DO_NOT_FOLLOW_POINTERS=0;
        if (!is_pointer) error("Cannot bind to non-pointer",buf);
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  IFNEQ *u1,0\n");
        fprintf(out,"    RAISE 10004 # Pointer already binded\n");
        fprintf(out,"  MOV *u1,u2\n");
        continue;
      }

      if (sscanf(buf,"destroy %s %[;]",w1,dummy)==2) {
        blk[top_blk].checkc=malloc(1000);
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"@%s",w1);
        DO_NOT_FOLLOW_POINTERS=1;
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        DO_NOT_FOLLOW_POINTERS=0;
        if (!is_pointer) error("Cannot destroy non-pointer",buf);
        fputs(dummy,out);
        fprintf(out,"  IFEQ *u1,0\n");
        fprintf(out,"    RAISE 10001 # Pointer not binded\n");
        fprintf(out,"  MOV u2,*u1\n");
        fprintf(out,"  DIV u2,%d\n",(MAX_ALLOC_MEMBLK/2));
        fprintf(out,"  IFEQ u2,0\n");
        fprintf(out,"    RAISE 10005 # Not a dynamic pointer\n");
        fprintf(out,"  DEALLOC u2\n");
        fprintf(out,"  MOV *u1,0\n");
        continue;
      }

      if (sscanf(buf,"create %s %[;]",w1,dummy)==2) {
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        DO_NOT_FOLLOW_POINTERS=1;
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        DO_NOT_FOLLOW_POINTERS=0;
        if (!is_pointer) error("Cannot destroy non-pointer",buf);
        fputs(dummy,out);
        fprintf(out,"  IFNEQ *u2,0\n");
        fprintf(out,"    RAISE 10004 # Pointer already binded\n");
        fprintf(out,"  ALLOC %d,3\n",dest_size);
        fprintf(out,"  MOV *u2,u1\n");
        continue;
      }


      if (sscanf(buf,"unbind %s %[;]",w1,dummy)==2) {
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        DO_NOT_FOLLOW_POINTERS=1;
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        DO_NOT_FOLLOW_POINTERS=0;
        if (!is_pointer) error("Cannot unbindy non-pointer",buf);
        fputs(dummy,out);
        fprintf(out,"  IFEQ *u1,0\n");
        fprintf(out,"    RAISE 10001 # Pointer not binded\n");
        fprintf(out,"  MOV *u1,0\n");
        continue;
      }



      if (sscanf(buf," %s += %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  ADD *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s := address %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"@%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (t1!=TYPE_UNSIGNED)
            error("Address is not compatible with this type",buf);
        if (!si1) error("Cannot assign address to complex variable",buf);
        fprintf(out,"  MOV *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s -= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  SUB *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s /= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  DIV *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s *= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  MUL *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s ~= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  NOT u2\n");
        fprintf(out,"  MOV *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s ^= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  XOR *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s %%= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  MOD *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s |= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  OR *u1,u2\n");
        continue;
      }

      if (sscanf(buf," %s &= %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        sprintf(blk[top_blk].checkc,"%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (!is_simple) error("Complex algebraic operation",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  AND *u1,u2\n");
        continue;
      }

      if (sscanf(buf,"%s := %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        if (t1!=its_type) error("Type mismatch",buf);
        fprintf(out,"  MOV s0,%d\n",dest_size);
        if (!is_immed) fprintf(out,"  CPCNT u1,u2\n");
        else fprintf(out,"  MOV *u1,u2\n");
        continue;
      }

      // Forced conversion operator:

      if (sscanf(buf,"%s := convert %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        fputs(dummy,out);
        if (dest_size!=s1) error("Symbol size mismatch",buf);
        if (is_simple!=si1) error("Simple type compared to complex type",buf);
        fprintf(out,"  MOV s0,%d\n",dest_size);
        if (!is_immed) fprintf(out,"  CPCNT u1,u2\n");
        else fprintf(out,"  MOV *u1,u2\n");
        continue;
      }


      if (sscanf(buf,"%s := length %s %[;]",w1,w2,dummy)==3) {
        int si1,t1,s1;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w1);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
        if (!isitwrit) error("Cannot modify non-writable parameter",buf);
        fputs(dummy,out);
        si1=is_simple;
        s1=dest_size;
        t1=its_type;
        dummy[0]=0;
        blk[top_blk].checkc=malloc(1000);
        sprintf(blk[top_blk].checkc,"@%s",w2);
        put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u2");
        if (!si1) error("Simple type expected",buf);
        fprintf(out,"  MOV *u1,%d\n",is_immed?1:dest_size);
        continue;
      }





      if (buf[0]!='}')
        error("Unparsable code statement",buf);

      continue;

      case STATE_SYSCALL:

        if (buf[strlen(buf)-1]==';') {
          if (isanexception) state=STATE_EXCEPT; else state=STATE_CODE;
        }

        if (sosen<0) {
          sscanf(buf,"%i",&sosen);
          if (sosen<0) error("Bad syscall number",buf);
          continue;
        }

        if (sscanf(buf,"%s := %s",w1,w2)==2) {
          char rt;
          int rn;
          if ((w1[0]=='u'||w1[0]=='s'||w1[0]=='f') && sscanf(w1,"%c%d",&rt,&rn)==2) {
            if (param_already) error("Bad paremeters / results order",buf);
            if (!strchr("usf",rt)) error("Unknown register type",buf);
            if ((rn<0) || (rn>=REGISTERS)) error("Unknown register number",buf);
            dummy[0]=0;
            if (!put_value_in_reg(dummy,w2,wai,"u15"))
              error("Simple type expected",buf);
            fputs(dummy,out);
            if (w1[0]=='f') {
              fprintf(out,"# We have to avoid value conversion.\n");
              fprintf(out,"  MOV *:TempFloat,u15\n");
              fprintf(out,"  MOV %s,*:TempFloat\n",w1);
            } else {
              fprintf(out,"  MOV %s,u15\n",w1);
            }
            if (state==STATE_CODE) {
              fprintf(out,"  SYSCALL %d # No return values\n",sosen);
            }

            continue;
            
          } else {


              if (!param_already) {
                param_already=1;
                fprintf(out,"  SYSCALL %d\n",sosen);
              }

              {
                 dummy[0]=0;
                 blk[top_blk].checkc=malloc(1000);
                 sprintf(blk[top_blk].checkc,"@%s",w1);
                 put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u14");
                 fputs(dummy,out);
                 if (!is_simple) error("Complex assignment not allowed here",buf);
                 fprintf(out,"  MOV *u14,%s\n",w2);
                 continue;
              }
  
              continue;
  
          }

        }

        error("Unparsable syscall",buf);

        continue;
 

      // Phun continues...

      case STATE_FNPAR:

        if (sscanf(buf,"length %s",w1)==1) {
          dummy[0]=0;
          blk[top_blk].checkc=malloc(1000);
          sprintf(blk[top_blk].checkc,"@%s",w1);
          put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
          sprintf(dummy,"%d",is_immed?1:dest_size);
          memset(buf,' ',strlen(w1)+7);
          memcpy(buf,dummy,strlen(dummy));
        }


        if (sscanf(buf,"convert %s",w1)==1) {
          int si;
          dummy[0]=0;
          blk[top_blk].checkc=malloc(1000);
          sprintf(blk[top_blk].checkc,"@%s",w1);
          put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
          fputs(dummy,out);
          if (is_simple) si=1; else si=typ[fn[xxx].s_type[sosen]].size;
          if (is_immed) {
            if (fn[xxx].writ[sosen]) error("Cannot pass immediate values as writable parameters",buf);
            fprintf(out,".DATA\n");
            fprintf(out,":immediate_thing_%d\n",immthing);
            fprintf(out,"  %s\n",w1);
            fprintf(out,".CODE\n");
            fprintf(out,"  MOV u1,:immediate_thing_%d\n",immthing);
            immthing++;
          }

          if (dest_size!=si) error("Symbol size mismatch",buf);
          else fprintf(out,"  MOV *:PPtr_%s_%s,u1\n",wtc,fn[xxx].s_name[sosen]);
        } else
        if (sscanf(buf,"address %s",w1)==1) {
          int si;
          dummy[0]=0;
          blk[top_blk].checkc=malloc(1000);
          sprintf(blk[top_blk].checkc,"@%s",w1);
          put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
          fputs(dummy,out);
          if (is_simple) si=1; else si=typ[fn[xxx].s_type[sosen]].size;
          if (is_immed) error("Cannot get immediate value address",buf);
          fprintf(out,".DATA\n");
          fprintf(out,":immediate_thing_%d\n",immthing);
          fprintf(out,"  0\n");
          fprintf(out,".CODE\n");
          fprintf(out,"  MOV *:immediate_thing_%d,u1\n",immthing);
          fprintf(out,"  MOV u1,:immediate_thing_%d\n",immthing);
          fprintf(out,"  MOV *:PPtr_%s_%s,u1\n",wtc,fn[xxx].s_name[sosen]);
          immthing++;
        } else
        if (sscanf(buf,"%s",w1)==1) {
          dummy[0]=0;
          blk[top_blk].checkc=malloc(1000);
          sprintf(blk[top_blk].checkc,"@%s",w1);
          put_value_in_reg(dummy,blk[top_blk].checkc,wai,"u1");
          fputs(dummy,out);

          if (is_immed) {
            if (fn[xxx].writ[sosen]) error("Cannot pass immediate values as writable parameters",buf);
            fprintf(out,".DATA\n");
            fprintf(out,":immediate_thing_%d\n",immthing);
            fprintf(out,"  %s\n",w1);
            fprintf(out,".CODE\n");
            fprintf(out,"  MOV u1,:immediate_thing_%d\n",immthing);
            immthing++;
          } else {
//            if (fn[xxx].s_type[sosen]!=base_type) error("Type mismatch",buf);
          }
          fprintf(out,"  MOV *:PPtr_%s_%s,u1\n",wtc,fn[xxx].s_name[sosen]);
        }

        sosen++;

        if (buf[strlen(buf)-1]==';') {
          if (sosen!=fn[xxx].params)
            error("Incorrect number of parameters",buf);
          fprintf(out,"  CALL :%s\n",wtc);
          if (isanexception) state=STATE_EXCEPT; else state=STATE_CODE;
        }

        continue;


      case STATE_LDEF:

        if (sscanf(buf,"%s : %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          sprintf(sym[top_sym].name,"LV_%s_",wai);
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);

          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { sym[top_sym].type=xxx; sym[top_sym].issim=0;
            sprintf(w1,"0 repeat %d",typ[xxx].size);
            sym[top_sym].in=strdup(w1);
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) sym[top_sym].type=TYPE_IPADDR; else
              error("Unknown type for subtype declaration",buf);
            sprintf(w1,"0");
            sym[top_sym].in=strdup(w1);

          }

          top_sym++;

          continue;
        }


        if (sscanf(buf,"%s : addressable %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          sprintf(sym[top_sym].name,"LV_%s_",wai);
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_ADDRESSABLE;
          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { 
            sym[top_sym].type=xxx; sym[top_sym].issim=0;
            sprintf(w1,"0 repeat %d",typ[xxx].size);
            sym[top_sym].in=strdup(w1);
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
            if (!strcmp(w2,"proc_addr")) sym[top_sym].type=TYPE_IPADDR; else
              error("Unknown type for subtype declaration",buf);
            sprintf(w1,"0");
            sym[top_sym].in=strdup(w1);
          }

          top_sym++;

          continue;
        }


        if (sscanf(buf,"%s : %s := %[{]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          sprintf(sym[top_sym].name,"LV_%s_",wai);
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].in=strdup("");
          xxx=lookup_type(w2);
          if (xxx>-1) sym[top_sym].type=xxx; else
            error("Unknown structural type",buf);

          if (typ[xxx].type!=TYPE_ARRAY)
            if (typ[xxx].type!=TYPE_STRUCT)
              error("Structural initializer for simple type",buf);

          if (typ[xxx].type==TYPE_ARRAY) now_at=typ[xxx].a_start; else
            now_at=0;

          state=STATE_LNEST;
          printk("+  Parsing local initializer for variable %s.\n",w1);

          continue;
        }


        if (sscanf(buf,"%s : addressable %s := %[{]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          sprintf(sym[top_sym].name,"LV_%s_",wai);
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_ADDRESSABLE;
          sym[top_sym].in=strdup("");
          xxx=lookup_type(w2);
          if (xxx>-1) sym[top_sym].type=xxx; else
            error("Unknown structural type",buf);

          if (typ[xxx].type!=TYPE_ARRAY)
            if (typ[xxx].type!=TYPE_STRUCT)
              error("Structural initializer for simple type",buf);

          if (typ[xxx].type==TYPE_ARRAY)
            if (!typ[xxx].a_simple)
              if ((typ[typ[xxx].a_type].type==TYPE_ARRAY) ||
                  (typ[typ[xxx].a_type].type==TYPE_STRUCT))
                    error("Multi-dimensional initialisers not allowed",buf);

          if (typ[xxx].type==TYPE_ARRAY) now_at=typ[xxx].a_start; else
            now_at=0;

          state=STATE_LNEST;
          printk("+  Parsing local initializer for variable %s.\n",w1);

          continue;
        }

        if (sscanf(buf,"%s : %s := %[ -~]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          sprintf(sym[top_sym].name,"LV_%s_",wai);
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);

          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { 
            sym[top_sym].type=xxx; sym[top_sym].issim=0;
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
              error("Unknown type for subtype declaration",buf);
          }

          if (sym[top_sym].issim)
            xxx=sym[top_sym].type;
          else xxx=typ[sym[top_sym].type].type;

          if (isdigit(dummy[0]) || dummy[0]=='-') {
            if (xxx!=TYPE_UNSIGNED)
              if (xxx!=TYPE_SIGNED)
                if (xxx!=TYPE_FLOAT) error("Senseless initializer (complex type)",buf);
          } else {
            if ((dummy[0]=='"') && (xxx==TYPE_BCHUNK)) {
            // ok
            } else error("Senseless initializer (must be simple)",buf);
          }

          dummy[strlen(dummy)-1]=0;
          sym[top_sym].in=strdup(dummy);

          top_sym++;
          continue;
        }


        if (sscanf(buf,"%s : addressable %s := %[ -~]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          sprintf(sym[top_sym].name,"LV_%s_",wai);
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_ADDRESSABLE;
          xxx=lookup_type(w2);
          sym[top_sym].issim=1;

          if (xxx>-1) { sym[top_sym].type=xxx; sym[top_sym].issim=0;
          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
              error("Unknown type for subtype declaration",buf);
          }

          if (sym[top_sym].issim)
            xxx=sym[top_sym].type;
          else xxx=typ[sym[top_sym].type].type;

          if (isdigit(dummy[0]) || dummy[0]=='-') {
            if (xxx!=TYPE_UNSIGNED)
              if (xxx!=TYPE_SIGNED)
                if (xxx!=TYPE_FLOAT) error("Senseless initializer (complex type)",buf);
          } else {
            if ((dummy[0]=='"') && (xxx==TYPE_BCHUNK)) {
              if ((strlen(dummy)-2)!=(typ[xxx].size)) error("Incorrect size of bchunk initializer",buf);
            } else error("Senseless initializer (must be simple)",buf);
          }

          dummy[strlen(dummy)-1]=0;
          sym[top_sym].in=strdup(dummy);

          top_sym++;

          continue;
        }



        if (sscanf(buf,"%s : pointer to %s %[;]",w1,w2,dummy)==3) {
          if (!validate_name(w1)) error("Incorrect variable name",buf);
          sprintf(sym[top_sym].name,"LV_%s_",wai);
          strcat(sym[top_sym].name,w1);
          if (lookup_sym(sym[top_sym].name)>-1) error("Variable already declared",buf);
          sym[top_sym].flag=V_POINTER;
          xxx=lookup_type(w2);
          sym[top_sym].issim=1;
          if (xxx>-1) { sym[top_sym].type=xxx; sym[top_sym].issim=0;

          } else {
            if (!strcmp(w2,"unsigned")) sym[top_sym].type=TYPE_UNSIGNED; else
            if (!strcmp(w2,"signed")) sym[top_sym].type=TYPE_SIGNED; else
            if (!strcmp(w2,"float")) sym[top_sym].type=TYPE_FLOAT; else
            if (!strcmp(w2,"bytechunk")) sym[top_sym].type=TYPE_BCHUNK; else
            if (!strcmp(w2,"proc_addr")) sym[top_sym].type=TYPE_IPADDR; else
              error("Unknown type for subtype declaration",buf);
          }

          sym[top_sym].in="0";

          top_sym++;

          continue;
        }

        if (buf[strlen(buf)-1]=='}') {
//          printk("WARNING: LOCAL INITIALIZERS SHOULD BE PUT HERE!!!\n");


  {
     int n;

     fprintf(out,"# LOCAL INITIALIZATION CODE\n");
     sprintf(dummy,"LV_%s_",wai);

     for (n=0;n<top_sym;n++) {
       if (!strncmp(dummy,sym[n].name,strlen(dummy))) {
         if (sym[n].issim || (sym[n].flag & V_POINTER)) fprintf(out,"  MOV s0,1\n");
           else fprintf(out,"  MOV s0,%d\n",typ[sym[n].type].size);
         fprintf(out,"  CPCNT :%s,:Ini_%s\n",sym[n].name,sym[n].name);
       }
     }

  }


          state=STATE_CODE;
          printk("+  End of local declarations.\n");
          continue;
        }

        error("Unparsable statement within local defs",buf);        

        break;


      case STATE_LNEST:

        if (buf[0]=='}') {
          if (typ[xxx].type==TYPE_ARRAY) {
            if ((now_at-1)!=typ[xxx].a_end) error("Incorrect number of fields",buf);
          } else if (now_at!=typ[xxx].s_fields) error("Incorrect number of fields",buf);
          top_sym++;
          state=STATE_LDEF;
          printk("+   End of local initializer.\n");
          continue;
        }

        if (buf[strlen(buf)-1]==',') buf[strlen(buf)-1]=0;
        fix_line(buf);
        if (buf[strlen(buf)-1]==' ') buf[strlen(buf)-1]=0;

        if (typ[xxx].type==TYPE_ARRAY) {
          if (typ[xxx].a_simple) { extyp=typ[xxx].a_type; exsiz=1; }
            else { extyp=typ[typ[xxx].a_type].type; exsiz=typ[typ[xxx].a_type].size; }
        } else {
          if (typ[xxx].s_flag[now_at] & V_POINTER) exsiz=1;
          if (typ[xxx].issim[now_at]) { extyp=typ[xxx].s_type[now_at]; exsiz=1; }
            else { extyp=typ[typ[xxx].s_type[now_at]].type; exsiz=typ[typ[xxx].s_type[now_at]].size; }
        }

        if (!strcmp(buf,"none")) {
          sprintf(dummy,"%s\n:unused_%d\n  0 repeat %d",sym[top_sym].in,unused++,exsiz);
          if (sym[top_sym].in) free(sym[top_sym].in);
          sym[top_sym].in=strdup(dummy);
          now_at++;
          continue;
        }

        if (typ[xxx].type==TYPE_ARRAY) {
          if (typ[xxx].a_simple) { extyp=typ[xxx].a_type; exsiz=1; }
            else { extyp=typ[typ[xxx].a_type].type; exsiz=typ[typ[xxx].a_type].size; }
        } else {
          if (typ[xxx].s_flag[now_at] & V_POINTER) {
             error("Cannot initialize pointers, use 'none' to skip",buf);
          }
          if (typ[xxx].issim[now_at]) { extyp=typ[xxx].s_type[now_at]; exsiz=1; }
            else { extyp=typ[typ[xxx].s_type[now_at]].type; exsiz=typ[typ[xxx].s_type[now_at]].size; }
        }

        if ( (extyp==TYPE_UNSIGNED) || (extyp==TYPE_FLOAT) || (extyp==TYPE_SIGNED) ) {
           if (!(isdigit(buf[0]) || buf[0]=='-')) error("Non-numeric initializer",buf);
           sprintf(dummy,"%s\n:unused_%d\n  %s",sym[top_sym].in,unused++,buf);
           if (sym[top_sym].in) free(sym[top_sym].in);
           sym[top_sym].in=strdup(dummy);
          now_at++;
          continue;
        }

        if (extyp==TYPE_BCHUNK) {
           if (buf[0]!='"') error("Non-text initializer",buf);
           if (exsiz!=((strlen(buf)+1)/4)) error("Incorrect initializer length",buf);
           sprintf(dummy,"%s\n:unused_%d\n  %s",sym[top_sym].in,unused++,buf);
           if (sym[top_sym].in) free(sym[top_sym].in);
           sym[top_sym].in=strdup(dummy);
          now_at++;
          continue;
        }

        error("Cannot initialize complex types, use 'none' to skip",buf);
        continue;

      default:
 
        // Old-school ;>

        printk("WARNING: parsing in unimplemented parser state %d\n",state);
        printk("         Line '%s' ignored! No exit condition.\n",buf);
        break;

    }
  }

  {
     int n,q;
     char* x;

     fprintf(out,"\n\n.DATA\n\n# Dumping variables:\n");
     for (n=0;n<top_sym;n++) {
       fprintf(out,":%s\n",sym[n].name);
       q=1;
kurka:
       x=sym[n].in;
       if (strstr(x,":unused_"))
          x=strchr(strchr(sym[n].in,'\n')+1,'\n')+1;
       fprintf(out,"  %s\n",x);
       if ((q) && (sym[n].name[0]=='L')) { 
         q=0;
         fprintf(out,":Ini_%s\n",sym[n].name);
         goto kurka;
       }

     }


     fprintf(out,"\n# Dumping function parameters (POINTERS):\n");

     for (n=0;n<top_fn;n++) {
       for (q=0;q<fn[n].params;q++) {
         fprintf(out,":PPtr_%s_%s%s\n",fn[n].name,fn[n].s_name[q],
         fn[n].writ[q] ? "  # writable" : "");
         fprintf(out,"  0\n");
       }
     }



     fprintf(out,"# End of variables.\n\n.CODE\n");

  }

  printk("=> Compilation done. Parsed %d effective lines, created %d functions,\n"
         "=> %d variables (using %d types), %d blocks.\n",line,top_fn,top_sym,top_typ,nest_cnt);

}





int main(int argc,char* argv[]) {
  char* x;
  printk("%s OS version %d.%03d.%04d *BETA* high-level language translator.\n",SYSNAME,SYS_MAJOR,SYS_MINOR,BUILD);
  printk("(C) 2000 Michal Zalewski <lcamtuf@ids.pl>\n\n");
  if (argc-2) {
    printk("Usage: %s filename.ahl\n"
           "This will produce filename.agt.\n\n",argv[0]);
    exit(1);
  }

  in=fopen(argv[1],"r");
  if (!in) {
    perror("Cannot open input file");
    exit(2);
  }
  x=strrchr(argv[1],'.');
  if (x) *x=0;
  strcat(argv[1],".agt");
  out=fopen(argv[1],"w");
  if (!out) {
    perror("Cannot create output file");
    exit(3);
  }
  fprintf(out,"# AHLL compiler static code:\n\n");
  fprintf(out,"!signature \"untitled AHLL program\"\n");
  fprintf(out,"!priority 1000\n\n");
  nam=argv[1];

  parse_file();
  write_code();
  // do_dependencies();
  // write_code();

  fclose(out);
  printk("Compiled successfully, output file is %s.\n",argv[1]);
  return 0;
}

/*

   Argante virtual OS
   ------------------

   Hierarchical Access Control test.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>
   Patched:    scrippie <scrippie@werule.nl> - "../" thingy

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "console.h"

struct rule_struct {
  int group;
  int uid;
  char sense;
  char object[MAX_OBJPATH];
  char operation[MAX_OPERPATH];
};


struct rule_struct rule[MAX_RULES];
int top_rule,have_rules;


void test_load_rules(char* filename) {
  FILE* f;
  char* x;
  int i;
  char buf[1000];
  f=fopen(filename,"r");
  if (!f) {
    printk("-> ERROR: Cannot open given HAC file.\n");
    return;
  }
  printk("+> Loading access control rules...\n");

  printk("=> Ruleset limit: %d bytes, %d entries.\n",sizeof(rule),MAX_RULES);


  have_rules=0;top_rule=0;

  while (fgets(buf,1000,f)) {
    char blam[100];
    if ((x=strrchr(buf,'#'))) *x=0;
    if ((x=strrchr(buf,'\n'))) *x=0;
    if (!strlen(buf)) continue;
    while (isspace(buf[strlen(buf)-1])) buf[strlen(buf)-1]=0; 
    if (!strlen(buf)) continue;
    for (i=0;i<strlen(buf);i++) buf[i]=tolower(buf[i]);
    if (sscanf(buf,"%i:%i %s %s %s",&rule[top_rule].group,&rule[top_rule].uid,
      rule[top_rule].object,rule[top_rule].operation,blam)==5) {
        rule[top_rule].sense=!strcmp(blam,"allow");
        top_rule++;
      }
    if (top_rule>=MAX_RULES) {
      printk("-> ERROR: no space left for HAC entries, aborting load.\n");
      fclose(f); have_rules=1;
      return;
    }
  }

  have_rules=1;
  if (top_rule) printk("=> Loaded %d HAC rules.\n",top_rule);
    else printk("=> Warning - loaded empty HAC set.\n");
  fclose(f);
}


int is_permitted(char* c,char* object,char* operation,int group,int uid) {
  int i;
  char x;
  if (!have_rules) test_load_rules(c);
  if (!have_rules) return 0;
  if (strstr(object,"/..") || !strncmp(object, "../",3)) {
    printk("=> Unsafe object path, denying access.\n");
    return 0;
  }
  for (i=0;i<top_rule;i++) {

      int len1, len2;
      char *p, *np;
      char *s, *ns;


    if (group!=rule[i].group) {
      // Check if it's a generic rule...
      if (rule[i].group) continue;
    } else {
      // Strict rule - excatcly the same uid?
      if (uid!=rule[i].uid) continue;
    }

    p = object;
    s = rule[i].object;
//    printk("debug: rule %s object %s\n",s, p);

    while (*p && *s) {
       // get one object segment...
//     printk("debug element\n");
       np = strchr(p, '/');
       if (np) len1 = np - p;
       else len1 = strlen(p);

       // get rule[i].object segment...
       ns = strchr(s, '/');
       if (ns) len2 = ns - s;
       else len2 = strlen(s);

       if (((len2 == 1) && (*s == '*'))) {
           // got a wildcard... temporary lcamtufized checks
           if (len2>MAX_OBJPATH/2) break;
           if (len1>MAX_OBJPATH/2) break;
           if (len1*len2==0) break;
       } else {
           if ((len1 != len2)) break;
           if (strncmp(p, s, len1) != 0) break;
       }
       p += len1;
        s += len2;
       if (*s == *p) {
           s++; p++;           // skip '/'
       } else break;           // or bail out if someone is cheating/longer
    }

    // rule is longer than object, or someone is cheating
    if (*s) continue;

    // Check if rule is as specific (or less specific) than access attempt
    if (strlen(rule[i].operation)>strlen(operation)) continue;

    // Check if it's the same access attempt
    if (strncmp(rule[i].operation,operation,strlen(rule[i].operation))) continue;

    // Cheater!
    x=operation[strlen(rule[i].operation)];
    if (x && (x!='/')) continue;

    printk("=> Using rule %d:\n",i);
    printk("=> %d:%d %s %s\n",rule[i].group,rule[i].uid,rule[i].operation,rule[i].object);

    return rule[i].sense; // You're lucky asshole ;>

  }

  printk("=> No rule found, using default deny.\n");

  return 0;
}




int main(int argc,char* argv[]) {
  printf("HAC test for Argante %d.%03d.\n",SYS_MAJOR,SYS_MINOR);
  printf("(C) 2000 Michal Zalewski <lcamtuf@tpi.pl>\n\n");
  if (argc-6) {
    printk("Usage:\n"
           "  %s file.set domain uid action resource\n"
           "     file.set \n"
           "     domain       - numerical current_domain identifier\n"
	   "     uid          - numerical domain_uid identifier\n"
           "     action       - type of access to be verified\n"
           "     resource     - resource to be accessed (or 'none')\n\n",
           argv[0]);
    exit(1);
  }

  printf("Test file:        %s\n",argv[1]);
  printf("VCPU domain:      %d\n",atoi(argv[2]));
  printf("VCPU domain_uid:  %d\n",atoi(argv[3]));
  printf("Access type:      %s\n",argv[4]);
  printf("Access item:      %s\n",argv[5]);
  printf("\n");
  if (is_permitted(argv[1],argv[5],argv[4],atoi(argv[2]),atoi(argv[1])))
    printf("\nResult:           PERMISSION GRANTED\n"); else
    printf("\nResult:           ACCESS DENIED\n");

  printf("\n");
  exit(0);      
}

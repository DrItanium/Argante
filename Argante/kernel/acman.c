/*

   Argante virtual OS
   ------------------

   Access Control Manager. This routines are not used directly from
   kernel, but are provided for module development. Used for centralized,
   customizable resource access management.

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
#include "bcode.h"
#include "acman.h"
#include "module.h"
#include "task.h"

struct rule_struct rule[MAX_RULES];
int top_rule,have_rules;


// load_rules(int c) - load HAC ruleset into memory

void load_rules(int c) {
  FILE* f;
  char* x;
  int i;
  char buf[1000];
  f=fopen(RULEFILE,"r");
  if (c==-2) printk("\n");
  if (!f) {
    if (c<0) printk("ERROR: Cannot open " RULEFILE " ACL file.\n"); else {
      non_fatal(ERROR_ACL_PROBLEM,"cannot open " RULEFILE " ACL file",c);
      failure=1;
    }
    return;
  }
  if (!have_rules)  {
    printk("HAC acman: Loading access control rules (" RULEFILE ")...\n");
  } else printk("Reloading access control rules (" RULEFILE ")...\n");

  printk("+ Ruleset limit: %d bytes, %d entries.\n",sizeof(rule),MAX_RULES);

  have_rules=0;top_rule=0;

  while (fgets(buf,sizeof(buf),f)) {
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
      printk("ERROR: no space left for rulesets, aborting load.\n");
      fclose(f); have_rules=1;
      return;
    }
  }
  have_rules=1;
  if (top_rule)
    printk("=> aclman: loaded %d access control rules.\n",top_rule);
    else printk("=> aclman: warning - loaded empty access control set.\n");
  fclose(f);
}


// is_permitted() - check if current VCPU is permitted to perform
// given operation on given object. Returns 0 (permission denied)
// or 1 (permission granted).


int is_permitted(int c,char* object,char* operation) {
  int group,uid;
  int i;
  char x;
  uid=cpu[c].domain_uid;
  group=cpu[c].current_domain;
  if (!have_rules) load_rules(c);
  if (!have_rules) return 0;
  if (strstr(object,"/..")) return 0;
  if (!strncmp(object, "../", 3)) return 0;

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
//	printk("debug element\n");
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
	    s++; p++; 		// skip '/'
	} else break;		// or bail out if someone is cheating/longer
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

//    printk("debug: match\n");
    return rule[i].sense; // You're lucky ;>

  }

  return 0;
}


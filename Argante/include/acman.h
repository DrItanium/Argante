/*

   Argante virtual OS 
   ------------------

   Hierarchical Access Control manager constructions

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#include "exception.h"

struct rule_struct {
  int group;
  int uid;
  char sense;
  char object[MAX_OBJPATH];
  char operation[MAX_OPERPATH];
};

void load_rules(int c);
int is_permitted(int c,char* object,char* operation);


#define VALIDATE(cp,res,act) { \
    char errbuf[512]; \
    if (!is_permitted(cp,res,act)) { \
        snprintf(errbuf,200,"DENIED [%d:%d] act='%s' obj='%s'", \
                 cpu[cp].current_domain,cpu[(cp)].domain_uid,act,res); \
      non_fatal(ERROR_NOPERM,errbuf,(cp)); \
      return; \
    } \
  }

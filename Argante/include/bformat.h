/*

   Argante virtual OS 
   ------------------

   Binary file header format specification.

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#define BFMT_MAGIC1 0x0defaced
#define BFMT_MAGIC2 0xdeadbeef

struct bformat {
  unsigned int magic1;
  char domains[MAX_EXEC_DOMAINS];
  unsigned int flags;
  unsigned int priority;
  unsigned int ipc_reg;
  unsigned int init_IP;
  int current_domain;
  int domain_uid;
  unsigned int bytesize;
  unsigned int memflags;
  unsigned int datasize;
  char signature[64];
  unsigned int magic2;
};

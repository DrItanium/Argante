/*

   Argante virtual OS 
   ------------------

   Memory allocation / access routines (->bcode.c).

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#ifndef __HAVE_MEMORY_H
#define __HAVE_MEMORY_H

#define MEM_FLAG_READ       0x00000001
#define MEM_FLAG_WRITE      0x00000002
#define MEM_FLAG_STRICT		0x00000010
#define MEM_FLAG_MAPPED		0x00001000
/* JK for reverse endian binaries */
#define MEM_FLAG_SWAPENDIAN	0x00002000

struct memarea {
  unsigned int flags;             // See MEM_FLAG_xxx
  unsigned int map_addr;          // Starting address
  int* real_memory;               // Allocated memory blk
  unsigned int size;              // Allocated memory size
};

#ifndef __HAVE_MY_OWN_MEMORY_MANAGEMENT

/*** Prototypes **************************************************************/

void g_push_ip_on_stack();
void g_pop_ip_from_stack();

void push_ip_on_stack( int c );
void pop_ip_from_stack( int c );

int  get_mem_value(int c,unsigned int addr);
int  mem_alloc(int c,unsigned int size,unsigned int flags);
void mem_dealloc(int c,int h);
void mem_realloc(int c,int h,int newsize);
void set_mem_value(int c,unsigned int addr,int value);
void set_mem_block(int,char*,unsigned int, unsigned int);
void get_mem_block(int,char*,unsigned int, unsigned int);
void* verify_access(int c,unsigned int addr,unsigned int cnt,unsigned int fl);

#endif /* ! __HAVE_MY_OWN_MEMORY_MANAGEMENT */

#endif /* __HAVE_MEMORY_H */

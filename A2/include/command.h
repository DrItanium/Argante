/*

   Argante virtual OS release 2
   ----------------------------

   IPC kernel communication messages

   Status: under development

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/

#ifndef _HAVE_COMMAND
#define _HAVE_COMMAND 1

#define KMSG_MAGIC 0x0defaced

struct kmsg {
  unsigned int magic;		// Const.
  int vcpu;			// Apply to (vcpu < 0 - current)
  unsigned int mid;		// message number
  unsigned int param;		// parameter (vary)
  unsigned int psiz;		// payload size
};

// Queries (vcpu<0 - for current):

#define KMSG_GREETING		0	// "Hi, respond with RE_OK"
#define KMSG_USEHAC		1	// Use given HAC file
#define KMSG_GETCONS		2	// Get console output (param=max)
#define KMSG_GETOUT		3	// Get process output (param=max)
#define KMSG_INPUT		4	// Put input in buffer
#define KMSG_LOADBIN		5	// Load executable binary (name in pl)
#define KMSG_SETDOM             6	// Set domains (param=first, pl=supp)
#define KMSG_RUN		8	// Proceed!
#define KMSG_STEP		9	// Step (param instructions)
#define KMSG_GETSTATUS		10	// Get vcpu stats (kmsg_status)
#define KMSG_KILL		12	// Kill VCPU
#define KMSG_SETRESPAWN		13	// Set 'respawn' flag to param
#define KMSG_STOP		14	// Stop execution
#define KMSG_CONT		15	// Continue execution
#define KMSG_BRKPT		16	// Put breakpoint at param
#define KMSG_LOADLIB		17	// Load given library
#define KMSG_UNLOADLIB		18	// Unload library, where param=tlid
#define KMSG_PUTLOG		19	// Add to console log
#define KMSG_SYSTAT		20      // System-wide stats (kmsg_systat)
#define KMSG_USEMAP		21      // Use specific fs mapping
#define KMSG_HALT		22	// Kill the server
#define KMSG_DUMPMEM		23      // Dump memory (param=code/data, psiz=8)
					// 0-3: address, 4-7:size
#define KMSG_GETSTACK		24	// Dump stack
#define KMSG_GETXSTCK		25	// Dump xstack
#define KMSG_GETCODE		26	// Dump code segment
#define KMSG_GETSYM		28	// ret: offset, name
#define KMSG_GETADDR		29	// ret: address for symbol in pl
#define KMSG_SETPRIO		30	// set priority

// Responses:

#define KMSG_RE_OK		10000	// Ok, eventual result will follow
#define KMSG_RE_ERROR		10001	// Error, message in payload


struct kmsg_status {
  char pname[MAX_PNAME];		// Process name
  unsigned int state;			// state
  unsigned int ip;			// current ip
  unsigned int sp;			// current stack ptr
  unsigned int mcount;			// memblocks
  unsigned int bsize;			// bytecode size
  unsigned int flags;			// flags
  unsigned int started_at;		// yup.
  unsigned int wait;			// wait parameter
  unsigned int lids[MAX_RSRVD];		// used LIDs
  unsigned int domain;			// domain
  unsigned int uid;			// uid
  unsigned int reg[REGISTERS];		// registers
  unsigned int domains[MAX_DOMAINS];	// available domains
  unsigned int mptr;			// metastack ptr
  unsigned int mstart;			// metastack start
  unsigned int msiz;  			// metastack size
  unsigned int prio;			// priority
  unsigned int fromgo;			// from struct clients
  unsigned int togo;			// ...
};


struct kmsg_systat {
  // System-wide statistics would go there.
};

// Client description struct

struct clients {
  unsigned int fd;		// Socket fd
  unsigned char* fromcons;	// from console buffer
  unsigned int conslen;		// from console buffer to-go
  unsigned char* fromprog;	// from program buffer 
  unsigned int fromlen;		// from program buffer to-go
  unsigned char* toprog;	// to program buffer
  unsigned int tolen;		// to program buffer to-go
};

#endif /* not _HAVE_COMMAND */

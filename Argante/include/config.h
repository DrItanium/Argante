/*

   Argante virtual OS
   ------------------

   Customizable configuration stuff.

   Status: done, add config stuff here

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

*/


// System identification. To make any changes easier (eg. changing OS
// name to "My Own Propertiary Operating System"), it's all here ;)

#define SYSNAME		     "Argante"
#define SYS_MAJOR 1
#define SYS_MINOR 1
#define PROMPT		     "[Agt] "
#define BOOTSCRIPT	     "conf/scripts/argboot.scr" // Bootscript :>

// Some VCPU configuration stuff. Some parameters might be modified safely
// (eg. max number of VCPUs or stack size), while others should be modified
// carefully. Also, setting too small / too high values for most of these
// parameters _might_ be harmful. Don't modify anything if you don't know
// what you're doing.

#define MAX_VCPUS	     128	// Max number of processes

#define MAX_EXEC_DOMAINS      16	// Max. number of execution domains
#define MAX_NAME	      32	// Longest executable name
#define REGISTERS	      16	// Number of registers
#define MAX_STACK	   16384	// Max length of call stack
#define MAX_MEMBLK	   16384	// Max number of memory blocks

#define STACK_GROW	      16	// Stack realloc ratio
#define MEM_GROW	       8 	// Mem realloc ratio

// "Secure" limits:

#define MAX_LOAD_BYTESIZE	 500000	// Max exec code size (in packets)
#define MAX_ALLOC_MEMBLK  ((1<<18) - 2) // Max allocated memblk (dwords)
					// 2^32 = MAX_MEMBLK*(MAX_ALLOC_ + 2)
#define MAX_PRIORITY	       10000000 // Highest priority
#define MIN_CYCLES_TO_RESPAWN	    32	// Min number of work cycles for task
				        // to be respawned (see documentation)
#define DEFAULT_SAFE_USLEEP     100	// If doing nothing, allow
					// sleep for some time...

// Modules configuration:

#define MAX_MODULES		     64 // Max. module slots
#define MAX_SERVE		    100 // Max. syscalls per module

// Compiler and HLL options:

#define COMPILER_MAXSYM		    600 // Max. number of compilation symbols
#define MAX_TYPES       512		// Max number of defined HLL types
#define MAX_STRFIELDS   64		// Max. structure members
#define MAX_FUNCTIONS   512		// Max. functions
#define MAX_SYMBOLS     2048		// Max. symbols
#define MAX_HLL_NAME    64		// Longest symbol name
#define MAX_HLL_NEST    32		// Max nesting levels
#define MAX_HLL_DEF	512		// Max HLL preprocessor items
#define HLL_MAX_DEPTH	10		// Max #include depth

// HAC options:

#define MAX_OBJPATH  	  	    400 // Max. ACL object path
#define MAX_OPERPATH 		     80 // Max. ACL operation path
#define MAX_RULES    		    250 // Max. number of ACL rules
#define RULEFILE      "conf/access.hac" // Ruleset file

// Filesystem configuration:

#define FSCONV	      "conf/fsconv.dat" // Filesystem objects location
#define MAX_DIRENTS		   2048 // Max number of dir entries
#define MAX_FS_PATH		   1000 // Max FS path length
#define MAX_FS_FD		    256 // Max descriptors
#define MAX_FS_CONV		    128 // Max fs conversion points

// Debugging subsystem:

#define MAXBREAKPOINTS	32

// Network module:

#define MAX_NET_SD                 512 // Max socket descriptors
#define MAX_LOW_SD                 16  // Max low-level socket descriptors
#define UNIXPATH        "fs/unix-sock" // Path to unix sockets location

// IPC module - bulba, add some descriptions here:

#define RIPC_CONFIG                     "conf/ipc.conf"
#define MAX_HOSTS                       256	// max number of hosts in rIPC network
#define STREAM_BUFFER_LEN               4097	// stream buffer len
#define IPC_REQUEST_TTL                 10
#define IPC_RESEND_TIME                 3
#define IPC_MAX_RESEND                  3
#define PAYLOAD                         4096
#define MAX_STREAM_PAYLOAD              4096
#define MAX_BLOCK_PAYLOAD               1024    // in dwords...
#define QUEUES_AT_ONCE                  512
#define QUEUES_MAX                      10
#define REQUESTS_AT_ONCE                512
#define REQUESTS_MAX                    10

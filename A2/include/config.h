/*

   Argante virtual OS release 2
   ----------------------------

   Customizable stuff

   Status: done

   Author:     Michal Zalewski <lcamtuf@ids.pl>
   Maintainer: Michal Zalewski <lcamtuf@ids.pl>

   Changed 19/06/01 by James Kehl:
   MAX_MEMBLK and MAX_BLKSIZ need to be a power of 2 for
   efficient memory accesses. (>> is faster than /)
   Changed 11/01/02 by James Kehl:
   - rearranged into 'compatibility' and changeable settings
   - added code page settings
   - fixed moronic MAX_MEMBLK
   
*/

#ifndef _HAVE_CONFIG
#define _HAVE_CONFIG 1

#define DEFAULT_SOCK	"/var/run/argante"

/* CANNOT BE CHANGED or binary compatibility will break */
#define A2_REGISTERS	32		/* Max number of registers */
#define A2_MAX_BLKSIZ	(1 << 18)	/* Max memblock size. */
#define A2_MAX_PNAME	32		/* Program name from the header */
#define A2_MAX_SNAME	48		/* Max symbol name */
#define A2_MAX_DOMAINS	32		/* Have to limit it, DoS condition ;) */

/* CAN BE CHANGED with caution */
#define A2_MAX_PRIORITY	10000		/* Huh. */
#define A2_MAX_VCPUS       256		/* Max VCPUs (have to limit, DoS) */
#define A2_MAX_MEMBLK	(1 << 14)	/* Max memblock count */
#define A2_MAX_STACK	1024		/* Max stack size (NOT metastack) */
/* This is actually a top limit on number of modules, but protects from accidental DoS
 * Don't make it too large... you'll overflow the LID fields... */
#define A2_MAX_RSRVD	32		/* Max number of per-module rsrvd blocks */
#define A2_MAX_VFDS	64		/* Max Common Virtual File Descriptors. */

/* As an efficiency thing, the call stack grows in large
 * jumps and doesn't often get downsized. If you undefine
 * CALLSTACK_DOWNSIZE it never does, which gives a minor
 * perf boost at the cost of chewing memory.
 */

#define CALLSTACK_UPSIZE 64 /* 256 bytes - pretty miniscule */
#define CALLSTACK_DOWNSIZE 256 /* 1k */

#ifndef NO_CODE_PAGES
#define CP_BITS		16	/* Larger for less pages and more space per page */
#define A2_MAX_CODEPAGES	64	/* Maximum number of code pages - 'DoS' */
#else
#define A2_MAX_CODE	4194304	/* Maximum amount of code - for those without pages */
#define A2_MAX_CODEPAGES	1	/* Well, yeah... */
#endif

/* IDLE is used to control priorities.
 * As it happens, linux's min sleep is 10ms - so use IDLE sparingly! */
#ifndef NO_THREADS
#define CANCEL pthread_testcancel()
#define IDLE usleep(1)
#else
#define CANCEL
#define IDLE usleep(1)
#endif

/* Time it takes for man_pollall() to return, in millis.
 * In single threaded mode, this sets the time-slice allocated to a VCPU. */
#define A2_POLLTIME 50

/* CAN BE CHANGED as much as you like because they don't do anything */
#if 0
#define A2_MAX_HACRULES	128		/* Just to keep it it limits. */
#define A2_MAX_CLIENTS     32		/* Max clients (have to limit, DoS) */
#endif

/* SHOULDN'T BE CHANGED because this is code */
#ifndef NO_CODE_PAGES
#define CP_PAGE_SIZE	(1 << CP_BITS)
#define CP_PAGE_OF(a)	(((a) >> ((sizeof(unsigned) * 8) - CP_BITS)))
#define CP_DATA_OF(a)	((a) & (0xffffffff >> ((sizeof(unsigned) * 8) - CP_BITS)))
#else
#define CP_PAGE_OF(a)	0
#define CP_DATA_OF(a)	(a)
#define CP_PAGE_SIZE	0xffffffff
#endif

#endif /* not _HAVE_CONFIG */

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
*/

#ifndef _HAVE_CONFIG
#define _HAVE_CONFIG 1

#define DEFAULT_SOCK	"/var/run/argante"

#define MAX_CLIENTS     32	// Max clients (have to limit, DoS)
#define MAX_VCPUS       256	// Max VCPUs (have to limit, DoS)
#define MAX_MEMBLK	(1 << 12)	// Max memblock count **** CHANGED ****
#define MAX_STACK	1024	// Max stack size (NOT metastack)
#define MAX_BLKSIZ	(1 << 24)	// Max memblock size. **** CHANGED ****
#define REGISTERS	32	// Max number of registers, do not touch ;)	
#define MAX_RSRVD	16	// Max number of per-module rsrvd blocks
#define MAX_PNAME	32	// Program name from the header
#define MAX_SNAME	48	// Max symbol name
#define MAX_DOMAINS	32	// Have to limit it, DoS condition ;)
#define MAX_CODE	5000000 // Max code size
#define MAX_PRIORITY	10000	// Huh.

#define MAX_HACRULES	128	// Just to keep it it limits.

#endif /* not _HAVE_CONFIG */

/* autocfg.h.in.  Generated automatically from configure.ac by autoheader.  */

/* The name that shows up at startup */
#define A2_VERSION "0.009 alpha"

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Define if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix). */
/* #undef HAVE_ALLOCA_H */

/* Define if you have the `bzero' function. */
/* #undef HAVE_BZERO */

/* Has a /dev/urandom entropy source */
/* #undef HAVE_DEV_URANDOM */

/* Define if you have the <dirent.h> header file, and it defines `DIR'. */
#define HAVE_DIRENT_H 1

/* Define if you have the <dir.h> header file. */
#define HAVE_DIR_H 1

/* Define if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define if you have the `ftime' function. */
#define HAVE_FTIME 1

/* Define if you have the `gethostname' function. */
#undef HAVE_GETHOSTNAME

/* Define if you have the `getline' function. */
/* #undef HAVE_GETLINE */

/* Define if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* Define if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if you have the <io.h> header file. */
#define HAVE_IO_H 1

/* Define if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define if you have the `localtime_r' function. */
/* #undef HAVE_LOCALTIME_R */

/* Define if you have the `mallinfo' function. */
/* #undef HAVE_MALLINFO */

/* Define if your system has a working `malloc' function. */
#define HAVE_MALLOC 1

/* Define if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define if you have the `memchr' function. */
#define HAVE_MEMCHR 1

/* Define if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Define if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define if you have the <netinet/in.h> header file. */
/* #undef HAVE_NETINET_IN_H */

/* Define if you have the `readdir_r' function. */
/* #undef HAVE_READDIR_R */

/* Define if you have the `rmdir' function. */
#define HAVE_RMDIR 1

/* Define if you have the `seekdir' function. */
/* #undef HAVE_SEEKDIR */

/* Define if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Define if `stat' has the bug that it succeeds when given the zero-length
   file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define if you have the `strcasecmp' function. */
/* #undef HAVE_STRCASECMP */

/* Define if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define if you have the `strcmpi' function. */
#define HAVE_STRCMPI 1

/* Define if you have the `strcspn' function. */
#define HAVE_STRCSPN 1

/* Define if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have the `strncasecmp' function. */
/* #undef HAVE_STRNCASECMP */

/* Define if you have the `strncmpi' function. */
#define HAVE_STRNCMPI 1

/* Define if you have the `strrchr' function. */
#define HAVE_STRRCHR 1

/* Define if you have the `strspn' function. */
#define HAVE_STRSPN 1

/* Define if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define if you have the `strtok_r' function. */
/* #undef HAVE_STRTOK_R */

/* Define if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define if you have the `sysinfo' function. */
/* #undef HAVE_SYSINFO */

/* Define if you have the <sys/dir.h> header file, and it defines `DIR'. */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/socket.h> header file. */
/* #undef HAVE_SYS_SOCKET_H */

/* Define if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/timeb.h> header file. */
#define HAVE_SYS_TIMEB_H 1

/* Define if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* Define if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define if you have the `usleep' function. */
/* #undef HAVE_USLEEP */

/* Define if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define if `lstat' dereferences a symlink specified with a trailing slash.
   */
/* #undef LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Disable code paging */
/* #undef NO_CODE_PAGES */

/* Disable multithreading */
#define NO_THREADS 1

/* Check memory read permissions */
/* #undef PARANOID */

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* The size of a `double', as computed by sizeof. */
/* Could use sizeof() ... */
#define SIZEOF_DOUBLE 8

/* The size of a `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
        STACK_DIRECTION > 0 => grows toward higher addresses
        STACK_DIRECTION < 0 => grows toward lower addresses
        STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Create statically-linked modules */
#define STATIC 1

/* Define if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>. */
/* #undef TIME_WITH_SYS_TIME */

/* Define if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Readline support */
/* #undef USE_READLINE */

/* Define if `lex' declares `yytext' as a `char *' by default, not a `char[]'.
   */
#define YYTEXT_POINTER 1

/* Use POSIX style pthreads */
/* #undef _POSIX_PTHREAD_SEMANTICS */

/* Enable reentrancy in C library */
#define _REENTRANT 1

/* A floating-point type as large as a long */
#define a2float float

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define as `__inline' if that's what the C compiler calls it, or to nothing
   if it is not supported. */
#define inline

/* Define to `long' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
#define ssize_t int

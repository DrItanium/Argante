# Process this file with autoconf to produce a configure script.
AC_INIT([A2], [0.009], [ecks@optusnet.com.au])
AC_DEFINE([A2_VERSION], ["0.009 alpha"], [The name that shows up at startup])
AC_CONFIG_SRCDIR([compiler/nagt.c])
AC_CONFIG_HEADER([autocfg.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_RANLIB
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

AC_MSG_CHECKING([for /dev/urandom])
if test -c /dev/urandom
then
	AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE_DEV_URANDOM], 1, [Has a /dev/urandom entropy source])
else
	AC_MSG_RESULT([no])
	AC_MSG_WARN([You don't have /dev/urandom: I am falling back onto a low-grade
random source. Change OS, write a better alternative, or put
up with ***cryptographically pathetic randomness***.]) 
fi


# I used to think autoconf was totally great, but it's decidedly icky
# replacing functions that aren't used - specially when they printk() :)
AC_DEFUN([A2_LIBOBJ], [
	a2_libobj_$2="$a2_libobj_$2 $1.$ac_objext"
	AC_SUBST([A2_LIBOBJ_$2], $a2_libobj_$2)
])

AC_DEFUN([A2_COMPAT_O], [
	a2_compat_o="$a2_compat_o $1.$ac_objext"
	AC_SUBST([[A2_COMPAT_O]], $a2_compat_o)
])

# Convenience: cache --with-xyz too. Because it's fun.
AC_DEFUN([A2_WITHLIB], [
	AC_CACHE_VAL(a2_cv_want_$1, a2_cv_want_$1=$3)
	AC_ARG_WITH([$1], AC_HELP_STRING([--with-$1], [$4 (default: $3)]),
		a2_cv_want_$1=$withval)
	if test x"$a2_cv_want_$1" = xyes
	then
		AC_CHECK_LIB([$1], [$2],
		[
		  AC_MSG_CHECKING([whether to $4])
		  AC_MSG_RESULT([yes])
		  $5
		  ],
		[
		  AC_MSG_CHECKING([whether to $4])
		  AC_MSG_RESULT([no, not found])
		  $6
		  ], [$7]
		)
	else
		AC_MSG_CHECKING([whether to $4])
		AC_MSG_RESULT([no, not requested])
		$6
	fi])

# optionally find a library combination which will compile and run a program. (gasp)
# option name in $1, libs in $2, default in $3, desc in $4, code in $5, conf in $6.
AC_DEFUN([A2_WITH_SEARCH_RUN_LIBS], [
	AC_CACHE_VAL(a2_cv_want_$1, a2_cv_want_$1=$3)
	AC_ARG_WITH([$1], AC_HELP_STRING([--with-$1], [$4 (default: $3)]),
		a2_cv_want_$1=$withval)
	withlib=""
	if test x"$a2_cv_want_$1" = xsearch
	then
		if test x"$cross_compiling" = xyes
		then
			AC_MSG_CHECKING([how to $4])
			AC_MSG_RESULT([no (cross compiling, need --with-$1=-l$1-lib)])
		else
			a2_save_libs=$LIBS
			a2_lib_found=""
			for a2_lib in $2; do
				LIBS=$a2_lib" $save_libs"
				AC_TRY_RUN([$5], [ a2_lib_found="$a2_lib"; break ], , [])
			done
			LIBS=$a2_save_libs
			if test x"$a2_lib_found" != x
			then
				AC_MSG_CHECKING([how to $4])
				AC_MSG_RESULT([$a2_lib_found])
				a2_cv_want_$1=$a2_lib_found
				$6
			else
				AC_MSG_CHECKING([how to $4])
				AC_MSG_RESULT([no, not found])
			fi
		fi
	elif test x"$a2_cv_want_$1" = xno
	then
		AC_MSG_CHECKING([how to $4])
		AC_MSG_RESULT([no, not requested])
	else
		$6
	fi
])

# Test various ways of getting readline.
#
A2_WITH_SEARCH_RUN_LIBS([readline], 
	[-lreadline] ["-lreadline -ltermcap"] ["-lreadline -lterminfo"] ["-lreadline -lcurses"],
	[search],
	[use readline command line editing],
	[
		char *readline(const char *prompt);
		int main() {
			readline("Readline test: press enter");
			return 0;
		}
	],
	[
		AC_DEFINE([USE_READLINE], 1, [Readline support])
		AC_SUBST([READLINE_LIBS], [$a2_lib_found])
	]
)

A2_WITHLIB([dl], [dlopen], [yes], [use dynamically-linked modules],
	[
	  AC_SUBST([MODULES], [DYNAMIC])
	  AC_SUBST([DL_LIBS], [-ldl])
	  ],
	[
	  AC_DEFINE([STATIC], 1, [Create statically-linked modules])
	  AC_SUBST([MODULES], [STATIC])
	  ]
	)

AC_LIBSOURCE(compat/semaphr.c)
A2_WITHLIB([pthread], [pthread_create], [yes], [use multithreading],
	[
	  a2_save_libs=$LIBS
	  LIBS="-lpthread $LIBS"
	  AC_SEARCH_LIBS([sem_init], [rt],
	  [
	  	# XXX: Some systems might need -lc_r - but which?
	  	AC_DEFINE([_REENTRANT], 1, [Enable reentrancy in C library])
	  	AC_DEFINE([_POSIX_PTHREAD_SEMANTICS], 1, [Use POSIX style pthreads])
		AC_SUBST([PTHREAD_LIBS], [$LIBS])
	  ],
	  [
AC_MSG_WARN([pthreads were found on your system, but not semaphores. Multithreading is
disabled. Please report this and your system's location of sem_init.])
	  	AC_DEFINE([NO_THREADS], 1, [Disable multithreading])
		A2_COMPAT_O(semaphr)
		A2_LIBOBJ(compat/semaphr, [KERNEL])
	  ])
	  LIBS=$a2_save_libs
	],
	[
	  AC_DEFINE([NO_THREADS], 1, [Disable multithreading])
	  A2_COMPAT_O(semaphr)
	  A2_LIBOBJ(compat/semaphr, [KERNEL])
	  ]
	)

# Find socket.
a2_save_libs=$LIBS
AC_SEARCH_LIBS([socket], [socket], [ AC_SUBST([SOCKET_LIBS], [$LIBS]) ],
[
  AC_MSG_WARN([Your system appears not to have socket()!])
])
LIBS=$a2_save_libs

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([alloca.h dir.h errno.h fcntl.h limits.h malloc.h sys/time.h sys/timeb.h io.h netinet/in.h sys/socket.h])
# These are checked by later calls: stdlib.h string.h unistd.h memory.h 
AC_HEADER_TIME

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_TYPE([ssize_t], [AC_DEFINE([ssize_t], [int], [Define to `int' if <sys/types.h> does not define.])])
AC_STRUCT_TM

#
# Get the elements of anyval to be all the same size.
#
dnl AC_CHECK_SIZEOF([int]) # I am curious, OK?
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([float])
AC_CHECK_SIZEOF([double])

AC_MSG_CHECKING([appropriate type of float to use])
if test x"$ac_cv_sizeof_long" = x"$ac_cv_sizeof_float"
then
	AC_MSG_RESULT([float])
	AC_DEFINE([a2float], [float], [A floating-point type as large as a long])
elif test x"$ac_cv_sizeof_long" = x"$ac_cv_sizeof_double"
then
	AC_MSG_RESULT([double])
	AC_DEFINE([a2float], [double], [A floating-point type as large as a long])
else
	AC_MSG_ERROR([cannot find a sizeof(float) == sizeof(int), please report this])
fi

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF

# We have emulation for these in compat/.
AC_DEFUN([A2_COMPAT_REPLACE_FUNCS], [
	AC_FOREACH([AC_Func], [$1], [AC_LIBSOURCE(compat/AC_Func.c)])
	AC_CHECK_FUNCS([$1], , [
		A2_COMPAT_O($ac_func)
		A2_LIBOBJ(compat/$ac_func, [$2])
	])
])

# Bzero emulation falls on memset, then onto bzero.c
AC_DEFUN([A2_COMPAT_REPLACE_PAIR], [
	AC_LIBSOURCE(compat/$3.c)
	AC_CHECK_FUNCS([$1], , [
		AC_CHECK_FUNCS([$2], , [
			A2_COMPAT_O($3)
			A2_LIBOBJ(compat/$3, [$4])
		])
	])
])

A2_COMPAT_REPLACE_FUNCS([getline usleep strtok_r], [ALL])

AC_LIBSOURCE(compat/ltime_r.c)
AC_CHECK_FUNCS([localtime_r], , [
	A2_COMPAT_O(ltime_r)
# Really modules, but can't put it there! Do YOU want 4 .so's with their own copy?
	A2_LIBOBJ(compat/ltime_r, [KERNEL])
])

A2_COMPAT_REPLACE_PAIR([strcasecmp], [strcmpi], [strcmpi], [ALL])
A2_COMPAT_REPLACE_PAIR([strncasecmp], [strncmpi], [strncmpi], [ALL])
A2_COMPAT_REPLACE_PAIR([bzero], [memset], [bzero], [ALL])
# We can work around these missing.
# I doubt readdir_r is really needed but it's better to be safe...
AC_CHECK_FUNCS([gettimeofday ftime gethostname])
AC_CHECK_FUNCS([mallinfo seekdir readdir_r])

# If we have struct sysinfo, we probably have a Linux-style sysinfo.
# Otherwise we don't care - Solaris sysinfo is not really equivalent.
AC_CHECK_FUNCS([sysinfo], , [
 AC_CHECK_MEMBER( struct sysinfo.freeram, [
  AC_DEFINE([HAVE_SYSINFO], 1, [Define if your system has Linux-style sysinfo.])
 ], , [#include <sys/sysinfo.h>])
])

# XXX: Don't have compat for these yet.
AC_CHECK_FUNCS([strchr strcspn strdup strrchr memchr strspn strstr strtol strtoul memmove socket])
AC_CHECK_FUNCS([mkdir rmdir])
AC_CHECK_FUNCS([strerror]) # LAC

# Checks for --enable options
AC_ARG_ENABLE([paranoid], AC_HELP_STRING([--enable-paranoid], [always check for read memory access (default: disabled)]),
		a2_cv_want_paranoid=$enableval, a2_cv_want_paranoid=no)
AC_ARG_ENABLE([codepages], AC_HELP_STRING([--disable-codepages], [disable paged-code support (default: enabled)]),
		a2_cv_want_codepages=$enableval, a2_cv_want_codepages=yes)

if test x"$a2_cv_want_paranoid" = xyes
then
	AC_DEFINE([PARANOID], 1, [Check memory read permissions])
fi

if test x"$a2_cv_want_codepages" != xyes
then
	AC_DEFINE([NO_CODE_PAGES], 1, [Disable code paging])
fi

# MODULES!!!
MODULES_ALL=["display fs locallib strfd cfd console alib dummy"]
MODULES_KERN=["display fs locallib strfd cfd console alib"]
MODULES_NATIVE=["display fs locallib strfd cfd console"]

AC_ARG_ENABLE([kern-mod], AC_HELP_STRING([--enable-kern-mod=mod1[[[,]]]mod2],
	[select kernel modules (default: $MODULES_KERN)])
	)
AC_ARG_ENABLE([native-mod], AC_HELP_STRING([--enable-native-mod=mod1[[[,]]]mod2],
	[select modules to be linked into native code (default: $MODULES_NATIVE)
	DO NOT ENABLE ALIB HERE!])
	)

AC_SUBST([MODULES_ALL], $MODULES_ALL)
AC_SUBST([MODULES_KERN], $MODULES_KERN)
AC_SUBST([MODULES_NATIVE], $MODULES_NATIVE)

AC_CONFIG_FILES([Makefile
                 compat/Makefile
                 include/Makefile
                 kernel/Makefile
                 modules/Makefile
                 compiler/Makefile
                 bench/Makefile
                 lib/Makefile
		 agent/Makefile
		 LAC/Makefile
		 Documentation/Makefile
	])
AC_OUTPUT

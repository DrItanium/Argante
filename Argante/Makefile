#
# Argante Virtual OS
# ------------------
#
# Grand Central Makefile
#
# Status: done
#
# AIX Author:       Mariusz Marcinkiewicz <tmoggie@ags.pl>
# BSD Author:       Michal Zalewski <lcamtuf@ids.pl>
# IRIX Author:      Michal Zalewski <lcamtuf@ids.pl>
# Linux Author:     Michal Zalewski <lcamtuf@ids.pl>
# Solaris Author:   Michal Zalewski <lcamtuf@ids.pl>
# Conversion by:    James Kehl <ecks@optusnet.com.au>
# Various fixes by: Adam Chodorowski <adam.chodorowski@bigfoot.com>
#
# Firstly, update the build number.
E=kernel/.argbuild

X=$(shell cat $E)
CFLAGS+=-DBUILD=$X

# Where are we?
TOP := $(shell pwd)

# Include the configuration.
include conf.mk

# Definitions

# Generic
CC = gcc
LDLIBS += -lm -lc
CFLAGS += -I../include -fomit-frame-pointer -O9 -ffast-math -Wall -fforce-mem -fforce-addr -fcaller-saves -fstrength-reduce -fthread-jumps -funroll-loops -fcse-follow-jumps -fcse-skip-blocks -frerun-cse-after-loop -fexpensive-optimizations -fschedule-insns2
SHROPTS = -shared -fPIC

#
# Optimization for Everyone.
# I don't know if it will work with other than i-x86's,
# flame me if not. Fingers crossed.
#
# chodorowski: Works on Linux/i386 and OpenBSD/i386, but not Solaris/sparc.
#
ARCH=$(shell uname -m)

ifneq ($(SYS),Solaris)
CFLAGS+=-mcpu=$(ARCH) -march=$(ARCH)
endif

# System dependant
# NOTE: SVGA and PACKET in modules makefile

# Linux
ifeq ($(SYS), Linux)
LDLIBS += -ldl -rdynamic

# Readline support? Linux only?
ifdef USE_READLINE
CFLAGS+=-DHAVE_READLINE -D__USE_TERMCAP
LDLIBS+=-lreadline
ifdef USE_TINFO
LDLIBS+=-ltinfo
else
LDLIBS+=-ltermcap
endif
endif

endif # Linux

# AIX
ifeq ($(SYS), AIX)
LDLIBS += -lc -ldl
SHROPTS	= -Xlinker -G
endif

# IRIX
# No Options!?!

ifeq ($(SYS), BSD)
# FIXME: linker options catered for in ./build
endif

# Solaris
ifeq ($(SYS), Solaris)
LDLIBS += -lsocket -Xlinker -export-dynamic -ldl 
SHROPTS = -L/usr/ucblib -lucb -Xlinker "-R/usr/ucblib"
endif

IDSTR ="`whoami`@`uname -n` on `date +'%A, %d %B %Y, %H:%M'`"

export CFLAGS LDLIBS SHROPTS CC IDSTR

DIRS=compiler/ kernel/ modules/ hll/ tools/

all: $(DIRS)
	@for dir in $(DIRS); do \
		$(MAKE) -C $$dir TOP=$(TOP); \
	done
#	let X=$X+1; echo $$X >$E

clean: $(DIRS)
	@for dir in $(DIRS); do \
		$(MAKE) -C $$dir TOP=$(TOP) clean; \
	done

# I have not touched anything below this line
# -------------------------------------------

publish: clean
	rm -f arg.tgz ; cd .. ; tar cfvz arg.tgz Argante;cd Argante
	scp ../arg.tgz lcamtuf@dione.ids.pl:public_html/arg.tgz
	scp README lcamtuf@dione.ids.pl:buka.org/agt/AgtDoc.txt -C
#	scp Documentation/Advocacy lcamtuf@dione.ids.pl:buka.org/agt/advocacy.txt -C
	scp Documentation/ChangeLog lcamtuf@dione.ids.pl:buka.org/agt/ChangeLog -C
	sysdep/announce
	rm -f ../arg.tgz

install: all
	rm -rf /usr/doc/Argante /usr/lib/argante
	cp argante /usr/bin/argante
	mkdir /usr/lib/argante || true
	cp modules/*.so /usr/lib/argante
	cp tools/agt* tools/vcpucons /usr/bin
	cp tools/ripcd /usr/bin
	cp -r Documentation /usr/doc/Argante
	cp -r conf /usr/lib/argante/conf
	cp -r hll/include /usr/lib/argante/hll-include
	cp -r Examples /usr/doc/Argante/
	cp compiler/agtc /usr/bin/
	cp hll/ahlt hll/elim hll/acc /usr/bin/
	cp Documentation/man/* /usr/man/man1

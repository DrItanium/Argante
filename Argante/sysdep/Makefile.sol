#
# Argante Virtual OS
# (C) 2000 Michal Zalewski <lcamtuf@ids.pl>
#
# Makefile for Solaris
#
# Status: done
#
# Author:     Michal Zalewski <lcamtuf@ids.pl>
# Maintainer: Michal Zalewski <lcamtuf@ids.pl>
#

OBJECTS = kernel/bcode.o kernel/console.o kernel/manager.o kernel/module.o \
          kernel/task.o kernel/acman.o kernel/debugger.o kernel/cmd.o

MODULES = modules/display.so modules/fs.so modules/ipc.so modules/network.so \
          modules/advmem.so modules/access.so modules/locallib.so modules/math.so \
#	  modules/packet.so modules/gfx.so


TOOLS	= tools/actest tools/binedit tools/disasm tools/vcpucons tools/agtexe 

CMPLR	= compiler/agtc

HLLT	= hll/ahlt

MAIN    = kernel/main-boot.c

OFILE	= argante
CFLAGS  += -Iinclude -Imodules -fomit-frame-pointer -O9 -ffast-math -fforce-mem -fforce-addr -fcaller-saves -fstrength-reduce -fthread-jumps -funroll-loops -fcse-follow-jumps -fcse-skip-blocks -frerun-cse-after-loop -fexpensive-optimizations -fschedule-insns2
LIBS	+= -lsocket -Xlinker -ldl -lm
FSLIBS  = -L/usr/ucblib -lucb -Xlinker "-R/usr/ucblib" 
CC	= gcc

IDSTR   = "`whoami`@`uname -n` on `date +'%A, %d %B %Y, %H:%M'`"

all: autogen $(OBJECTS) $(OFILE) $(CMPLR) $(HLLT) $(TOOLS)

autogen: include/exception.h modules/syscall.h
	compiler/autogen.sh
	include/autogen.sh

${MODULES}:
	cd modules; gmake display.so fs.so ipc.so network.so advmem.so access.so locallib.so math.so

$(OFILE): $(MAIN) $(OBJECTS) ${MODULES}
	$(CC) $(LIBS) $(CFLAGS) $(MAIN) -o $(OFILE) $(OBJECTS) -D IDSTR=\"$(IDSTR)\" 
	strip $(OFILE)
#	cd tools/ripc-daemon; test -f /usr/include/openssl/ssl.h && LIBS="$(LIBS)" gmake -f Makefile.ssl || true
#	cd tools/ripc-daemon; test -f /usr/include/openssl/ssl.h || LIBS="$(LIBS)" gmake -f Makefile.nossl


compiler: $(CMPLR)

hllt: $(HLLT)


test: clean all
	./compiler/agtc compiler/examples/hello.agt
	./compiler/agtc compiler/examples/fs.agt
	./compiler/agtc compiler/examples/tcp.agt
	cd Examples/Mini-HTTP;./BUILD 1
	@sleep 1
	./$(OFILE) conf/scripts/test.scr

publish: clean
	rm -f arg.tgz ; cd .. ; tar cfvz arg.tgz Argante;cd Argante
	scp ../arg.tgz lcamtuf@dione:public_html/arg.tgz
	rm -f ../arg.tgz

clean:
	rm -f kernel/*.o modules/*.so core $(OFILE) $(CMPLR) compiler/examples/*.img compiler/autogen.h tools/actest tools/binedit tools/disasm $(HLLT)

softclean:
	rm -f kernel/*.o modules/*.so core $(OFILE) $(CMPLR) compiler/autogen.h tools/actest tools/binedit tools/disasm $(HLLT)

install: clean all
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

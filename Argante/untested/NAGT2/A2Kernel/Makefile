# I investigated the effects of -mrtd on i386 code,
# it didn't seem a worthwhile change.
#CFLAGS=-mrtd
#CFLAGS=-Wall -O2 -g
CFLAGS=-Wall -O9 -fomit-frame-pointer -O9 -ffast-math -Wall -fforce-mem -fforce-addr -fcaller-saves -fstrength-reduce -fthread-jumps -funroll-loops -fcse-follow-jumps -fcse-skip-blocks -frerun-cse-after-loop -fexpensive-optimizations -fschedule-insns2
TARGETS=cmd.o bcode.o memory.o test.o

a2: $(TARGETS)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

dummy: $(TARGETS)

cmd.o: cmd.c cmdtabs.h
	$(CC) $(LDFLAGS) $(CFLAGS) -c $< -o $@

cmdtabs.h: cmd.c autogen.pl
	./autogen.pl < $<
# They're all produced at the same time
lang2.h: cmdtabs.h
cmd.h: cmdtabs.h

clean:
	rm -f $(TARGETS) lang2.h cmd.h cmdtabs.h *.o


include ../../../conf.mk
include ../common.mk


ifdef USE_CURSES
CFLAGS+=-DCURSES
LDLIBS+=-lcurses -ltinfo

all: agtman
	cp -f agtman ../..
else
all:
	@echo "ncurses GMI disabled. Are you some sort of 3l33t koder?"
endif

agtman: agtmain.o cursrtns.o funcs.o iortns.o notglib.o
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

clean:
	rm -f *.o agtman ../../agtman


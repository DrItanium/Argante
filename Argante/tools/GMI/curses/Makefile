
include ../common.mk
CFLAGS+=-DCURSES
LDLIBS+=-lcurses

all: agtman
	cp -f agtman ../..

agtman: agtmain.o cursrtns.o funcs.o iortns.o notglib.o
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

clean:
	rm -f *.o agtman ../../agtman


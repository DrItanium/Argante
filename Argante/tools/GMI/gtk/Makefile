
include ../common.mk
LDLIBS+=-lgtk -lgdk
CFLAGS+=-DGTK

all: xagtman
	cp -f xagtman ../..

xagtman: agtmain.o gtkrtns.o funcs.o iortns.o
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

clean:
	rm -f *.o xagtman ../../xagtman



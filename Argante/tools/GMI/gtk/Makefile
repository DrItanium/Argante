
include ../../../conf.mk
include ../common.mk

ifdef USE_GTK
LDLIBS+=$(shell gtk-config --libs)
CFLAGS+=-DGTK $(shell gtk-config --cflags)

all: xagtman
	cp -f xagtman ../..
else
all:
	@echo "GTK GMI disabled."
endif

xagtman: agtmain.o gtkrtns.o funcs.o iortns.o
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

clean:
	rm -f *.o xagtman ../../xagtman


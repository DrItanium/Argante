# SSL Unified Makefile
#
# Maintainer: James Kehl <ecks@optusnet.com.au>
# (If nobody objects!)
# Some fixes by Adam Chodorowski

# Look up where the includes and libs for SSL are located.
#
# FIXME: This uses 'locate', which may fail on some platforms
#        (Solaris for example) - investigate other options!!

SSL_INCS=$(foreach file,$(shell locate ssl/ssl.h),-I$(dir $(file)))
SSL_LIBS=$(foreach file,$(shell locate libssl.so),-L$(dir $(file)))

ifneq ($(SSL_INCS),)
CFLAGS+=-DUSE_SSL
LDLIBS+=$(SSL_LIBS) -lssl -lcrypto

all: ripcd
else
all: warn ripcd
endif

ripcd: ripcd.c ssl.h
	$(CC) $(CFLAGS) -DCFGFILE='"r1"' -c $< -o $@.o
	$(CC) $(CFLAGS) $@.o $(LDLIBS) -o $@
	ln -sf ripc-daemon/$@ ../.

ssl.h:
	@echo "/*** Auto-generated file. Do not edit! ***/" >ssl.h
	@echo "#include \""$(word 1,$(shell locate ssl/ssl.h))"\"" >>ssl.h
	@echo "#include \""$(word 1,$(shell locate ssl/err.h))"\"" >>ssl.h

warn:
	@echo
	@echo "WARNING: building remote IPC connectivity daemon without SSL support!"
	@echo "That's because you apparently do not have OpenSSL development libraries"
	@echo "installed on your system. It won't affect rIPC functionality, but makes"
	@echo "all remote data transfers dependent on security of underlying communication"
	@echo "layer. So, if you have secure or tunneled link between rIPC endpoints (ssh"
	@echo "tunnels, VPN networks), it does not matter. Otherwise, it *might* cause"
	@echo "problems. Also, non-SSL daemons could not connect to SSL-enabled rIPC HUBs."
	@echo

# This is one contributor to the third-snapshot bloat!
clean:
	-rm -f core *.o cli ripcd ssl.h

cli: cli.c

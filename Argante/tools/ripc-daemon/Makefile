# SSL Unified Makefile
#
# Maintainer: James Kehl <ecks@optusnet.com.au>
# (If nobody objects!)

# A cute hack. I hope you use locate!
SSL_DIR1=$(dir $(shell locate ssl.h))

ifneq ($(SSL_DIR1),)

SSL_DIR=$(SSL_DIR1)..
CFLAGS+=-DUSE_SSL -I$(SSL_DIR)/include 
LDLIBS+=-L$(SSL_DIR)/lib -lssl -lcrypto

all: ripcd
else
all: warn ripcd
endif

ripcd: ripcd.c 
	$(CC) $(CFLAGS) -DCFGFILE='"r1"' -o $@.o -c $^
	$(CC) $(CFLAGS) $@.o $(LDLIBS) -o $@
	ln -sf ripc-daemon/$@ ../.

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
	rm -f core *.o cli ripcd 

cli: cli.c

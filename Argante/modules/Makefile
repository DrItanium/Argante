include $(TOP)/conf.mk

MODULES = display.so fs.so ipc.so network.so advmem.so access.so locallib.so math.so

ifdef USE_PACKET
MODULES+= packet.so
endif

ifdef USE_SVGALIB
MODULES+= gfx.so
endif

ifdef USE_GGI
MODULES+= ggi.so
endif

all: $(MODULES)

%.so: %.c
	@echo "Compiling module" $@ "..."
	@$(CC) $(CFLAGS) $(LDLIBS) $(SHROPTS) $^ -o $@

gfx.so: gfx.c
	@echo "Compiling module" $@ "..."
	@$(CC) $(CFLAGS) $(LDLIBS) -lvga $(SHROPTS) $^ -o $@

ggi.so: ggi.c
	@echo "Compiling module" $@ "..."
	@$(CC) $(CFLAGS) $(LDLIBS) -lggi $(SHROPTS) $^ -o $@

clean:
	rm -f $(MODULES)

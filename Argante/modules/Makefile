include ../conf.mk
  
MODULES = display.so fs.so ipc.so network.so advmem.so access.so locallib.so math.so
  
ifdef USE_PACKET
MODULES+= packet.so
endif
  
ifdef USE_SVGALIB
MODULES+= gfx.so
endif
  
all: $(MODULES)
  
%.so: %.c
	$(CC) $(CFLAGS) $(LDLIBS) $(SHROPTS) $^ -o $@
  
gfx.so: gfx.c
	$(CC) $(CFLAGS) $(LDLIBS) -lvga $(SHROPTS) $^ -o $@
  
clean: 
	rm -f $(MODULES)

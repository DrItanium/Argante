

AGTC = ./agtc

# order matters!
PARTS = include.arg main.arg config.arg packet.arg time.arg \
        route.arg msg.arg ipc.arg endian.arg ether.arg 
SRCFILE = dvr.arg
OFILE = dvr.img

$(OFILE): $(SRCFILE)
	$(AGTC) $(SRCFILE)

$(SRCFILE): $(PARTS)
	cat /dev/null > $(SRCFILE)
	for a in $(PARTS) ; do \
	    cat $$a >> $(SRCFILE) ; \
	done

clean:
	rm -f $(SRCFILE) $(OFILE)


CFLAGS=-O2 -Wall -g  -IA2Kernel -I../../include -I../../compiler
TARGETS=nagt imgdump

all: $(TARGETS)

lang2.h:
	make -C A2Kernel lang2.h
	ln -s A2Kernel/lang2.h lang2.h

nagt: nagt.o blobstak.o
nagt.o: nagt.c lang2.h
	$(CC) $(LDFLAGS) $(CFLAGS) -c $< -o $@

imgdump: imgdump.c lang2.h
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o $@

# BlobStak obstack emulation
blobstak.o: blobstak.c
# Getline emulation?

# Some testing stuff
test: test_v2.img
test2: test_v22.img
test3: test_v23.img

test_v2.img: test_v2.agt nagt
	./nagt -c test_v2.agt $@
test_v22.img: test_v2.img imgdump
	./imgdump -D $^ > test_v22.agt
	./nagt -c test_v22.agt $@
test_v23.img: test_v22.img imgdump
	./imgdump -D $^ > test_v23.agt
	./nagt -c test_v23.agt $@

clean:
	rm -f $(TARGETS) lang2 lang2.h *.o *.img test_v22.agt test_v23.agt
	make -C A2Kernel clean

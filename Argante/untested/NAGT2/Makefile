CFLAGS=-O2 -Wall -g -I../../include -I../../compiler

all: nagt

lang2.h: lang2.c
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o lang2
	./lang2 > $@
	rm -f lang2

nagt: nagt.o blobstak.o
nagt.o: nagt.c lang2.h
	$(CC) $(LDFLAGS) $(CFLAGS) -c $< -o $@

# BlobStak obstack emulation
blobstak.o: blobstak.c
# Getline emulation?

# Some testing stuff
test: test_v2.img

test_v2.img: test_v2.agt nagt
	./nagt -c test_v2.agt $@

clean:
	rm -f nagt lang2 lang2.h *.o *.img

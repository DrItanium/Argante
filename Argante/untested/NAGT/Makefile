CFLAGS=-O2 -Wall -g -I../../include -I../../compiler

all: nagt optim link

lang2.h: lang2.c
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o lang2
	./lang2 > $@
	rm -f lang2

nagt: nagt.c lang2.h
	$(CC) $(LDFLAGS) $(CFLAGS) $< -o $@
	
optim: optim.c
link: link.c

# Some testing stuff
test: link test1.img test2.img
	./link test1.img test2.img
	../../tools/disasm lout.img lout.s

test1.img: test1.arg nagt
	./nagt -c test1.arg $@

test2.img: test2.arg nagt
	./nagt -c test2.arg $@

clean:
	rm -f nagt optim link lang2 lang2.h *.o *.img

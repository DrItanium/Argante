CFLAGS=-O2 -Wall -g -I../../include -I../../compiler

all: nagt optim link

nagt: nagt.o
optim: optim.o
link: link.o

# Some testing stuff
test: link test1.img test2.img
	./link test1.img test2.img
	../../tools/disasm lout.img lout.s

test1.img: test1.arg nagt
	./nagt test1.arg $@

test2.img: test2.arg nagt
	./nagt test2.arg $@



ENDS=agtman xagtman

all: $(ENDS)

xagtman: *.c
	cd gtk; make

agtman: *.c
	cd curses; make

clean:
	rm -f $(ENDS)
	cd gtk; make clean
	cd curses; make clean

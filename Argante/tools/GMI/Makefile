
ENDS=agtgtk agtcur

all: $(ENDS)

agtgtk: *.c
	make -C gtk

agtcur: *.c
	make -C curses

clean:
	rm -f $(ENDS)
	make -C gtk clean
	make -C curses clean

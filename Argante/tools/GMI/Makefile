
ENDS=agtman xagtman

all: $(ENDS)

xagtman: *.c
	@$(MAKE) -C gtk    TOP=$(TOP)

agtman: *.c
	@$(MAKE) -C curses TOP=$(TOP)

clean:
	rm -f $(ENDS)
	@$(MAKE) -C gtk    TOP=$(TOP) clean
	@$(MAKE) -C curses TOP=$(TOP) clean

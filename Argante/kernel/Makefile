OBJECTS = bcode.o console.o manager.o module.o task.o acman.o debugger.o cmd.o
MAIN    = main-boot.c

CFLAGS+=-D IDSTR=\"$(IDSTR)\"

all: argante
	install -s $< ..

argante: $(OBJECTS) $(MAIN)
	$(CC) $(CFLAGS) $(LDLIBS) $^ -o $@

clean:
	rm -f $(OBJECTS) argante


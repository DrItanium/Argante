
CFLAGS=-O2 -Wall -g
LDLIBS=

%.o: ../%.c
	$(CC) $(CFLAGS) $< -c -o $@

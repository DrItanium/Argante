
CC = gcc

AFLAGS =-I../include -I. -shared -fPIC

MODULES = display.so fs.so ipc.so network.so advmem.so access.so locallib.so math.so packet.so gfx.so

all: ${MODULES}

display.so : display.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

fs.so : fs.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

ipc.so : ipc.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

network.so : network.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

advmem.so : advmem.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

access.so : access.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

locallib.so : locallib.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

math.so : math.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

packet.so : packet.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

gfx.so : gfx.c
	${CC} -o $@ ${CFLAGS} $^ ${AFLAGS}

clean: 
	rm -f *.so

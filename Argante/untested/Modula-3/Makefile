CFLAGS=-g
CC=gcc

OBJECTS=m3tree.o m3scaner.o m3parser.o m3common.o m3context.o m3types.o \
	m3code.o m3agt.o

m3 : $(OBJECTS)
	$(CC) $(OBJECTS) -lfl -o m3
m3code.o : m3common.h
m3tree.o : m3common.h m3parser.h
m3context.o : m3common.h m3parser.h
m3scaner.c : m3parser.c m3.l m3common.h 
	flex -om3scaner.c m3.l
m3parser.h : m3parser.c
m3parser.c : m3.y m3common.h m3tree.h
	bison -d -v -o m3parser.c m3.y

clean:
	rm -f *.o core m3scaner.[oc] m3parser.[och] m3parser.output *~ m3

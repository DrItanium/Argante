CFLAGS=-g -O2 -Wall

main: main.o iyy.o string.o i.tab.o expr.o

string.o: string.c

i.tab.h i.tab.c: i.y
	bison -d -v $^

iyy.o: iyy.c
	$(CC) $(CFLAGS) $^ -c -o $@

i.tab.o: i.tab.c

iyy.c: i.l i.tab.h
	flex -p -p -o$@ $<

clean:
	rm -f *.o i.tab.h i.tab.c iyy.c iyy.o main

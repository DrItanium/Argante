CFLAGS=-g -O2 -Wall

main: main.o iyy.o string.o

string.o: string.c

i.tab.h i.tab.c: i.y
	bison -d -v $^

iyy.o: lex.yy.c
	$(CC) $(CFLAGS) $^ -c -o $@

lex.yy.c: i.l
	flex -p -p -o$@ $^

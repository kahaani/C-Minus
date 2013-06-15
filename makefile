CC = gcc
LEX = flex
YACC = bison

CFLAGS =

tmc: scan.o parse.o util.o scan.aux.o parse.aux.o symtab.o main.o
	$(CC) $(CFLAGS) -o tmc scan.o parse.o util.o scan.aux.o parse.aux.o symtab.o main.o

main.o: main.c parse.tab.h
	$(CC) $(CFLAGS) -c main.c -o main.o

symtab.o: symtab.c parse.tab.h
	$(CC) $(CFLAGS) -c symtab.c -o symtab.o

parse.aux.o: parse.aux.c parse.tab.h
	$(CC) $(CFLAGS) -c parse.aux.c -o parse.aux.o

scan.aux.o: scan.aux.c parse.tab.h
	$(CC) $(CFLAGS) -c scan.aux.c -o scan.aux.o

util.o: util.c parse.tab.h
	$(CC) $(CFLAGS) -c util.c -o util.o

scan.o parse.o parse.tab.h: scan.l parse.y globals.h
	$(YACC) -d parse.y
	$(CC) $(CFLAGS) -c parse.tab.c -o parse.o
	$(LEX) scan.l
	$(CC) $(CFLAGS) -c lex.yy.c -o scan.o

clean:
	-rm *.o
	-rm lex.yy.c
	-rm *.tab.*


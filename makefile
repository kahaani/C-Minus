CC = gcc
LEX = flex
YACC = bison

CFLAGS =

cmc: scan.o parse.o semantic.o \
		scan.aux.o parse.aux.o semantic.aux.o \
		util.o main.o
	$(CC) $(CFLAGS) -o cmc \
		scan.o parse.o semantic.o \
		scan.aux.o parse.aux.o semantic.aux.o \
		util.o main.o

main.o: main.c parse.tab.h
	$(CC) $(CFLAGS) -c main.c -o main.o

util.o: util.c parse.tab.h
	$(CC) $(CFLAGS) -c util.c -o util.o

semantic.o: semantic.c parse.tab.h
	$(CC) $(CFLAGS) -c semantic.c -o semantic.o

semantic.aux.o: semantic.aux.c parse.tab.h
	$(CC) $(CFLAGS) -c semantic.aux.c -o semantic.aux.o

parse.aux.o: parse.aux.c parse.tab.h
	$(CC) $(CFLAGS) -c parse.aux.c -o parse.aux.o

scan.aux.o: scan.aux.c parse.tab.h
	$(CC) $(CFLAGS) -c scan.aux.c -o scan.aux.o

scan.o parse.o parse.tab.h: scan.l parse.y \
		globals.h util.h semantic.h \
		scan.aux.h parse.aux.h semantic.h
	$(YACC) -d parse.y
	$(CC) $(CFLAGS) -c parse.tab.c -o parse.o
	$(LEX) scan.l
	$(CC) $(CFLAGS) -c lex.yy.c -o scan.o

clean:
	-rm *.o
	-rm lex.yy.c
	-rm *.tab.*


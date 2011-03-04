# $Id: Makefile,v 1.11 2004/02/04 05:19:37 fabio Exp fabio $

# Makefile for ddcal

# Set PREFIX to the directory where you want to install DDcal.
PREFIX	= /usr/local/bin

# Set WHERE to the directory of the CUDD distribution.
WHERE	= ../
INCLUDE	= $(WHERE)/include
LIBS	= $(WHERE)/cudd/libcudd.a $(WHERE)/mtr/libmtr.a \
	$(WHERE)/st/libst.a $(WHERE)/util/libutil.a \
	$(WHERE)/epd/libepd.a
STDLIBS	= -lfl -lm
LEX	= flex
YACC	= bison -y
# If you do not have flex and bison, you may want to try the next three
# definitions instead of the previous three.
#STDLIBS	= -ll -lm
#LEX	= lex
#YACC	= yacc
#YACC	= byacc
CC	= gcc
#CC	= cc
#CFLAGS	= -g -Wall
CFLAGS	= -g -O6
#CFLAGS	= -O
INSTALL	= cp

# Use these definitions for Unix
EXE	=
PL	=
# Use these definitions for Windows/Cygwin
#EXE	= .exe
#PL	= .pl

PROGRAM = ddcal

.PHONY : all
.PHONY : install
.PHONY : clean
.PHONY : distclean

all:	$(PROGRAM)$(EXE)

lex.yy.c: $(PROGRAM).l $(PROGRAM).h
	$(LEX) $(PROGRAM).l

y.tab.c: $(PROGRAM).y $(PROGRAM).h $(WHERE)/include/cudd.h
	$(YACC) -d $(PROGRAM).y

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -c lex.yy.c

y.tab.o: y.tab.c
	$(CC) $(CFLAGS) -I$(INCLUDE) -c y.tab.c

$(PROGRAM)$(EXE): y.tab.o lex.yy.o $(LIBS)
	$(CC) $(CFLAGS) -o $(PROGRAM) y.tab.o lex.yy.o $(LIBS) $(STDLIBS)

install: $(PROGRAM)$(EXE)
	$(INSTALL) $(PROGRAM)$(EXE) $(PREFIX)
	$(INSTALL) DDcal.pl $(PREFIX)/DDcal$(PL)

clean:
	rm -rf *.o y.tab.c y.tab.h lex.yy.c core *~ *.bak

distclean: clean
	rm -rf $(PROGRAM)$(EXE)

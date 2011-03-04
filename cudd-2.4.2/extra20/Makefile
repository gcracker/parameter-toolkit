# Makefile for EXTRA library of DD-based procedures - October 11, 2003

# comment out the following line to see what's happening
.SILENT:

# modify these macros to point to your include and lib directory
INCDIR = ../include
LIBDIR = ../

# modify this line to compile with compilers other than g++
CC       = gcc

# Flags :
#      -O    = optimize
#      -Wall = display all warnings
#      -c    = compile or assemble source but don't link 
#      -g    = produce debugging information
#      -a    = generate extra code to write profile information;
#              this records the number of times each block is executed

#----------------------------------#
# Define Machine Independent Flags #
#----------------------------------#

# Settings for cc
#ICFLAGS        =
#ICFLAGS	= -g
#ICFLAGS        = -O
# Settings for optimized code with gcc
#ICFLAGS	= -g -Wall
#ICFLAGS	= -g -O3 -Wall
ICFLAGS	= -g -O3 -Wall


#--------------------------------#
# Define Machine Dependent Flags #
#--------------------------------#

# When no special flags are needed
#XCFLAGS	= -DHAVE_IEEE_754 -DBSD
# Linux with Gcc 2.8.1 or higher on i686.
XCFLAGS = -mtune=native -DHAVE_IEEE_754 -DBSD -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8

#LFLAGS = -pg -Wall

NAME = extra

CFILES = extraAddMisc.c extraAddSpectra.c extraBddAuto.c \
	extraBddBoundSet.c extraBddDistance.c extraBddKmap.c \
	extraBddMisc.c extraBddPermute.c extraBddSupp.c \
	extraBddSymm.c extraBddUnate.c extraBddWidth.c \
	extraDdMinterm.c extraDdMisc.c extraDdNodePath.c \
	extraDdPrint.c extraDdShift.c extraDdSigma.c extraDdTimed.c \
	extraDdTransfer.c extraUtilFile.c extraUtilMemory.c \
	extraUtilMisc.c extraUtilProgress.c extraZddCover.c \
	extraZddExor.c extraZddFactor.c extraZddGraph.c \
	extraZddIsop.c extraZddLitCount.c extraZddMaxMin.c \
	extraZddMisc.c extraZddPermute.c extraZddSubSup.c

HEADER = extra.h

OBJS	= $(CFILES:.c=.o)

.SUFFIXES: .o .c

.c.o:
	echo Compiling file $< ...
	$(CC) -c $< -I$(INCDIR) $(ICFLAGS) $(XCFLAGS)

all: lib$(NAME).a

lib$(NAME).a: $(OBJS)
	echo Creating library lib$(NAME).a ...
	ar -r lib$(NAME).a $(OBJS)
	ranlib lib$(NAME).a

distclean:
	rm *.o libextra.a

clean:
	rm *.o libextra.a         



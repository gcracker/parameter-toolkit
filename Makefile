# $Id$
#
# Makefile for the ParaMeter tool
#---------------------------------------------------------------------------

# Beginning of the configuration section. These symbol definitions can
# be overridden from the command line.

PLATFORM := linux_x86_64
#PLATFORM := linux

# linux_x86_64 settings
XCFLAGS64 = -DHAVE_IEEE_754 -DBSD -DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8
# linux settings
XCFLAGS	= -malign-double -DHAVE_IEEE_754 -DBSD

# C++ compiler
CPP	= g++

# Specific options for compilation of C++ files.
#CPPFLAGS =
# Stricter standard conformance for g++.
CPPFLAGS = -std=c++98

# C compiler used for all targets except optimize_dec, which always uses cc.
CC	= gcc
#CC	= icc

# Use ICFLAGS to specify machine-independent compilation flags.
# These three are typical settings for cc.
DEBUG_ICFLAGS	= -g -O0
ICFLAGS	= -g -O6 -Wall

# End of the configuration section.
#---------------------------------------------------------------------------

MFLAG   = -DMNEMOSYNE
MNEMLIB	= ../mnemosyne/libmnem.a

DDWDIR	= .
IDIR	= $(DDWDIR)/include
INCLUDE = -I$(IDIR)

BDIRS	= cudd-2.4.2/cudd \
	  cudd-2.4.2/dddmp \
	  cudd-2.4.2/mtr \
	  cudd-2.4.2/st \
	  cudd-2.4.2/util \
	  cudd-2.4.2/epd

CUDD_HOME = cudd-2.4.2
CUDDDIRS = $(BDIRS) cudd-2.4.2/nanotrav
CMPOPPDIRS = tmt/tmtlib tmt/tmt-x86 svt/svtlib/ adamant/
PMDIRS = ParaMeter/HOOPS-1500 ParaMeter/ParaMeter-0.0.1

#------------------------------------------------------------------------

.PHONY : build
.PHONY : nanotrav
.PHONY : check_leaks
.PHONY : optimize_dec
.PHONY : testcudd
.PHONY : libobj
.PHONY : testobj
.PHONY : testdddmp
.PHONY : testmtr
.PHONY : lint
.PHONY : all
.PHONY : clean
.PHONY : distclean


build:
ifeq ($(PLATFORM), linux_x86_64)
	# first make the stuff for the CUDD library
	@for dir in $(CUDDDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make CC=$(CC) RANLIB=$(RANLIB) MFLAG= MNEMLIB= ICFLAGS="$(ICFLAGS)" XCFLAGS="$(XCFLAGS64)" DDDEBUG="$(DDDEBUG)" MTRDEBUG="$(MTRDEBUG)" LDFLAGS="$(LDFLAGS)" PURE="$(PURE)" EXE="$(EXE)" )\
	done

	# make ParaMeter tools
	@for dir in  $(PMDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make linux_x86_64 )\
	done
	
	# make the cmpopp stuff, which includes tmt and Adamantium
	@for dir in $(CMPOPPDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make)\
	done
else
	@for dir in $(CUDDDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make CC=$(CC) RANLIB=$(RANLIB) MFLAG= MNEMLIB= ICFLAGS="$(ICFLAGS)" XCFLAGS="$(XCFLAGS)" DDDEBUG="$(DDDEBUG)" MTRDEBUG="$(MTRDEBUG)" LDFLAGS="$(LDFLAGS)" PURE="$(PURE)" EXE="$(EXE)" )\
	done
	@for dir in  $(PMDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make linux)\
	done
	@for dir in $(CMPOPPDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make)\
	done
endif 

debug:
ifeq ($(PLATFORM), linux_x86_64)
	@for dir in $(CUDDDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make CC=$(CC) RANLIB=$(RANLIB) MFLAG= MNEMLIB= ICFLAGS="$(DEBUG_ICFLAGS)" XCFLAGS="$(XCFLAGS64)" DDDEBUG="$(DDDEBUG)" MTRDEBUG="$(MTRDEBUG)" LDFLAGS="$(LDFLAGS)" PURE="$(PURE)" EXE="$(EXE)" )\
	done
	@for dir in  $(PMDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make linux_x86_64-debug )\
	done
# the cmpopp stuff currently does not have an easy debug make mode
	@for dir in $(CMPOPPDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make)\
	done
else
	@for dir in $(CUDDDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make CC=$(CC) RANLIB=$(RANLIB) MFLAG= MNEMLIB= ICFLAGS="$(DEBUG_ICFLAGS)" XCFLAGS="$(XCFLAGS)" DDDEBUG="$(DDDEBUG)" MTRDEBUG="$(MTRDEBUG)" LDFLAGS="$(LDFLAGS)" PURE="$(PURE)" EXE="$(EXE)" )\
	done
	@for dir in  $(PMDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make linux-debug)\
	done
	@for dir in $(CMPOPPDIRS); do \
		(cd $$dir; \
		echo Making $$dir ...; \
		make)\
	done
endif 

all:
	sh ./setup.sh
	@for dir in $(DIRS); do \
		(cd $$dir; \
		echo Making all in $$dir ...; \
		make CC=$(CC) RANLIB=$(RANLIB) MFLAG= MNEMLIB= ICFLAGS="$(ICFLAGS)" XCFLAGS="$(XCFLAGS)" DDDEBUG="$(DDDEBUG)" MTRDEBUG="$(MTRDEBUG)" LDFLAGS="$(LDFLAGS)" PURE="$(PURE)" EXE="$(EXE)" all )\
	done

clean:
	@for dir in $(CUDDDIRS); do	\
	    (cd $$dir; 	\
	     echo Cleaning $$dir ...; \
	     make -s clean	) \
	done
	@for dir in  $(PMDIRS); do \
		(cd $$dir; \
		echo Cleaning $$dir ...; \
		make clean)\
	done
	@for dir in $(CMPOPPDIRS); do \
		(cd $$dir; \
		echo Cleaning $$dir ...; \
		make clean)\
	done
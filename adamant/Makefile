# C sources and headers to include in the project

CUDD_HOME := ../cudd-2.4.2/

SRCS := adamant.c adamant_config.c adamant_latency.c adamant_runtime.c	\
	adamant_schedule.c adamant_stats.c				\
	adamant_meminfo_pagetable.c 					\
	heap.c misc.c							\
	adamant_plot.c adamant_cd.c adamant_branch_prediction.c		\
	adamant_window.c adamant_split_oper.c                           \
	adamant_bdd.c adamant_bdd_buffer.c                              \
	adamant_bdd_layer.c adamant_bdd_sort.c                          \
	adamant_zdd.c adamant_dd_hot.c adamant_zdd_test.c
#	adamant_bfd.c

HDRS := adamant.h adamant_config.h adamant_latency.h adamant_runtime.h	\
	adamant_schedule.h adamant_stats.h adamant_plot.h		\
	heap.h misc.h							\
	adamant_cd.h adamant_cd_impl.h adamant_branch_prediction.h	\
	adamant_optimize.h adamant_optimize_type.h			\
	adamant_window.h adamant_bdd.h                                  \
	adamant_bdd_buffer.h adamant_bdd_layer.h                        \
	adamant_bdd_sort.h adamant_zdd.h 				\
	adamant_zdd_test.h adamant_dd_hot.h
#	adamant_bfd.h


CUDD_CFLAGS := -I$(CUDD_HOME)/include -pthread

CUDD_LIBS := -L$(CUDD_HOME)/dddmp -ldddmp -L$(CUDD_HOME)/cudd -lcudd -L$(CUDD_HOME)/mtr \
	-lmtr -L$(CUDD_HOME)/st -lst -L$(CUDD_HOME)/util -lutil \
	-L$(CUDD_HOME)/extra20 -lextra -L$(CUDD_HOME)/epd -lepd \
	-lm -lpthread
#-lbfd

# Executable name (don't include .exe for Windows)
EXE  := adamantium

# dependence on TMT
TMT_LIB := tmt
TMT_DIR ?= ../tmt/tmtlib

# dependence on SVT
SVT_LIB := svt
SVT_DIR ?= ../svt/svtlib

PKGS := glib-2.0 gobject-2.0 

########################################################
# Nothing should need to be changed below this line!
########################################################

# attempt to detect Windows
ifneq ($(origin windir), undefined)
	EXE := $(EXE).exe
endif

OBJS := $(SRCS:%.c=%.o)
DEBUG_OBJS := $(SRCS:%.c=%_dbg.o)

PKG_CFLAGS := `pkg-config $(PKGS) --cflags`
PKG_LDFLAGS := `pkg-config $(PKGS) --libs`

#XCFLAGS	= -funroll-loops -DHAVE_IEEE_754 -DBSD

# NOTE: if you want to use different flags, modify your 
# environment variable instead of changing the Makefile.
#CFLAGS := -Wall -Wno-unused -g $(CFLAGS) $(PKG_CFLAGS) -I $(TMT_DIR) -I $(SVT_DIR) -D_ISOC99_SOURCE $(CUDD_CFLAGS)
CFLAGS := -Wall -Wno-unused -O3 -mtune=native $(CFLAGS) $(XCFLAGS) $(PKG_CFLAGS) -I $(TMT_DIR) -I $(SVT_DIR) -D_ISOC99_SOURCE $(CUDD_CFLAGS)
DEBUG_CFLAGS := -g -Wall -Wno-unused -O0 $(DEBUG_CFLAGS) $(PKG_CFLAGS) -I $(TMT_DIR) -I $(SVT_DIR) -D_ISOC99_SOURCE $(CUDD_CFLAGS)
LDFLAGS := $(LDFLAGS) $(PKG_LDFLAGS) -L $(TMT_DIR) -l$(TMT_LIB) -L $(SVT_DIR) -l$(SVT_LIB) $(CUDD_LIBS) -lbz2 -lgmp

.PHONY: clean all cflags

$(EXE): $(OBJS) $(TMT_DIR)/lib$(TMT_LIB).a $(SVT_DIR)/lib$(SVT_LIB).a 
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $@

debug: $(DEBUG_OBJS) $(TMT_DIR)/lib$(TMT_LIB).a $(SVT_DIR)/lib$(SVT_LIB).a 
	$(CC) $(DEBUG_OBJS) $(DEBUG_CFLAGS) $(LDFLAGS) -o $(EXE)_debug

$(EXE).condor: $(OBJS) $(TMT_DIR)/lib$(TMT_LIB).a $(SVT_DIR)/lib$(SVT_LIB).a 
	condor_compile $(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $@

install: $(EXE)
	cp $(EXE) ../bin

release: $(EXE)

install_condor: $(EXE).condor
	cp $(EXE).condor ../bin

cflags:
	@echo $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

%_dbg.o: %.c
	$(CC) -c $< $(DEBUG_CFLAGS) -o $@

clean:
	$(RM) $(EXE) $(OBJS) $(DEBUG_OBJS) $(EXE)_debug Makefile.dep `find -regex ".*/core\(\.[0-9]+\)?"`

all: clean $(EXE)

cleanrelease: clean

cleandebug: clean

Makefile.dep: Makefile $(SRCS) $(HDRS)
	@$(CC) -MM $(CFLAGS) $(SRCS) > Makefile.dep
	@ctags $(SRCS) $(HDRS)
	@etags $(SRCS) $(HDRS)

-include Makefile.dep

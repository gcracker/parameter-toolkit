#
# Computes average latency for each load
#

# C sources and headers to include in the project
SRCS := latency.c
HDRS := 

# Executable name
EXE  := latency.c

########################################################
# Build setup and rules
########################################################

# Object files
OBJS := $(SRCS:.C=.o)

# atm library
ATM_ROOT_DIR := ${HOME}/atm
ATM_LIB := atm
ATM_CFLAGS := -I$(ATM_ROOT_DIR)/include
ATM_LDFLAGS := -L$(ATM_ROOT_DIR)/lib
ATM_LDLIBS := -l$(ATM_LIB)

# tmt library
TMT_DIR ?= ../../tmtlib
TMT_LIB := tmt
TMT_CFLAGS := -I$(TMT_DIR)
TMT_LDFLAGS := -L$(TMT_DIR)
TMT_LDLIBS := -l$(TMT_LIB) -lbz2

# Packages used by this program
PKGS := glib-2.0 gobject-2.0 libxml-2.0
PKG_CFLAGS := `pkg-config $(PKGS) --cflags`
PKG_LDFLAGS := `pkg-config $(PKGS) --libs-only-L`
PKG_LDLIBS := `pkg-config $(PKGS) --libs-only-l`

# Includes and libraries
CC := gcc
OPT := -O2
CFLAGS := -Wall $(OPT) $(CFLAGS) $(ATM_CFLAGS) $(TMT_CFLAGS) $(PKG_CFLAGS)
LDFLAGS := $(ATM_LDFLAGS) $(TMT_LDFLAGS) $(PKG_LDFLAGS) -static
LDLIBS := $(ATM_LDLIBS) $(TMT_LDLIBS) $(PKG_LDLIBS)

# Rules
.PHONY: clean install $(EXE)

all: $(EXE)

$(EXE): $(OBJS) $(TMT_DIR)/lib$(TMT_LIB).a $(ATM_ROOT_DIR)/lib/lib$(ATM_LIB).a
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	-rm -f *.o $(EXE) *.out *.tested *.failed

Makefile.dep: Makefile $(SRCS) $(HDRS)
	@$(CC) -MM $(CCFLAGS) $(SRCS) > Makefile.dep
	@ctags $(SRCS) $(HDRS)
	@etags $(SRCS) $(HDRS)

-include Makefile.dep

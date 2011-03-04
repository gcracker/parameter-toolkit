##
## TMT (Trace Making Thing) IA-64
##

# C sources and headers to include in the project
SRCS := tmt-ia64.C
HDRS := 

# Executable name
EXE := tmt-ia64

########################################################
# Nothing should need to be changed below this line!
########################################################

OBJS := $(SRCS:.C=.o)

##
## PIN Make configuration
##
-include Makefile.config

##
## pkg-config package dependencies
##
PKGS = glib-2.0
PKG_CFLAGS := `pkg-config $(PKGS) --cflags`
PKG_LDFLAGS := `pkg-config $(PKGS) --libs-only-L`
PKG_LDLIBS := `pkg-config $(PKGS) --libs-only-l`

##
## Tmtlib
##
TMT_ROOT_DIR=../tmt
TMT_CFLAGS=-I$(TMT_ROOT_DIR)
TMT_LDFLAGS=-L$(TMT_ROOT_DIR)
TMT_LDLIBS=-ltmt -lbz2

##
## Includes and Libraries
##
LDFLAGS = $(DBG) $(PIN_LDFLAGS) $(TMT_LDFLAGS) $(PKG_LDFLAGS)
LD32FLAGS = $(DBG) $(PIN_LDFLAGS) $(TMT_LDFLAGS) $(PKG_LDFLAGS)
LDLIBS = $(PIN_LDLIBS) $(TMT_LDLIBS) $(PKG_LDLIBS)
CFLAGS = $(DBG) $(OPT) -Wall $(TMT_CFLAGS) $(PKG_CFLAGS) $(PIN_CFLAGS)
CXXFLAGS = $(CFLAGS)

.PHONY: clean all install

install: $(EXE)
	cp $(EXE) ../bin

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) $(EXE) $(OBJS) `find -regex ".*/core\(\.[0-9]+\)?"`

all: clean $(EXE)


Makefile.dep: Makefile $(SRCS) $(HDRS)
	@$(CC) -MM $(CFLAGS) $(SRCS) > Makefile.dep
	@ctags $(SRCS) $(HDRS)
	@etags $(SRCS) $(HDRS)

-include Makefile.dep

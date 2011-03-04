#
# TMT (Trace Making Thing) x86
#

# C sources and headers to include in the project
SRCS := tmt-x86.C
HDRS := 

# Executable name
EXE := tmt-x86

########################################################
# Build setup and rules
########################################################

# GLib
GLIB_CFLAGS := `pkg-config --cflags glib-2.0`
GLIB_LDFLAGS := `pkg-config --libs-only-L glib-2.0`
GLIB_LDLIBS := `pkg-config --libs-only-l glib-2.0`

# Tmtlib
TMT_ROOT_DIR=../tmtlib
TMT_CFLAGS=-I$(TMT_ROOT_DIR)
TMT_LDFLAGS=-L$(TMT_ROOT_DIR)
TMT_LDLIBS=-ltmt -lbz2

# PIN
ifndef PIN_ROOT
PIN_ROOT = $(HOME)/pin-2.6
endif

PIN_KIT = $(PIN_ROOT)
PIN_HOME = $(PIN_ROOT)/source
-include $(PIN_HOME)/tools/makefile.gnu.config

# Object files
OBJS := $(SRCS:.C=.o)

# Includes and libraries
OPT=-O2
CFLAGS = -DPINTOOL_BUILD -Wno-unknown-pragmas $(OPT) # -Werror
CXXFLAGS = $(CFLAGS) $(PIN_CXXFLAGS) $(TMT_CFLAGS) $(GLIB_CFLAGS)
LDFLAGS = $(PIN_LDFLAGS) $(TMT_LDFLAGS) $(GLIB_LDFLAGS)
LDLIBS = $(PIN_LIBS) $(TMT_LDLIBS) $(GLIB_LDLIBS)

# Rules

.PHONY: all install

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

install: $(EXE)
	cp ${EXE} ../../bin

clean:
	$(RM) -f *.o $(EXE) *.out *.tested *.failed

#makefile.dep: makefile $(SRCS) $(HDRS)
#	@$(CXX) -MM $(CXXFLAGS) $(SRCS) > makefile.dep
#	@ctags $(SRCS) $(HDRS)
#	@etags $(SRCS) $(HDRS)
# -include makefile.dep

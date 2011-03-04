##
## TMT (Trace Making Thing) Library Makefile
##

# C sources and headers to include in the project
LIB_SRCS := tmt.c tmtoper.c tmtio.c tmthash.c tmt_ia64.c
BIN_SRCS := more_dests.c
HDRS := tmt.h tmtoper.h tmtio.h tmthash.h TCgen.h tmt_ia64.h tmt_isa.h
# Note that TCgen.c is #included in tmt.c so the funcs get inlined


# Library name (e.g. for libxxx.a, just put xxx)
LIB  := tmt

PKGS := glib-2.0

CC := gcc

########################################################
# Nothing should need to be changed below this line!
########################################################

BIN := more_dests

LIB := lib$(LIB).a

LIB_OBJS := $(LIB_SRCS:%.c=%.o)
BIN_OBJS := $(BIN_SRCS:%.c=%.o)

PKG_CFLAGS := `pkg-config $(PKGS) --cflags`
PKG_LDFLAGS := `pkg-config $(PKGS) --libs`

# NOTE: if you want to use different flags, modify your 
# environment variable instead of changing the Makefile.
CFLAGS := -Wall -O2 -fPIC $(CFLAGS) $(PKG_CFLAGS)
LDFLAGS := $(LDFLAGS) $(PKG_LDFLAGS) -lbz2 -L. -ltmt

.PHONY: clean all install

all: $(LIB) $(BIN)

install: $(BIN)
	cp $(BIN) ../bin

$(LIB): $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $?

$(BIN): $(BIN_OBJS) $(LIB)
	$(CC) -o $@ $(BIN_OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(LIB) $(LIB_OBJS) $(BIN_OBJS) `find -regex ".*/core\(\.[0-9]+\)?"`

Makefile.dep: Makefile $(BIN_SRCS) $(LIB_SRCS) $(HDRS)
	@$(CC) -MM $(CFLAGS) $(BIN_SRCS) $(LIB_SRCS) > Makefile.dep
	@ctags $(LIB_SRCS) $(BIN_SRCS) $(HDRS)
	@etags $(LIB_SRCS) $(BIN_SRCS) $(HDRS)

-include Makefile.dep

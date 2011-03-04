##
## SVT (Schedule Viewing Thing) Library Makefile
##

# C sources and headers to include in the project
LIB_SRCS := sched_io.c TCgen_sched.c pagetable64.c plot.c
HDRS := sched_io.h TCgen_sched.h pagetable64.h plot.h

# Library name (e.g. for libxxx.a, just put xxx)
LIB  := svt

PKGS := glib-2.0

########################################################
# Nothing should need to be changed below this line!
########################################################

LIB := lib$(LIB).a

LIB_OBJS := $(LIB_SRCS:%.c=%.o)

PKG_CFLAGS := `pkg-config $(PKGS) --cflags`
PKG_LDFLAGS := `pkg-config $(PKGS) --libs`

# NOTE: if you want to use different flags, modify your 
# environment variable instead of changing the Makefile.
CFLAGS := -Wall -O2 -I. $(CFLAGS) $(PKG_CFLAGS)
LDFLAGS := $(LDFLAGS) $(PKG_LDFLAGS) -lbz2 -L. -ltmt

.PHONY: clean all

all: $(LIB)

$(LIB): $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $?

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(LIB) $(LIB_OBJS) `find -regex ".*/core\(\.[0-9]+\)?"`

Makefile.dep: Makefile $(LIB_SRCS) $(HDRS)
	@$(CC) -MM $(CFLAGS) $(LIB_SRCS) > Makefile.dep
	@ctags $(LIB_SRCS) $(HDRS)
	@etags $(LIB_SRCS) $(HDRS)

-include Makefile.dep

##
## TMT (Schedule Viewing Thing) Library Makefile
##

# C sources and headers to include in the project
dump_prog_SRCS := dump-prog.c
dump_prog_HDRS := 
dump_prog_OBJS := $(dump_prog_SRCS:%.c=%.o)

SRCS := $(dump_prog_SRCS)
HDRS := $(dump_prog_HDRS)
OBJS := $(SRCS:%.c=%.o)

BINS := dump-prog


# GLib
GLIB_CFLAGS := `pkg-config --cflags glib-2.0`
GLIB_LDFLAGS := `pkg-config --libs-only-L glib-2.0`
GLIB_LDLIBS := `pkg-config --libs-only-l glib-2.0`

# TMT
TMT_ROOT_DIR=../../tmtlib
TMT_CFLAGS=-I$(TMT_ROOT_DIR)
TMT_LDFLAGS=-L$(TMT_ROOT_DIR)
TMT_LDLIBS=-ltmt -lbz2

CFLAGS := -D_ISOC99_SOURCE -O2 -Wall $(CFLAGS) $(GLIB_CFLAGS) $(TMT_CFLAGS)
LDFLAGS := $(LDFLAGS) $(GLIB_LDFLAGS) $(TMT_LDFLAGS)
LDLIBS := $(LDLIBS) $(GLIB_LDLIBS) $(TMT_LDLIBS)


.PHONY: clean all install

all: $(BINS)

dump-prog: $(dump_prog_OBJS) $(TMT_ROOT_DIR)/libtmt.a
	$(CC) -o $@ $< $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(OBJS) `find -regex ".*/core\(\.[0-9]+\)?"`

install: $(BINS)
	cp $(BINS) ../../../bin

Makefile.dep: Makefile $(SRCS) $(HDRS)
	@$(CC) -MM $(CFLAGS) $(SRCS) > Makefile.dep
	@ctags $(SRCS) $(HDRS)
	@etags $(SRCS) $(HDRS)

-include Makefile.dep

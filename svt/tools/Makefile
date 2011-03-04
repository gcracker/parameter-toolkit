##
## SVT (Schedule Viewing Thing) Library Makefile
##

# C sources and headers to include in the project
display_schedule_SRCS := display-schedule.c
display_schedule_HDRS := 
display_schedule_OBJS := $(display_schedule_SRCS:%.c=%.o)

SRCS := $(display_schedule_SRCS)
HDRS := $(display_schedule_HDRS)
OBJS := $(SRCS:%.c=%.o)

BINS := display-schedule


# GLib
GLIB_CFLAGS := `pkg-config --cflags glib-2.0`
GLIB_LDFLAGS := `pkg-config --libs-only-L glib-2.0`
GLIB_LDLIBS := `pkg-config --libs-only-l glib-2.0`

# SVT
SVT_ROOT_DIR=../svtlib
SVT_CFLAGS=-I$(SVT_ROOT_DIR)
SVT_LDFLAGS=-L$(SVT_ROOT_DIR)
SVT_LDLIBS=-lsvt -lbz2

CFLAGS := -D_ISOC99_SOURCE -O2 -Wall $(CFLAGS) $(GLIB_CFLAGS) $(SVT_CFLAGS)
LDFLAGS := $(LDFLAGS) $(GLIB_LDFLAGS) $(SVT_LDFLAGS)
LDLIBS := $(LDLIBS) $(GLIB_LDLIBS) $(SVT_LDLIBS)


.PHONY: clean all install

all: $(BINS)

display-schedule: $(display_schedule_OBJS) $(SVT_ROOT_DIR)/libsvt.a
	$(CC) -o $@ $< $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(OBJS) `find -regex ".*/core\(\.[0-9]+\)?"`

install: $(BINS)
	cp $(BINS) ../../bin

Makefile.dep: Makefile $(SRCS) $(HDRS)
	@$(CC) -MM $(CFLAGS) $(SRCS) > Makefile.dep
	@ctags $(SRCS) $(HDRS)
	@etags $(SRCS) $(HDRS)

-include Makefile.dep

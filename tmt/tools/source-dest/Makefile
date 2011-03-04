# C sources and headers to include in the project
SRCS := srcdest.c
HDRS := 

# Executable name (don't include .exe for Windows)
EXE  := source-dest

########################################################
# Nothing should need to be changed below this line!
########################################################

OBJS := $(SRCS:%.c=%.o)

# dependence on TMT
TMT_LIB := tmt
TMT_DIR ?= ../../tmtlib

# pkg-config packages
PKGS := glib-2.0
PKG_CFLAGS := `pkg-config $(PKGS) --cflags`
PKG_LDFLAGS := `pkg-config $(PKGS) --libs`

# NOTE: if you want to use different flags, modify your 
# environment variable instead of changing the Makefile.
CFLAGS := -Wall $(CFLAGS) $(PKG_CFLAGS) -I $(TMT_DIR)
LDFLAGS := $(LDFLAGS) $(PKG_LDFLAGS) -L $(TMT_DIR) -l$(TMT_LIB) \
           -lbz2 -static

.PHONY: clean all cflags

$(EXE): $(OBJS) $(TMT_DIR)/lib$(TMT_LIB).a 
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $@

install: $(EXE)
	cp $(EXE) ../../../bin

cflags:
	@echo $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	$(RM) $(EXE) $(OBJS) `find -regex ".*/core\(\.[0-9]+\)?"`

all: clean $(EXE)

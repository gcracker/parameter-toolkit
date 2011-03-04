PKGS := glib-2.0

OBJECTS :=  reverser.o trace_reverse.o

GLIB_CFLAGS := `pkg-config $(PKGS) --cflags`
GLIB_LDFLAGS := `pkg-config $(PKGS) --libs`

TMT_CFLAGS := -I ../../tmt/tmtlib 
TMT_LDFLAGS := -L ../../tmt/tmtlib -ltmt

REV_CFLAGS :=  -Wall $(GLIB_CFLAGS) $(TMT_CFLAGS)
REV_LDFLAGS :=  $(GLIB_LDFLAGS) $(TMT_LDFLAGS)

CFLAGS := -O3  $(REV_CFLAGS) 

LDFLAGS := $(REV_LDFLAGS) -lbz2

trace_reverse: $(OBJECTS)
	gcc $(OBJECTS) $(LDFLAGS) -o trace_reverse

reverser.o: reverser.c reverser.h
	gcc $(CFLAGS) -c reverser.c

trace_reverse.o: trace_reverse.c reverser.c reverser.h
	gcc $(CFLAGS) -c trace_reverse.c reverser.c

clean:
	rm -f core* trace_reverse $(OBJECTS)

install:
	rm -rf ./bin/trace_reverse
	cp trace_reverse ./bin/

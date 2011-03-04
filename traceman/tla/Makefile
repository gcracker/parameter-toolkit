PKGS := glib-2.0

OBJECTS :=  tlaio.o tla.o bst.o

GLIB_CFLAGS := `pkg-config $(PKGS) --cflags`
GLIB_LDFLAGS := `pkg-config $(PKGS) --libs`

TMT_CFLAGS := -I ../../tmt/tmtlib 
TMT_LDFLAGS := -L ../../tmt/tmtlib -ltmt

SVT_CFLAGS:= -I ../../svt/svtlib
SVT_LD_FLAGS:= -L ../../svt/svtlib -lsvt

REV_CFLAGS :=  -Wall $(GLIB_CFLAGS) $(TMT_CFLAGS) $(SVT_CFLAGS)
REV_LDFLAGS :=  $(GLIB_LDFLAGS) $(TMT_LDFLAGS) $(SVT_LD_FLAGS)

CFLAGS := -O3  $(REV_CFLAGS) 

LDFLAGS := $(REV_LDFLAGS) -lbz2

tla: $(OBJECTS)
	gcc $(OBJECTS) $(LDFLAGS) -o tla

bst.o: bst.c bst.h dict_info.h
	gcc $(CFLAGS) -c bst.c

tlaio.o: tlaio.c tlaio.h bst.c bst.h dict_info.h 
	gcc $(CFLAGS) -c tlaio.c bst.c

tla.o: tla.c tlaio.c tlaio.h bst.c bst.h dict_info.h
	gcc $(CFLAGS) -c tla.c tlaio.c bst.c

clean:
	rm -f core* tla $(OBJECTS)

install: tla
	rm -rf ./bin/tla
	cp tla ./bin/

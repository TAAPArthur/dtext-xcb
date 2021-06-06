FT_CFLAGS  = `freetype-config --cflags`
FT_LDFLAGS = `freetype-config --libs`

XLIB_LDFLAGS = -lxcb -lxcb-render -lxcb-render-util

CFLAGS  += -std=c99 ${FT_CFLAGS} -Wall -Wextra -pedantic
LDFLAGS += ${XLIB_LDFLAGS} ${FT_LDFLAGS}

EXECS := $(patsubst examples/%.c,build/%,$(wildcard examples/*.c))

all: libdtext.so

libdtext.so: dtext.c
	${CC} ${CFLAGS} -fPIC -shared -o $@ $^ ${CFLAGS} ${LDFLAGS}

install: libdtext.so
	install -Dt $(DESTDIR)/usr/lib/ $^

examples: $(EXECS)

build:
	mkdir build

build/%: examples/%.c dtext.c dtext.h | build
	${CC} -I. ${CFLAGS} ${LDFLAGS} $< dtext.c -o $@

clean:
	rm -f $(EXECS) *.so

.PHONY: all clean

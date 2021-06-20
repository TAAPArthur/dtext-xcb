FT_CFLAGS  = -I /usr/include/freetype2
FT_LDFLAGS = -lfreetype

XLIB_LDFLAGS = -lxcb -lxcb-render -lxcb-render-util

CFLAGS  ?= -std=c99 -Wall
CFLAGS  += ${FT_CFLAGS}
LDFLAGS += ${XLIB_LDFLAGS} ${FT_LDFLAGS}

ifdef DEBUG
CFLAGS += -g
endif

SRCS := $(wildcard *.c)
EXECS := $(patsubst examples/%.c,build/%,$(wildcard examples/*.c))

all: libdtext.so

libdtext.so: $(wildcard *.c)
	${CC} ${CFLAGS} -fPIC -shared -o $@ $^ ${LDFLAGS}

install: libdtext.so
	install -Dt $(DESTDIR)/usr/lib/ $^
	install -Dt $(DESTDIR)/usr/include/dtext/ *.h

examples: $(EXECS)

build:
	mkdir build


build/%: examples/%.c $(SRCS)  | build
	${CC} -I. ${CFLAGS} -g ${LDFLAGS} $^ -o $@

clean:
	rm -f $(EXECS) *.so

.PHONY: all clean

# See LICENSE file for copyright and license details.

WARN_CFLAGS = -Wall -Wextra -pedantic -D_XOPEN_SOURCE=700

DBG_CFLAGS  = -g
DBG_LDFLAGS = -g

FT_CFLAGS  = `freetype-config --cflags`
FT_LDFLAGS = `freetype-config --libs`

XLIB_CFLAGS  = -I/usr/X11R6/include/
XLIB_LDFLAGS = -lxcb -lxcb-render -lxcb-render-util

CFLAGS  += -std=c99 ${XLIB_CFLAGS} ${FT_CFLAGS} ${WARN_CFLAGS} ${DBG_CFLAGS}
LDFLAGS += ${XLIB_LDFLAGS} ${FT_LDFLAGS} ${DBG_LDFLAGS}
# CC = cc

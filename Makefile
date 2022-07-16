all:

install:
	install -Dt $(DESTDIR)/usr/include/ dtext_xcb.h

uninstall:
	rm $(DESTDIR)/usr/include/dtext_xcb.h

.PHONY: all

###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3

SVNVER = `svnversion`

include config.mak

.PHONY: clean Release Debug libemugrph-debug ui-agar-debug vm-debug libemugrph-release ui-release vm-release debugger-debug debugger-release
all: Release Debug

Release: ui-agar-release libemugrph-release vm-release debugger-release
	make -C sdl xm7

Debug: ui-agar-debug libemugrph-debug vm-debug debugger-debug
	make -C sdl xm7.debug


libemugrph-release:
	make -C libemugrph Release/libemugrph.a

libemugrph-debug:
	make -C libemugrph Debug/libemugrph.a

ui-agar-release:
	make -C ui-agar Release/libui-agar.a

ui-agar-debug:
	make -C ui-agar Debug/libui-agar.a

vm-release:
	make -C vm Release/libxm7_vm.a

vm-debug:
	make -C vm Debug/libxm7_vm.a

debugger-release:
	make -C xm7-debugger Release/libxm7_debug.a

debugger-debug:
	make -C xm7-debugger Debug/libxm7_debug.a



install: all
	$(INSTALL)  -d $(SHAREDIR)
	$(INSTALL) ./sdl/xm7 $(PREFIX)/bin/xm7
	$(INSTALL) ./sdl/xm7.debug $(PREFIX)/bin/xm7.debug
	$(INSTALL) -m 0644 ./resource/*.ico $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.xml $(SHAREDIR)
#	$(INSTALL) -m 0644 ./ipagui.ttf $(SHAREDIR)


clean:
	make -C vm clean
	make -C ui-agar clean
	make -C libemugrph clean
	make -C xm7-debugger clean
	make -C sdl clean
	
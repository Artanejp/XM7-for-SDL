###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3


include config.mak
include common.mak

.PHONY: clean Release Debug libfmgen008-debug ui-agar-debug vm-debug libfmgen008-release ui-release vm-release debugger-debug debugger-release
all: Release Debug


ifdef WITH_DEBUGGER
Release: ui-agar-release vm-release debugger-release libfmgen008-release
	make -C sdl $(TARGET_RELEASE)

Debug: ui-agar-debug vm-debug debugger-debug libfmgen008-debug
	make -C sdl $(TARGET_DEBUG)
else
Release: ui-agar-release vm-release libfmgen008-release
	make -C sdl $(TARGET_RELEASE)

Debug: ui-agar-debug vm-debug libfmgen008-debug
	make -C sdl $(TARGET_DEBUG)
endif

libfmgen008-release:
	make -C fmgen Release/libfmgen007a.a

libfmgen008-debug:
	make -C fmgen Debug/libfmgen007a.a

ui-agar-release:
	make -C ui-agar Release

ui-agar-debug:
	make -C ui-agar Debug

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
	$(INSTALL) -m 0644 ./resource/*.png $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.xml $(SHAREDIR)
	$(INSTALL) -m 0644 ./locales/ja/*.mo ./locales/ja/*.po $(SHAREDIR)ja/LC_MESSAGES/
	$(INSTALL) -m 0644 ./locales/C/*.mo ./locales/C/*.po $(SHAREDIR)C/LC_MESSAGES/
#	$(INSTALL) -m 0644 ./ipagui.ttf $(SHAREDIR)


clean:
	make -C vm clean
	make -C ui-agar clean
	make -C fmgen clean
	make -C xm7-debugger clean
	make -C sdl clean
	
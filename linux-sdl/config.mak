###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
#XM7_VER		= 3
#OPTION		= -DMOUSE -DMR2 -DFDDSND -DMEMWATCH
OPTION		= -DMOUSE -DMR2 -DFDDSND
OPTION          += -DUSE_OPENGL
#OPTION		= -DMOUSE -DMR2
#OPTION		+= -DUSE_GTKOPTION		+= -DUSE_GTKOPTION		+= -DUSE_GTKOPTION		+= -DUSE_GTK
OPTION		+= -DUSE_AGAR

INSTALL          = install -c 
PREFIX           = /usr/local
SHAREDIR         = $(PREFIX)/share/xm7/

## Embed目的の場合はkanameやkonatuのような小さなフォントにすること
UI_FONT           = ipagp.ttf
STAT_FONT          = ipagp.ttf
VFD_FONT           = ipagui.ttf
CMT_FONT           = ipagui.ttf
FONTPATH           = /usr/local/share/xm7:/usr/share/fonts/X11/misc/:/usr/share/fonts/truetype/ipafont:/usr/share/fonts/truetype/mona:/usr/share/fonts/truetype/misaki:.:./.xm7

LDFLAGS = 
ifeq ($(build_type),Debug)
CFLAGS = -g -O0 -export-dynamic
else
CFLAGS =  -O3 -export-dynamic
CFLAGS += -msse4a -mabm -m3dnow
endif

CFLAGS += -DXM7_VER=$(XM7_VER) $(OPTION) -D_XWIN -DNDEBUG -DUIDIR=\"$(SHAREDIR)\" -DRSSDIR=\"$(SHAREDIR)\"
CFLAGS += -DUI_FONT=\"$(UI_FONT)\" -DFUNC_FONT=\"$(FUNC_FONT)\" -DSTAT_FONT=\"$(STAT_FONT)\" -DVFD_FONT=\"$(VFD_FONT)\" -DCMT_FONT=\"$(CMT_FONT)\"
CFLAGS += -DFONTPATH=\"$(FONTPATH)\"
CFLAGS +=  -I. -I/usr/local/include -I/usr/local/include/libemugrph
CFLAGS += `pkg-config --cflags sdl`
#CFLAGS += `pkg-config --cflags gtk+-x11-2.0` `pkg-config --cflags gdk-pixbuf-2.0`
CFLAGS += -I/usr/local/include/agar
ASFLAGS =	-DXM7_VER=$(XM7_VER) -f elf -d _XWIN

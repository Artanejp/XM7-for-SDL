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

## SVNの時はコメントアウト外す
SVNVER = `svnversion`


## Embed目的の場合はkanameやkonatuのような小さなフォントにすること
UI_FONT           = ipagp.ttf
STAT_FONT          = ipagp.ttf
VFD_FONT           = ipagui.ttf
CMT_FONT           = ipagui.ttf
FONTPATH           = /usr/local/share/xm7:/usr/share/fonts/X11/misc/:/usr/share/fonts/truetype/ipafont:/usr/share/fonts/truetype/mona:/usr/share/fonts/truetype/misaki:.:./.xm7

LDFLAGS = 

CC = gcc
CXX = g++
AR = ar

CFLAGS = -DXM7_VER=$(XM7_VER) $(OPTION) -D_XWIN -DNDEBUG -DUIDIR=\"$(SHAREDIR)\" -DRSSDIR=\"$(SHAREDIR)\"
CFLAGS += -DUI_FONT=\"$(UI_FONT)\" -DFUNC_FONT=\"$(FUNC_FONT)\" -DSTAT_FONT=\"$(STAT_FONT)\" -DVFD_FONT=\"$(VFD_FONT)\" -DCMT_FONT=\"$(CMT_FONT)\"
CFLAGS += -DFONTPATH=\"$(FONTPATH)\"

ifdef SVNVER
CFLAGS += -DSVNVER=\"$(SVNVER)\"
endif

CFLAGS +=  -I. -I/usr/local/include
CFLAGS += -I../libemugrph/ -I../ui-agar/ -I../sdl/ -I../vm/
CFLAGS += `pkg-config --cflags sdl`
#CFLAGS += -I/usr/local/include/SDL
CFLAGS += `agar-config --cflags`
# OpenMP
CFLAGS += -fopenmp

CFLAGS_DEBUG = -pg -g -O0 -export-dynamic
CFLAGS_DEBUG += $(CFLAGS)

CFLAGS_RELEASE =  -O3 -export-dynamic
#CFLAGS_RELEASE += -msse4a -mabm -m3dnow
CFLAGS_RELEASE += $(CFLAGS)

ASFLAGS =	-DXM7_VER=$(XM7_VER) -f elf -d _XWIN

###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile for SDL / vm]
XM7_VER		= 3
include ../config.mak

COMMONSRC = \
	agar_cfg.cpp \
	agar_cmd.cpp \
	agar_draw.cpp \
	agar_guicore.cpp \
	agar_osd.cpp \
	agar_osd_led.cpp \
	agar_osd_stat.cpp \
	agar_osd_tape.cpp \
	agar_osd_vfd.cpp \
	agar_sch.cpp \
	agar_sdl.cpp \
	agar_sdlscaler.cpp \
	agar_sdlview.c \
	agar_svram.cpp \
	agar_vramutil.cpp \
	agar_main.cpp \
	AgarKbdInterface.cpp

GUISRC = \
	agar_gui_menu.cpp \
	agar_gui_pref.cpp \
	agar_menu_about.cpp \
	agar_menu_debug.cpp \
	agar_menu_disk.cpp \
	agar_menu_tape.cpp \
	agar_menu_tool.cpp \
	agar_ui_vkey.cpp \
	agar_toolbox.cpp \
	
DEBUGSRC = \
	agar_surfaceconsole.cpp \
	agar_dbg_disasm.cpp \
	agar_dbg_fdcdump.cpp \
	agar_dbg_memdump.cpp \
	agar_dbg_mmr.cpp \
	agar_dbg_regdump.cpp \


GLSRC = \
	agar_gldraw2.cpp \
	agar_glutil.cpp
	
.SUFFIXES: .c .cpp .asm .o

SRCS =	$(COMMONSRC) $(GLSRC) $(GUISRC) $(DEBUGSRC)
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

all:	Release/libui-agar.a Debug/libui-agar.a 

Debug/libui-agar.a: ${OBJS_DEBUG}
	$(AR) crv $@ ${OBJS_DEBUG}
	ranlib $@

Release/libui-agar.a: ${OBJS_RELEASE}
	$(AR) crv $@ ${OBJS_RELEASE}
	ranlib $@

Release/%.o: %.c
	mkdir -p Release
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/%.o: %.cpp
	mkdir -p Release
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Debug/%.o: %.c
	mkdir -p Debug
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/%.o: %.cpp
	mkdir -p Debug
	$(CXX) ${CXXFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

	

.asm.o:
	nasm ${ASFLAGS} $<

install: all
	$(INSTALL)  -d $(SHAREDIR)
	$(INSTALL) ./xm7 $(PREFIX)/bin
	$(INSTALL) -m 0644 *.ui $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.ico $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.xml $(SHAREDIR)
#	$(INSTALL) -m 0644 ./ipagui.ttf $(SHAREDIR)

.PHONY: clean
clean:
	rm -f Debug/*.o Release/*.o
	rm -f Release/*.so Debug/*.so
	rm -f Debug/*.lo Release/*.lo
	rm -f Release/*.a Debug/*.a
#	rm -f *~
###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3

include ../config.mak

#CFLAGS += -I../xm7-sdl/

VIDEODRVSRC = \
	agar_draw.cpp agar_gldraw2.cpp \
	agar_glutil.cpp \
	agar_sdlview.c \
	agar_sdlscaler.cpp

INPUTSRC = \
	AgarKbdInterface.cpp \
	agar_ui_vkey.cpp

UISRC = \
	agar_guicore.cpp \
	agar_toolbox.cpp \
	agar_gui_menu.cpp \
	agar_menu_disk.cpp \
	agar_menu_tape.cpp \
	agar_menu_debug.cpp \
	agar_menu_tool.cpp \
	agar_menu_about.cpp \
	agar_gui_pref.cpp 


DEBUGGERSRC = \
	agar_surfaceconsole.cpp \
	agar_dbg_memdump.cpp \
	agar_dbg_disasm.cpp \
	agar_dbg_regdump.cpp \
	agar_dbg_fdcdump.cpp


LOCALSRC = \
	agar_cfg.cpp  \
        agar_cmd.cpp agar_sch.cpp \
	agar_main.cpp agar_sdl.cpp \
	agar_osd.cpp \
	agar_vramutil.cpp \
	agar_svram.cpp \
	$(VIDEODRVSRC) \
	$(INPUTSRC) \
	$(UISRC) \
	$(DEBUGGERSRC)

SRCS =	${LOCALSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

.SUFFIXES: .o .c .cc .cpp 

all:	Debug/libui-agar.a  Release/libui-agar.a

Debug/libui-agar.a: ${OBJS_DEBUG}
	ar crv $@ ${OBJS_DEBUG}

Release/libui-agar.a: ${OBJS_RELEASE}
	ar crv $@ ${OBJS_RELEASE}

Release/%.o: %.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/%.o: %.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Debug/%.o: %.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/%.o: %.cpp
	$(CXX) ${CXXFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)



.PHONY: clean
clean:
	rm -f *.la *.a *.o *.lo *~
	rm -f Release/*.o Release/*.lo Release/*.a Release/*.la
	rm -f Debug/*.la Debug/*.a Debug/*.o Debug/*.lo *~


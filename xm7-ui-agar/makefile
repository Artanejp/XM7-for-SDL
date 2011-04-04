###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3
include ../xm7-sdl/config.mak

#CFLAGS += -I../xm7-sdl/

VIDEODRVSRC = \
	agar_draw.cpp agar_gldraw.cpp

INPUTSRC = \
	AgarKbdInterface.cpp \
	agar_ui_vkey.cpp

UISRC = \
	agar_guicore.cpp \
	agar_toolbox.cpp \
	agar_gui_menu.cpp \
	agar_menu_disk.cpp \
	agar_menu_tape.cpp \
	agar_menu_tool.cpp \
	agar_menu_about.cpp \
	agar_gui_pref.cpp 
	
LOCALSRC = \
	agar_cfg.cpp  \
        agar_cmd.cpp agar_sch.cpp \
	agar_main.cpp agar_sdl.cpp \
	agar_osd.cpp \
	$(VIDEODRVSRC) \
	$(INPUTSRC) \
	$(UISRC)

SRCS =	${LOCALSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))





.SUFFIXES: .o .c .cc .cpp 
all:	Debug/libxm7-ui-agar.a  Release/libxm7-ui-agar.a

Debug/libxm7-ui-agar.a: ${OBJS_DEBUG}
	ar crv $@ ${OBJS_DEBUG}

Release/libxm7-ui-agar.a: ${OBJS_RELEASE}
	ar crv $@ ${OBJS_RELEASE}

Release/agar_draw.o: agar_draw.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/agar_gldraw.o: agar_gldraw.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/AgarKbdInterface.o: AgarKbdInterface.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_ui_vkey.o: agar_ui_vkey.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_guicore.o: agar_guicore.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_toolbox.o: agar_toolbox.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_gui_menu.o: agar_gui_menu.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_menu_disk.o: agar_menu_disk.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_menu_tape.o: agar_menu_tape.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_menu_tool.o: agar_menu_tool.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_menu_about.o: agar_menu_about.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_gui_pref.o: agar_gui_pref.cpp 
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_cfg.o: agar_cfg.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_cmd.o: agar_cmd.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_sch.o: agar_sch.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/agar_main.o: agar_main.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_sdl.o: agar_sdl.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/agar_osd.o: agar_osd.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

#########
Debug/agar_draw.o: agar_draw.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/agar_gldraw.o: agar_gldraw.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/AgarKbdInterface.o: AgarKbdInterface.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_ui_vkey.o: agar_ui_vkey.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_guicore.o: agar_guicore.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_toolbox.o: agar_toolbox.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_gui_menu.o: agar_gui_menu.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_menu_disk.o: agar_menu_disk.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_menu_tape.o: agar_menu_tape.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_menu_tool.o: agar_menu_tool.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/agar_menu_about.o: agar_menu_about.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_gui_pref.o: agar_gui_pref.cpp 
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_cfg.o: agar_cfg.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/agar_cmd.o: agar_cmd.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_sch.o: agar_sch.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/agar_main.o: agar_main.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_sdl.o: agar_sdl.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/agar_osd.o: agar_osd.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)


#.asm.o:
#	nasm ${ASFLAGS} $<


.PHONY: clean
clean:
	rm -f *.la *.a *.o *.lo *~
	rm -f Release/*.o Release/*.lo Release/*.a Release/*.la
	rm -f Debug/*.la Debug/*.a Debug/*.o Debug/*.lo *~


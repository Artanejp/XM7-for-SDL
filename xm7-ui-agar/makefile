###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3
include ../xm7-sdl/config.mak


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


VIDEODRVOBJ = \
	agar_draw.o agar_gldraw.o

INPUTOBJ = \
	AgarKbdInterface.o \
	agar_ui_vkey.o
	
UIOBJ = \
	agar_guicore.o \
	agar_toolbox.o \
	agar_gui_menu.o \
	agar_menu_disk.o \
	agar_menu_tape.o \
	agar_menu_tool.o \
	agar_menu_about.o \
	agar_gui_pref.o 

LOCALOBJS = \
	agar_cfg.o \
        agar_cmd.o agar_sch.o \
	agar_main.o agar_sdl.o \
	agar_osd.o \
	$(VIDEODRVOBJ)  \
	$(INPUTOBJ) \
	$(UIOBJ)


VIDEODRVSHOBJ = \
	agar_draw.lo agar_gldraw.lo

INPUTSHOBJ = \
	AgarKbdInterface.lo \
	agar_ui_vkey.lo
	
UISHOBJ = \
	agar_guicore.lo \
	agar_toolbox.lo \
	agar_gui_menu.lo \
	agar_menu_disk.lo \
	agar_menu_tape.lo \
	agar_menu_tool.lo \
	agar_menu_about.lo \
	agar_gui_pref.lo 

SHOBJS = \
	agar_cfg.lo \
        agar_cmd.lo agar_sch.lo \
	agar_main.lo agar_sdl.lo \
	agar_osd.lo \
	$(VIDEODRVSHOBJ)  \
	$(INPUTSHOBJ) \
	$(UISHOBJ)




SRCS =	${LOCALSRC}
OBJS =	 ${LOCALOBJS}

LIBS += -L/usr/local/lib

LIBS += `pkg-config --libs sdl`
LIBS +=  -lSDL_ttf

LIBS += -lGL -lGLU
LIBS += -lag_core -lag_gui -lag_dev 
LIBS += -ljpeg -lpng
#LIBS += -lmemwatch
CFLAGS += -I../xm7-sdl/


.SUFFIXES: .o .c .cc .cpp 
all:	libxm7-ui-agar.a 

libxm7-ui-agar.a: ${LOCALOBJS}
	ar crv $@ ${LOCALOBJS}

#libxm7-ui-agar.la: ${SHOBJS}
#	${LIBTOOL}  --mode=link ${CXX} -version-info 0 -rpath ${PREFIX}/lib -o $@ ${SHOBJS} ${LIBS}

.c.o:
	${LIBTOOL} --mode=compile ${CC} ${LIBTOOLFLAGS} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
	
.c.lo: ${LIBTOOL}
	${LIBTOOL} --mode=compile ${CC} ${LIBTOOLFLAGS} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

.c.po:
	${CC} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

.cpp.o:
	${LIBTOOL} --mode=compile ${CXX} ${LIBTOOLFLAGS} ${CFLAGS} ${CPPFLAGS} -o $@ -c $<
	
.cpp.lo: ${LIBTOOL}
	${LIBTOOL} --mode=compile ${CXX} ${LIBTOOLFLAGS}  ${CFLAGS} ${CPPFLAGS} -o $@ -c $<

.cpp.po:
	${CXX} -pg -DPROF ${CFLAGS} ${CPPFLAGS} -o $@ -c $<


.asm.o:
	nasm ${ASFLAGS} $<



clean:
	rm -f libxm7-ui-agar.a libxm7-ui-agar.la *.o *.lo *~


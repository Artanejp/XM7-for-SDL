###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
OPTION          += -DUSE_OPENGL
INSTALL          = install -c 
PREFIX           = /usr/local


VIDEODRVSRC = \
	EmuGrphLib.cpp EmuGrph4096c.cpp  EmuGrph400l.cpp \
	EmuGrph256kc.cpp 

SCALERSRC = \
        EmuGrphScale1x1.cpp \
        EmuGrphScale1x2.cpp EmuGrphScale1x2i.cpp \
        EmuGrphScale2x2.cpp EmuGrphScale2x2i.cpp \
	EmuGrphScale2x4.cpp EmuGrphScale2x4i.cpp \
	EmuGrphScale4x4.cpp EmuGrphScale4x4i.cpp 	

GLSCALERSRC = \
         EmuGLUtils.cpp \
	 EmuAgarGL.cpp


LOCALSRC = \
	$(VIDEODRVSRC) $(GLSCALERSRC) $(SCALERSRC) 

VIDEODRVOBJ = \
	EmuGrphLib.o EmuGrph4096c.o  EmuGrph400l.o \
	EmuGrph256kc.o 

SCALEROBJ = \
        EmuGrphScale1x1.o \
        EmuGrphScale1x2.o EmuGrphScale1x2i.o \
        EmuGrphScale2x2.o EmuGrphScale2x2i.o \
	EmuGrphScale2x4.o EmuGrphScale2x4i.o \
	EmuGrphScale4x4.o EmuGrphScale4x4i.o 	

GLSCALEROBJ = \
         EmuGLUtils.o \
	 EmuAgarGL.o

LOCALOBJ = \
	$(VIDEODRVOBJ) $(SCALEROBJ) $(GLSCALEROBJ) 

.SUFFIXES: .c .cpp .asm .o

SRCS =	${COMMONSRC} ${LOCALSRC}
OBJS =	${COMMONOBJ} ${LOCALOBJ}

LDFLAGS = 
CFLAGS =        -g -O0 -export-dynamic
#CFLAGS =        -O3 -export-dynamic
CFLAGS += -fPIC

CFLAGS +=  -I. -I/usr/local/include 
CFLAGS += -I/usr/local/include/agar
CFLAGS += `pkg-config --cflags sdl`
CFLAGS += `pkg-config --cflags gtk+-x11-2.0` `pkg-config --cflags gdk-pixbuf-2.0`

LIBS += `pkg-config --libs sdl`
LIBS += -lGL -lGLU
#LIBS += -lmemwatch




all:	libemugrph.so

libemugrph.so: ${OBJS}
	g++ -shared ${LDFLAGS} -o libemugrph.so ${OBJS} ${LIBS}


.c.o:
	gcc ${CFLAGS} -o $@ -c $<

.cpp.o:
	g++ ${CFLAGS} -o $@ -c $<

.asm.o:
	nasm ${ASFLAGS} $<

clean:
	rm -f libemugrph.so *.o *~

install: all
	install -c libemugrph.so $(PREFIX)/lib
	install -d $(PREFIX)/include/libemugrph
	install -c -m 0644 *.h $(PREFIX)/include/libemugrph
	ldconfig
	
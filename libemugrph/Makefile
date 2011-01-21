###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
OPTION          += -DUSE_OPENGL
INSTALL          = install -c 
PREFIX           = /usr/local
CC           = gcc
CXX           = g++
LIBTOOL           = libtool
LIB           = emugrph

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

VIDEODRVSHOBJ = \
	EmuGrphLib.lo EmuGrph4096c.lo  EmuGrph400l.lo \
	EmuGrph256kc.lo 

SCALERSHOBJ = \
        EmuGrphScale1x1.lo \
        EmuGrphScale1x2.lo EmuGrphScale1x2i.lo \
        EmuGrphScale2x2.lo EmuGrphScale2x2i.lo \
	EmuGrphScale2x4.lo EmuGrphScale2x4i.lo \
	EmuGrphScale4x4.lo EmuGrphScale4x4i.lo 	

GLSCALERSHOBJ = \
         EmuGLUtils.lo \
	 EmuAgarGL.lo



LOCALOBJ = \
	$(VIDEODRVOBJ) $(SCALEROBJ) $(GLSCALEROBJ) 

LOCALSHOBJ = \
	$(VIDEODRVSHOBJ) $(SCALERSHOBJ) $(GLSCALERSHOBJ) 

SRCS =	${COMMONSRC} ${LOCALSRC}
OBJS =	${COMMONOBJ} ${LOCALOBJ}
SHOBJS =	${COMMONSHOBJ} ${LOCALSHOBJ}

LDFLAGS = 
#CFLAGS =        -g -O0 -export-dynamic
CFLAGS =        -O3 -export-dynamic
#CFLAGS += -fPIC

CFLAGS +=  -I. -I/usr/local/include 
CFLAGS += -I/usr/local/include/agar
CFLAGS += `pkg-config --cflags sdl`
CFLAGS += `pkg-config --cflags gtk+-x11-2.0` `pkg-config --cflags gdk-pixbuf-2.0`
LIBS += -lag_core -lag_gui -lfreetype -lpng -ljpeg

LIBS += `pkg-config --libs sdl`
LIBS += -lGL -lGLU
#LIBS += -lmemwatch


.SUFFIXES: .o .po .lo .c .cc .cpp 

all:	libemugrph.la libemugrph.a

libemugrph.a: ${OBJS}
	${LIBTOOL} --mode=link ${CXX} -o libemugrph.a \
	-rpath ${PREFIX}/lib \
	${LDFLAGS} ${SHOBJS} \
	${LIBS} ${LIB_XOBJS}


libemugrph.la: ${SHOBJS}
	${LIBTOOL}  --mode=link ${CXX} -version-info 0 -rpath ${PREFIX}/lib -o $@ ${SHOBJS} ${LIBS}

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
	rm -f *.so *.o *.lo *.la *.a *~ 
	rm -fR .libs/
	

install: all
	install -c .libs/libemugrph.so.0.0.0 $(PREFIX)/lib
	install -c .libs/libemugrph.a $(PREFIX)/lib
	install -c libemugrph.la $(PREFIX)/lib
	install -d $(PREFIX)/include/libemugrph
	install -c -m 0644 *.h $(PREFIX)/include/libemugrph
	ldconfig
	
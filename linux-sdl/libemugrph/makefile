###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
include ../config.mak
OPTION          += -DUSE_OPENGL


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


.SUFFIXES: .c .cpp .asm .o

SRCS = \
	$(VIDEODRVSRC) $(GLSCALERSRC) $(SCALERSRC) 


CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

all:	Release/libemugrph.a Debug/libemugrph.a 

Debug/libemugrph.a: ${OBJS_DEBUG}
	ar crv $@ ${OBJS_DEBUG}

Release/libemugrph.a: ${OBJS_RELEASE}
	ar crv $@ ${OBJS_RELEASE}


Release/EmuGrphLib.o: EmuGrphLib.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrph4096c.o: EmuGrph4096c.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrph400l.o: EmuGrph400l.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrph256kc.o: EmuGrph256kc.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale1x1.o: EmuGrphScale1x1.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale1x2.o: EmuGrphScale1x2.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale1x2i.o: EmuGrphScale1x2i.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale2x2.o: EmuGrphScale2x2.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale2x2i.o: EmuGrphScale2x2i.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale2x4.o: EmuGrphScale2x4.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale2x4i.o: EmuGrphScale2x4i.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale4x4.o: EmuGrphScale4x4.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGrphScale4x4i.o: EmuGrphScale4x4i.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuGLUtils.o: EmuGLUtils.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/EmuAgarGL.o: EmuAgarGL.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)



Debug/EmuGrphLib.o: EmuGrphLib.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrph4096c.o: EmuGrph4096c.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrph400l.o: EmuGrph400l.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrph256kc.o: EmuGrph256kc.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale1x1.o: EmuGrphScale1x1.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale1x2.o: EmuGrphScale1x2.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale1x2i.o: EmuGrphScale1x2i.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale2x2.o: EmuGrphScale2x2.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale2x2i.o: EmuGrphScale2x2i.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale2x4.o: EmuGrphScale2x4.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale2x4i.o: EmuGrphScale2x4i.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale4x4.o: EmuGrphScale4x4.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGrphScale4x4i.o: EmuGrphScale4x4i.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuGLUtils.o: EmuGLUtils.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/EmuAgarGL.o: EmuAgarGL.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)



.asm.o:
	nasm ${ASFLAGS} $<

clean:
	rm -f *.so *.o *.lo *.la *.a *~ 
	rm -fR .libs/
	rm -f Debug/*.o Debug/*.a Debug/*.so
	rm -f Release/*.o Release/*.a Release/*.so
	
install: all
	install -c .libs/libemugrph.so.0.0.0 $(PREFIX)/lib
	install -c .libs/libemugrph.a $(PREFIX)/lib
	install -c libemugrph.la $(PREFIX)/lib
	install -d $(PREFIX)/include/libemugrph
	install -c -m 0644 *.h $(PREFIX)/include/libemugrph
	ldconfig
	
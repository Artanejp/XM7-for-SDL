###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3
include ../config.mak

	
VIDEODRVSRC = \
	api_draw.cpp \
	draw_thread.cpp \
	api_vram8.cpp \
	api_vram4096.cpp \
	api_vram256k.cpp \
	api_vramvec.c 	

SNDDRVSRC = \
	api_snd2.cpp api_wavwriter.cpp  \
	snd_buffer.cpp \
	SndDrvTmpl.cpp SndDrvOpn.cpp SndDrvWav.cpp SndDrvBeep.cpp SndDrvCMT.cpp

INPUTSRC = \
	api_kbd.cpp api_js.cpp api_mouse.cpp \
	SDLKbdInterface.cpp \
	SDLJoyInterface.cpp \
	KbdInterface.cpp 

UISRC = 

LOCALSRC = \
	sdl_cpuid.c \
	sdl_inifile.c \
        sdl_file.c \
	windows_main.cpp \
	$(VIDEODRVSRC) \
	$(SNDDRVSRC) $(INPUTSRC) \
	$(UISRC)



.SUFFIXES: .c .cpp .asm .o .d

SRCS =	${COMMONSRC} ${LOCALSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))


OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

DEPS = $(patsubst %.c,%.d,$(CSRCS))
DEPS += $(patsubst %.cpp,%.d,$(CXXSRCS))
DEPS_RELEASE = $(addprefix Release/, $(DEPS))
DEPS_DEBUG =	$(addprefix Debug/,  $(DEPS))


LIBS += -L/usr/local/lib

#LIBS += `pkg-config --libs sdl`
LIBS += `sdl-config --libs`
LIBS +=  -lSDL_mixer -lSDL_ttf
#LIBS +=  -lSDL_mixer-1.3 -lSDL_ttf-1.3 

ifdef USE_OPENGL
LIBS += -lGL -lpthread
#LIBS += -lGLU -lpthread libGL.so.1
endif

ifdef USE_OPENCL
LIBS += -lOpenCL
endif

#LIBS += -ljpeg -lpng -lfreetype -lfontconfig

ifdef USE_OPENMP
LIBS += -lgomp
endif

## Uncomment below if you *not* wish static-link agar.
#LIBS += `agar-config --libs`

## Uncomment below if you wish static-link agar.
LIBS += /usr/local/lib/libag_gui.a /usr/local/lib/libag_core.a \
        /usr/local/lib/libag_dev.a \
	-lXinerama -ljpeg -lpng -lfreetype \
	-lfontconfig

LIBS_RELEASE += ../ui-agar/Release/libui-agar.a
LIBS_RELEASE += ../vm/Release/libxm7_vm.a
ifdef WITH_DEBUGGER
LIBS_RELEASE += ../xm7-debugger/Release/libxm7_debug.a
endif
LIBS_RELEASE += ../fmgen/Release/libfmgen007a.a


#LIBS_DEBUG = ../libemugrph/Debug/libemugrph.a
LIBS_DEBUG += ../ui-agar/Debug/libui-agar.a
LIBS_DEBUG += ../vm/Debug/libxm7_vm.a
ifdef WITH_DEBUGGER
LIBS_DEBUG += ../xm7-debugger/Debug/libxm7_debug.a
endif
LIBS_DEBUG += ../fmgen/Debug/libfmgen007a.a

all:	xm7 xm7.debug

xm7: ${OBJS_RELEASE} ${LIBS_RELEASE}
	$(CXX) ${LDFLAGS} -o xm7 $^ ${LIBS} ${LIBS_RELEASE}
	strip xm7

xm7.debug: ${OBJS_DEBUG} ${LIBS_DEBUG}
	$(CXX) -pg -g ${LDFLAGS} -o xm7.debug $^ ${LIBS} ${LIBS_DEBUG}


#Deps
$(OBJS_RELEASE): ../config.mak \


$(OBJS_DEBUG): ../config.mak \


# Rules
Release/%.o: %.c 
	@mkdir -p Release
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/%.o: %.cpp
	@mkdir -p Release
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Debug/%.o: %.c
	@mkdir -p Debug
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)



Debug/%.o: %.cpp
	@mkdir -p Debug
	$(CXX) ${CXXFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)



.asm.o:
	nasm ${ASFLAGS} $<


install: all
	$(INSTALL)  -d $(SHAREDIR)
	$(INSTALL) ./xm7 $(PREFIX)/bin
	$(INSTALL) -m 0644 ./resource/*.ico $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.xml $(SHAREDIR)
#	$(INSTALL) -m 0644 ./ipagui.ttf $(SHAREDIR)

.PHONY: clean


Debug/%.d: %.c
	@mkdir -p Debug/
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS_DEBUG) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Debug\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

Debug/%.d: %.cpp
	@mkdir -p Debug/
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS_DEBUG) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Debug\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

Release/%.d: %.c
	@mkdir -p Release/
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS_RELEASE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Release\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

Release/%.d: %.cpp
	@mkdir -p Release/
	@set -e; rm -f $@; \
	$(CXX) -M $(CXXFLAGS_RELEASE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,Release\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


clean:
	rm -f xm7.debug xm7 *.o *~ Debug/*.o Debug/*.d Release/*.o Release/*.d *.d

.DEPEND:
-include $(OBJS_RELEASE:.o=.d)
-include $(OBJS_DEBUG:.o=.d)


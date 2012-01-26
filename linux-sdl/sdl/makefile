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
	SndDrvTmpl.cpp SndDrvOpn.cpp SndDrvWav.cpp SndDrvBeep.cpp SndDrvCMT.cpp

INPUTSRC = \
	api_kbd.cpp api_js.cpp api_mouse.cpp \
	SDLKbdInterface.cpp \
	SDLJoyInterface.cpp \
	KbdInterface.cpp 

UISRC = 

LOCALSRC = \
	sdl_inifile.c \
        sdl_file.c \
	$(VIDEODRVSRC) \
	$(SNDDRVSRC) $(INPUTSRC) \
	$(UISRC)



.SUFFIXES: .c .cpp .asm .o

SRCS =	${COMMONSRC} ${LOCALSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))


LIBS += -L/usr/local/lib

LIBS += `pkg-config --libs sdl`
LIBS +=  -lSDL_mixer -lSDL_ttf
#LIBS +=  -lSDL_mixer-1.3 -lSDL_ttf-1.3 

LIBS += -lGL -lGLU -lpthread
LIBS += `agar-config --libs`
LIBS += -ljpeg -lpng -lfreetype
LIBS += -lgomp

#LIBS_RELEASE = ../libemugrph/Release/libemugrph.a
LIBS_RELEASE += ../ui-agar/Release/libui-agar.a
LIBS_RELEASE += ../vm/Release/libxm7_vm.a
LIBS_RELEASE += ../xm7-debugger/Release/libxm7_debug.a

#LIBS_DEBUG = ../libemugrph/Debug/libemugrph.a
LIBS_DEBUG += ../ui-agar/Debug/libui-agar.a
LIBS_DEBUG += ../vm/Debug/libxm7_vm.a
LIBS_DEBUG += ../xm7-debugger/Debug/libxm7_debug.a

all:	xm7 xm7.debug

xm7: ${OBJS_RELEASE} ${LIBS_RELEASE}
	$(CXX) ${LDFLAGS} -o xm7 $^ ${LIBS} ${LIBS_RELEASE}
	strip xm7

xm7.debug: ${OBJS_DEBUG} ${LIBS_DEBUG}
	$(CXX) -pg -g ${LDFLAGS} -o xm7.debug $^ ${LIBS} ${LIBS_DEBUG}



Release/sdl_inifile.o: sdl_inifile.c
	$(CC) -O3 ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/sdl_file.o: sdl_file.c
	$(CC) -O3 ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)


Release/api_draw.o: api_draw.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/draw_thread.o: draw_thread.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_scaler.o: api_scaler.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_vram8.o: api_vram8.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_vram4096.o: api_vram4096.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_vram256k.o: api_vram256k.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_vramvec.o: api_vramvec.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)


Release/api_snd2.o: api_snd2.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_wavwriter.o: api_wavwriter.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/SndDrvTmpl.o: SndDrvTmpl.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvOpn.o: SndDrvOpn.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvWav.o: SndDrvWav.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvBeep.o: SndDrvBeep.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvCMT.o: SndDrvCMT.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)


Release/api_kbd.o: api_kbd.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/api_js.o: api_js.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/api_mouse.o: api_mouse.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SDLKbdInterface.o: SDLKbdInterface.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SDLJoyInterface.o: SDLJoyInterface.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/KbdInterface.o: KbdInterface.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)



Debug/sdl_inifile.o: sdl_inifile.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/sdl_file.o: sdl_file.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/api_draw.o: api_draw.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/draw_thread.o: draw_thread.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_scaler.o: api_scaler.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_vram8.o: api_vram8.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_vram4096.o: api_vram4096.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_vram256k.o: api_vram256k.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_vramvec.o: api_vramvec.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)


Debug/api_snd2.o: api_snd2.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)


Debug/api_wavwriter.o: api_wavwriter.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvTmpl.o: SndDrvTmpl.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvOpn.o: SndDrvOpn.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvWav.o: SndDrvWav.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvBeep.o: SndDrvBeep.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvCMT.o: SndDrvCMT.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)


Debug/api_kbd.o: api_kbd.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/api_js.o: api_js.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/api_mouse.o: api_mouse.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SDLKbdInterface.o: SDLKbdInterface.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SDLJoyInterface.o: SDLJoyInterface.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/KbdInterface.o: KbdInterface.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)


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
	rm -f xm7.dbg xm7 *.o *~ Debug/*.o Release/*.o


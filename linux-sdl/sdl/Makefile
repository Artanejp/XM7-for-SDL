##
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3
include ../config.mak
include ../common.mak

	
VIDEODRVSRC = \
	api_draw.cpp \
	draw_thread.cpp \

#	api_vram8.c \
#	api_vram4096.c \
#	api_vram256k.c \
#	api_vramvec.c 	

SNDDRVSRC = \
	api_snd2.cpp api_wavwriter.cpp  \
	snd_buffer.cpp \
	SndDrvTmpl.cpp SndDrvOpn.cpp SndDrvWav.cpp SndDrvBeep.cpp SndDrvCMT.cpp \
	copysoundbuffer.c

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



LIBS_VRAM   = libvram-generic.a
LIBS_VRAM_PATH = vram/generic

.SUFFIXES: .c .cpp .asm .o .d

SRCS =	${COMMONSRC} ${LOCALSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))


OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))
LIBS_VRAM_RELEASE = $(addprefix $(LIBS_VRAM_PATH)/Release/, $(LIBS_VRAM)) 
LIBS_VRAM_DEBUG   = $(addprefix $(LIBS_VRAM_PATH)/Debug/, $(LIBS_VRAM)) 


DEPS = $(patsubst %.c,%.d,$(CSRCS))
DEPS += $(patsubst %.cpp,%.d,$(CXXSRCS))
DEPS_RELEASE = $(addprefix Release/, $(DEPS))
DEPS_DEBUG =	$(addprefix Debug/,  $(DEPS))


all:	$(TARGET_DEBUG) $(TARGET_RELEASE)

$(TARGET_RELEASE): ${OBJS_RELEASE} ${LIBS_RELEASE} ${LIBS_VRAM_RELEASE}
	$(CXX) ${LDFLAGS} -o $(TARGET_RELEASE)  $^ ${LIBS_RELEASE} ${LIBS_VRAM_RELEASE} ${LIBS}
	strip $(TARGET_RELEASE)

$(TARGET_DEBUG): ${OBJS_DEBUG} ${LIBS_DEBUG} ${LIBS_VRAM_DEBUG}
	$(CXX) -pg -g ${LDFLAGS} -o $(TARGET_DEBUG)  $^ ${LIBS_DEBUG} ${LIBS_VRAM_DEBUG} ${LIBS}

$(LIBS_VRAM_RELEASE):
	make -C $(LIBS_VRAM_PATH) Release
	
$(LIBS_VRAM_DEBUG):
	make -C $(LIBS_VRAM_PATH) Debug
	
	

-include ../rules.mak


install: all
	$(INSTALL)  -d $(SHAREDIR)
	$(INSTALL) ./$(TARGET_RELEASE) $(PREFIX)/bin
	$(INSTALL) ./$(TARGET_DEBUG) $(PREFIX)/bin
	$(INSTALL) -m 0644 ./resource/*.ico $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.xml $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.png $(SHAREDIR)
#	$(INSTALL) -m 0644 ./ipagui.ttf $(SHAREDIR)

.PHONY: clean


clean:
	rm -f $(TARGET_DEBUG) $(TARGET_RELEASE) *.o *~ Debug/*.o Debug/*.d Release/*.o Release/*.d
	make -C $(LIBS_VRAM_PATH) clean

.DEPEND:
-include $(OBJS_RELEASE:.o=.d)
-include $(OBJS_DEBUG:.o=.d)


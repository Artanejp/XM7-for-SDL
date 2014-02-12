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

#	copysoundbuffer.c

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



LIBS_VRAM_PATH = vram/generic
LIBS_SOUNDBUFFER_PATH = soundbuffer/generic 


.SUFFIXES: .c .cpp .asm .o .d

SRCS =	${COMMONSRC} ${LOCALSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))


OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

LIBS_VRAM_RELEASE = vram/generic/Release/libvram-generic.a
LIBS_VRAM_DEBUG   = vram/generic/Debug/libvram-generic.a
ifdef BUILD_SSE2
LIBS_VRAM_SSE2 = libvram-sse2.a 
LIBS_VRAM_PATH_SSE2 = vram/sse2
LIBS_VRAM_RELEASE += vram/sse2/Release/libvram-sse2.a
LIBS_VRAM_DEBUG   += vram/sse2/Debug/libvram-sse2.a
LIBS_VRAM_PATH += ${LIBS_VRAM_PATH_SSE2}
endif

LIBS_SOUNDBUFFER_RELEASE = soundbuffer/generic/Release/libsndbuffer-generic.a
LIBS_SOUNDBUFFER_DEBUG   = soundbuffer/generic/Debug/libsndbuffer-generic.a
ifdef BUILD_SSE2
LIBS_SOUNDBUFFER_SSE2 = libsndbuffer-sse2.a 
LIBS_SOUNDBUFFER_PATH_SSE2 = soundbuffer/sse2
LIBS_SOUNDBUFFER_RELEASE += soundbuffer/sse2/Release/libsndbuffer-sse2.a
LIBS_SOUNDBUFFER_DEBUG   += soundbuffer/sse2/Debug/libsndbuffer-sse2.a
LIBS_SOUNDBUFFER_PATH += ${LIBS_SOUNDBUFFER_PATH_SSE2}
endif

DEPS = $(patsubst %.c,%.d,$(CSRCS))
DEPS += $(patsubst %.cpp,%.d,$(CXXSRCS))
DEPS_RELEASE = $(addprefix Release/, $(DEPS))
DEPS_DEBUG =	$(addprefix Debug/,  $(DEPS))

ifdef BUILD_SSE2
CFLAGS += -DUSE_SSE2
endif

all:	$(TARGET_DEBUG) $(TARGET_RELEASE)

$(TARGET_RELEASE): ${OBJS_RELEASE} ${LIBS_RELEASE} ${LIBS_VRAM_RELEASE} ${LIBS_SOUNDBUFFER_RELEASE}
	$(CXX) ${LDFLAGS} -o $(TARGET_RELEASE)  $^ ${LIBS_RELEASE} ${LIBS_VRAM_RELEASE} ${LIBS_SOUNDBUFFER_RELEASE} ${LIBS}
	strip $(TARGET_RELEASE)


$(TARGET_DEBUG): ${OBJS_DEBUG} ${LIBS_DEBUG} ${LIBS_VRAM_DEBUG} ${LIBS_SOUNDBUFFER_DEBUG}
	$(CXX) -pg -g ${LDFLAGS} -o $(TARGET_DEBUG)  $^ ${LIBS_DEBUG} ${LIBS_VRAM_DEBUG} ${LIBS_SOUNDBUFFER_DEBUG} ${LIBS}

$(LIBS_VRAM_DEBUG):
	for dir_idv in $(LIBS_VRAM_PATH) ; do \
	make -C $$dir_idv Debug; \
	done

$(LIBS_SOUNDBUFFER_DEBUG):
	for dir_ids in $(LIBS_SOUNDBUFFER_PATH) ; do \
	make -C $$dir_ids Debug; \
	done

$(LIBS_VRAM_RELEASE):
	for dir_irv in $(LIBS_VRAM_PATH) ; do \
	make -C $$dir_irv Release; \
	done

$(LIBS_SOUNDBUFFER_RELEASE):
	for dir_irs in $(LIBS_SOUNDBUFFER_PATH) ; do \
	make -C $$dir_irs Release; \
	done



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
	rm -f $(TARGET_DEBUG) $(TARGET_RELEASE) *.o *~ Debug/*.o Release/*.o
	for dir_iss in $(LIBS_SOUNDBUFFER_PATH) ; do \
	make -C $$dir_iss clean; \
	done
	for dir_ivv in $(LIBS_VRAM_PATH) ; do \
	make -C $$dir_ivv clean; \
	done

.DEPEND:
-include $(OBJS_RELEASE:.o=.d)
-include $(OBJS_DEBUG:.o=.d)


###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile GTK For SDL]
XM7_VER		= 3
include config.mak

COMMONSRC = \
        aluline.c \
        apalet.c \
        disasm.c \
        display.c \
	dmac.c \
	fdc.c \
	jcard.c \
	jsubsys.c \
	kanji.c \
	keyboard.c \
	maincpu.c \
	mainetc.c \
	mainmem.c \
	midi.c \
	mmr.c \
	mouse.c \
	multipag.c \
	opn.c \
	rtc.c \
	schedule.c \
	subcpu.c \
	subctrl.c \
	submem.c \
	system.c \
	tapelp.c \
	tools.c \
	ttlpalet.c \
	opna.cpp \
	psg.cpp \
	fmgen.cpp \
	juliet.cpp \
	vm_sub.c \
	m6809.c
	
VIDEODRVSRC = \
	api_draw.cpp api_scaler.cpp 

SNDDRVSRC = \
	api_snd2.cpp api_wavwriter.cpp \
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
#LIBS += `pkg-config --libs gtk+-x11-2.0` `pkg-config --libs gdk-pixbuf-2.0`
LIBS +=  \
        -lSDL_mixer -lSDL_ttf

LIBS += -lGL -lGLU -lpthread
LIBS += -lag_core -lag_gui -lag_dev 
#LIBS += -ldl /usr/local/lib/libag_core.a  /usr/local/lib/libag_gui.a 
LIBS += -ljpeg -lpng

LIBS_RELEASE = ../libemugrph/Release/liblibemugrph.a
LIBS_RELEASE += ../xm7-ui-agar/Release/libxm7-ui-agar.a

LIBS_DEBUG = ../libemugrph/Debug/liblibemugrph.a
LIBS_DEBUG += ../xm7-ui-agar/Debug/libxm7-ui-agar.a

all:	xm7 xm7.debug

xm7: ${OBJS_RELEASE} ${LIBS_RELEASE}
	g++ ${LDFLAGS} -o xm7 $^ ${LIBS}
	strip xm7

xm7.debug: ${OBJS_DEBUG} ${LIBS_DEBUG}
	g++ -g ${LDFLAGS} -o xm7.debug $^ ${LIBS}


Release/aluline.o: aluline.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/apalet.o: apalet.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/disasm.o: disasm.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/display.o: display.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/dmac.o: dmac.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/fdc.o: fdc.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/jcard.o: jcard.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/jsubsys.o: jsubsys.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/kanji.o: kanji.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/keyboard.o: keyboard.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/maincpu.o: maincpu.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mainetc.o: mainetc.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mainmem.o: mainmem.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/midi.o: midi.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mmr.o: mmr.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mouse.o: mouse.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/multipag.o: multipag.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/opn.o: opn.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/rtc.o: rtc.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/schedule.o: schedule.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/subcpu.o: subcpu.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/subctrl.o: subctrl.c 
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/submem.o: submem.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/system.o: system.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/tapelp.o: tapelp.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/tools.o: tools.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/ttlpalet.o: ttlpalet.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/vm_sub.o: vm_sub.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/m6809.o: m6809.c m6809ops.h m6809tbl.h
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/sdl_inifile.o: sdl_inifile.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/sdl_file.o: sdl_file.c
	gcc ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)


Release/opna.o: opna.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/psg.o: psg.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/fmgen.o: fmgen.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/juliet.o: juliet.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_draw.o: api_draw.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/api_scaler.o: api_scaler.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_snd2.o: api_snd2.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/api_wavwriter.o: api_wavwriter.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/SndDrvTmpl.o: SndDrvTmpl.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvOpn.o: SndDrvOpn.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvWav.o: SndDrvWav.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvBeep.o: SndDrvBeep.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SndDrvCMT.o: SndDrvCMT.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)


Release/api_kbd.o: api_kbd.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/api_js.o: api_js.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/api_mouse.o: api_mouse.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SDLKbdInterface.o: SDLKbdInterface.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/SDLJoyInterface.o: SDLJoyInterface.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/KbdInterface.o: KbdInterface.cpp
	g++ ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)



Debug/aluline.o: aluline.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/apalet.o: apalet.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/disasm.o: disasm.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/display.o: display.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/dmac.o: dmac.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/fdc.o: fdc.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/jcard.o: jcard.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/jsubsys.o: jsubsys.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/kanji.o: kanji.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/keyboard.o: keyboard.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/maincpu.o: maincpu.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mainetc.o: mainetc.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mainmem.o: mainmem.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/midi.o: midi.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mmr.o: mmr.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mouse.o: mouse.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/multipag.o: multipag.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/opn.o: opn.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/rtc.o: rtc.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/schedule.o: schedule.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/subcpu.o: subcpu.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/subctrl.o: subctrl.c 
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)


Debug/submem.o: submem.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/system.o: system.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/tapelp.o: tapelp.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/tools.o: tools.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/ttlpalet.o: ttlpalet.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/vm_sub.o: vm_sub.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/m6809.o: m6809.c m6809ops.h m6809tbl.h
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/sdl_inifile.o: sdl_inifile.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/sdl_file.o: sdl_file.c
	gcc ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)


Debug/opna.o: opna.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/psg.o: psg.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/fmgen.o: fmgen.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/juliet.o: juliet.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_draw.o: api_draw.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/api_scaler.o: api_scaler.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_snd2.o: api_snd2.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/api_wavwriter.o: api_wavwriter.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvTmpl.o: SndDrvTmpl.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvOpn.o: SndDrvOpn.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvWav.o: SndDrvWav.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvBeep.o: SndDrvBeep.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SndDrvCMT.o: SndDrvCMT.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)


Debug/api_kbd.o: api_kbd.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/api_js.o: api_js.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/api_mouse.o: api_mouse.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SDLKbdInterface.o: SDLKbdInterface.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/SDLJoyInterface.o: SDLJoyInterface.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/KbdInterface.o: KbdInterface.cpp
	g++ ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)


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


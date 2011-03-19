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
	
#VIDEODRVSRC = \
#        sdl_draw.c \
#        sdl_draw_640.c sdl_draw_400l.c sdl_draw_4096.c sdl_draw_256k.c

VIDEODRVSRC = \
	api_draw.cpp api_scaler.cpp \
	agar_draw.cpp agar_gldraw.cpp

SNDDRVSRC = \
	api_snd.cpp \
	SndDrvTmpl.cpp SndDrvOpn.cpp SndDrvWav.cpp SndDrvBeep.cpp SndDrvCMT.cpp

INPUTSRC = \
	api_kbd.cpp api_js.cpp api_mouse.cpp \
	AgarKbdInterface.cpp \
	SDLKbdInterface.cpp \
	SDLJoyInterface.cpp \
	KbdInterface.cpp \
	agar_ui_vkey.cpp


#	GtkKbdInterface.cpp \
#	KbdInterface.cpp 

UISRC = \
	agar_guicore.cpp \
	agar_toolbox.cpp \
	agar_gui_menu.cpp \
	agar_menu_disk.cpp \
	agar_menu_tape.cpp \
	agar_menu_tool.cpp \
	agar_menu_about.cpp \
	agar_gui_pref.cpp 
	
#	sdl_gtkdlg.c \
#        gtk_prop2.c gtk_propkeyboard.c \
#        gtk_cfgdlg.c gtk_main.c gtk_toolbox.c

#LOCALSRC = \
#	sdl_cfg.c  sdl_inifile.c \
#        sdl_cmd.c sdl_sch.c sdl_file.c \
#	sdl_main.c \
#	sdl_bar.c \

LOCALSRC = \
	agar_cfg.cpp  sdl_inifile.c \
        agar_cmd.cpp agar_sch.cpp sdl_file.c \
	agar_main.cpp agar_sdl.cpp \
	agar_osd.cpp \
	util_ringbuffer.c \
	$(VIDEODRVSRC) \
	$(SNDDRVSRC) $(INPUTSRC) \
	$(UISRC)

COMMONOBJ = \
        aluline.o \
        apalet.o \
        disasm.o \
        display.o \
        dmac.o \
	fdc.o \
	jcard.o \
	jsubsys.o \
	kanji.o \
	keyboard.o \
	maincpu.o \
	mainetc.o \
	mainmem.o \
	midi.o \
	mmr.o \
	mouse.o \
	multipag.o \
	opn.o \
	rtc.o \
	schedule.o \
	subcpu.o \
	subctrl.o \
	submem.o \
	system.o \
	tapelp.o \
	tools.o \
	ttlpalet.o \
	opna.o \
	psg.o \
	fmgen.o \
	juliet.o \
	vm_sub.o \
	m6809.o

#	cpu_x86.o \
#	xw_sub.o


#VIDEODRVOBJ = \
#        sdl_draw_640.o sdl_draw_400l.o sdl_draw_4096.o sdl_draw_256k.o

VIDEODRVOBJ = \
	api_draw.o api_scaler.o \
	agar_draw.o agar_gldraw.o

SNDDRVOBJ = \
	SndDrvTmpl.o SndDrvOpn.o SndDrvWav.o SndDrvBeep.o SndDrvCMT.o

INPUTOBJ = \
	SDLJoyInterface.o \
	AgarKbdInterface.o \
	SDLKbdInterface.o \
	KbdInterface.o \
	agar_ui_vkey.o
	
#	SDLKbdInterface.o \
#	GtkKbdInterface.o \
#	KbdInterface.o \

UIOBJ = \
	agar_guicore.o \
	agar_toolbox.o \
	agar_gui_menu.o \
	agar_menu_disk.o \
	agar_menu_tape.o \
	agar_menu_tool.o \
	agar_menu_about.o \
	agar_gui_pref.o 

#	agar_cfg.o  sdl_inifile.o \
#        agar_cmd.o agar_sch.o sdl_file.o \
#	agar_main.o agar_sdl.o \
#	sdl_bar.o \
	
#	sdl_gtkdlg.o \
#        gtk_prop2.o gtk_propkeyboard.o \
#        gtk_cfgdlg.o gtk_main.o gtk_toolbox.o

LOCALOBJ = \
	agar_cfg.o  sdl_inifile.o \
        agar_cmd.o agar_sch.o sdl_file.o \
	agar_main.o agar_sdl.o \
	agar_osd.o \
	util_ringbuffer.o \
	api_kbd.o api_js.o api_mouse.o \
	api_snd.o \
	$(VIDEODRVOBJ)  \
	$(SNDDRVOBJ) $(INPUTOBJ) $(UIOBJ)

.SUFFIXES: .c .cpp .asm .o

SRCS =	${COMMONSRC} ${LOCALSRC}
OBJS =	${COMMONOBJ} ${LOCALOBJ}

LIBS += -L/usr/local/lib
LIBS += -lemugrph
LIBS += `pkg-config --libs sdl`
LIBS += `pkg-config --libs gtk+-x11-2.0` `pkg-config --libs gdk-pixbuf-2.0`
LIBS +=  \
        -lSDL_mixer -lSDL_ttf

LIBS += -lGL -lGLU
LIBS += -lag_core -lag_gui -lag_dev 
#LIBS += -ldl /usr/local/lib/libag_core.a  /usr/local/lib/libag_gui.a 
LIBS += -ljpeg -lpng
#LIBS += -lmemwatch




all:	xm7 xm7.dbg

xm7: ${OBJS}
	g++ ${LDFLAGS} -o xm7 ${OBJS} ${LIBS}
	strip xm7

xm7.dbg: ${OBJS}
	g++ ${LDFLAGS} -o xm7.dbg ${OBJS} ${LIBS}


.c.o:
	gcc ${CFLAGS} -o $@ -c $<

.cpp.o:
	g++ ${CFLAGS} -o $@ -c $<

.asm.o:
	nasm ${ASFLAGS} $<

clean:
	rm -f xm7.dbg xm7 *.o *~

install: all
	$(INSTALL)  -d $(SHAREDIR)
	$(INSTALL) ./xm7 $(PREFIX)/bin
	$(INSTALL) -m 0644 *.ui $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.ico $(SHAREDIR)
	$(INSTALL) -m 0644 ./resource/*.xml $(SHAREDIR)
#	$(INSTALL) -m 0644 ./ipagui.ttf $(SHAREDIR)
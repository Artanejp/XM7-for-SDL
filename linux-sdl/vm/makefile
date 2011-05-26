###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile for SDL / vm]
XM7_VER		= 3
include ../config.mak

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

.SUFFIXES: .c .cpp .asm .o

SRCS =	${COMMONSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

all:	Release/libxm7_vm.a Debug/libxm7_vm.a 

Debug/libxm7_vm.a: ${OBJS_DEBUG}
	ar crv $@ ${OBJS_DEBUG}

Release/libxm7_vm.a: ${OBJS_RELEASE}
	ar crv $@ ${OBJS_RELEASE}

Release/aluline.o: aluline.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/apalet.o: apalet.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/disasm.o: disasm.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/display.o: display.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/dmac.o: dmac.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/fdc.o: fdc.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/jcard.o: jcard.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/jsubsys.o: jsubsys.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/kanji.o: kanji.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/keyboard.o: keyboard.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/maincpu.o: maincpu.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mainetc.o: mainetc.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mainmem.o: mainmem.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/midi.o: midi.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mmr.o: mmr.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/mouse.o: mouse.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/multipag.o: multipag.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/opn.o: opn.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/rtc.o: rtc.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/schedule.o: schedule.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/subcpu.o: subcpu.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/subctrl.o: subctrl.c 
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/submem.o: submem.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/system.o: system.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/tapelp.o: tapelp.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/tools.o: tools.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/ttlpalet.o: ttlpalet.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)
	
Release/vm_sub.o: vm_sub.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/m6809.o: m6809.c m6809ops.h m6809tbl.h
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/opna.o: opna.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/psg.o: psg.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/fmgen.o: fmgen.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/juliet.o: juliet.cpp
	$(CXX) ${CFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)


Debug/aluline.o: aluline.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/apalet.o: apalet.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/disasm.o: disasm.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/display.o: display.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/dmac.o: dmac.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/fdc.o: fdc.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/jcard.o: jcard.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/jsubsys.o: jsubsys.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/kanji.o: kanji.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/keyboard.o: keyboard.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/maincpu.o: maincpu.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mainetc.o: mainetc.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mainmem.o: mainmem.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/midi.o: midi.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mmr.o: mmr.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/mouse.o: mouse.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/multipag.o: multipag.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/opn.o: opn.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/rtc.o: rtc.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/schedule.o: schedule.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/subcpu.o: subcpu.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/subctrl.o: subctrl.c 
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)


Debug/submem.o: submem.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/system.o: system.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/tapelp.o: tapelp.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/tools.o: tools.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/ttlpalet.o: ttlpalet.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)
	
Debug/vm_sub.o: vm_sub.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/m6809.o: m6809.c m6809ops.h m6809tbl.h
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/opna.o: opna.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/psg.o: psg.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/fmgen.o: fmgen.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/juliet.o: juliet.cpp
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

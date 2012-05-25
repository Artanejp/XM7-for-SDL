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
	juliet.cpp \
	vm_sub.c \
	m6809.c

#	opna.cpp \
#	psg.cpp \
#	fmgen.cpp \

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
	$(AR) crv $@ ${OBJS_DEBUG}

Release/libxm7_vm.a: ${OBJS_RELEASE}
	$(AR) crv $@ ${OBJS_RELEASE}

Release/%.o: %.c
	$(CC) ${CFLAGS_RELEASE} -o $@ -c $(filter %.c, $<)

Release/%.o: %.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Debug/%.o: %.c
	$(CC) ${CFLAGS_DEBUG} -o $@ -c $(filter %.c, $<)

Debug/%.o: %.cpp
	$(CXX) ${CXXFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)


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
	rm -f xm7.dbg xm7 *.o *~ 
	rm -f Debug/*.o Release/*.o
	rm -f Release/*.so Debug/*.so
	rm -f Debug/*.lo Release/*.lo
	rm -f Release/*.a Debug/*.a

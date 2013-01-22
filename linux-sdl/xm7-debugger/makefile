###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile for SDL / vm]
XM7_VER		= 3
include ../config.mak
include ../common.mak

COMMONSRC = \
	dbg_breakpoint.cpp \
	dbg_disasm09.cpp \
	dbg_memdump.cpp \
	memaccess.cpp
	
.SUFFIXES: .c .cpp .asm .o

SRCS =	${COMMONSRC}
CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

all:	Release/libxm7_debug.a Debug/libxm7_debug.a 

Debug/libxm7_debug.a: ${OBJS_DEBUG}
	$(AR) crv $@ ${OBJS_DEBUG}
	ranlib $@

Release/libxm7_debug.a: ${OBJS_RELEASE}
	$(AR) crv $@ ${OBJS_RELEASE}
	ranlib $@

-include ../rules.mak

.PHONY: clean
clean:
	rm -f xm7.dbg xm7 *.o *~ 
	rm -f Debug/*.o Release/*.o
	rm -f Release/*.so Debug/*.so
	rm -f Debug/*.lo Release/*.lo
	rm -f Release/*.a Debug/*.a
	rm -f Release/*.d Debug/*.d

.DEPEND:
-include $(OBJS_RELEASE:.o=.d)
-include $(OBJS_DEBUG:.o=.d)

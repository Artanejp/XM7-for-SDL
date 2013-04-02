###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile for SDL / vm]
XM7_VER		= 3
include ../../../config.mak
include ../../../common.mak
DPATH = $(realpath ../../..)

COMMONSRC = \
	copysoundbuffer.c 
	

.SUFFIXES: .c .cpp .asm .o

SRCS =	$(COMMONSRC)

CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))
CLSRCS = $(filter %.cl, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS += $(patsubst %.cl,%.o,$(CLSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS))

CFLAGS += -I../.. -I../../../vm -I../../../ui-agar -I../../../fmgen 


all:	Release/libsndbuffer-sse2.a Debug/libsndbuffer-sse2.a 

Debug: Debug/libsndbuffer-sse2.a

Release: Release/libsndbuffer-sse2.a


Debug/libsndbuffer-sse2.a: ${OBJS_DEBUG}
	$(AR) crv $@ ${OBJS_DEBUG}
	ranlib $@

Release/libsndbuffer-sse2.a: ${OBJS_RELEASE}
	$(AR) crv $@ ${OBJS_RELEASE}
	ranlib $@

-include ../../../rules.mak

.PHONY: clean
clean:
	rm -f Debug/*.o Release/*.o
	rm -f Release/*.so Debug/*.so
	rm -f Debug/*.lo Release/*.lo
	rm -f Release/*.a Debug/*.a
	rm -f Release/*.d Debug/*.d
	rm -f *~

.DEPEND:
-include $(OBJS_RELEASE:.o=.d)
-include $(OBJS_DEBUG:.o=.d)

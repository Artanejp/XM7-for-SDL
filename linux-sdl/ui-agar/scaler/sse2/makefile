###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile for ui-agar / scaler / sse2]
XM7_VER		= 3
include ../../../config.mak
include ../../../common.mak
DPATH = $(realpath ../../..)

COMMONSRC = \
	scaler_x2_sse2.c \
	scaler_x4_sse2.c

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

CFLAGS += -I../../../sdl -I../../../vm -I../../../ui-agar -I../../../fmgen 
CFLAGS += -msse2 -msse -mmmx

all:	Release/libscaler-sse2.a Debug/libscaler-sse2.a 

Debug: Debug/libscaler-sse2.a

Release: Release/libscaler-sse2.a


Debug/libscaler-sse2.a: ${OBJS_DEBUG}
	$(AR) crv $@ ${OBJS_DEBUG}
	ranlib $@

Release/libscaler-sse2.a: ${OBJS_RELEASE}
	$(AR) crv $@ ${OBJS_RELEASE}
	ranlib $@

-include ../../../rules.mak

.PHONY: clean
clean:
	rm -f Debug/*.o Release/*.o
	rm -f Release/*.so Debug/*.so
	rm -f Debug/*.lo Release/*.lo
	rm -f Release/*.a Debug/*.a
	rm -f *~

.DEPEND:
-include $(OBJS_RELEASE:.o=.d)
-include $(OBJS_DEBUG:.o=.d)

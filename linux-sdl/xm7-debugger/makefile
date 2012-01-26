###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2004 GIMONS
###	Copyright (C) 2010 K.Ohta
###	[ Makefile for SDL / vm]
XM7_VER		= 3
include ../config.mak

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
	ar crv $@ ${OBJS_DEBUG}

Release/libxm7_debug.a: ${OBJS_RELEASE}
	ar crv $@ ${OBJS_RELEASE}

	
Debug/dbg_breakpoint.o: dbg_breakpoint.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/dbg_disasm09.o: dbg_disasm09.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)
	
Debug/dbg_memdump.o: dbg_memdump.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/dbg_fonts.o: dbg_fonts.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Debug/memaccess.o: memaccess.cpp
	$(CXX) ${CFLAGS_DEBUG} -o $@ -c $(filter %.cpp, $<)

Release/dbg_breakpoint.o: dbg_breakpoint.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/dbg_disasm09.o: dbg_disasm09.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)
	
Release/dbg_memdump.o: dbg_memdump.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/dbg_fonts.o: dbg_fonts.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

Release/memaccess.o: memaccess.cpp
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(filter %.cpp, $<)

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

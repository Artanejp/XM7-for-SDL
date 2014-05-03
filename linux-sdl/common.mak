###
###	FM-7 EMULATOR "XM7"
###
###	Copyright (C) 2013 K.Ohta
###	[ makefile; common direction]


################## Programs #################################
ifdef CROSS_BUILD
CC = $(CROSS_TARGET)-gcc
CXX = $(CROSS_TARGET)-g++
AR = $(CROSS_TARGET)-ar
else
ifndef CC
CC = gcc
endif
ifndef CXX
CXX = g++
endif
ifndef AR
AR = ar
endif
endif
INSTALL          = install -c 

################### VARIABLES ################################
#XM7_VER		= 3
#OPTION		= -DMOUSE -DMR2 -DFDDSND -DMEMWATCH
OPTION		= -DMOUSE -DMR2 -DFDDSND
#OPTION		= -DMOUSE -DMR2
OPTION		+= -DUSE_AGAR
OPTION          += -D_XWIN -DNDEBUG
#OPTION         += -DFDC_DEBUG

################# Build Flags ################################

ifndef PREFIX
 ifeq ($(OS),Windows)
  PREFIX         = /mingw
 endif
else
  PREFIX         = /usr/local
endif

ifeq ($(OS),Windows)
# Windows
SHAREDIR         = ./xm7/
FONTPATH           = :.:./.xm7/:./xm7/:
CONFPATH	= \"./xm7/\"
OPTION          += -m32 -mwindows
OPTION		+= -D_WINDOWS -D_WIN32
TARGET_DEBUG    = xm7.debug.exe
TARGET_RELEASE  = xm7.exe
else
# Linux etc...
SHAREDIR         = $(PREFIX)/share/xm7/
FONTPATH          = /usr/local/share/xm7/F-Font:/usr/share/fonts:/usr/share/fonts/truetype/ipafont:~/.xm7:.:
TARGET_DEBUG    = xm7.debug
TARGET_RELEASE  = xm7
endif

ifndef BIG_ENDIAN
OPTION		+= -D_M_IX86
endif

ifdef USE_OPENGL
OPTION          += -DUSE_OPENGL
endif


CFLAGS += -DXM7_VER=$(XM7_VER) $(OPTION)  -DUIDIR=\"$(SHAREDIR)\" -DRSSDIR=\"$(SHAREDIR)\"
CFLAGS += -DUI_FONT=\"$(UI_FONT)\" -DFUNC_FONT=\"$(FUNC_FONT)\" -DSTAT_FONT=\"$(STAT_FONT)\" -DVFD_FONT=\"$(VFD_FONT)\" -DCMT_FONT=\"$(CMT_FONT)\"
CFLAGS += -DFONTPATH=\"$(FONTPATH)\"
CFLAGS += -DUI_PT=$(UI_PT) -DSTAT_PT=$(STAT_PT)

ifeq ($(OS),Windows)
# OpenCL is disabled in windows...

else

 ifdef USE_OPENCL
  CFLAGS += -D_USE_OPENCL
 endif
endif

ifdef WITH_DEBUGGER

CFLAGS += -D_WITH_DEBUGGER

endif

CFLAGS += -I. 
CFLAGS += -I../ui-agar/ -I../sdl/ -I../vm/ -I../xm7-debugger/ -I../fmgen/ -I../

ifdef CROSS_BUILD

CFLAGS += -I/usr/$(CROSS_TARGET)/include -I$(CROSS_PREFIX)/include -I/usr/$(CROSS_TARGET)/include
 
 ifeq ($(OS),Windows)
# Windows
CFLAGS += -D_REENTRANT
CFLAGS += `$(CROSS_PREFIX)/bin/sdl-config --cflags`
CFLAGS += `$(CROSS_PREFIX)/bin/agar-config --cflags`
 
 else

# Linux etc...
CFLAGS += -pthread -D_REENTRANT
CFLAGS += `$(CROSS_PREFIX)/bin/sdl-config --cflags`
CFLAGS += `$(CROSS_PREFIX)/bin/agar-config --cflags`
 
 endif

else # NOT CROSS

 ifeq ($(OS),Windows)
# Windows
CFLAGS += -D_REENTRANT
CFLAGS += `sdl-config --cflags`
CFLAGS += `agar-config --cflags`

 else
# Linux etc...
CFLAGS += -pthread -D_REENTRANT
CFLAGS += `sdl-config --cflags`
CFLAGS += `agar-config --cflags`
 endif

endif


# OpenMP
ifdef USE_OPENMP
CFLAGS += -fopenmp
endif

CFLAGS += -flax-vector-conversions
CFLAGS_DEBUG = -pg -g -O0
CFLAGS_DEBUG += $(CFLAGS)

CFLAGS_RELEASE = $(CFLAGS)


CXXFLAGS = -fpermissive 
CXXFLAGS += $(CFLAGS) 

CXXFLAGS_DEBUG = -pg -g -O0
CXXFLAGS_DEBUG += $(CXXFLAGS)

CXXFLAGS_RELEASE =  $(CXXFLAGS)
#CXXFLAGS_RELEASE += -O3

ifeq ($(OS),Windows)
CFLAGS_RELEASE +=  -O1
#CFLAGS_RELEASE += -fprefetch-loop-arrays -fbranch-probabilities

CXXFLAGS_RELEASE +=  -O1
#CXXFLAGS_RELEASE += -fprefetch-loop-arrays -fbranch-probabilities

else

#CFLAGS_RELEASE += -pthread
CFLAGS_RELEASE +=  -O3 -ftree-vectorize
#CFLAGS_RELEASE +=  -O3
CFLAGS_RELEASE +=  -floop-block -fprefetch-loop-arrays -fbranch-probabilities

#CXXFLAGS_RELEASE += -pthread
CXXFLAGS_RELEASE +=  -O3 -ftree-vectorize
#CXXFLAGS_RELEASE +=  -O3
CXXFLAGS_RELEASE += -fprefetch-loop-arrays -fbranch-probabilities

endif

ifeq ($(ARCH),x86_64)
ARCH_FLAGS += -D_X86_64
endif

ifeq ($(ARCH),i386)
ARCH_FLAGS += -D_ia32
endif

ifeq ($(ARCH),i486)
ARCH_FLAGS += -D_ia32
endif

ifeq ($(ARCH),i586)
ARCH_FLAGS += -D_ia32
endif

ifeq ($(ARCH),i686)
ARCH_FLAGS += -D_ia32
endif

ifeq ($(ARCH),ia32)
ARCH_FLAGS += -D_ia32
endif


# Architecture Depend Flag

CFLAGS_RELEASE += $(ARCH_FLAGS)
#CFLAGS_RELEASE += -minline-all-stringops

CXXFLAGS_RELEASE += $(ARCH_FLAGS)
#CXXFLAGS_RELEASE += -minline-all-stringops 

# Faster below
CFLAGS_DEBUG += $(ARCH_FLAGS)
CXXFLAGS_DEBUG += $(ARCH_FLAGS)


ASFLAGS =	-DXM7_VER=$(XM7_VER) -f elf -d _XWIN


##################### Linker Flags #####################
## Uncomment below if you *not* wish static-link agar.
#LIBS = 
#LIBS += `$(PREFIX)/bin/agar-config --libs` -lwinmm
## Uncomment below if you wish static-link agar.

ifdef CROSS_BUILD

#LIBS += $(CROSS_PREFIX)/lib/libag_gui.a \
#	$(CROSS_PREFIX)/lib/libag_core.a \
#	$(CROSS_PREFIX)/lib/libag_dev.a \
#	-lpng -lfreetype \
#	-lpthread \
#	-lfontconfig

LIBS += `$(CROSS_PREFIX)/bin/agar-config --libs`
LIBS += `$(CROSS_PREFIX)/bin/sdl-config --libs`

else

LIBS += `agar-config --libs`
LIBS += `sdl-config --libs`

endif



ifeq ($(OS),Windows)

#LDFLAGS = -static-libgcc -static-libstdc++ -mwindows
 ifdef CROSS_BUILD

LDFLAGS += -mwindows 
LDFLAGS += -L/usr/$(CROSS_TARGET)/lib -L$(CROSS_PREFIX)/lib

 else

LDFLAGS += -pthread -mwindows

 endif

LIBS +=   -lSDL_mixer 

 ifdef USE_OPENGL

   LIBS += -lopengl32 
   
 endif

 ifdef USE_OPENCL
   LIBS += -lOpenCL
 endif


LIBS += -lintl -liconv -lcharset
LIBS += -lpng -lfreetype -lwinmm

 ifdef USE_OPENMP
 
  LIBS += -lgomp
 
 endif

LIBS += -lpthread -lz \
#	-lwinmm 
	
else
# Linux
#Test
LDFLAGS = -lgc

 ifdef CROSS_BUILD
   LDFLAGS += -L/usr/$(CROSS_TARGET)/lib -L$(PREFIX)/lib
 endif


LIBS += -L/usr/local/lib
LIBS += `sdl-config --libs`
LIBS += `agar-config --libs`

#LIBS +=  -lSDL_mixer

 ifdef USE_OPENGL
  LIBS += -lGL -lpthread
  #LIBS += -lGLU -lpthread libGL.so.1
 endif

 ifdef USE_OPENCL
   LIBS += -lOpenCL
 endif

 ifdef USE_OPENMP
   LIBS += -lgomp
 endif

#LIBS += -ljpeg -lpng -lfreetype -lfontconfig

## Uncomment below if you *not* wish static-link agar.

## Uncomment below if you wish static-link agar.
#LIBS += /usr/local/lib/libag_gui.a /usr/local/lib/libag_core.a \
#        /usr/local/lib/libag_dev.a \
#	-lXinerama -ljpeg -lpng -lfreetype \
#	-lfontconfig


endif

LIBS_RELEASE = ../ui-agar/scaler/generic/Release/libscaler-generic.a
# SIMD Dependented
ifdef BUILD_SSE2
LIBS_RELEASE += ../ui-agar/scaler/sse2/Release/libscaler-sse2.a
endif
LIBS_RELEASE += ../ui-agar/Release/libui-agar.a
LIBS_RELEASE += ../vm/Release/libxm7_vm.a
LIBS_RELEASE += ../fmgen/Release/libfmgen007a.a


LIBS_DEBUG = ../ui-agar/scaler/generic/Debug/libscaler-generic.a
LIBS_DEBUG += ../ui-agar/Debug/libui-agar.a
LIBS_DEBUG += ../vm/Debug/libxm7_vm.a
LIBS_DEBUG += ../fmgen/Debug/libfmgen007a.a
ifdef BUILD_SSE2
LIBS_DEBUG += ../ui-agar/scaler/sse2/Debug/libscaler-sse2.a
endif


ifdef WITH_DEBUGGER
LIBS_RELEASE += ../xm7-debugger/Release/libxm7_debug.a
LIBS_DEBUG += ../xm7-debugger/Debug/libxm7_debug.a
endif

# EOF

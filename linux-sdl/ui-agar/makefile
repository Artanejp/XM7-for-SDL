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
	agar_cfg.cpp \
	agar_cmd.cpp \
	agar_draw.cpp \
	agar_guicore.cpp \
	agar_osd.cpp \
	agar_osd_led.cpp \
	agar_osd_stat.cpp \
	agar_osd_tape.cpp \
	agar_osd_vfd.cpp \
	agar_sch.cpp \
	agar_sdl.cpp \
	agar_sdlscaler.cpp \
	agar_sdlview.c \
	agar_svram.cpp \
	agar_vramutil.cpp \
	agar_main.cpp \
	agar_logger.cpp \
	AgarKbdInterface.cpp

GUISRC = \
	agar_gui_menu.cpp \
	agar_gui_pref.cpp \
	agar_gui_pref_cl.cpp \
	agar_menu_about.cpp \
	agar_menu_debug.cpp \
	agar_menu_disk.cpp \
	agar_menu_tape.cpp \
	agar_menu_tool.cpp \
	agar_ui_vkey.cpp \
	agar_toolbox.cpp \
	agar_mouse.cpp \

ifdef WITH_DEBUGGER
DEBUGSRC = \
	agar_surfaceconsole.cpp \
	agar_dbg_disasm.cpp \
	agar_dbg_fdcdump.cpp \
	agar_dbg_memdump.cpp \
	agar_dbg_mmr.cpp \
	agar_dbg_regdump.cpp \

else
DEBUGSRC = 
endif


GLSRC = \
	agar_gldraw2.cpp \
	agar_glutil.cpp 

CLSRC = cl_render.cl \
	agar_glcl.cpp


SCALER_LIBS_DEBUG   = scaler/generic/Debug/libscaler-generic.a
SCALER_LIBS_RELEASE = scaler/generic/Release/libscaler-generic.a

ifdef BUILD_SSE2
CFLAGS += -DUSE_SSE2
SCALER_LIBS_DEBUG += scaler/sse2/Debug/libscaler-sse2.a 
SCALER_LIBS_RELEASE += scaler/sse2/Release/libscaler-sse2.a
endif

.SUFFIXES: .cl .c .cpp .asm .o

SRCS =	$(COMMONSRC) $(GUISRC) 

ifdef USE_OPENGL
SRCS += $(GLSRC)
endif
ifdef USE_OPENCL
SRCS += $(CLSRC)
endif
ifdef WITH_DEBUGGER
SRCS += $(DEBUGSRC) 
endif

CSRCS =	$(filter %.c, $(SRCS))
CXXSRCS = $(filter %.cpp, $(SRCS))
CLSRCS = $(filter %.cl, $(SRCS))

OBJS = $(patsubst %.c,%.o,$(CSRCS))
OBJS += $(patsubst %.cpp,%.o,$(CXXSRCS))
OBJS += $(patsubst %.cl,%.o,$(CLSRCS))
OBJS_RELEASE = $(addprefix Release/, $(OBJS))
OBJS_DEBUG =	$(addprefix Debug/,  $(OBJS)) 

all: Debug Release

Release: Release/libui-agar.a $(SCALER_LIBS_RELEASE)  

Debug: Debug/libui-agar.a $(SCALER_LIBS_DEBUG)  

scaler/generic/Debug/libscaler-generic.a:
	+make -C scaler/generic Debug

scaler/generic/Release/libscaler-generic.a:
	+make -C scaler/generic Release

ifdef BUILD_SSE2
scaler/sse2/Debug/libscaler-sse2.a:
	+make -C scaler/sse2 Debug

scaler/sse2/Release/libscaler-sse2.a:
	+make -C scaler/sse2 Release

endif

Debug/libui-agar.a: ${OBJS_DEBUG}
	$(AR) crv $@ ${OBJS_DEBUG}
	ranlib $@

Release/libui-agar.a: ${OBJS_RELEASE}
	$(AR) crv $@ ${OBJS_RELEASE}
	ranlib $@

Release/%.o: %.cl
	mkdir -p Release
	gawk -f cl2cpp.awk -v VARNAME=$(patsubst %.cl,%,$<) $(filter %.cl, $<) > $(patsubst %.cl,%.cpp,$<)
	$(CXX) ${CXXFLAGS_RELEASE} -o $@ -c $(patsubst %.cl,%.cpp,$<)
	rm $(patsubst %.cl,%.cpp,$<)

Debug/%.o: %.cl
	mkdir -p Debug
	gawk -f cl2cpp.awk -v VARNAME=$(patsubst %.cl,%,$<) $(filter %.cl, $<) > $(patsubst %.cl,%.cpp,$<)
	$(CXX) ${CXXFLAGS_DEBUG} -o $@ -c $(patsubst %.cl,%.cpp,$<)
	rm $(patsubst %.cl,%.cpp,$<)

-include ../rules.mak

.PHONY: clean
clean:
	+rm -f Debug/*.o Release/*.o
	+rm -f Release/*.so Debug/*.so
	+rm -f Debug/*.lo Release/*.lo
	+rm -f Release/*.a Debug/*.a
	+rm -f *~
	+make -C scaler/generic clean
ifdef BUILD_SSE2
	+make -C scaler/sse2 clean
endif
#	+rm -f Release/*.d Debug/*.d

.DEPEND:
-include $(OBJS_RELEASE:.o=.d)
-include $(OBJS_DEBUG:.o=.d)

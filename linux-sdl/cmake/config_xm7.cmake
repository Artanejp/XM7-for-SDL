# Set configuration for building XM7/SDL.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

include(CheckFunctionExists)

set(LOCAL_LIBS      
		     xm7_ui_agar
                     xm7_vm
		     xm7_fmgen
		     xm7_vram-generic
		     xm7_scaler-generic
		     xm7_soundbuffer-generic
		     )

add_definitions(-DUSE_AGAR)

# Build Flags
add_definitions(-DXM7_VER=${XM7_VERSION})

add_definitions(-DFDDSND)

add_definitions(-DMOUSE)

add_definitions(-DMR2 -D_XWIN -DNDEBUG -D_M_IX86)

check_function_exists("nanosleep" HAVE_NANOSLEEP)
if(NOT HAVE_NANOSLEEP)
  check_library_exists("rt" "nanosleep" "" LIB_RT_HAS_NANOSLEEP)
endif(NOT HAVE_NANOSLEEP)

if(HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)
  add_definitions(-DHAVE_NANOSLEEP)
endif(HAVE_NANOSLEEP OR LIB_RT_HAS_NANOSLEEP)

find_package(OpenGL)

if(USE_OPENCL)
 if(OPENGL_FOUND)
   find_package(OpenCL)
   if(OPENCL_FOUND)
    include_directories(${OPENCL_INCLUDE_DIRS})
    add_definitions(-D_USE_OPENCL -DUSE_OPENCL)
    set(OPENCL_LIBRARY ${OPENCL_LIBRARIES})
    set(USE_OPENGL ON)
   endif()
 endif()
endif()

if(USE_OPENGL)
 if(OPENGL_FOUND)
   include_directories(${OPENGL_INCLUDE_PATH})
   add_definitions(-D_USE_OPENGL -DUSE_OPENGL)
 else()
   set(USE_OPENGL OFF)
   set(USE_OPENCL OFF)
   unset(OPENCL_LIBRARY)
 endif()
endif()



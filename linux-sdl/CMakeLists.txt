# Build XM7/SDL, Agar.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.


cmake_minimum_required (VERSION 2.6)
cmake_policy(SET CMP0011 NEW)

message("")
message("** Start of configure XM7/SDL,Agar **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


project (xm7)

set(BUILD_SHARED_LIBS OFF)
set(USE_OPENMP ON CACHE BOOL "Build using OpenMP")
set(USE_OPENCL ON CACHE BOOL "Build using OpenCL")
set(USE_OPENGL ON CACHE BOOL "Build using OpenGL")
set(XM7_VERSION 3)
set(WITH_DEBUGGER ON CACHE BOOL "Build witn XM7 Debugger.")

include(detect_target_cpu)
#include(windows-mingw-cross)
# set entry
set(CMAKE_SYSTEM_PROCESSOR ${ARCHITECTURE} CACHE STRING "Set processor to build.")

include(config_xm7)

set(AGAR_LIBS ag_core ag_dev ag_gui)
set(AGAR_DEPLIBS m jpeg png z dl uim-scm uim)


include_directories(/usr/local/include/agar)
link_directories(/usr/local/lib)

find_package(Gettext)
include_directories(${GETTEXT_INCLUDE_PATH})
include(compile_gettext_catalogue)

find_package(Freetype)
include_directories(${FREETYPE_INCLUDE_PATH})

find_package(Iconv)


if(USE_OPENMP)
  find_package(OpenMP)
  include_directories(${OPENMP_INCLUDE_PATH})
endif()

find_package(Threads)
include_directories(${THREADS_INCLUDE_PATH})

find_package(SDL)
include_directories(${SDL_INCLUDE_PATH})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ui-agar)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/sdl)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fmgen)


if(ICONV_FOUND)
 include_directories(${ICONV_INCLUDE_DIRS})
 set(LOCAL_LIBS ${LOCAL_LIBS} ${ICONV_LIBRARIES})
endif()


# GCC Only?
if(CMAKE_COMPILER_IS_GNUCC) 
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flax-vector-conversions")
endif()

if(CMAKE_COMPILER_IS_GNUCXX) 
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -flax-vector-conversions")
endif()


include(simd-x86)

if(WITH_DEBUGGER)
 include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xm7-debugger)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WITH_DEBUGGER")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WITH_DEBUGGER")
 add_definitions(-D_WITH_DEBUGGER)
 set(LOCAL_LIBS ${LOCAL_LIBS} xm7_debugger)
 add_subdirectory(xm7-debugger)
endif()


add_subdirectory(vm)
add_subdirectory(ui-agar)
add_subdirectory(ui-agar/scaler/generic)
add_subdirectory(fmgen)


add_subdirectory(sdl/soundbuffer/generic)
add_subdirectory(sdl/vram/generic)
add_subdirectory(sdl)

add_subdirectory(resource)
add_subdirectory(locales/C)
add_subdirectory(locales/ja)




#add_executable(xm7 ALIAS xm7_main)


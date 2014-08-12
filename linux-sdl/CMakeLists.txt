cmake_minimum_required (VERSION 2.6)

message("")
message("** Start of configure XM7/SDL,Agar **")
message("")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


project (xm7)

set(BUILD_SHARED_LIBS OFF)
set(USE_SSE2 1)
set(USE_MMX  1)
set(USE_OPENMP 1)
set(USE_OPENCL 1)
set(WITH_DEBUGGER 1)


set(AGAR_LIBS ag_core ag_dev ag_gui)
set(AGAR_DEPLIBS m jpeg png16 z dl uim-scm uim)
set(LOCAL_LIBS      
		     xm7_ui_agar
                     xm7_vm
		     xm7_fmgen
		     xm7_vram-generic
		     xm7_scaler-generic
		     xm7_soundbuffer-generic

		     )


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_AGAR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_AGAR")
include_directories(/usr/local/include/agar)
link_directories(/usr/local/lib)

find_package(Gettext)
include_directories(${GETTEXT_INCLUDE_PATH})

find_package(Freetype)
include_directories(${FREETYPE_INCLUDE_PATH})

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


# Build Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DXM7_VER=3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXM7_VER=3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFDDSND")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFDDSND")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMOUSE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMOUSE")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMR2 -D_XWIN -DNDEBUG -D_M_IX86")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMR2 -D_XWIN -DNDEBUG -D_M_IX86")

# GCC Only?
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flax-vector-conversions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -flax-vector-conversions")



if(USE_SSE2)
 set(LOCAL_LIBS ${LOCAL_LIBS} xm7_vram-sse2)
 set(LOCAL_LIBS ${LOCAL_LIBS} xm7_soundbuffer-sse2)
 set(LOCAL_LIBS ${LOCAL_LIBS} xm7_scaler-sse2)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_SSE2")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SSE2")
 add_subdirectory(sdl/soundbuffer/sse2)
 add_subdirectory(sdl/vram/sse2)
 add_subdirectory(ui-agar/scaler/sse2)
endif()

#if(USE_SSE)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_SSE")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SSE")
#endif()

if(USE_MMX)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_MMX")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_MMX")
 set(LOCAL_LIBS ${LOCAL_LIBS} xm7_soundbuffer-mmx)
 add_subdirectory(sdl/soundbuffer/mmx)
endif()

if(WITH_DEBUGGER)
 include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xm7-debugger)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WITH_DEBUGGER")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WITH_DEBUGGER")

 set(LOCAL_LIBS ${LOCAL_LIBS} xm7_debugger)
 add_subdirectory(xm7-debugger)
endif()

find_package(OpenGL)

if(USE_OPENCL)
 if(OPENGL_FOUND)
   find_package(OpenCL)
   if(OPENCL_FOUND)
    include_directories(${OPENCL_INCLUDE_DIRS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_USE_OPENCL -DUSE_OPENCL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_OPENCL -DUSE_OPENCL")
    set(OPENCL_LIBRARY ${OPENCL_LIBRARIES})
    set(USE_OPENGL 1)
   endif()
 endif()
endif()

if(USE_OPENGL)
 if(OPENGL_FOUND)
   include_directories(${OPENGL_INCLUDE_PATH})
   set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -D_USE_OPENGL -DUSE_OPENGL")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_OPENGL -DUSE_OPENGL")
 else()
   set(USE_OPENGL 0)
   set(USE_OPENCL 0)
   unset(OPENCL_LIBRARY)
 endif()
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


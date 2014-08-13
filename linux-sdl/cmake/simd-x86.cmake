# Set simd X86.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

if((CMAKE_SYSTEM_PROCESSOR EQUAL "x86")
  OR (CMAKE_SYSTEM_PROCESSOR EQUAL "amd64")
  OR (CMAKE_SYSTEM_PROCESSOR EQUAL "x86_64")
  OR (CMAKE_SYSTEM_PROCESSOR EQUAL "ia32"))
 set(USE_SSE2 ON CACHE BOOL "Using SSE2 SIMD instructions, sometimes faster if enabled.")
 set(USE_MMX  ON CACHE BOOL "Using MMX SIMD instructions, sometimes faster if enabled.")
endif()


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


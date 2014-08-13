# Set configuration for building XM7/SDL.
# (C) 2014 K.Ohta <whatisthis.sowhat@gmail.com>
# This is part of XM7/SDL, but license is apache 2.2,
# this part was written only me.

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

# Build Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DXM7_VER=${XM7_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXM7_VER=${XM7_VERSION}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFDDSND")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFDDSND")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMOUSE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMOUSE")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMR2 -D_XWIN -DNDEBUG -D_M_IX86")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMR2 -D_XWIN -DNDEBUG -D_M_IX86")


find_package(OpenGL)

if(USE_OPENCL)
 if(OPENGL_FOUND)
   find_package(OpenCL)
   if(OPENCL_FOUND)
    include_directories(${OPENCL_INCLUDE_DIRS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_USE_OPENCL -DUSE_OPENCL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_OPENCL -DUSE_OPENCL")
    set(OPENCL_LIBRARY ${OPENCL_LIBRARIES})
    set(USE_OPENGL ON)
   endif()
 endif()
endif()

if(USE_OPENGL)
 if(OPENGL_FOUND)
   include_directories(${OPENGL_INCLUDE_PATH})
   set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -D_USE_OPENGL -DUSE_OPENGL")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_OPENGL -DUSE_OPENGL")
 else()
   set(USE_OPENGL OFF)
   set(USE_OPENCL OFF)
   unset(OPENCL_LIBRARY)
 endif()
endif()



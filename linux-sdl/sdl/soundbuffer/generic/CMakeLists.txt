message("* sdl/soundbuffer/generic")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -msse -mmmx")
add_library(xm7_soundbuffer-generic copysoundbuffer.c)

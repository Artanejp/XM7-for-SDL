message("* sdl/soundbuffer/mmx")

set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -mmmx -msse -msse2")
add_library(xm7_soundbuffer-sse2 copysoundbuffer.c)

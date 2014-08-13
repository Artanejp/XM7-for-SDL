message("* sdl/soundbuffer/mmx")

#set(CMAKE_BUILD_SETTING_C_FLAGS "${CMAKE_C_FLAGS} -mmmx")
add_compile_options(-mmmx)
add_library(xm7_soundbuffer-mmx copysoundbuffer.c)

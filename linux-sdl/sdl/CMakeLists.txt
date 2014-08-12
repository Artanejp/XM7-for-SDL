cmake_minimum_required (VERSION 2.6)

message("* sdl")


add_executable(xm7
                 api_draw.cpp
                 api_snd2.cpp api_wavwriter.cpp
		 snd_buffer.cpp
		 SndDrvTmpl.cpp SndDrvOpn.cpp SndDrvWav.cpp SndDrvBeep.cpp SndDrvCMT.cpp
		 api_kbd.cpp api_js.cpp api_mouse.cpp
		 SDLKbdInterface.cpp
		 SDLJoyInterface.cpp
		 KbdInterface.cpp 
		 draw_thread.cpp
		 sdl_cpuid.c
		 sdl_inifile.c
		 sdl_file.c
		 windows_main.cpp
		 )

target_link_libraries(xm7 ${LOCAL_LIBS}
                           ${AGAR_LIBS}
			   ${OPENGL_LIBRARY}
			   ${OPENCL_LIBRARY}
			   ${GETTEXT_LIBRARY}
			   ${OPENMP_LIBRARY}
			   ${SDL_LIBRARY}
			   ${AGAR_DEPLIBS}
			   ${THREADS_LIBRARY}
			   fontconfig
			   freetype
			   X11 Xinerama
			   fontconfig
)

install(TARGETS xm7 DESTINATION bin)
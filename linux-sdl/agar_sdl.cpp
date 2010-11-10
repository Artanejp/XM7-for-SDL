/*
 * agar_sdl.cpp
 *
 *  Created on: 2010/11/09
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *      Agarでは処理しきれないSDLイベントなどのハンドラ
 */

#include <SDL.h>
#ifdef USE_AGAR
//#include "agar_xm7.h"
#else
#include "sdl.h"
#endif
#include "api_js.h"
#include "SDLJoyInterface.h"

void EventSDL(void)
{
	SDL_Surface *p;
	SDL_Event eventQueue;

	/*
	 * JoyStickなどはSDLが管理する
	 */

	if(SDL_WasInit(SDL_INIT_VIDEO) != 0) {
		p = SDL_GetVideoSurface();
		if(p == NULL) return;
		while (SDL_PollEvent(&eventQueue))
		{
			switch (eventQueue.type)
			{
			case SDL_JOYAXISMOTION:	/* JS動く */
				OnMoveJoy(&eventQueue);
				break;
			case SDL_JOYBUTTONDOWN:
				OnPressJoy(&eventQueue);
				break;
			case SDL_JOYBUTTONUP:
				OnReleaseJoy(&eventQueue);
				break;
			case SDL_SYSWMEVENT:
				printf("NOTICE: SYSWM\n");
				break;
			default:
				break;
			}
		}
	}
}



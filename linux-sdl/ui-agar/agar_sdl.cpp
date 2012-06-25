/*
 * agar_sdl.cpp
 *
 *  Created on: 2010/11/09
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *      Agarでは処理しきれないSDLイベントなどのハンドラ
 */

#include <SDL/SDL.h>
#ifdef USE_AGAR
//#include "agar_xm7.h"
#else
#include "xm7_sdl.h"
#endif
#include <agar/core.h>
#include <agar/gui.h>
#include "api_js.h"
#include "SDLJoyInterface.h"

extern BOOL EventGuiSingle(AG_Driver *drv, AG_DriverEvent *ev);

void ConvertSDLEvent(AG_Driver *obj, SDL_Event *event, AG_DriverEvent *dev)
{
//	AG_SDL_GetNextEvent(void *obj, AG_DriverEvent *dev)
	AG_Driver *drv = obj;
	SDL_Event ev = *event;

//	if(agDriverSw) {
//		drv = &agDriverSw->_inherit;
//	} else {
//
//	}

	switch (ev.type) {
	case SDL_MOUSEMOTION:
		AG_MouseMotionUpdate(drv->mouse, ev.motion.x, ev.motion.y);
		dev->type = AG_DRIVER_MOUSE_MOTION;
		dev->win = NULL;
		dev->data.motion.x = ev.motion.x;
		dev->data.motion.y = ev.motion.y;
		break;
	case SDL_MOUSEBUTTONUP:
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED,
				ev.button.button);
		dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev.button.button;
		dev->data.button.x = ev.button.x;
		dev->data.button.y = ev.button.y;
		break;
	case SDL_MOUSEBUTTONDOWN:
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED,
				ev.button.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev.button.button;
		dev->data.button.x = ev.button.x;
		dev->data.button.y = ev.button.y;
		break;
	case SDL_KEYDOWN:
		AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED,
				(AG_KeySym)ev.key.keysym.sym,
				(Uint32)ev.key.keysym.unicode);

		dev->type = AG_DRIVER_KEY_DOWN;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev.key.keysym.sym;
		dev->data.key.ucs = (Uint32)ev.key.keysym.unicode;
		break;
	case SDL_KEYUP:
		AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED,
				(AG_KeySym)ev.key.keysym.sym,
				(Uint32)ev.key.keysym.unicode);

		dev->type = AG_DRIVER_KEY_UP;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev.key.keysym.sym;
		dev->data.key.ucs = (Uint32)ev.key.keysym.unicode;
		break;
	case SDL_VIDEORESIZE:
		dev->type = AG_DRIVER_VIDEORESIZE;
		dev->win = NULL;
		dev->data.videoresize.x = 0;
		dev->data.videoresize.y = 0;
		dev->data.videoresize.w = (int)ev.resize.w;
		dev->data.videoresize.h = (int)ev.resize.h;
		break;
	case SDL_VIDEOEXPOSE:
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = NULL;
		break;
	case SDL_QUIT:
	case SDL_USEREVENT:
		dev->type = AG_DRIVER_CLOSE;
		dev->win = NULL;
		break;
	}
}



BOOL EventSDL(AG_Driver *drv)
{
//	SDL_Surface *p;
	SDL_Event eventQueue;
	AG_DriverEvent event;

	/*
	 * JoyStickなどはSDLが管理する
	 */
//	AG_SDL_GetNextEvent(void *obj, AG_DriverEvent *dev)

//	if(SDL_WasInit(SDL_INIT_JOYSTICK) != 0) {
//		p = SDL_GetVideoSurface();
//		if(p == NULL) return TRUE;
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
			default:
				ConvertSDLEvent(drv, &eventQueue, &event);
				if(!EventGuiSingle(drv, &event)) return FALSE;
				break;
			}
		}
//	}
	return TRUE;
}



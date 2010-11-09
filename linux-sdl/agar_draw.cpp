/*
 * agar_draw.cpp
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */

#include <SDL.h>
#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>


extern AG_Window *MainWindow;


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


static void InitGUI(void)
{
	Uint flags = AG_VIDEO_DOUBLEBUF | AG_VIDEO_HWSURFACE | AG_VIDEO_ASYNCBLIT |
			AG_VIDEO_OPENGL_OR_SDL | AG_VIDEO_RESIZABLE;
	AG_InitVideo(w, h, 32, flags);
	MainWindow = AG_WindowNew(0);
}




void ResizeWindow(int w, int h)
{
	AG_ResizeDisplay((Uint)w, (Uint)h);
}
static void ProcessGUI(void)
{
	AG_BeginRendering();
	AG_WindowDraw();
	AG_EndRendering();

}

void AGDrawTaskEvent(void)
{
	EventSDL();
	EventGUI();
	ProcessGUI();
}

void AGDrawTaskMain(void)
{

		if(newResize) {
			nDrawWidth = newDrawWidth;
			nDrawHeight = newDrawHeight;
			ResizeWindow(nDrawWidth, nDrawHeight);
			newResize = FALSE;
		}
		ChangeResolution();
		SelectDraw2();

#if XM7_VER >= 3
		/*
		 *    いずれかを使って描画
		 */
		SDL_SemWait(DrawInitSem);
		switch (bMode) {
		case SCR_400LINE:
			Draw400l();
			break;
		case SCR_262144:
			Draw256k();
			break;
		case SCR_4096:
			Draw320();
			break;
		case SCR_200LINE:
			Draw640All();
			break;
		}
		SDL_SemPost(DrawInitSem);
#else				/*  */
		/*
		 * どちらかを使って描画
		 */
		if (bAnalog) {
			Draw320All();
		}
		else {
			Draw640All();
		}
#endif				/*  */
		//        SDL_UnlockSurface(p);
		Flip();
}


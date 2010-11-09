/*
 * agar_draw.cpp
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include "api_draw.h"
#include "api_scaler.h"

extern AG_Window *MainWindow;
extern void EventSDL(void);


static void InitGUI(int w, int h)
{
	Uint flags = AG_VIDEO_DOUBLEBUF | AG_VIDEO_HWSURFACE | AG_VIDEO_ASYNCBLIT |
			AG_VIDEO_OPENGL_OR_SDL | AG_VIDEO_RESIZABLE;
	AG_InitVideo(w, h, 32, flags);
	MainWindow = AG_WindowNew(0);
}
void ResizeWindow_Agar(int w, int h)
{
	AG_ResizeDisplay(w, h);
}
static void ProcessGUI(void)
{
	AG_BeginRendering(NULL);
	AG_WindowDraw(MainWindow);
	AG_EndRendering(NULL);

}

void AGDrawTaskEvent(void)
{
	EventSDL();
//	EventGUI();
	ProcessGUI();
}

void AGDrawTaskMain(void)
{

		if(newResize) {
			nDrawWidth = newDrawWidth;
			nDrawHeight = newDrawHeight;
			ResizeWindow_Agar(nDrawWidth, nDrawHeight);
			newResize = FALSE;
		}
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


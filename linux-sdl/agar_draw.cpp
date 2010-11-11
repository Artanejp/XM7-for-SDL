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

extern AG_GLView *DrawArea;
extern AG_Window *MainWindow;
extern Uint32 nDrawTick1;
extern void EventSDL(void);
extern void EventGUI(AG_Driver *drv);



extern void InitGL(int w, int h);

void InitGUI(int w, int h)
{
	Uint flags = AG_VIDEO_DOUBLEBUF | AG_VIDEO_HWSURFACE | AG_VIDEO_ASYNCBLIT |
			AG_VIDEO_OPENGL_OR_SDL | AG_VIDEO_RESIZABLE;
	AG_InitVideo(w, h, 32, flags);
	InitGL(w, h);
	MainWindow = AG_WindowNew(0);
}

void ResizeWindow_Agar(int w, int h)
{
	AG_ResizeDisplay(w, h);
}
static void ProcessGUI(void)
{
}


void AGDrawTaskEvent(void)
{
	Uint32 nDrawTick2;
	AG_Window *win;
	AG_Driver *drv;

	nDrawTick2 = AG_GetTicks();
	if(nDrawTick2 < nDrawTick1) nDrawTick1 = 0; // オーバーフロー対策
	if(agDriverSw) {
		if(nDrawTick2 - nDrawTick1 > agDriverSw->rNom) {
			// ここにGUIの処理入れる
			AG_LockVFS(&agDrivers);
			if (agDriverSw) {
				/* With single-window drivers (e.g., sdlfb). */
				AG_BeginRendering(agDriverSw);
				AG_FOREACH_WINDOW(win, agDriverSw) {
					AG_ObjectLock(win);
					AG_WindowDraw(win);
					AG_ObjectUnlock(win);
				}
				AG_EndRendering(agDriverSw);
				AG_UnlockVFS(&agDrivers);
				nDrawTick1 = nDrawTick2;
				drv = &agDriverSw->_inherit;
				EventGUI(drv);
			}
		}
	}
	EventSDL();
}

void AGDrawTaskMain(void)
{

	AG_Window *win;
	Uint32 *nDrawTick2;
		if(newResize) {
			nDrawWidth = newDrawWidth;
			nDrawHeight = newDrawHeight;
			ResizeWindow_Agar(nDrawWidth, nDrawHeight);
			newResize = FALSE;
		}
		SelectDraw2();
		/* Render the Agar windows */
#if XM7_VER >= 3
		/*
		 *    いずれかを使って描画
		 */
//		SDL_SemWait(DrawInitSem);
		AG_ObjectLock(DrawArea);
		if(scalerGL){
			scalerGL->SetDrawArea(AGWIDGET(DrawArea), 0, 0, nDrawWidth, nDrawHeight);
		}
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
		AG_GLViewDraw(DrawArea);
//		SDL_SemPost(DrawInitSem);
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
//		Flip();

}


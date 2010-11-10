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
extern void EventSDL(void);

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

void EventGUI(void)
{
	AG_Driver *drv;
	AG_DriverEvent ev;
	if(MainWindow == NULL) return;
	drv = AGWIDGET(MainWindow)->drv;
	if(drv == NULL) return;
	 if (AG_PendingEvents(drv) > 0) {
				/*
				 * Case 2: There are events waiting to be processed.
				 */
				do {
					/* Retrieve the next queued event. */
					if (AG_GetNextEvent(drv, &ev) == 1) {
						switch (ev.type) {
						default:
							break;
						}

						/* Forward the event to Agar. */
						if (AG_ProcessEvent(drv, &ev) == -1)
							return;
					}
				} while (AG_PendingEvents(drv) > 0);
	 }
}

void AGDrawTaskEvent(void)
{
	EventSDL();
	EventGUI();
}

void AGDrawTaskMain(void)
{

	AG_Window *win;
		if(newResize) {
			nDrawWidth = newDrawWidth;
			nDrawHeight = newDrawHeight;
			ResizeWindow_Agar(nDrawWidth, nDrawHeight);
			newResize = FALSE;
		}
		SelectDraw2();
		/* Render the Agar windows */
		AG_LockVFS(&agDrivers);
		if (agDriverSw) {
			/* With single-window drivers (e.g., sdlfb). */
			AG_BeginRendering(agDriverSw);
			AG_FOREACH_WINDOW(win, agDriverSw) {
			AG_ObjectLock(win);
			AG_WindowDraw(win);
			AG_ObjectUnlock(win);
			}
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
		AG_ObjectUnlock(DrawArea);
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

			AG_EndRendering(agDriverSw);
		} else {
		}
		AG_UnlockVFS(&agDrivers);
		//        SDL_UnlockSurface(p);
//		Flip();
		ProcessGUI();

}


/*
 * agar_draw.cpp
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
#include <libemugrph/EmuAgarGL.h>
#include "api_draw.h"
#include "api_scaler.h"

extern "C" {
extern AG_GLView *DrawArea;
extern AG_Window *MainWindow;
extern AG_Menu  *MenuBar;
extern AG_Box *OsdArea;
}
extern EmuAgarGL *scalerGL;
extern Uint32 nDrawTick1;
extern void EventSDL(AG_Driver *drv);
extern void EventGUI(AG_Driver *drv);



void InitGUI(int w, int h)
{
}

void ResizeWindow_Agar(int w, int h)
{
	AG_ResizeDisplay(w, h);
	if(AGWIDGET(MenuBar)) {
		AG_WidgetSetSize(AGWIDGET(MenuBar),24, newDrawWidth);
	}
}

static void ProcessGUI(void)
{
}

void AGEventOverlayGL(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();
	if(scalerGL == NULL) return;
//	AG_WidgetBlit(glv, 0, 32);
//	scalerGL->DiscardTexture(scalerGL->GetTextureID());
}


void AGEventScaleGL(AG_Event *event)
{
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram ;

	if(scalerGL == NULL) return;
	pixvram = scalerGL->GetVramSurface();
	if(pixvram == NULL) return;
	scalerGL->SetDrawArea(wid, 0, 0, nDrawWidth, nDrawHeight);
	scalerGL->SetViewPort(0, 0, nDrawWidth, nDrawHeight);
	scalerGL->SetOffset(0,32);
	scalerGL->SetTextureID(scalerGL->CreateTexture(pixvram));
	scalerGL->DrawTexture(scalerGL->GetTextureID());
	scalerGL->DiscardTexture(scalerGL->GetTextureID());
//	AG_WidgetUpdate(wid);

}

void AGEventDrawGL(AG_Event *event)
{
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram;

#if XM7_VER >= 3
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
	if(scalerGL == NULL) return;
	pixvram = scalerGL->GetVramSurface();
	scalerGL->SetDrawArea(wid, 0, 0, nDrawWidth, nDrawHeight);
	scalerGL->SetViewPort(0, 0, nDrawWidth, nDrawHeight);
	scalerGL->SetOffset(0,32);
	scalerGL->SetTextureID(scalerGL->CreateTexture(pixvram));
	scalerGL->DrawTexture(scalerGL->GetTextureID());
	scalerGL->DiscardTexture(scalerGL->GetTextureID());
}



void AGDrawTaskEvent(BOOL flag)
{
	Uint32 nDrawTick2;
	AG_Window *win;
	AG_Driver *drv;
	AG_Surface *pixvram;
	Uint32 fps;
	if(nDrawFPS > 2) {
		fps = 1000 / nDrawFPS;
	} else {
		fps = 500;
	}

	nDrawTick2 = AG_GetTicks();
	if(nDrawTick2 < nDrawTick1) nDrawTick1 = 0; // オーバーフロー対策
	if(agDriverSw) {
		if(flag && ((nDrawTick2 - nDrawTick1) > fps)) {
			// ここにGUIの処理入れる
			AG_LockVFS(&agDrivers);
			if (agDriverSw) {
				/* With single-window drivers (e.g., sdlfb). */
				AG_BeginRendering(agDriverSw);
				AG_WidgetDraw(DrawArea);
//				AG_WidgetDraw(MenuBar);
//				if(OsdArea != NULL) {
//					AG_WidgetDraw(OsdArea);
//				}
				AG_FOREACH_WINDOW(win, agDriverSw) {
						AG_ObjectLock(win);
						AG_WindowDraw(win);
						AG_ObjectUnlock(win);
				}
				nDrawTick1 = nDrawTick2;
				AG_EndRendering(agDriverSw);
			}
			AG_UnlockVFS(&agDrivers);
		}	else {
			drv = &agDriverSw->_inherit;
			EventSDL(drv);
			EventGUI(drv);
		}
	}
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
}


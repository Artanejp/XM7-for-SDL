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

extern AG_GLView *DrawArea;
extern AG_Window *MainWindow;
extern EmuAgarGL *scalerGL;
extern Uint32 nDrawTick1;
extern void EventSDL(void);
extern void EventGUI(AG_Driver *drv);




void InitGUI(int w, int h)
{
}

void ResizeWindow_Agar(int w, int h)
{
	AG_ResizeDisplay(w, h);
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
	scalerGL->SetTextureID(scalerGL->CreateTexture(pixvram));
	scalerGL->DrawTexture(scalerGL->GetTextureID());
	scalerGL->DiscardTexture(scalerGL->GetTextureID());
}

void AGEventDrawGL(AG_Event *event)
{
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram;

	AG_ObjectLock(wid);
#if XM7_VER >= 3
	if(scalerGL){
		scalerGL->SetDrawArea(wid, 0, 32, nDrawWidth, nDrawHeight);
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
//	AG_GLViewDraw(wid);
//	AG_ObjectUnlock(wid);
	AG_WidgetUpdate(wid);
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
	if(pixvram == NULL) return;
	scalerGL->SetTextureID(scalerGL->CreateTexture(pixvram));
	scalerGL->DrawTexture(scalerGL->GetTextureID());
	scalerGL->DiscardTexture(scalerGL->GetTextureID());
}



void AGDrawTaskEvent(BOOL flag)
{
	Uint32 nDrawTick2;
	AG_Window *win;
	AG_Driver *drv;
	AG_GLView *glv = DrawArea;
	AG_Surface *pixvram;

	nDrawTick2 = AG_GetTicks();
	if(nDrawTick2 < nDrawTick1) nDrawTick1 = 0; // オーバーフロー対策
	if(agDriverSw) {
		if(nDrawTick2 - nDrawTick1 > agDriverSw->rNom) {
			// ここにGUIの処理入れる
			AG_LockVFS(&agDrivers);
			if (agDriverSw) {
				/* With single-window drivers (e.g., sdlfb). */
				AG_BeginRendering(agDriverSw);
#if XM7_VER >= 3
			if(scalerGL != NULL) {
				scalerGL->SetDrawArea(DrawArea, 0,0,nDrawWidth, nDrawHeight);
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
		/*
		 *    いずれかを使って描画
		 */
		//		SDL_SemWait(DrawInitSem);
	if(scalerGL != NULL) {
		AG_Color c;
		AG_Rect r;
		pixvram = scalerGL->GetVramSurface();
		if(pixvram != NULL) {
#if 1
		scalerGL->SetViewPort(0, 0, nDrawWidth, nDrawHeight);
		scalerGL->SetOffset(0,24);
		scalerGL->SetTextureID(scalerGL->CreateTexture(pixvram));
		scalerGL->DrawTexture(scalerGL->GetTextureID());
		scalerGL->DiscardTexture(scalerGL->GetTextureID());
#else

		//				AG_WidgetBlit(AGWIDGET(glv), pixvram,0,0);
		//AG_WidgetBlit(AGWIDGET(glv), pixvram,0,0);

#endif
		}
	}
//		AG_GLViewDraw(glv);
	//AG_WidgetDraw(AGWIDGET(glv));
	//AG_WidgetShow(AGWIDGET(glv));
	//	AG_ObjectUnlock(glv);
	//	AG_WidgetUpdate(AGWIDGET(glv));
	AG_FOREACH_WINDOW(win, agDriverSw) {
		AG_ObjectLock(win);
		AG_WindowDraw(win);
		AG_ObjectUnlock(win);
	}
				AG_EndRendering(agDriverSw);
//				Flip();
				AG_UnlockVFS(&agDrivers);
				nDrawTick1 = nDrawTick2;
			}
		}	else {
			drv = &agDriverSw->_inherit;
			EventGUI(drv);
			EventSDL();
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

		//        SDL_UnlockSurface(p);
//		Flip();

}


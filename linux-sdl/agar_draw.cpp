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
extern void DrawStatus(void);
}
extern EmuAgarGL *scalerGL;
extern Uint32 nDrawTick1;
extern void EventSDL(AG_Driver *drv);
extern void EventGUI(AG_Driver *drv);
extern void DrawOSDGL(AG_GLView *w);

extern GLuint tid_ins;
extern GLuint tid_kana;
extern GLuint tid_caps;
extern GLuint tid_fd0;
extern GLuint tid_fd1;
extern GLuint tid_cmt;
extern GLuint tid_caption;

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
	if(DrawArea == NULL) return;
	DrawOSDGL(DrawArea);
}


void AGEventScaleGL(AG_Event *event)
{
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram ;

	if(scalerGL == NULL) return;
	pixvram = scalerGL->GetVramSurface();
	if(pixvram == NULL) return;
#if 0
	scalerGL->SetDrawArea(wid, 0, DRAW_OFSET, nDrawWidth, nDrawHeight);
//	scalerGL->SetViewPort(0, 0, nDrawWidth, nDrawHeight);
	scalerGL->SetViewPort(0, 0, nDrawWidth, nDrawHeight , nDrawWidth, OSD_HEIGHT);
	scalerGL->SetOffset(0,DRAW_OFSET);
	scalerGL->SetTextureID(scalerGL->CreateTexture(pixvram));
	scalerGL->DrawTexture(scalerGL->GetTextureID());
	DrawOSDGL(DrawArea);
	scalerGL->DiscardTexture(scalerGL->GetTextureID());
#endif
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
	SelectDraw2();
	pixvram = scalerGL->GetVramSurface();
	if(pixvram == NULL) return;
	scalerGL->SetDrawArea(wid, 0, DRAW_OFSET, pixvram->w, pixvram->h);
	scalerGL->SetViewPort(wid->wid.x, wid->wid.y, nDrawWidth, nDrawHeight, nDrawWidth, OSD_HEIGHT);
	scalerGL->SetOffset(0, DRAW_OFSET);
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
			if(DrawArea) {
//				AG_SizeAlloc r;
				AG_WidgetSetSize(AGWIDGET(DrawArea), newDrawWidth, newDrawHeight);
			}
		}
//		SelectDraw2();
		/* Render the Agar windows */
}


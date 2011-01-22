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
#include "agar_gldraw.h"
#include <SDL.h>

extern void DrawOSDGL(AG_GLView *w);
extern "C" {
extern AG_GLView *OsdArea;
}

void InitGUI(int w, int h)
{
}

void ResizeWindow_Agar(int w, int h)
{
	int hh;
	int ww;
	int sh = RootVideoHeight - 10;
	int sw = RootVideoWidth - 16;
//	AG_ResizeDisplay(w, h);
	if(DrawArea == NULL) return;
	if(w > sw) w = sw;
	if(h > sh) h = sh;
	switch(nAspect) {
	case nAspect11:
		if(w>h) {
			h = (w * 5) / 8;
		} else {
			w = (h * 8) / 5;
		}
		break;
	case nAspect43:
		if(w>h) {
			h = (w * 6) / 8;
		} else {
			w = (h * 8) / 6;
		}
		break;
	}
	if(w > sw) w = sw;
	if(h > sh) h = sh;

	AG_WidgetSetSize(AGWIDGET(DrawArea), w, h);
	if(OsdArea != NULL){
		ww = w>OsdArea->wid.w?w:OsdArea->wid.w;
		hh = h + OsdArea->wid.h;
	} else{
		ww = w;
		hh = h;
	}
	if(MenuBar != NULL) {
		hh += MenuBar->wid.h;
		ww = ww>MenuBar->wid.w?ww:ww>MenuBar->wid.w;
	}
	sh = RootVideoHeight;
	sw = RootVideoWidth;
	if(ww > sw) ww = sw;
	if(hh > sh) hh = sh;

	AG_ResizeDisplay(ww, hh);
}

static void ProcessGUI(void)
{
}


void AGDrawTaskMain(void)
{

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
	SelectDraw2();
		/* Render the Agar windows */
}


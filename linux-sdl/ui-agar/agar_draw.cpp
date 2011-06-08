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
#include "EmuAgarGL.h"
#include "api_draw.h"
#include "api_scaler.h"

#include "agar_draw.h"
#include "agar_gldraw.h"
#include "DrawAGNonGL.h"

#include <SDL.h>

extern BYTE bMode;
Uint32 nDrawTick1E;
static BYTE oldBMode;
DrawAGNonGL *DrvNonGL;

extern "C" {
    AG_Box *DrawArea;
}

void InitDrawArea(int w, int h)
{
    DrvNonGL = new DrawAGNonGL;
    if(DrvNonGL != NULL) {
        DrvNonGL->InitDraw(w, h);
    }
}

void DetachDrawArea(void)
{
    if(DrvNonGL)
    {
        delete DrvNonGL;
        DrvNonGL = NULL;
    }
}


void LinkDrawArea(AG_Widget *w)
{

    if(w == NULL) return;
    if(DrvNonGL == NULL) return;
    DrvNonGL->LinkVram(w);
}

void UnlinkDrawArea(AG_Widget *w)
{

    if(w == NULL) return;
    if(DrvNonGL == NULL) return;
    DrvNonGL->UnlinkVram(w);
}


SDL_Surface *GetDrawSurface(void)
{
    if(DrvNonGL == NULL) return NULL;
    return DrvNonGL->GetSDLSurface();
}

void ResizeWindow_Agar(int w, int h)
{
	int hh;
	AG_Driver *drv;

	if(agDriverSw) {
	    drv = &agDriverSw->_inherit;
	} else {
	    if(MainWindow == NULL) return;
	    drv = AGDRIVER(MainWindow);
	}
#if 0
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
#endif

	if(DrawArea != NULL) {
        AG_WidgetSetSize(AGWIDGET(DrawArea), w, h);
        AG_WidgetSetPosition(AGWIDGET(DrawArea), 0, MenuBar->wid.h + 10);
        InitDrawArea(w, h);
        LinkDrawArea(AGWIDGET(DrawArea));
	}
	if(GLDrawArea != NULL) {
        AG_GLViewSizeHint(GLDrawArea, w, h);
        AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h);
        AG_WidgetSetPosition(AGWIDGET(GLDrawArea), 0, MenuBar->wid.h + 10);
	}
	nDrawWidth = w;
	nDrawHeight = h;
	hh = h;
	if(MenuBar) {
        AG_ObjectLock(AGOBJECT(drv));
		AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
        AG_WidgetEnable(AGWIDGET(MenuBar));
        AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
        hh = h + MenuBar->wid.h + 10;
       	AG_ObjectLock(AGOBJECT(drv));
//    AG_WidgetFocus(AGWIDGET(MenuBar));
	}
	if(MainWindow) {
		AG_WindowSetGeometry(MainWindow, 0,  MenuBar->wid.h + 10, w, h );
        AG_Redraw(AGWIDGET(MainWindow));
	}
	if(AG_UsingGL(drv)) {
        AG_ResizeDisplay(w, hh);
	}
    printf("Resize to %d x %d\n", w, hh);
//    AG_Redraw(AGWIDGET(MenuBar));
}

void ResizeWindow_Agar2(int w, int h)
{
	int hh;
	int ww;
	AG_Driver *drv;

	if(agDriverSw) {
	    drv = &agDriverSw->_inherit;
	} else {
	    if(MainWindow == NULL) return;
	    drv = AGDRIVER(MainWindow);
	}
#if 0
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
#endif

	ww = w;
	hh = h;
	hh = h - MenuBar->wid.h - 10;
	if(hh < 0) hh = 0;
	if(DrawArea != NULL) {
        AG_WidgetSetSize(AGWIDGET(DrawArea), ww, hh);
        AG_WidgetSetPosition(AGWIDGET(DrawArea), 0, MenuBar->wid.h + 10);
        InitDrawArea(ww, hh);
        LinkDrawArea(AGWIDGET(DrawArea));
	}
	if(GLDrawArea != NULL) {
        AG_GLViewSizeHint(GLDrawArea, ww, hh);
        AG_WidgetSetSize(AGWIDGET(GLDrawArea), ww, hh);
        AG_WidgetSetPosition(AGWIDGET(GLDrawArea), 0, MenuBar->wid.h + 10);
	}
 	AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
 	nDrawWidth = w;
	nDrawHeight = hh;
 	AG_WidgetEnable(AGWIDGET(MenuBar));
	if(MainWindow) {
		AG_WindowSetGeometry(MainWindow, 0, MenuBar->wid.h + 10 , w, hh);
	}
//    AG_WidgetFocus(AGWIDGET(MenuBar));
	if(AG_UsingGL(drv)) {
        AG_ResizeDisplay(w, hh);
	}
    printf("Resize to %d x %d\n", w, h);
    if(MainWindow) {
        AG_Redraw(AGWIDGET(MainWindow));
    }
// 	AG_Redraw(AGWIDGET(MenuBar));
}


void AGDrawTaskMain(void)
{
	Uint32 nDrawTick2E;
	Uint32 fps;

	if(nEmuFPS > 2) {
		fps = 1000 / nEmuFPS;
	} else {
		fps = 500;
	}
	nDrawTick2E = AG_GetTicks();
	if(nDrawTick1E > nDrawTick2E) {
		nDrawTick1E = 0;
	}
	if(((nDrawTick2E - nDrawTick1E)<fps) && (bMode == oldBMode)) return;
	nDrawTick1E = nDrawTick2E;
	oldBMode = bMode;
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


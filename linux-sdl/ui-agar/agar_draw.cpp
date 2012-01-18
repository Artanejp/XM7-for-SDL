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
//#include "EmuAgarGL.h"
#include "api_draw.h"
#include "api_scaler.h"

#include "agar_draw.h"
#include "agar_gldraw.h"
//#include "DrawAGNonGL.h"

#include <SDL.h>

extern BYTE bMode;
Uint32 nDrawTick1E;
static BYTE oldBMode;

extern "C" {
    AG_Box *DrawArea;
}

void InitDrawArea(int w, int h)
{
}

void DetachDrawArea(void)
{
}


void LinkDrawArea(AG_Widget *w)
{
    if(w == NULL) return;
}

void UnlinkDrawArea(AG_Widget *w)
{
    if(w == NULL) return;
}


SDL_Surface *GetDrawSurface(void)
{
    return NULL;
}

void ResizeWindow_Agar(int w, int h)
{
	int hh;
	int ofset;

	AG_Driver *drv;

	if(agDriverSw) {
	    drv = &agDriverSw->_inherit;
	} else {
	    if(MainWindow == NULL) return;
	    drv = AGDRIVER(MainWindow);
	}
	ofset = (int)((float)h * (40.0f / 440.0f));
	if(DrawArea != NULL) {
	   AG_WidgetSetSize(AGWIDGET(DrawArea), w, h + ofset);
	   AG_WidgetSetPosition(AGWIDGET(DrawArea), 0, 0);
	   LinkDrawArea(AGWIDGET(DrawArea));
	   AG_Redraw(AGWIDGET(DrawArea));
	}
	if(GLDrawArea != NULL) {
        AG_GLViewSizeHint(GLDrawArea, w, h);
        AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h + ofset);
	}
	nDrawWidth = w;
	nDrawHeight = h;
	hh = h + ofset;
	if(MenuBar) {
        AG_ObjectLock(AGOBJECT(drv));
		AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
        AG_WidgetEnable(AGWIDGET(MenuBar));
        AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
        hh = h + MenuBar->wid.h;
       	AG_ObjectLock(AGOBJECT(drv));
//    AG_WidgetFocus(AGWIDGET(MenuBar));
	}
	if(MainWindow) {
		AG_WindowSetGeometry(MainWindow, 0, 0 , w, h + ofset + MenuBar->wid.h );
        AG_Redraw(AGWIDGET(MainWindow));
        AG_WindowFocus(MainWindow);
	}
	if(AG_UsingGL(drv) && (agDriverSw != NULL)) {
        AG_ResizeDisplay(w, hh);
	}
    printf("Resize to %d x %d\n", w, hh);
//    AG_Redraw(AGWIDGET(MenuBar));
}
/*
* Resize (Window only)
*/
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

    ww = w;
    hh = h - MenuBar->wid.h;
	if(hh < 0) hh = 0;
	if(DrawArea != NULL) {
        AG_WidgetSetSize(AGWIDGET(DrawArea), ww, hh);
        AG_WidgetSetPosition(AGWIDGET(DrawArea), 0, 0);
        LinkDrawArea(AGWIDGET(DrawArea));
        AG_Redraw(AGWIDGET(DrawArea));
	}
	if(GLDrawArea != NULL) {
        AG_GLViewSizeHint(GLDrawArea, ww, hh);
        AG_WidgetSetSize(AGWIDGET(GLDrawArea), ww, hh);
        AG_WidgetSetPosition(AGWIDGET(GLDrawArea), 0, 0);
	}
 	AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
 	nDrawWidth = w;
	nDrawHeight = hh;
 	AG_WidgetEnable(AGWIDGET(MenuBar));
	if(MainWindow) {
		AG_WindowSetGeometry(MainWindow, 0, 0, w, hh);
        AG_Redraw(AGWIDGET(MainWindow));
        AG_WindowFocus(MainWindow);
	}
    printf("Resize to %d x %d\n", w, h);
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
	SelectDraw2();
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
//        if(DrvNonGL) {
//	   DrvNonGL->Flip();
//	}
		/* Render the Agar windows */
}


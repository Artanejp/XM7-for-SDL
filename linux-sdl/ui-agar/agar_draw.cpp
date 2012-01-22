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
//#include "api_scaler.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"
//#include "DrawAGNonGL.h"

#include <SDL.h>

extern BYTE bMode;

Uint32 nDrawTick1E;
static BYTE oldBMode;

extern "C" {
XM7_SDLView *DrawArea;
}


void InitGL(int w, int h)
{
    AG_Driver *drv;

    if(agDriverSw) {
        drv = &agDriverSw->_inherit;
    } else if(MainWindow != NULL) {
        drv = AGDRIVER(MainWindow);
    } else {
        return;
    }
    SDL_SemWait(DrawInitSem);
    if(AG_UsingGL(NULL)) {
        InitGL_AG2(w, h);
    } else {
        InitNonGL(w, h);
//        AG_ResizeDisplay(w, h);
    }
    SDL_SemPost(DrawInitSem);
}

void InitNonGL(int w, int h)
{
	Uint32 flags;
	char *ext;

	if(InitVideo) return;
    InitVideo = TRUE;

    vram_pb = NULL;
    vram_pg = NULL;
    vram_pr = NULL;

	flags = SDL_RESIZABLE;

	InitVramSemaphore();
	pVirtualVram = NULL;
	InitVirtualVram();
    return;
}


void DetachDrawArea(void)
{
    if(DrawArea == NULL) return;
    XM7_SDLViewSurfaceDetach(DrawArea);
    AG_ObjectDetach(AGOBJECT(DrawArea));
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
	ofset = 40;
	if(MainWindow) {
		AG_WindowSetGeometry(MainWindow, 0, 0 , w, h + ofset + MenuBar->wid.h );
        AG_Redraw(AGWIDGET(MainWindow));
        AG_WindowFocus(MainWindow);
	}
	if(DrawArea != NULL) {
        AG_SizeAlloc a;
	    a.x = 0;
	    a.y = 0;
	    a.w = w;
	    a.h = h;
//	   AG_WidgetSetSize(AGWIDGET(DrawArea), w, h + ofset);
	   AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
	   AG_Redraw(AGWIDGET(DrawArea));
	}
	if(GLDrawArea != NULL) {
        AG_GLViewSizeHint(GLDrawArea, w, h);
//        AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h + ofset);
        AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h);
	}
	nDrawWidth = w;
	nDrawHeight = h;
	hh = h + ofset;
	if(MenuBar) {
        AG_ObjectLock(AGOBJECT(drv));
		AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
        AG_WidgetEnable(AGWIDGET(MenuBar));
        AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
        hh = hh + MenuBar->wid.h;
       	AG_ObjectLock(AGOBJECT(drv));
//    AG_WidgetFocus(AGWIDGET(MenuBar));
	}

	if(AG_UsingGL(NULL) ) {
	   AG_ResizeDisplay(w, hh);
	}
    printf("Resize to %d x %d\n", w, hh );
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
        int ofset = 40;

	if(agDriverSw) {
	    drv = &agDriverSw->_inherit;
	} else {
	    if(MainWindow == NULL) return;
	    drv = AGDRIVER(MainWindow);
	}
    ww = w;
    if(MenuBar != NULL) {
        hh = h - MenuBar->wid.h;
    }
    hh = hh - ofset;
	if(hh < 0) hh = 0;
	if(DrawArea != NULL) {
        AG_SizeAlloc a;
	    a.x = 0;
	    a.y = 0;
	    a.w = ww;
	    a.h = hh;
	   AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
       AG_WidgetSetPosition(AGWIDGET(DrawArea), 4, 0);
	}
	if(GLDrawArea != NULL) {
        if(MainWindow) {
            AG_WindowSetGeometry(MainWindow, 0, 0, w, h);
            AG_Redraw(AGWIDGET(MainWindow));
            AG_WindowFocus(MainWindow);
        }
        AG_GLViewSizeHint(GLDrawArea, ww, hh);
        AG_WidgetSetPosition(AGWIDGET(GLDrawArea), 0, 0);
        AG_WidgetSetSize(AGWIDGET(GLDrawArea), ww, hh);
	}
 	nDrawWidth = w;
	nDrawHeight = hh;
 	AG_WidgetEnable(AGWIDGET(MenuBar));
	if(MenuBar != NULL) {
        AG_ObjectLock(AGOBJECT(drv));
        AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
        AG_ObjectUnlock(AGOBJECT(drv));
	}
    printf("Resize2 to %d x %d\n", w, h);
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
		/* Render the Agar windows */
}


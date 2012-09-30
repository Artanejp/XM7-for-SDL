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
#include <SDL/SDL.h>


#include "api_draw.h"
//#include "api_scaler.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"


extern BYTE bMode;

Uint32 nDrawTick1E;
static BYTE oldBMode;

extern void ResizeStatus(AG_Widget *parent, int w, int h, int y);

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
#ifdef USE_OPENGL
   if(AG_UsingGL(NULL)) {
        InitGL_AG2(w, h);
    } else {
        InitNonGL(w, h);
//        AG_ResizeDisplay(w, h);
    }
#else
        InitNonGL(w, h);
   //     AG_ResizeDisplay(w, h);
#endif
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
    if(CheckVramSemaphore) DetachVramSemaphore();
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
	int ww;
//	int ofset;

	AG_Driver *drv;
        if((w < 100) || (h < 100)) return;
	if(agDriverSw) {
	    drv = &agDriverSw->_inherit;
	} else {
	    if(MainWindow == NULL) return;
	    drv = AGDRIVER(MainWindow);
	}
//	ofset = 40;
        nDrawWidth = w;
        nDrawHeight = h;
        AG_ObjectLock(AGOBJECT(drv));
        ww = w;
        hh = 0;
      if(MenuBar) {
//	AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
//      AG_WidgetEnable(AGWIDGET(MenuBar));
//        AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
        hh = hh + MenuBar->wid.h;
//    AG_WidgetFocus(AGWIDGET(MenuBar));
	}
       if(GLDrawArea) {
	  AG_SizeReq r;
//	  AG_ObjectLock(AGOBJECT(GLDrawArea));
	  r.w = w;
	  r.h = h;
//	  AG_WidgetSizeReq(AGWIDGET(GLDrawArea), &r);
	  AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h);
	  AG_GLViewSizeHint(GLDrawArea, w, h);
	  hh += h;
       } else if(DrawArea) {
	  AG_SizeAlloc a;
//	  AG_ObjectLock(AGOBJECT(GLDrawArea));
	  a.w = w;
	  a.h = h;
	  a.x = 0;
	  a.y = 0;
	  AG_ObjectLock(AGOBJECT(DrawArea));
	  AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
	  AG_WidgetSetSize(AGWIDGET(DrawArea), w, h);
//	  AG_GLViewSizeHint(DrawArea, w, h);
	  AG_ObjectUnlock(AGOBJECT(DrawArea));
	  hh += h;
       }

   
       if(pStatusBar){
//	  AG_WidgetHide(pStatusBar);
//	  AG_ObjectDetach(AGOBJECT(pStatusBar));
//        pStatusBar = AG_BoxNewHoriz(AGWIDGET(MainWindow), AG_BOX_HFILL);

	  float hhh;
	  if(DrawArea != NULL) {
	       hhh = (float)(AGWIDGET(DrawArea)->h);
	  }
#ifdef USE_OPENGL
	  else if(GLDrawArea != NULL) {
	       hhh = (float)(AGWIDGET(GLDrawArea)->h);
	  }
#endif	  
	  AG_WidgetSetSize(pStatusBar, w, hhh);
          ResizeStatus(AGWIDGET(pStatusBar), w, hhh , hh);
	  hh = hh + AGWIDGET(pStatusBar)->h;
       }
   
      if(MainWindow) AG_WindowSetGeometry(MainWindow, 0, 0, w, hh);
      if(MenuBar) {
	AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
        AG_WidgetEnable(AGWIDGET(MenuBar));
        AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
//        hh = hh + MenuBar->wid.h;
//    AG_WidgetFocus(AGWIDGET(MenuBar));
	}

    printf("Resize to %d x %d\n", ww, hh );
#ifdef USE_OPENGL
//	if(AG_UsingGL(NULL)) {
        if((GLDrawArea != NULL) && MainWindow) {
	   AG_ResizeDisplay(AGWIDGET(MainWindow)->w + 10, AGWIDGET(MainWindow)->h + 10);
	}
   
//	}
#endif /* USE_OPENGL */
    AG_ObjectUnlock(AGOBJECT(drv));
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

        if((w < 100) || (h < 100)) return;
   return;
	if(DrawArea != NULL) {
        AG_SizeAlloc a;
        AG_SizeReq r;
        AG_Window *win;

        win = AG_ParentWindow(DrawArea);
        if(win == NULL) return;
        AG_WidgetSizeReq(AGWIDGET(win), &r);
        hh = h - (STAT_HEIGHT * h * 2) / 800;
        ww = w;
        if(MenuBar){
            hh = hh - MenuBar->wid.h;
            AG_WidgetSetSize(MenuBar, w, MenuBar->wid.h);
        }
       if(pStatusBar){
//	  AG_WidgetHide(pStatusBar);
//	  AG_ObjectDetach(AGOBJECT(pStatusBar));
//          pStatusBar = AG_BoxNewHoriz(AGWIDGET(MainWindow), AG_BOX_HFILL);
	  AG_WidgetSetSize(pStatusBar, w, (STAT_HEIGHT * hh * 2) / 800 + 5);
          ResizeStatus(AGWIDGET(pStatusBar), w, (STAT_HEIGHT * hh * 2) / 800, MenuBar->wid.h);
	  hh = hh - AGWIDGET(pStatusBar)->h;
       }

	    a.x = 0;
	    a.y = 0;
	    a.w = ww;
	    a.h = hh;
	   AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
        if(pStatusBar != NULL) {
	   AG_WidgetSetPosition(AGWIDGET(DrawArea), 0, AGWIDGET(pStatusBar)->h);
	} else {
	   AG_WidgetSetPosition(AGWIDGET(DrawArea), 0, 0);
	}
	   AG_WidgetSetSize(DrawArea, ww, hh);
       if(MainWindow) {
           AG_Redraw(AGWIDGET(MainWindow));
       }
       }

#ifdef USE_OPENGL
   if(GLDrawArea != NULL) {
        AG_SizeReq r;

        hh = h - (STAT_HEIGHT * h * 2) / 800;
        ww = w;
        if(MenuBar){
            hh = hh - MenuBar->wid.h;
            AG_WidgetSetSize(MenuBar, w, MenuBar->wid.h);
        }
       if(pStatusBar){
//	  AG_WidgetHide(pStatusBar);
//	  AG_ObjectDetach(AGOBJECT(pStatusBar));
//          pStatusBar = AG_BoxNewHoriz(AGWIDGET(MainWindow), AG_BOX_HFILL);
	  AG_WidgetSetSize(pStatusBar, w, (STAT_HEIGHT * hh * 2) / 800 + 5);
          ResizeStatus(AGWIDGET(pStatusBar), w, (STAT_HEIGHT * hh * 2) / 800, MenuBar->wid.h);
	  hh = hh - AGWIDGET(pStatusBar)->h;
       }
        //if(MainWindow) {
        //    AG_WindowSetGeometry(MainWindow, 0, 0, ww, hh);
        //    AG_Redraw(AGWIDGET(MainWindow));
        //    AG_WindowFocus(MainWindow);
        //}
        AG_GLViewSizeHint(GLDrawArea, ww, hh);
        if(pStatusBar != NULL) {
	   AG_WidgetSetPosition(AGWIDGET(GLDrawArea), 0, AGWIDGET(pStatusBar)->h);
	} else {
	   AG_WidgetSetPosition(AGWIDGET(GLDrawArea), 0, 0);
	}
        AG_WidgetSetSize(AGWIDGET(GLDrawArea), ww, hh);
	}
	nDrawWidth = w;
	nDrawHeight = hh;
#else
   {
        if(pStatusBar != NULL) {
	   int hhh;
	   hhh = hh + AGWIDGET(pStatusBar)->h;
	   AG_ResizeDisplay(ww, hhh);
	} else {
	   AG_ResizeDisplay(ww, hh);
	}


   }
   nDrawWidth = w;
   nDrawHeight = hh;

#endif /* USE_OPENGL */
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


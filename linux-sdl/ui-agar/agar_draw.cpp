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

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"
#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
#endif // _USE_OPENCL

extern BYTE bMode;

Uint32 nDrawTick1E;
static BYTE oldBMode;
extern BOOL   bResizeGUIFlag;

extern void ResizeStatus(AG_Widget *parent, int w, int h, int y);

extern "C" {
XM7_SDLView *DrawArea;
//#ifdef _USE_OPENCL
BOOL bUseOpenCL;
//#endif   
SDL_Surface *DrawSurface = NULL;
}

void InitGL(int w, int h)
{
    AG_Driver *drv;

    SDL_SemWait(DrawInitSem);
#ifdef USE_OPENGL
   if(AG_UsingGL(NULL)) {
        InitGL_AG2(w, h);
    } else {
        InitNonGL(w, h);
//        bUseOpenCL = FALSE;
    }
#else
   InitNonGL(w, h);
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
//   DrawSurface = SDL_SetVideoMode(w, h, 24, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
//   AG_InitVideoSDL (->s, AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF | AG_VIDEO_RESIZABLE);
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
#ifdef USE_OPENGL
      if(GLDrawArea) {
	  AG_SizeAlloc a;
	  a.w = w;
	  a.h = h;
	  a.x = 0;
	  a.y = 0;
	  AG_WidgetSizeAlloc(AGWIDGET(GLDrawArea), &a);
	  AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h);
	  AG_GLViewSizeHint(GLDrawArea, w, h);
	  hh += h;
       } else 
#endif
       if(DrawArea) {
	  AG_SizeAlloc a;
	  a.w = w;
	  a.h = h;
	  a.x = 0;
	  a.y = 0;
	  AG_ObjectLock(AGOBJECT(DrawArea));
	  AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
	  AG_WidgetSetSize(AGWIDGET(DrawArea), w, h);
	  AG_ObjectUnlock(AGOBJECT(DrawArea));
	  hh += AGWIDGET(DrawArea)->h;
       }

       if(pStatusBar){
	  float hhh;
	  hhh = ((float)h / 800.0) * (float)STAT_HEIGHT + 2.0; 
	  AG_WidgetSetSize(pStatusBar, w, (int)hhh);
          ResizeStatus(AGWIDGET(pStatusBar), w, (int)hhh, h);
//	  hh = hh + AGWIDGET(pStatusBar)->h;
	  hh = hh + hhh + 25;
       }
      if(MenuBar) {
	AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
        AG_WidgetEnable(AGWIDGET(MenuBar));
        AG_WidgetSetSize(AGWIDGET(MenuBar), w, MenuBar->wid.h);
        hh = hh + MenuBar->wid.h;
//    AG_WidgetFocus(AGWIDGET(MenuBar));
	}

//    if(MainWindow) AG_WindowSetGeometry(MainWindow, 0, 0, w, hh);

    printf("Resize to %d x %d\n", ww, hh );
     if(agDriverSw && (AG_UsingGL(NULL) != 0)) {
	AG_ResizeDisplay(w + 10, hh + 10);
     } else {
#if 0
	if((DrawArea != NULL)  && agDriverSw) {
	   AG_ResizeDisplay(w + 10, hh + 10);
	}
#endif
     }

   AG_ObjectUnlock(AGOBJECT(drv));
}

/*
* Resize (Window only)
*/
void ResizeWindow_Agar2(int w, int h)
{
   int hh;
   int ww;
   AG_Driver *drv;
//   if(agDriverSw) {
//	if((AGWIDGET(MainWindow)->w < w) || (AGWIDGET(MainWindow)->h < h)) {
//	   AG_ResizeDisplay(AGWIDGET(MainWindow)->w + 10, AGWIDGET(MainWindow)->h + 30);
//	}
  // }
   
   if(MenuBar != NULL) {
      AG_Redraw(AGWIDGET(MenuBar));
   }
   if(pStatusBar != NULL) {
      AG_Redraw(AGWIDGET(pStatusBar));
   }
   
   //     printf("Resize2 to %d x %d\n", w, h);
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


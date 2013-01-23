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
static BOOL bManualScaled;

extern BOOL   bResizeGUIFlag;
extern SDL_semaphore *DrawInitSem;

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
    bManualScaled = FALSE;
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
   AG_SetVideoResizeCallback(ResizeWindow_Agar);

   SDL_SemPost(DrawInitSem);
}

void InitNonGL(int w, int h)
{
   char *ext;

   if(InitVideo) return;
   InitVideo = TRUE;

   vram_pb = NULL;
   vram_pg = NULL;
   vram_pr = NULL;

//   DrawSurface = SDL_SetVideoMode(w, h, 24, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
//   AG_InitVideoSDL (screen, AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF | AG_VIDEO_RESIZABLE);
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
   float hhh;
   
   AG_Driver *drv;
   if((w < 100) || (h < 100)) return;
   if((bManualScaled == TRUE)){
	bManualScaled = FALSE;
	return;
   }
   bManualScaled = FALSE;

   if(agDriverSw) {
      drv = &agDriverSw->_inherit;
   } else {
      if(MainWindow == NULL) return;
      drv = AGDRIVER(MainWindow);
   }

   nDrawWidth = w;
   nDrawHeight = h;
   AG_ObjectLock(AGOBJECT(drv));
   ww = w;
   hh = h;
   LockVram();
   if(MainWindow) AG_WindowSetGeometry(MainWindow, 0, 0, w, h);
   
   if(MenuBar != NULL) {
      AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
      AG_WidgetEnable(AGWIDGET(MenuBar));
      AG_WidgetSetSize(AGWIDGET(MenuBar), w, AGWIDGET(MenuBar)->h);
      hh = hh - AGWIDGET(MenuBar)->h;
   }
   if(pStatusBar != NULL){
      if((w < 640) || (hh < 400)) {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 4.0;
      } else {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 2.0;
      }
      
      
      AG_WidgetSetSize(pStatusBar, w, (int)hhh);
      hh = hh - AGWIDGET(pStatusBar)->h;
      ResizeStatus(AGWIDGET(pStatusBar), w, hh, hh);
   }

   
#ifdef USE_OPENGL
   if(GLDrawArea != NULL) {
      AG_SizeAlloc a;
      a.w = w;
      a.h = hh;
      a.x = 0;
      a.y = 0;
      AG_ObjectLock(AGOBJECT(GLDrawArea));
      AG_WidgetSizeAlloc(AGWIDGET(GLDrawArea), &a);
      AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, hh);
      AG_GLViewSizeHint(GLDrawArea, w, hh);
      AG_ObjectUnlock(AGOBJECT(GLDrawArea));
   } else 
#endif
     if(DrawArea != NULL) {
	AG_SizeAlloc a;
	a.w = w;
	a.h = hh;
	a.x = 0;
	a.y = 0;
	
	AG_ObjectLock(AGOBJECT(DrawArea));
	AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
	AG_WidgetSetSize(AGWIDGET(DrawArea), w, hh);
	AG_ObjectUnlock(AGOBJECT(DrawArea));
       }
   

   printf("Resize to %d x %d ( %d x %d)\n", w, h, ww, hh );
   UnlockVram();

   AG_ObjectUnlock(AGOBJECT(drv));
}

/*
* Resize (Window only)
*/
void ResizeWindow_Agar2(int w, int h)
{
   int hh = h;
   int ww = w;
   float hhh;
   AG_Driver *drv;
   
   bManualScaled = TRUE;
   if(MenuBar != NULL) {
      AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
      AG_WidgetEnable(AGWIDGET(MenuBar));
      AG_WidgetSetSize(AGWIDGET(MenuBar), w, AGWIDGET(MenuBar)->h);
      hh = hh + AGWIDGET(MenuBar)->h;
   }
   if(pStatusBar != NULL){
      if((w < 640) || (h < 400)) {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 4.0;
      } else {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 2.0;
      }
      AG_WidgetSetSize(pStatusBar, w, (int)hhh);
      ResizeStatus(AGWIDGET(pStatusBar), w, h, hh);
      hh = hh + AGWIDGET(pStatusBar)->h;
   }
#ifdef USE_OPENGL
   if(GLDrawArea != NULL) {
      AG_SizeAlloc a;
      a.w = w;
      a.h = h;
      a.x = 0;
      a.y = 0;
      AG_ObjectLock(AGOBJECT(GLDrawArea));
      AG_WidgetSizeAlloc(AGWIDGET(GLDrawArea), &a);
      AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h);
      AG_GLViewSizeHint(GLDrawArea, w, h);
      AG_ObjectUnlock(AGOBJECT(GLDrawArea));
      } else 
#endif
   if(DrawArea != NULL) {
	AG_SizeAlloc a;
	a.w = w;
	a.h = h;
	a.x = 0;
	a.y = 0;
	
	AG_ObjectLock(AGOBJECT(DrawArea));
	AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
	AG_WidgetSetSize(AGWIDGET(DrawArea), w, h);
	AG_ObjectUnlock(AGOBJECT(DrawArea));
      }
   hh = hh + 15; // Add Pad.
   if(MainWindow) AG_WindowSetGeometry(MainWindow, 0, 0, w, hh);
   if((agDriverSw) && (hh != h)){
      if(DrawArea != NULL) {
	 LockVram();
	 AG_ResizeDisplay(ww, hh);
	 UnlockVram();
      } else {
	 AG_ResizeDisplay(ww, hh);
      }
      
   }

   
   printf("Resize2 to %d x %d\n", ww, hh);
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


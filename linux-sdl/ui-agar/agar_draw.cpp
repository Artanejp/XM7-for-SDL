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
#include "agar_cfg.h"
#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
#endif // _USE_OPENCL
extern BYTE bMode;

extern "C"{
   extern DWORD XM7_timeGetTime(void);	/* timeGetTime互換関数 */
   extern void  XM7_Sleep(DWORD t);	/* Sleep互換関数 */
}


Uint32 nDrawTick1E;
static BYTE oldBMode;
static BOOL bManualScaled;

extern BOOL   bResizeGUIFlag;
extern SDL_semaphore *DrawInitSem;

extern void ResizeStatus(AG_Widget *parent, int w, int h, int y);

extern "C" {
XM7_SDLView *DrawArea;
//AG_Pixmap *DrawArea;
BOOL bUseOpenCL;
BOOL bUseSIMD;
AG_Surface *DrawSurface = NULL;
int DrawSurfaceId = 0;
unsigned int nRenderMethod;
}

void InitGL(int w, int h)
{
    AG_Driver *drv;
    if(MainWindow) drv = AGDRIVER(MainWindow);
    bManualScaled = FALSE;
//    nRenderMethod = 0;
    SDL_SemWait(DrawInitSem);
#ifdef USE_OPENGL
   if(AG_UsingGL(NULL)) {
      InitGL_AG2(w, h);
    } else {
        InitNonGL(w, h);
        bUseOpenCL = FALSE;
    }
#else
   InitNonGL(w, h);
#endif
   AG_SetVideoResizeCallback(ResizeWindow_Agar);
   bUseSIMD = FALSE;
   SDL_SemPost(DrawInitSem);
}

void InitNonGL(int w, int h)
{
   char *ext;
   AG_PixelFormat *fmt;
   if(InitVideo) return;
   InitVideo = TRUE;

   vram_pb = NULL;
   vram_pg = NULL;
   vram_pr = NULL;

   InitVramSemaphore();
   pVram2 = NULL;
   
   InitVirtualVram();
   return;
}


void DetachDrawArea(void)
{
    if(CheckVramSemaphore) DetachVramSemaphore();
    if(DrawArea != NULL) AG_ObjectDetach(DrawArea);
//    if(DrawSurface != NULL) AG_SurfaceFree(DrawSurface);
}


extern "C" 
{
   
   AG_Surface *GetDrawSurface(void)
     {
	return DrawSurface;
     }
}

void ResizeWindow_Agar(int w, int h)
{
   int hh;
   int ww;
   float hhh;
   
   AG_Driver *drv;
   if((w < 100) || (h < 100)) return;

   if(agDriverSw) {
      if(MainWindow == NULL) return;
      drv = &agDriverSw->_inherit;
   } else {
      if(MainWindow == NULL) return;
      drv = AGDRIVER(MainWindow);
   }
   if((bManualScaled == TRUE) && (agDriverSw)){
	bManualScaled = FALSE;
	return;
   }
   bManualScaled = FALSE;

   AG_ObjectLock(AGOBJECT(drv));
   ww = w;

   LockVram();
   if(MainWindow) {
      AG_WindowSetGeometry(MainWindow, 0, 0, w, h);
      hh = AGWIDGET(MainWindow)->h - 20;
      ww = AGWIDGET(MainWindow)->w;
   } else {
      hh = h;
      ww = w;
   }
   
   
   if(MenuBar != NULL) {
      AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
      AG_WidgetEnable(AGWIDGET(MenuBar));
      AG_WidgetSetSize(AGWIDGET(MenuBar), w, AGWIDGET(MenuBar)->h);
      hh = hh - AGWIDGET(MenuBar)->h;
   }
   if(pStatusBar != NULL){
      if((w < 640) || (hh < 400)) {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 1.0;
      } else {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 1.0;
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

//	if(DrawSurface != NULL) {
//	   AG_SurfaceResize(DrawSurface, w, hh);
//	}
	
	AG_ObjectLock(AGOBJECT(DrawArea));
	AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
	AG_WidgetSetSize(AGWIDGET(DrawArea), w, hh);
	AG_ObjectUnlock(AGOBJECT(DrawArea));
     }
   
   nDrawWidth = w;
   nDrawHeight = hh;

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
   nDrawWidth = w;
   nDrawHeight = h;
   if(MenuBar != NULL) {
      AG_MenuSetPadding(MenuBar, 0 , 0, 0, 0);
      AG_WidgetEnable(AGWIDGET(MenuBar));
      AG_WidgetSetSize(AGWIDGET(MenuBar), w, AGWIDGET(MenuBar)->h);
      hh = hh + AGWIDGET(MenuBar)->h;
   }
   if(pStatusBar != NULL){
      if((w < 640) || (h < 400)) {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 1.0;
      } else {
	 hhh = ((float)w / 640.0) * (float)STAT_HEIGHT * 1.0;
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

        LockVram();
//	if(DrawSurface != NULL) {
//	   AG_SurfaceResize(DrawSurface, w, h);
//	}
#if 1
	AG_ObjectLock(AGOBJECT(DrawArea));
	AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
	AG_WidgetSetSize(AGWIDGET(DrawArea), w, h);
	AG_ObjectUnlock(AGOBJECT(DrawArea));
//	AG_PixmapUpdateCurrentSurface(DrawArea);
#else
      
#endif
        UnlockVram();
      }
   hh = hh + 20; // Add Pad.
   if(DrawArea != NULL) {
	 LockVram();
	 //AG_ResizeDisplay(ww, hh);
	 UnlockVram();
   } else {
      AG_ResizeDisplay(ww, hh);
      if(MainWindow) AG_WindowSetGeometry(MainWindow, 0, 0, w, hh);
   }
  
   printf("Resize2 to %d x %d\n", w, h);
}

//extern void XM7_SDLViewUpdateSrc(AG_Pixmap *my, void *Fn);
void AGDrawTaskMain(void)
{
	Uint32 nDrawTick2E;
	Uint32 fps;
        
	if(nEmuFPS > 2) {
		fps = 1000 / nEmuFPS;

	} else {
		fps = 500;
	}
	nDrawTick2E = XM7_timeGetTime();
	if(nDrawTick1E > nDrawTick2E) {
		nDrawTick1E = 0;
	}
	if(((nDrawTick2E - nDrawTick1E)<fps) && (bMode == oldBMode)) return;
  
	nDrawTick1E = nDrawTick2E;
	oldBMode = bMode;

        SelectDraw2();
        if(nRenderMethod == RENDERING_RASTER) return;
#ifdef _USE_OPENCL
       if((cldraw != NULL) && bGL_PIXEL_UNPACK_BUFFER_BINDING) return; // OK?
#endif	   
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


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
#include "agar_logger.h"

#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
#endif // _USE_OPENCL
extern BYTE bMode;
extern BYTE bModeOld;

extern "C"{
   extern DWORD XM7_timeGetTime(void);	/* timeGetTime互換関数 */
   extern void  XM7_Sleep(DWORD t);	/* Sleep互換関数 */
}


Uint32 nDrawTick1E;
static BOOL bManualScaled;

extern BOOL   bResizeGUIFlag;
extern SDL_semaphore *DrawInitSem;

extern void ResizeStatus(AG_Widget *parent, int w, int h, int y);

extern "C" {
XM7_SDLView *DrawArea;
BOOL bUseOpenCL;
BOOL bUseSIMD;
AG_Surface *DrawSurface = NULL;
int DrawSurfaceId = 0;
unsigned int nRenderMethod;
}

extern "C" {
  BOOL IsUsingCL(void)
  {
#ifdef _USE_OPENCL    
    if((cldraw != NULL) && (bCLEnabled != FALSE)) return TRUE;
#endif
    return FALSE;
  }
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
   if(DrawArea != NULL) return; // Temporally workaround.

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
   if(MainWindow){
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
      AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, hh);
      AG_WidgetSizeAlloc(AGWIDGET(GLDrawArea), &a);
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
   int ww = (w * 102) / 100;
   float hhh;
   AG_Driver *drv;
   
   
	
   bManualScaled = TRUE;
   nDrawWidth = w;
   nDrawHeight = h;
   LockVram();
   AG_SetVideoResizeCallback(NULL);
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
      AG_WidgetSetSize(AGWIDGET(GLDrawArea), w, h);
      AG_WidgetSizeAlloc(AGWIDGET(GLDrawArea), &a);
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
	AG_WidgetSetSize(AGWIDGET(DrawArea), ww, h);
	AG_WidgetSizeAlloc(AGWIDGET(DrawArea), &a);
        AG_ObjectUnlock(AGOBJECT(DrawArea));
      }
   hh = hh + 20; // Add Pad.
   if(DrawArea != NULL) {
//	 LockVram();
	 AG_ResizeDisplay((ww * 102)/ 100, (hh * 102) / 100);
         if(MainWindow) AG_WindowSetGeometry(MainWindow, 0, 0, (ww * 102)/ 100, hh);
//	 UnlockVram();
   } else {
      AG_ResizeDisplay((ww * 102)/ 100, (hh * 102) / 100);
      if(MainWindow) AG_WindowSetGeometry(MainWindow, 0, 0, ww, hh);
   }
//   AG_SetVideoResizeCallback(ResizeWindow_Agar);
   UnlockVram();
   XM7_DebugLog(XM7_LOG_DEBUG, "Resize2 to %d x %d", w, h);
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
	nDrawTick2E = XM7_timeGetTime();
	if(nDrawTick1E > nDrawTick2E) {
		nDrawTick1E = 0;
	}
	if(((nDrawTick2E - nDrawTick1E) < fps) && SelectCheck()) return;
  
	if(!SelectCheck()) {
	  //bNextFrameRender = TRUE;
	   bClearFlag = TRUE;
	   SelectDraw2();
	   if((nRenderMethod == RENDERING_RASTER) || (bCLEnabled)){
	      SetDirtyFlag(0, 400, TRUE);
	   } else {
	      SetDrawFlag(TRUE);
	   }
	}
   
        nDrawTick1E = nDrawTick2E;
#ifdef _USE_OPENCL
        if(bCLEnabled) return;
#endif
        if(nRenderMethod == RENDERING_RASTER) return;
#if XM7_VER >= 3
	switch (bModeOld) {
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


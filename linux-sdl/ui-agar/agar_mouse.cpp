/*
 * agar_mouse.cpp
 *
 *  Created on: 2012/12/22
 *  Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *  Mouse event handler.
 *  Connect Inteligent-Mouse to agar.
 */
#include <SDL/SDL.h>
#include <libintl.h>

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
#include <agar/gui/cursors.h>

#include "xm7.h"
#include "fdc.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#include "agar_toolbox.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"
#include "agar_osd.h"
#include "agar_logger.h"
#else
#include "xm7_sdl.h"
#include "sdl_cfg.h"
#endif

#include "api_mouse.h"
#include "sdl_inifile.h"
#include "api_draw.h"
#include "sdl_cpuid.h"


int nMouseX;
int nMouseY;
unsigned int nMouseButton;
static int nMouseOldX;
static int nMouseOldY;
static int nPhysicalX;
static int nPhysicalY;
static int nMouseSX;	/* マウス X座標保存 */
static int nMouseSY;	/* マウス Y座標保存 */
static BOOL bMouseCursor;	/* マウス カーソル表示状態 */
static Uint8 pMouseCursor[8]; 
static Uint8 pMouseMask[8]; 
static AG_Cursor *pCursor = NULL;

static void CalcMouseMove(int w, int h, int x, int y)
{
    if((x < 0) || (x >= w)) return; // Out of bounce
    if((y < 0) || (y >= h)) return; // Out of bounce
    nPhysicalX = (x * 640) / w - 320;
    nPhysicalY = (y * 400) / h - 200;
    nMouseX -= (nPhysicalX - nMouseOldX);
    nMouseY -= (nPhysicalY - nMouseOldY); 
    nMouseOldX = nPhysicalX;
    nMouseOldY = nPhysicalY;
//    XM7_DebugLog(XM7_LOG_DEBUG, "Mouse: %d %d", nMouseX, nMouseY);
}


void InitMouseSub(void)
{
   AG_Driver *drv = NULL;
   nMouseOldX = 0;
   nMouseOldY = 0;
   nPhysicalX = 0;
   nPhysicalY = 0;
   bMouseCursor = TRUE;
   
   memset(pMouseCursor, 0x00, sizeof(pMouseCursor));
   memset(pMouseCursor, 0x00, sizeof(pMouseMask));
#ifdef USE_OPENGL
   if(GLDrawArea != NULL) {
      drv = AGDRIVER(GLDrawArea);
   } else
#endif
          if (DrawArea != NULL) {
      drv = AGDRIVER(DrawArea);
   }
   if(drv != NULL) {
//      pCursor = AG_CursorNew(drv, 8, 8, (Uint8 *)pMouseCursor, (Uint8 *)pMouseMask, 1, 1);
   }
   
}

void DetachMouseSub(void)
{
}

void GetMousePos(int *x, int *y)
{
    *x = nPhysicalX;
    *y = nPhysicalY;
}

extern "C" 
{


/*
 *  マウス キャプチャ状態設定
 */
void SetMouseCapture(BOOL en)
{
	int   x;
	int   y;
	//SDL_SysWMinfo sdlinfo;
	/*
	 * キャプチャ停止中/VM停止中/非アクティブ時は強制的に無効
	 */
	if (!mos_capture || stopreq_flag || !run_flag || !bActivate) {
		en = FALSE;
	}

	/*
	 * カーソル表示/消去
	 */
	if (bMouseCursor == en) {
	   if (!en) {

#ifdef USE_OPENGL
		   if(GLDrawArea != NULL) {
		      AG_HideCursor(AGDRIVER(GLDrawArea));
		   } else
#endif
		     if (DrawArea != NULL) {
		      AG_HideCursor(AGDRIVER(DrawArea));
		   }
		} else {
#ifdef USE_OPENGL
		   if(GLDrawArea != NULL) {
		      AG_ShowCursor(AGDRIVER(GLDrawArea));
		   } else 
#endif
		     if (DrawArea != NULL) {
		      AG_ShowCursor(AGDRIVER(DrawArea));
		   }
		}
	   
	   bMouseCursor = !en;
	}

	/*
	 * キャプチャ状態に変化がなければ帰る
	 */
	if (bMouseCaptureFlag == en) {
		return;
	}
	if (en) {

		/*
		 * カーソル位置を保存
		 */
		// gtk_widget_get_pointer(drawArea, &x, &y);
		GetMousePos(&x, &y);
		nMouseSX = x;
		nMouseSY = y;

		/*
		 * 描画ウィンドウのサイズを求める
		 */
		// gdk_window_get_size(drawArea->window, &nDAreaW, &nDAreaH);

		/*
		 * 中心座標を求める
		 */
		/*
		 * カーソルをウィンドウ中央に固定
		 */
#ifdef USE_OPENGL
	        if(GLDrawArea != NULL) {
		    x = GLDrawArea->wid.w / 2;
		    y = GLDrawArea->wid.h / 2;
		   AG_ExecMouseAction (GLDrawArea, AG_ACTION_ON_BUTTONUP, AG_MOUSE_NONE, x, y);
		if(AG_UsingSDL(AGDRIVER(GLDrawArea))) SDL_WM_GrabInput(SDL_GRAB_ON);
		} else
#endif
	        if(DrawArea != NULL) {
		    x = AGWIDGET(DrawArea)->w / 2;
		    y = AGWIDGET(DrawArea)->h / 2;
		   AG_ExecMouseAction (DrawArea, AG_ACTION_ON_BUTTONUP, AG_MOUSE_NONE, x, y);
		   if(AG_UsingSDL(AGDRIVER(DrawArea))) SDL_WM_GrabInput(SDL_GRAB_ON);
		}
	}
	else {

		/*
		 * クリップ解除
		 */
		//gdk_pointer_ungrab(GDK_CURRENT_TIME);
		//        if(SDL_GetWMInfo(&sdlinfo)) {
		//        XUngrabPointer(sdlinfo.info.x11.display, CurrentTime);
		//}
		/*
		 * カーソル位置を復元
		 */
#ifdef USE_OPENGL
	       if(GLDrawArea != NULL) {
		    x = nMouseSX;
		    y = nMouseSY;
		   AG_ExecMouseAction (GLDrawArea, AG_ACTION_ON_BUTTONUP, AG_MOUSE_NONE, x, y);
		if(AG_UsingSDL(AGDRIVER(GLDrawArea))) SDL_WM_GrabInput(SDL_GRAB_OFF);
		} else 
#endif
	        if(DrawArea != NULL) {
		    x = nMouseSX;
		    y = nMouseSY;
		   AG_ExecMouseAction (DrawArea, AG_ACTION_ON_BUTTONUP, AG_MOUSE_NONE, x, y);
		if(AG_UsingSDL(AGDRIVER(DrawArea))) SDL_WM_GrabInput(SDL_GRAB_OFF);
		}

	}

	/*
	 * キャプチャ状態を保存
	 */
	bMouseCaptureFlag = en;

	/*
	 * マウス移動距離をクリア
	 */
	nMouseX = 0;
	nMouseY = 0;
}


void OnMouseMotionSDL(AG_Event *event)
{
    XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
    int x = AG_INT(1);
    int y = AG_INT(2);
    int w;
    int h;
    AG_Surface *su;
   
    if(my == NULL) return;
    if((bMouseCaptureFlag != TRUE) || (mos_capture != TRUE)) return;
    su = AGWIDGET_SURFACE(my, my->mySurface);
    w = su->w;
    h = su->h;
    CalcMouseMove(w, h, x, y);
}

void OnMouseButtonDownSDL(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   int button = AG_INT(1);
   int x = AG_INT(2);
   int y = AG_INT(3);
   int w;
   int h;
   AG_Surface *su;
   if(my == NULL) return;
   if((bMouseCaptureFlag != TRUE) || (mos_capture != TRUE)) return;
   su = AGWIDGET_SURFACE(my, my->mySurface);
   w = su->w;
   h = su->h;
   CalcMouseMove(w, h, x, y);
   if(button == AG_MOUSE_LEFT) {
	nMouseButton &= ~0x10;
   }
   if(button == AG_MOUSE_RIGHT) {
	nMouseButton &= ~0x20;
   }

}

void OnMouseButtonUpSDL(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   int button = AG_INT(1);
   int x = AG_INT(2);
   int y = AG_INT(3);
   int w;
   int h;
   AG_Surface *su;
   if(my == NULL) return;

   if((bMouseCaptureFlag != TRUE) || (mos_capture != TRUE)) return;
   su = AGWIDGET_SURFACE(my, my->mySurface);
   w = su->w;
   h = su->h;
   CalcMouseMove(w, h, x, y);
   if(button == AG_MOUSE_LEFT) {
	nMouseButton |= 0x10;
   }
   if(button == AG_MOUSE_RIGHT) {
	nMouseButton |= 0x20;
   }

}

void OnMouseMotionGL(AG_Event *event)
{
    AG_GLView *my = (AG_GLView *)AG_SELF();
    int x = AG_INT(1);
    int y = AG_INT(2);
    int w;
    int h;
    if(my == NULL) return;
    if((bMouseCaptureFlag != TRUE) || (mos_capture != TRUE)) return;
    w = my->wid.w;
    h = my->wid.h;
    CalcMouseMove(w, h, x, y);
}

void OnMouseButtonDownGL(AG_Event *event)
{
   AG_GLView *my = (AG_GLView *)AG_SELF();
   int button = AG_INT(1);
   int x = AG_INT(2);
   int y = AG_INT(3);
   int w;
   int h;
   if(my == NULL) return;

   if((bMouseCaptureFlag != TRUE) || (mos_capture != TRUE)) return;
    w = my->wid.w;
    h = my->wid.h;
   CalcMouseMove(w, h, x, y);
   if(button == AG_MOUSE_LEFT) {
	nMouseButton &= ~0x10;
   }
   if(button == AG_MOUSE_RIGHT) {
	nMouseButton &= ~0x20;
   }

}

void OnMouseButtonUpGL(AG_Event *event)
{
   AG_GLView *my = (AG_GLView *)AG_SELF();
   int button = AG_INT(1);
   int x = AG_INT(2);
   int y = AG_INT(3);
   int w;
   int h;
   if(my == NULL) return;

   if((bMouseCaptureFlag != TRUE) || (mos_capture != TRUE)) return;
   w = my->wid.w;
   h = my->wid.h;
   CalcMouseMove(w, h, x, y);
   if(button == AG_MOUSE_LEFT) {
	nMouseButton |= 0x10;
   }
   if(button == AG_MOUSE_RIGHT) {
	nMouseButton |= 0x20;
   }

}

}

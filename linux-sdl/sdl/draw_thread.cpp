/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2003 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta (whatisthis.sowhat@gmail.com)
 * Threaded Draw Routine
 *  History:
 *   20111009 : Separate from "api_draw.cpp"
 */

 #ifdef USE_GTK
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#endif

#include <SDL.h>
#ifdef USE_AGAR
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#else
#include <SDL_syswm.h>
#ifdef USE_OPENGL
#include <SDL_opengl.h>
#endif
#endif

#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#endif
#include "sdl.h"

#include "api_vram.h"
#include "api_draw.h"
#include "api_scaler.h"

#ifdef USE_AGAR
extern AG_Window *MainWindow;
#endif

#ifdef USE_AGAR
extern void AG_DrawInitsub(void);
extern void AG_DrawDetachsub(void);
#else
extern void SDL_DrawInitsub(void);
extern void SDL_DrawDetachsub(void);
#endif

extern Uint32 nDrawTick1D;
extern Uint32 nDrawTick1E;
extern GLuint uVramTextureID;
extern BOOL   bOldFullScan;
extern BOOL DrawINGFlag;
extern BOOL DrawSHUTDOWN;
extern BOOL DrawWaitFlag;

/*
 * ビデオドライバ関連
 */
extern EmuGrphLib *vramhdr;
extern EmuGrph400l *vramhdr_400l;
extern EmuGrph4096c *vramhdr_4096;
extern EmuGrph256kc *vramhdr_256k;

/*
 * Local Variables
 */
static WORD nDrawCount;


/*
 * 描画は別スレッドで行う
 */
static WORD DrawCountSet(WORD fps)
{
	DWORD intr;
	DWORD wait;

#if XM7_VER >= 3
	if (screen_mode == SCR_400LINE) {
#else
		if (enable_400line && enable_400linecard) {
#endif
			/*
			 * 400ライン(24kHzモード) 0.98ms + 16.4ms
			 */
			intr = 980 + 16400;
		} else {
			/*
			 * 200ライン(15kHzモード) 1.91ms + 12.7ms
			 */
			intr = 1910 + 12700;
		}
		if(fps<= 0) fps=1;
		wait = 1000000 / (DWORD)fps ;
		wait = wait / intr + 1; /* 整数化 */
		return (WORD) wait;
}

/*
 *  セレクトチェック
 */
static BOOL SelectCheck(void)
{

#if XM7_VER >= 3
	/*
	 * 限りない手抜き(ォ
	 */
	if (bMode == screen_mode) {
		return TRUE;
	} else {
		return FALSE;
	}
#else				/*  */
/*
 * 320x200
 */
	if (mode320) {
		if (bAnalog) {
			return TRUE;
		} else {
			return FALSE;
		}
	}

	/*
	 * 640x200
	 */
	if (!bAnalog) {
		return TRUE;
	} else {
		return FALSE;
	}
#endif /*  */
}

/*
 *  セレクト(内部向き)
 */
BOOL SelectDraw2(void)
{
#ifdef USE_AGAR
        AG_Widget *wid;
        AG_Color nullcolor;
        AG_Rect rect;
        SDL_Surface *p;
#else // SDL
		Uint32 nullcolor;
		SDL_Surface *p;
		SDL_Rect rect;
#endif



		/*
		 * 一致しているかチェック
		 */
		if (SelectCheck()) {
			return TRUE;
		}
#ifdef USE_AGAR
//		if(agDriverOps == NULL) return FALSE;
        if(GLDrawArea != NULL) {
		   wid = AGWIDGET(GLDrawArea);
		} else if(DrawArea != NULL) {
		   wid = AGWIDGET(DrawArea);
		} else {
		   return FALSE;
		}
		p = GetDrawSurface();
#else
		p = SDL_GetVideoSurface();
		if(p == NULL) return FALSE;
#endif
		rect.h = nDrawWidth;
		rect.w = nDrawHeight;
		rect.x = 0;
		rect.y = 0;
		if(!bUseOpenGL) {
			/*
			 * すべてクリア
			 */
#ifdef USE_AGAR
//			AG_ObjectLock(wid);
			nullcolor.r = 0;
			nullcolor.g = 0;
			nullcolor.b = 0;
			nullcolor.a = 255;
//			AG_FillRect(drv->sRef, &rect, nullcolor);
//			AG_ObjectUnlock(wid);
#else
			SDL_LockSurface(p);
			nullcolor = SDL_MapRGBA(p->format, 0, 0, 0, 255);
			SDL_FillRect(p, &rect, nullcolor);
			SDL_UnlockSurface(p);
#endif
			/*
			 * すべてクリア
			 */
#ifdef USE_AGAR
			//AG_DriverClose(drv);
			bClearFlag = TRUE;
			SDLDrawFlag.ForcaReDraw = TRUE;
#else
			if(realDrawArea != p) {
				SDL_LockSurface(realDrawArea);
				rect.h = realDrawArea->h;
				rect.w = realDrawArea->w;
				rect.x = 0;
				rect.y = 0;
				nullcolor = SDL_MapRGBA(realDrawArea->format, 0, 0, 0, 255);
				SDL_FillRect(realDrawArea, &rect, nullcolor);
				SDL_UnlockSurface(realDrawArea);
			}
#endif
		}
		bOldFullScan = bFullScan;
		/*
		 * セレクト
		 */
#if XM7_VER >= 3
		switch (screen_mode) {
		case SCR_400LINE:
			return Select400l();
		case SCR_262144:
			return Select256k();
		case SCR_4096:
			return Select320();
		default:
			return Select640();
		}

#else				/*  */
		if (mode320) {
			return Select320();
		}
		return Select640();

#endif				/*  */
		return TRUE;
}

static int DrawTaskMain(void *arg)
{
		if(newResize) {
			nDrawWidth = newDrawWidth;
			nDrawHeight = newDrawHeight;
			ResizeWindow(nDrawWidth, nDrawHeight);
//			SetupGL(nDrawWidth, nDrawHeight);
			newResize = FALSE;
		}
		ChangeResolution();
		SelectDraw2();
#if XM7_VER >= 3
		/*
		 *    いずれかを使って描画
		 */
		SDL_SemWait(DrawInitSem);
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
		SDL_SemPost(DrawInitSem);
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
		//        SDL_UnlockSurface(p);
		Flip();
		return 0;
}

#ifdef USE_AGAR
void *DrawThreadMain(void *p)
#else
int DrawThreadMain(void *p)
#endif
{
#ifdef USE_AGAR
		nDrawTick1D = AG_GetTicks();
		AG_DrawInitsub();
#else
        SDL_DrawInitsub();
#endif
//		ResizeWindow(640,480);
		InitGL(640,480);
		//nDrawCount = DrawCountSet(nDrawFPS);
        nDrawCount = (100000 / nDrawFPS) / 100 + 1;
		while(1) {
#ifndef USE_AGAR
			if(DrawMutex == NULL) {
				SDL_Delay(1);
				continue;
			}
			if(DrawCond == NULL) {
				SDL_Delay(1);
				continue;
			}
#endif
#ifdef USE_AGAR
//			AG_MutexLock(&DrawMutex);
//			AG_CondWait(&DrawCond, &DrawMutex);
		       nDrawCount = (100000 / nDrawFPS) / 100 + 1;
		       AG_Delay(nDrawCount);
#else
			SDL_mutexP(DrawMutex);
			SDL_CondWait(DrawCond, DrawMutex);
#endif
			if(DrawSHUTDOWN) {
#ifdef USE_AGAR
                AG_DrawDetachsub();
#else
                SDL_DrawDetachsub();
#endif
                DrawSHUTDOWN = FALSE;
				return 0; /* シャットダウン期間 */
			}
#ifndef USE_OPENGL
			DrawStatus();
#endif

#ifdef USE_AGAR
			//if(DrawArea == NULL) continue;
#endif
//			if(nDrawCount > 0) {
//				nDrawCount --;
//				continue;
//			} else {
			   //nDrawCount = DrawCountSet(nDrawFPS);
//			   nDrawCount = 1000 / nDrawFPS + 1;
//			}
			DrawWaitFlag = TRUE;
			DrawINGFlag = TRUE;
#ifdef USE_AGAR
			AGDrawTaskMain();
#else
			DrawTaskMain(NULL);
#endif
			DrawINGFlag = FALSE;
			DrawWaitFlag = FALSE;
			//while(DrawWaitFlag) SDL_Delay(1); /* 非表示期間 */
		}
}


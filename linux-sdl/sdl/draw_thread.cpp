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

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"

#include "agar_xm7.h"
#include "agar_cfg.h"
#include "agar_draw.h"
#ifdef USE_OPENGL
#include "agar_gldraw.h"
#endif /* USE_OPENGL */

#include "agar_glutil.h"

#include "xm7_sdl.h"

#include "api_vram.h"
#include "api_draw.h"
//#include "api_scaler.h"

extern "C" {
extern void  XM7_Sleep(DWORD init);
extern void  XM7_Sync1ms(DWORD init);
extern DWORD XM7_timeGetTime(void);
};

extern void AG_DrawInitsub(void);
extern void AG_DrawDetachsub(void);
//extern AG_Mutex DrawMutex;
//extern AG_Cond DrawCond;


extern Uint32 nDrawTick1D;
extern Uint32 nDrawTick1E;
extern GLuint uVramTextureID;
extern BOOL   bOldFullScan;
extern BOOL DrawINGFlag;
extern BOOL DrawSHUTDOWN;
extern BOOL DrawWaitFlag;

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


extern "C" {   
	
/*
 *  セレクトチェック
 */
BOOL SelectCheck(void)
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
}
   
/*
 *  セレクト(内部向き)
 */
BOOL SelectDraw2(void)
{
    AG_Widget *wid;
//		if(agDriverOps == NULL) return FALSE;
//   if((bCLEnabled) && SelectCheck()) return FALSE;
   now_raster = 0;
#ifdef USE_OPENGL
   if(GLDrawArea != NULL) {
      wid = AGWIDGET(GLDrawArea);
   } else if(DrawArea != NULL) {
      wid = AGWIDGET(DrawArea);
   } else {
      return FALSE;
   }
   if(!bUseOpenGL) {
#else
   if(DrawArea != NULL) {
      wid = AGWIDGET(DrawArea);
   } else {
      return FALSE;
   }
#endif   

      /*
       * すべてクリア
       */
      //AG_DriverClose(drv);
#ifdef USE_OPENGL
   }
#endif /* USE_OPENGL */
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

#else /*  */
   if (mode320) {
      return Select320();
   }
   return Select640();
#endif /*  */
   return TRUE;
}

void *DrawThreadMain(void *p)
{
   DWORD ticks;
   AG_DrawInitsub();

   InitGL(640,480);
   nDrawCount = DrawCountSet(nDrawFPS);
   while(1) {
     ticks = XM7_timeGetTime();
     XM7_Sync1ms(ticks);
     //AG_CondWait(&DrawCond, &DrawMutex);
      if(DrawSHUTDOWN) {
	 AG_DrawDetachsub();
	 DrawSHUTDOWN = FALSE;
	 AG_ThreadExit(NULL);
	 //return; /* シャットダウン期間 */
      }

      if(nDrawCount > 0) {
	 nDrawCount --;
	 continue;
      } else {
	 nDrawCount = DrawCountSet(nDrawFPS);
      }
      AGDrawTaskMain();
      DrawINGFlag = FALSE;
      DrawWaitFlag = FALSE;
   }
}


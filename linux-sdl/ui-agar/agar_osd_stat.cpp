/*
 * agar_osd_stat.cpp
 * Created on: 2012/06/28
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *     History: June,28,2012 Split from agar_osd.cpp
 */
#include <SDL/SDL.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <iconv.h>

#include "xm7.h"
#include "keyboard.h"
#include "tapelp.h"
#include "display.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "fdc.h"
#include "mouse.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "xm7_sdl.h"
#endif

#include "agar_osd.h"
#include "sdl_sch.h"
#include "api_draw.h"
#include "api_mouse.h"

#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"

#define OSD_STRLEN 256
struct OsdStatPack
{
        int Changed;
        BOOL init;
        char message[OSD_STRLEN + 1];
        AG_Mutex mutex;
        int width;
        int height;
        BOOL mouse_capture;
};
static struct OsdStatPack *pOsdStat;
static struct XM7_SDLView *pwSTAT;
static char MsgString[OSD_STRLEN + 1];

extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;

struct OsdStatPack *InitStat(int w,int h)
{
    struct OsdStatPack *p;
    AG_PixelFormat fmt;
    AG_Color black;
    int size = getnFontSize();

    SetPixelFormat(&fmt);
    p = (struct OsdStatPack *)malloc(sizeof(struct OsdStatPack));
    if(p == NULL) return NULL;
    memset(p, 0x00, sizeof(struct OsdStatPack));
    p->init = TRUE;
    p->Changed = FALSE;
    p->message[0] = '\0';
    AG_MutexInit(&(p->mutex));
    p->width = w;
    p->height = h;
    p->mouse_capture = FALSE;
    return p;
}

void DeleteStat(struct OsdStatPack *p)
{
   if(p == NULL) return;
   AG_MutexDestroy(&(p->mutex));
   free(p);
}

extern "C" {
static void DrawStatFn(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   struct OsdStatPack *disp = (struct OsdStatPack *)AG_PTR(1);
   AG_Surface *dst;
   AG_Color black, n;
   AG_Rect rect;
   int size;
   int len;

   if((disp == NULL)  || (my == NULL)) return;


   dst = XM7_SDLViewGetSrcSurface(my);
   if(dst == NULL) return;
    n.r = 255;
    n.g = 255;
    n.b = 255;
    n.a = 255;

    black.r = 0;
    black.g = 0;
    black.b = 0;
    black.a = 255;

   AG_MutexLock(&(disp->mutex));
   if((disp->Changed == FALSE) && (disp->init == FALSE) && (disp->mouse_capture == bMouseCaptureFlag)) {
       AG_MutexUnlock(&(disp->mutex));
       return;
   }
   rect.w = dst->w;
   rect.h = dst->h;
   rect.x = 0;
   rect.y = 0;
   AG_FillRect(dst, NULL, black);
   size = getnFontSize();
   len = strlen(disp->message);

   if((pStatusFont != NULL) && (size > 2) & (len <= OSD_STRLEN) && (len >= 0)){
      AG_Surface *tmps;
      AG_Font *font;

      AG_PushTextState();
      AG_TextFont(pStatusFont);
      font = AG_TextFontPts(size);
      AG_TextFont(font);
      
      AG_TextColor(n);
      AG_TextBGColor(black);
      tmps = AG_TextRender(disp->message);
      AG_SurfaceBlit(tmps, NULL, dst, 0, 0);
      AG_SurfaceFree(tmps);
      AG_PopTextState();
   }
   AG_WidgetUpdateSurface(AGWIDGET(my), my->mySurface);
   disp->init = FALSE;
   disp->Changed = FALSE;
   disp->mouse_capture = bMouseCaptureFlag;
   AG_MutexUnlock(&(disp->mutex));
}
}

static void CreateStat(AG_Widget *parent, struct OsdStatPack *p)
{
   AG_Surface *out;
   
  if(p == NULL) return;
  if(parent == NULL) return;
  pwSTAT = XM7_SDLViewNew(parent, NULL, NULL);
  if(pwSTAT == NULL) return;
  out = XM7_SDLViewSurfaceNew(pwSTAT, STAT_WIDTH , STAT_HEIGHT);
  AG_WidgetSetSize(pwSTAT, STAT_WIDTH , STAT_HEIGHT);
  XM7_SDLViewDrawFn(pwSTAT, DrawStatFn, "%p", p);
  AG_WidgetShow(pwSTAT);
}

void InitStatOSD(AG_Widget *parent)
{
   if(parent == NULL) return;
   pOsdStat = NULL;
   pOsdStat = InitStat(STAT_WIDTH, STAT_HEIGHT * 2);
   if(pOsdStat == NULL) return;
   CreateStat(parent, pOsdStat);
}

void DestroyStatOSD(void)
{
   if(pOsdStat != NULL) {
     DeleteStat(pOsdStat);
     pOsdStat = NULL;
   }
}

void LinkSurfaceSTAT(void)
{
}

void ResizeStatOSD(AG_Widget *parent, int w, int h)
{
  int total =  STAT_WIDTH + VFD_WIDTH * 2
          + CMT_WIDTH + LED_WIDTH * 3;
  float ww = (float)w;
  float wSTAT = (float)STAT_WIDTH / (float)total;

  if(pwSTAT == NULL) return;
  if(pOsdStat == NULL) return;
  AG_MutexLock(&(pOsdStat->mutex));
  pOsdStat->width = (int)(ww / 640.0f * (float)STAT_WIDTH);
  pOsdStat->height =  (int)(w / 640.0f * (float)STAT_HEIGHT);
  pOsdStat->init = TRUE;
  AG_WidgetSetSize(pwSTAT, pOsdStat->width, pOsdStat->height);

  AG_MutexUnlock(&(pOsdStat->mutex));
  {
     AG_SizeAlloc a;
     a.w = pOsdStat->width;
     a.h = pOsdStat->height;
     a.x = 0;
     a.y = 0;
     AG_WidgetSizeAlloc(pwSTAT, &a);
  
  }
}

void ClearStatOSD(void)
{
   if(pOsdStat == NULL) return;
   pOsdStat->message[0] = '\0';
   pOsdStat->Changed = TRUE;
}

void DrawMainCaption(BOOL redraw)
{
   char string[1024];
   char tmp[128];
   char *p;

   string[0] = '\0';
   if(pOsdStat == NULL) return;
   /*
    * RUN MODE
    */
   if (run_flag) {
       strcpy(string, "[RUN ]");
   } else {
       strcpy(string, "[STOP]");
   }
   /*
    * BOOT MODE
    */
   if(boot_mode == BOOT_BASIC){
      strcat(string, "[BAS]");
   } else if(boot_mode == BOOT_DOS) {
        strcat(string, "[DOS]");
    } else {
        strcat(string, "[???]");
    }
    if(bMouseCaptureFlag == FALSE) {
       strcat(string, "[ ]");
    } else {
       strcat(string, "[M]");
    }
    strcat(string, " ");

	/*
	 * CPU速度比率
	 */
	if (bAutoSpeedAdjust) {
		sprintf(tmp, "(%3d%%) ", speed_ratio / 100);
		strcat(string, tmp);
	}

	/*
	 * フロッピーディスクドライブ 0
	 */
	if (fdc_ready[0] != FDC_TYPE_NOTREADY) {

		/*
		 * ファイルネーム＋拡張子のみ取り出す
		 */
#ifdef _WINDOWS
		p = strrchr(fdc_fname[0], '\\');
#else
		p = strrchr(fdc_fname[0], '/');
#endif
		if (p == NULL) {
			p = fdc_fname[0];
		} else {
			p++;
		}
		sprintf(tmp, "- %s ", p);
		strcat(string, tmp);
	}

	/*
	 * フロッピーディスクドライブ 1
	 */
	if (fdc_ready[1] != FDC_TYPE_NOTREADY) {
		if ((strcmp(fdc_fname[0], fdc_fname[1]) != 0) ||
				(fdc_ready[0] == FDC_TYPE_NOTREADY)) {
			/*
			 * ファイルネーム＋拡張子のみ取り出す
			 */
#ifdef _WINDOWS
		p = strrchr(fdc_fname[1], '\\');
#else
		p = strrchr(fdc_fname[1], '/');
#endif
			if (p == NULL) {
				p = fdc_fname[1];
			} else {
				p++;
			}
			sprintf(tmp, "(%s) ", p);
			strcat(string, tmp);
		}
	}

	/*
	 * テープ
	 */
	if (tape_fileh != NULL) {

		/*
		 * ファイルネーム＋拡張子のみ取り出す
		 */
#ifdef _WINDOWS
		p = strrchr(tape_fname, '\\');
#else
		p = strrchr(tape_fname, '/');
#endif
		if (p == NULL) {
			p = tape_fname;
		} else {
			p++;
		}
		sprintf(tmp, "- %s ", p);
		strcat(string, tmp);
	}
    /*
     * Compare
     */
    if((strncmp(string, pOsdStat->message, OSD_STRLEN - 1) != 0) ||
       (redraw == TRUE)){
            AG_MutexLock(&(pOsdStat->mutex));
            pOsdStat->Changed = TRUE;
            strncpy(pOsdStat->message, string, OSD_STRLEN - 1);
            AG_MutexUnlock(&(pOsdStat->mutex));
    }

}


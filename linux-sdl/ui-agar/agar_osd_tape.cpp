/*
 * agar_osd_tape.cpp
 *
 *  Created on: 2012/06/08
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *     History: June,08,2012 Split from agar_osd.cpp
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

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_gldraw.h"
#else
#include "xm7_sdl.h"
#endif

#include "agar_osd.h"
#include "sdl_sch.h"
#include "api_draw.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"


enum {
    OSD_CMT_EMPTY = 0,
    OSD_CMT_NORM,
    OSD_CMT_READ,
    OSD_CMT_WRITE
};

struct OsdCMTPack {
    int OldStat;
    int OldCount;
    BOOL OldMotor;
    int stat;
    BOOL init;
    BOOL Changed;
    AG_Mutex mutex;
    AG_Surface *pSurface;
};

static int     nTape;           /* テープ */
static int     nOldTape;        /* テープ(過去) */
static XM7_SDLView *pwCMT;
static int nCMTWidth;
static int nCMTHeight;
static BOOL bTapeRecOld;
static BOOL bTapeInOld;
static struct OsdCMTPack *pCMTStat;

extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;

static void UpdateCMTCount(AG_Surface *dst, int count,struct OsdCMTPack *pStatus);
static BOOL UpdateCMT(XM7_SDLView *my, AG_Surface *dst, struct OsdCMTPack *pStatus);
static void UpdateCMTCount(int count, struct OsdCMTPack *pStatus);

extern "C" {
static void DrawCMTFn(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   struct OsdCMTPack *disp = (struct OsdCMTPack *)AG_PTR(1);
   AG_Surface *dst;
   int count = nTape;

   if((disp == NULL)  || (my == NULL)) return;
   dst = XM7_SDLViewGetSrcSurface(my);
   if(dst == NULL) return;
   AG_MutexLock(&(disp->mutex));
   if((count == disp->OldCount) && (disp->stat == disp->OldStat)
       && (disp->init == FALSE)) {
      disp->init = FALSE;
      disp->OldStat = disp->stat;
      disp->OldCount = count;
      AG_MutexUnlock(&(disp->mutex));
    return;
   }
   if(UpdateCMT(my, dst, disp) && (pCMTStat->Changed == TRUE)){
      pCMTStat->Changed = FALSE;
      UpdateCMTCount(count, disp);
      AG_SurfaceBlit(disp->pSurface, NULL, dst, 4, 4);
      AG_WidgetUpdateSurface(AGWIDGET(my), my->mySurface);
   }
//   AG_SurfaceCopy(dst, pDrawCMT);
   AG_MutexUnlock(&(disp->mutex));
}

}

static BOOL UpdateCMT(XM7_SDLView *my, AG_Surface *dst, struct OsdCMTPack *pStatus)
{
   if((dst == NULL) || (pStatusFont == NULL)
    || (pStatus == NULL)) return FALSE;
//   AG_SurfaceLock(dst);
//   AG_ObjectLock(AGOBJECT(dst));
   if((pStatus->stat > OSD_CMT_WRITE) || (pStatus->stat  < 0)){
      pStatus->stat = OSD_CMT_EMPTY;
   }
   if((pStatus->OldCount == nTape) && (pStatus->stat == pStatus->OldStat)) {
       pStatus->OldCount = nTape;
       pStatus->OldStat = pStatus->stat;
       return FALSE;
    }
//   AG_SurfaceBlit(pStatus->pSurface[pStatus->stat], NULL, dst, 4, 4);
   pStatus->OldCount = nTape;
   pStatus->OldStat = pStatus->stat;
   return TRUE;
}

static void UpdateCMTCount(int count, struct OsdCMTPack *pStatus)
{
   AG_Rect rect;
   int i;
   int x;
   int y;
   int w;
   int h;
   int size;
   char string[16];
   AG_Color r, b, n, black;
   AG_Surface *dst;
   
   if(pStatus == NULL) return;

   dst = pStatus->pSurface;
   if((dst == NULL) || (pStatusFont == NULL))return;
   r.r = 255;
   r.g = 0;
   r.b = 0;
   r.a = 255;

   black.r = 0;
   black.g = 0;
   black.b = 0;
   black.a = 255;

   b.r = 0;
   b.g = 0;
   b.b = 255;
   b.a = 255;

   n.r = 255;
   n.g = 255;
   n.b = 255;
   n.a = 255;
 //   AG_SurfaceLock(dst);
    {
        AG_Color fg, bg;
        AG_Surface *tmp;
        if(count < 0) count = 0; //
        if(tape_rec) {
	   sprintf(string, "Ｒ%04d", count % 10000);
	} else {
	   sprintf(string, "　%04d", count % 10000);
	}
       
        size = getnFontSize();
        AG_PushTextState();
        AG_TextFont(pStatusFont);
        AG_TextFontPts(size);
        switch(pStatus->stat){
        case OSD_CMT_EMPTY:
            bg = black;
            fg = black;
            break;
        case OSD_CMT_NORM:
            bg = black;
            fg = n;
            break;
        case OSD_CMT_READ:
            bg = r;
            fg = black;
            break;
        case OSD_CMT_WRITE:
            bg = b;
            fg = black;
            break;
        default:
            bg = black;
            fg = black;
            pStatus->stat = OSD_CMT_EMPTY;
            break;
        }
        if(pStatus->stat != OSD_CMT_EMPTY){
            AG_TextColor(fg);
            AG_TextBGColor(bg);
            tmp = AG_TextRender(string);
            AG_FillRect(dst, NULL, bg);
            AG_SurfaceBlit(tmp, NULL, dst, 4, 4);
            AG_SurfaceFree(tmp);
        } else {
            AG_FillRect(dst, NULL, bg);
        }

        AG_PopTextState();
    }
//    AG_SurfaceUnlock(dst);
//    pStatus->OldStat = pStatus->stat;
   return;
}


void CreateCMT(AG_Widget *parent, bool initflag)
{
    AG_Surface *out;
    AG_Color black;
    AG_PixelFormat fmt;

    SetPixelFormat(&fmt);

    black.r = black.g = black.b = 0;
    black.a = 255;
    pwCMT = XM7_SDLViewNew(parent, NULL, NULL);
    if(pwCMT == NULL) return;
    out = XM7_SDLViewSurfaceNew(pwCMT, nCMTWidth, nCMTHeight);
    AG_FillRect(out, NULL, black);
    pCMTStat->pSurface = AG_SurfaceStdRGBA(nCMTWidth, nCMTHeight);
    XM7_SDLViewDrawFn(pwCMT, DrawCMTFn,"%p", pCMTStat);
    AG_WidgetShow(pwCMT);
}


void InitTapeOSD(AG_Widget *parent)
{
    if(parent == NULL) return;
    nCMTHeight = CMT_HEIGHT;
    nCMTWidth = CMT_WIDTH;
    pCMTStat = (struct OsdCMTPack *)malloc(sizeof(struct OsdCMTPack));
    if(pCMTStat == NULL) return;
    memset(pCMTStat, 0x00, sizeof(struct OsdCMTPack));
    pCMTStat->init = TRUE;
    pCMTStat->OldCount = 0;
    pCMTStat->stat = OSD_CMT_EMPTY;
    pCMTStat->OldStat = -1;
    pCMTStat->pSurface = NULL;
    pCMTStat->Changed = FALSE;
    pCMTStat->OldMotor = FALSE;
    AG_MutexInit(&(pCMTStat->mutex));
    CreateCMT(parent, TRUE);
}

void DestroyTapeOSD(void)
{
    if(pCMTStat != NULL){
        if(pCMTStat->pSurface != NULL) AG_SurfaceFree(pCMTStat->pSurface);
        AG_MutexDestroy(&(pCMTStat->mutex));
        free(pCMTStat);
    }
   if(pwCMT != NULL) AG_ObjectDetach(AGOBJECT(pwCMT));
   pwCMT = NULL;
}


void LinkSurfaceCMT(void)
{
}

void ResizeTapeOSD(AG_Widget *parent, int w, int h)
{
       int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3;
       float ww = (float)w;
       float wCMT = (float)CMT_WIDTH / (float)total;
       AG_Surface *surface;

       if((pwCMT == NULL) || (pCMTStat == NULL)) return;
       nCMTWidth = (int)(ww / 640.0f * (float)CMT_WIDTH);
       nCMTHeight = (int)(ww / 640.0f * (float)STAT_HEIGHT);
       AG_MutexLock(&(pCMTStat->mutex));
       surface = XM7_SDLViewGetSrcSurface(pwCMT);
       AG_ObjectLock(pwCMT);
       if(surface != NULL) AG_SurfaceResize(surface, nCMTWidth, nCMTHeight);
       if(pCMTStat->pSurface == NULL) {
	  pCMTStat->pSurface = AG_SurfaceStdRGBA(nCMTWidth, nCMTHeight);
       } else {
	  AG_SurfaceResize(pCMTStat->pSurface, nCMTWidth, nCMTHeight);
       }
       AG_ObjectUnlock(pwCMT);
       pCMTStat->init = TRUE;
       AG_WidgetSetSize(pwCMT, nCMTWidth, nCMTHeight);
       //AG_WidgetSetPosition(pwCMT, (int)(((float)(STAT_WIDTH + VFD_WIDTH * 2) / (float)total) *  ww), 0);
       AG_MutexUnlock(&(pCMTStat->mutex));

}

void ClearTapeOSD(void)
{
   	nTape = -1;
	nOldTape = 0;
}


/*
 *  テープ描画
 */
void DrawTape(int override)
{
	int 		i;
	int             num;
	char     protect[16];
	BOOL bTapeIn = FALSE;
	BOOL bTapeRec = FALSE;

        if((pwCMT == NULL) || (pCMTStat == NULL)) return;
	if(tape_writep){
		strcpy(protect, "■");
	} else {
		strcpy(protect, "　");
	}
	/*
	 * ナンバー計算
	 */
	num = 0;
	if (tape_fileh != NULL) {
	   bTapeIn = TRUE;
		num = (int) ((tape_offset >> 10) % 10000); // Slow
		if (tape_motor && tape_rec) {
			    bTapeRec = TRUE;
		}
	}

	/*
	 * 番号比較
	 */
	 if ((bTapeIn == bTapeInOld) && (override != TRUE)
	     && (bTapeRecOld == bTapeRec) && (num == nTape) && (pCMTStat->OldMotor == tape_motor)){
		 return;
	 }

	/*
	 * 描画
	 */
        AG_MutexLock(&(pCMTStat->mutex));
	if ((!bTapeIn) && (bTapeIn != bTapeInOld)) {
	   if(pwCMT == NULL) return;
            nTape = 0;
            pCMTStat->stat = OSD_CMT_EMPTY;
	}
        
	if ((nTape != num) || (bTapeRec != bTapeRecOld) || (bTapeIn != bTapeInOld) || (override == TRUE)) {
	   bTapeRecOld = bTapeRec;
	   if(bTapeIn) {
	      if(tape_motor) {
		 if (bTapeRec) {
		    pCMTStat->stat = OSD_CMT_WRITE;
		 }   else {
		    pCMTStat->stat = OSD_CMT_READ;
		 }
	      } else {
		 pCMTStat->stat = OSD_CMT_NORM;
	      }
	      pCMTStat->Changed = TRUE;
	   } else if(bTapeInOld) {
	      pCMTStat->stat = OSD_CMT_EMPTY;
	      pCMTStat->Changed = TRUE;
	   }
	} else { // All same as.
	   if(pCMTStat->OldMotor != tape_motor) {
		pCMTStat->OldMotor = tape_motor;
	        if(!tape_motor) {
		   pCMTStat->stat = OSD_CMT_NORM;
		   pCMTStat->Changed = TRUE;
		}
	   } else {
	      pCMTStat->Changed = FALSE;
	   }
        }
        nTape = num;
        bTapeRecOld = bTapeRec;
        bTapeInOld = bTapeIn;
        AG_MutexUnlock(&(pCMTStat->mutex));
}

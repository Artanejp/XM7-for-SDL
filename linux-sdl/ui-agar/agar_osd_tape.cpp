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
    AG_Surface *pRead;
    AG_Surface *pWrite;
    AG_Surface *pNorm;
    AG_Surface *pEmpty;
    int OldStat;
    int OldCount;
    int stat;
    BOOL init;
    AG_Mutex mutex;
};

static int     nTape;           /* テープ */
static int     nOldTape;        /* テープ(過去) */
static XM7_SDLView *pwCMT;
static int nCMTWidth;
static int nCMTHeight;
static BOOL bTapeRecOld;
static BOOL bTapeInOld;
static struct OsdCMTPack *pCMTStat;
static AG_Surface *pDrawCMT;

extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;

static void UpdateCMTCount(AG_Surface *dst, int count,struct OsdCMTPack *pStatus);

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
   UpdateCMTCount(dst, count, disp);
//   AG_SurfaceCopy(dst, pDrawCMT);
   AG_WidgetUpdateSurface(AGWIDGET(my), my->mySurface);
   disp->init = FALSE;
   disp->OldStat = disp->stat;
   disp->OldCount = count;
   AG_MutexUnlock(&(disp->mutex));
}

}



static void UpdateCMTCount(AG_Surface *dst, int count, struct OsdCMTPack *pStatus)
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
        sprintf(string, "%04d", count % 10000);
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
    pStatus->OldStat = pStatus->stat;
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
    pDrawCMT = AG_SurfaceNew(AG_SURFACE_PACKED, nCMTWidth, nCMTHeight, &fmt, AG_SRCALPHA);
    AG_FillRect(out, NULL, black);
    XM7_SDLViewDrawFn(pwCMT, DrawCMTFn,"%p", pCMTStat);
    AG_WidgetShow(pwCMT);
}


void InitTapeOSD(AG_Widget *parent)
{
    if(parent == NULL) return;
    pDrawCMT = NULL;
    nCMTHeight = CMT_HEIGHT * 2;
    nCMTWidth = CMT_WIDTH;
    pCMTStat = (struct OsdCMTPack *)malloc(sizeof(struct OsdCMTPack));
    if(pCMTStat == NULL) return;
    memset(pCMTStat, 0x00, sizeof(struct OsdCMTPack));
    pCMTStat->init = TRUE;
    pCMTStat->OldCount = 0;
    pCMTStat->stat = OSD_CMT_EMPTY;
    pCMTStat->OldStat = -1;
    AG_MutexInit(&(pCMTStat->mutex));
    CreateCMT(parent, TRUE);
}

void DestroyTapeOSD(void)
{
    if(pCMTStat != NULL){
        AG_MutexDestroy(&(pCMTStat->mutex));
        if(pCMTStat->pEmpty != NULL) AG_SurfaceFree(pCMTStat->pEmpty);
        if(pCMTStat->pNorm != NULL) AG_SurfaceFree(pCMTStat->pNorm);
        if(pCMTStat->pRead != NULL) AG_SurfaceFree(pCMTStat->pRead);
        if(pCMTStat->pWrite != NULL) AG_SurfaceFree(pCMTStat->pWrite);
        free(pCMTStat);
    }
    if(pDrawCMT != NULL) AG_SurfaceFree(pDrawCMT);
   if(pwCMT != NULL) AG_ObjectDetach(AGOBJECT(pwCMT));
   pwCMT = NULL;
}


void LinkSurfaceCMT(void)
{
}

void ResizeTapeOSD(AG_Widget *parent, int w, int h)
{
       int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3 + 50;
       float ww = (float)w;
       float wCMT = (float)CMT_WIDTH / (float)total;

       if((pwCMT == NULL) || (pCMTStat == NULL)) return;
       nCMTWidth = (int)(ww * wCMT);
       nCMTHeight = (int)(wCMT * STAT_HEIGHT);
       AG_MutexLock(&(pCMTStat->mutex));
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

    if(pwCMT == NULL) return;
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
		num = (int) ((tape_offset >> 8) % 10000);
		if (tape_motor && tape_rec) {
			    bTapeRec = TRUE;
		}
	}

	/*
	 * 番号比較
	 */
	 if ((bTapeIn == bTapeInOld) && (override != TRUE)
      && (bTapeRecOld == bTapeRec) && (num == nTape)){
		 return;
	 }

	/*
	 * 描画
	 */
    AG_MutexLock(&(pCMTStat->mutex));
	if ((!bTapeIn) && (bTapeIn != bTapeInOld)) {
	        if(pwCMT == NULL) return;
            bTapeInOld = bTapeIn;
            nTape = 0;
            pCMTStat->stat = OSD_CMT_EMPTY;
	} else if((num != nTape)  || (bTapeInOld != bTapeIn)
       || (override = TRUE)){
		bTapeInOld = bTapeIn;
        pCMTStat->stat = OSD_CMT_NORM;
		/*
		 * カウンタ番号レンダリング(仮)
		 */
	}
	if (((nTape != num) || (bTapeRec != bTapeRecOld))
            && bTapeIn) {
			nTape = num;
			bTapeRecOld = bTapeRec;
			bTapeInOld = bTapeIn;
			if (bTapeRec) {
			    pCMTStat->stat = OSD_CMT_WRITE;
			}   else {
			    pCMTStat->stat = OSD_CMT_READ;
			}
		} else if(bTapeIn){
   			nTape = num;
			bTapeRecOld = bTapeRec;
			bTapeInOld = bTapeIn;
		    pCMTStat->stat = OSD_CMT_NORM;
        } else {
			nTape = num;
			bTapeRecOld = bTapeRec;
			bTapeInOld = bTapeIn;
		    pCMTStat->stat = OSD_CMT_EMPTY;
        }
        AG_MutexUnlock(&(pCMTStat->mutex));

}

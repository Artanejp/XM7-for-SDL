/*
 * agar_osd_led.cpp
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

struct OsdLEDPack 
{
   int OldStat;
   BOOL init;
   AG_Surface *pON;
   AG_Surface *pOFF;
   AG_Mutex mutex;
};

static struct OsdLEDPack *pOsdLEDIns;
static struct OsdLEDPack *pOsdLEDCAPS;
static struct OsdLEDPack *pOsdLEDKana;

static AG_Surface      *pIns; /* INSキープリレンダリング(ON) */
static AG_Surface      *pKana; /* カナキープリレンダリング(ON) */
static AG_Surface      *pCaps; /* Capsキープリレンダリング(ON) */
static XM7_SDLView     *pWidIns;
static XM7_SDLView     *pWidCaps;
static XM7_SDLView     *pWidKana;



static int nLedWidth;
static int nLedHeight;
static int nwCaps[2]; //
static int nwIns[2];
static int nwKana[2];
static int     nCAP;		/* CAPキー */
static int     nKANA;		/* かなキー */
static int     nINS;		/* INSキー */


extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;

/*
 * Draw LEDs(New)
 */

struct OsdLEDPack *InitLED(int w, int h, const char *str)
{
   struct OsdLEDPack *p;
   AG_PixelFormat fmt;
   AG_Surface *tmps;
   AG_Color r, b, black, n;
   int size =  getnFontSize();

   p = (struct OsdLEDPack *)malloc(sizeof(struct OsdLEDPack));
   if(p == NULL) return NULL;
   memset(p, 0x00, sizeof(struct OsdLEDPack));
   p->init = TRUE;

   SetPixelFormat(&fmt);
   p->pON = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   if(p->pON == NULL) {
	free(p);
        return NULL;
   }

   p->pOFF = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   if(p->pOFF == NULL) {
	free(p);
        AG_SurfaceFree(p->pON);
        return NULL;
   }

   AG_MutexInit(&(p->mutex));
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
   
   if((pStatusFont != NULL) && (size > 2)){
      AG_PushTextState();
      AG_TextFont(pStatusFont);
      AG_TextFontPts(size);
      AG_TextColor(black);
      AG_TextBGColor(r);

      AG_FillRect(p->pOFF, NULL, black);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pOFF, 1, 0);
      AG_SurfaceFree(tmps);
      AG_FillRect(p->pOFF, NULL, black);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pOFF, 1, 0);
      AG_SurfaceFree(tmps);
      
      AG_FillRect(p->pON, NULL, black);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pON, 1, 0);
      AG_SurfaceFree(tmps);
      AG_FillRect(p->pON, NULL, black);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pON, 1, 0);
      AG_SurfaceFree(tmps);
      AG_PopTextState();
   }

   return p;
}

void DeleteLED(struct OsdLEDPack *p)
{
   if(p == NULL) return;
   AG_MutexDestroy(&(p->mutex));
   if(p->pON != NULL) AG_SurfaceFree(p->pON);
   if(p->pOFF != NULL) AG_SurfaceFree(p->pOFF);
   free(p);
}


static void DrawLEDFn(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   struct OsdLEDPack *disp = (struct OsdLEDPack *)AG_PTR(1);
   int *stat = (int *)AG_PTR(2);
   AG_Surface *dst;
   
   if(disp == NULL) return;
   if(my == NULL) return;
   dst = XM7_SDLViewGetSrcSurface(my);
   if(dst == NULL) return;
   AG_MutexLock(&(disp->mutex));
   if(stat == NULL) {
	if((disp->OldStat != TRUE) || (disp->init == TRUE)) {
	   AG_SurfaceBlit(disp->pON, NULL, dst, 1, 0); 
	}
        disp->OldStat = TRUE;
        disp->init = FALSE;
        AG_MutexUnlock(&(disp->mutex));
        return;
   }
   if(*stat == FALSE) {
      if((disp->init == TRUE) || (disp->OldStat != *stat)) AG_SurfaceBlit(disp->pOFF, NULL, dst, 1, 0); 
   } else {
      if((disp->init == TRUE) || (disp->OldStat != *stat)) AG_SurfaceBlit(disp->pON, NULL, dst, 1, 0); 
   }
   disp->init = FALSE;
   disp->OldStat = *stat;
   AG_MutexUnlock(&(disp->mutex));

}

static void CreateLEDs(AG_Widget *parent)
{
    AG_PixelFormat fmt;
    AG_SizeAlloc a;
    int size;
    int w, h;
   
   w = LED_WIDTH * 2;
   nLedWidth = w;
   h = LED_HEIGHT * 2;
   nLedHeight = w;
   SetPixelFormat(&fmt);

   pIns = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   pCaps = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   pKana = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);

   pWidIns = XM7_SDLViewNew(parent, NULL, NULL);
   pWidCaps = XM7_SDLViewNew(parent, NULL, NULL);
   pWidKana = XM7_SDLViewNew(parent, NULL, NULL);
   pOsdLEDIns = InitLED(nLedWidth, nLedHeight, "INS");
   pOsdLEDCAPS = InitLED(nLedWidth, nLedHeight, "CAPS");
   pOsdLEDKana = InitLED(nLedWidth, nLedHeight, "カナ");
   nCAP = 0;
   nINS = 0;
   nKANA = 0;
   XM7_SDLViewDrawFn(pWidIns, DrawLEDFn, "%p%p", pOsdLEDIns, &nINS);
   XM7_SDLViewDrawFn(pWidCaps, DrawLEDFn, "%p%p", pOsdLEDCAPS, &nCAP);
   XM7_SDLViewDrawFn(pWidKana, DrawLEDFn, "%p%p", pOsdLEDKana, &nKANA);

   a.w = w;
   a.h = h;
   a.x = 0;
   a.y = 0;
   AG_WidgetSizeAlloc(pWidIns, &a);
   AG_WidgetSizeAlloc(pWidCaps, &a);
   AG_WidgetSizeAlloc(pWidKana, &a);
   AG_WidgetSetSize(pWidIns, w, h);
   AG_WidgetSetSize(pWidCaps, w, h);
   AG_WidgetSetSize(pWidKana, w, h);
   AG_WidgetShow(pWidIns);
   AG_WidgetShow(pWidCaps);
   AG_WidgetShow(pWidKana);
}



void InitLeds(AG_Widget *parent)
{
   CreateLEDs(parent);
}

/*
 *  CAPキー描画
 */
void DrawCAP(void)
{
	int            num;
	/*
	 * 番号決定
	 */
	if (caps_flag) {
		num = 1;
	} else {
		num = 0;
	}
	/*
	 * 描画、ワーク更新
	 */
	nCAP = num;
}


/*
 *  かなキー描画
 */
void DrawKANA(void)
{
	int            num;

	/*
	 * 番号決定
	 */
	if (kana_flag) {
		num = 1;
	} else {
		num = 0;
	}

	/*
	 * 描画、ワーク更新
	 */
	nKANA = num;
}


/*
 *  INSキー描画
 */
void DrawINS(void)
{
	int            num;

	/*
	 * 番号決定
	 */
	if (ins_flag) {
		num = 1;
	}  else {
		num = 0;
	}

	/*
	 * 描画、ワーク更新
	 */
	nINS = num;
}




void DestroyLeds(void)
{
   if(pWidIns != NULL) AG_ObjectDetach(AGOBJECT(pWidIns));
   if(pWidCaps != NULL) AG_ObjectDetach(AGOBJECT(pWidCaps));
   if(pWidKana != NULL) AG_ObjectDetach(AGOBJECT(pWidKana));
   DeleteLED(pOsdLEDIns);
   DeleteLED(pOsdLEDCAPS);
   DeleteLED(pOsdLEDKana);
}

void LinkSurfaceLeds(void)
{

}

void ResizeLeds(AG_Widget *parent, int w, int h)
{
    int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3 + 50;
    float wLed = (float)LED_WIDTH / (float)total;
    float ww = (float)w;
    int nFontSize;
   
    nLedHeight = h;
    nLedWidth = (int)(ww * wLed);
    if(nLedWidth <= 0) return;
    nFontSize = (int)(STAT_PT * (float)h * 1.0f) / (STAT_HEIGHT * 2.0f);
    AG_WidgetSetSize(pWidIns, w, h);
    AG_WidgetSetSize(pWidCaps, w, h);
    AG_WidgetSetSize(pWidKana, w, h);


}

void ClearLeds(void)
{
   	nCAP = -1;
	nKANA = -1;
	nINS = -1;
}

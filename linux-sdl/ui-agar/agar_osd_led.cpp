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

struct OsdLEDPack *InitLED(int w, int h, const char *str, int xoff, int yoff)
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

      AG_TextColor(n);
      AG_TextBGColor(black);
      AG_FillRect(p->pOFF, NULL, black);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pOFF, xoff, yoff);
      AG_SurfaceFree(tmps);

      AG_TextColor(black);
      AG_TextBGColor(r);
      AG_FillRect(p->pON, NULL, r);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pON, xoff, yoff);
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

extern "C" {
static void DrawLEDFn(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   int *stat = (int *)AG_PTR(1);
   struct OsdLEDPack *disp = (struct OsdLEDPack *)AG_PTR(2);
   AG_Surface *dst;
   AG_Color black, r;
   AG_Rect rect;

   if((disp == NULL)  || (my == NULL)) return;


   dst = XM7_SDLViewGetSrcSurface(my);
   if(dst == NULL) return;
    r.r = 255;
    r.g = 0;
    r.b = 0;
    r.a = 255;

    black.r = 0;
    black.g = 0;
    black.b = 0;
    black.a = 255;

   AG_MutexLock(&(disp->mutex));
   if((disp->init == FALSE) && (*stat == disp->OldStat)){
       AG_MutexUnlock(&(disp->mutex));
       return;
   }
   rect.w = dst->w;
   rect.h = dst->h;
   rect.x = 0;
   rect.y = 0;
   if(*stat == FALSE) {
      AG_SurfaceCopy(dst, disp->pOFF);
   } else {
      AG_SurfaceCopy(dst, disp->pON);
   }
   AG_WidgetUpdateSurface(AGWIDGET(my), my->mySurface);
   disp->init = FALSE;
   disp->OldStat = *stat;
   AG_MutexUnlock(&(disp->mutex));
}

}
static void CreateLEDs(AG_Widget *parent)
{
    AG_PixelFormat fmt;
    AG_SizeAlloc a;
    AG_Surface *stmp;
    AG_Color black;
    int size;
    int w, h;


   w = LED_WIDTH;
   nLedWidth = w;
   h = LED_HEIGHT;
   nLedHeight = w;
   SetPixelFormat(&fmt);
   black.r = 0;
   black.g = 0;
   black.b = 0;
   black.a = 255;

   pWidIns = XM7_SDLViewNew(parent, NULL, NULL);
   pWidCaps = XM7_SDLViewNew(parent, NULL, NULL);
   pWidKana = XM7_SDLViewNew(parent, NULL, NULL);

   nCAP = 0;
   nINS = 0;
   nKANA = 0;

   pOsdLEDIns = InitLED(nLedWidth, nLedHeight, "INS", 4, 4);
   XM7_SDLViewSurfaceNew(pWidIns, w, h);
   stmp = XM7_SDLViewGetSrcSurface(pWidIns);
   AG_FillRect(stmp, NULL, black);
   XM7_SDLViewDrawFn(pWidIns, DrawLEDFn, "%p,%p", &nINS, pOsdLEDIns);

   pOsdLEDCAPS = InitLED(nLedWidth, nLedHeight, "CAP", 0, 4);
   XM7_SDLViewSurfaceNew(pWidCaps, w, h);
   stmp = XM7_SDLViewGetSrcSurface(pWidCaps);
   AG_FillRect(stmp, NULL, black);
   XM7_SDLViewDrawFn(pWidCaps, DrawLEDFn, "%p,%p", &nCAP, pOsdLEDCAPS);

   pOsdLEDKana = InitLED(nLedWidth, nLedHeight, "カナ", 6, 4);
   XM7_SDLViewSurfaceNew(pWidKana, w, h);
   stmp = XM7_SDLViewGetSrcSurface(pWidKana);
   AG_FillRect(stmp, NULL, black);
   XM7_SDLViewDrawFn(pWidKana, DrawLEDFn, "%p,%p", &nKANA, pOsdLEDKana);

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
    if(nCAP != num){
	   nCAP = num;
	   if(pWidCaps != NULL) AG_WidgetUpdate(pWidCaps);
    }
	nCAP = num;
}


/*
 *  かなキー描画
 */
void DrawKANA(void)
{
	int            num;
	AG_Surface *p;

	/*
	 * 番号決定
	 */
    if(pOsdLEDKana == NULL) return;
	if (kana_flag) {
		num = 1;
	} else {
		num = 0;
	}
	/*
	 * 描画、ワーク更新
	 */
    if(nKANA != num){
	   nKANA = num;
	   if(pWidKana != NULL) AG_Redraw(pWidKana);
    }
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
        if(nINS != num) {
	   nINS = num;
	   if(pWidIns != NULL) AG_Redraw(pWidIns);
	}
   
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

static AG_Surface *ResizeOneLed(XM7_SDLView *wid, struct OsdLEDPack *p, char *str)
{
   AG_Surface *src;
   AG_Color n, r, black;
   AG_PixelFormat fmt;
   int size =  getnFontSize();
   int w = nLedWidth;
   int h = nLedHeight;
   
   if((wid == NULL) || (p == NULL)) return NULL;
   SetPixelFormat(&fmt);

   src = XM7_SDLViewGetSrcSurface(wid);
   if(src == NULL) {
      src = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
      if(src == NULL) return NULL;
      wid->Surface = src;
   } else {
      AG_SurfaceResize(src, w, h);
   }
   
    r.r = 255;
    r.g = 0;
    r.b = 0;
    r.a = 255;

    black.r = 0;
    black.g = 0;
    black.b = 0;
    black.a = 255;

    n.r = 255;
    n.g = 255;
    n.b = 255;
    n.a = 255;
   
   if(p->pON != NULL) AG_SurfaceFree(p->pON);
   if(p->pOFF != NULL) AG_SurfaceFree(p->pOFF);

   p->pON = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   if(p->pON == NULL) {
//	free(p);
        return NULL;
   }

   p->pOFF = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   if(p->pOFF == NULL) {
//	free(p);
        AG_SurfaceFree(p->pON);
        return NULL;
   }
     if((pStatusFont != NULL) && (size > 2)){
      AG_Surface *tmps;
      int xoff = (3 * nLedWidth) / LED_WIDTH;
      int yoff = (3 * nLedHeight) / LED_HEIGHT;
      AG_PushTextState();
      AG_TextFont(pStatusFont);
      AG_TextFontPts(size);

      AG_TextColor(n);
      AG_TextBGColor(black);
      AG_FillRect(p->pOFF, NULL, black);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pOFF, xoff, yoff);
      AG_SurfaceFree(tmps);

      AG_TextColor(black);
      AG_TextBGColor(r);
      AG_FillRect(p->pON, NULL, r);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pON, xoff, yoff);
      AG_SurfaceFree(tmps);

      AG_PopTextState();
   }
   AG_WidgetUpdateSurface(AGWIDGET(wid), wid->mySurface);

   return src;
}

   

void ResizeLeds(AG_Widget *parent, int w, int h)
{
    int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3;
    float wLed = (float)LED_WIDTH / (float)total;
    float ww = (float)w;
    int nFontSize;

    nLedHeight = (int)((float)h / 400.0f * (float)STAT_HEIGHT);
    nLedWidth = (int)(ww / 640.0f * (float)LED_WIDTH);
    if(nLedWidth <= 0) return;
    if((pOsdLEDIns == NULL) || (pOsdLEDCAPS == NULL) || (pOsdLEDKana == NULL)) return; 
    if((pWidIns == NULL) || (pWidCaps == NULL) || (pWidKana == NULL)) return; 
//    nFontSize = (int)(STAT_PT * (float)h * 1.0f) / (STAT_HEIGHT * 2.0f);
    AG_MutexLock(&(pOsdLEDIns->mutex));
    AG_MutexLock(&(pOsdLEDCAPS->mutex));
    AG_MutexLock(&(pOsdLEDKana->mutex));
    {
       AG_WidgetSetSize(pWidIns, nLedWidth, nLedHeight);
       ResizeOneLed(pWidIns, pOsdLEDIns, "Ins");
       AG_WidgetSetSize(pWidCaps, nLedWidth, nLedHeight);
       ResizeOneLed(pWidCaps, pOsdLEDCAPS, "CAP");
       AG_WidgetSetSize(pWidKana, nLedWidth, nLedHeight);
       ResizeOneLed(pWidKana, pOsdLEDKana, "カナ");
    }
    AG_MutexUnlock(&(pOsdLEDIns->mutex));
    AG_MutexUnlock(&(pOsdLEDCAPS->mutex));
    AG_MutexUnlock(&(pOsdLEDKana->mutex));
//    {
//       AG_SizeAlloc a;
//       a.w = nLedWidth;
//       a.h = nLedHeight;
//       a.x = 0;
//       a.y = 0;

//       AG_WidgetSizeAlloc(pWidIns, &a);
//       AG_WidgetUpdate(pWidIns);
//       AG_WidgetSizeAlloc(pWidCaps, &a);
//       AG_WidgetUpdate(pWidCaps);
//       AG_WidgetSizeAlloc(pWidKana, &a);
//       AG_WidgetUpdate(pWidKana);
//    }
}

void ClearLeds(void)
{
   	nCAP = -1;
	nKANA = -1;
	nINS = -1;
}

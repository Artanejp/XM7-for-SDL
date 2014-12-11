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
   AG_Surface *surface;
   int numON;
   int numOFF;
   AG_Mutex mutex;
};

static struct OsdLEDPack *pOsdLEDIns;
static struct OsdLEDPack *pOsdLEDCAPS;
static struct OsdLEDPack *pOsdLEDKana;

static AG_Pixmap     *pWidIns;
static AG_Pixmap     *pWidCaps;
static AG_Pixmap     *pWidKana;
static AG_Box *pLedBox;


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
static void ResizeOneLed(AG_Pixmap *wid, struct OsdLEDPack *p, char *str, int stat);

struct OsdLEDPack *InitLED(AG_Pixmap *wid, int w, int h, const char *str, int stat)
{
   struct OsdLEDPack *p;
   AG_PixelFormat fmt;
   AG_Surface *tmps;
   AG_Color r, b, black, n;
   int size =  getnFontSize();

   p = (struct OsdLEDPack *)malloc(sizeof(struct OsdLEDPack));
   if(p == NULL) return NULL;
   memset(p, 0x00, sizeof(struct OsdLEDPack));

   SetPixelFormat(&fmt);
   p->pON = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   if(p->pON == NULL) {
	free(p);
        return NULL;
   }

   p->pOFF = AG_SurfaceNew(AG_SURFACE_PACKED , w,  h, &fmt, AG_SRCALPHA);
   if(p->pOFF == NULL) {
        AG_SurfaceFree(p->pON);
	free(p);
        return NULL;
   }
   nLedWidth = w;
   nLedHeight = h;
   AG_MutexInit(&(p->mutex));
   ResizeOneLed(wid, p, str, stat);
   p->init = TRUE;
   return p;
}

void DeleteLED(struct OsdLEDPack *p)
{
   if(p == NULL) return;
   AG_MutexDestroy(&(p->mutex));
   if(p->pON != NULL) AG_SurfaceFree(p->pON);
   if(p->pOFF != NULL) AG_SurfaceFree(p->pOFF);
   p->pON = p->pOFF = NULL;
   free(p);
}



static void CreateLEDs(AG_Widget *parent)
{
    AG_PixelFormat fmt;
    AG_SizeAlloc a;
    AG_Surface *stmp;
    AG_Color black;
    int size;
    int w, h;


   w = nLedWidth;
   h = nLedHeight;
   SetPixelFormat(&fmt);
   black.r = 0;
   black.g = 0;
   black.b = 0;
   black.a = 255;

   pWidIns = AG_PixmapNew(pLedBox, AG_PIXMAP_RESCALE, w, h);
   AG_WidgetSetSize(pWidIns, w, h);
   pWidCaps = AG_PixmapNew(pLedBox, AG_PIXMAP_RESCALE, w, h);
   AG_WidgetSetSize(pWidCaps, w, h);
   pWidKana = AG_PixmapNew(pLedBox, AG_PIXMAP_RESCALE, w, h);
   AG_WidgetSetSize(pWidKana, w, h);

   nCAP = 0;
   nINS = 0;
   nKANA = 0;

   pOsdLEDIns = InitLED(pWidIns, nLedWidth, nLedHeight, " INS", FALSE);

   pOsdLEDCAPS = InitLED(pWidCaps, nLedWidth, nLedHeight, " CAP", FALSE);

   pOsdLEDKana = InitLED(pWidKana, nLedWidth, nLedHeight, " カナ", FALSE);

   AG_WidgetShow(pWidIns);
   AG_WidgetShow(pWidCaps);
   AG_WidgetShow(pWidKana);
}



void InitLeds(AG_Widget *parent)
{
   nLedWidth = LED_WIDTH;
   nLedHeight = LED_HEIGHT;
   pLedBox = AG_BoxNew(parent, AG_BOX_HORIZ, AG_BOX_EXPAND);

   CreateLEDs(AGWIDGET(pLedBox));
}

static void DrawLEDSub(AG_Pixmap *wid, struct OsdLEDPack *pack, int old_var, int new_var)
{
   if((wid == NULL) || (pack == NULL)) return;
   AG_MutexLock(&(pack->mutex));
   if((old_var != new_var) || (pack->init != FALSE)) {
	if(new_var == 0) {
	   AG_PixmapSetSurface(wid, pack->numOFF);
	} else {
	   AG_PixmapSetSurface(wid, pack->numON);
	}
      pack->init = FALSE;
      //if((pack->pOFF != NULL) || (pack->pON != NULL)) pack->init = FALSE;
      AG_Redraw(wid);
   }
   AG_MutexUnlock(&(pack->mutex));
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
   DrawLEDSub(pWidCaps, pOsdLEDCAPS, nCAP, num);
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
   DrawLEDSub(pWidKana, pOsdLEDKana, nKANA, num);
   nKANA = num;
}


      
      
/*
 *  INSキー描画
 */
void DrawINS(void)
{
	int            num;
        AG_Pixmap *wid;
        struct OsdLEDPack *pack;

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
   DrawLEDSub(pWidIns, pOsdLEDIns, nINS, num);
   nINS = num;
}




void DestroyLeds(void)
{
   if(pWidIns != NULL) AG_ObjectDetach(AGOBJECT(pWidIns));
   if(pWidCaps != NULL) AG_ObjectDetach(AGOBJECT(pWidCaps));
   if(pWidKana != NULL) AG_ObjectDetach(AGOBJECT(pWidKana));
   pWidIns = pWidCaps = pWidKana = NULL;
   DeleteLED(pOsdLEDIns);
   DeleteLED(pOsdLEDCAPS);
   DeleteLED(pOsdLEDKana);
   pOsdLEDIns  = NULL;
   pOsdLEDCAPS = NULL;
   pOsdLEDKana = NULL;
}

void LinkSurfaceLeds(void)
{

}

static void ResizeOneLed(AG_Pixmap *wid, struct OsdLEDPack *p, char *str, int stat)
{
   AG_Color n, r, black;
   AG_PixelFormat fmt;

   int size =  (nLedWidth * STAT_PT) / LED_WIDTH;
   int w = nLedWidth;
   int h = nLedHeight;
   
   if((wid == NULL) || (p == NULL)) return NULL;
   SetPixelFormat(&fmt);
   
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
   
   if(p->pON == NULL) p->pON = AG_SurfaceNew(AG_SURFACE_PACKED, w, h, &fmt, AG_SRCALPHA);
   if(p->pOFF == NULL) p->pOFF = AG_SurfaceNew(AG_SURFACE_PACKED, w, h, &fmt, AG_SRCALPHA);
   if(AG_SurfaceResize(p->pON , w,  h) < 0) {
      return;
    }
    if(AG_SurfaceResize(p->pOFF , w,  h) < 0) {
      return;
    }
   
   if((pStatusFont != NULL) && (size > 2)){
      AG_Surface *tmps;
      AG_Font *font;
      int ratio = (nLedWidth * 100) / LED_WIDTH;
	
      AG_PushTextState();
      AG_TextFont(pStatusFont);
      font = AG_TextFontPct(ratio);
//      font = AG_FetchFont(StatusFont, 32, 0);
      if(font != NULL) {
	 AG_TextFont(font);
      }
      
      AG_TextJustify(AG_TEXT_CENTER);

      AG_TextColor(n);
      AG_TextBGColor(black);
      AG_FillRect(p->pOFF, NULL, black);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pOFF, 0, 0);
      AG_SurfaceFree(tmps);
      
      AG_TextColor(black);
      AG_TextBGColor(r);
      AG_FillRect(p->pON, NULL, r);
      tmps = AG_TextRender(str);
      AG_SurfaceBlit(tmps, NULL, p->pON, 0, 0);
      AG_SurfaceFree(tmps);

      AG_PopTextState();
   }
   p->numON  = AG_PixmapAddSurface(wid, p->pON);
   p->numOFF = AG_PixmapAddSurface(wid, p->pOFF);
   p->init = TRUE;
   return;
}

   

void ResizeLeds(AG_Widget *parent, int w, int h)
{
    int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3;
    float wLed = (float)LED_WIDTH / (float)total;
    float ww = (float)w;
    int nFontSize;

    //nLedHeight = (int)(wLed * (float)h);
    nLedHeight = (int)(ww / 640.0f * (float)(STAT_HEIGHT));
    nLedWidth = (int)(wLed * (float)w);
    //nLedHeight = (int)(wLed * (float)h);
    if((nLedWidth <= 0) || (nLedHeight <= 0))return;
   
    if((pOsdLEDIns == NULL) || (pOsdLEDCAPS == NULL) || (pOsdLEDKana == NULL)) return; 
    if((pWidIns == NULL) || (pWidCaps == NULL) || (pWidKana == NULL)) return; 

    {
       //AG_WidgetSetSize(pLedBox, nLedWidth * 3, nLedHeight);
       DestroyLeds();
       CreateLEDs(AGWIDGET(pLedBox));
    }
}

void ClearLeds(void)
{
   	nCAP = -1;
	nKANA = -1;
	nINS = -1;
}

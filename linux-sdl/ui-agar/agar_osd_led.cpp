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

static AG_Surface      *pInsOn; /* INSキープリレンダリング(ON) */
static AG_Surface      *pInsOff; /* INSキープリレンダリング(OFF) */
static AG_Surface      *pKanaOn; /* カナキープリレンダリング(ON) */
static AG_Surface      *pKanaOff; /* カナキープリレンダリング(OFF) */
static AG_Surface      *pCapsOn; /* Capsキープリレンダリング(ON) */
static AG_Surface      *pCapsOff; /* Capsキープリレンダリング(OFF) */

static int nLedWidth;
static int nLedHeight;
static AG_Pixmap *pwCAPS;
static AG_Pixmap *pwINS;
static AG_Pixmap *pwKana;
static int nwCaps[2]; //
static int nwIns[2];
static int nwKana[2];
static int     nCAP;		/* CAPキー */
static int     nKANA;		/* かなキー */
static int     nINS;		/* INSキー */


extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;


static void CreateLEDs(AG_Widget *parent, BOOL initflag)
{
    AG_PixelFormat fmt;
    AG_Surface *tmps;
    AG_Color r, b, black, n;
    int size;

   SetPixelFormat(&fmt);
   size =  getnFontSize();

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
   if(initflag){
     if(pwINS != NULL){
        AG_ObjectDetach(AGOBJECT(pwINS));
        pwINS = NULL;
     }
      if(pwKana != NULL){
        AG_ObjectDetach(AGOBJECT(pwKana));
        pwKana = NULL;
      }
      if(pwCAPS != NULL){
        AG_ObjectDetach(AGOBJECT(pwCAPS));
        pwCAPS = NULL;
    }
   }



   if(pStatusFont == NULL) return;
   if(size <= 2) return;


    AG_PushTextState();
	AG_TextFont(pStatusFont);
        AG_TextFontPts(size);
	AG_TextColor(black);
	AG_TextBGColor(r);

   if(pInsOn) {
      AG_SurfaceResize(pInsOn, nLedWidth, nLedHeight);
   } else {
      pInsOn = AG_SurfaceNew(AG_SURFACE_PACKED , nLedWidth, nLedHeight, &fmt, AG_SRCALPHA);
   }

	AG_FillRect(pInsOn, NULL, r);
	tmps = AG_TextRender("Ins");
	AG_SurfaceBlit(tmps, NULL, pInsOn, 1, 0);
	AG_SurfaceFree(tmps);

   if(pCapsOn) {
      AG_SurfaceResize(pCapsOn, nLedWidth, nLedHeight);
   } else {
      pCapsOn = AG_SurfaceNew(AG_SURFACE_PACKED , nLedWidth, nLedHeight, &fmt, AG_SRCALPHA);
   }
	AG_FillRect(pCapsOn, NULL, r);
	tmps = AG_TextRender("CAP");
	AG_SurfaceBlit(tmps, NULL, pCapsOn, 1, 0);
	AG_SurfaceFree(tmps);

   if(pKanaOn) {
      AG_SurfaceResize(pKanaOn, nLedWidth, nLedHeight);
   } else {
      pKanaOn = AG_SurfaceNew(AG_SURFACE_PACKED , nLedWidth, nLedHeight, &fmt, AG_SRCALPHA);
   }
	AG_FillRect(pKanaOn, NULL, r);
	tmps = AG_TextRender("カナ");
	AG_SurfaceBlit(tmps, NULL, pKanaOn, 1, 0);
	AG_SurfaceFree(tmps);
	AG_TextColor(n);
	AG_TextBGColor(black);
   if(pInsOff) {
      AG_SurfaceResize(pInsOff, nLedWidth, nLedHeight);
   } else {
      pInsOff = AG_SurfaceNew(AG_SURFACE_PACKED , nLedWidth, nLedHeight, &fmt, AG_SRCALPHA);
   }

	AG_FillRect(pInsOff, NULL, black);
	tmps = AG_TextRender("Ins");
	AG_SurfaceBlit(tmps, NULL, pInsOff, 1, 0);
	AG_SurfaceFree(tmps);

   if(pCapsOff) {
      AG_SurfaceResize(pCapsOff, nLedWidth, nLedHeight);
   } else {
      pCapsOff = AG_SurfaceNew(AG_SURFACE_PACKED , nLedWidth, nLedHeight, &fmt, AG_SRCALPHA);
   }
	AG_FillRect(pCapsOff, NULL, black);
	tmps = AG_TextRender("CAP");
	AG_SurfaceBlit(tmps, NULL, pCapsOff, 1, 0);
	AG_SurfaceFree(tmps);

   if(pKanaOff) {
      AG_SurfaceResize(pKanaOff, nLedWidth, nLedHeight);
   } else {
      pKanaOff = AG_SurfaceNew(AG_SURFACE_PACKED , nLedWidth, nLedHeight, &fmt, AG_SRCALPHA);
   }
	AG_FillRect(pKanaOff, NULL, black);
	tmps = AG_TextRender("カナ");
	AG_SurfaceBlit(tmps, NULL, pKanaOff, 1, 0);
	AG_SurfaceFree(tmps);
    AG_PopTextState();
}


void InitLeds(AG_Widget *parent)
{
   nLedWidth = LED_WIDTH * 2;
   nLedHeight = LED_HEIGHT * 2;
   pInsOn = pInsOff = NULL;
   pCapsOn = pCapsOff = NULL;
   pKanaOn = pKanaOff = NULL;
   pwINS = NULL;
   pwCAPS = NULL;
   pwKana = NULL;
   CreateLEDs(parent, TRUE);
    pwCAPS = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nLedWidth, nLedHeight);
    pwINS = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nLedWidth, nLedHeight);
    pwKana = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nLedWidth, nLedHeight);
    AG_WidgetSetSize(pwCAPS, nLedWidth, nLedHeight);
    AG_WidgetSetSize(pwINS, nLedWidth, nLedHeight);
    AG_WidgetSetSize(pwKana, nLedWidth, nLedHeight);
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
    if(pwCAPS == NULL) return;
    AG_ObjectLock(pwCAPS);
	if (nCAP) {
	      AG_PixmapSetSurface(pwCAPS, nwCaps[ID_ON]);
       } else {
	     AG_PixmapSetSurface(pwCAPS, nwCaps[ID_OFF]);
	}
    AG_ObjectUnlock(pwCAPS);
     AG_Redraw(pwCAPS);
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
    if(pwKana == NULL) return;
    AG_ObjectLock(pwKana);

	if (nKANA) {
	      AG_PixmapSetSurface(pwKana, nwKana[ID_ON]);
	} else {
	      AG_PixmapSetSurface(pwKana, nwKana[ID_OFF]);
	}
    AG_ObjectUnlock(pwKana);
    AG_Redraw(pwKana);
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
    if(pwINS == NULL) return;
    AG_ObjectLock(pwINS);
	if (nINS) {
	      AG_PixmapSetSurface(pwINS, nwIns[ID_ON]);
	} else {
	      AG_PixmapSetSurface(pwINS, nwIns[ID_OFF]);
	}
     AG_ObjectUnlock(pwINS);

     AG_Redraw(pwINS);
}




void DestroyLeds(void)
{
	if(pInsOn !=NULL ) {
		AG_SurfaceFree(pInsOn);
		pInsOn = NULL;
	}
	if(pInsOff != NULL) {
		AG_SurfaceFree(pInsOff);
		pInsOff = NULL;
	}
	if(pCapsOn !=NULL ) {
		AG_SurfaceFree(pCapsOn);
		pCapsOn = NULL;
	}
	if(pCapsOff != NULL) {
		AG_SurfaceFree(pCapsOff);
		pCapsOff = NULL;
	}
	if(pKanaOn !=NULL ) {
		AG_SurfaceFree(pKanaOn);
		pKanaOn = NULL;
	}
	if(pKanaOff !=NULL ) {
		AG_SurfaceFree(pKanaOff);
		pKanaOff = NULL;
	}

}

void LinkSurfaceLeds(void)
{
       // CAPS
    nwCaps[ID_OFF] = AG_PixmapAddSurface(pwCAPS, pCapsOff);
    nwCaps[ID_ON] = AG_PixmapAddSurface(pwCAPS, pCapsOn);
    AG_WidgetShow(pwCAPS);

   // INS
    nwIns[ID_OFF] = AG_PixmapAddSurface(pwINS, pInsOff);
    nwIns[ID_ON] = AG_PixmapAddSurface(pwINS, pInsOn);
    AG_WidgetShow(pwINS);

    // Kana
    nwKana[ID_ON] = AG_PixmapAddSurface(pwKana, pKanaOn);
    nwKana[ID_OFF] = AG_PixmapAddSurface(pwKana, pKanaOff);
    AG_WidgetShow(pwKana);

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
    AG_WidgetHide(pwCAPS);
    AG_ObjectDetach(AGOBJECT(pwCAPS));
    AG_WidgetHide(pwINS);
    AG_ObjectDetach(AGOBJECT(pwINS));
    AG_WidgetHide(pwKana);
    AG_ObjectDetach(AGOBJECT(pwKana));

    CreateLEDs(parent, FALSE);
    nwCaps[ID_ON] = 0;
    pwCAPS =  AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE , pCapsOn);
    nwCaps[ID_OFF] = AG_PixmapAddSurface(pwCAPS, pCapsOff);
    AG_WidgetSetSize(pwCAPS, nLedWidth, nLedHeight);

    nwIns[ID_ON] = 0;
    pwINS = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE ,  pInsOn);
    nwIns[ID_OFF] = AG_PixmapAddSurface(pwINS, pInsOff);
    AG_WidgetSetSize(pwINS, nLedWidth, nLedHeight);

    nwKana[ID_ON] = 0;
    pwKana = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE , pKanaOn);
    nwKana[ID_OFF] = AG_PixmapAddSurface(pwKana, pKanaOff);
    AG_WidgetSetSize(pwKana, nLedWidth, nLedHeight);
}

void ClearLeds(void)
{
   	nCAP = -1;
	nKANA = -1;
	nINS = -1;
}

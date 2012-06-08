/*
 * agar_osd.cpp
 *
 *  Created on: 2010/11/26
 *      Author: whatisthis
 */

#include <SDL.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

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

#include <iconv.h>

/*
 * Global変数
 */
char StatusFont[MAXPATHLEN];

/*
 *  スタティック ワーク
 */
static char     szCaption[132];	/* キャプション */
static char     szOldCaption[132];	/* キャプション */
static int     nCAP;		/* CAPキー */
static int     nKANA;		/* かなキー */
static int     nINS;		/* INSキー */
static int     nDrive[2];	/* フロッピードライブ */
static int     nDriveOld[2];	/* フロッピードライブ */
static char    szDrive[2][16 + 1];	/* フロッピードライブ */
static char    szOldDrive[2][16+1];       /* フロッピードライブ(過去) */
static BOOL     old_writep[4];  /* 過去のライトプロテクトフラグ */
static int     nTape;           /* テープ */
static int     nOldTape;        /* テープ(過去) */
static AG_Surface      *pInsOn; /* INSキープリレンダリング(ON) */
static AG_Surface      *pInsOff; /* INSキープリレンダリング(OFF) */
static AG_Surface      *pKanaOn; /* カナキープリレンダリング(ON) */
static AG_Surface      *pKanaOff; /* カナキープリレンダリング(OFF) */
static AG_Surface      *pCapsOn; /* Capsキープリレンダリング(ON) */
static AG_Surface      *pCapsOff; /* Capsキープリレンダリング(OFF) */
static AG_Surface      *pFDRead[2]; /* Drive0 Read */
static AG_Surface      *pFDWrite[2]; /* Drive0 Write */
static AG_Surface      *pFDNorm[2]; /* Drive0 Normal */
static AG_Surface      *pCMTRead; /* Tape Read */
static AG_Surface      *pCMTWrite; /* Tape Write */
static AG_Surface      *pCMTNorm; /* Tape Normal */
static AG_Surface      *pCaption; /* Caption */

/*  リサイズ */
static int nLedWidth;
static int nLedHeight;
static int nVfdWidth;
static int nVfdHeight;
static int nCMTWidth;
static int nCMTHeight;
static int nCaptionWidth;
static int nCaptionHeight;
static int nStatFontSize;
static int nFontSize;

static AG_Font         *pStatusFont;
static AG_Color			r;
static AG_Color 		b;
static AG_Color			n;
static AG_Color 		black;
static AG_Color			alpha;
static BOOL bGLMode; /* 描画にOpenGLを使うか否か*/

/*
* Widget Textures
*/
enum {
    ID_EMPTY = 0,
    ID_IN,
    ID_READ,
    ID_WRITE,
    ID_END
};
enum {
    ID_OFF = 0,
    ID_ON
};


static AG_Pixmap *pwCaption;
static AG_Pixmap *pwFD[2];
static AG_Pixmap *pwCMT;
static AG_Pixmap *pwCAPS;
static AG_Pixmap *pwINS;
static AG_Pixmap *pwKana;
static int nwCaption;
static int nwFD[2][4]; // R/W/Noaccess/Empty
static int nwCMT[4]; // R/W/Noaccess/Empty
static int nwCaps[2]; //
static int nwIns[2];
static int nwKana[2];
extern "C" {
AG_Box *pStatusBar;
}

/*
 * Set Pixelformat of surfaces
 */
static void SetPixelFormat(AG_PixelFormat *fmt)
{
   if(fmt == NULL) return;
   // Surfaceつくる
	fmt->BitsPerPixel = 32;
	fmt->BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
	fmt->Rmask = 0x000000ff; // R
	fmt->Gmask = 0x0000ff00; // G
	fmt->Bmask = 0x00ff0000; // B
	fmt->Amask = 0xff000000; // A
#else
	fmt->Rmask = 0x00ff0000; // R
	fmt->Gmask = 0x0000ff00; // G
	fmt->Bmask = 0xff000000; // B
	fmt->Amask = 0x000000ff; // A
#endif
	fmt->Rshift = 0;
	fmt->Gshift = 8;
	fmt->Bshift = 16;
	fmt->Ashift = 24;
	fmt->Rloss = 0;
	fmt->Gloss = 0;
	fmt->Bloss = 0;
	fmt->Aloss = 0;
	fmt->palette = NULL;

}


static void UpdateCMTMessages(char *string)
{
    AG_Rect rect;
    AG_Surface *tmp;

   if(pStatusFont == NULL) return;
   if(nFontSize <= 2) return;
   if(pCMTRead == NULL) return;
   if(pCMTWrite == NULL) return;
   if(pCMTNorm == NULL) return;
   AG_PushTextState();
	AG_TextFont(pStatusFont);
        AG_TextFontPts(nFontSize);
	rect.x = 0;
	rect.y = 0;
	rect.h = nCMTHeight;
	rect.w = nCMTWidth;

        AG_SurfaceLock(pCMTRead);
        AG_FillRect(pCMTRead, &rect, r);
	AG_TextColor(black);
	AG_TextBGColor(r);
//        AG_ObjectLock(pCMTRead);
	tmp = AG_TextRender(string);
//         if(tmp->w < rect.w) rect.w = tmp->w;
//         if(tmp->h < rect.w) rect.h = tmp->h;
	AG_SurfaceBlit(tmp, &rect, pCMTRead, 0, 0);
	AG_SurfaceFree(tmp);
//        AG_ObjectUnlock(pCMTRead);
        AG_SurfaceUnlock(pCMTRead);



        AG_SurfaceLock(pCMTWrite);
	AG_FillRect(pCMTWrite, &rect, b);
	AG_TextColor(black);
	AG_TextBGColor(b);
	tmp = AG_TextRender(string);
//       if(tmp->w < rect.w) rect.w = tmp->w;
//         if(tmp->h < rect.w) rect.h = tmp->h;
	AG_SurfaceBlit(tmp, &rect, pCMTWrite, 0, 0);
	AG_SurfaceFree(tmp);
        AG_SurfaceUnlock(pCMTWrite);



        AG_SurfaceLock(pCMTNorm);
	AG_FillRect(pCMTNorm, &rect, black);
	AG_TextColor(n);
	AG_TextBGColor(black);
	tmp = AG_TextRender(string);
//         if(tmp->w < rect.w) rect.w = tmp->w;
//         if(tmp->h < rect.w) rect.h = tmp->h;
	AG_SurfaceBlit(tmp, &rect, pCMTNorm, 0, 0);
	AG_SurfaceFree(tmp);
        AG_SurfaceUnlock(pCMTNorm);
    AG_PopTextState();
}

static void UpdateVFDMessages(int drive, char *outstr)
{
    AG_Rect rect;
    AG_Surface *tmp;

   if(pStatusFont == NULL) return;
   if(nFontSize <= 2) return;

    AG_PushTextState();
	 AG_TextFont(pStatusFont);
        AG_TextFontPts(nFontSize);
	 AG_TextColor(alpha);
	 AG_TextBGColor(r);
	 rect.x = 0;
	 rect.y = 0;
	 rect.h = nVfdHeight;
	 rect.w = nVfdWidth;
	 AG_FillRect(pFDRead[drive], &rect, r);
	 tmp = AG_TextRender(outstr);
//         if(tmp->w < rect.w) rect.w = tmp->w;
//         if(tmp->h < rect.w) rect.h = tmp->h;
	 AG_SurfaceBlit(tmp, &rect, pFDRead[drive], 0, 0);
	 AG_SurfaceFree(tmp);

         AG_TextBGColor(b);
	 AG_FillRect(pFDWrite[drive], &rect, b);
	 tmp = AG_TextRender(outstr);
//       if(tmp->w < rect.w) rect.w = tmp->w;
//         if(tmp->h < rect.w) rect.h = tmp->h;
	 AG_SurfaceBlit(tmp, &rect, pFDWrite[drive], 0, 0);
	 AG_SurfaceFree(tmp);

	 AG_TextColor(n);
	 AG_TextBGColor(black);
	 AG_FillRect(pFDNorm[drive], &rect, black);
	 tmp = AG_TextRender(outstr);
//         if(tmp->w < rect.w) rect.w = tmp->w;
//         if(tmp->h < rect.w) rect.h = tmp->h;
	 AG_SurfaceBlit(tmp, &rect, pFDNorm[drive], 0, 0);
	 AG_SurfaceFree(tmp);
         AG_PopTextState();

}

static void InitMessages(AG_Widget *parent)
{
   AG_PixelFormat fmt;
   AG_Color col;
   AG_Rect rect;
   AG_Surface *p;
   AG_Box *dummy;
   int i;

   SetPixelFormat(&fmt);
    col.a = 255;
    col.b = 0;
    col.r = 0;
    col.g = 0;
    if(parent == NULL) return;
//    p = AG_SurfaceNew(AG_SURFACE_PACKED, nCaptionWidth, nCaptionHeight, &fmt, AG_SRCALPHA);
//    AG_FillRect(p, NULL, col);
    //dummy = AG_BoxNewHoriz(parent, AG_BOX_HFILL);
    if(pwCaption == NULL) pwCaption = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nCaptionWidth, nCaptionHeight);
     AG_WidgetSetSize(pwCaption, nCaptionWidth, nCaptionHeight);

    for(i = 1; i >= 0 ; i--) {
        if(pwFD[i] == NULL) pwFD[i] = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nVfdWidth, nVfdHeight);
           AG_WidgetSetSize(pwFD[i], nVfdWidth, nVfdHeight);
    }
    if(pwCMT == NULL) pwCMT = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nCMTWidth, nCMTHeight);
    AG_WidgetSetSize(pwCMT, nCMTWidth, nCMTHeight);
//    AG_SurfaceFree(p);
}


static void InitBox(AG_Widget *parent)
{
    int i;

    int height;

    height = 20;

    if(parent == NULL) return;
    AG_ObjectLock(parent);
    InitMessages(parent);

    if(pwCAPS == NULL) pwCAPS = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nLedWidth, nLedHeight);
    if(pwINS == NULL) pwINS = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nLedWidth, nLedHeight);
    if(pwKana == NULL) pwKana = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nLedWidth, nLedHeight);
    AG_WidgetSetSize(pwCAPS, nLedWidth, nLedHeight);
    AG_WidgetSetSize(pwINS, nLedWidth, nLedHeight);
    AG_WidgetSetSize(pwKana, nLedWidth, nLedHeight);
   AG_ObjectUnlock(parent);
}

static int LinkSurface(void)
{
    int i;
    int j;
    AG_Surface *p;
    AG_Rect rect;
    AG_Color col;
    int height = 20;

    // Caption
   nwCaption = 0;
 	  nwCaption = AG_PixmapAddSurface(pwCaption, pCaption);
   	  AG_PixmapUpdateCurrentSurface(pwCaption);
          AG_WidgetShow(pwCaption);
    // FD
   for(i = 0; i < 2 ; i++) {
		nwFD[i][ID_EMPTY] = 0;
		nwFD[i][ID_IN] = AG_PixmapAddSurface(pwFD[i], pFDNorm[i]);
		nwFD[i][ID_READ] = AG_PixmapAddSurface(pwFD[i], pFDRead[i]);
		nwFD[i][ID_WRITE] = AG_PixmapAddSurface(pwFD[i], pFDWrite[i]);
		AG_WidgetShow(pwFD[i]);
    }
    //CMT
    nwCMT[ID_EMPTY] = 0;
    nwCMT[ID_IN] = AG_PixmapAddSurface(pwCMT, pCMTNorm);
    nwCMT[ID_READ] = AG_PixmapAddSurface(pwCMT, pCMTRead);
    nwCMT[ID_WRITE] = AG_PixmapAddSurface(pwCMT, pCMTWrite);
   AG_WidgetShow(pwCMT);

    // CAPS
    nwCaps[ID_OFF] = 0;
    nwCaps[ID_ON] = AG_PixmapAddSurface(pwCAPS, pCapsOn);
   AG_WidgetShow(pwCAPS);

   // INS
    nwIns[ID_OFF] = 0;
    nwIns[ID_ON] = AG_PixmapAddSurface(pwINS, pInsOn);
	AG_WidgetShow(pwINS);

    // Kana
    nwKana[ID_OFF] = 0;
    nwKana[ID_ON] = AG_PixmapAddSurface(pwKana, pKanaOn);
	AG_WidgetShow(pwKana);
}

/*
* OSDを表示する実体
* (1280x880) 空間で、(x, 878) - (x, 878 -h)に表示される
*/


void DrawOSDGL(AG_GLView *w)
{
    return;
}

void DrawOSDEv(AG_Event *event)
{
    return;
}

static void CreateLEDs(AG_Widget *parent, BOOL initflag)
{
    AG_PixelFormat fmt;
    AG_Surface *tmps;

   SetPixelFormat(&fmt);

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
   if(nFontSize <= 2) return;


    AG_PushTextState();
	AG_TextFont(pStatusFont);
        AG_TextFontPts(nFontSize);
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

static void CreateVFD(AG_Widget *parent, BOOL initflag)
{
    AG_PixelFormat fmt;
    AG_Rect rec;
    int i;

   SetPixelFormat(&fmt);

   if(pStatusFont == NULL) return;

   rec.x = 0;
   rec.y = 0;
   rec.w = nVfdWidth;
   rec.h = nVfdHeight;
    for(i = 0; i < 2; i++){

#if 0
       if(pFDRead[i] != NULL){
            AG_SurfaceResize(pFDRead[i], rec.w, rec.h);
        } else {
	    pFDRead[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	}

        if(pFDWrite[i] != NULL){
            AG_SurfaceResize(pFDWrite[i],rec.w, rec.h);
        } else {
	   pFDWrite[i] =AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	}

        if(pFDNorm[i] != NULL){
            AG_SurfaceResize(pFDNorm[i], rec.w, rec.h);
        } else {
	    pFDNorm[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	}
#else
       if(pFDRead[i] != NULL){
            AG_SurfaceFree(pFDRead[i]);
        }
       if(pFDWrite[i] != NULL){
            AG_SurfaceFree(pFDWrite[i]);
        }
       if(pFDNorm[i] != NULL){
            AG_SurfaceFree(pFDNorm[i]);
        }
       pFDRead[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
       pFDWrite[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
       pFDNorm[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
#endif
       AG_FillRect(pFDRead[i], NULL, r);
	AG_FillRect(pFDWrite[i], NULL, b);
	AG_FillRect(pFDNorm[i], NULL, alpha);
       UpdateVFDMessages(i, "               ");
    }
}

static void CreateCMT(AG_Widget *parent, bool initflag)
{
    AG_Rect rec;
    AG_PixelFormat fmt;

   SetPixelFormat(&fmt);


   if(pStatusFont == NULL) return;

   rec.x = 0;
   rec.y = 0;
   rec.w = nCMTWidth;
   rec.h = nCMTHeight;
  if(pCMTRead != NULL){
     AG_SurfaceResize(pCMTRead, rec.w, rec.h);
  } else {
     pCMTRead = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
  }
  if(pCMTNorm != NULL){
     AG_SurfaceResize(pCMTNorm, rec.w, rec.h);
  } else {
     pCMTNorm = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
  }
  if(pCMTWrite != NULL){
     AG_SurfaceResize(pCMTWrite, rec.w, rec.h);
  } else {
     pCMTWrite = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
  }

   AG_FillRect(pCMTRead, NULL, r);
   AG_FillRect(pCMTWrite, NULL, b);
   AG_FillRect(pCMTNorm, NULL, black);
    UpdateCMTMessages("     ");
}

static void CreateCaption(AG_Widget *parent, BOOL initflag)
{
    AG_Rect rec;
    AG_PixelFormat fmt;

    SetPixelFormat(&fmt);

    if((pCaption != NULL) && (initflag != TRUE)){
        AG_SurfaceResize(pCaption, nCaptionWidth, nCaptionHeight);
    } else {
       rec.x = 0;
	rec.y = 0;
	rec.w = nCaptionWidth;
	rec.h = nCaptionHeight;
	pCaption = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	AG_FillRect(pCaption, &rec,  black);
    }

}
/*-[ ステータスバー ]-------------------------------------------------------*/
/*
 *  ステータスバーの生成
 */
void CreateStatus(AG_Widget *parent)
{
	AG_Rect rec;
	AG_Surface *tmps;
	AG_PixelFormat fmt;
	int i;

	r.r = 255; // r->g
	r.g = 0; // g->b
	r.b = 0;  // b->r
	r.a = 255; // a->a

	b.r = 0;
	b.g = 0;
	b.b = 255;
	b.a = 255;

	n.r = 255;
	n.g = 255;
	n.b = 255;
	n.a = 255;

	black.r = 0;
	black.g = 0;
	black.b = 0;
	black.a = 255; //
	// アルファチャンネル
	alpha.r = 1;
	alpha.g = 1;
	alpha.b = 1;
	alpha.a = 180; //

	// Surfaceつくる
    SetPixelFormat(&fmt);
	pStatusFont =  AG_FetchFont (STAT_FONT,STAT_PT * 4, 0);
	// Init Var
	nLedWidth = LED_WIDTH * 2;
	nLedHeight = LED_HEIGHT * 2;
	pInsOn = pInsOff = NULL;
	pCapsOn = pCapsOff = NULL;
	pKanaOn = pKanaOff = NULL;
	pwINS = NULL;
	pwCAPS = NULL;
	pwKana = NULL;
	// Create Widgets;
	nFontSize = STAT_PT;
   CreateLEDs(parent, TRUE);
    pFDRead[0] = pFDWrite[0] = pFDNorm[0] = NULL;
    pFDRead[1] = pFDWrite[1] = pFDNorm[1] = NULL;
    nVfdHeight = VFD_HEIGHT * 2;
    nVfdWidth = VFD_WIDTH * 2;
    pwFD[0] = pwFD[1] = NULL;
    CreateVFD(parent, TRUE);

    pCMTRead = pCMTWrite = pCMTNorm = NULL;
    pwCMT = NULL;
    nCMTHeight = CMT_HEIGHT * 2;
    nCMTWidth = CMT_WIDTH * 2;
    CreateCMT(parent, TRUE);
	/*
	 * RECT Tape
	 */

    pCaption = NULL;
    pwCaption = NULL;
    nCaptionHeight = STAT_HEIGHT * 2;
    nCaptionWidth = STAT_WIDTH * 2;
    CreateCaption(parent, TRUE);
    if(parent) {
        InitBox(parent);
        LinkSurface();
    }
}

void DestroyStatus(void)
{
	int i, j;
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
	if(pCaption !=NULL ) {
		AG_SurfaceFree(pCaption);
		pCaption = NULL;
	}
	for(i = 0 ; i < 2 ; i++ ) {
		if(pFDRead[i] != NULL) {
			AG_SurfaceFree(pFDRead[i]);
			pFDRead[i] = NULL;
		}
		if(pFDWrite[i] != NULL) {
			AG_SurfaceFree(pFDWrite[i]);
			pFDWrite[i] = NULL;
		}
		if(pFDNorm[i] != NULL) {
			AG_SurfaceFree(pFDNorm[i]);
			pFDNorm[i] = NULL;
		}
	}
	if(pCMTRead !=NULL ) {
		AG_SurfaceFree(pCMTRead);
		pCMTRead = NULL;
	}
	if(pCMTWrite !=NULL ) {
		AG_SurfaceFree(pCMTWrite);
		pCMTWrite = NULL;
	}
	if(pCMTNorm !=NULL ) {
		AG_SurfaceFree(pCMTNorm);
		pCMTNorm = NULL;
	}
	if(pStatusFont != NULL) {
		AG_DestroyFont(pStatusFont);
		pStatusFont = NULL;
	}
}
/*
 *  キャプション描画
 */
static BOOL UpdateMainCaption(BOOL override)
{
   	char           string[1024];
	char           tmp[128];
	char          *p;
	AG_Surface   *tmps;
	AG_Rect rect;
	AG_Font *fp;

	/*
	 * 動作状況に応じて、コピー
	 */
	if (run_flag) {
		strcpy(string, "XM7[実行]");
	}
	else {
		strcpy(string, "XM7[停止]");
	}
	// Add Bootmode
    if(boot_mode == BOOT_BASIC){
        strcat(string, "[BAS]");
    } else if(boot_mode == BOOT_DOS) {
        strcat(string, "[DOS]");
    } else {
        strcat(string, "[???]");
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
		p = strrchr(fdc_fname[0], '/');
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
			p = strrchr(fdc_fname[1], '/');
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
	if (tape_fileh != -1) {

		/*
		 * ファイルネーム＋拡張子のみ取り出す
		 */
		p = strrchr(tape_fname, '/');
		if (p == NULL) {
			p = tape_fname;
		} else {
			p++;
		}
		sprintf(tmp, "- %s ", p);
		strcat(string, tmp);
	}

	/*
	 * 比較描画
	 */

	strncpy(szCaption, string, 128);
	if((strncmp(szOldCaption, szCaption, 128) != 0) || (override == TRUE)) {
	strncpy(szOldCaption, szCaption, 128);
        if(pCaption == NULL) return FALSE;
        if(pwCaption == NULL) return FALSE;
        if(pStatusFont == NULL) return FALSE;
	if(nFontSize <= 2) return FALSE;
	if(strlen(szCaption) <= 0) return TRUE; 
	   AG_PushTextState();
	   AG_TextFont(pStatusFont);
	   AG_TextFontPts(nFontSize);

	   AG_TextColor(n);
	   AG_TextBGColor(black);
	   AG_FillRect(pCaption, NULL, black);
	   tmps = AG_TextRender(szCaption);
	   rect.x = 0;
	   rect.y = 0;
	   rect.h = nCaptionHeight;
	   rect.w = nCaptionWidth;
//	   if((pCaption != NULL) && (rect.h > 10) && (rect.w > 10)) {
	      AG_SurfaceBlit(tmps, &rect, pCaption, 2, 0);
//	   }

	   AG_SurfaceFree(tmps);
	   AG_PopTextState();
	}
   return TRUE;
}


static void DrawMainCaption(BOOL override)
{
   if( !UpdateMainCaption(override)) return;
   if(pwCaption != NULL) {
	AG_ObjectLock(pwCaption);
	      AG_PixmapSetSurface(pwCaption, nwCaption);
	      AG_PixmapUpdateSurface(pwCaption, nwCaption);
	      AG_Redraw(pwCaption);
	AG_ObjectUnlock(pwCaption);
	}
}



/*
 *  CAPキー描画
 */
static void DrawCAP(void)
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
static void DrawKANA(void)
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
static void DrawINS(void)
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

/*
 *  ドライブ描画
 */
static void DrawDrive(int drive, BOOL override)
{
	int            num;
	int i;
	char          name[128];
	char          string[64];
	char          utf8[256];
	char		outstr[300];
	char          *pIn, *pOut;
	iconv_t       hd;
	size_t        in, out;
	AG_Surface *tmp;
	AG_Rect		rect;

	memset(string, 0x00, sizeof(string));
	memset(utf8, 0x00, sizeof(utf8));
	memset(outstr, 0x00, sizeof(outstr));


	ASSERT((drive >= 0) && (drive <= 1));

	if((pFDRead[drive] == NULL) || (pFDWrite[drive] == NULL) || (pFDNorm[drive] == NULL)) return;

	/*
	 * 番号セット
	 */
	 if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
		 num = 255;
	 }  else {
		 num = fdc_access[drive];
		 if (num == FDC_ACCESS_SEEK) {
			 num = FDC_ACCESS_READY;
		 }
	 }

	 /*
	  * 名前取得
	  */
	 name[0] = '\0';
	 utf8[0] = '\0';
	 if (fdc_ready[drive] == FDC_TYPE_D77) {
		 strncpy(name, fdc_name[drive][fdc_media[drive]], 126);
	 }
	 if (fdc_ready[drive] == FDC_TYPE_2D) {
		 strcpy(name, "2D DISK");
	 }
	 if (fdc_ready[drive] == FDC_TYPE_VFD) {
		 strcpy(name, "VFD DISK");
	 }

	 /*
	  * 番号比較
	  */
	 if (nDrive[drive] == num) {
		 if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
			 if(strlen(szDrive[drive]) > 0) {
				 //レンダリング
			 }
			 szDrive[drive][0] = '\0';
			 szOldDrive[drive][0] = '\0';
			 /*
			  * テクスチャ破棄必要か？
			  */
			 return;
		 }
		 if (strcmp(szDrive[drive], name) == 0) {
		 }
	 }

	 /*
	  * 描画
	  */
	 nDrive[drive] = num;
	 memset(szDrive[drive], 0, 16);
	 strncpy(szDrive[drive], name, 16);
	 if (nDrive[drive] == 255) {
		 strcpy(string, "");
	 } else {
		 strcpy(string, szDrive[drive]);
	 }


	 if((strcmp(szDrive[drive], szOldDrive[drive]) != 0) || (old_writep[drive] != fdc_writep[drive]) || (override == TRUE)) {
		 /*
		  * 過去のファイルネームと違うのでフォントレンダリングする
		  */
		 memset(szOldDrive[drive], 0, 16);
		 strncpy(szOldDrive[drive], szDrive[drive], 16);
	    if(pwFD[drive] == NULL) return;
		 pIn = string;
		 pOut = utf8;
		 in = strlen(pIn);

		 out = 256;
		 hd = iconv_open("utf8", "cp932");
		 if(hd >= 0) {
			 while(in>0) {
				 iconv(hd, &pIn, &in, &pOut, &out);
			 }
			 iconv_close(hd);
		 }
		 if(strlen(utf8) >0) {
			 if(fdc_writep[drive]) {
				 sprintf(outstr, "■ %s", utf8); // 書込み禁止
			 } else {
				 sprintf(outstr, "　 %s", utf8); // 書込み許可
			 }
			 old_writep[drive] = fdc_writep[drive];
		 }
	    if(pwFD[drive] != NULL){
	       AG_ObjectLock(pwFD[drive]);
	       UpdateVFDMessages(drive, outstr);
	       AG_PixmapUpdateSurface(pwFD[drive], nwFD[drive][ID_IN]);
	       AG_PixmapUpdateSurface(pwFD[drive], nwFD[drive][ID_READ]);
	       AG_PixmapUpdateSurface(pwFD[drive], nwFD[drive][ID_WRITE]);
	       AG_ObjectUnlock(pwFD[drive]);
	    }
	 }
	 if (nDrive[drive] == FDC_ACCESS_READ) {
	    if(nwFD[drive][ID_READ] >= 0)  AG_PixmapSetSurface(pwFD[drive], nwFD[drive][ID_READ]);
	 } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
	    if(nwFD[drive][ID_WRITE] >= 0) AG_PixmapSetSurface(pwFD[drive], nwFD[drive][ID_WRITE]);
	 } else {
	    if(nwFD[drive][ID_IN] >= 0) AG_PixmapSetSurface(pwFD[drive], nwFD[drive][ID_IN]);
	 }
	   AG_Redraw(pwFD[drive]);
}


/*
 *  テープ描画
 */
static void DrawTape(int override)
{
	int 		i;
	int             num;
	char            string[128];
	char     protect[16];
	AG_Rect rect;
	AG_Surface *tmp;


	if(tape_writep){
		strcpy(protect, "■");
	} else {
		strcpy(protect, "　");
	}
	/*
	 * ナンバー計算
	 */
	num = 30000;
	if (tape_fileh != -1) {
		num = (int) ((tape_offset >> 8) % 10000);
		if (tape_motor) {
			if (tape_rec) {
				num += 20000;
			}

			else {
				num += 10000;
			}
		}
	}

	/*
	 * 番号比較
	 */
	 if ((nTape == num) && (override != TRUE)){
		 return;
	 }

	/*
	 * 描画
	 */
	 nTape = num;
	if (nTape >= 30000) {
		string[0] = '\0';
		//strcpy(string, "OVER");
	}
	else {
		sprintf(string, "%s%04d", protect, nTape % 10000);
	}
	if((nOldTape != nTape)  || (nTape == 0) || (override = TRUE)){
		nOldTape = nTape;
        if(pwCMT == NULL) return;
		/*
		 * カウンタ番号レンダリング(仮)
		 */
		if(pStatusFont != NULL) {
		   if(pwCMT != NULL){
		   AG_ObjectLock(pwCMT);
		   UpdateCMTMessages(string);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);
		   AG_ObjectUnlock(pwCMT);
		   }
		}
	}
		if ((nTape >= 10000) && (nTape < 30000)) {
			if (nTape >= 20000) {
			   if(nwCMT[ID_WRITE] >= 0) {
			      AG_PixmapSetSurface(pwCMT, nwCMT[ID_WRITE]);
			   }
			}   else {
			   if(nwCMT[ID_READ] >= 0) {
			      AG_PixmapSetSurface(pwCMT, nwCMT[ID_READ]);
			   }
			}
		} else {
		   if(nwCMT[ID_IN] >= 0) {
			   AG_PixmapSetSurface(pwCMT, nwCMT[ID_IN]);
		   }
	       }
	   AG_Redraw(pwCMT);
}

/*
 *  描画
 */

void DrawStatus(void)
{
    DrawMainCaption(FALSE);
	DrawCAP();
	DrawKANA();
	DrawINS();
	DrawDrive(0, FALSE);
	DrawDrive(1, FALSE);
	DrawTape(FALSE);
}


/*
 *  強制描画
 */
void DrawStatusForce(void)
{
	DrawMainCaption(TRUE);
	DrawCAP();
	DrawKANA();
	DrawINS();
	DrawDrive(0, TRUE);
	DrawDrive(1, TRUE);
	DrawTape(TRUE);
}

void ResizeStatus(AG_Widget *parent, int w, int h, int y)
{
    int i;
    int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3 + 50;
    int pos = 0;
    int ww = (float)w;
    float wLed = (float)LED_WIDTH / (float)total;
    float wCMT = (float)CMT_WIDTH / (float)total;
    float wFD = (float)VFD_WIDTH / (float)total;
    float wCaption = (float)STAT_WIDTH / (float)total;
    AG_Box *pad;
    if(parent == NULL) return;
	return;

   nLedHeight = nVfdHeight = nCMTHeight = nCaptionHeight = h;
    nLedWidth = (int)(ww * wLed);
    nVfdWidth = (int)(ww * wFD);
    nCMTWidth = (int)(ww * wCMT);
    nCaptionWidth = (int)(ww * wCaption);
    if(nLedWidth <= 0) return;
    if(nVfdWidth <= 0) return;
    if(nCMTWidth <= 0) return;
    if(nCaptionWidth <= 0) return;

   nFontSize = (int)(STAT_PT * (float)h * 1.0f) / (STAT_HEIGHT * 2.0f);

    if(parent) {
    AG_ObjectLock(AGOBJECT(parent));

       AG_WidgetHide(pwCAPS);
       AG_ObjectDetach(AGOBJECT(pwCAPS));
       AG_WidgetHide(pwINS);
       AG_ObjectDetach(AGOBJECT(pwINS));
       AG_WidgetHide(pwKana);
       AG_ObjectDetach(AGOBJECT(pwKana));
       for(i = 0; i < 2; i++) {
	  AG_WidgetHide(pwFD[i]);
	  AG_ObjectDetach(AGOBJECT(pwFD[i]));
       }
       AG_WidgetHide(pwCMT);
       AG_ObjectDetach(AGOBJECT(pwCMT));
       if(pwCaption != NULL) {
	  AG_WidgetHide(pwCaption);
	  AG_ObjectDetach(AGOBJECT(pwCaption));
	  pwCaption = NULL;
       }

       pad = AG_BoxNewHoriz(parent, AG_HBOX_VFILL);

//       CreateCaption(parent, FALSE);
       UpdateMainCaption(TRUE);
       {
	  pwCaption = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE , pCaption);
	  nwCaption = 0;
	  AG_WidgetSetSize(pwCaption, nCaptionWidth, nCaptionHeight);
	  AG_PixmapUpdateSurface(pwCaption, nwCaption);
	  AG_PixmapSetSurface(pwCaption, nwCaption);
	  AG_WidgetShow(pwCaption);
	  AG_Redraw(pwCaption);
       }
//       pad = AG_BoxNewHoriz(parent, 0);
//       CreateVFD(parent, FALSE);
       for(i = 1; i >= 0 ; i--) {
	  nwFD[i][ID_EMPTY] = 0;
	  UpdateVFDMessages(i, "               ");
	  nwFD[i][ID_IN] = 0;
	  pwFD[i] = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE , pFDNorm[i]);
	  nwFD[i][ID_READ] = AG_PixmapAddSurface(pwFD[i], pFDRead[i]);
	  nwFD[i][ID_WRITE] = AG_PixmapAddSurface(pwFD[i], pFDWrite[i]);
	  AG_PixmapUpdateSurface(pwFD[i], nwFD[i][ID_IN]);
	  AG_PixmapUpdateSurface(pwFD[i], nwFD[i][ID_READ]);
	  AG_PixmapUpdateSurface(pwFD[i], nwFD[i][ID_WRITE]);
	  AG_PixmapSetSurface(pwFD[i], nwFD[i][ID_IN]);
	  AG_WidgetSetSize(pwFD[i], nVfdWidth, nVfdHeight);
	  AG_WidgetShow(pwFD[i]);
	  AG_Redraw(pwFD[i]);
       }

//       pad = AG_BoxNewHoriz(parent, 0);
//       CreateCMT(parent, FALSE);
       UpdateCMTMessages("       ");
       nwCMT[ID_IN] = 0;
       pwCMT = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE , pCMTNorm);
       nwCMT[ID_READ] = AG_PixmapAddSurface(pwCMT, pCMTRead);
       nwCMT[ID_WRITE] = AG_PixmapAddSurface(pwCMT, pCMTWrite);
       AG_WidgetSetSize(pwCMT, nCMTWidth, nCMTHeight);
       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);
       AG_PixmapSetSurface(pwCMT, nwCMT[ID_IN]);
       AG_WidgetShow(pwCMT);


//       pad = AG_BoxNewHoriz(parent, 0);
//       CreateLEDs(parent, FALSE);
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

       AG_ObjectUnlock(AGOBJECT(parent));

    }
   DrawStatusForce();
}

/*
 *  再描画
 */
void PaintStatus(void)
{
	AG_Surface *p;
	/*
	 * 記憶ワークをすべてクリアする
	 */
	szCaption[0] = '\0';
	szOldCaption[0] = '\0';
	nCAP = -1;
	nKANA = -1;
	nINS = -1;
	nDrive[0] = -1;
	nDrive[1] = -1;
	nDriveOld[0] = -1;
	nDriveOld[1] = -1;
	szDrive[0][0] = '\0';
	szDrive[1][0] = '\0';
	szOldDrive[0][0] = '\0';
	szOldDrive[1][0] = '\0';
	nTape = -1;
	nOldTape = 0;

	/*
	 * 描画
	 */
     DrawStatusForce();
}

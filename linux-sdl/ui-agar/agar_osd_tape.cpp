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

static int     nTape;           /* テープ */
static int     nOldTape;        /* テープ(過去) */
static AG_Pixmap *pwCMT;
static AG_Surface      *pCMTReadLetter[10]; /* Tape Read */
static AG_Surface      *pCMTWriteLetter[10]; /* Tape Write */
static AG_Surface      *pCMTNormLetter[10]; /* Tape Normal */
static AG_Surface      *pCMTRead; /* Tape Read */
static AG_Surface      *pCMTWrite; /* Tape Write */
static AG_Surface      *pCMTNorm; /* Tape Normal */
static int nCMTWidth;
static int nCMTHeight;
static int nwCMT[4]; // R/W/Noaccess/Empty
static AG_Surface *pCMTLetters[10]; // 0123456789
static AG_Surface *pCMTBlank; // 0123456789
static BOOL bTapeRecOld;
static BOOL bTapeInOld;

extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;

static void DetachTapeLetters(void)
{
    int i;
    for(i = 0; i < 10; i++){
        if(pCMTReadLetter[i] != NULL) AG_SurfaceFree(pCMTReadLetter[i]);
        if(pCMTWriteLetter[i] != NULL) AG_SurfaceFree(pCMTWriteLetter[i]);
        if(pCMTNormLetter[i] != NULL) AG_SurfaceFree(pCMTNormLetter[i]);
        pCMTWriteLetter[i] = NULL;
        pCMTReadLetter[i] = NULL;
        pCMTNormLetter[i] = NULL;
    }

}
static void InitTapeLetters(void)
{
   int i;
   int size = getnFontSize();
   char table[10][4] = { "0", "1", "2", "3", "4",
                        "5", "6", "7", "8", "9"};
   AG_Color r, n, b, black;
   AG_Rect rec;
   AG_PixelFormat fmt;

   char string[16];
   AG_Surface *tmp;
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
   SetPixelFormat(&fmt);


   AG_PushTextState();
   AG_TextFont(pStatusFont);
   AG_TextFontPts(size);
   rec.x = 0;
   rec.y = 0;

   for(i = 0; i < 10; i++){
    strcpy(string, table[i]);
	AG_TextColor(black);
	AG_TextBGColor(r);
	tmp = AG_TextRender(string);
    rec.h = CMT_HEIGHT;
    rec.w = tmp->w;
    pCMTReadLetter[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
    AG_FillRect(pCMTReadLetter[i], &rec, r);
	AG_SurfaceBlit(tmp, NULL, pCMTReadLetter[i], 0, 0);
	AG_SurfaceFree(tmp);

	AG_TextColor(black);
	AG_TextBGColor(b);
	tmp = AG_TextRender(string);
    rec.h = CMT_HEIGHT;
    rec.w = tmp->w;
    pCMTWriteLetter[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
    AG_FillRect(pCMTWriteLetter[i], &rec, b);
	AG_SurfaceBlit(tmp, NULL, pCMTWriteLetter[i], 0, 0);
	AG_SurfaceFree(tmp);

	AG_TextColor(n);
	AG_TextBGColor(black);
	tmp = AG_TextRender(string);
    rec.h = CMT_HEIGHT;
    rec.w = tmp->w;

    pCMTNormLetter[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	AG_FillRect(pCMTNormLetter[i], &rec, black);
	AG_SurfaceBlit(tmp, NULL, pCMTNormLetter[i], 0, 0);
	AG_SurfaceFree(tmp);
   }
    strcpy(string, "       ");
    pCMTBlank = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);

    AG_TextColor(n);
    AG_TextBGColor(black);
    tmp = AG_TextRender(string);
    AG_SurfaceBlit(tmp, NULL, pCMTBlank, 0, 0);
    AG_SurfaceFree(tmp);
    AG_PopTextState();

}


static AG_Surface *UpdateCMTSub(int n, int stat)
{
   switch(stat){
    case ID_IN:
      return pCMTNormLetter[n % 10];
      break;
    case ID_READ:
      return pCMTReadLetter[n % 10];
      break;
    case ID_WRITE:
      return pCMTWriteLetter[n % 10];
      break;
    default:
      break;
   }
 return NULL;
}

static void SetCMTBlank()
{
   AG_Color r, b, n, black;
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
   if(pCMTRead == NULL) return;
   if(pCMTWrite == NULL) return;
   if(pCMTNorm == NULL) return;
   AG_FillRect(pCMTRead, NULL, r);
   AG_FillRect(pCMTWrite, NULL, b);
   AG_FillRect(pCMTNorm, NULL, black);
   if(pwCMT == NULL) return;
   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);
}

static void UpdateCMTCount(int count)
{
   AG_Rect rect;
   AG_Surface *tmp;
   int i;
   int x;
   int y;
   int w;
   int h;
   int size;
   char string[16];
   AG_Color r, b, n, black;

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

   if(pCMTRead == NULL) return;
   if(pCMTWrite == NULL) return;
   if(pCMTNorm == NULL) return;

   sprintf(string, "%04d", count);
   AG_SurfaceLock(pCMTRead);
   AG_SurfaceLock(pCMTWrite);
   AG_SurfaceLock(pCMTNorm);
//   AG_ObjectLock(pStatusFont);
   size = getnFontSize();
   AG_PushTextState();
   AG_TextFont(pStatusFont);
   AG_TextFontPts(size);

   AG_TextColor(black);
   AG_TextBGColor(r);
   tmp = AG_TextRender(string);
   AG_FillRect(pCMTRead, NULL, r);
   AG_SurfaceBlit(tmp, NULL, pCMTRead, 0, 0);
   AG_SurfaceFree(tmp);

   AG_TextColor(black);
   AG_TextBGColor(b);
   tmp = AG_TextRender(string);
   AG_FillRect(pCMTWrite, NULL, b);
   AG_SurfaceBlit(tmp, NULL, pCMTWrite, 0, 0);
   AG_SurfaceFree(tmp);

   AG_TextColor(n);
   AG_TextBGColor(black);
   tmp = AG_TextRender(string);
   AG_FillRect(pCMTNorm, NULL, black);
   AG_SurfaceBlit(tmp, NULL, pCMTNorm, 0, 0);
   AG_SurfaceFree(tmp);
   AG_PopTextState();
//   AG_ObjecUnLock(pStatusFont);
   AG_SurfaceUnlock(pCMTRead);
   AG_SurfaceUnlock(pCMTWrite);
   AG_SurfaceUnlock(pCMTNorm);

   return;

   x = 4;
   tmp = UpdateCMTSub(count / 1000, ID_READ);
   w = tmp->w;
   h = tmp->h;
   rect.w = w;
   rect.h = h;
   rect.x = 0;
   rect.y = 0;

//   tmp = UpdateCMTSub(count / 1000, ID_READ);
   AG_SurfaceBlit(tmp, &rect, pCMTRead, x, 0);
   tmp = UpdateCMTSub(count / 1000, ID_WRITE);
   AG_SurfaceBlit(tmp, &rect, pCMTWrite, x, 0);
   tmp = UpdateCMTSub(count / 1000, ID_IN);
   AG_SurfaceBlit(tmp, &rect, pCMTNorm, x, 0);
   x = x + w + 2;

   tmp = UpdateCMTSub(count / 100, ID_READ);
   AG_SurfaceBlit(tmp, &rect, pCMTRead, x, 0);
   tmp = UpdateCMTSub(count / 100, ID_WRITE);
   AG_SurfaceBlit(tmp, &rect, pCMTWrite, x, 0);
   tmp = UpdateCMTSub(count / 100, ID_IN);
   AG_SurfaceBlit(tmp, &rect, pCMTNorm, x, 0);
   x = x + w + 2;

   tmp = UpdateCMTSub(count / 10, ID_READ);
   AG_SurfaceBlit(tmp, &rect, pCMTRead, x, 0);
   tmp = UpdateCMTSub(count / 10, ID_WRITE);
   AG_SurfaceBlit(tmp, &rect, pCMTWrite, x, 0);
   tmp = UpdateCMTSub(count / 10, ID_IN);
   AG_SurfaceBlit(tmp, &rect, pCMTNorm, x, 0);
   x = x + w + 2;

   tmp = UpdateCMTSub(count % 10, ID_READ);
   AG_SurfaceBlit(tmp, &rect, pCMTRead, x, 0);
   tmp = UpdateCMTSub(count % 10, ID_WRITE);
   AG_SurfaceBlit(tmp, &rect, pCMTWrite, x, 0);
   tmp = UpdateCMTSub(count % 10, ID_IN);
   AG_SurfaceBlit(tmp, &rect, pCMTNorm, x, 0);
//   x = x + w + 2;

   AG_SurfaceUnlock(pCMTRead);
   AG_SurfaceUnlock(pCMTWrite);
   AG_SurfaceUnlock(pCMTNorm);

//   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
//   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
//   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);

}


void CreateCMT(AG_Widget *parent, bool initflag)
{
   AG_Rect rec;
   AG_PixelFormat fmt;
   AG_Color r, b, n, black;
   bTapeRecOld = FALSE;
   bTapeInOld = FALSE;
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

   SetPixelFormat(&fmt);


   if(pStatusFont == NULL) return;

   rec.x = 0;
   rec.y = 0;
   rec.w = nCMTWidth;
   rec.h = nCMTHeight;
    DetachTapeLetters();

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
   InitTapeLetters();

   AG_FillRect(pCMTRead, NULL, r);
   AG_FillRect(pCMTWrite, NULL, b);
   AG_FillRect(pCMTNorm, NULL, black);
}


void InitTapeOSD(AG_Widget *parent)
{
    if(parent == NULL) return;
    nCMTHeight = CMT_HEIGHT;
    nCMTWidth = CMT_WIDTH;
    CreateCMT(parent, TRUE);
    pwCMT = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE, pCMTNorm);
//    pwCMT = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nCMTWidth, nCMTHeight);
    AG_WidgetSetSize(pwCMT, nCMTWidth, nCMTHeight);

    nwCMT[ID_EMPTY] = 0;
}

void DestroyTapeOSD(void)
{

   if(pCMTNorm != NULL) AG_SurfaceFree(pCMTNorm);
   if(pCMTRead != NULL) AG_SurfaceFree(pCMTRead);
   if(pCMTWrite != NULL) AG_SurfaceFree(pCMTWrite);
   pCMTNorm = pCMTRead = pCMTWrite = NULL;
   if(pwCMT != NULL) AG_ObjectDetach(AGOBJECT(pwCMT));
   pwCMT = NULL;

}


void LinkSurfaceCMT(void)
{
    nwCMT[ID_EMPTY] = 0;
    nwCMT[ID_IN] = AG_PixmapAddSurface(pwCMT, pCMTNorm);
    nwCMT[ID_READ] = AG_PixmapAddSurface(pwCMT, pCMTRead);
    nwCMT[ID_WRITE] = AG_PixmapAddSurface(pwCMT, pCMTWrite);
}

void ResizeTapeOSD(AG_Widget *parent, int w, int h)
{
       int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3 + 50;
       float ww = (float)w;
       float wCMT = (float)CMT_WIDTH / (float)total;

       AG_ObjectLock(pwCMT);
       AG_WidgetHide(pwCMT);
       AG_ObjectDetach(AGOBJECT(pwCMT));

       nCMTWidth = (int)(ww * wCMT);
       nCMTHeight = h;
//       pwCMT = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nCMTWidth, nCMTHeight);
       pwCMT = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE, pCMTNorm);
       AG_WidgetSetSize(pwCMT, nCMTWidth, nCMTHeight);
       CreateCMT(parent, FALSE);
       LinkSurfaceCMT();
       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);
   //    AG_ObjectUnlock(pwCMT);
      AG_WidgetShow(pwCMT);
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
    if(pCMTNorm == NULL) return;
    if(pCMTWrite == NULL) return;
    if(pCMTRead == NULL) return;
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
    AG_ObjectLock(pwCMT);

	if ((!bTapeIn) && (bTapeIn != bTapeInOld)) {
	        if(pwCMT == NULL) return;
	        SetCMTBlank();
            AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
		    AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
		    AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);
            bTapeInOld = bTapeIn;
            nTape = 0;
	} else if((num != nTape)  || (bTapeInOld != bTapeIn)
       || (override = TRUE)){
		bTapeInOld = bTapeIn;
		/*
		 * カウンタ番号レンダリング(仮)
		 */
		   if(pwCMT != NULL){
		      if(bTapeIn) UpdateCMTCount(num % 10000);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);
		   }
	}

		if (((nTape != num) || (bTapeRec != bTapeRecOld))
            && bTapeIn) {
			nTape = num;
			bTapeRecOld = bTapeRec;
			bTapeInOld = bTapeIn;
			if (bTapeRec) {
			   if(nwCMT[ID_WRITE] >= 0) {
			      AG_PixmapSetSurface(pwCMT, nwCMT[ID_WRITE]);
			   }
			}   else {
			   if(nwCMT[ID_READ] >= 0) {
			      AG_PixmapSetSurface(pwCMT, nwCMT[ID_READ]);
			   }
			}
		} else if(bTapeIn){
   			nTape = num;
			bTapeRecOld = bTapeRec;
			bTapeInOld = bTapeIn;
		   if(nwCMT[ID_IN] >= 0) {
			   AG_PixmapSetSurface(pwCMT, nwCMT[ID_IN]);
		   }
        } else {
			nTape = num;
			bTapeRecOld = bTapeRec;
			bTapeInOld = bTapeIn;
        }

	   AG_ObjectUnlock(pwCMT);
//	   AG_Redraw(pwCMT);
}

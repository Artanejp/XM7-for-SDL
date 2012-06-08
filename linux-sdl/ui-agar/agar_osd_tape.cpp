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


extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;


static void InitTapeLetters(void)
{
   int i;
   int size = getnFontSize();
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

   AG_PushTextState();
   AG_TextFont(pStatusFont);
   AG_TextFontPts(size);
   rec.x = 0;
   rec.y = 0;
   rec.h = nCMTHeight / 6;
   rec.w = nCMTWidth / 6;
   for(i = 0; i < 10; i++){
        string[0] = '0' + i;
        string[1] = '\0';
      
        pCMTReadLetter[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
        AG_FillRect(pCMTReadLetter[i], NULL, r);
	AG_TextColor(black);
	AG_TextBGColor(r);
	tmp = AG_TextRender(string);
	AG_SurfaceBlit(tmp, &rec, pCMTReadLetter[i], 0, 0);
	AG_SurfaceFree(tmp);

        pCMTWriteLetter[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
        AG_FillRect(pCMTWriteLetter[i], NULL, b);
	AG_TextColor(black);
	AG_TextBGColor(b);
	tmp = AG_TextRender(string);
	AG_SurfaceBlit(tmp, &rec, pCMTWriteLetter[i], 0, 0);
	AG_SurfaceFree(tmp);

         pCMTNormLetter[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	AG_FillRect(pCMTNormLetter[i], NULL, black);
	AG_TextColor(n);
	AG_TextBGColor(black);
	tmp = AG_TextRender(string);
	AG_SurfaceBlit(tmp, &rec, pCMTNormLetter[i], 0, 0);
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
   
   
   if(pCMTRead == NULL) return;
   if(pCMTWrite == NULL) return;
   if(pCMTNorm == NULL) return;
   x = 4;
   AG_SurfaceBlit(UpdateCMTSub(count / 1000, ID_READ), NULL, pCMTRead, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count / 1000, ID_WRITE), NULL, pCMTWrite, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count / 1000, ID_IN), NULL, pCMTNorm, x, 0);
   x = x + pCMTNormLetter[0]->w + 2;

   AG_SurfaceBlit(UpdateCMTSub(count / 100, ID_READ), NULL, pCMTRead, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count / 100, ID_WRITE), NULL, pCMTWrite, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count / 100, ID_IN), NULL, pCMTNorm, x, 0);
   x = x + pCMTNormLetter[0]->w + 2;

   AG_SurfaceBlit(UpdateCMTSub(count / 10, ID_READ), NULL, pCMTRead, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count / 10, ID_WRITE), NULL, pCMTWrite, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count / 10, ID_IN), NULL, pCMTNorm, x, 0);
   x = x + pCMTNormLetter[0]->w + 2;

   AG_SurfaceBlit(UpdateCMTSub(count % 10, ID_READ), NULL, pCMTRead, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count % 10, ID_WRITE), NULL, pCMTWrite, x, 0);
   AG_SurfaceBlit(UpdateCMTSub(count % 10, ID_IN), NULL, pCMTNorm, x, 0);
   x = x + pCMTNormLetter[0]->w + 2;

   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
   AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);

}


void CreateCMT(AG_Widget *parent, bool initflag)
{
    AG_Rect rec;
    AG_PixelFormat fmt;
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

   SetPixelFormat(&fmt);


   if(pStatusFont == NULL) return;
    pCMTRead = pCMTWrite = pCMTNorm = NULL;
    pwCMT = NULL;
    nCMTHeight = CMT_HEIGHT * 2;
    nCMTWidth = CMT_WIDTH * 2;

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
   if(pwCMT == NULL) return;
   nwCMT[ID_IN] = AG_PixmapAddSurface(pwCMT, pCMTNorm);
   nwCMT[ID_READ] = AG_PixmapAddSurface(pwCMT, pCMTRead);
   nwCMT[ID_WRITE] = AG_PixmapAddSurface(pwCMT, pCMTWrite);
}


void InitTapeOSD(AG_Widget *parent)
{
    if(parent == NULL) return;
    pwCMT = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nCMTWidth, nCMTHeight);
    AG_WidgetSetSize(pwCMT, nCMTWidth, nCMTHeight);

    nwCMT[ID_EMPTY] = 0;
    nwCMT[ID_IN] = AG_PixmapAddSurface(pwCMT, pCMTNorm);
    nwCMT[ID_READ] = AG_PixmapAddSurface(pwCMT, pCMTRead);
    nwCMT[ID_WRITE] = AG_PixmapAddSurface(pwCMT, pCMTWrite);
    AG_WidgetShow(pwCMT);

}

void DestroyTapeOSD(void)
{
   if(pwCMT == NULL) return;
   AG_ObjectDetach(AGOBJECT(pwCMT));

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
    AG_WidgetShow(pwCMT);
}

void ResizeTapeOSD(AG_Widget *parent, int w, int h)
{
       int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3 + 50;
       float ww = (float)w;
       float wCMT = (float)CMT_WIDTH / (float)total;

       AG_WidgetHide(pwCMT);
       AG_ObjectDetach(AGOBJECT(pwCMT));
//       UpdateCMTMessages("       ");
       nCMTWidth = (int)(ww * wCMT);
       nCMTHeight = h;
       pwCMT = AG_PixmapNew(parent, AG_PIXMAP_RESCALE , nCMTWidth, nCMTHeight);
   
       CreateCMT(parent, FALSE);
   
//       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
//       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
//       AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);

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
	char            string[128];
	char     protect[16];
	AG_Rect rect;
	AG_Surface *tmp;

        if(pwCMT == NULL) return;
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
	        if(pwCMT == NULL) return;
	        SetCMTBlank();
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
		      if(nTape < 30000) UpdateCMTCount(nTape % 10000);
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
//	   AG_Redraw(pwCMT);
}

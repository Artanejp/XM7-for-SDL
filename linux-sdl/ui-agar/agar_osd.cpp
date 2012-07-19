/*
 * agar_osd.cpp
 *
 *  Created on: 2010/11/26
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
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
/*  リサイズ */
static int nStatFontSize;
static int nFontSize;

AG_Font         *pStatusFont;
static AG_Color			r;
static AG_Color 		b;
static AG_Color			n;
static AG_Color 		black;
static AG_Color			alpha;


static int nwFD[2][4]; // R/W/Noaccess/Empty
extern "C" {
AG_HBox *pStatusBar;
}

extern void DrawMainCaption(BOOL redraw);
extern void ClearStatOSD(void);
extern void ResizeStatOSD(AG_Widget *parent, int w, int h);
extern void InitStatOSD(AG_Widget *parent);
extern void DestroyStatOSD(void);
extern void LinkSurfaceSTAT(void);

extern void InitLeds(AG_Widget *parent);
extern void DestroyLeds(void);
extern void DrawCAP(void);
extern void DrawKANA(void);
extern void DrawINS(void);
extern void ResizeLeds(AG_Widget *parent, int w, int h);
extern void LinkSurfaceLeds(void);
extern void ClearLeds(void);

extern void InitTapeOSD(AG_Widget *parent);
extern void DestroyTapeOSD(void);
extern void LinkSurfaceCMT(void);
extern void ResizeTapeOSD(AG_Widget *parent, int w, int h);
extern void ClearTapeOSD(void);
extern void DrawTape(int override);
extern void CreateCMT(AG_Widget *parent, bool initflag);

extern void InitVFD(AG_Widget *parent);
extern void DestroyVFD(void);
extern void LinkSurfaceVFD(void);
extern void ResizeVFD(AG_Widget *parent, int w, int h);
extern void ClearVFD(void);
extern void DrawDrive(int drive, BOOL override);

/*
 * Set Pixelformat of surfaces
 */
void SetPixelFormat(AG_PixelFormat *fmt)
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
    pStatusFont =  AG_FetchFont (STAT_FONT,STAT_PT, 0);
	// Init Var

   nFontSize = STAT_PT;
    if(parent) {
       InitStatOSD(parent);
       InitVFD(parent);
       InitTapeOSD(parent);
       InitLeds(parent);
    }
}

void DestroyStatus(void)
{
	int i, j;

     DestroyStatOSD();
     DestroyLeds();
     DestroyVFD();
     DestroyTapeOSD();
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
    float ww = (float)w;
    float wFD = (float)VFD_WIDTH / (float)total;
    float wCaption = (float)STAT_WIDTH / (float)total;
    AG_Box *pad;

    if(parent == NULL) return;


   nFontSize = (int)(STAT_PT * ww) / ((float)total * 2.0f);

   AG_WidgetSetSize(parent, w, (w * h) / total);
   ResizeStatOSD(parent, w, h);
   ResizeVFD(parent, w, h);
   ResizeTapeOSD(parent, w, h);
   ResizeLeds(parent, w, h);
   DrawStatusForce();
   AG_WidgetUpdate(parent);
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
        ClearStatOSD();
        ClearLeds();
        ClearVFD();
        ClearTapeOSD();
	/*
	 * 描画
	 */
     DrawStatusForce();
}

int getnFontSize(void)
{
   return nFontSize;
}

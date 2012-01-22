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
#include "sdl.h"
#endif



#include "sdl_bar.h"
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
static int     nInitialDrawFlag; /* 強制再表示フラグ */
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

/* 表示Surface */
static AG_Surface      *pOSDIns;
static AG_Surface      *pOSDKana;
static AG_Surface      *pOSDCaps;
static AG_Surface      *pOSDFD;
static AG_Surface      *pOSDCMT;
static AG_Surface      *pOSDCaption;


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


static void InitVFDMessages(AG_Widget *parent)
{
   AG_PixelFormat fmt;
   AG_Color col;
   AG_Rect rect;
   AG_Surface *p;
   AG_Box *dummy;
   int i;
   int height = 20;
	// Surfaceつくる
	fmt.BitsPerPixel = 32;
	fmt.BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
	fmt.Rmask = 0x000000ff; // R
	fmt.Gmask = 0x0000ff00; // G
	fmt.Bmask = 0x00ff0000; // B
	fmt.Amask = 0xff000000; // A
#else
	fmt.Rmask = 0x00ff0000; // R
	fmt.Gmask = 0x0000ff00; // G
	fmt.Bmask = 0xff000000; // B
	fmt.Amask = 0x000000ff; // A
#endif
	fmt.Rshift = 0;
	fmt.Gshift = 8;
	fmt.Bshift = 16;
	fmt.Ashift = 24;
	fmt.Rloss = 0;
	fmt.Gloss = 0;
	fmt.Bloss = 0;
	fmt.Aloss = 0;
	fmt.palette = NULL;

    rect.x = 0;
    rect.y = 0;
    rect.w = VFD_WIDTH * 2;
    rect.h = height * 2;

    col.a = 255;
    col.b = 0;
    col.r = 0;
    col.g = 0;
    p = AG_SurfaceNew(AG_SURFACE_PACKED, rect.w, rect.h, &fmt, AG_SRCALPHA);
    AG_FillRect(p, NULL, col);
    pwCaption = AG_PixmapFromSurfaceCopy(parent, AG_PIXMAP_RESCALE, p);
    AG_WidgetSetSize(pwCaption, 200, height);
    //dummy = AG_BoxNewHoriz(parent, AG_BOX_HFILL);
    for(i = 1; i >= 0 ; i--) {
        pwFD[i] = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE, pFDNorm[i]);
    }
    pwCMT = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE, pCMTNorm);
   AG_SurfaceFree(p);
}


static void InitBox(AG_Widget *parent)
{
    int i;

    int height;
   
    height = 20;

    if(parent == NULL) return;
    InitVFDMessages(parent);

    pwCAPS = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE, pCapsOff);
    pwINS = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE, pInsOff);
    pwKana = AG_PixmapFromSurface(parent, AG_PIXMAP_RESCALE, pKanaOff);
}

static int LinkSurface(void)
{
    int i;
    int j;
    AG_Surface *p;
    AG_PixelFormat fmt;
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
* OpenGL Textures
*/
GLuint tid_ins_on;
GLuint tid_kana_on;
GLuint tid_caps_on;
GLuint tid_ins_off;
GLuint tid_kana_off;
GLuint tid_caps_off;

GLuint tid_fd[4][3];
GLuint tid_cmt[3];
GLuint tid_caption;
GLfloat LedAlpha;
GLfloat FDDAlpha;



/*
* OSDを表示する実体
* (1280x880) 空間で、(x, 878) - (x, 878 -h)に表示される
*/

static void DrawCAP(void);
static void DrawINS(void);
static void DrawKANA(void);
static void DrawMainCaption(void);
static void DrawDrive(AG_Widget *w, int drive);
static void DrawTape(void);

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
	fmt.BitsPerPixel = 32;
	fmt.BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
	fmt.Rmask = 0x000000ff; // R
	fmt.Gmask = 0x0000ff00; // G
	fmt.Bmask = 0x00ff0000; // B
	fmt.Amask = 0xff000000; // A
#else
	fmt.Rmask = 0x00ff0000; // R
	fmt.Gmask = 0x0000ff00; // G
	fmt.Bmask = 0xff000000; // B
	fmt.Amask = 0x000000ff; // A
#endif
	fmt.Rshift = 0;
	fmt.Gshift = 8;
	fmt.Bshift = 16;
	fmt.Ashift = 24;
	fmt.Rloss = 0;
	fmt.Gloss = 0;
	fmt.Bloss = 0;
	fmt.Aloss = 0;
	fmt.palette = NULL;

	pStatusFont =  AG_FetchFont (STAT_FONT,STAT_PT, 0);
	AG_PushTextState();
	AG_TextFont(pStatusFont);
	AG_TextColor(black);
	AG_TextBGColor(r);
	pInsOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
	AG_FillRect(pInsOn, NULL, r);
	tmps = AG_TextRender("Ins");
	AG_SurfaceBlit(tmps, NULL, pInsOn, 4, 0);
	AG_SurfaceFree(tmps);
//	tid_ins_on = CreateTexture(pInsOn);
	//    	AG_SurfaceFree(pInsOn);
	//    	pInsOn = NULL;

	pCapsOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
	AG_FillRect(pCapsOn, NULL, r);
	tmps = AG_TextRender("CAP");
	AG_SurfaceBlit(tmps, NULL, pCapsOn, 1, 0);
	AG_SurfaceFree(tmps);
//	tid_caps_on = CreateTexture(pCapsOn);
	//        AG_SurfaceFree(pCapsOn);
	//        pCapsOn = NULL;

	pKanaOn = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
	AG_FillRect(pKanaOn, NULL, r);
	tmps = AG_TextRender("カナ");
	AG_SurfaceBlit(tmps, NULL, pKanaOn, 8, 0);
	AG_SurfaceFree(tmps);
//	tid_kana_on = CreateTexture(pKanaOn);
	//    	AG_SurfaceFree(pKanaOn);
	//    	pKanaOn = NULL;

	AG_TextColor(n);
	AG_TextBGColor(alpha);

	pInsOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
	AG_FillRect(pInsOff, NULL, alpha);
	tmps = AG_TextRender("Ins");
	AG_SurfaceBlit(tmps, NULL, pInsOff, 4, 0);
	AG_SurfaceFree(tmps);
//	tid_ins_off = CreateTexture(pInsOff);
	//    	AG_SurfaceFree(pInsOff);
	//    	pInsOff = NULL;

	pCapsOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH * 2, LED_HEIGHT * 2,  &fmt, AG_SRCALPHA);
	AG_FillRect(pCapsOff, NULL, alpha);
	tmps = AG_TextRender("CAP");
	AG_SurfaceBlit(tmps, NULL, pCapsOff, 1, 0);
	AG_SurfaceFree(tmps);
//	tid_caps_off = CreateTexture(pCapsOff);
	//        AG_SurfaceFree(pCapsOff);
	//        pCapsOff = NULL;


	pKanaOff = AG_SurfaceNew(AG_SURFACE_PACKED , LED_WIDTH *2, LED_HEIGHT * 2, &fmt, AG_SRCALPHA);
	AG_FillRect(pKanaOff, NULL, alpha);
	tmps = AG_TextRender("カナ");
	AG_SurfaceBlit(tmps, NULL, pKanaOff, 8, 0);
	AG_SurfaceFree(tmps);
//	tid_kana_off = CreateTexture(pKanaOff);
	//    	AG_SurfaceFree(pKanaOff);
	//    	pKanaOff = NULL;
	AG_PopTextState();

	AG_PushTextState();
	AG_TextFont(pStatusFont);

	/*
	 * RECT Drive1
	 */
	rec.x = 0;
	rec.y = 0;
	rec.w = VFD_WIDTH * 2;
	rec.h = VFD_HEIGHT * 2;
	for(i = 0 ; i < 2 ; i++) {
		pFDRead[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
		pFDWrite[i] =AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
		pFDNorm[i] = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
		AG_FillRect(pFDRead[i], &rec, r);
		AG_FillRect(pFDWrite[i], &rec, b);
		AG_FillRect(pFDNorm[i], &rec, alpha);
	}

	/*
	 * RECT Tape
	 */
	 rec.x = 0;
	rec.y = 0;
	rec.w = CMT_WIDTH * 2;
	rec.h = CMT_HEIGHT * 2;
	pCMTRead = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	pCMTWrite = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	pCMTNorm = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);

	AG_FillRect(pCMTRead, &rec, r);
	AG_FillRect(pCMTWrite, &rec, b);
	AG_FillRect(pCMTNorm, &rec, black);

	rec.x = 0;
	rec.y = 0;
	rec.w = STAT_WIDTH * 2;
	rec.h = STAT_HEIGHT * 2;

	pCaption = AG_SurfaceNew(AG_SURFACE_PACKED , rec.w, rec.h, &fmt, AG_SRCALPHA);
	AG_FillRect(pCaption, &rec,  alpha);

	AG_PopTextState();

	/*
	 * 表示サーフェス(非GL)
	 */
	pOSDIns = NULL;
	pOSDCaps = NULL;
	pOSDKana = NULL;
	pOSDFD = NULL;
	pOSDCMT = NULL;
	pOSDCaption = NULL;

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
#if 0
	DiscardTexture(tid_ins_on);
	DiscardTexture(tid_caps_on);
	DiscardTexture(tid_kana_on);
	DiscardTexture(tid_ins_off);
	DiscardTexture(tid_caps_off);
	DiscardTexture(tid_kana_off);
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 3; j++){
			DiscardTexture(tid_fd[i][j]);
		}
	}
	for(i = 0; i < 3 ; i++) {
		DiscardTexture(tid_cmt[i]);
	}
	DiscardTexture(tid_caption);
#endif
	/*
	 * 表示サーフェス(非GL)
	 */
	pOSDIns = NULL;
	pOSDCaps = NULL;
	pOSDKana = NULL;
	pOSDFD = NULL;
	pOSDCMT = NULL;
	pOSDCaption = NULL;
}
/*
 *  キャプション描画
 */
static void DrawMainCaption(void)
{
	char           string[1024];
	char           tmp[128];
	char          *p;
	AG_Surface   *tmps;
	AG_Rect rect;
	AG_Font *fp;

	if(pCaption == NULL) return;
	/*
	 * 動作状況に応じて、コピー
	 */
	if (run_flag) {
		strcpy(string, "XM7[実行]");
	}
	else {
		strcpy(string, "XM7[停止]");
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
	if((strncmp(szOldCaption, szCaption, 128) != 0) || (nInitialDrawFlag == TRUE)) {
		AG_PushTextState();
		AG_TextFont(pStatusFont);
		AG_TextColor(n);
		AG_TextBGColor(alpha);
		AG_FillRect(pCaption, NULL, alpha);
		tmps = AG_TextRender(szCaption);
		rect.x = 0;
		rect.y = 0;
		rect.h = STAT_HEIGHT * 2;
		rect.w = STAT_WIDTH * 2;
		AG_SurfaceBlit(tmps, &rect, pCaption, 2, 0);
		AG_SurfaceFree(tmps);
		AG_PopTextState();
		strcpy(szOldCaption, szCaption);
	   if(pwCaption != NULL) {
	      AG_PixmapSetSurface(pwCaption, nwCaption);
	      AG_PixmapUpdateSurface(pwCaption, nwCaption);
	      AG_Redraw(pwCaption);
	   }
	   
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
	if (nCAP) {
	   AG_PixmapSetSurface(pwCAPS, nwCaps[ID_ON]);
    } else {
	   AG_PixmapSetSurface(pwCAPS, nwCaps[ID_OFF]);
	}
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
	if (nKANA) {
	   AG_PixmapSetSurface(pwKana, nwKana[ID_ON]);
	} else {
	   AG_PixmapSetSurface(pwKana, nwKana[ID_OFF]);
	}
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
	if (nINS) {
	   AG_PixmapSetSurface(pwINS, nwIns[ID_ON]);
	} else {
	   AG_PixmapSetSurface(pwINS, nwIns[ID_OFF]);
	}
     AG_Redraw(pwINS);
}

/*
 *  ドライブ描画
 */
static void DrawDrive(int drive)
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
			 //            	for(i = 0; i < 3 ; i++) {
			 //            		DiscardTexture(tid_fd[drive][i]);
			 //            	}
			 //            	return;
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


	 if((strcmp(szDrive[drive], szOldDrive[drive]) != 0) || (old_writep[drive] != fdc_writep[drive])) {
		 /*
		  * 過去のファイルネームと違うのでフォントレンダリングする
		  */
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
		 /*
		  * 旧いテクスチャを破棄
		  */

//		 for(i = 0; i < 3 ; i++) {
//			 DiscardTexture(tid_fd[drive][i]);
//		 }
	        AG_ObjectLock(pwFD[drive]);
		AG_PushTextState();
		 AG_TextFont(pStatusFont);
		 AG_TextColor(alpha);
		 AG_TextBGColor(r);
		 rect.x = 0;
		 rect.y = 0;
		 rect.h = VFD_HEIGHT * 2;
		 rect.w = VFD_WIDTH * 2;
		 AG_FillRect(pFDRead[drive], &rect, r);
		 tmp = AG_TextRender(outstr);
		 AG_SurfaceBlit(tmp, &rect, pFDRead[drive], 0, 0);
		 AG_SurfaceFree(tmp);

		 AG_TextBGColor(b);
		 AG_FillRect(pFDWrite[drive], &rect, b);
		 tmp = AG_TextRender(outstr);
		 AG_SurfaceBlit(tmp, &rect, pFDWrite[drive], 0, 0);
		 AG_SurfaceFree(tmp);

		 AG_TextColor(n);
		 AG_TextBGColor(black);
		 AG_FillRect(pFDNorm[drive], &rect, black);
		 tmp = AG_TextRender(outstr);
		 AG_SurfaceBlit(tmp, &rect, pFDNorm[drive], 0, 0);
		 AG_SurfaceFree(tmp);

	    if(pwFD[drive] != NULL) {
	       AG_PixmapUpdateSurface(pwFD[drive], nwFD[drive][ID_IN]);
	       AG_PixmapUpdateSurface(pwFD[drive], nwFD[drive][ID_READ]);
	       AG_PixmapUpdateSurface(pwFD[drive], nwFD[drive][ID_WRITE]);
	    }
        AG_ObjectUnlock(pwFD[drive]);
	    

		 AG_PopTextState();
		 memset(szOldDrive[drive], 0, 16);
		 strncpy(szOldDrive[drive], szDrive[drive], 16);
	 }
	 if (nDrive[drive] == FDC_ACCESS_READ) {
	    AG_PixmapSetSurface(pwFD[drive], nwFD[drive][ID_READ]);
	 } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
	    AG_PixmapSetSurface(pwFD[drive], nwFD[drive][ID_WRITE]);
	 } else {
	    AG_PixmapSetSurface(pwFD[drive], nwFD[drive][ID_IN]);
	 }
	   AG_Redraw(pwFD[drive]);
}


/*
 *  テープ描画
 */
static void DrawTape(void)
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
	 if ((nTape == num) && (nInitialDrawFlag != TRUE)){
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
       if(pwCMT == NULL) return;
	if((nOldTape != nTape)  || (nTape == 0)){
		/*
		 * カウンタ番号レンダリング(仮)
		 */
//		for(i = 0; i < 3 ; i++) {
//			DiscardTexture(tid_cmt[i]);
//		}
		if(pStatusFont != NULL) {
			AG_PushTextState();
		   AG_ObjectLock(pwCMT);

		   AG_FillRect(pCMTRead, &rect, r);
			rect.x = 0;
			rect.y = 0;
			rect.h = CMT_HEIGHT * 2;
			rect.w = CMT_WIDTH * 2;
			AG_TextFont(pStatusFont);
			AG_TextColor(black);
			AG_TextBGColor(r);
			tmp = AG_TextRender(string);
			AG_SurfaceBlit(tmp, &rect, pCMTRead, 0, 0);
			AG_SurfaceFree(tmp);



			AG_FillRect(pCMTWrite, &rect, b);
			AG_TextFont(pStatusFont);
			AG_TextColor(black);
			AG_TextBGColor(b);
			tmp = AG_TextRender(string);
			AG_SurfaceBlit(tmp, &rect, pCMTWrite, 0, 0);
			AG_SurfaceFree(tmp);



			AG_FillRect(pCMTNorm, &rect, n);
			AG_TextFont(pStatusFont);
			AG_TextColor(black);
			AG_TextBGColor(n);
			tmp = AG_TextRender(string);
			AG_SurfaceBlit(tmp, &rect, pCMTNorm, 0, 0);
			AG_SurfaceFree(tmp);
		   if(pwCMT != NULL) {
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_IN]);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_READ]);
		      AG_PixmapUpdateSurface(pwCMT, nwCMT[ID_WRITE]);
		   }
		   
		      AG_ObjectUnlock(pwCMT);

			AG_PopTextState();
		}
	}
		if ((nTape >= 10000) && (nTape < 30000)) {
			if (nTape >= 20000) {
			   AG_PixmapSetSurface(pwCMT, nwCMT[ID_WRITE]);
				//DrawTexture(pCMTWrite, pOSDCMT, tid_cmt[2], 1280 - LED_WIDTH * 6 - CMT_WIDTH * 2 - 8 , 800 - CMT_HEIGHT*2 - 4,  CMT_WIDTH * 2, CMT_HEIGHT * 2);
			}   else {
			   AG_PixmapSetSurface(pwCMT, nwCMT[ID_READ]);
				//DrawTexture(pCMTRead, pOSDCMT, tid_cmt[1], 1280 - LED_WIDTH * 6 - CMT_WIDTH * 2 - 8 , 800 - CMT_HEIGHT*2 - 4,  CMT_WIDTH * 2, CMT_HEIGHT * 2);
			}
		} else {
			//DrawTexture(pCMTNorm, pOSDCMT, tid_cmt[0], 1280 - LED_WIDTH * 6 - CMT_WIDTH * 2 - 8 , 800 - CMT_HEIGHT*2 - 4,  CMT_WIDTH * 2, CMT_HEIGHT * 2);
			   AG_PixmapSetSurface(pwCMT, nwCMT[ID_IN]);
		}
	   AG_Redraw(pwCMT);
		nOldTape = nTape;
}

/*
 *  描画
 */

void DrawStatus(void)
{
//#ifndef USE_OPENGL
	DrawMainCaption();
	DrawCAP();
	DrawKANA();
	DrawINS();
	DrawDrive(0);
	DrawDrive(1);
	DrawTape();
	nInitialDrawFlag = FALSE;
//#else
//	nInitialDrawFlag = FALSE;
//#endif
}


/*
 *  強制描画
 */
void DrawStatusForce(void)
{
	nInitialDrawFlag = TRUE;
	DrawStatus();
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
	nInitialDrawFlag = TRUE;
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
	      DrawStatus();
}

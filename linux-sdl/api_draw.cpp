/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2003 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 *  [SDL 表示 ]
 *  2010.10.28 sdl_draw.c から移動
 */


#ifdef USE_GTK
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#endif

#include <SDL.h>
#ifdef USE_AGAR
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#else
#include <SDL_syswm.h>
#ifdef USE_OPENGL
#include <SDL_opengl.h>
#endif
#endif

#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif

#include "api_draw.h"
#include "api_scaler.h"

#ifdef USE_AGAR
extern AG_Window *MainWindow;
extern AG_GLView *DrawArea;
#endif
/*
 *  グローバル ワーク
 */
DWORD   rgbTTLGDI[16];	/* デジタルパレット */
DWORD   rgbAnalogGDI[4096];	/* アナログパレット */

// guchar pBitsGDI[400*640*3]; /* ビットデータ */
BYTE            GDIDrawFlag[80 * 50];	/* 8x8ドットのメッシュを作る *//* 8x8 再描画領域フラグ */
BOOL            bFullScan;		/* フルスキャン(Window) */
BOOL            bDirectDraw;		/* 直接書き込みフラグ */
SDL_Surface     *realDrawArea;	/* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する) */
WORD            nDrawTop;			/* 描画範囲上 */
WORD            nDrawBottom;		/* 描画範囲下 */
WORD            nDrawLeft;		/* 描画範囲左 */
WORD            nDrawRight;		/* 描画範囲右 */
WORD            nDrawWidth;
WORD            nDrawHeight;
BOOL            bPaletFlag;		/* パレット変更フラグ */
BOOL            bClearFlag;
int             nOldVideoMode;
WORD			nDrawFPS;   /* FPS値 20100913 */
BOOL  bUseOpenGL; /* OPENGLを描画に使う */
SDL_semaphore *DrawInitSem;

/*
 *  スタティック ワーク
 */
#if XM7_VER >= 3
BYTE    bMode;		/* 画面モード */

#else				/*  */
static BOOL     bAnalog;	/* アナログモードフラグ */

#endif				/*  */
static BYTE     bNowBPP;	/* 現在のビット深度 */
static BOOL   bOldFullScan;	/* クリアフラグ */
static WORD    nOldDrawWidth;
static WORD    nOldDrawHeight;


#if XM7_VER >= 3
static BOOL     bWindowOpen;	/* ハードウェアウィンドウ状態
 */
static WORD    nWindowDx1;	/* ウィンドウ左上X座標 */
static WORD    nWindowDy1;	/* ウィンドウ左上Y座標 */
static WORD    nWindowDx2;	/* ウィンドウ右下X座標 */
static WORD    nWindowDy2;	/* ウィンドウ右下Y座標 */

#endif				/*  */

/*
 * マルチスレッド向け定義
 */
static BOOL DrawINGFlag;
static BOOL DrawSHUTDOWN;
static BOOL DrawWaitFlag;
static SDL_Thread *DrawThread;
static WORD nDrawCount;
SDL_cond *DrawCond;
SDL_mutex *DrawMutex;
int newDrawWidth;
int newDrawHeight;
BOOL newResize;
Uint32 nDrawTick1;



/*
 *  プロトタイプ宣言
 */
#ifdef __cplusplus
extern "C"
{
#endif
static  BOOL Select640(void);
static  BOOL Select400l(void);
static  BOOL Select320(void);
static  BOOL Select256k(void);
static void ChangeResolution(void) ;
#ifdef __cplusplus
}
#endif


/*
 * ビデオドライバ関連
 */
EmuGrphLib *vramhdr;
EmuGrph400l *vramhdr_400l;
EmuGrph4096c *vramhdr_4096;
EmuGrph256kc *vramhdr_256k;




void VramReader(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
		if(vramhdr != NULL) {
			vramhdr->GetVram(addr, cbuf, mpage);
		}
}

void VramReader_400l(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
		if(vramhdr_400l != NULL) {
			vramhdr_400l->GetVram(addr, cbuf, mpage);
		}
}

void VramReader_4096(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
		if(vramhdr_4096 != NULL) {
			vramhdr_4096->GetVram(addr, cbuf, mpage);
		}
}

void VramReader_256k(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
		if(vramhdr_256k != NULL) {
			vramhdr_256k->GetVram(addr, cbuf, mpage);

		}
}


/*
 *  セレクトチェック
 */
static BOOL SelectCheck(void)
{

#if XM7_VER >= 3
	/*
	 * 限りない手抜き(ォ
	 */
	if (bMode == screen_mode) {
		return TRUE;
	} else {
		return FALSE;
	}
#else				/*  */
/*
 * 320x200
 */
	if (mode320) {
		if (bAnalog) {
			return TRUE;
		} else {
			return FALSE;
		}
	}

	/*
	 * 640x200
	 */
	if (!bAnalog) {
		return TRUE;
	} else {
		return FALSE;
	}
#endif /*  */
}

/*
 * 描画は別スレッドで行う
 */
static WORD DrawCountSet(WORD fps)
{
	DWORD intr;
	DWORD wait;

#if XM7_VER >= 3
	if (screen_mode == SCR_400LINE) {
#else
		if (enable_400line && enable_400linecard) {
#endif
			/*
			 * 400ライン(24kHzモード) 0.98ms + 16.4ms
			 */
			intr = 980 + 16400;
		} else {
			/*
			 * 200ライン(15kHzモード) 1.91ms + 12.7ms
			 */
			intr = 1910 + 12700;
		}
		if(fps<= 0) fps=1;
		wait = 1000000 / (DWORD)fps ;
		wait = wait / intr + 1; /* 整数化 */
		return (WORD) wait;
}

	/*
	 *  セレクト(内部向き)
	 */
BOOL SelectDraw2(void)
{
#ifdef USE_AGAR
		AG_Color nullcolor;
		AG_Driver *drv;
		AG_Rect rect;
#else // SDL
		Uint32 nullcolor;
		SDL_Surface *p;
		SDL_Rect rect;
#endif



		/*
		 * 一致しているかチェック
		 */
		if (SelectCheck()) {
			return TRUE;
		}
#ifdef USE_AGAR
		if(agDriverOps == NULL) return FALSE;
		if(DrawArea == NULL) return FALSE;
		drv = AGWIDGET(DrawArea)->drv;
		if(drv == NULL) return FALSE;
#else
		p = SDL_GetVideoSurface();
		if(p == NULL) return FALSE;
#endif
		rect.h = nDrawWidth;
		rect.w = nDrawHeight;
		rect.x = 0;
		rect.y = 0;
		if(!bUseOpenGL) {
			/*
			 * すべてクリア
			 */
#ifdef USE_AGAR
			AG_ObjectLock(DrawArea);
			nullcolor.r = 0;
			nullcolor.g = 0;
			nullcolor.b = 0;
			nullcolor.a = 255;

			AG_FillRect(drv->sRef, &rect, nullcolor);
			AG_ObjectUnlock(DrawArea);
#else
			SDL_LockSurface(p);
			nullcolor = SDL_MapRGBA(p->format, 0, 0, 0, 255);
			SDL_FillRect(p, &rect, nullcolor);
			SDL_UnlockSurface(p);
#endif
			/*
			 * すべてクリア
			 */
#ifdef USE_AGAR
			//AG_DriverClose(drv);
#else
			if(realDrawArea != p) {
				SDL_LockSurface(realDrawArea);
				rect.h = realDrawArea->h;
				rect.w = realDrawArea->w;
				rect.x = 0;
				rect.y = 0;
				nullcolor = SDL_MapRGBA(realDrawArea->format, 0, 0, 0, 255);
				SDL_FillRect(realDrawArea, &rect, nullcolor);
				SDL_UnlockSurface(realDrawArea);
			}
#endif
		}
		bOldFullScan = bFullScan;
		/*
		 * セレクト
		 */
#if XM7_VER >= 3
		switch (screen_mode) {
		case SCR_400LINE:
			return Select400l();
		case SCR_262144:
			return Select256k();
		case SCR_4096:
			return Select320();
		default:
			return Select640();
		}

#else				/*  */
		if (mode320) {
			return Select320();
		}
		return Select640();

#endif				/*  */
		return TRUE;
}
#ifdef USE_GTK
extern GtkWidget       *gtkDrawArea;
#endif

static void ResizeWindow(int w, int h)
{
    char          EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
    SDL_SysWMinfo sdlinfo;
#ifdef USE_AGAR
            ResizeWindow_Agar(w, h);
#else
#if 0
#ifdef USE_GTK
            sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
                    gdk_x11_drawable_get_xid(gtkDrawArea->window));
            SDL_putenv(EnvMainWindow);
//            SDL_InitSubSystem(SDL_INIT_VIDEO);
#endif
            InitGL(w, h);
#ifdef USE_GTK
            gtk_widget_set_size_request(gtkDrawArea, w, h);
            SDL_GetWMInfo(&sdlinfo);
            gtk_socket_add_id(GTK_SOCKET(gtkDrawArea), sdlinfo.info.x11.window);
#endif
#else
            InitGL(w, h);
#endif
#endif
}

static int DrawTaskMain(void *arg)
{
		SDL_Surface *p;


		if(newResize) {
			nDrawWidth = newDrawWidth;
			nDrawHeight = newDrawHeight;
			ResizeWindow(nDrawWidth, nDrawHeight);
//			SetupGL(nDrawWidth, nDrawHeight);
			newResize = FALSE;
		}
		ChangeResolution();
		SelectDraw2();
#if XM7_VER >= 3
		/*
		 *    いずれかを使って描画
		 */
		SDL_SemWait(DrawInitSem);
		switch (bMode) {
		case SCR_400LINE:
			Draw400l();
			break;
		case SCR_262144:
			Draw256k();
			break;
		case SCR_4096:
			Draw320();
			break;
		case SCR_200LINE:
			Draw640All();
			break;
		}
		SDL_SemPost(DrawInitSem);
#else				/*  */
		/*
		 * どちらかを使って描画
		 */
		if (bAnalog) {
			Draw320All();
		}
		else {
			Draw640All();
		}
#endif				/*  */
		//        SDL_UnlockSurface(p);
		Flip();
		return 0;
}

static void initsub(void);
static void detachsub(void);

int DrawThreadMain(void *p)
{
#ifdef USE_AGAR
		Uint32 nDrawTick2;

		nDrawTick1 = AG_GetTicks();
#endif
		initsub();
		ResizeWindow(640,400);
		InitGL(640,480);
		nDrawCount = DrawCountSet(nDrawFPS);
		newResize = FALSE;
		while(1) {
			if(DrawMutex == NULL) {
				SDL_Delay(1);
				continue;
			}
			if(DrawCond == NULL) {
				SDL_Delay(1);
				continue;
			}
#if 1
			SDL_mutexP(DrawMutex);
			SDL_CondWait(DrawCond, DrawMutex);
#else
			SDL_Delay(33 - 1);
#endif
			if(DrawSHUTDOWN) {
				detachsub();
				return 0; /* シャットダウン期間 */
			}
#ifndef USE_OPENGL
			DrawStatus();
#endif

#ifdef USE_AGAR
			if(DrawArea == NULL) continue;
#endif

			if(nDrawCount > 0) {
				nDrawCount --;
#ifdef USE_AGAR
				AGDrawTaskEvent(FALSE);
#endif
				continue;
			} else {
				nDrawCount = DrawCountSet(nDrawFPS);
			}
			DrawWaitFlag = TRUE;
			DrawINGFlag = TRUE;
#ifdef USE_AGAR
			AGDrawTaskMain();
			AGDrawTaskEvent(TRUE);
#else
			DrawTaskMain(NULL);
#endif
			DrawINGFlag = FALSE;
			DrawWaitFlag = FALSE;
			//while(DrawWaitFlag) SDL_Delay(1); /* 非表示期間 */
		}
}



/*
 *  BITBLT
 */
BOOL BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight,
			int nSrcLeft, int nSrcTop)
{

#ifdef USE_AGAR
	AG_Rect srcrect, dstrect;
#else
	SDL_Rect srcrect, dstrect;
#endif

	if(bUseOpenGL) {
		Flip();
		return TRUE;
	}
	srcrect.x = nSrcLeft;
	srcrect.y = nSrcTop;
	srcrect.w = (Uint16) nWidth;
	srcrect.h = (Uint16) nHeight;

	dstrect.x = nDestLeft;
	dstrect.y = nDestTop;
	dstrect.w = (Uint16) nWidth;
	dstrect.h = (Uint16) nHeight;
		/*
		 * データ転送
		 */
#ifndef USE_AGAR
		displayArea = SDL_GetVideoSurface();
#endif
		/*
		 * 擬似インタレース設定をここでやる
		 */
		if (bOldFullScan != bFullScan) {
			if (!bFullScan) {
				RenderSetOddLine();
			} else {
				RenderFullScan();
			}
		} else {
//			SDL_UpdateRect(displayArea, 0, 0, displayArea->w, displayArea->h);
//			SDL_Flip(displayArea);
		}
		bOldFullScan = bFullScan;
}

static void detachsub(void)
{

    detachsub_scaler();
	// 最後にVRAMハンドラ
	if(vramhdr != NULL) {
		delete vramhdr;
		vramhdr = NULL;
	}
	// 最後にVRAMハンドラ
	if(vramhdr_400l != NULL) {
		delete vramhdr_400l;
		vramhdr_400l = NULL;
	}
	// 最後にVRAMハンドラ
	if(vramhdr_4096 != NULL) {
		delete vramhdr_4096;
		vramhdr_4096 = NULL;
	}
	if(vramhdr_256k != NULL) {
		delete vramhdr_256k;
		vramhdr_256k = NULL;
	}
}


static void initsub()
{

	initsub_scaler();
	//	b256kFlag = FALSE;
	vramhdr = new EmuGrphLib();
	vramhdr_400l = new EmuGrph400l();
	vramhdr_4096 = new EmuGrph4096c;
	vramhdr_256k = new EmuGrph256kc;

	if((vramhdr == NULL) || (vramhdr_400l == NULL) ||
			(vramhdr_4096 == NULL) || (vramhdr_256k == NULL)) {
		detachsub();
		return;
	}
	vramhdr->SetPaletteTable((Uint32 *)rgbTTLGDI);
	vramhdr->SetVram(vram_dptr, 80, 200);
	vramhdr_400l->SetPaletteTable((Uint32 *)rgbTTLGDI);
	vramhdr_400l->SetVram(vram_dptr, 80, 400);
	vramhdr_4096->SetPaletteTable((Uint32 *)rgbAnalogGDI);
	vramhdr_4096->SetVram(vram_dptr, 40, 200);
	vramhdr_256k->SetPaletteTable(NULL);
	vramhdr_256k->SetVram(vram_dptr, 40, 200);

	init_scaler();

}


#ifdef __cplusplus
extern "C"
{
#endif


void ResizeGL(int w, int h)
{
	newDrawWidth = w;
	newDrawHeight = h;
	newResize = TRUE;
}
/*
 *  初期化
 */

void ChangeResolution(void)
{
#ifndef USE_AGAR
        SDL_Surface *p;
#endif
        if((nOldDrawHeight == nDrawHeight) && (nOldDrawWidth == nDrawWidth)){
                return;
        }

#if XM7_VER >= 3
        nOldVideoMode = screen_mode;
#else
        nOldVideoMode = mode320;
#endif
        /*
         * KILL Thread
         */
        SDL_SemWait(DrawInitSem);
#ifdef USE_AGAR
        ResizeWindow_Agar(nDrawWidth, nDrawHeight);
#else
#ifdef USE_GTK
        ChangeResolutionGTK(nDrawWidth, nDrawHeight, nDrawWidth, nDrawHeight);
#endif
#endif
        SDL_SemPost(DrawInitSem);
#ifndef USE_AGAR
        displayArea = SDL_GetVideoSurface();
        realDrawArea = SDL_GetVideoSurface();
#endif
        nOldDrawHeight = nDrawHeight;
        nOldDrawWidth = nDrawWidth;
}



void	InitDraw(void)
{
		/*
		 * ワークエリア初期化
		 */
#if XM7_VER >= 3
		bMode = SCR_200LINE;
#else				/*  */
		bAnalog = FALSE;

#endif				/*  */
		bNowBPP = 24;
		memset(rgbTTLGDI, 0, sizeof(rgbTTLGDI));
		memset(rgbAnalogGDI, 0, sizeof(rgbAnalogGDI));
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		nOldDrawHeight = 480;
		nOldDrawWidth = 640;
		nDrawHeight = 480;
		nDrawWidth = 640;
		nDrawFPS = 25;
		nDrawCount = DrawCountSet(nDrawFPS);
		nOldVideoMode =
				bOldFullScan = TRUE;
		bPaletFlag = FALSE;
#if XM7_VER >= 3
		nOldVideoMode = SCR_200LINE;
#else
		nOldVideoMode = FALSE;
#endif
		SetDrawFlag(FALSE);
		nDrawTick1 = 0;


#if XM7_VER >= 3
		bWindowOpen = FALSE;
		nWindowDx1 = 640;
		nWindowDy1 = 400;
		nWindowDx2 = 0;
		nWindowDy2 = 0;
#endif				/*  */
		bDirectDraw = TRUE;
		DrawCond = NULL;
		DrawMutex = NULL;
		DrawThread = NULL;
		DrawINGFlag = FALSE;
		DrawSHUTDOWN = FALSE;
		DrawWaitFlag = FALSE;
		bUseOpenGL = TRUE;

		/*
		 * 直接書き込み→間接書き込み
		 */
		realDrawArea = SDL_GetVideoSurface();
		if(!DrawMutex) {
			DrawMutex = SDL_CreateMutex();
		}
		if(!DrawCond) {
			DrawCond = SDL_CreateCond();
		}
#ifndef USE_AGAR
		if(!DrawThread) {
			DrawThread = SDL_CreateThread(DrawThreadMain,NULL);
			SDL_mutexV(DrawMutex);
		}
#endif
		if(!DrawInitSem) {
			DrawInitSem = SDL_CreateSemaphore(1);
			SDL_SemPost(DrawInitSem);
		}
}


	/*
	 *  クリーンアップ
	 */
void	CleanDraw(void)
{
		int reti;

		DrawSHUTDOWN = TRUE;
		if(DrawCond != NULL) {
			SDL_CondSignal(DrawCond);
		}
#ifndef USE_AGAR
		if(DrawThread != NULL) {
			//			SDL_KillThread(DrawThread);
			SDL_WaitThread(DrawThread, &reti);
			DrawThread = NULL;
		}
#endif
		if(DrawCond != NULL) {
			SDL_DestroyCond(DrawCond);
			DrawCond = NULL;
		}
		if(DrawInitSem != NULL) {
			SDL_DestroySemaphore(DrawInitSem);
		}

		if(DrawMutex != NULL) {
			SDL_DestroyMutex(DrawMutex);
			DrawMutex = NULL;
		}
		detachsub();
}



	/*
	 *  全ての再描画フラグを設定
	 */
void SetDrawFlag(BOOL flag)
{
		memset(GDIDrawFlag, (BYTE) flag, sizeof(GDIDrawFlag));
}

/*
 *  640x200、デジタルモード セレクト
 */
static  BOOL Select640(void)
{

/*
 * 全領域無効
 */
        nDrawTop = 0;
        nDrawBottom = 400;
        nDrawLeft = 0;
        nDrawRight = 640;
        bPaletFlag = TRUE;
        SetDrawFlag(TRUE);

#if XM7_VER >= 3
/*
 * デジタル/200ラインモード
 */
        bMode = SCR_200LINE;

#else				/*  */
/*
 * デジタルモード
 */
        bAnalog = FALSE;

#endif				/*  */
        return TRUE;
}


#if XM7_VER >= 3
    /*
     *  640x400、デジタルモード セレクト
     */
static  BOOL Select400l(void)
{

/*
 * 全領域無効
 */
        nDrawTop = 0;
        nDrawBottom = 400;
        nDrawLeft = 0;
        nDrawRight = 640;
        bPaletFlag = TRUE;
        SetDrawFlag(TRUE);
/*
 * デジタル/400ラインモード
 */
        bMode = SCR_400LINE;
        return TRUE;
}


#endif				/*  */

    /*
     *  320x200、アナログモード セレクト
     */
static  BOOL Select320(void)
{
/*
 * 全領域無効
 */
        nDrawTop = 0;
        nDrawBottom = 400;
        nDrawLeft = 0;
        nDrawRight = 640;
        bPaletFlag = TRUE;
        SetDrawFlag(TRUE);
#if XM7_VER >= 3
/*
 * アナログ/200ラインモード
 */
        bMode = SCR_4096;

#else				/*  */
/*
 * アナログモード
 */
        bAnalog = TRUE;

#endif				/*  */
        return TRUE;
}


#if XM7_VER >= 3
    /*
     *  320x200、26万色モード セレクト
     */
static  BOOL Select256k()
{

/*
 * 全領域無効
 */
        nDrawTop = 0;
        nDrawBottom = 400;
        nDrawLeft = 0;
        nDrawRight = 640;
        bPaletFlag = TRUE;
        SetDrawFlag(TRUE);

/*
 * アナログ(26万色)/200ラインモード
 */
        bMode = SCR_262144;
        return TRUE;
}

/*
 *  セレクト
 */
BOOL SelectDraw(void)
{
	BOOL ret;
	int reti;
#ifdef USE_AGAR
	AG_Driver *drv;
	AG_Color nullcolor;
	AG_Rect rect;
	if(DrawArea == NULL) return FALSE;
	drv = AGWIDGET(DrawArea)->drv;
	if(drv == NULL) return FALSE;
#else
	SDL_Surface *p;
	Uint32 nullcolor;
	SDL_Rect rect;

	p = SDL_GetVideoSurface();
	displayArea = p;
#endif

	rect.h = nDrawWidth;
	rect.w = nDrawHeight;
	rect.x = 0;
	rect.y = 0;
	DrawSHUTDOWN = FALSE;
#ifdef USE_AGAR
	if(drv == NULL) return TRUE;
#else
	if(p == NULL) return TRUE;
#endif
	/*
	 * すべてクリア
	 */
	 if(!bUseOpenGL) {
#ifdef USE_AGAR
		 AG_ObjectLock(DrawArea);
		 nullcolor.r = 0;
		 nullcolor.g = 0;
		 nullcolor.b = 0;
		 nullcolor.a = 255;

		 AG_FillRect(drv->sRef, &rect, nullcolor);
		 AG_ObjectUnlock(DrawArea);
#else
		 SDL_LockSurface(p);
		 nullcolor = SDL_MapRGBA(p->format, 0, 0, 0, 255);
		 SDL_FillRect(p, &rect, nullcolor);
		 SDL_UnlockSurface(p);
#endif
	 } else { // OpenGLのとき
	 }
	 /*
	  * すべてクリア
	  */
#ifndef USE_AGAR
	 if((realDrawArea != p) && (realDrawArea != NULL)) {
		 if(!bUseOpenGL) {
			 SDL_LockSurface(realDrawArea);
			 rect.h = realDrawArea->h;
			 rect.w = realDrawArea->w;
			 rect.x = 0;
			 rect.y = 0;
			 nullcolor = SDL_MapRGBA(realDrawArea->format, 0, 0, 0, 255);
			 SDL_FillRect(realDrawArea, &rect, nullcolor);
			 SDL_UnlockSurface(realDrawArea);
		 } else { // OpenGLのときここ
		 }
	 }
#endif
	 bOldFullScan = bFullScan;
	 if(!DrawMutex) {
		 DrawMutex = SDL_CreateMutex();
	 }
	 if(!DrawCond) {
		 DrawCond = SDL_CreateCond();
	 }
#ifndef USE_AGAR
	 if(!DrawThread) {
		 DrawThread = SDL_CreateThread(DrawThreadMain,NULL);
		 SDL_mutexV(DrawMutex);
	 }
#endif
	 /*
	  * セレクト
	  */
#if XM7_VER >= 3
	 switch (screen_mode) {
	 case SCR_400LINE:
		 ret =  Select400l();
	 case SCR_262144:
		 ret = Select256k();
	 case SCR_4096:
		 ret = Select320();
	 default:
		 ret = Select640();
	 }
#else				/*  */
if (mode320) {
	ret = Select320();
} else {
	ret =  Select640();
}
} else {
	ret = TRUE;
}
#endif				/*  */
	return ret;
}

/*
 *  オールクリア
 */
void AllClear(void)
{
	int            i;
#ifdef USE_AGAR
	AG_Color nullcolor;
	AG_Rect rect;
	AG_Driver *drv;
#else
	Uint32 nullcolor;
	SDL_Surface *p;
	SDL_Rect rect;
#endif
	for (i = 0; i < (80 * 50); i++) {
		GDIDrawFlag[i] = 0;
	}
#ifdef USE_AGAR
	if(DrawArea == NULL) return;
	drv = AGWIDGET(DrawArea)->drv;
	if(drv == NULL) return;
#else
	p = SDL_GetVideoSurface();
	if(p == NULL) return;
#endif
	if(!bUseOpenGL) {
		rect.h = nDrawHeight;
		rect.w = nDrawWidth;
		rect.x = 0;
		rect.y = 0;
		/*
		 * すべてクリア
		 */
#ifdef USE_AGAR
		 AG_ObjectLock(DrawArea);
		 nullcolor.r = 0;
		 nullcolor.g = 0;
		 nullcolor.b = 0;
		 nullcolor.a = 255;
		 AG_FillRect(drv->sRef, &rect, nullcolor);
		 AG_ObjectUnlock(DrawArea);
#else
		SDL_LockSurface(p);
		nullcolor = SDL_MapRGBA(p->format, 0, 0, 0, 255);
		SDL_FillRect(p, &rect, nullcolor);
		SDL_UnlockSurface(p);
		if (bDirectDraw){
			realDrawArea = p;
		} else	{
			if(realDrawArea != NULL) {
				rect.h = realDrawArea->h;
				rect.w = realDrawArea->w;
				rect.x = 0;
				rect.y = 0;
				/*
				 * すべてクリア
				 */
				SDL_LockSurface(realDrawArea);
				nullcolor = SDL_MapRGBA(realDrawArea->format, 0, 0, 0, 255);
				SDL_FillRect(realDrawArea, &rect, nullcolor);
				SDL_UnlockSurface(realDrawArea);
			}
		}
		Flip();
#endif
	} else {
		// OpenGL
	}
	/*
	 * 全領域をレンダリング対象とする
	 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
	bClearFlag = FALSE;
}


/*
 *  フルスキャン
 */
void RenderFullScan(void)
{
	BYTE * p;
	BYTE * q;
	WORD u;
	Uint32 pitch;

#ifdef USE_AGAR
	AG_Driver *drv;
	AG_Color nullcolor;
	AG_Surface *s;

	return;
	if(agDriverOps == NULL) return;
	if(DrawArea == NULL) return;
	drv = AGWIDGET(DrawArea)->drv;
	if(drv == NULL) return;
	s = drv->sRef;
#else
	SDL_Surface *s = SDL_GetVideoSurface();
	Uint32 nullcolor;
#endif
	if(bUseOpenGL) {
#ifdef USE_AGAR
#endif
		return;
	}
	if(s == NULL) {
#ifdef USE_AGAR
#endif
		return;
	}

	if(!bUseOpenGL) {
#ifdef USE_AGAR
		AG_ObjectLock(DrawArea);
#else
		SDL_LockSurface(s);
#endif
		/*
		 * ポインタ初期化
		 */

		p = (BYTE *) s->pixels;
		pitch = s->pitch;
		q = p + pitch;

		/*
		 * ループ
		 */
		for (u = 0; u < nDrawBottom; u += (WORD) 2) {
			memcpy(q, p, pitch);
			p += pitch * 2;
			q += pitch * 2;
		}
#ifdef USE_AGAR
		AG_ObjectUnlock(DrawArea);
#else
		SDL_UnlockSurface(s);
#endif
//		displayArea = SDL_GetVideoSurface();
//		SDL_UpdateRect(s, 0, 0, s->w, s->h);
	}
	Flip();
}


/*
 *  奇数ライン設定
 */
void RenderSetOddLine(void)
{
	WORD u;
	BYTE * p;
	BYTE * q;
	Uint32 pitch;

#ifdef USE_AGAR
	AG_Driver *drv;
	AG_Color nullcolor;
	AG_Surface *s;
	AG_Rect r;
	return;
	if(agDriverOps == NULL) return;
	if(DrawArea == NULL) return;
	drv = AGWIDGET(DrawArea)->drv;
	if(drv == NULL) return;
	s = drv->sRef;
#else
	SDL_Surface *s = SDL_GetVideoSurface();
	SDL_Rect r;
	Uint32 nullcolor;
#endif
	if(bUseOpenGL) {
#ifdef USE_AGAR
#endif
		return;
	}
	if(s == NULL) {
#ifdef USE_AGAR
#endif
		return;
	}

	if(!bUseOpenGL) {
#ifdef USE_AGAR
		AG_ObjectLock(DrawArea);
		nullcolor.r = 0;
		nullcolor.g = 0;
		nullcolor.b = 0;
		nullcolor.a = 255;
#else
		SDL_LockSurface(s);
		nullcolor = SDL_MapRGBA(s->format, 0, 0, 0, 255);
#endif

		switch (nDrawWidth) {
		case 1280:
			r.x = 0;
			r.w = s->w;
			for (u = 2 ; u < nDrawBottom; u += (WORD) 4) {
				r.y = u;
				r.h = 2;
#ifdef USE_AGAR
				AG_FillRect(drv->sRef, &r, nullcolor);
#else
				SDL_FillRect(s, &r, nullcolor);
#endif
			}
			break;
		case 640:
		default:
			r.x = 0;
			r.w = s->w;
			for (u = 1; u < nDrawBottom; u += (WORD) 2) {
				r.y = u;
				r.h = 1;
#ifdef USE_AGAR
				AG_FillRect(drv->sRef, &r, nullcolor);
#else
				SDL_FillRect(s, &r, nullcolor);
#endif
			}
			break;
		}
#ifdef USE_AGAR
		AG_ObjectUnlock(DrawArea);
#else
		SDL_UnlockSurface(s);
		displayArea = SDL_GetVideoSurface();
		SDL_UpdateRect(displayArea, 0, 0, displayArea->w, displayArea->h);
#endif
	}
	Flip();
}


/*
 *  描画(通常)
 */
void OnDraw(void)
{
#if 0
	if(nDrawCount > 0) {
		nDrawCount --;
	} else {
		nDrawCount = DrawCountSet(nDrawFPS);
		if(DrawCond) SDL_CondSignal(DrawCond);
	}
#else
	/*
	 * 描画スレッドのKICKを1/60secごとにする。
	 */
	if(DrawCond) SDL_CondSignal(DrawCond);
#endif
}


/*
 *  描画(PAINT) *GTK依存だが、ダミー。
 */

#ifdef USE_GTK
gint
OnPaint(GtkWidget * widget, GdkEventExpose * event)
#else
int
OnPaint(void)
#endif
{
}


/*-[ VMとの接続 ]-----------------------------------------------------------*/

/*
 *  VRAMセット
 */
void vram_notify(WORD addr, BYTE dat)
{
	WORD x;
	WORD y;

	/*
	 * y座標算出
	 */
#if XM7_VER >= 3
	switch (bMode) {
	case SCR_400LINE:
		addr &= 0x7fff;
		x = (WORD) ((addr % 80) << 3);
		y = (WORD) (addr / 80);
		break;
	case SCR_262144:
	case SCR_4096:
		addr &= 0x1fff;
		x = (WORD) ((addr % 40) << 4);
		y = (WORD) ((addr / 40) << 1);
		break;
	case SCR_200LINE:
		addr &= 0x3fff;
		x = (WORD) ((addr % 80) << 3);
		y = (WORD) ((addr / 80) << 1);
		break;
	}

#else				/*  */
	if (bAnalog) {
		addr &= 0x1fff;
		x = (WORD) ((addr % 40) << 4);
		y = (WORD) ((addr / 40) << 1);
	}

	else {
		addr &= 0x3fff;
		x = (WORD) ((addr % 80) << 3);
		y = (WORD) ((addr / 80) << 1);
	}

#endif				/*  */

	/*
	 * オーバーチェック
	 */
	if ((x >= 640) || (y >= 400)) {
		return;
	}

	/*
	 * 再描画フラグを設定
	 */
	GDIDrawFlag[(y >> 3) * 80 + (x >> 3)] = 1;

	/*
	 * 垂直方向更新
	 */
	if (nDrawTop > y) {
		nDrawTop = y;
	}
	if (nDrawBottom <= y) {

#if XM7_VER >= 3
		if (bMode == SCR_400LINE) {
			nDrawBottom = (WORD) (y + 1);
		}

		else {
			nDrawBottom = (WORD) (y + 2);
		}

#else				/*  */
		nDrawBottom = (WORD) (y + 2);

#endif				/*  */
	}

	/*
	 * 水平方向更新
	 */
	if (nDrawLeft > x) {
		nDrawLeft = x;
	}
	if (nDrawRight <= x) {

#if XM7_VER >= 3
		if (bMode & SCR_ANALOG) {

#else				/*  */
			if (bAnalog) {

#endif				/*  */
				nDrawRight = (WORD) (x + 16);
			}

			else {
				nDrawRight = (WORD) (x + 8);
			}
		}

	}


	/*
	 *  TTLパレットセット
	 */
void	ttlpalet_notify(void)
{

	/*
	 * 不要なレンダリングを抑制するため、領域設定は描画時に行う
	 */
	bPaletFlag = TRUE;
}

/*
 *  アナログパレットセット
 */
void 	apalet_notify(void)
{
	bPaletFlag = TRUE;
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
}

/*
 *  再描画要求
 */
void 	display_notify(void)
{

	/*
	 * 再描画
	 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	bClearFlag = TRUE;
	SetDrawFlag(TRUE);
}

/*
 *  ディジタイズ要求通知
 */
void	digitize_notify(void)
{

}
#if XM7_VER >= 3
/*
 *  ハードウェアウィンドウ通知
 */
void window_notify(void)
{
	WORD tmpLeft, tmpRight;
	WORD tmpTop, tmpBottom;
	WORD tmpDx1, tmpDx2;
	WORD tmpDy1, tmpDy2;
	BYTE * p;
	int     i;

	/*
	 * 26万色モード時は何もしない
	 */
	if (bMode == SCR_262144) {
		return;
	}

	/*
	 * 前もってクリッピングする
	 */
	window_clip(bMode);

	/*
	 * ウィンドウサイズを補正
	 */
	tmpDx1 = window_dx1;
	tmpDy1 = window_dy1;
	tmpDx2 = window_dx2;
	tmpDy2 = window_dy2;
	if (bMode != SCR_400LINE) {
		tmpDy1 <<= 1;
		tmpDy2 <<= 1;
	}
	if (bMode == SCR_4096) {
		tmpDx1 <<= 1;
		tmpDx2 <<= 1;
	}
	if (bWindowOpen != window_open) {
		if (window_open) {

			/*
			 * ウィンドウを開いた場合
			 */
			 tmpLeft = tmpDx1;
			 tmpRight = tmpDx2;
			 tmpTop = tmpDy1;
			 tmpBottom = tmpDy2;
		} else {

			/*
			 * ウィンドウを閉じた場合
			 */
			tmpLeft = nWindowDx1;
			tmpRight = nWindowDx2;
			tmpTop = nWindowDy1;
			tmpBottom = nWindowDy2;
		}
	} else {
		if (window_open) {

			/*
			 * 更新領域サイズを現在のものに設定
			 */
			tmpTop = nDrawTop;
			tmpBottom = nDrawBottom;
			tmpLeft = nDrawLeft;
			tmpRight = nDrawRight;

			/*
			 * 座標変更チェック
			 */
			 if (!((nWindowDx1 == tmpDx1) &&
					 (nWindowDy1 == tmpDy1) &&
					 (nWindowDx2 == tmpDx2) && (nWindowDy2 == tmpDy2))) {

				 /*
				  * 左上X
				  */
				  if (nWindowDx1 < tmpDx1) {
					  tmpLeft = nWindowDx1;
				  } else {
					  tmpLeft = tmpDx1;
				  }

				  /*
				   * 右下X
				   */
				  if (nWindowDx2 > tmpDx2) {
					  tmpRight = nWindowDx2;
				  } else {
					  tmpRight = tmpDx2;
				  }

				  /*
				   * 左上Y
				   */
				  if (nWindowDy1 < tmpDy1) {
					  tmpTop = nWindowDy1;
				  } else {
					  tmpTop = tmpDy1;
				  }

				  /*
				   * 右下Y
				   */
				  if (nWindowDy2 > tmpDy2) {
					  tmpBottom = nWindowDy2;
				  } else {
					  tmpBottom = tmpDy2;
				  }
			 }
		} else {
			/*
			 * ウィンドウが開いていないので何もしない
			 */
			return;
		}
	}

	/*
	 * 処理前の再描画領域と比較して広ければ領域を更新
	 */
	if (tmpLeft < nDrawLeft) {
		nDrawLeft = tmpLeft;
	}
	if (tmpRight > nDrawRight) {
		nDrawRight = tmpRight;
	}
	if (tmpTop < nDrawTop) {
		nDrawTop = tmpTop;
	}
	if (tmpBottom > nDrawBottom) {
		nDrawBottom = tmpBottom;
	}

	/*
	 * 再描画フラグを更新
	 */
	 if ((nDrawLeft < nDrawRight) && (nDrawTop < nDrawBottom)) {
		 p = &GDIDrawFlag[(nDrawTop >> 3) * 80 + (nDrawLeft >> 3)];
		 for (i = (nDrawTop >> 3); i < ((nDrawBottom + 7) >> 3); i++) {
			 memset(p, 1, (640 - 0) >> 3);
			 p += 80;
		 }
	 }

	 /*
	  * ウィンドウオープン状態を保存
	  */
	 bWindowOpen = window_open;
	 nWindowDx1 = tmpDx1;
	 nWindowDy1 = tmpDy1;
	 nWindowDx2 = tmpDx2;
	 nWindowDy2 = tmpDy2;
}
#endif				/*  */
void OnFullScreen(void)
{
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
	if(p != NULL) {
		SDL_WM_ToggleFullScreen(p);
	}
}
void	OnWindowedScreen(void)
{
#ifdef USE_AGAR
#else
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
	if(p != NULL) {
		SDL_WM_ToggleFullScreen(p);
	}
#endif
}
#ifdef __cplusplus
}
#endif
/*
 * パレット関連処理
 */

static void Palet640Sub(Uint32 i, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
#ifdef USE_AGAR
	if(scalerGL != NULL){
		scalerGL->CalcPalette(i, r, g, b, a);
	}
#else
	SDL_Surface *p;
#if 1
	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	if(vramhdr != NULL) {
		vramhdr->CalcPalette(i, r, g, b, a, p);
	}
//	if(vramhdr_400l != NULL) {
//		vramhdr_400l->CalcPalette(i, r, g, b, a, p);
//	}
#else
	if(vramhdr != NULL) {
		vramhdr->CalcPalette(i, r, g, b, a);
	}
	if(vramhdr_400l != NULL) {
		vramhdr_400l->CalcPalette(i, r, g, b, a);
	}
#endif
#endif
}

#ifdef __cplusplus
extern "C"
{
#endif

void Palet640(void)
{
	int i;
	int             vpage;
	BYTE          tmp;
	BYTE         g,r,b;
	BYTE         a = 255; // Alpha

	vpage = (~(multi_page >> 4)) & 0x07;
	if(vramhdr != NULL) {
		vramhdr->SetPaletteTable((Uint32 *)rgbTTLGDI);
	}
	if(vramhdr_400l != NULL) {
		vramhdr_400l->SetPaletteTable((Uint32 *)rgbTTLGDI);
	}
#ifdef USE_AGAR
	if(scalerGL != NULL){
		scalerGL->SetPaletteTable((Uint32 *)rgbTTLGDI);
	}
#endif

	for (i = 0; i < 8; i++) {
		if (crt_flag) {
			/*
			 * CRT ON
			 */
			tmp = ttl_palet[i & vpage] & 0x07;
			b = ((tmp & 0x01)==0)?0:255;
			r = ((tmp & 0x02)==0)?0:255;
			g = ((tmp & 0x04)==0)?0:255;
			Palet640Sub(i & 7, r, g, b, a);
		} else {
			/*
			 * CRT OFF
			 */
			r = 0;
			g = 0;
			b = 0;
			Palet640Sub(i & 7, r, g, b, a);
		}
	}
	/*
	 * 奇数ライン用
	 */
	Palet640Sub(8, 0, 0, 0, a);
	Palet640Sub(9, 0, 255, 0, a);
}

static inline void Palet320Sub(Uint32 i, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
#ifdef USE_AGAR
	if(scalerGL != NULL){
		scalerGL->CalcPalette(i, r, g, b, a);
	}
#else
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	if(vramhdr_4096 != NULL) {
		vramhdr_4096->CalcPalette(i, r, g, b, a, p);
	}
#endif
}

void Palet320(void)
{

	int     i,
	j;
	DWORD   color;
	DWORD   r,
	g,
	b;
	int     amask;

	/*
	 * アナログマスクを作成
	 */
	 if(vramhdr != NULL) {
		 vramhdr->SetPaletteTable((Uint32 *)rgbAnalogGDI);
	 }
#ifdef USE_AGAR
	if(scalerGL != NULL){
		scalerGL->SetPaletteTable((Uint32 *)rgbAnalogGDI);
	}
#endif
	 amask = 0;
	 if (!(multi_page & 0x10)) {
		 amask |= 0x000f;
	 }
	 if (!(multi_page & 0x20)) {
		 amask |= 0x00f0;
	 }
	 if (!(multi_page & 0x40)) {
		 amask |= 0x0f00;
	 }
	 for (i = 0; i < 4096; i++) {
		 /*
		  * 最下位から5bitづつB,G,R
		  */
		  color = 0;
		  if (crt_flag) {
			  j = i & amask;
			  r = apalet_r[j] <<4;
			  g = apalet_g[j] <<4;
			  b = apalet_b[j] <<4;
		  } else {
			  r = 0;
			  g = 0;
			  b = 0;
		  }
		  Palet320Sub(i, r, g, b, 255);
	 }
#endif
}



#ifdef __cplusplus
}
#endif

/*
 * 描画処理
 */


#ifdef __cplusplus
extern "C"
{
#endif



void Draw640All(void)
{
	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	WORD wdtop, wdbtm;
	SDL_Surface *p;

#ifdef USE_AGAR
	AG_Driver *drv;

	if(agDriverOps == NULL) return;
//	drv = AG_DriverOpen(agDriverOps);
//	drv = agDriverSw;
//	if(drv == NULL) return;
//	p = (SDL_Surface *)(drv->sRef);

#else
	p = SDL_GetVideoSurface();
#endif
//	if(p == NULL) return;
	/*
	 * パレット設定
	 */
	/*
	 *描画モードを変えたら強制的にPalet640すること。
	 */
	SetVramReader_200l();
	if(bPaletFlag) {
		Palet640();
		bPaletFlag = FALSE;
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}
	/*
	 * クリア処理
	 */
	if (bClearFlag) {
		AllClear();
	}

	if(!bUseOpenGL) {
		if(bFullScan) {
			switch(nDrawWidth) {
			case 1280:
				if(scaler2x4 != NULL) {
					PutVramFunc = &Scaler_2x4;
				}
				break;
			case 640:
			default:
				if(scaler1x1 != NULL) {
					PutVramFunc = &Scaler_1x2;
				}
				break;
			}
		} else {
			switch(nDrawWidth) {
			case 1280:
				if(scaler2x4i != NULL) {
					PutVramFunc = &Scaler_2x4i;
				}
				break;
			case 640:
			default:
				if(scaler1x2i != NULL) {
					PutVramFunc = &Scaler_1x2i;
				}
				break;
			}
		}
	} else {
		if(scalerGL != NULL) {
			PutVramFunc = &Scaler_GL;
		}
	}
	/*
	 * レンダリング
	 */
	if(PutVramFunc == NULL) return;
	if(vramhdr_400l == NULL) return;
	if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		if(window_open) { // ハードウェアウインドウ開いてる
			if ((nDrawTop >> 1) < window_dy1) {
				vramhdr->SetVram(vram_dptr, 80, 200);
				PutVramFunc(p, 0, nDrawTop >> 1, 640, window_dy1, multi_page);
			}
			/* ウィンドウ内の描画 */
			if ((nDrawTop >> 1) > window_dy1) {
				wdtop = nDrawTop >> 1;
			}
			else {
				wdtop = window_dy1;
			}

			if ((nDrawBottom >> 1)< window_dy2) {
				wdbtm = nDrawBottom >> 1;
			}
			else {
				wdbtm = window_dy2;
			}

			if (wdbtm > wdtop) {
				vramhdr->SetVram(vram_bdptr, 80, 200);
				PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
			}
			/* ハードウェアウインドウ外下部 */
			if ((nDrawBottom >> 1) > window_dy2) {
				vramhdr->SetVram(vram_dptr, 80, 200);
				PutVramFunc(p, 0 , wdbtm, 640, (nDrawBottom >> 1) - wdbtm, multi_page);
			}
		} else { // ハードウェアウィンドウ開いてない
			vramhdr->SetVram(vram_dptr, 80, 200);
			PutVramFunc(p, 0, 0, 640, 200, multi_page);
		}
	}

	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	//	bPaletFlag = FALSE;
	SetDrawFlag(FALSE);
}


void Draw400l(void)
{

	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	SDL_Surface *p;
	WORD wdtop, wdbtm;
	/*
	 * パレット設定
	 */
	SetVramReader_400l();
	if (bPaletFlag) {
		Palet640();
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}


	/*
	 * クリア処理
	 */
	 if (bClearFlag) {
		 AllClear();
	 }

	/*
	 * レンダリング
	 */
#ifdef USE_AGAR

#else
	 p = SDL_GetVideoSurface();
	 if(p == NULL) return;
#endif
	 /*
	  * レンダリング
	  */
	 if(!bUseOpenGL) {
		 if(bFullScan) {
			 switch(nDrawWidth) {
			 case 1280:
				 if(scaler2x2 != NULL) {
					 PutVramFunc = &Scaler_2x2;
				 }
				 break;
			 case 640:
			 default:
				 if(scaler1x1 != NULL) {
					 PutVramFunc = &Scaler_1x1;
				 }
				 break;
			 }
		 } else {
			 switch(nDrawWidth) {
			 case 1280:
				 if(scaler2x2i != NULL) {
					 PutVramFunc = &Scaler_2x2i;
				 }
				 break;
			 case 640:
			 default:
				 if(scaler1x1 != NULL) {
					 PutVramFunc = &Scaler_1x1;
				 }
				 break;
			 }
		 }
	 } else {
		 if(scalerGL != NULL) {
			 PutVramFunc = &Scaler_GL;
		 }
	 }
	 if(PutVramFunc == NULL) return;
	 if(vramhdr_400l == NULL) return;
	 if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		 if(window_open) { // ハードウェアウインドウ開いてる
			 if (nDrawTop < window_dy1) {
				 vramhdr_400l->SetVram(vram_dptr, 80, 400);
				 PutVramFunc(p, 0, nDrawTop, 640, window_dy1, multi_page);
			 }
			 /* ウィンドウ内の描画 */
			 if (nDrawTop > window_dy1) {
				 wdtop = nDrawTop;
			 }
			 else {
				 wdtop = window_dy1;
			 }

			 if (nDrawBottom < window_dy2) {
				 wdbtm = nDrawBottom;
			 }
			 else {
				 wdbtm = window_dy2;
			 }

			 if (wdbtm > wdtop) {
				 vramhdr_400l->SetVram(vram_bdptr, 80, 400);
				 PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
			 }
			 /* ハードウェアウインドウ外下部 */
			 if (nDrawBottom  > window_dy2) {
				 vramhdr_400l->SetVram(vram_dptr, 80, 400);
				 PutVramFunc(p, 0 , wdbtm, 640, nDrawBottom - wdbtm, multi_page);
			 }
		 } else { // ハードウェアウィンドウ開いてない
			 vramhdr_400l->SetVram(vram_dptr, 80, 400);
			 PutVramFunc(p, 0, 0, 640, 400, multi_page);
		 }
	 }
	 nDrawTop = 0;
	 nDrawBottom = 400;
	 nDrawLeft = 0;
	 nDrawRight = 640;
	 bPaletFlag = FALSE;
	 //SetDrawFlag(FALSE);

}


void Draw320(void)
{
	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	SDL_Surface *p;
	WORD wdtop, wdbtm;

	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	/*
	 * パレット設定
	 */
	if(!bUseOpenGL) {
		if(bFullScan) {
			switch(nDrawWidth) {
			case 1280:
				PutVramFunc = &Scaler_4x4;
				break;
			case 640:
			default:
				PutVramFunc = &Scaler_2x2;
				break;
			}
		} else {
			switch(nDrawWidth) {
			case 1280:
				PutVramFunc = &Scaler_4x4i;
				break;
			case 640:
			default:
				PutVramFunc = &Scaler_2x2i;
				break;
			}
		}
	} else {
		PutVramFunc = &Scaler_GL;
	}

	//	if(bPaletFlag) {
	Palet320();
	SetVramReader_4096();
	nDrawTop = 0;
	nDrawBottom = 200;
	nDrawLeft = 0;
	nDrawRight = 320;
	//SetDrawFlag(TRUE);
	//	}
	/*
	 * クリア処理
	 */
	if (bClearFlag) {
		AllClear();
	}
	/*
	 * レンダリング
	 */
	if(PutVramFunc == NULL) return;
	if(vramhdr == NULL) return;
	if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		if(window_open) { // ハードウェアウインドウ開いてる
			if (nDrawTop < window_dy1) {
				vramhdr_4096->SetVram(vram_dptr, 40, 200);
				PutVramFunc(p, 0, nDrawTop, 320, window_dy1, multi_page);
			}
			/* ウィンドウ内の描画 */
			if (nDrawTop > window_dy1) {
				wdtop = nDrawTop;
			}
			else {
				wdtop = window_dy1;
			}

			if (nDrawBottom < window_dy2) {
				wdbtm = nDrawBottom;
			}
			else {
				wdbtm = window_dy2;
			}

			if (wdbtm > wdtop) {
				vramhdr_4096->SetVram(vram_bdptr, 40, 200);
				PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
			}
			/* ハードウェアウインドウ外下部 */
			if (nDrawBottom  > window_dy2) {
				vramhdr_4096->SetVram(vram_dptr, 40, 200);
				PutVramFunc(p, 0 , wdbtm, 320, nDrawBottom - wdbtm, multi_page);
			}
		} else { // ハードウェアウィンドウ開いてない
			vramhdr_4096->SetVram(vram_dptr, 40, 200);
			PutVramFunc(p, 0, 0, 320, 200, multi_page);
		}
	}

	nDrawTop = 0;
	nDrawBottom = 200;
	nDrawLeft = 0;
	nDrawRight = 320;
	bPaletFlag = FALSE;
	//    SetDrawFlag(FALSE);
}

void Draw256k(void)
{
	SDL_Surface *p;
	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);

	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	if(!bUseOpenGL) {
		if(bFullScan) {
			switch(nDrawWidth) {
			case 1280:
				PutVramFunc = &Scaler_4x4;
				break;
			case 640:
			default:
				PutVramFunc = &Scaler_2x2;
				break;
			}
		} else {
			switch(nDrawWidth) {
			case 1280:
				PutVramFunc = &Scaler_4x4i;
				break;
			case 640:
			default:
				PutVramFunc = &Scaler_2x2i;
				break;
			}
		}
	} else {
		PutVramFunc = &Scaler_GL;
	}
	nDrawTop = 0;
	nDrawBottom = 200;
	nDrawLeft = 0;
	nDrawRight = 320;
	//    SetDrawFlag(TRUE);

	/*
	 * クリア処理
	 */
	if (bClearFlag) {
		AllClear();
	}

	/*
	 * レンダリング
	 */
	if(PutVramFunc == NULL) return;
	if(vramhdr_256k == NULL) return;
	/*
	 * 26万色モードの時は、ハードウェアウィンドウを考慮しない。
	 */
	vramhdr_256k->SetVram(vram_dptr, 40, 200);
	SetVramReader_256k();
	PutVramFunc(p, 0, 0, 320, 200, multi_page);
	nDrawTop = 0;
	nDrawBottom = 200;
	nDrawLeft = 0;
	nDrawRight = 320;
	bPaletFlag = FALSE;
	//    SetDrawFlag(FALSE);
}


#ifdef __cplusplus
}
#endif

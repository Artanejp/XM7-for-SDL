/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2003 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 *  [SDL 表示 ]
 *  2010.10.28 sdl_draw.c から移動
 */


#include <SDL.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#ifdef USE_OPENGL
  #include "agar_gldraw.h"
#endif /* USE_OPENGL */

//#include "sdl.h"

#include "api_vram.h"
#include "api_draw.h"
//#include "api_scaler.h"

/*
 *  グローバル ワーク
 */
DWORD   rgbTTLGDI[16];	/* デジタルパレット */
DWORD   rgbAnalogGDI[4096];	/* アナログパレット */
struct DrawPieces SDLDrawFlag;

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
WORD 			nEmuFPS; /* エミュレーションFPS値 20110123 */
WORD 			nAspect;  /* アスペクト比 20110123 */
BOOL			bSyncToVSYNC; /* VSYNC同期(OpenGLのみ) */
BOOL 			bSmoosing; /* スムージング処理する(GLのみ?) */
BOOL            bOldFullScan;	/* クリアフラグ(過去) */

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
BOOL DrawINGFlag;
BOOL DrawSHUTDOWN;
BOOL DrawWaitFlag;


AG_Cond DrawCond;
AG_Mutex DrawMutex;
AG_Thread DrawThread;

int newDrawWidth;
int newDrawHeight;
BOOL newResize;

extern Uint32 nDrawTick1D;
extern Uint32 nDrawTick1E;
#ifdef USE_OPENGL
extern GLuint uVramTextureID;
#endif /* USE_OPENGL */


/*
 *  プロトタイプ宣言
 */

/*
 * ビデオドライバ関連
 */


void ResizeWindow(int w, int h)
{
//    char          EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
//    SDL_SysWMinfo sdlinfo;
            ResizeWindow_Agar(w, h);
}

static void initsub(void);
static void detachsub(void);


void AG_DrawInitsub(void)
{
//	initsub();
}
void AG_DrawDetachsub(void)
{
//	detachsub();
}

void Flip(void)
{

}
/*
 *  BITBLT
 */
BOOL XM7_BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight,
			int nSrcLeft, int nSrcTop)
{

	AG_Rect srcrect, dstrect;

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
		/*
		 * 擬似インタレース設定をここでやる
		 */
		if (bOldFullScan != bFullScan) {
			if (!bFullScan) {
				RenderSetOddLine();
			} else {
				RenderFullScan();
			}
		}
		bOldFullScan = bFullScan;
		return TRUE;
}

static void detachsub(void)
{

//    detachsub_scaler();
	// 最後にVRAMハンドラ
}


static void initsub()
{

//	initsub_scaler();
	//	b256kFlag = FALSE;
    SetVram_200l(vram_dptr);

//	init_scaler();

}


extern "C"
{

void ResizeGL(int w, int h)
{
	newDrawWidth = w;
	newDrawHeight = h;
	newResize = TRUE;
}

/*
 *  初期化
 */



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
		memset((void *)&SDLDrawFlag, 0, sizeof(SDLDrawFlag));

		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		nOldDrawHeight = 480;
		nOldDrawWidth = 640;
		nDrawHeight = 480;
		nDrawWidth = 640;
		nDrawFPS = 25;
		nEmuFPS = 20;
		nAspect = nAspectFree;
		bSyncToVSYNC = TRUE;
		bSmoosing = FALSE;

		bFullScan = TRUE;
		bPaletFlag = FALSE;
		bOldFullScan = bFullScan;
#if XM7_VER >= 3
		nOldVideoMode = SCR_200LINE;
#else
		nOldVideoMode = FALSE;
#endif
		SetDrawFlag(FALSE);
		nDrawTick1D = 0;
		nDrawTick1E = 0;
		newResize = FALSE;


#if XM7_VER >= 3
		bWindowOpen = FALSE;
		nWindowDx1 = 640;
		nWindowDy1 = 400;
		nWindowDx2 = 0;
		nWindowDy2 = 0;
#endif				/*  */
		bDirectDraw = TRUE;
		DrawINGFlag = FALSE;
		DrawSHUTDOWN = FALSE;
		DrawWaitFlag = FALSE;
//		bUseOpenGL = TRUE;

		/*
		 * 直接書き込み→間接書き込み
		 */

//        realDrawArea = GetDrawSurface();
		AG_MutexInit(&DrawMutex);
		AG_CondInit(&DrawCond);
		AG_MutexUnlock(&DrawMutex);
		AG_ThreadCreate(&DrawThread, DrawThreadMain, NULL);
		if(!DrawInitSem) {
			DrawInitSem = SDL_CreateSemaphore(1);
			SDL_SemPost(DrawInitSem);
		}
		/*
		 *  VRAMテクスチャ生成
		 */
#ifdef USE_OPENGL
                 uVramTextureID = 0;
#endif /* USE_OPENGL */
   //		 initvramtbl_8_vec();
		 initvramtbl_4096_vec();
}


	/*
	 *  クリーンアップ
	 */
void	CleanDraw(void)
{

		DrawSHUTDOWN = TRUE;
		AG_CondSignal(&DrawCond);
		AG_ThreadJoin(DrawThread, NULL);

//		AG_MutexDestroy(&DrawMutex);
//		AG_CondDestroy(&DrawCond);
//                DrawThread = NULL;

		if(DrawInitSem != NULL) {
			SDL_DestroySemaphore(DrawInitSem);
		}

   //		 detachvramtbl_8_vec();
		 detachvramtbl_4096_vec();
		detachsub();
}



	/*
	 *  全ての再描画フラグを設定
	 */
void SetDrawFlag(BOOL flag)
{
    int x;
    int y;
    int ip;
   
#ifdef _OPENMP
       #pragma omp parallel for shared(SDLDrawFlag, flag) private(x)
#endif
   for(y = 0; y < 50 ; y++) {
        for(x = 0; x < 80; x++){
           SDLDrawFlag.read[x][y] =
           SDLDrawFlag.write[x][y] = (uint8_t) flag;
        }
    }
    SDLDrawFlag.Drawn = flag;
    SDLDrawFlag.APaletteChanged = flag;
    SDLDrawFlag.DPaletteChanged = flag;
    SDLDrawFlag.ForcaReDraw = flag;
}

/*
 *  640x200、デジタルモード セレクト
 */
BOOL Select640(void)
{

   /*
    * 全領域無効
    */
   nDrawTop = 0;
   nDrawBottom = 200;
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
BOOL Select400l(void)
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
BOOL Select320(void)
{
/*
 * 全領域無効
 */
   nDrawTop = 0;
   nDrawBottom = 200;
   nDrawLeft = 0;
   nDrawRight = 320;
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
BOOL Select256k()
{
   /*
    * 全領域無効
    */
   nDrawTop = 0;
   nDrawBottom = 200;
   nDrawLeft = 0;
   nDrawRight = 320;
   bPaletFlag = TRUE;
   SetDrawFlag(TRUE);
   
   /*
    * アナログ(26万色)/200ラインモード
    */
   bMode = SCR_262144;
   return TRUE;
}
#endif
   
/*
 *  セレクト
 */
BOOL SelectDraw(void)
{
   BOOL ret;
   AG_Color nullcolor;
   AG_Rect rect;
   AG_Widget *w;


   rect.h = nDrawWidth;
   rect.w = nDrawHeight;
   rect.x = 0;
   rect.y = 0;
   DrawSHUTDOWN = FALSE;
   /*
    * すべてクリア
    */
   if(!bUseOpenGL) {
    if(DrawArea != NULL) {
       AG_ObjectLock(DrawArea);
       nullcolor.r = 0;
       nullcolor.g = 0;
       nullcolor.b = 0;
       nullcolor.a = 255;
       AG_FillRect(AGWIDGET(DrawArea)->drv->sRef , &rect, nullcolor);
       AG_ObjectUnlock(DrawArea);
    }
   } else { // OpenGLのとき
   }
   /*
    * すべてクリア
    */
	 bOldFullScan = bFullScan;
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
	int x;
	int y;
	AG_Color nullcolor;
	AG_Rect rect;
	AG_Driver *drv;
	AG_Widget *w;
     SetDrawFlag(TRUE);
#ifdef USE_OPENGL
   if(DrawArea != NULL) {
        drv = AGWIDGET(DrawArea)->drv;
        if(drv == NULL) return;
        w = AGWIDGET(DrawArea);
    } else if(GLDrawArea != NULL) {
        drv = AGWIDGET(GLDrawArea)->drv;
        if(drv == NULL) return;
        w = AGWIDGET(GLDrawArea);
    } else {
        return;
    }
#else /* USE_OPENGL */
   if(DrawArea != NULL) {
        drv = AGWIDGET(DrawArea)->drv;
        if(drv == NULL) return;
        w = AGWIDGET(DrawArea);
    } else {
        return;
    }

#endif /* USE_OPENGL */
   if(!bUseOpenGL) {
      rect.h = nDrawHeight;
      rect.w = nDrawWidth;
      rect.x = 0;
      rect.y = 0;
      /*
       * すべてクリア
       */
      AG_ObjectLock(w);
      nullcolor.r = 0;
      nullcolor.g = 0;
      nullcolor.b = 0;
      nullcolor.a = 255;
      AG_FillRect(drv->sRef, &rect, nullcolor);
      AG_ObjectUnlock(w);
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
   return; // Scalerがやるので
}


/*
 *  奇数ライン設定
 */
void RenderSetOddLine(void)
{
   return; // Scalerがやるので
}


/*
 *  描画(通常)
 */
void OnDraw(void)
{
   /*
    * 描画スレッドのKICKを1/60secごとにする。
    */
   AG_CondSignal(&DrawCond);
}


/*
 *  描画(PAINT) *GTK依存だが、ダミー。
 */

int OnPaint(void)
{
    return 1;
}


/*-[ VMとの接続 ]-----------------------------------------------------------*/

/*
 *  VRAMセット
 */
void vram_notify(WORD addr, BYTE dat)
{
	int x;
	int y;
	int xx;
	int yy;

	/*
	 * y座標算出
	 */
#if XM7_VER >= 3
	switch (bMode) {
	case SCR_400LINE:
		addr &= 0x7fff;
		x = addr % 80;
		y = (addr / 80) >>3;
		xx = (addr % 80) << 3;
		yy = addr / 80;
		break;
	case SCR_262144:
	case SCR_4096:
		addr &= 0x1fff;
		x = addr % 40;
		y = (addr / 40) >> 3;
		xx = (addr % 40) << 4;
		yy = (addr / 40) << 1;
		break;
	case SCR_200LINE:
		addr &= 0x3fff;
		x = addr % 80;
		y = (addr / 80) >>3;
		xx = (addr % 80) << 3;
		yy = (addr / 80) << 1;
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
	if ((xx >= 640) || (yy >= 400)) {
		return;
	}

	/*
	 * 再描画フラグを設定
	 */
        LockVram();
        SDLDrawFlag.read[x][y] = TRUE;
        UnlockVram();
	/*
	 * 垂直方向更新
	 */
	if (nDrawTop > yy) {
		nDrawTop = yy;
	}
	if (nDrawBottom <= yy) {

#if XM7_VER >= 3
		if (bMode == SCR_400LINE) {
			nDrawBottom = (WORD) (yy + 1);
		}

		else {
			nDrawBottom = (WORD) (yy + 2);
		}

#else				/*  */
		nDrawBottom = (WORD) (yy + 2);

#endif				/*  */
	}

	/*
	 * 水平方向更新
	 */
	if (nDrawLeft > xx) {
		nDrawLeft = xx;
	}
	if (nDrawRight <= xx) {

#if XM7_VER >= 3
		if (bMode & SCR_ANALOG) {

#else				/*  */
			if (bAnalog) {

#endif				/*  */
				nDrawRight = (WORD) (xx + 16);
			}

			else {
				nDrawRight = (WORD) (xx + 8);
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
	SDLDrawFlag.DPaletteChanged = TRUE;
        SetDrawFlag(TRUE);
}

/*
 *  アナログパレットセット
 */
void 	apalet_notify(void)
{
	bPaletFlag = TRUE;
	SDLDrawFlag.APaletteChanged = TRUE;
//	nDrawTop = 0;
//	nDrawBottom = 200;
//	nDrawLeft = 0;
//	nDrawRight = 320;
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
//	nDrawTop = 0;
//	nDrawBottom = 400;
//	nDrawLeft = 0;
//	nDrawRight = 640;
	bPaletFlag = TRUE;
	bClearFlag = TRUE;
//	SetDrawFlag(TRUE);
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
	int x, y;
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
	     for(y = nDrawTop >> 3; y < ((nDrawBottom + 7) >> 3); y++) {
	         for(x = nDrawLeft >> 3; x < (nDrawRight >>3); x ++){
                SDLDrawFlag.read[x][y] = TRUE;
	         }
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
   return; // Full Screenは実装考える 20121128
}
void	OnWindowedScreen(void)
{
   return; // Full Screenは実装考える 20121128
}

}

/*
 * パレット関連処理
 */

static void Palet640Sub(Uint32 i, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    CalcPalette_8colors((int)i, r, g, b, a);
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
//	CalcPalette_AG_GL(rgbAnalogGDI, i, r, g, b, a);
    CalcPalette_4096Colors(i, r, g, b, a);
}

void Palet320(void)
{

	int     i,
	j;
	DWORD   r,
	g,
	b;
	int     amask;

	/*
	 * アナログマスクを作成
	 */
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
//     LockVram();
	 for (i = 0; i < 4096; i++) {
		 /*
		  * 最下位から5bitづつB,G,R
		  */
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
//     UnlockVram();

}



#ifdef __cplusplus
}
#endif

/*
 * 描画処理
 */


extern "C"
{

void Draw640All(void)
{
   void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
   WORD wdtop, wdbtm;
   SDL_Surface *p;

   if(agDriverOps == NULL) return;
   p = GetDrawSurface();
   /*
    * パレット設定
    */
   /*
    *描画モードを変えたら強制的にPalet640すること。
    */
   if(bPaletFlag) { // 描画モードでVRAM変更
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
   PutVramFunc = &PutVram_AG_SP;
   /*
    * レンダリング
    */
   if(PutVramFunc == NULL) return;
   if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
      if(window_open) { // ハードウェアウインドウ開いてる
	 if ((nDrawTop >> 1) < window_dy1) {
	    SetVram_200l(vram_dptr);
	    PutVramFunc(p, 0, nDrawTop >> 1, 640, window_dy1, multi_page);
	 }
	 /* ウィンドウ内の描画 */
	 if ((nDrawTop >> 1) > window_dy1) {
	    wdtop = nDrawTop >> 1;
	 } else {
	    wdtop = window_dy1;
	 }

	 if ((nDrawBottom >> 1)< window_dy2) {
	    wdbtm = nDrawBottom >> 1;
	 }
	 else {
	    wdbtm = window_dy2;
	 }
	 
	 if (wdbtm > wdtop) {
	    //		vramhdr->SetVram(vram_bdptr, 80, 200);
	    SetVram_200l(vram_bdptr);
	    PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
	 }
	 /* ハードウェアウインドウ外下部 */
	 if ((nDrawBottom >> 1) > window_dy2) {
	    //	vramhdr->SetVram(vram_dptr, 80, 200);
	    SetVram_200l(vram_dptr);
	    PutVramFunc(p, 0 , wdbtm, 640, (nDrawBottom >> 1) - wdbtm, multi_page);
	 }
      } else { // ハードウェアウィンドウ開いてない
	 //	vramhdr->SetVram(vram_dptr, 80, 200);
	 SetVram_200l(vram_dptr);
	 PutVramFunc(p, 0, 0, 640, 200, multi_page);
      }
   }
   nDrawTop = 0;
   nDrawBottom = 400;
   nDrawLeft = 0;
   nDrawRight = 640;
}


void Draw400l(void)
{

   void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
   WORD wdtop, wdbtm;
   SDL_Surface *p;
   if(agDriverOps == NULL) return;
   p = GetDrawSurface();
   /*
    * パレット設定
    */
//	SetVramReader_400l();
	if (bPaletFlag) {
		Palet640();
	}

        if (bClearFlag) {
	       AllClear();
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
   PutVramFunc = &PutVram_AG_SP;
   if(PutVramFunc == NULL) return;
   // if(vramhdr_400l == NULL) return;
   if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
      if(window_open) { // ハードウェアウインドウ開いてる
	 if (nDrawTop < window_dy1) {
	    //		 vramhdr_400l->SetVram(vram_dptr, 80, 400);
	    SetVram_200l(vram_dptr);
	    PutVramFunc(p, 0, nDrawTop, 640, window_dy1 - nDrawTop, multi_page);
	 }
	 /* ウィンドウ内の描画 */
	 if (nDrawTop > window_dy1) {
	    wdtop = nDrawTop;
	 } else {
	    wdtop = window_dy1;
	 }
	 
	 if (nDrawBottom < window_dy2) {
	    wdbtm = nDrawBottom;
	 } else {
	    wdbtm = window_dy2;
	 }
	 
	 if (wdbtm > wdtop) {
	    //	 vramhdr_400l->SetVram(vram_bdptr, 80, 400);
	    SetVram_200l(vram_bdptr);
	    PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
	 }
	 /* ハードウェアウインドウ外下部 */
	 if (nDrawBottom  > window_dy2) {
	    //	 vramhdr_400l->SetVram(vram_dptr, 80, 400);
	    SetVram_200l(vram_dptr);
	    PutVramFunc(p, 0 , wdbtm, 640, nDrawBottom - wdbtm, multi_page);
	 }
      } else { // ハードウェアウィンドウ開いてない
	 //           vramhdr_400l->SetVram(vram_dptr, 80, 400);
	 SetVram_200l(vram_dptr);
	 PutVramFunc(p, 0, 0, 640, 400, multi_page);
      }
   }
   if (bPaletFlag) {
      nDrawTop = 0;
      nDrawBottom = 400;
      nDrawLeft = 0;
      nDrawRight = 640;
      SetDrawFlag(TRUE);
   }
   
   bPaletFlag = FALSE;
   
}


void Draw320(void)
{
	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	SDL_Surface *p;
	WORD wdtop, wdbtm;

	if(agDriverOps == NULL) return;
	p = GetDrawSurface();
   /*
    * パレット設定
    */
    PutVramFunc = &PutVram_AG_SP;
   if(bPaletFlag) {
      Palet320();
      SDLDrawFlag.APaletteChanged = TRUE;
      //        SetVramReader_4096();
      nDrawTop = 0;
      nDrawBottom = 200;
      nDrawLeft = 0;
      nDrawRight = 320;
      SetDrawFlag(TRUE);
      bPaletFlag = FALSE;
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
   if(PutVramFunc == NULL) return;
   //if(vramhdr == NULL) return;
   if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
      if(window_open) { // ハードウェアウインドウ開いてる
	 if (nDrawTop < window_dy1) {
	    //			vramhdr_4096->SetVram(vram_dptr, 40, 200);
            SetVram_200l(vram_dptr);
	    PutVramFunc(p, 0, nDrawTop, 320, window_dy1, multi_page);
	 }
	 /* ウィンドウ内の描画 */
	 if (nDrawTop > window_dy1) {
	    wdtop = nDrawTop;
	 } else {
	    wdtop = window_dy1;
	 }
	 
	 if (nDrawBottom < window_dy2) {
	    wdbtm = nDrawBottom;
	 } else {
	    wdbtm = window_dy2;
	 }
	 
	 if (wdbtm > wdtop) {
	    //				vramhdr_4096->SetVram(vram_bdptr, 40, 200);
	    SetVram_200l(vram_bdptr);
	    PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
	 }
	 /* ハードウェアウインドウ外下部 */
	 if (nDrawBottom  > window_dy2) {
	    //				vramhdr_4096->SetVram(vram_dptr, 40, 200);
	    SetVram_200l(vram_dptr);
	    PutVramFunc(p, 0 , wdbtm, 320, nDrawBottom - wdbtm, multi_page);
	 }
      } else { // ハードウェアウィンドウ開いてない
	 //			vramhdr_4096->SetVram(vram_dptr, 40, 200);
	 SetVram_200l(vram_dptr);
	 PutVramFunc(p, 0, 0, 320, 200, multi_page);
      }
   }
   
   nDrawTop = 0;
   nDrawBottom = 200;
   nDrawLeft = 0;
   nDrawRight = 320;
   bPaletFlag = FALSE;
}

void Draw256k(void)
{
   SDL_Surface *p;
   void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
   if(agDriverOps == NULL) return;
   p = GetDrawSurface();

   PutVramFunc = &PutVram_AG_SP;
   nDrawTop = 0;
   nDrawBottom = 200;
   nDrawLeft = 0;
   nDrawRight = 320;
   //    SetDrawFlag(TRUE);
   // 
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
   /*
    * 26万色モードの時は、ハードウェアウィンドウを考慮しない。
    */
   PutVramFunc(p, 0, 0, 320, 200, multi_page);
   
   nDrawTop = 0;
   nDrawBottom = 200;
   nDrawLeft = 0;
   nDrawRight = 320;
   bPaletFlag = FALSE;
}

}


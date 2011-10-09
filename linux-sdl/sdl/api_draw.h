/*
 * api_draw.h
 *
 *  Created on: 2010/11/03
 *      Author: whatisthis
 */

#ifndef API_DRAW_H_
#define API_DRAW_H_

#ifdef USE_GTK
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#endif

#ifdef USE_AGAR
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#endif

#ifdef USE_SDL
#include <SDL.h>
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
#include "xm7.h"
#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif



#ifdef __cplusplus
#include "api_scaler.h"
#include "EmuGrphLib.h"
#include "EmuGrph400l.h"
#include "EmuGrph4096c.h"
#include "EmuGrph256kc.h"
extern SDL_semaphore   *DrawInitSem;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct DrawPieces {
    BOOL read[80][50];
    BOOL write[80][50];
    BOOL Drawn;
    BOOL DPaletteChanged;
    BOOL APaletteChanged;
    BOOL ForcaReDraw;
};
/* アスペクト */
enum {
	nAspect43 = 0, /* 4:3 */
	nAspect11, /* 1:1 */
	nAspectFree
};

/*
 * グローバル変数
 */
extern int  RootVideoWidth; /* ルートウィンドウ最高幅 */
extern int  RootVideoHeight; /* ルートウィンドウ最高高さ */

extern DWORD   rgbTTLGDI[16];	/* デジタルパレット */
extern DWORD   rgbAnalogGDI[4096];	/* アナログパレット */
extern struct DrawPieces SDLDrawFlag; /* 書き替えフラグ */

// guchar pBitsGDI[400*640*3]; /* ビットデータ */
//extern BYTE            GDIDrawFlag[80 * 50];	/* 8x8ドットのメッシュを作る *//* 8x8 再描画領域フラグ */
extern BOOL            bFullScan;		/* フルスキャン(Window) */
extern BOOL            bDirectDraw;		/* 直接書き込みフラグ */
extern SDL_Surface     *realDrawArea;	/* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する) */
extern WORD            nDrawTop;			/* 描画範囲上 */
extern WORD            nDrawBottom;		/* 描画範囲下 */
extern WORD            nDrawLeft;		/* 描画範囲左 */
extern WORD            nDrawRight;		/* 描画範囲右 */
extern WORD            nDrawWidth;
extern WORD            nDrawHeight;
extern BOOL            bPaletFlag;		/* パレット変更フラグ */
extern BOOL            bClearFlag;
extern int             nOldVideoMode;
extern WORD			nDrawFPS;   /* FPS値 20100913 */
extern WORD        nEmuFPS; /* エミュレーションFPS 20110123 */
extern WORD        nAspect; /* ASPECT比 20110123 */
extern BOOL bUseOpenGL; /* OPENGLを描画に使う */
extern BOOL bSyncToVSYNC; /* VSYNC同期(OpenGLのみ) */
extern 	BOOL bSmoosing; /* スムージング処理する(GLのみ?) */

extern int newDrawWidth;
extern int newDrawHeight;
extern BOOL newResize;
extern BYTE bMode;

/*
 * 初期化・終了
 */

extern void InitDraw(void);
extern void CleanDraw(void);


extern void VramReader(Uint32 addr, Uint32 *cbuf, Uint32 mpage);
extern void VramReader_400l(Uint32 addr, Uint32 *cbuf, Uint32 mpage);
extern void VramReader_4096(Uint32 addr, Uint32 *cbuf, Uint32 mpage);
extern void VramReader_256k(Uint32 addr, Uint32 *cbuf, Uint32 mpage);

extern void ResizeWindow(int w, int h);
extern BOOL SelectDraw(void);
extern void AllClear(void);
extern void RenderFullScan(void);
extern void RenderSetOddLine(void);

extern void OnDraw(void);
#ifdef USE_GTK
extern gint OnPaint(GtkWidget * widget, GdkEventExpose * event);
#else
extern int OnPaint(void);
#endif

extern BOOL Select640(void);
extern BOOL Select400l(void);
extern BOOL Select320(void);
extern BOOL Select256k(void);
extern void ChangeResolution(void);
/*
 * XM7 NOTIFY APIs
 */

//extern void vram_notify(WORD addr, BYTE dat);
//extern void	ttlpalet_notify(void);
//extern void 	apalet_notify(void);
//extern void 	display_notify(void);
#if XM7_VER >= 3
//extern void window_notify(void);
#endif
extern void OnFullScreen(void);
extern void OnWindowedScreen(void);
extern void SetDrawFlag(BOOL flag);

/*
 * VRAMAPI
 */


/*
 * 描画・パレット
 */
extern void Draw640All(void);
extern void Draw320(void);
extern void Draw400l(void);
extern void Draw256k(void);

extern void Palet640(void);
extern void Palet320(void);
extern BOOL BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight,
		int nSrcLeft, int nSrcTop);
extern BOOL SelectDraw2(void);

/*
 * ウインドウリサイズ
 */
extern void ResizeGL(int w, int h);

#ifdef __cplusplus
}
#endif
#ifdef USE_AGAR
extern void ResizeWindow_Agar(int w, int h);
extern void ResizeWindow_Agar2(int w, int h);
extern void AGDrawTaskEvent(BOOL Flag);
extern void AGDrawTaskMain(void);
extern void *DrawThreadMain(void *p);
extern void AG_DrawInitsub(void);
extern void AG_DrawDetachsub(void);
#else
extern void SDL_DrawInitsub(void);
extern void SDL_DrawDetachsub(void);
extern int DrawThreadMain(void *p);
#endif


#endif /* API_DRAW_H_ */

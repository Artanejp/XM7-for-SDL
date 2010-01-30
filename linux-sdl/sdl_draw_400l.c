/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *      Copyright (C) 2001-2003 Ryu Takegami
 *      Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *      [ SDL 表示 / 640ドット(400line)パート]
 *               20100125 sdl_draw.cを分割
 */

#ifdef _XWIN

#include <SDL/SDL.h>
#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"
#include "sdl.h"
#include "sdl_draw.h"

/*
 *      グローバル ワーク(のextern設定)
 */
extern DWORD    rgbTTLGDI[16];	/* デジタルパレット */
extern DWORD    rgbAnalogGDI[4096];	/* アナログパレット */
// guchar pBitsGDI[400*640*3]; /* ビットデータ */
extern BYTE     GDIDrawFlag[4000];	/* 8x8 再描画領域フラグ */
extern BOOL     bFullScan;	/* フルスキャン(Window) */
extern BOOL     bDirectDraw;	/* 直接書き込みフラグ */
extern SDL_Surface *realDrawArea;	/* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する) 
					 */
extern WORD     nDrawTop;	/* 描画範囲上 */
extern WORD     nDrawBottom;	/* 描画範囲下 */
extern WORD     nDrawLeft;	/* 描画範囲左 */
extern WORD     nDrawRight;	/* 描画範囲右 */
extern BOOL     bPaletFlag;	/* パレット変更フラグ */
extern BOOL     bClearFlag;	/* クリアフラグ */


/*
 * 共通ルーチンのextern設定
 */
extern BOOL     BitBlt(int nDestLeft, int nDestTop, int nWidth,
		       int nHeight, int nSrcLeft, int nSrcTop);
extern void     RenderFullScan(void);
extern void     RenderSetOddLine(void);
extern void     SetDrawFlag(BOOL flag);
extern void     AllClear(void);
/*
 * SETDOT（inline）
 * 24bpp前提,SurfaceLockしません!!
 */
static inline void
__SETDOT(WORD x, WORD y, DWORD c)
{

    // Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch 
    // 
    // + x * displayArea->format->BytesPerPixel;
    Uint8          *addr =
	(Uint8 *) realDrawArea->pixels + y * realDrawArea->pitch +
	x * realDrawArea->format->BytesPerPixel;
    DWORD          *addr32 = (DWORD *) addr;

    /*
     * 少しでも速度を稼ぐために[ABRG]に１バイトづつ書くのではなく、
     * Uint32(DOWRD)で一気に書き込む
     */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    *addr32 = (c >> 8) | 0xff000000;
#else
    *addr32 = (c << 8) | 0x000000ff;

#endif

}

static inline void
__SETDOT_DDRAW(WORD x, WORD y, DWORD c)
{

    Uint8          *addr =
	(Uint8 *) realDrawArea->pixels + y * realDrawArea->pitch +
	x * realDrawArea->format->BytesPerPixel;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[3] = 0xff;		/* A */
    addr[2] = c & 0xff;
    addr[1] = (c >> 8) & 0xff;
    addr[0] = (c >> 16) & 0xff;
#else
    addr[0] = 0xff;		/* A */
    addr[1] = c & 0xff;		/* B */
    addr[2] = (c >> 8) & 0xff;	/* R */
    addr[3] = (c >> 16) & 0xff;	/* G */
#endif
}

#if XM7_VER >= 3
/*
 *      640x400、デジタルモード
 *      ウィンドウ外描画サブ
 */
static void
Draw400lSub(int top, int bottom)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    BYTE           *vramptr;
    BYTE            col[2];
    DWORD           r,
                    g,
                    b;
    BYTE            c0,
                    c1,
                    c2,
                    c3,
                    c4,
                    c5,
                    c6,
                    c7;
    BYTE            cb,
                    cr,
                    cg;


    SDL_LockSurface(realDrawArea);
    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {

	/*
	 * xループ 
	 */
	for (x = nDrawLeft >> 3; x < nDrawRight >> 3; x++) {
	    bit = 0x80;
	    // if(GDIDrawFlag[(y >>3) * 80 + x] == 0) continue; /*
	    // 再描画フラグを真面目に見てみることにした 
	    // 
	    // */
	    vramptr = vram_dptr;
	    /*
	     * オフセット設定 
	     */
	    offset = 80 * y + x;
	    cb = vramptr[offset + 0x00000];
	    cr = vramptr[offset + 0x08000];
	    cg = vramptr[offset + 0x10000];

	    c0 = (cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2);
	    c1 = ((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1);
	    c2 = ((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04);
	    c3 = ((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +
		((cg & 0x08) >> 1);
	    c4 = ((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) +
		((cg & 0x10) >> 2);
	    c5 = ((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) +
		((cg & 0x20) >> 3);
	    c6 = ((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) +
		((cg & 0x40) >> 4);
	    c7 = ((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) +
		((cg & 0x80) >> 5);
	    if (bDirectDraw) {
		__SETDOT_DDRAW((x << 3) + 0, y, rgbTTLGDI[c7]);
		__SETDOT_DDRAW((x << 3) + 1, y, rgbTTLGDI[c6]);
		__SETDOT_DDRAW((x << 3) + 2, y, rgbTTLGDI[c5]);
		__SETDOT_DDRAW((x << 3) + 3, y, rgbTTLGDI[c4]);
		__SETDOT_DDRAW((x << 3) + 4, y, rgbTTLGDI[c3]);
		__SETDOT_DDRAW((x << 3) + 5, y, rgbTTLGDI[c2]);
		__SETDOT_DDRAW((x << 3) + 6, y, rgbTTLGDI[c1]);
		__SETDOT_DDRAW((x << 3) + 7, y, rgbTTLGDI[c0]);
	    } else {
		__SETDOT((x << 3) + 0, y, rgbTTLGDI[c7]);
		__SETDOT((x << 3) + 1, y, rgbTTLGDI[c6]);
		__SETDOT((x << 3) + 2, y, rgbTTLGDI[c5]);
		__SETDOT((x << 3) + 3, y, rgbTTLGDI[c4]);
		__SETDOT((x << 3) + 4, y, rgbTTLGDI[c3]);
		__SETDOT((x << 3) + 5, y, rgbTTLGDI[c2]);
		__SETDOT((x << 3) + 6, y, rgbTTLGDI[c1]);
		__SETDOT((x << 3) + 7, y, rgbTTLGDI[c0]);
	    }
	}
    }
    // SDL_UnlockSurface(displayArea);
    SDL_UnlockSurface(realDrawArea);

}

/*
 *      640x400、デジタルモード
 *      ウィンドウ内描画サブ
 */
static void
Draw400lWSub(int top, int bottom, int left, int right)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    BYTE           *vramptr;
    BYTE            col[2];
    DWORD           r,
                    g,
                    b;
    BYTE            c0,
                    c1,
                    c2,
                    c3,
                    c4,
                    c5,
                    c6,
                    c7;
    BYTE            cb,
                    cr,
                    cg;
    // SDL_LockSurface(displayArea);
    SDL_LockSurface(realDrawArea);
    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {

	/*
	 * xループ 
	 */
	for (x = left; x < right; x++) {
	    // if(GDIDrawFlag[(y >>3) * 80 + x] == 0) continue; /*
	    // 再描画フラグを真面目に見てみることにした 
	    // 
	    // */
	    bit = 0x80;
	    vramptr = vram_bdptr;

	    /*
	     * オフセット設定 
	     */
	    offset = 80 * y + x;
	    cb = vramptr[offset + 0x00000];
	    cr = vramptr[offset + 0x08000];
	    cg = vramptr[offset + 0x10000];

	    c0 = (cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2);
	    c1 = ((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1);
	    c2 = ((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04);
	    c3 = ((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +
		((cg & 0x08) >> 1);
	    c4 = ((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) +
		((cg & 0x10) >> 2);
	    c5 = ((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) +
		((cg & 0x20) >> 3);
	    c6 = ((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) +
		((cg & 0x40) >> 4);
	    c7 = ((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) +
		((cg & 0x80) >> 5);
	    if (bDirectDraw) {
		__SETDOT_DDRAW((x << 3) + 0, y, rgbTTLGDI[c7]);
		__SETDOT_DDRAW((x << 3) + 1, y, rgbTTLGDI[c6]);
		__SETDOT_DDRAW((x << 3) + 2, y, rgbTTLGDI[c5]);
		__SETDOT_DDRAW((x << 3) + 3, y, rgbTTLGDI[c4]);
		__SETDOT_DDRAW((x << 3) + 4, y, rgbTTLGDI[c3]);
		__SETDOT_DDRAW((x << 3) + 5, y, rgbTTLGDI[c2]);
		__SETDOT_DDRAW((x << 3) + 6, y, rgbTTLGDI[c1]);
		__SETDOT_DDRAW((x << 3) + 7, y, rgbTTLGDI[c0]);
	    } else {
		__SETDOT((x << 3) + 0, y, rgbTTLGDI[c7]);
		__SETDOT((x << 3) + 1, y, rgbTTLGDI[c6]);
		__SETDOT((x << 3) + 2, y, rgbTTLGDI[c5]);
		__SETDOT((x << 3) + 3, y, rgbTTLGDI[c4]);
		__SETDOT((x << 3) + 4, y, rgbTTLGDI[c3]);
		__SETDOT((x << 3) + 5, y, rgbTTLGDI[c2]);
		__SETDOT((x << 3) + 6, y, rgbTTLGDI[c1]);
		__SETDOT((x << 3) + 7, y, rgbTTLGDI[c0]);
	    }

	}
    }
    // SDL_UnlockSurface(displayArea);
    SDL_UnlockSurface(realDrawArea);

}

/*
 *      640x400、デジタルモード
 *      描画
 */
void
Draw400l(void)
{
    WORD            wdtop,
                    wdbtm;

    /*
     * パレット設定 
     */
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
    if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
	/*
	 * ウィンドウオープン時 
	 */
	if (window_open) {
	    /*
	     * ウィンドウ外 上側の描画 
	     */
	    if (nDrawTop < window_dy1) {
		Draw400lSub(nDrawTop, window_dy1);
	    }

	    /*
	     * ウィンドウ内の描画 
	     */
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
		Draw400lWSub(wdtop, wdbtm, window_dx1, window_dx2);
	    }

	    /*
	     * ウィンドウ外 下側の描画 
	     */
	    if (nDrawBottom > window_dy2) {
		Draw400lSub(window_dy2, nDrawBottom);
	    }
	} else {
	    Draw400lSub(nDrawTop, nDrawBottom);
	}
    }

    BitBlt(nDrawLeft, nDrawTop,
	   (nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
	   nDrawLeft, nDrawTop);

    nDrawTop = 400;
    nDrawBottom = 0;
    nDrawLeft = 640;
    nDrawRight = 0;
    bPaletFlag = FALSE;
    SetDrawFlag(FALSE);
}

#endif				/* XM7_VER >= 3 */


#endif				/* _X11 */

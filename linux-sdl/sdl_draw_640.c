/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *      Copyright (C) 2001-2003 Ryu Takegami
 *      Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *      [ SDL 表示 / 640ドットパート]
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

#include "sdl_drawcommon.h"

/*
 *      640x200、デジタルモード
 *      ウィンドウ外描画サブ
 */

static void
Draw640Sub2_DDRAW640p(int top, int bottom)
{
    int             x,
                    y;
    int             i;
    BYTE            bit;
    Uint8          *addr;
    DWORD           c[8];

    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {
	/*
	 * xループ     
	 */
	for (x = nDrawLeft >> 3; x < nDrawRight >> 3; x++) {
	    addr = (Uint8 *) realDrawArea->pixels +
		(y << 1) * realDrawArea->pitch +
		(x << 3) * realDrawArea->format->BytesPerPixel;
	    __GETVRAM_3bpp(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_640p(addr, realDrawArea->format->BytesPerPixel,
				 realDrawArea->pitch, c);
	}

    }
}

static void
Draw640Sub2_DDRAW640i(int top, int bottom)
{
    int             x,
                    y;
    int             i;
    BYTE            bit;
    Uint8          *addr;

    DWORD           c[8];

    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {
	/*
	 * xループ 
	 */
	for (x = nDrawLeft >> 3; x < nDrawRight >> 3; x++) {
	    addr = (Uint8 *) realDrawArea->pixels +
		(y << 1) * realDrawArea->pitch +
		(x << 3) * realDrawArea->format->BytesPerPixel;
	    __GETVRAM_3bpp(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_640i(addr, realDrawArea->format->BytesPerPixel,
				 realDrawArea->pitch, c);
	}

    }
}


static void
Draw640Sub2_DDRAW1280p(int top, int bottom)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    Uint8          *addr;
    DWORD           c[8];

    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {
	/*
	 * xループ 
	 */
	for (x = nDrawLeft >> 3; x < nDrawRight >> 3; x++) {
	    addr = (Uint8 *) realDrawArea->pixels +
		(y << 2) * realDrawArea->pitch +
		(x << 4) * realDrawArea->format->BytesPerPixel;

	    __GETVRAM_3bpp(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640p(addr,
				  realDrawArea->format->BytesPerPixel,
				  realDrawArea->pitch, c);
	}
    }
}

static void
Draw640Sub2_DDRAW1280i(int top, int bottom)
{
    int             x,
                    y;
    int             i;
    BYTE            bit;
    Uint8          *addr;
    DWORD           c[8];
    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {
	/*
	 * xループ 
	 */
	for (x = nDrawLeft >> 3; x < nDrawRight >> 3; x++) {
	    addr = (Uint8 *) realDrawArea->pixels +
		(y << 2) * realDrawArea->pitch +
		(x << 4) * realDrawArea->format->BytesPerPixel;
	    __GETVRAM_3bpp(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640i(addr,
				  realDrawArea->format->BytesPerPixel,
				  realDrawArea->pitch, c);
	}

    }
}

static void
Draw640Sub(int top, int bottom)
{
    int             x,
                    y;
    int             i;
    int             offset;
    BYTE            bit;
    Uint8          *addr;
    DWORD           c[8];

    SDL_LockSurface(realDrawArea);
    switch (nDrawWidth) {
    case 1280:
	if (bFullScan) {
	    Draw640Sub2_DDRAW1280p(top, bottom);
	} else {
	    Draw640Sub2_DDRAW1280i(top, bottom);
	}
	break;
    default:
	if (bFullScan) {
	    Draw640Sub2_DDRAW640p(top, bottom);
	} else {
	    Draw640Sub2_DDRAW640i(top, bottom);
	}
	break;
    }

    SDL_UnlockSurface(realDrawArea);
}


static void
Draw640WSub2_DDRAW640p(int top, int bottom, int left, int right)
{
    Uint8          *pos,
                   *posbase;
    int             x,
                    y;
    int             pitch,
                    bpp;
    DWORD           c[8];


    pitch = realDrawArea->pitch;
    bpp = realDrawArea->format->BytesPerPixel;
    posbase =
	(Uint8 *) realDrawArea->pixels + top * pitch * 2 + left * bpp;

    for (y = top; y < bottom; y++) {
	pos = posbase;
	/*
	 * xループ 
	 */
	for (x = left; x < right; x++) {
	    __GETVRAM_3bpp(vram_bdptr, x, y, c);
	    __SETBYTE_DDRAW_640p(pos, bpp, pitch, c);
	    pos += bpp * 8;
	}
	posbase += (pitch * 2);
    }
}

static void
Draw640WSub2_DDRAW640i(int top, int bottom, int left, int right)
{
    Uint8          *pos,
                   *posbase;
    int             x,
                    y;
    int             pitch,
                    bpp;
    DWORD           c[8];

    pitch = realDrawArea->pitch;
    bpp = realDrawArea->format->BytesPerPixel;
    posbase =
	(Uint8 *) realDrawArea->pixels + top * pitch * 2 + left * bpp;

    for (y = top; y < bottom; y++) {
	pos = posbase;
	/*
	 * xループ 
	 */
	for (x = left; x < right; x++) {
	    __GETVRAM_3bpp(vram_bdptr, x, y, c);
	    __SETBYTE_DDRAW_640i(pos, bpp, pitch, c);
	    pos += bpp * 8;
	}
	posbase += pitch * 2;
    }
}

static void
Draw640WSub2_DDRAW1280p(int top, int bottom, int left, int right)
{
    Uint8          *pos,
                   *posbase;
    int             x,
                    y;
    int             pitch,
                    bpp;
    DWORD           c[8];


    pitch = realDrawArea->pitch;
    bpp = realDrawArea->format->BytesPerPixel;
    posbase =
	(Uint8 *) realDrawArea->pixels + top * pitch * 4 + left * bpp * 2;

    for (y = top; y < bottom; y++) {
	pos = posbase;
	/*
	 * xループ 
	 */
	for (x = left; x < right; x++) {
	    __GETVRAM_3bpp(vram_bdptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640p(pos, bpp, pitch, c);
	    pos += bpp * 16;
	}
	posbase += pitch * 4;
    }
}

static void
Draw640WSub2_DDRAW1280i(int top, int bottom, int left, int right)
{
    Uint8          *pos,
                   *posbase;
    int             x,
                    y;
    int             pitch,
                    bpp;
    DWORD           c[8];


    pitch = realDrawArea->pitch;
    bpp = realDrawArea->format->BytesPerPixel;
    posbase =
	(Uint8 *) realDrawArea->pixels + top * pitch * 4 + left * bpp * 2;

    for (y = top; y < bottom; y++) {
	pos = posbase;
	/*
	 * xループ 
	 */
	for (x = left; x < right; x++) {
	    __GETVRAM_3bpp(vram_bdptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640i(pos, bpp, pitch, c);
	    pos += bpp * 16;
	}
	posbase += pitch * 4;
    }
}

#if XM7_VER >= 3



/*
 *      640x200、デジタルモード
 *      ウィンドウ内描画サブ
 */
static void
Draw640WSub(int top, int bottom, int left, int right)
{


    SDL_LockSurface(realDrawArea);

    switch (nDrawWidth) {
    case 1280:
	if (bFullScan) {
	    Draw640WSub2_DDRAW1280p(top, bottom, left, right);
	} else {
	    Draw640WSub2_DDRAW1280i(top, bottom, left, right);
	}
	break;
    default:
	if (bFullScan) {
	    Draw640WSub2_DDRAW640p(top, bottom, left, right);
	} else {
	    Draw640WSub2_DDRAW640i(top, bottom, left, right);
	}
	break;
    }
    SDL_UnlockSurface(realDrawArea);
}
#endif
/*
 *      640x200/400、デジタルモード
 *      パレット設定
 */
void
Palet640(void)
{
    int             i;
    int             vpage;


    /*
     * パレットテーブル 
     */
    static DWORD    rgbTable[] = {
	0x00000000,
	0x000000ff,
	0x00ff0000,
	0x00ff00ff,
	0x0000ff00,
	0x0000ffff,
	0x00ffff00,
	0x00ffffff
    };

    /*
     * マルチページより、表示プレーン情報を得る 
     */
    vpage = (~(multi_page >> 4)) & 0x07;

    /*
     * 640x200/400、デジタルパレット 
     */
    for (i = 0; i < 8; i++) {
	if (crt_flag) {
	    /*
	     * CRT ON 
	     */
	    rgbTTLGDI[i] = rgbTable[ttl_palet[i & vpage] & 0x07];
	} else {
	    /*
	     * CRT OFF 
	     */
	    rgbTTLGDI[i] = rgbTable[0];
	}
    }

    /*
     * 奇数ライン用 
     */
    rgbTTLGDI[8] = rgbTable[0];
    rgbTTLGDI[9] = rgbTable[4];
}

/*
 *      640x200、デジタルモード
 *      描画
 */
void
Draw640(void)
{
#if XM7_VER >= 3
    WORD            wdtop,
                    wdbtm;
#endif
    realDrawArea = SDL_GetVideoSurface();
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
#if XM7_VER >= 3
	/*
	 * ウィンドウオープン時 
	 */
	if (window_open) {
	    /*
	     * ウィンドウ外 上側の描画 
	     */
	    if ((nDrawTop >> 1) < window_dy1) {
		Draw640Sub(nDrawTop >> 1, window_dy1);
	    }

	    /*
	     * ウィンドウ内の描画 
	     */
	    if ((nDrawTop >> 1) > window_dy1) {
		wdtop = (WORD) (nDrawTop >> 1);
	    } else {
		wdtop = window_dy1;
	    }

	    if ((nDrawBottom >> 1) < window_dy2) {
		wdbtm = (WORD) (nDrawBottom >> 1);
	    } else {
		wdbtm = window_dy2;
	    }

	    if (wdbtm > wdtop) {
		Draw640WSub(wdtop, wdbtm, window_dx1, window_dx2);
	    }

	    /*
	     * ウィンドウ外 下側の描画 
	     */
	    if ((nDrawBottom >> 1) > window_dy2) {
		Draw640Sub(window_dy2, nDrawBottom >> 1);
	    }
	} else {
	    Draw640Sub(nDrawTop >> 1, nDrawBottom >> 1);
	}
#else
	Draw640Sub(nDrawTop >> 1, nDrawBottom >> 1);
#endif
	// if(!bFullScan){
	// RenderSetOddLine();
	// }
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


#endif				/* _XWIN */

/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *      Copyright (C) 2001-2003 Ryu Takegami
 *      Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *      [ SDL 表示 / 4096色パート]
 *               20100125 sdl_draw.cを分割
 */

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
 *      320x200、アナログモード
 *      パレット設定
 */
void
Palet320()
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
                        r = apalet_r[j];
                        g = apalet_g[j];
                        b = apalet_b[j];
                } else {
                        r = 0;
                        g = 0;
                        b = 0;
	}
                switch(realDrawArea->format->BitsPerPixel) {
                case 24:
                case 32:
                        /*
                         * R 
                         */
                        r <<= 4;
                        if (r > 0) {
                        r |= 0x0f;
                        }
                        color |= r;
                        color <<= 8;

                        /*
                         * G 
                         */
                        g <<= 4;
                        if (g > 0) {
                                g |= 0x0f;
                        }
                        color |= g;
                        color <<= 8;

                        /*
                         * B 
                         */
                        b <<= 4;
                        if (b > 0) {
                                b |= 0x0f;
                        }
                        color |= b;
                        break;
                case 16:
                case 15:
                default:
                        /*
                         * R 
                         */
                        r &= 0x0f;
                        color |= r;
                        color <<= 4;

                        /*
                         * G 
                         */
                        g &= 0x0f;
                        color |= g;
                        color <<= 4;

                        /*
                         * B 
                         */
                        b &= 0x0f;
                        color |= b;
                        break;
                }

                /*
                 * セット 
                 */
                rgbAnalogGDI[i] = color;
        }
}




/*
 *      320x200、アナログモード
 *      ウィンドウ外描画用サブ
 */
static void
Draw320Sub(int top, int bottom)
{
    int             x,
                    y;
    DWORD           c[8];
    Uint8          *addr;

    SDL_LockSurface(realDrawArea);
    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {
            /*
             * xループ 
             */
            for (x = nDrawLeft >> 4; x < nDrawRight >> 4; x++) {
                    __GETVRAM_12bpp(vram_dptr, x, y, c);
                    
                    switch (nDrawWidth) {
                    case 1280:
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 2) * realDrawArea->pitch +
                                    (x << 5) * realDrawArea->format->BytesPerPixel;
                            switch(realDrawArea->format->BitsPerPixel) {
                            case 32:
                            case 24:
                                    if(bFullScan) {
                                    __SETBYTE_DDRAW_1280_320p(addr,
                                                   realDrawArea->
                                                   format->BytesPerPixel,
                                                   realDrawArea->pitch, c);
                                    } else {
                                    __SETBYTE_DDRAW_1280_320i(addr,
                                                              realDrawArea->
                                                              format->BytesPerPixel,
                                                              realDrawArea->pitch, c);
                                    }
                                    break;
                            case 16:
                            case 15:
                            default:
                                    if(bFullScan) {
                                    __SETBYTE_DDRAW_1280_320p_16(addr,
                                                   realDrawArea->
                                                   format->BytesPerPixel,
                                                   realDrawArea->pitch, c);
                                    } else {
                                    __SETBYTE_DDRAW_1280_320i_16(addr,
                                                              realDrawArea->
                                                              format->BytesPerPixel,
                                                              realDrawArea->pitch, c);
                                    }
                                    break;
                            }
                            break;
                    case 640:
                    default:
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 4) * realDrawArea->format->BytesPerPixel;
                            switch(realDrawArea->format->BitsPerPixel) {
                            case 24:
                            case 32:
                            if (bFullScan) {
                                    __SETBYTE_DDRAW_320p(addr,
                                                         realDrawArea->format->
                                                         BytesPerPixel,
                                                         realDrawArea->pitch, c);
                            } else {
                                    __SETBYTE_DDRAW_320i(addr,
                                                         realDrawArea->format->
                                                         BytesPerPixel,
                                                         realDrawArea->pitch, c);
                            }       
                            break;
                            case 16:
                            case 15:
                            default:
                            if (bFullScan) {
                                    __SETBYTE_DDRAW_320p_16(addr,
                                                         realDrawArea->format->
                                                         BytesPerPixel,
                                                         realDrawArea->pitch, c);
                            } else {
                                    __SETBYTE_DDRAW_320i_16(addr,
                                                         realDrawArea->format->
                                                         BytesPerPixel,
                                                         realDrawArea->pitch, c);
                            }       
                            break;
                            }
                    break;
                    }
            }
    }
    SDL_UnlockSurface(realDrawArea);
}

#if XM7_VER >= 3
/*
 *      320x200、アナログモード
 *      ウィンドウ内描画用サブ
 */
static void
Draw320WSub(int top, int bottom, int left, int right)

{
    int             x,
                    y;
    DWORD           c[8];
    BYTE            bit;
    BYTE            b[4],
                    r[4],
                    g[4];
    Uint8          *addr;

    SDL_LockSurface(realDrawArea);
    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {

	/*
	 * xループ 
	 */
	for (x = left; x < right; x++) {
	    __GETVRAM_12bpp(vram_bdptr, x, y, c);
	    switch (nDrawWidth) {
	    case 1280:
		addr = (Uint8 *) realDrawArea->pixels +
		    (y << 2) * realDrawArea->pitch +
		    (x << 5) * realDrawArea->format->BytesPerPixel;
		if (bFullScan) {
		    __SETBYTE_DDRAW_1280_320p(addr,
					      realDrawArea->
					      format->BytesPerPixel,
					      realDrawArea->pitch, c);
		} else {
		    __SETBYTE_DDRAW_1280_320i(addr,
					      realDrawArea->
					      format->BytesPerPixel,
					      realDrawArea->pitch, c);
		}
		break;
	    case 640:
	    default:
		addr = (Uint8 *) realDrawArea->pixels +
		    (y << 1) * realDrawArea->pitch +
		    (x << 4) * realDrawArea->format->BytesPerPixel;
		if (bFullScan) {
		    __SETBYTE_DDRAW_320p(addr,
					 realDrawArea->format->
					 BytesPerPixel,
					 realDrawArea->pitch, c);
		} else {
		    __SETBYTE_DDRAW_320i(addr,
					 realDrawArea->format->
					 BytesPerPixel,
					 realDrawArea->pitch, c);
		}
		break;
	    }
	}
    }
    SDL_UnlockSurface(realDrawArea);
}

/*
 *      320x200、アナログモード
 *      描画
 */
void
Draw320(void)
{
#if XM7_VER >= 3
    WORD            wdtop,
                    wdbtm;
#endif

    /*
     * パレット設定 
     */
    if (bPaletFlag) {
	Palet320();
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
    if (nDrawTop >= nDrawBottom) {
	return;
    }
#if XM7_VER >= 3
    /*
     * ウィンドウオープン時 
     */
    if (window_open) {
	/*
	 * ウィンドウ外 上側の描画 
	 */
	if ((nDrawTop >> 1) < window_dy1) {
	    Draw320Sub(nDrawTop >> 1, window_dy1);
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
	    Draw320WSub(wdtop, wdbtm, window_dx1, window_dx2);
	}

	/*
	 * ウィンドウ外 下側の描画 
	 */
	if ((nDrawBottom >> 1) > window_dy2) {
	    Draw320Sub(window_dy2, nDrawBottom >> 1);
	}
    } else {
	Draw320Sub(nDrawTop >> 1, nDrawBottom >> 1);
    }
#else
    Draw320Sub(nDrawTop >> 1, nDrawBottom >> 1);
#endif

    // if (!bFullScan) {
    // RenderSetOddLine();
    // }

    BitBlt(nDrawLeft, nDrawTop,
	   (nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
	   nDrawLeft, nDrawTop);

    /*
     * 次回に備え、ワークリセット 
     */
    nDrawTop = 400;
    nDrawBottom = 0;
    nDrawLeft = 640;
    nDrawRight = 0;
    bPaletFlag = FALSE;
    SetDrawFlag(FALSE);
}

#endif				/* XM7_VER >=3 */



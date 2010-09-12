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
            if(y >= realDrawArea->h) break;            

            /*
             * xループ 
             */
            for (x = 0 >> 4; x < 640 >> 4; x++) {
                    if(x << 4  >= realDrawArea->w) break;            

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

/*
 * 320x200, アナログモード
 * タイマイベント描画
 */
void
Draw320All(void)
{
#if XM7_VER >= 3
#endif
    realDrawArea = SDL_GetVideoSurface();
    if(realDrawArea == NULL) return;
    /*
     * パレット設定 
     */
    /*
     *描画モードを変えたら強制的にPalet320すること。
     */
    Palet320();
    nDrawTop = 0;
    nDrawBottom = 400;
    nDrawLeft = 0;
    nDrawRight = 640;
    SetDrawFlag(TRUE);
    
    /*
     * クリア処理 
     */
    if (bClearFlag) {
	AllClear();
    }

    /*
     * レンダリング 
     */
    switch(nDrawWidth) {
    case 1280:
            Draw320Sub(0,400>>1);
            break;
    case 640:
    default:
            Draw320Sub(0,400>>1);
            break;
    }
	if(!bFullScan){
            RenderSetOddLine();
    } else {
            RenderFullScan();
    }


    //BitBlt(nDrawLeft, nDrawTop,
//	   (nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
//	   nDrawLeft, nDrawTop);

    nDrawTop = 400;
    nDrawBottom = 0;
    nDrawLeft = 0;
    nDrawRight = 640;
    bPaletFlag = FALSE;
    SetDrawFlag(FALSE);
}






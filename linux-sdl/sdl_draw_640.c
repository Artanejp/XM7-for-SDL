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
    Uint8          *addr;
    DWORD           c[8];

    /*
     * yループ 
     */
    switch(realDrawArea->format->BitsPerPixel) {
    case 24:
            for (y = top; y < bottom; y++) {
      //              if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;            
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640p(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 32:
            for (y = top; y < bottom; y++) {
    //                if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 ; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640p(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 16:
            for (y = top; y < bottom; y++) {
  //                  if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 ; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640p(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 15:
            for (y = top; y < bottom; y++) {
//                    if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x =  0 >> 3; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640p(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 8:
            for (y = top; y < bottom; y++) {
              //      if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 >> 3; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640p_8(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    default:
            for (y = top; y < bottom; y++) {
                //    if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 >> 3; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640p(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    }

}


static void
Draw640Sub2_DDRAW640i(int top, int bottom)
{
    int             x,
                    y;
    Uint8          *addr;

    DWORD           c[8];
    switch(realDrawArea->format->BitsPerPixel) {
    case 24:
            for (y = top; y < bottom; y++) {
            //        if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;            
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640i(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 32:
            for (y = top; y < bottom; y++) {
          //          if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 ; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640i(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 16:
            for (y = top; y < bottom; y++) {
        //            if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 ; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640i(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 15:
            for (y = top; y < bottom; y++) {
      //              if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x =  0 >> 3; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640i(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    case 8:
            for (y = top; y < bottom; y++) {
    //                if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 >> 3; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640i_8(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    default:
            for (y = top; y < bottom; y++) {
  //                  if( y >= realDrawArea->h) return;
                    /*
                     * xループ     
                     */
                    for (x = 0 >> 3; x < 640 >> 3; x++) {
                            if(x << 3  >= realDrawArea->w) break;
                            addr = (Uint8 *) realDrawArea->pixels +
                                    (y << 1) * realDrawArea->pitch +
                                    (x << 3) * realDrawArea->format->BytesPerPixel;
                            __GETVRAM_3bpp(vram_dptr, x, y, c);
                            __SETBYTE_DDRAW_640i(addr, realDrawArea->format->BytesPerPixel,
                                                 realDrawArea->pitch, c);
                    }
            }
            break;
    }
}


static void
Draw640Sub2_DDRAW1280p(int top, int bottom)
{
    int             x,
                    y;
    Uint8          *addr;
    DWORD           c[8];

    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {
            //if( y >= realDrawArea->h) return;
	/*
	 * xループ 
	 */
	for (x = 0 >> 3; x < 640 >> 3; x++) {
            if(x << 3  >= realDrawArea->w) break;
	    addr = (Uint8 *) realDrawArea->pixels +
		(y << 2) * realDrawArea->pitch +
		(x << 4) * realDrawArea->format->BytesPerPixel;
        switch(realDrawArea->format->BitsPerPixel) {
        case 24:
        case 32:
	    __GETVRAM_3bpp(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640p(addr,
				  realDrawArea->format->BytesPerPixel,
				  realDrawArea->pitch, c);
        break;
        case 8:
        default:
	    __GETVRAM_3bpp_HW(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640p_8(addr,
				  realDrawArea->format->BytesPerPixel,
				  realDrawArea->pitch, c);
        break;
        }
	}
    }
}

static void
Draw640Sub2_DDRAW1280i(int top, int bottom)
{
    int             x,
                    y;
    Uint8          *addr;
    DWORD           c[8];
    /*
     * yループ 
     */
    for (y = top; y < bottom; y++) {
//            if( y >= realDrawArea->h) return;
	/*
	 * xループ 
	 */
	for (x = 0 >> 3; x < 640 >> 3; x++) {
            if(x << 3  >= realDrawArea->w) break;
	    addr = (Uint8 *) realDrawArea->pixels +
		(y << 2) * realDrawArea->pitch +
		(x << 4) * realDrawArea->format->BytesPerPixel;
        switch(realDrawArea->format->BitsPerPixel) {
        case 24:
        case 32:
	    __GETVRAM_3bpp(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640i(addr,
				  realDrawArea->format->BytesPerPixel,
				  realDrawArea->pitch, c);
        break;
        case 8:
        default:
	    __GETVRAM_3bpp(vram_dptr, x, y, c);
	    __SETBYTE_DDRAW_1280_640i(addr,
				  realDrawArea->format->BytesPerPixel,
				  realDrawArea->pitch, c);
        break;
        }
	}

    }
}

static void
Draw640Sub(int top, int bottom)
{

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



/*
 *      640x200/400、デジタルモード
 *      パレット設定
 */
void
Palet640(void)
{
    int             i;
    int             vpage;
    SDL_Color       hardPalette[10];        

    /*
     * パレットテーブル 
     */
    static DWORD    rgbTable_24[] = {
	0x00000000,
	0x000000ff,
	0x00ff0000,
	0x00ff00ff,
	0x0000ff00,
	0x0000ffff,
	0x00ffff00,
	0x00ffffff
    };
    static DWORD    rgbTable_16[] = {
	0x00000000,
	0x0000002f,
	0x0000f800,
	0x0000f82f,
	0x000007c0,
	0x000007ff,
	0x0000ffc0,
	0x0000ffff
    };
    static DWORD    rgbTable_15[] = {
	0x00000000,
	0x0000001f,
	0x00007c00,
	0x00007c1f,
	0x000003e0,
	0x000003ff,
	0x00007fe0,
	0x00007fff
    };
    static DWORD    rgbTable_8[] = {
	0x00000000,
	0x00000001,
	0x00000004,
	0x00000005,
	0x00000002,
	0x00000003,
	0x00000006,
	0x00000007
    };
    static DWORD *rgbTable;
   
    /*
     * マルチページより、表示プレーン情報を得る 
     */
    vpage = (~(multi_page >> 4)) & 0x07;
    realDrawArea = SDL_GetVideoSurface();
    switch(realDrawArea->format->BitsPerPixel){
    case 24:
    case 32:
            rgbTable = rgbTable_24;
            break;
    case 16:
            rgbTable = rgbTable_16;
            break;
    case 15:
            rgbTable = rgbTable_15;
            break;
    case 8:
            rgbTable = rgbTable_8;
            break;
    default:
            rgbTable = rgbTable_8;
            break;
    }

    /*
     * 640x200/400、デジタルパレット 
     */
    for (i = 0; i < 8; i++) {
	if (crt_flag) {
	    /*
	     * CRT ON 
	     */
            rgbTTLGDI[i] = rgbTable[ttl_palet[i & vpage] & 0x07];
            if(realDrawArea->format->BitsPerPixel <= 8) {
                    hardPalette[i].b = (i & 0x01) << 7;  
                    hardPalette[i].r = (i & 0x02) << 6;  
                    hardPalette[i].g = (i & 0x04) << 5;  
            }

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
    if(realDrawArea->format->BitsPerPixel <= 8) {
            hardPalette[8]  = hardPalette[0];
            hardPalette[9]  = hardPalette[4];
            SDL_SetPalette(realDrawArea, SDL_LOGPAL | SDL_PHYSPAL , hardPalette, 0,10);
    }

}

/*
 *      640x200、デジタルモード
 *      描画
 */

/*
 *      640x200、デジタルモード
 *      描画
 */
void
Draw640All(void)
{
#if XM7_VER >= 3
#endif
    realDrawArea = SDL_GetVideoSurface();
    if(realDrawArea == NULL) return;
    /*
     * パレット設定 
     */
    /*
     *描画モードを変えたら強制的にPalet640すること。
     */
    Palet640();
    nDrawTop = 0;
    nDrawBottom = 400;
    nDrawLeft = 0;
    nDrawRight = 640;
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
    switch(nDrawWidth) {
    case 1280:
            Draw640Sub(0,400>>1);
            break;
    case 640:
    default:
            Draw640Sub(0,400>>1);
            break;
    }
	if(!bFullScan){
            RenderSetOddLine();
    } else {
            RenderFullScan();
    }



    nDrawTop = 0;
    nDrawBottom = 400;
    nDrawLeft = 0;
    nDrawRight = 640;
    bPaletFlag = FALSE;
//    SetDrawFlag(FALSE);
}


#endif				/* _XWIN */

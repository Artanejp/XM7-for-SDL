/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *      Copyright (C) 2001-2003 Ryu Takegami
 *      Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *      [ SDL 表示 / 26万色パート]
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

#if XM7_VER >= 3
/*
 *      320x200、26万色モード
 *      描画
 */
void
Draw256k(void)
{
    int             x,
                    y;
    int             offset;
    int             i;
    BYTE            bit;
    DWORD           color;
    BYTE 	   *addr;

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
    // SDL_LockSurface(displayArea);
    SDL_LockSurface(realDrawArea);
    /*
     * yループ 
     */
    for (y = nDrawTop >> 1; y < nDrawBottom >> 1; y++) {

	/*
	 * xループ 
	 */
	for (x = nDrawLeft >> 4; x < nDrawRight >> 4; x++) {
	    // if(GDIDrawFlag[(y >>3) * 80 + x] == 0) continue; /*
	    // 再描画フラグを真面目に見てみることにした 
	    // 
	    // */
	    bit = 0x80;

	    /*
	     * オフセット設定 
	     */
	    offset = 40 * y + x;

	    /*
	     * ビットループ 
	     */
	    for (i = 0; i < 8; i++) {
		color = 0;
		if (!(multi_page & 0x40)) {
		    /*
		     * G評価 
		     */
		    if (vram_c[offset + 0x10000] & bit) {
			color |= 0x008300;
		    }
		    if (vram_c[offset + 0x12000] & bit) {
			color |= 0x004300;
		    }
		    if (vram_c[offset + 0x14000] & bit) {
			color |= 0x002300;
		    }
		    if (vram_c[offset + 0x16000] & bit) {
			color |= 0x001300;
		    }
		    if (vram_c[offset + 0x28000] & bit) {
			color |= 0x000b00;
		    }
		    if (vram_c[offset + 0x2a000] & bit) {
			color |= 0x000700;
		    }
		}

		if (!(multi_page & 0x20)) {
		    /*
		     * R評価 
		     */
		    if (vram_c[offset + 0x08000] & bit) {
			color |= 0x830000;
		    }
		    if (vram_c[offset + 0x0a000] & bit) {
			color |= 0x430000;
		    }
		    if (vram_c[offset + 0x0c000] & bit) {
			color |= 0x230000;
		    }
		    if (vram_c[offset + 0x0e000] & bit) {
			color |= 0x130000;
		    }
		    if (vram_c[offset + 0x20000] & bit) {
			color |= 0x0b0000;
		    }
		    if (vram_c[offset + 0x22000] & bit) {
			color |= 0x070000;
		    }
		}

		if (!(multi_page & 0x10)) {
		    /*
		     * B評価 
		     */
		    if (vram_c[offset + 0x00000] & bit) {
			color |= 0x000083;
		    }
		    if (vram_c[offset + 0x02000] & bit) {
			color |= 0x000043;
		    }
		    if (vram_c[offset + 0x04000] & bit) {
			color |= 0x000023;
		    }
		    if (vram_c[offset + 0x06000] & bit) {
			color |= 0x000013;
		    }
		    if (vram_c[offset + 0x18000] & bit) {
			color |= 0x00000b;
		    }
		    if (vram_c[offset + 0x1a000] & bit) {
			color |= 0x000007;
		    }
		}

		/*
		 * CRTフラグ 
		 */
		if (!crt_flag) {
		    color = 0;
		}
		switch(nDrawWidth) {
		case 1280:
			addr = (Uint8 *) realDrawArea->pixels +
				(y << 2) * realDrawArea->pitch +
				(x << 5) * realDrawArea->format->BytesPerPixel;
			if(bFullScan) {
				__SETDOT_DDRAW_320p_DUP(addr, 
						realDrawArea->format->BytesPerPixel,
						realDrawArea->pitch, color);	
				__SETDOT_DDRAW_320p_DUP(addr + realDrawArea->pitch, 
						realDrawArea->format->BytesPerPixel,
						realDrawArea->pitch, color);
			} else {
				__SETDOT_DDRAW_320p_DUP(addr, 
						realDrawArea->format->BytesPerPixel,
						realDrawArea->pitch, color);	
			}
			break;
			//case 320:
			//addr = (Uint8 *) realDrawArea->pixels +
			//	y * realDrawArea->pitch +
			//	(x << 3) * realDrawArea->format->BytesPerPixel;
			//break;

		default:
			addr = (Uint8 *) realDrawArea->pixels +
				(y << 1) * realDrawArea->pitch +
				(x << 4) * realDrawArea->format->BytesPerPixel;
			if(bFullScan) {
				__SETDOT_DDRAW_320p(addr, 
						realDrawArea->format->BytesPerPixel,
						realDrawArea->pitch, color);	
			} else {
				__SETDOT_DDRAW_320i(addr, 
						realDrawArea->format->BytesPerPixel,
						realDrawArea->pitch, color);	
			}
			break;
		}

		/*
		 * 次のビットへ 
		 */
		bit >>= 1;
	    }
	}
    }
    // SDL_UnlockSurface(displayArea);
    SDL_UnlockSurface(realDrawArea);
    if (!bFullScan) {
            RenderSetOddLine();
    } else {
            RenderFullScan();
    }
//    BitBlt(nDrawLeft, nDrawTop,
//	   (nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
//	   nDrawLeft, nDrawTop);

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


#endif				/* _XWIN */

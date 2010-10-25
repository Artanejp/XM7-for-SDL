/*
 * api_draw_4096.cpp
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <SDL/SDL.h>
#if 1
#include <SDL/SDL_rotozoom.h>
#endif

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

#ifdef __cplusplus
}
#endif


#include "EmuGrph4096c.h"
#include "EmuGrph256kc.h"

#include "EmuGrphScale2x2.h"
#include "EmuGrphScale2x2i.h"
#include "EmuGrphScale4x4.h"
#include "EmuGrphScale4x4i.h"


#ifdef __cplusplus
extern "C"
{
#endif
extern DWORD rgbAnalogGDI[];
#ifdef __cplusplus
}
#endif


static EmuGrph4096c *vramhdr;
static EmuGrph256kc *vramhdr_256k;

static EmuGrphScale2x2 *scaler2x2;
static EmuGrphScale2x2i *scaler2x2i;
static EmuGrphScale4x4 *scaler4x4;
static EmuGrphScale4x4i *scaler4x4i;

static BOOL b256kFlag;

#ifdef __cplusplus
extern "C"
{
#endif

void
Palet320(void)
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

#ifdef __cplusplus
}
#endif



static void VramReader(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
	if(b256kFlag) {
		if(vramhdr_256k != NULL) {
			vramhdr_256k->GetVram(addr, cbuf, mpage);

		}
	} else {
		if(vramhdr != NULL) {
			vramhdr->GetVram(addr, cbuf, mpage);
		}
	}
}

static void PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
	if(vramhdr != NULL) {
		vramhdr->PutWord(disp, pixsize, cbuf);
	}
}

static void PutWord2x(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
		if(vramhdr != NULL) {
			vramhdr->PutWordx2(disp, pixsize, cbuf);
		}
}

static void PutWord4x(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
	if(vramhdr != NULL) {
		scaler4x4->PutWord4x(disp, pixsize, cbuf);
	}
}



static void init_4096_scaler(void)
{
	if(scaler2x2 == NULL) {
		scaler2x2 = new EmuGrphScale2x2;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x2->SetVramReader(VramReader);
		scaler2x2->SetPutWord(PutWord2x);
	}
	if(scaler2x2i == NULL) {
		scaler2x2i = new EmuGrphScale2x2i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x2i->SetVramReader(VramReader);
		scaler2x2i->SetPutWord(PutWord2x);
	}
	if(scaler4x4 == NULL) {
		scaler4x4 = new EmuGrphScale4x4;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler4x4->SetVramReader(VramReader);
		scaler4x4->SetPutWord(PutWord4x);
	}
	if(scaler4x4i == NULL) {
		scaler4x4i = new EmuGrphScale4x4i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler4x4i->SetVramReader(VramReader);
		scaler4x4i->SetPutWord(PutWord4x);
	}
}

static void Setup_4096(void)
{
	b256kFlag = FALSE;
}

static void Setup_256k(void)
{
	b256kFlag = TRUE;
}


#ifdef __cplusplus
extern "C"
{
#endif


BOOL init_4096(void)
{
	vramhdr = new EmuGrph4096c;
	if(vramhdr == NULL) return FALSE;
	vramhdr->SetPaletteTable((Uint32 *)rgbAnalogGDI);
	vramhdr->SetVram(vram_dptr, 40, 200);

	vramhdr_256k = new EmuGrph256kc;
	if(vramhdr_256k == NULL) {
		delete vramhdr;
		vramhdr = NULL;
		return FALSE;
	}
	vramhdr_256k->SetPaletteTable(NULL);
	vramhdr_256k->SetVram(vram_dptr, 40, 200);

	b256kFlag = FALSE;
	scaler2x2 = NULL;
	scaler2x2i = NULL;
	scaler4x4 = NULL;
	scaler4x4i = NULL;
	init_4096_scaler();
	return TRUE;
}

void detach_4096(void)
{
	if(scaler2x2 != NULL) {
		delete scaler2x2;
		scaler2x2 = NULL;
	}
	if(scaler2x2i != NULL) {
		delete scaler2x2i;
		scaler2x2i = NULL;
	}
	if(scaler4x4 != NULL) {
		delete scaler4x4;
		scaler4x4 = NULL;
	}
	if(scaler4x4i != NULL) {
		delete scaler4x4i;
		scaler4x4i = NULL;
	}
	// 最後にVRAMハンドラ
	if(vramhdr != NULL) {
		delete vramhdr;
		vramhdr = NULL;
	}
	if(vramhdr_256k != NULL) {
		delete vramhdr_256k;
		vramhdr_256k = NULL;
	}
}



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
	 *描画モードを変えたら強制的にPalet640すること。
	 */

	Palet320();
	Setup_4096();
	nDrawTop = 0;
	nDrawBottom = 400;
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
	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
			if(scaler4x4 != NULL) {
				scaler4x4->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		case 640:
		default:
			if(scaler2x2 != NULL) {
				scaler2x2->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		}
	} else {
		switch(nDrawWidth) {
		case 1280:
			if(scaler4x4i != NULL) {
				scaler4x4i->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		case 640:
		default:
			if(scaler2x2i != NULL) {
				scaler2x2i->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		}

	}
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 320;
	bPaletFlag = FALSE;
	//    SetDrawFlag(FALSE);
}

void Draw256k(void)
{
#if XM7_VER >= 3
#endif
	realDrawArea = SDL_GetVideoSurface();
	if(realDrawArea == NULL) return;

	Setup_256k();
//	Palet320();
	nDrawTop = 0;
	nDrawBottom = 400;
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
	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
			if(scaler4x4 != NULL) {
				scaler4x4->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		case 640:
		default:
			if(scaler2x2 != NULL) {
				scaler2x2->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		}
	} else {
		switch(nDrawWidth) {
		case 1280:
			if(scaler4x4i != NULL) {
				scaler4x4i->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		case 640:
		default:
			if(scaler2x2i != NULL) {
				scaler2x2i->PutVram(realDrawArea, 0, 0, 320, 200, (Uint32) multi_page );
			}
			break;
		}

	}
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 320;
	bPaletFlag = FALSE;
	//    SetDrawFlag(FALSE);
}
#ifdef __cplusplus
}
#endif

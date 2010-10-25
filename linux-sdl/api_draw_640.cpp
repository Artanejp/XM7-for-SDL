/*
 * api_draw_640.cpp
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */
#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
}
#endif


#include "EmuGrphLib.h"
#include "EmuGrph400l.h"

#include "EmuGrphScale1x1.h"
#include "EmuGrphScale1x2.h"
#include "EmuGrphScale1x2i.h"
#include "EmuGrphScale2x2.h"
#include "EmuGrphScale2x2i.h"
#include "EmuGrphScale2x4.h"
#include "EmuGrphScale2x4i.h"


#ifdef __cplusplus
extern "C"
{
#endif
extern DWORD rgbTTLGDI[];
#ifdef __cplusplus
}
#endif


static EmuGrphLib *vramhdr;
static EmuGrph400l *vramhdr_400l;

static EmuGrphScale1x1 *scaler1x1;
static EmuGrphScale1x2 *scaler1x2;
static EmuGrphScale1x2i *scaler1x2i;

static EmuGrphScale2x2 *scaler2x2;
static EmuGrphScale2x2i *scaler2x2i;

static EmuGrphScale2x4 *scaler2x4;
static EmuGrphScale2x4i *scaler2x4i;

static BOOL b400lFlag;

#ifdef __cplusplus
extern "C"
{
#endif

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
	if(vramhdr != NULL) {
		vramhdr->SetPaletteTable((Uint32 *)rgbTTLGDI);
		//    	vramhdr->InitPalette();
	}
	if(vramhdr_400l != NULL) {
		vramhdr_400l->SetPaletteTable((Uint32 *)rgbTTLGDI);
		//    	vramhdr->InitPalette();
	}
#if 1
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
#endif

}

#ifdef __cplusplus
}
#endif


static void VramReader(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
	if(b400lFlag) {
		if(vramhdr_400l != NULL) {
			vramhdr_400l->GetVram(addr, cbuf, mpage);
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
	if(scaler2x4 != NULL) {
		scaler2x4->PutWord2x(disp, pixsize, cbuf);
	} else if(scaler2x4i != NULL) {
		scaler2x4i->PutWord2x(disp, pixsize, cbuf);
	}
}


static void init_640_scaler(void)
{
	if(scaler1x1 == NULL) {
		scaler1x1 = new EmuGrphScale1x1;
		//		scaler1x2->SetConvWord(&vramhdr->ConvWord);
		scaler1x1->SetVramReader(VramReader);
		scaler1x1->SetPutWord(PutWord);
	}
	if(scaler1x2 == NULL) {
		scaler1x2 = new EmuGrphScale1x2;
		//		scaler1x2->SetConvWord(&vramhdr->ConvWord);
		scaler1x2->SetVramReader(VramReader);
		scaler1x2->SetPutWord(PutWord);
	}
	if(scaler1x2i == NULL) {
		scaler1x2i = new EmuGrphScale1x2i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler1x2i->SetVramReader(VramReader);
		scaler1x2i->SetPutWord(PutWord);
	}
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

	if(scaler2x4 == NULL) {
		scaler2x4 = new EmuGrphScale2x4;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x4->SetVramReader(VramReader);
		scaler2x4->SetPutWord(PutWord2x);
	}
	if(scaler2x4i == NULL) {
		scaler2x4i = new EmuGrphScale2x4i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x4i->SetVramReader(VramReader);
		scaler2x4i->SetPutWord(PutWord2x);
	}

}

#ifdef __cplusplus
extern "C"
{
#endif


BOOL init_640(void)
{
	b400lFlag = FALSE;
	vramhdr = new EmuGrphLib();
	if(vramhdr == NULL) return FALSE;
	vramhdr->SetPaletteTable((Uint32 *)rgbTTLGDI);
	vramhdr->SetVram(vram_dptr, 80, 200);

	vramhdr_400l = new EmuGrph400l();
	if(vramhdr_400l == NULL) {
		delete vramhdr;
		return FALSE;
	}
	vramhdr_400l->SetPaletteTable((Uint32 *)rgbTTLGDI);
	vramhdr_400l->SetVram(vram_dptr, 80, 400);

	scaler1x1 = NULL;
	scaler1x2 = NULL;
	scaler1x2i = NULL;

	scaler2x2 = NULL;
	scaler2x2i = NULL;

	scaler2x4 = NULL;
	scaler2x4i = NULL;
	init_640_scaler();
	return TRUE;
}

void detach_640(void)
{
	if(scaler1x1 != NULL) {
		delete scaler1x1;
		scaler1x1 = NULL;
	}
	if(scaler1x2 != NULL) {
		delete scaler1x2;
		scaler1x2 = NULL;
	}
	if(scaler1x2i != NULL) {
		delete scaler1x2i;
		scaler1x2i = NULL;
	}
	if(scaler2x2 != NULL) {
		delete scaler2x2;
		scaler2x2 = NULL;
	}
	if(scaler2x2i != NULL) {
		delete scaler2x2i;
		scaler2x2i = NULL;
	}
	if(scaler2x4 != NULL) {
		delete scaler2x4;
		scaler2x4 = NULL;
	}
	if(scaler2x4i != NULL) {
		delete scaler2x4i;
		scaler2x4i = NULL;
	}
	// 最後にVRAMハンドラ
	if(vramhdr != NULL) {
		delete vramhdr;
		vramhdr = NULL;
	}
}


static void PutVram_1x1(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler1x1->PutVram(p, x, y, w, h, mpage);
}

static void PutVram_1x2(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler1x2->PutVram(p, x, y, w, h, mpage);
}

static void PutVram_1x2i(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler1x2i->PutVram(p, x, y, w, h, mpage);
}

static void PutVram_2x2(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler2x2->PutVram(p, x, y, w, h, mpage);
}

static void PutVram_2x2i(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler2x2i->PutVram(p, x, y, w, h, mpage);
}

static void PutVram_2x4(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler2x4->PutVram(p, x, y, w, h, mpage);
}
static void PutVram_2x4i(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler2x4i->PutVram(p, x, y, w, h, mpage);
}

void
Draw640All(void)
{
	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	/*
	 * パレット設定
	 */
	/*
	 *描画モードを変えたら強制的にPalet640すること。
	 */

	Palet640();
	b400lFlag = FALSE;
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

	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
			if(scaler2x4 != NULL) {
				PutVramFunc = &PutVram_2x4;
			}
			break;
		case 640:
		default:
			if(scaler1x1 != NULL) {
				PutVramFunc = &PutVram_1x2;
			}
			break;
		}
	} else {
		switch(nDrawWidth) {
		case 1280:
			if(scaler2x4i != NULL) {
				PutVramFunc = &PutVram_2x4i;
			}
			break;
		case 640:
		default:
			if(scaler1x2i != NULL) {
				PutVramFunc = &PutVram_1x2i;
			}
			break;
		}
		}
	if(window_open) {
		if(vramhdr_400l != NULL) {
			vramhdr_400l->SetVram(vram_bdptr, 80, 200);
		}
	} else {
		if(vramhdr_400l != NULL) {
			vramhdr_400l->SetVram(vram_dptr, 80, 200);
		}
	}
	if(PutVramFunc != NULL) {
		PutVramFunc(p, 0, 0, 640, 200, multi_page);
	}

	/*
	 * レンダリング
	 */
	PutVramFunc(realDrawArea, 0, 0, 640, 200, (Uint32) multi_page );

	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = FALSE;
	//    SetDrawFlag(FALSE);
}


void
Draw400l(void)
{

	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	SDL_Surface *p;
    /*
     * パレット設定
     */
	b400lFlag = TRUE;
//	PutVramFunc = NULL;

	Palet640();

    /*
     * クリア処理
     */
    if (bClearFlag) {
	AllClear();
    }

    /*
     * レンダリング
     */

	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	p = SDL_GetVideoSurface();
	if(p == NULL) return;

	/*
	 * レンダリング
	 */
	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
			if(scaler2x2 != NULL) {
				PutVramFunc = &PutVram_2x2;
			}
			break;
		case 640:
		default:
			if(scaler1x1 != NULL) {
				PutVramFunc = &PutVram_1x1;
			}
			break;
		}
	} else {
		switch(nDrawWidth) {
		case 1280:
			if(scaler2x2i != NULL) {
				PutVramFunc = &PutVram_2x2i;
			}
			break;
		case 640:
		default:
			if(scaler1x1 != NULL) {
				PutVramFunc = &PutVram_1x1;
			}
			break;
		}
		}

	if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		if(window_open) {
			if(vramhdr_400l != NULL) {
				vramhdr_400l->SetVram(vram_bdptr, 80, 400);
			}
		} else {
			if(vramhdr_400l != NULL) {
				vramhdr_400l->SetVram(vram_dptr, 80, 400);
			}
		}
		if(PutVramFunc != NULL) {
			PutVramFunc(p, 0, 0, 640, 400, multi_page);
		}
	}
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = FALSE;

}
#ifdef __cplusplus
}
#endif

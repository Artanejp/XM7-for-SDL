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
#include "EmuGLUtils.h"

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

static EmuGLUtils *scalerGL;

static BOOL b400lFlag;

#ifdef __cplusplus
extern "C"
{
#endif

static void Palet640Sub(Uint32 i, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	if(vramhdr != NULL) {
		vramhdr->CalcPalette(i, r, g, b, a, p);
	}
	if(vramhdr_400l != NULL) {
		vramhdr_400l->CalcPalette(i, r, g, b, a, p);
	}

}

void
Palet640(void)
{
	int i;
	int             vpage;
	BYTE          tmp;
	BYTE         g,r,b;
	BYTE         a = 255; // Alpha

	vpage = (~(multi_page >> 4)) & 0x07;
	if(vramhdr != NULL) {
		vramhdr->SetPaletteTable((Uint32 *)rgbTTLGDI);
	}
	if(vramhdr_400l != NULL) {
		vramhdr_400l->SetPaletteTable((Uint32 *)rgbTTLGDI);
	}

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

#ifdef __cplusplus
}
#endif


static void VramReader(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
		if(vramhdr != NULL) {
			vramhdr->GetVram(addr, cbuf, mpage);
		}
}

static void VramReader_400l(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
		if(vramhdr_400l != NULL) {
			vramhdr_400l->GetVram(addr, cbuf, mpage);
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
	}
}


static void init_640_scaler(void)
{
	if(scaler1x1 == NULL) {
		scaler1x1 = new EmuGrphScale1x1;
		//		scaler1x2->SetConvWord(&vramhdr->ConvWord);
		scaler1x1->SetVramReader(VramReader, 80, 400);
		scaler1x1->SetPutWord(PutWord);
	}
	if(scaler1x2 == NULL) {
		scaler1x2 = new EmuGrphScale1x2;
		//		scaler1x2->SetConvWord(&vramhdr->ConvWord);
		scaler1x2->SetVramReader(VramReader, 80, 400);
		scaler1x2->SetPutWord(PutWord);
	}
	if(scaler1x2i == NULL) {
		scaler1x2i = new EmuGrphScale1x2i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler1x2i->SetVramReader(VramReader, 80, 400);
		scaler1x2i->SetPutWord(PutWord);
	}
	if(scaler2x2 == NULL) {
		scaler2x2 = new EmuGrphScale2x2;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x2->SetVramReader(VramReader, 80, 400);
		scaler2x2->SetPutWord(PutWord2x);
	}
	if(scaler2x2i == NULL) {
		scaler2x2i = new EmuGrphScale2x2i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x2i->SetVramReader(VramReader, 80, 400);
		scaler2x2i->SetPutWord(PutWord2x);
	}

	if(scaler2x4 == NULL) {
		scaler2x4 = new EmuGrphScale2x4;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x4->SetVramReader(VramReader, 80, 400);
		scaler2x4->SetPutWord(PutWord2x);
	}
	if(scaler2x4i == NULL) {
		scaler2x4i = new EmuGrphScale2x4i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler2x4i->SetVramReader(VramReader, 80, 400);
		scaler2x4i->SetPutWord(PutWord2x);
	}
	if(scalerGL == NULL) {
		scalerGL = new EmuGLUtils;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scalerGL->SetVramReader(VramReader, 80, 400);
		scalerGL->SetPutWord(PutWord);
	}


}

#ifdef __cplusplus
extern "C"
{
#endif

void Flip640(void)
{
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
#ifdef USE_OPENGL
	if(scalerGL != NULL) {
		scalerGL->Flip();
	} else {
		SDL_Flip(p);
	}
#else
	SDL_Flip();
#endif
}

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
	scalerGL = NULL;
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
	if(scalerGL != NULL) {
		delete scalerGL;
		scalerGL = NULL;
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

static void PutVram_GL(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scalerGL->SetViewPort(0, 0, 640, 400);
	scalerGL->PutVram(p, x, y, w, h, mpage);
}

static void SetVramReader_200l()
{
	if(scaler1x1 != NULL) {
		scaler1x1->SetVramReader(VramReader, 80, 400);
	}
	if(scaler1x2 != NULL) {
		scaler1x2->SetVramReader(VramReader, 80, 400);
	}
	if(scaler1x2i != NULL) {
		scaler1x2i->SetVramReader(VramReader, 80, 400);
	}
	if(scaler2x2 != NULL) {
		scaler2x2->SetVramReader(VramReader, 80, 400);
	}
	if(scaler2x2i != NULL) {
		scaler2x2i->SetVramReader(VramReader, 80, 400);
	}
	if(scaler2x4 != NULL) {
		scaler2x4->SetVramReader(VramReader, 80, 400);
	}
	if(scaler2x4i != NULL) {
		scaler2x4i->SetVramReader(VramReader, 80, 400);
	}
	if(scalerGL != NULL) {
		scalerGL->SetVramReader(VramReader, 80, 400);
	}
//	if(scaler4x4 != NULL) {
//		scaler4x4->SetVramReader(VramReader, 80, 400);
//	}
//	if(scaler4x4i != NULL) {
//		scaler4x4i->SetVramReader(VramReader, 80, 400);
//	}
}

static void SetVramReader_400l()
{
	if(scaler1x1 != NULL) {
		scaler1x1->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scaler1x2 != NULL) {
		scaler1x2->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scaler1x2i != NULL) {
		scaler1x2i->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scaler2x2 != NULL) {
		scaler2x2->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scaler2x2i != NULL) {
		scaler2x2i->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scaler2x4 != NULL) {
		scaler2x4->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scaler2x4i != NULL) {
		scaler2x4i->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scalerGL != NULL) {
		scalerGL->SetVramReader(VramReader, 80, 400);
	}

//	if(scaler4x4 != NULL) {
//		scaler4x4->SetVramReader(VramReader_400l, 80, 400);
//	}
//	if(scaler4x4i != NULL) {
//		scaler4x4i->SetVramReader(VramReader_400l, 80, 400);
//	}
}


void
Draw640All(void)
{
	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	SDL_Surface *p;
	WORD wdtop, wdbtm;

	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	/*
	 * パレット設定
	 */
	/*
	 *描画モードを変えたら強制的にPalet640すること。
	 */
	SetVramReader_200l();
	if(bPaletFlag) {
		Palet640();
	        bPaletFlag = FALSE;
		b400lFlag = FALSE;
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

	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
#ifndef USE_OPENGL
			if(scaler2x4 != NULL) {
				PutVramFunc = &PutVram_2x4;
			}
#else
			if(scalerGL != NULL) {
				PutVramFunc = &PutVram_GL;
			}
#endif

			break;
		case 640:
		default:
#ifndef USE_OPENGL
			if(scaler1x1 != NULL) {
				PutVramFunc = &PutVram_1x2;
			}
#else
			if(scalerGL != NULL) {
				PutVramFunc = &PutVram_GL;
			}
#endif
			break;
		}
	} else {
		switch(nDrawWidth) {
		case 1280:
#ifndef USE_OPENGL
			if(scaler2x4i != NULL) {
				PutVramFunc = &PutVram_2x4i;
			}
#else
			if(scalerGL != NULL) {
				PutVramFunc = &PutVram_GL;
			}
#endif
			break;
		case 640:
		default:
#ifndef USE_OPENGL
			if(scaler1x2i != NULL) {
				PutVramFunc = &PutVram_1x2i;
			}
#else
			if(scalerGL != NULL) {
				PutVramFunc = &PutVram_GL;
			}
#endif
			break;
		}
		}

	/*
	 * レンダリング
	 */
	if(PutVramFunc == NULL) return;
	if(vramhdr_400l == NULL) return;
	if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		if(window_open) { // ハードウェアウインドウ開いてる
            if ((nDrawTop >> 1) < window_dy1) {
    				vramhdr->SetVram(vram_dptr, 80, 200);
    				PutVramFunc(p, 0, nDrawTop >> 1, 640, window_dy1, multi_page);
            }
            /* ウィンドウ内の描画 */
            if ((nDrawTop >> 1) > window_dy1) {
                    wdtop = nDrawTop >> 1;
            }
            else {
                    wdtop = window_dy1;
            }

            if ((nDrawBottom >> 1)< window_dy2) {
                    wdbtm = nDrawBottom >> 1;
            }
            else {
                    wdbtm = window_dy2;
            }

            if (wdbtm > wdtop) {
            		vramhdr->SetVram(vram_bdptr, 80, 200);
            		PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
            }
    		/* ハードウェアウインドウ外下部 */
            if ((nDrawBottom >> 1) > window_dy2) {
           		vramhdr->SetVram(vram_dptr, 80, 200);
           		PutVramFunc(p, 0 , wdbtm, 640, (nDrawBottom >> 1) - wdbtm, multi_page);
            }
		} else { // ハードウェアウィンドウ開いてない
			vramhdr->SetVram(vram_dptr, 80, 200);
			PutVramFunc(p, 0, 0, 640, 200, multi_page);
		}
	}

	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
//	bPaletFlag = FALSE;
	SetDrawFlag(FALSE);
}


void
Draw400l(void)
{

	void (*PutVramFunc)(SDL_Surface *, int, int, int, int, Uint32);
	SDL_Surface *p;
	WORD wdtop, wdbtm;
    /*
     * パレット設定
     */
	b400lFlag = TRUE;
	SetVramReader_400l();
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
	if(PutVramFunc == NULL) return;
	if(vramhdr_400l == NULL) return;
	if((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
		if(window_open) { // ハードウェアウインドウ開いてる
            if (nDrawTop < window_dy1) {
    				vramhdr_400l->SetVram(vram_dptr, 80, 400);
    				PutVramFunc(p, 0, nDrawTop, 640, window_dy1, multi_page);
            }
            /* ウィンドウ内の描画 */
            if (nDrawTop > window_dy1) {
                    wdtop = nDrawTop;
            }
            else {
                    wdtop = window_dy1;
            }

            if (nDrawBottom < window_dy2) {
                    wdbtm = nDrawBottom;
            }
            else {
                    wdbtm = window_dy2;
            }

            if (wdbtm > wdtop) {
            		vramhdr_400l->SetVram(vram_bdptr, 80, 400);
            		PutVramFunc(p, window_dx1, wdtop, window_dx2 - window_dx1 , wdbtm - wdtop , multi_page);
            }
    		/* ハードウェアウインドウ外下部 */
            if (nDrawBottom  > window_dy2) {
           		vramhdr_400l->SetVram(vram_dptr, 80, 400);
           		PutVramFunc(p, 0 , wdbtm, 640, nDrawBottom - wdbtm, multi_page);
            }
		} else { // ハードウェアウィンドウ開いてない
			vramhdr_400l->SetVram(vram_dptr, 80, 400);
			PutVramFunc(p, 0, 0, 640, 400, multi_page);
		}
	}
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = FALSE;
    //SetDrawFlag(FALSE);

}
#ifdef __cplusplus
}
#endif

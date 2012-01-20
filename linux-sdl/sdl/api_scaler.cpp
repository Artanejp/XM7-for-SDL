/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2003 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 *  [SDL 表示 ]
 *  2010.10.28 api_draw.cpp からスケーラ部分分割
 */

#ifdef USE_AGAR
//#include <agar/core.h>
//#include <agar/core/types.h>
#ifdef USE_OPENGL
#include "agar_gldraw.h"
#endif
#endif

#include <SDL.h>

#include "api_draw.h"
#include "api_scaler.h"


#ifdef USE_AGAR
//EmuAgarGL *scalerGL;
extern AG_Window *MainWindow;
//EmuAgarGL *pScalerGL;
#else
EmuGLUtils *pScalerGL;
#endif
EmuGrphScaleTmpl *pSwScaler;


#ifdef __cplusplus
extern "C"
{
#endif

#if 1
static inline void putdot(GLubyte *addr, Uint32 c)
{
	Uint32 *addr32 = (Uint32 *)addr;
	*addr32 = c;
}

#else
static inline void putdot(GLubyte *addr, Uint32 c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[0] = 0xff;		/* A */
    addr[1] = c & 0xff;	/* B */
    addr[2] = (c >> 8) & 0xff;	/* R */
    addr[3] = (c >> 16) & 0xff; /* G */
#else
    addr[3] = 0xff;		/* A */
    addr[2] = c & 0xff;	/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff; /* G */
#endif
}
#endif

void PutWordGL(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
		putdot((GLubyte *)&disp[0], cbuf[7]);
		putdot((GLubyte *)&disp[1], cbuf[6]);
		putdot((GLubyte *)&disp[2], cbuf[5]);
		putdot((GLubyte *)&disp[3], cbuf[4]);
		putdot((GLubyte *)&disp[4], cbuf[3]);
		putdot((GLubyte *)&disp[5], cbuf[2]);
		putdot((GLubyte *)&disp[6], cbuf[1]);
		putdot((GLubyte *)&disp[7], cbuf[0]);
}

static inline void putdot8(GLubyte *addr, Uint32 c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[0] = 0xff;		/* A */
    addr[1] = c & 0xff;	/* B */
    addr[2] = (c >> 8) & 0xff;	/* R */
    addr[3] = (c >> 16) & 0xff; /* G */
#else
//    addr[3] = 0xff;		/* A */
    addr[3] = c & 0xff;	/* B */
    addr[2] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff; /* G */
#endif
}

void PutWordGL8(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
		putdot((GLubyte *)&disp[0], cbuf[7]);
		putdot((GLubyte *)&disp[1], cbuf[6]);
		putdot((GLubyte *)&disp[2], cbuf[5]);
		putdot((GLubyte *)&disp[3], cbuf[4]);
		putdot((GLubyte *)&disp[4], cbuf[3]);
		putdot((GLubyte *)&disp[5], cbuf[2]);
		putdot((GLubyte *)&disp[6], cbuf[1]);
		putdot((GLubyte *)&disp[7], cbuf[0]);
}

void PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
	if(vramhdr != NULL) {
		vramhdr->PutWord(disp, pixsize, cbuf);
	}
}

void PutWord_4096(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
	if(vramhdr_4096 != NULL) {
		vramhdr_4096->PutWord(disp, pixsize, cbuf);
	}
}

void SetVramReader_200l()
{
#ifdef USE_AGAR
//	SetVramReader_AG_GL(VramReader, 80, 200);
    SetVramReader_GL2(VramReader, 80, 200);
#else
	if(pScalerGL != NULL) {
		pScalerGL->SetVramReader(VramReader, 80, 200);
		pScalerGL->SetPutWord(PutWordGL8);
	}
#endif
	if(pSwScaler != NULL) {
		pSwScaler->SetVramReader(VramReader, 80, 200);
		pSwScaler->SetPutWord(PutWord);
	}
}

void SetVramReader_400l()
{
#ifdef USE_AGAR
//	SetVramReader_AG_GL(VramReader_400l, 80, 400);
    SetVramReader_GL2(VramReader_400l, 80, 400);
#else
	if(pScalerGL != NULL) {
		pScalerGL->SetVramReader(VramReader_400l, 80, 400);
		pScalerGL->SetPutWord(PutWordGL8);
	}
#endif
	if(pSwScaler != NULL) {
		pSwScaler->SetVramReader(VramReader_400l, 80, 400);
		pSwScaler->SetPutWord(PutWord);
	}
}

void SetVramReader_4096(void)
{
#ifdef USE_AGAR
//	SetVramReader_AG_GL(VramReader_4096, 40, 200);
    SetVramReader_GL2(VramReader_4096, 40, 200);
#else
	if(pScalerGL != NULL) {
		pScalerGL->SetVramReader(VramReader_4096, 40, 200);
		pScalerGL->SetPutWord(PutWordGL);
	}
#endif
	if(pSwScaler != NULL) {
		pSwScaler->SetVramReader(VramReader_4096, 40, 200);
		pSwScaler->SetPutWord(PutWord);
	}
}

void SetVramReader_256k(void)
{
#ifdef USE_AGAR
//	SetVramReader_AG_GL(VramReader_256k, 40, 200);
    SetVramReader_GL2(VramReader_256k, 40, 200);
#else
	if(pScalerGL != NULL) {
		pScalerGL->SetVramReader(VramReader_256k, 40, 200);
		pScalerGL->SetPutWord(PutWordGL);
	}
#endif
	if(pSwScaler != NULL) {
		pSwScaler->SetVramReader(VramReader_256k, 40, 200);
		pSwScaler->SetPutWord(PutWord);
	}
}

static void SelectScaler200l(int w, int h)
{
    	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
            pSwScaler = new EmuGrphScale2x4;
            break;
        case 640:
            pSwScaler = new EmuGrphScale1x2;
            break;
		default:
            pSwScaler = new EmuGrphScale1x1;
			break;
			}
		} else {
			switch(nDrawWidth) {
			case 1280:
                pSwScaler = new EmuGrphScale2x4i;
                break;
			case 640:
	            pSwScaler = new EmuGrphScale1x2i;
                break;
			default:
	            pSwScaler = new EmuGrphScale1x1;
				break;
			}
		}
}


static void SelectScaler400l(int w, int h)
{
    	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
            pSwScaler = new EmuGrphScale2x2;
            break;
        case 640:
            pSwScaler = new EmuGrphScale1x1;
            break;
		default:
            pSwScaler = new EmuGrphScale1x1;
			break;
			}
		} else {
			switch(nDrawWidth) {
			case 1280:
                pSwScaler = new EmuGrphScale2x2i;
                break;
			case 640:
	            pSwScaler = new EmuGrphScale1x1;
                break;
			default:
	            pSwScaler = new EmuGrphScale1x1;
				break;
			}
		}
}

static void SelectScaler320(int w, int h)
{
    	if(bFullScan) {
		switch(nDrawWidth) {
		case 1280:
            pSwScaler = new EmuGrphScale4x4;
            break;
        case 640:
            pSwScaler = new EmuGrphScale2x2;
            break;
		default:
            pSwScaler = new EmuGrphScale1x1;
			break;
			}
		} else {
			switch(nDrawWidth) {
			case 1280:
                pSwScaler = new EmuGrphScale4x4i;
                break;
			case 640:
	            pSwScaler = new EmuGrphScale2x2i;
                break;
			default:
	            pSwScaler = new EmuGrphScale1x1;
				break;
			}
		}
}

/*
 * Select Scaler
 */
void SelectScaler(int w, int h)
{
    if(pSwScaler != NULL) {
        delete pSwScaler;
        pSwScaler = NULL;
    }
	switch (bMode) {
	case SCR_400LINE:
		SelectScaler400l(w, h);
		SetVramReader_400l();
		break;
	case SCR_262144:
		SelectScaler320(w, h);
		SetVramReader_256k();
		break;
	case SCR_4096:
		SelectScaler320(w, h);
		SetVramReader_4096();
		break;
	case SCR_200LINE:
		SelectScaler200l(w, h);
		SetVramReader_200l();
		break;
	}
}

void init_scaler(void)
{
#ifdef USE_AGAR
//	InitGL_AG_GL(80 * 8, 200);
	SetVramReader_GL2(VramReader, 80, 200);
#else
    pSwScaler = NULL;
    // 最初は1x1にする
    pSwScaler = new EmuGrphScale1x1;
    if(pSwScaler != NULL) {
        pSwScaler->SetVramReader(VramReader, 80, 400);
		pSwScaler->SetPutWord(PutWord);
    }

	if(pScalerGL == NULL) {
#ifdef USE_AGAR
//		pScalerGL = new EmuAgarGL;
#else
		pScalerGL = new EmuGLUtils;
		pScalerGL->SetVramReader(VramReader, 80, 400);
		pScalerGL->SetPutWord(PutWordGL8);
#endif
	}
#endif
}

void initsub_scaler()
{
	//	b256kFlag = FALSE;
    if(pSwScaler != NULL) {
        delete pSwScaler;
        pSwScaler = NULL;
    }
#ifndef USE_AGAR
    if(pScalerGL != NULL) {
        delete pScalerGL;
       	pScalerGL = NULL;
    }
#endif
}


void detachsub_scaler(void)
{
#ifdef USE_OPENGL
//	Detach_AG_GL();
	DetachGL_AG2();
//	DetachGL_AG_Blocked();
#else
	if(scalerGL != NULL) {
		delete scalerGL;
		scalerGL = NULL;
	}
#endif
	if(pSwScaler != NULL) {
		delete pSwScaler;
		pSwScaler = NULL;
	}
}

void InitNonGL(int w, int h)
{
	Uint32 flags;
	char *ext;

	if(InitVideo) return;
    InitVideo = TRUE;

    vram_pb = NULL;
    vram_pg = NULL;
    vram_pr = NULL;

	flags = SDL_RESIZABLE;

	InitVramSemaphore();
	pVirtualVram = NULL;
	InitVirtualVram();
    return;
}


void InitGL(int w, int h)
{
#ifdef USE_AGAR
    AG_Driver *drv;

    if(agDriverSw) {
        drv = &agDriverSw->_inherit;
    } else if(MainWindow != NULL) {
        drv = AGDRIVER(MainWindow);
    } else {
        return;
    }
    SDL_SemWait(DrawInitSem);
    if(AG_UsingGL(NULL)) {
//        InitGL_AG_GL(w, h);
        InitGL_AG2(w, h);
//        InitGL_AG_Blocked(w, h);
    } else {
        InitNonGL(w, h);
        AG_ResizeDisplay(w, h);
    }
    SDL_SemPost(DrawInitSem);
#else
    SDL_SemWait(DrawInitSem);
   if(scalerGL) {
    	scalerGL->InitGL(w, h);
    }
    SDL_SemPost(DrawInitSem);
#endif
}

void Flip(void)
{
	SDL_Surface *p;
	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	if(DrawInitSem == NULL) return;
	SDL_SemWait(DrawInitSem);
	if(!bUseOpenGL) {
		SDL_Flip(p);
	} else {
#ifdef USE_AGAR
		Flip_AG_GL();
#else
		if(scalerGL != NULL) {
			scalerGL->Flip();
		} else {
			SDL_Flip(p);
		}
#endif
	}
	SDL_SemPost(DrawInitSem);
}


void Scaler_GL(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
#ifndef USE_AGAR
	scalerGL->SetViewPort(0,0, w, h);
    if(!bFullScan) {
    	scalerGL->SetScanLine(TRUE);
    } else {
    	scalerGL->SetScanLine(FALSE);
    }
    scalerGL->PutVram(p, x, y, w, h, mpage);
#else
    /*
     * Agar+OpenGLの場合、WidgetがViewPort設定するのでここでは
     * テクスチャのアップデートだけ。
     */
//    PutVram_AG_GL2(p, x, y, w, h, mpage);
#endif
//	scalerGL->SetViewPort();
}

void SwScaler(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip)
{
	if(pSwScaler == NULL) return;
	pSwScaler->PutVram(p, x, y, w, h, multip );
}

#ifdef __cplusplus
}
#endif


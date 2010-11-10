/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2003 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 *  [SDL 表示 ]
 *  2010.10.28 api_draw.cpp からスケーラ部分分割
 */

#include "api_draw.h"
#include "api_scaler.h"


EmuGrphScale1x1 *scaler1x1;
EmuGrphScale1x2 *scaler1x2;
EmuGrphScale1x2i *scaler1x2i;
EmuGrphScale2x2 *scaler2x2;
EmuGrphScale2x2i *scaler2x2i;
EmuGrphScale2x4 *scaler2x4;
EmuGrphScale2x4i *scaler2x4i;
EmuGrphScale4x4 *scaler4x4;
EmuGrphScale4x4i *scaler4x4i;

#ifdef USE_AGAR
EmuAgarGL *scalerGL;
#else
EmuGLUtils *scalerGL;
#endif


#ifdef __cplusplus
extern "C"
{
#endif

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

void PutWord2x(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
	if(scaler2x4 != NULL) {
		scaler2x4->PutWord2x(disp, pixsize, cbuf);
	}
}

void PutWord_4096(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
	if(vramhdr_4096 != NULL) {
		vramhdr_4096->PutWord(disp, pixsize, cbuf);
	}
}

void PutWord2x2(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
		if(vramhdr_4096 != NULL) {
			vramhdr_4096->PutWordx2(disp, pixsize, cbuf);
		}
}

void PutWord4x(Uint32 *disp, Uint32 pixsize, Uint32 *cbuf)
{
	if(scaler4x4 != NULL) {
		scaler4x4->PutWord4x(disp, pixsize, cbuf);
	}
}

void SetVramReader_200l()
{
	if(scaler1x1 != NULL) {
		scaler1x1->SetVramReader(VramReader, 80, 200);
	}
	if(scaler1x2 != NULL) {
		scaler1x2->SetVramReader(VramReader, 80, 200);
		scaler1x2->SetPutWord(PutWord);
	}
	if(scaler1x2i != NULL) {
		scaler1x2i->SetVramReader(VramReader, 80, 200);
		scaler1x2i->SetPutWord(PutWord);
	}
	if(scaler2x2 != NULL) {
		scaler2x2->SetVramReader(VramReader, 80, 200);
		scaler2x2->SetPutWord(PutWord2x);
	}
	if(scaler2x2i != NULL) {
		scaler2x2i->SetVramReader(VramReader, 80, 200);
		scaler2x2i->SetPutWord(PutWord2x);
	}
	if(scaler2x4 != NULL) {
		scaler2x4->SetVramReader(VramReader, 80, 200);
		scaler2x4->SetPutWord(PutWord2x);
	}
	if(scaler2x4i != NULL) {
		scaler2x4i->SetVramReader(VramReader, 80, 200);
		scaler2x4i->SetPutWord(PutWord2x);
	}
	if(scalerGL != NULL) {
		scalerGL->SetVramReader(VramReader, 80, 200);
		scalerGL->SetPutWord(PutWordGL8);
	}
}

void SetVramReader_400l()
{
	if(scaler1x1 != NULL) {
		scaler1x1->SetVramReader(VramReader_400l, 80, 400);
	}
	if(scaler1x2 != NULL) {
		scaler1x2->SetVramReader(VramReader_400l, 80, 400);
		scaler1x2->SetPutWord(PutWord);
	}
	if(scaler1x2i != NULL) {
		scaler1x2i->SetVramReader(VramReader_400l, 80, 400);
		scaler1x2i->SetPutWord(PutWord);
	}
	if(scaler2x2 != NULL) {
		scaler2x2->SetVramReader(VramReader_400l, 80, 400);
		scaler2x2->SetPutWord(PutWord2x);
	}
	if(scaler2x2i != NULL) {
		scaler2x2i->SetVramReader(VramReader_400l, 80, 400);
		scaler2x2i->SetPutWord(PutWord2x);
	}
	if(scaler2x4 != NULL) {
		scaler2x4->SetVramReader(VramReader_400l, 80, 400);
		scaler2x4->SetPutWord(PutWord2x);
	}
	if(scaler2x4i != NULL) {
		scaler2x4i->SetVramReader(VramReader_400l, 80, 400);
		scaler2x4i->SetPutWord(PutWord2x);
	}
	if(scalerGL != NULL) {
		scalerGL->SetVramReader(VramReader_400l, 80, 400);
		scalerGL->SetPutWord(PutWordGL);
	}
}

void SetVramReader_4096(void)
{
	if(scaler2x2 != NULL) {
		scaler2x2->SetVramReader(VramReader_4096, 40, 200);
		scaler2x2->SetPutWord(PutWord2x2);
	}
	if(scaler2x2i != NULL) {
		scaler2x2i->SetVramReader(VramReader_4096, 40, 200);
		scaler2x2i->SetPutWord(PutWord2x2);
	}
	if(scaler4x4 != NULL) {
		scaler4x4->SetVramReader(VramReader_4096, 40, 200);
		scaler4x4->SetPutWord(PutWord4x);
	}
	if(scaler4x4i != NULL) {
		scaler4x4i->SetVramReader(VramReader_4096, 40, 200);
		scaler4x4i->SetPutWord(PutWord4x);
	}
	if(scalerGL != NULL) {
		scalerGL->SetVramReader(VramReader_4096, 40, 200);
		scalerGL->SetPutWord(PutWordGL);
	}
}

void SetVramReader_256k(void)
{
	if(scaler2x2 != NULL) {
		scaler2x2->SetVramReader(VramReader_256k, 40, 200);
		scaler2x2->SetPutWord(PutWord2x);
	}
	if(scaler2x2i != NULL) {
		scaler2x2i->SetVramReader(VramReader_256k, 40, 200);
		scaler2x2i->SetPutWord(PutWord2x);
	}
	if(scaler4x4 != NULL) {
		scaler4x4->SetVramReader(VramReader_256k, 40, 200);
		scaler4x4->SetPutWord(PutWord4x);
	}
	if(scaler4x4i != NULL) {
		scaler4x4i->SetVramReader(VramReader_256k, 40, 200);
		scaler4x4i->SetPutWord(PutWord4x);
	}
	if(scalerGL != NULL) {
		scalerGL->SetVramReader(VramReader_256k, 40, 200);
		scalerGL->SetPutWord(PutWordGL);
	}
}




void init_scaler(void)
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
	if(scaler4x4 == NULL) {
		scaler4x4 = new EmuGrphScale4x4;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler4x4->SetVramReader(VramReader, 40, 200);
		scaler4x4->SetPutWord(PutWord4x);
	}
	if(scaler4x4i == NULL) {
		scaler4x4i = new EmuGrphScale4x4i;
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scaler4x4i->SetVramReader(VramReader, 40, 200);
		scaler4x4i->SetPutWord(PutWord4x);
	}
	if(scalerGL == NULL) {
#ifdef USE_AGAR
		scalerGL = new EmuAgarGL;
#else
		scalerGL = new EmuGLUtils;
#endif
		//		scaler1x2i->SetConvWord(&vramhdr->ConvWord);
		scalerGL->SetVramReader(VramReader, 80, 400);
		scalerGL->SetPutWord(PutWordGL8);
	}
}

void initsub_scaler()
{
	//	b256kFlag = FALSE;
	scaler1x1 = NULL;
	scaler1x2 = NULL;
	scaler1x2i = NULL;
	scaler2x2 = NULL;
	scaler2x2i = NULL;
	scaler2x4 = NULL;
	scaler2x4i = NULL;
	scaler4x4 = NULL;
	scaler4x4i = NULL;
	scalerGL = NULL;
}


void detachsub_scaler(void)
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

	if(scaler1x1 != NULL) {
		delete scaler1x1;
		scaler1x1 = NULL;
	}
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

	if(scalerGL != NULL) {
		delete scalerGL;
		scalerGL = NULL;
	}

}

void SetupGL(int w, int h)
{
#ifdef USE_OPENGL
	SDL_SemWait(DrawInitSem);
	scalerGL->InitGL(w, h);
	SDL_SemPost(DrawInitSem);
#else
#ifdef USE_AGAR

#else
	SDL_SetVideoMode(w, h, 32, SDL_OPENGL | SDL_RESIZABLE);
#endif
#endif
}


void InitGL(int w, int h)
{
#if 0
        int rgb_size[4];
        int flags;
        BOOL accel = TRUE;
        BOOL sync = TRUE;

    if(SDL_WasInit(SDL_INIT_VIDEO) == 0) return;
    //if(!bUseOpenGL) return;
    SDL_SemWait(InitSem);
    rgb_size[0] = 8;
    rgb_size[1] = 8;
    rgb_size[2] = 8;
	flags = SDL_OPENGL | SDL_RESIZABLE;

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
//     if ( fsaa ) {
//              SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
//              SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, fsaa );
//      }
     if ( accel ) {
             SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
     }
     if ( sync ) {
             SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
     } else {
             SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 0 );
     }
// 	if(SDL_SetVideoMode(w, h, 32, flags) == 0)
// 	{
// 		return ;
// 	}

     printf("Screen BPP: %d\n", SDL_GetVideoSurface()->format->BitsPerPixel);
     printf("\n");
     printf( "Vendor     : %s\n", glGetString( GL_VENDOR ) );
     printf( "Renderer   : %s\n", glGetString( GL_RENDERER ) );
     printf( "Version    : %s\n", glGetString( GL_VERSION ) );
     printf( "Extensions : %s\n", glGetString( GL_EXTENSIONS ) );
     printf("\n");
   if(scalerGL) {
    	scalerGL->InitGL(w, h);
    }
    SDL_SemPost(InitSem);

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
		if(scalerGL != NULL) {
			scalerGL->Flip();
		} else {
			SDL_Flip(p);
		}
	}
	SDL_SemPost(DrawInitSem);
}


void Scaler_1x2(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler1x2->PutVram(p, x, y, w, h, mpage);
}

void Scaler_1x2i(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler1x2i->PutVram(p, x, y, w, h, mpage);
}

void Scaler_2x4(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler2x4->PutVram(p, x, y, w, h, mpage);
}
void Scaler_2x4i(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scaler2x4i->PutVram(p, x, y, w, h, mpage);
}

void Scaler_GL(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	scalerGL->SetViewPort(0,0,p->w, p->h);
//	scalerGL->SetViewPort();
        if(!bFullScan) {
	   scalerGL->SetScanLine(TRUE);
	} else {
	   scalerGL->SetScanLine(FALSE);
	}
	scalerGL->PutVram(p, x, y, w, h, mpage);
}


void Scaler_1x1(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip)
{
	if(scaler1x1 == NULL) return;
	scaler1x1->PutVram(p, x, y, w, h, multip );
}

void Scaler_2x2(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip)
{
	if(scaler2x2 == NULL) return;
	scaler2x2->PutVram(p, x, y, w, h, multip );
}

void Scaler_2x2i(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip)
{
	if(scaler2x2i == NULL) return;
	scaler2x2i->PutVram(p, x, y, w, h, multip );
}
void Scaler_4x4(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip)
{
	if(scaler4x4 == NULL) return;
	scaler4x4->PutVram(p, x, y, w, h, multip );
}

void Scaler_4x4i(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip)
{
	if(scaler4x4 == NULL) return;
	scaler4x4i->PutVram(p, x, y, w, h, multip );
}


#ifdef __cplusplus
}
#endif


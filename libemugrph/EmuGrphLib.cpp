/*
 * EmuGrphLib.cpp
 * 8色描画クラス(一応XM7用)
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#include "EmuGrphLib.h"

EmuGrphLib::EmuGrphLib() {
	// TODO Auto-generated constructor stub
	vram_pr = NULL;
	vram_pg = NULL;
	vram_pb = NULL;

	vram_w = 0;
	vram_h = 0;
	palette = NULL;
}

EmuGrphLib::~EmuGrphLib() {
	// TODO Auto-generated destructor stub
}



/*
 * パレット値を計算する
 */
void EmuGrphLib::CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Surface *disp)
{
	Uint32 ds;

	if(palette == NULL) return;
	if(disp == NULL) return;

	ds = ((r << disp->format->Rshift) & disp->format->Rmask) |
			((g << disp->format->Gshift) & disp->format->Gmask) |
			((b << disp->format->Bshift) & disp->format->Bmask) |
			((a << disp->format->Ashift) & disp->format->Amask);
	palette[src] = ds;
}
/*
 * RGBA固定の場合(OpenGLとか)
 */
void EmuGrphLib::CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint32 ds;

	if(palette == NULL) return;
#if 0
	ds = a<<24 + r<<16 +
#endif

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	ds =(r<<8) + (g<<16) + (b<<0);
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
//			(a << 0);
	palette[src] = ds;
}


void EmuGrphLib::ConvWord(SDL_Surface *p, Uint32 *dst, Uint32 *src)
{
	int i;
	for(i = 0; i < 8; i++) {
		dst[i] = (((src[i] & 0xff0000) >> 16)<< p->format->Rshift) & p->format->Rmask;
		dst[i] |= (((src[i] & 0x00ff00) >> 8)<< p->format->Gshift) & p->format->Gmask;
		dst[i] |= ((src[i] & 0x0000ff)<< p->format->Bshift) & p->format->Bmask;
		dst[i] |= (0xff << p->format->Ashift ) & p->format->Amask;
	}
}


void EmuGrphLib::InitPalette(void)
{
	SDL_Surface *p;
	Uint8 r,g,b,a;

	a = 255;
	p = SDL_GetVideoSurface();
	if(p == NULL) return; // これでいいのか？
	for(r = 0; r < 2 ; r++){
		for(g = 0; g < 2 ; g++) {
			for(b = 0; b < 2; b++) {
				CalcPalette(b + (r<<1) + (g<<2) , (r<<7), (g<<7), (b<<7), a, p );
			}
		}
	}
}


/*
 * パレットテーブルを設定する
 */
void EmuGrphLib::SetPaletteTable(Uint32 *p)
{
	palette = p;
}

/*
 * vramアドレスを設定する
 */
void EmuGrphLib::SetVram(Uint8 *p, Uint32 w, Uint32 h)
{
	vram_pb = p + 0;
	vram_pg = p + 0x10000;
	vram_pr = p + 0x8000;

	vram_w = w;
	vram_h = h;

}

void EmuGrphLib::SetVram(Uint8 *pr, Uint8 *pg, Uint8 *pb, Uint32 w, Uint32 h)
{
	vram_pb = pb;
	vram_pg = pg;
	vram_pr = pr;

	vram_w = w;
	vram_h = h;
}
/*
 * サンプル/等倍
 */
void EmuGrphLib::GetVram(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
	GetVram(addr, cbuf);
}


void EmuGrphLib::GetVram(Uint32 addr, Uint32 *cbuf)
{
   Uint8    cb,
            cr,
            cg;

        cb = vram_pb[addr];
        cr = vram_pr[addr];
        cg = vram_pg[addr];
        cbuf[0] =   palette[(cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2)];
        cbuf[1] =   palette[((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1)];
        cbuf[2] =   palette[((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04)];
        cbuf[3] =   palette[((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +((cg & 0x08) >> 1)];
        cbuf[4] =   palette[((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) + ((cg & 0x10) >> 2)];
        cbuf[5] =   palette[((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) + ((cg & 0x20) >> 3)];
        cbuf[6] =   palette[((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) + ((cg & 0x40) >> 4)];
        cbuf[7] =   palette[((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) + ((cg & 0x80) >> 5)];

}

static inline void
__SETDOT(Uint8 * addr, Uint32 c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
#endif
}


void EmuGrphLib::PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *c)
{
	Uint8 *d = (Uint8 *)disp;
	__SETDOT(d, c[7]);
	d += pixsize;
	__SETDOT(d, c[6]);
	d += pixsize;
	__SETDOT(d, c[5]);
	d += pixsize;
	__SETDOT(d, c[4]);
	d += pixsize;
	__SETDOT(d, c[3]);
	d += pixsize;
	__SETDOT(d, c[2]);
	d += pixsize;
	__SETDOT(d, c[1]);
	d += pixsize;
	__SETDOT(d, c[0]);
}

/*
 * 以下、サンプルとして3bpp画面,640x200->640x400
 */
void EmuGrphLib::PutVram(BOOL interlace)
{
	Uint32 cbuf[8];
	SDL_Surface *p;
	Uint32 x;
	Uint32 y;
	int ofset;
	Uint32 *disp;
	Uint32 pixsize;
	Uint32 w;
	Uint32 h;
	Uint32 nullline; // インタレース縞用
	SDL_Rect rect;
	if(palette == NULL) return;

	p = SDL_GetVideoSurface();
	if(p == NULL) return;
	SDL_LockSurface(p);
	pixsize = p->format->BytesPerPixel;
	w = p->w;
	h = p->h;
	nullline = SDL_MapRGBA(p->format, 0 , 0, 0 , p->format->alpha );
	for(y = 0; y < vram_h; y++) {
		if(y >= h) break;
		ofset = y * vram_w;
		disp =(Uint32 *)((Uint8 *)p->pixels + (y * 2)* p->pitch);
		for(x = 0; x < vram_w ; x++) {
			if((x<<3) >= w) break;
			GetVram(ofset , cbuf);
			PutWord(disp, pixsize, cbuf);
			if(!interlace){
				PutWord(disp + p->pitch, pixsize, cbuf);
			}
			disp = disp + (pixsize << 3); //8バイト分
			ofset++;
		}
		//下段にインタレース縞
		if(interlace) {
			rect.h = 1;
			rect.w = w;
			rect.x = 0;
			rect.y = y*2 + 1;
			SDL_FillRect(p, &rect, nullline);
		}
	}
	SDL_UnlockSurface(p);
}

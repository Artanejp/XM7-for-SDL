/*
 * EmuGrph4096c.cpp
 *
 * 4096色描画クラス(一応XM7用)
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#include "EmuGrph4096c.h"

EmuGrph4096c::EmuGrph4096c() {
	// TODO Auto-generated constructor stub

}

EmuGrph4096c::~EmuGrph4096c() {
	// TODO Auto-generated destructor stub
}

void EmuGrph4096c::CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Surface *disp)
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

void EmuGrph4096c::InitPalette(void)
{
	SDL_Surface *p;
	Uint8 r,g,b,a;

	p = SDL_GetVideoSurface();
	a = 255;
	if(p == NULL) return; // これでいいのか？
	for(r = 0; r < 16 ; r++){
		for(g = 0; g < 16 ; g++) {
			for(b = 0; b < 16; b++) {
				CalcPalette(b + (r<<4) +(g<<8) , (r<<4), (g<<4), (b<<4), a,p );
			}
		}
	}
}

void EmuGrph4096c::GetVram(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
	GetVram(addr, cbuf);
}

void EmuGrph4096c::SetPaletteTable(Uint32 *p)
{
	palette = p;
}
void EmuGrph4096c::GetVram(Uint32 addr, Uint32 *cbuf)
{
        Uint8            b[4],
                        r[4],
                        g[4];
        Uint32 			dat[8];

        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上を考慮して、
         * インライン展開と細かいループの廃止を同時に行う
         */

        g[3] = vram_pg[addr + 0x00000];
        g[2] = vram_pg[addr + 0x02000];
        g[1] = vram_pg[addr + 0x04000];
        g[0] = vram_pg[addr + 0x06000];

        r[3] = vram_pr[addr + 0x00000];
        r[2] = vram_pr[addr + 0x02000];
        r[1] = vram_pr[addr + 0x04000];
        r[0] = vram_pr[addr + 0x06000];

        b[3] = vram_pb[addr + 0x00000];
        b[2] = vram_pb[addr + 0x02000];
        b[1] = vram_pb[addr + 0x04000];
        b[0] = vram_pb[addr + 0x06000];


        /*
         * bit7
         */
        dat[0] =
    	((b[0] & 0x01)) + ((b[1] & 0x01) << 1) + ((b[2] & 0x01) << 2) +
    	((b[3] & 0x01) << 3)
    	+ ((r[0] & 0x01) << 4) + ((r[1] & 0x01) << 5) +
    	((r[2] & 0x01) << 6) + ((r[3] & 0x01) << 7)
    	+ ((g[0] & 0x01) << 8) + ((g[1] & 0x01) << 9) +
    	((g[2] & 0x01) << 10) + ((g[3] & 0x01) << 11);
        cbuf[0] = palette[dat[0]];

        /*
         * bit6
         */
        dat[1] =
    	((b[0] & 0x02) >> 1) + ((b[1] & 0x02)) + ((b[2] & 0x02) << 1) +
    	((b[3] & 0x02) << 2)
    	+ ((r[0] & 0x02) << 3) + ((r[1] & 0x02) << 4) +
    	((r[2] & 0x02) << 5) + ((r[3] & 0x02) << 6)
    	+ ((g[0] & 0x02) << 7) + ((g[1] & 0x02) << 8) +
    	((g[2] & 0x02) << 9) + ((g[3] & 0x02) << 10);
        cbuf[1] = palette[dat[1]];

        /*
         * bit5
         */
        dat[2] =
    	((b[0] & 0x04) >> 2) + ((b[1] & 0x04) >> 1) + ((b[2] & 0x04)) +
    	((b[3] & 0x04) << 1)
    	+ ((r[0] & 0x04) << 2) + ((r[1] & 0x04) << 3) +
    	((r[2] & 0x04) << 4) + ((r[3] & 0x04) << 5)
    	+ ((g[0] & 0x04) << 6) + ((g[1] & 0x04) << 7) +
    	((g[2] & 0x04) << 8) + ((g[3] & 0x04) << 9);
        cbuf[2] = palette[dat[2]];

        /*
         * bit4
         */
        dat[3] =
    	((b[0] & 0x08) >> 3) + ((b[1] & 0x08) >> 2) +
    	((b[2] & 0x08) >> 1) + ((b[3] & 0x08))
    	+ ((r[0] & 0x08) << 1) + ((r[1] & 0x08) << 2) +
    	((r[2] & 0x08) << 3) + ((r[3] & 0x08) << 4)
    	+ ((g[0] & 0x08) << 5) + ((g[1] & 0x08) << 6) +
    	((g[2] & 0x08) << 7) + ((g[3] & 0x08) << 8);
        cbuf[3] = palette[dat[3]];

        /*
         * bit3
         */
        dat[4] =
    	((b[0] & 0x10) >> 4) + ((b[1] & 0x10) >> 3) +
    	((b[2] & 0x10) >> 2) + ((b[3] & 0x10) >> 1)
    	+ ((r[0] & 0x10)) + ((r[1] & 0x10) << 1) + ((r[2] & 0x10) << 2) +
    	((r[3] & 0x10) << 3)
    	+ ((g[0] & 0x10) << 4) + ((g[1] & 0x10) << 5) +
    	((g[2] & 0x10) << 6) + ((g[3] & 0x10) << 7);
        cbuf[4] = palette[dat[4]];

        /*
         * bit2
         */
        dat[5] =
    	((b[0] & 0x20) >> 5) + ((b[1] & 0x20) >> 4) +
    	((b[2] & 0x20) >> 3) + ((b[3] & 0x20) >> 2)
    	+ ((r[0] & 0x20) >> 1) + ((r[1] & 0x20)) + ((r[2] & 0x20) << 1) +
    	((r[3] & 0x20) << 2)
    	+ ((g[0] & 0x20) << 3) + ((g[1] & 0x20) << 4) +
    	((g[2] & 0x20) << 5) + ((g[3] & 0x20) << 6);
        cbuf[5] = palette[dat[5]];

        /*
         * bit1
         */
        dat[6] =
    	((b[0] & 0x40) >> 6) + ((b[1] & 0x40) >> 5) +
    	((b[2] & 0x40) >> 4) + ((b[3] & 0x40) >> 3)
    	+ ((r[0] & 0x40) >> 2) + ((r[1] & 0x40) >> 1) + ((r[2] & 0x40)) +
    	((r[3] & 0x40) << 1)
    	+ ((g[0] & 0x40) << 2) + ((g[1] & 0x40) << 3) +
    	((g[2] & 0x40) << 4) + ((g[3] & 0x40) << 5);
        cbuf[6] = palette[dat[6]];

        /*
         * bit0
         */
        dat[7] =
    	((b[0] & 0x80) >> 7) + ((b[1] & 0x80) >> 6) +
    	((b[2] & 0x80) >> 5) + ((b[3] & 0x80) >> 4)
    	+ ((r[0] & 0x80) >> 3) + ((r[1] & 0x80) >> 2) +
    	((r[2] & 0x80) >> 1) + ((r[3] & 0x80))
    	+ ((g[0] & 0x80) << 1) + ((g[1] & 0x80) << 2) +
    	((g[2] & 0x80) << 3) + ((g[3] & 0x80) << 4);
        cbuf[7] = palette[dat[7]];

}

static inline void
__SETDOT(Uint8 * addr, Uint32 c, Uint32 pixels)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */

    addr += pixels;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */

 #endif
}

static inline void
__SETDOTx2(Uint8 * addr, Uint32 c, Uint32 pixels)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */

    addr += pixels;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */

    addr += pixels;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */

    addr += pixels;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
#endif
}

void EmuGrph4096c::PutWordx2(Uint32 *disp, Uint32 pixsize, Uint32 *c)
{
	Uint8 *d = (Uint8 *)disp;
	__SETDOTx2(d, c[0], pixsize);
	d += pixsize * 2;

	__SETDOTx2(d, c[1], pixsize);
	d += pixsize * 2;

	__SETDOTx2(d, c[2], pixsize);
	d += pixsize * 2;

	__SETDOTx2(d, c[3], pixsize);
	d += pixsize * 2;

	__SETDOTx2(d, c[4], pixsize);
	d += pixsize * 2;

	__SETDOTx2(d, c[5], pixsize);
	d += pixsize * 2;

	__SETDOTx2(d, c[6], pixsize);
	d += pixsize * 2;

	__SETDOTx2(d, c[7], pixsize);

}

void EmuGrph4096c::PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *c)
{
	Uint8 *d = (Uint8 *)disp;
	__SETDOT(d, c[7], pixsize);
	d += pixsize * 2;

	__SETDOT(d, c[6], pixsize);
	d += pixsize * 2;

	__SETDOT(d, c[5], pixsize);
	d += pixsize * 2;

	__SETDOT(d, c[4], pixsize);
	d += pixsize * 2;

	__SETDOT(d, c[3], pixsize);
	d += pixsize * 2;

	__SETDOT(d, c[2], pixsize);
	d += pixsize * 2;

	__SETDOT(d, c[1], pixsize);
	d += pixsize * 2;

	__SETDOT(d, c[0], pixsize);

}



void EmuGrph4096c::PutVram(BOOL interlace)
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
			if((x<<4) >= w) break;
			GetVram(ofset , cbuf);
			PutWord(disp, pixsize, cbuf);
			if(!interlace){
				PutWord(disp + p->pitch, pixsize, cbuf);
			}
			disp = disp + (pixsize << 4); //8*2バイト分
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

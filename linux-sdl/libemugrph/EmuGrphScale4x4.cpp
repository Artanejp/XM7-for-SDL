/*
 * EmuGrphScale4x4.cpp
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#include "EmuGrphScale4x4.h"

EmuGrphScale4x4::EmuGrphScale4x4() {
	// TODO Auto-generated constructor stub
	convword = NULL;
}

EmuGrphScale4x4::~EmuGrphScale4x4() {
	// TODO Auto-generated destructor stub
}

void EmuGrphScale4x4::SetVramReader(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
	getvram = p;
	vramwidth = w;
	vramheight = h;
}

void EmuGrphScale4x4::SetPutWord(void p(Uint32 *, Uint32, Uint32 *))
{
	putword = p;
}

void EmuGrphScale4x4::SetConvWord(void p(SDL_Surface *, Uint32 *, Uint32 *))
{
	convword = p;
}

static inline void
__SETDOT(Uint8 * addr, Uint32 c,Uint32 pixels)
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

static void PutWord4x(Uint32 *disp,Uint32 pixels, Uint32 *cbuf)
{
	Uint8 *d =(Uint8 *)disp;
	__SETDOT(d, cbuf[0], pixels);
	d+=pixels*4;
	__SETDOT(d, cbuf[1], pixels);
	d+=pixels*4;
	__SETDOT(d, cbuf[2], pixels);
	d+=pixels*4;
	__SETDOT(d, cbuf[3], pixels);
	d+=pixels*4;
	__SETDOT(d, cbuf[4], pixels);
	d+=pixels*4;
	__SETDOT(d, cbuf[5], pixels);
	d+=pixels*4;
	__SETDOT(d, cbuf[6], pixels);
	d+=pixels*4;
	__SETDOT(d, cbuf[7], pixels);
	d+=pixels*4;
}
/*
 * 2x4, Not Interlaced.
 */
void EmuGrphScale4x4::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	Uint32 wbuf[8];
	Uint32 wbuf2[8];
	int xx,yy;
	int ww, hh;
	Uint32 addr;
	Uint32 *disp;
	Uint32 *disp2;
	Uint32 *disp3;
	Uint32 *disp4;

	ww = w + x;
	if(ww > (vramwidth * 8)) ww = vramwidth * 8;
	hh = h + y;
	if(hh > vramheight) hh = vramheight;

	if(p == NULL) return;
	if(putword == NULL) return;
	if(getvram == NULL) return;
	SDL_LockSurface(p);
	for(yy = y; yy < hh ; yy++) {
		if(yy >= vramheight) break;
		addr = yy * vramwidth + x / 8;
		if(convword != NULL) {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 4 *p->pitch + xx * p->format->BytesPerPixel * 32);
				disp2 = (Uint32 *)((Uint8 *)disp + p->pitch);
				disp3 = (Uint32 *)((Uint8 *)disp2 + p->pitch);
				disp4 = (Uint32 *)((Uint8 *)disp3 + p->pitch);
				getvram(addr, wbuf, mpage);
//				convword(p, wbuf2, wbuf);
				putword(disp, p->format->BytesPerPixel, wbuf2);
				putword(disp2, p->format->BytesPerPixel, wbuf2);
				putword(disp3, p->format->BytesPerPixel, wbuf2);
				putword(disp4, p->format->BytesPerPixel, wbuf2);
				addr++;
			}
		} else {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 4 *p->pitch + xx * p->format->BytesPerPixel * 32);
				disp2 = (Uint32 *)((Uint8 *)disp + p->pitch);
				disp3 = (Uint32 *)((Uint8 *)disp2 + p->pitch);
				disp4 = (Uint32 *)((Uint8 *)disp3 + p->pitch);
				getvram(addr, wbuf, mpage);
				putword(disp, p->format->BytesPerPixel, wbuf);
				putword(disp2, p->format->BytesPerPixel, wbuf);
				putword(disp3, p->format->BytesPerPixel, wbuf);
				putword(disp4, p->format->BytesPerPixel, wbuf);
				addr++;
			}
		}
	}
	SDL_UnlockSurface(p);
	SDL_UpdateRect(p, 0, 0, p->w, p->h);
//	SDL_Flip(p);
}

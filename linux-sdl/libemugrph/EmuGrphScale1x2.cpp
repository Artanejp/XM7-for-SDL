/*
 * EmuGrphScale1x2.cpp
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#include "EmuGrphScale1x2.h"

EmuGrphScale1x2::EmuGrphScale1x2() {
	// TODO Auto-generated constructor stub
	convword = NULL;
	putword = NULL;
	getvram = NULL;
}

EmuGrphScale1x2::~EmuGrphScale1x2() {
	// TODO Auto-generated destructor stub
}

void EmuGrphScale1x2::SetVramReader(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
	getvram = p;
	vramwidth = w;
	vramheight = h;
}

void EmuGrphScale1x2::SetPutWord(void p(Uint32 *, Uint32, Uint32 *))
{
	putword = p;
}

void EmuGrphScale1x2::SetConvWord(void p(SDL_Surface *, Uint32 *, Uint32 *))
{
	convword = p;
}

/*
 * 1x2, Not Interlaced.
 */
void EmuGrphScale1x2::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	Uint32 wbuf[8];
	Uint32 wbuf2[8];
	int xx,yy;
	Uint32 addr;
	Uint32 *disp;
	Uint32 *disp2;
	int ww, hh;

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
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 2 *p->pitch + xx * p->format->BytesPerPixel * 8);
				disp2 = (Uint32 *)((Uint8 *)disp + p->pitch);
				getvram(addr, wbuf, mpage);
				convword(p, wbuf2, wbuf);
				putword(disp, p->format->BytesPerPixel, wbuf2);
				putword(disp2, p->format->BytesPerPixel, wbuf2);
				addr++;
			}
		} else {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 2 *p->pitch + xx * p->format->BytesPerPixel * 8);
				disp2 = (Uint32 *)((Uint8 *)disp + p->pitch);
				getvram(addr, wbuf, mpage);
				putword(disp, p->format->BytesPerPixel, wbuf);
				putword(disp2, p->format->BytesPerPixel, wbuf);
				addr++;
			}
		}
	}
	SDL_UnlockSurface(p);
	SDL_UpdateRect(p, 0, 0, p->w, p->h);
//	SDL_Flip(p);
}

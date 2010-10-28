/*
 * EmuGrphScale2x2i.cpp
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#include "EmuGrphScale2x2i.h"

EmuGrphScale2x2i::EmuGrphScale2x2i() {
	// TODO Auto-generated constructor stub
	convword = NULL;
}

EmuGrphScale2x2i::~EmuGrphScale2x2i() {
	// TODO Auto-generated destructor stub
}

void EmuGrphScale2x2i::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	Uint32 wbuf[8];
	Uint32 wbuf2[8];
	int xx,yy;
	int ww,hh;
	Uint32 addr;
	Uint32 *disp;
	SDL_Rect rect;
	Uint32 nullline;

	ww = w + x;
	if(ww > (vramwidth *8)) ww = vramwidth*8;
	hh = h + y;
	if(hh > vramheight) hh = vramheight;

	if(p == NULL) return;
	if(putword == NULL) return;
	if(getvram == NULL) return;
	nullline = SDL_MapRGBA(p->format, 0, 0, 0, 255);
	SDL_LockSurface(p);
	for(yy = y; yy < hh ; yy++) {
		if(yy >= vramheight) break;
		addr = yy * vramwidth + x / 8;
		if(convword != NULL) {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((void *)p->pixels + yy * 2 *p->pitch + xx * p->format->BytesPerPixel * 16);
				getvram(addr, wbuf, mpage);
				convword(p, wbuf2, wbuf);
				putword(disp, p->format->BytesPerPixel, wbuf2);
				addr++;
			}
		} else {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((void *)p->pixels + yy * 2 *p->pitch + xx * p->format->BytesPerPixel * 16);
				getvram(addr, wbuf, mpage);
				putword(disp, p->format->BytesPerPixel, wbuf);
				addr++;
			}
		}
		rect.x = x;
		rect.y = yy * 2+ 1;
		rect.h = 1;
		rect.w = w;
		SDL_FillRect(p, &rect, nullline);
	}
	SDL_UnlockSurface(p);
	SDL_UpdateRect(p, 0, 0, p->w, p->h);
//	SDL_Flip(p);
}

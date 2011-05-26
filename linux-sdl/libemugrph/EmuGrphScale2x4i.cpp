/*
 * EmuGrphScale2x4i.cpp
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#include "EmuGrphScale2x4i.h"

EmuGrphScale2x4i::EmuGrphScale2x4i() {
	// TODO Auto-generated constructor stub

}

EmuGrphScale2x4i::~EmuGrphScale2x4i() {
	// TODO Auto-generated destructor stub
}

/*
 * 2x4, Not Interlaced.
 */
void EmuGrphScale2x4i::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	Uint32 wbuf[8];
	Uint32 wbuf2[8];
	int xx,yy;
	int ww,hh;
	Uint32 addr;
	Uint32 *disp;
	Uint32 *disp2;
	SDL_Rect rect;
	Uint32 nullline;


	if(p == NULL) return;
	if(putword == NULL) return;
	if(getvram == NULL) return;

	nullline = SDL_MapRGBA(p->format, 0, 0, 0, 255);
	ww = w + x;
	if(ww > (vramwidth * 8)) ww = vramwidth * 8;
	hh = h + y;
	if(hh > vramheight) hh = vramheight;
	SDL_LockSurface(p);
	for(yy = y; yy < hh ; yy++) {
		if(yy >= vramheight) break;
		addr = yy * vramwidth + x / 8;
		if(convword != NULL) {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 4 *p->pitch + xx * p->format->BytesPerPixel * 16);
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
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 4 *p->pitch + xx * p->format->BytesPerPixel * 16);
				disp2 = (Uint32 *)((Uint8 *)disp + p->pitch);
				getvram(addr, wbuf, mpage);
				putword(disp, p->format->BytesPerPixel, wbuf);
				putword(disp2, p->format->BytesPerPixel, wbuf);
				addr++;
			}
		}
		rect.x = 0;
		rect.y = yy * 4 + 2;
		rect.w = w;
		rect.h = 2;
		SDL_FillRect(p, &rect, nullline);
	}
	SDL_UnlockSurface(p);
	SDL_UpdateRect(p, 0, 0, p->w, p->h);
//	SDL_Flip(p);
}

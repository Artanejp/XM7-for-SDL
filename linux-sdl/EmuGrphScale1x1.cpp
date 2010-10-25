/*
 * EmuGrphScale1x1.cpp
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#include "EmuGrphScale1x1.h"

EmuGrphScale1x1::EmuGrphScale1x1() {
	// TODO Auto-generated constructor stub
	convword = NULL;

}

EmuGrphScale1x1::~EmuGrphScale1x1() {
	// TODO Auto-generated destructor stub
}
/*
 * 1x1, Not Interlaced.
 */
void EmuGrphScale1x1::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	Uint32 wbuf[8];
	Uint32 wbuf2[8];
	int xx,yy;
	Uint32 addr;
	Uint32 *disp;
	Uint32 *disp2;


	if(p == NULL) return;
	if(putword == NULL) return;
	if(getvram == NULL) return;
	SDL_LockSurface(p);
	for(yy = y; yy < h ; yy++) {
		addr = yy * w / 8;
		if(convword != NULL) {
			for (xx = x / 8; xx < w / 8; xx++) {
				disp = (Uint32 *)((void *)p->pixels + yy * p->pitch + xx * p->format->BytesPerPixel * 8);
				getvram(addr, wbuf, mpage);
				convword(p, wbuf2, wbuf);
				putword(disp, p->format->BytesPerPixel, wbuf2);
				addr++;
			}
		} else {
			for (xx = x / 8; xx < w / 8; xx++) {
				disp = (Uint32 *)((void *)p->pixels + yy * p->pitch + xx * p->format->BytesPerPixel * 8);
				getvram(addr, wbuf, mpage);
				putword(disp, p->format->BytesPerPixel, wbuf);
				addr++;
			}
		}
	}
	SDL_UnlockSurface(p);
	SDL_UpdateRect(p, 0, 0, p->w, p->h);
	//SDL_Flip(p);
}

/*
 * EmuGrphScale1x2i.cpp
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#include "EmuGrphScale1x2i.h"

EmuGrphScale1x2i::EmuGrphScale1x2i() {
	// TODO Auto-generated constructor stub
	convword = NULL;
	putword = NULL;
	getvram = NULL;
}

EmuGrphScale1x2i::~EmuGrphScale1x2i() {
	// TODO Auto-generated destructor stub
}


void EmuGrphScale1x2i::SetVramReader(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
	getvram = p;
	vramwidth = w;
	vramheight = h;
}

void EmuGrphScale1x2i::SetPutWord(void p(Uint32 *, Uint32, Uint32 *))
{
	putword = p;
}

void EmuGrphScale1x2i::SetConvWord(void p(SDL_Surface *, Uint32 *, Uint32 *))
{
	convword = p;
}

/*
 * 1x2, Interlaced.
 */
void EmuGrphScale1x2i::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	Uint32 wbuf[8];
	Uint32 wbuf2[8];
	SDL_Rect rect;
	Uint32 nullline;
	int xx,yy;
	Uint32 addr;
	Uint32 *disp;
	int	 ww, hh;

	ww = w + x;
	if(ww > (vramwidth * 8)) ww = vramwidth * 8;
	hh = h + y;
	if(hh > vramheight) hh = vramheight;

	if(p == NULL) return;
	if(putword == NULL) return;
	if(getvram == NULL) return;
	SDL_LockSurface(p);
#if SDL_VERSION_ATLEAST(1,3,0)
      nullline = SDL_MapRGBA(p->format, 0 , 0, 0 , 0 );
#else
   nullline = SDL_MapRGBA(p->format, 0 , 0, 0 , p->format->alpha );
#endif
	for(yy = y; yy < hh ; yy++) {
		if(yy >= vramheight) break;
		addr = yy * vramwidth + x / 8;
		if(convword != NULL) {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 2 * p->pitch + xx * 8 * p->format->BytesPerPixel);
				getvram(addr, wbuf, mpage);
				convword(p, wbuf2, wbuf);
				putword(disp, p->format->BytesPerPixel, wbuf2);
				addr++;
			}
		} else {
			for (xx = x / 8; xx < ww / 8; xx++) {
				if(xx >= vramwidth) break;
				disp = (Uint32 *)((Uint8 *)p->pixels + yy * 2 * p->pitch + xx * 8 * p->format->BytesPerPixel);
				getvram(addr, wbuf, mpage);
				putword(disp, p->format->BytesPerPixel, wbuf);
				disp = disp + 8 * p->format->BytesPerPixel;
				addr++;
			}
		}
		rect.h = 1;
		rect.w = w;
		rect.x = 0;
		rect.y = yy * 2 + 1;
		SDL_FillRect(p, &rect, nullline);
	}
	SDL_UnlockSurface(p);
	SDL_UpdateRect(p, 0, 0, p->w, p->h);

}

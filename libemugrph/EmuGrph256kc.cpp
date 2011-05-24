/*
 * EmuGrph256kc.cpp
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#include "EmuGrph256kc.h"

EmuGrph256kc::EmuGrph256kc() {
	// TODO Auto-generated constructor stub

}

EmuGrph256kc::~EmuGrph256kc() {
	// TODO Auto-generated destructor stub
}

void EmuGrph256kc::InitPalette(void)
{
	return;
}

void EmuGrph256kc::CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Surface *disp)
{
	return;
}

void EmuGrph256kc::SetVram(Uint8 *p, Uint32 w, Uint32 h)
{
	vram_p = p;
	vram_w = w;
	vram_h = h;
}

void EmuGrph256kc::SetVram(Uint8 *pr, Uint8 *pg, Uint8 *pb, Uint32 w, Uint32 h)
{
	vram_p = pb;
	vram_w = w;
	vram_h = h;
}

void EmuGrph256kc::GetVram(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
    Uint8           b[6],
                    r[6],
                    g[6];
    Uint32 	rr, gg, bb, aa;
    int i;

    /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う
     */

    cbuf[0] = 0x00000000;
    cbuf[1] = 0x00000000;
    cbuf[2] = 0x00000000;
    cbuf[3] = 0x00000000;
    cbuf[4] = 0x00000000;
    cbuf[5] = 0x00000000;
    cbuf[6] = 0x00000000;
    cbuf[7] = 0x00000000;

    if(!(mpage & 0x40)){
    	g[5] = vram_p[addr + 0x10000];
    	g[4] = vram_p[addr + 0x12000];
    	g[3] = vram_p[addr + 0x14000];
    	g[2] = vram_p[addr + 0x16000];
    	g[1] = vram_p[addr + 0x28000];
    	g[0] = vram_p[addr + 0x2a000];
    }
    if(!(mpage & 0x20)){
    	r[5] = vram_p[addr + 0x08000];
    	r[4] = vram_p[addr + 0x0a000];
    	r[3] = vram_p[addr + 0x0c000];
    	r[2] = vram_p[addr + 0x0e000];
    	r[1] = vram_p[addr + 0x28000];
    	r[0] = vram_p[addr + 0x2a000];
    }
    if(!(mpage & 0x10)){
    	b[5] = vram_p[addr + 0x00000];
    	b[4] = vram_p[addr + 0x02000];
    	b[3] = vram_p[addr + 0x04000];
    	b[2] = vram_p[addr + 0x06000];
    	b[1] = vram_p[addr + 0x18000];
    	b[0] = vram_p[addr + 0x1a000];
    }

 for(i = 7; i >= 0;i--){
	 rr = 0;
	 gg = 0;
	 bb = 0;
	 aa = 255;
   if(!(mpage & 0x20)){
	   rr= (r[0] & 0x80)>>5 | (r[1] & 0x80)>>4 | (r[2] & 0x80)>>3 | (r[3] & 0x80)>>2 | (r[4] & 0x80)>>1 | (r[5] & 0x80);
	    r[0]<<=1;
	    r[1]<<=1;
	    r[2]<<=1;
	    r[3]<<=1;
	    r[4]<<=1;
	    r[5]<<=1;
	    if((rr & 0x0000f8)!=0) rr |= 0x000003;
   }
   if(!(mpage & 0x40)){
	   gg = (g[0] & 0x80)>>5 | (g[1] & 0x80)>>4 | (g[2] & 0x80)>>3 | (g[3] & 0x80)>>2 | (g[4] & 0x80)>>1 | (g[5] & 0x80);
	    g[0]<<=1;
	    g[1]<<=1;
	    g[2]<<=1;
	    g[3]<<=1;
	    g[4]<<=1;
	    g[5]<<=1;
	    if((gg & 0x0000f8)!=0) gg |= 0x000003;
   }
   if(!(mpage & 0x10)){
	   bb = (b[0] & 0x80)>>5 | (b[1] & 0x80)>>4 | (b[2] & 0x80)>>3 | (b[3] & 0x80)>>2 | (b[4] & 0x80)>>1 | (b[5] & 0x80);
	    b[0]<<=1;
	    b[1]<<=1;
	    b[2]<<=1;
	    b[3]<<=1;
	    b[4]<<=1;
	    b[5]<<=1;
	    if((bb & 0x0000f8)!=0) bb |= 0x000003;
   }
	cbuf[i] =rr + (gg << 8)+ (bb << 16) + (aa<<24);
 }
}


void EmuGrph256kc::PutVram(BOOL interlace)
{
	Uint32 cbuf[8];
	Uint32 cbuf2[8];
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
#if SDL_VERSION_ATLEAST(1,3,0)
      nullline = SDL_MapRGBA(p->format, 0 , 0, 0 , 0 );
#else   
   nullline = SDL_MapRGBA(p->format, 0 , 0, 0 , p->format->alpha );
#endif
	for(y = 0; y < vram_h; y++) {
		if(y >= h) break;
		ofset = y * vram_w;
		disp =(Uint32 *)((Uint8 *)p->pixels + (y * 2)* p->pitch);
		for(x = 0; x < vram_w ; x++) {
			if((x<<4) >= w) break;
			GetVram(ofset , cbuf, 0);
			ConvWord(p, cbuf2, cbuf);
			PutWord(disp, pixsize, cbuf2);
			if(!interlace){
				PutWord(disp + p->pitch, pixsize, cbuf2);
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

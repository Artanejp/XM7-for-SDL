/*
 * api_vram256k.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
//#include "api_scaler.h"
#include "api_vram.h"


static inline void putword(Uint32 *disp, Uint32 *cbuf)
{
   
		disp[0] = cbuf[7];
		disp[1] = cbuf[6];
		disp[2] = cbuf[5];
		disp[3] = cbuf[4];
		disp[4] = cbuf[3];
		disp[5] = cbuf[2];
		disp[6] = cbuf[1];
		disp[7] = cbuf[0];
}

static inline Uint32 lshift_5bit1v(v4hi *v)
{
   Uint8 ret;
   ret =   (v->b[0] & 0x80)>>5 | (v->b[1] & 0x80)>>4 | (v->b[2] & 0x80)>>3 
	      | (v->b[3] & 0x80)>>2 | (v->b[4] & 0x80)>>1 | (v->b[5] & 0x80);
   if((ret & 0x0000f8)!=0) ret |= 0x000003;
   v->b[0] <<= 1;
   v->b[1] <<= 1;
   v->b[2] <<= 1;
   v->b[3] <<= 1;
   v->b[4] <<= 1;
   v->b[5] <<= 1;
   v->b[6] <<= 1;
   v->b[7] <<= 1;
   return ret;
}

static void gpixel2cbuf(Uint32 addr, Uint32 *b, Uint32 mpage)
{
   Uint8 ret = 0;
   v4hi v;
   v4hi *p;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x40)){
        v.b[5] = vram_p[addr + 0x10000]; 
        v.b[4] = vram_p[addr + 0x12000]; 
        v.b[3] = vram_p[addr + 0x14000]; 
        v.b[2] = vram_p[addr + 0x16000]; 
        v.b[1] = vram_p[addr + 0x28000]; 
        v.b[0] = vram_p[addr + 0x2a000]; 

        b[7] = lshift_5bit1v(&v) << 8;
        b[6] = lshift_5bit1v(&v) << 8;
        b[5] = lshift_5bit1v(&v) << 8;
        b[4] = lshift_5bit1v(&v) << 8;
        b[3] = lshift_5bit1v(&v) << 8;
        b[2] = lshift_5bit1v(&v) << 8;
        b[1] = lshift_5bit1v(&v) << 8;
        b[0] = lshift_5bit1v(&v) << 8;
   } else {
        p = (v4hi *)b;
	*p++ = v;
	*p = v;
   }
   

}

static void rpixel2cbuf(Uint32 addr, Uint32 *b, Uint32 mpage)
{
   Uint8 ret = 0;
   v4hi v;
   v4hi *p;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x20)){
        v.b[5] = vram_p[addr + 0x08000]; 
        v.b[4] = vram_p[addr + 0x0a000]; 
        v.b[3] = vram_p[addr + 0x0c000]; 
        v.b[2] = vram_p[addr + 0x0e000]; 
        v.b[1] = vram_p[addr + 0x20000]; 
        v.b[0] = vram_p[addr + 0x22000]; 

        b[7] = lshift_5bit1v(&v);
        b[6] = lshift_5bit1v(&v);
        b[5] = lshift_5bit1v(&v);
        b[4] = lshift_5bit1v(&v);
        b[3] = lshift_5bit1v(&v);
        b[2] = lshift_5bit1v(&v);
        b[1] = lshift_5bit1v(&v);
        b[0] = lshift_5bit1v(&v);
   } else {
        p = (v4hi *)b;
	*p++ = v;
	*p = v;
   }
}

static void bpixel2cbuf(Uint32 addr, Uint32 *b, Uint32 mpage)
{
   Uint8 ret = 0;
   v4hi v;
   v4hi *p;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x10)){
        v.b[5] = vram_p[addr + 0x00000]; 
        v.b[4] = vram_p[addr + 0x02000]; 
        v.b[3] = vram_p[addr + 0x04000]; 
        v.b[2] = vram_p[addr + 0x06000]; 
        v.b[1] = vram_p[addr + 0x18000]; 
        v.b[0] = vram_p[addr + 0x1a000]; 

        b[7] = lshift_5bit1v(&v) << 16;
        b[6] = lshift_5bit1v(&v) << 16;
        b[5] = lshift_5bit1v(&v) << 16;
        b[4] = lshift_5bit1v(&v) << 16;
        b[3] = lshift_5bit1v(&v) << 16;
        b[2] = lshift_5bit1v(&v) << 16;
        b[1] = lshift_5bit1v(&v) << 16;
        b[0] = lshift_5bit1v(&v) << 16;
   } else {
        p = (v4hi *)b;
	*p++ = v;
	*p = v;
   }
}


static void getvram_256k(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
    Uint8           b[6],
                    r[6],
                    g[6];
    Uint32 	rr, gg, bb, aa;
    Uint32 rbuf[8], gbuf[8], bbuf[8];
    int i;
    Uint8 *vram_p = vram_pb;
    v4hi *p, *qb, *qg, *qr;
    /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う
     */
    p = (v4hi *)cbuf;
    qr = (v4hi *)rbuf;
    qg = (v4hi *)gbuf;
    qb = (v4hi *)bbuf;

   
#if 0
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
    	r[1] = vram_p[addr + 0x20000]; // 28
    	r[0] = vram_p[addr + 0x22000]; // 2a
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
#else
   bpixel2cbuf(addr, bbuf, mpage);
   rpixel2cbuf(addr, rbuf, mpage);
   gpixel2cbuf(addr, gbuf, mpage);
   p->v = qr->v + qg->v + qb->v;
   p++;
   qr++;
   qg++;
   qb++;
   p->v = qr->v + qg->v + qb->v;
#endif   
}


void CreateVirtualVram256k(Uint32 *p, int x, int y, int w, int h, int mode, Uint32 mpage)
{
	int ww, hh;
	int xx, yy;
	Uint32 addr;
	Uint32 *disp;
	Uint32 c[8];

    if(p == NULL) return;
	LockVram();
	ww = (w>>3) + (x>>3);
	hh = h + y;
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * 40 + xx ;
			getvram_256k(addr, c, mpage);
			disp = &p[xx * 8 + 320 * yy];
			putword(disp,  c);
			addr++;
			}
	}
   UnlockVram();
   return;
}

/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram256k_1Pcs(Uint32 *p, int x, int y, int pitch, int mpage)
{
    Uint32 c[8];
    Uint8 *disp = (Uint8 *)p;
    Uint32 addr;

    addr = y * 40 + x;
    // Loop廃止(高速化)

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, c, mpage);
    putword((Uint32 *)disp,  c);

}


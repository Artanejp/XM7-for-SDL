/*
 * api_vram256k.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
//#include "api_scaler.h"
#include "api_vram.h"

extern v8hi_t lshift_6bit8v_SSE2(v8hi_t v);

static void putword(Uint32 *disp, v8hi_t cx)
{
    v8hi_t *dst = (v8hi_t *)disp;
//    v8hi_t src = cx;
//    Uint32 *c;
//    c = cx;
    __builtin_prefetch(dst, 1, 2);
    *dst = cx;
//    src++;
//    dst++;
//    dst->v = src->v;
//    disp[0] = cx[0];
//    disp[1] = cx[1];
//    disp[2] = cx[2];
//    disp[3] = cx[3];
//    disp[4] = cx[4];
//    disp[5] = cx[5];
//    disp[6] = cx[6];
//    disp[7] = cx[7];
}



static v8hi_t gpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   register v8hi_t v;
   register v8hi_t v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x40)){
        v.b[5] = vram_p[addr + 0x10000]; 
        v.b[4] = vram_p[addr + 0x12000]; 
        v.b[3] = vram_p[addr + 0x14000]; 
        v.b[2] = vram_p[addr + 0x16000]; 
        v.b[1] = vram_p[addr + 0x28000]; 
        v.b[0] = vram_p[addr + 0x2a000]; 
        v1 = lshift_6bit8v_SSE2(v);
        return v1;
    
    } else {
       register v8hi_t r;
       r.v = (v8si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
}

static v8hi_t rpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   register v8hi_t v;
   register v8hi_t v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x20)){
        v.b[5] = vram_p[addr + 0x08000]; 
        v.b[4] = vram_p[addr + 0x0a000]; 
        v.b[3] = vram_p[addr + 0x0c000]; 
        v.b[2] = vram_p[addr + 0x0e000]; 
        v.b[1] = vram_p[addr + 0x20000]; 
        v.b[0] = vram_p[addr + 0x22000]; 
        v1 = lshift_6bit8v_SSE2(v);
        return v1;
   } else {
       register v8hi_t r;
       r.v = (v8si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
}

static v8hi_t bpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   register v8hi_t v;
   register v8hi_t v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x10)){
        v.b[5] = vram_p[addr + 0x00000]; 
        v.b[4] = vram_p[addr + 0x02000]; 
        v.b[3] = vram_p[addr + 0x04000]; 
        v.b[2] = vram_p[addr + 0x06000]; 
        v.b[1] = vram_p[addr + 0x18000]; 
        v.b[0] = vram_p[addr + 0x1a000]; 

        v1 = lshift_6bit8v_SSE2(v);
//        v1.v <<= 16;
        return v1;
   } else {
       register v8hi_t r;
       r.vv = (v8si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
}




static v8hi_t getvram_256k(Uint32 addr, Uint32 mpage)
{
   register v8hi_t r, g, b;
   v8hi_t a;
   register v8hi_t dst;
   /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う
     */
   
   b = bpixel2cbuf(addr, mpage);
   r = rpixel2cbuf(addr, mpage);
   g = gpixel2cbuf(addr, mpage);
#ifdef AG_LITTLE_ENDIAN
   a.vv = (v8ui){0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000};
   dst.vv = (b.vv << 16 ) | (g.vv << 8) | r.vv | a.vv;
#else   
#endif
   return dst;
}



/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram256k_1Pcs_SSE2(Uint32 *p, int x, int y, int pitch, int mpage)
{
    register v8hi_t c;
    register Uint8 *disp = (Uint8 *)p;
    register Uint32 addr;
    pitch = sizeof(Uint32) * 8;
   
    addr = y * 40 + x;
    // Loop廃止(高速化)

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);
    addr += 40;
    disp += pitch;

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);
    addr += 40;
    disp += pitch;

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);
    addr += 40;
    disp += pitch;

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);
    addr += 40;
    disp += pitch;

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);
    addr += 40;
    disp += pitch;

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);
    addr += 40;
    disp += pitch;

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);
    addr += 40;
    disp += pitch;

    c = getvram_256k(addr, mpage);
    putword((Uint32 *)disp, c);

}

void CreateVirtualVram256k_Line_SSE2(Uint32 *p, int ybegin, int yend, int mpage)
{
    register v8hi_t c;
    register v8hi_t *disp;
    register Uint32 addr;
    int yy;
    int xx;
    const int pitch = sizeof(Uint32) * 8;
    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       for(yy = ybegin; yy < yend; yy++) {
	  addr = yy * 40;
	  disp = (v8hi_t *)((Uint8 *)p + (pitch * addr));
	  for(xx = 0; xx < (40 / 8); xx++) {
	     putword((Uint32 *)disp,  c);
	     disp++;
	     putword((Uint32 *)disp,  c);
	     disp++;
	     putword((Uint32 *)disp,  c);
	     disp++;
	     putword((Uint32 *)disp,  c);
	     disp++;
	     putword((Uint32 *)disp,  c);
	     disp++;
	     putword((Uint32 *)disp,  c);
	     disp++;
	     putword((Uint32 *)disp,  c);
	     disp++;
	     putword((Uint32 *)disp,  c);
	     disp++;
	  }
       }
    } else {
       for(yy = ybegin; yy < yend; yy++) {
	  addr = yy * 40;
	  disp = (v8hi_t *)((Uint8 *)p + (pitch * addr));
	  for(xx = 0; xx < (40 / 8); xx++) {
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	     c = getvram_256k(addr, mpage);
	     putword((Uint32 *)disp,  c);
	     disp++;
	     addr++;
	  }
       }
    } 
}

void CreateVirtualVram256k_WindowedLine_SSE2(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mpage)
{
   CreateVirtualVram256k_Line_SSE2(p, ybegin, yend, mpage);
}

Api_Vram_FuncList api_vram256k_sse2 = {
   CreateVirtualVram256k_1Pcs_SSE2,
   CreateVirtualVram256k_Line_SSE2,
   CreateVirtualVram256k_WindowedLine_SSE2
};

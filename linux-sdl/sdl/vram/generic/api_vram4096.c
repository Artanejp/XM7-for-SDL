/*
 * api_vram4096.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
#include "api_vram.h"

Uint8 *vram_pb;
Uint8 *vram_pr;
Uint8 *vram_pg;


void CalcPalette_4096Colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint32 ds;
    r = r & 0xf0;
    g = g & 0xf0;
    b = b & 0xf0;
   
    if((index > 4095) || (index < 0)) return;
#ifdef SDL_LIL_ENDIAN
	ds =r | (g << 8) | (b << 16) | (a<<24);
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
    rgbAnalogGDI[index] = ds;
}

static inline void putword2_vec(Uint32 *disp, volatile v8hi_t cbuf)
{
   v8hi_t *dst = (v8hi_t *)disp;
   v8hi_t r1;
   
   r1.i[0] = rgbAnalogGDI[cbuf.i[0]];
   r1.i[1] = rgbAnalogGDI[cbuf.i[1]];
   r1.i[2] = rgbAnalogGDI[cbuf.i[2]];
   r1.i[3] = rgbAnalogGDI[cbuf.i[3]];
   r1.i[4] = rgbAnalogGDI[cbuf.i[4]];
   r1.i[5] = rgbAnalogGDI[cbuf.i[5]];
   r1.i[6] = rgbAnalogGDI[cbuf.i[6]];
   r1.i[7] = rgbAnalogGDI[cbuf.i[7]];
   dst->v = r1.v;
}

static inline void getvram_4096_vec(Uint32 addr, v8hi_t *cbuf)
{

    uint8_t r0, r1, r2, r3;
    uint8_t g0, g1, g2, g3;
    uint8_t b0, b1, b2, b3;
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */
    g3 = vram_pg[addr + 0x00000];
    g2 = vram_pg[addr + 0x02000];
    g1 = vram_pg[addr + 0x04000];
    g0 = vram_pg[addr + 0x06000];
    cbuf->v = 
        aPlanes[G0 + g0] |
        aPlanes[G1 + g1] |
        aPlanes[G2 + g2] |
        aPlanes[G3 + g3] ;

   
    r3 = vram_pr[addr + 0x00000];
    r2 = vram_pr[addr + 0x02000];
    r1 = vram_pr[addr + 0x04000];
    r0 = vram_pr[addr + 0x06000];
    cbuf->v = cbuf->v |
        aPlanes[R0 + r0] |
        aPlanes[R1 + r1] |
        aPlanes[R2 + r2] |
        aPlanes[R3 + r3] ;

    b3 = vram_pb[addr + 0x00000];
    b2 = vram_pb[addr + 0x02000];
    b1 = vram_pb[addr + 0x04000];
    b0 = vram_pb[addr + 0x06000];
    cbuf->v = cbuf->v |
        aPlanes[B0 + b0] |
        aPlanes[B1 + b1] |
        aPlanes[B2 + b2] |
        aPlanes[B3 + b3] ;
   return;
}

/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram4096_1Pcs(Uint32 *p, int x, int y, int pitch, int mode)
{
//    Uint32 c[8];
    v8hi_t c;
    Uint8 *disp = (Uint8 *)p;
    Uint32 addr;

    pitch = sizeof(Uint32) * 8;
    addr = y * 40 + x;
    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v8si){0,0,0,0,0,0,0,0};
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
       disp += pitch;
       putword2_vec((Uint32 *)disp,  c);
//       disp += pitch;
    } else {
       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;

       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;

       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;

       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
       addr += 40;
       disp += pitch;
       
       getvram_4096_vec(addr, &c);
       putword2_vec((Uint32 *)disp,  c);
    }
   
}
   

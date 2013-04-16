/*
 * api_vram8.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
#include "api_vram.h"
#include "sdl_cpuid.h"



void SetVram_200l(Uint8 *p)
{
    vram_pb = p + 0;
    vram_pg = p + 0x10000;
    vram_pr = p + 0x8000;
}

void SetVram_400l(Uint8 *p)
{
    vram_pb = p + 0;
    vram_pg = p + 0x10000;
    vram_pr = p + 0x8000;
}


void CalcPalette_8colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
     Uint32 ds;

//     if((index > 10) || (index < 0)) return;
//     LockVram();
#ifdef AG_LITTLE_ENDIAN
	ds = r | (g << 8) | (b << 16) | 0xff000000;
//	ds = 0xffffffff;
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
    rgbTTLGDI[index & 7] = ds;
//    UnlockVram();
}

#if (__GNUC__ >= 4)
static void getvram_8_vec(Uint32 addr, v4hi *cbuf)
{
    uint8_t r, g, b;
//    volatile v4hi cbuf __attribute__((aligned(32)));
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */

    g = vram_pg[addr];
    r = vram_pr[addr];
    b = vram_pb[addr];

    cbuf->v = aPlanes[B0 + b] |
              aPlanes[B1 + r] |
              aPlanes[B2 + g];
   return;
}

static inline void  putword8_vec(Uint32 *disp, volatile v4hi c, v8hi_t pal)
{

   v8hi_t *dst = (v8hi_t *)disp;
   register v8hi_t r1;
   
//   if(disp == NULL) return;
   c.v &= (v4si){7, 7, 7, 7, 7, 7, 7, 7,};
   r1.i[0] = pal.i[c.s[0]]; // ?!
   r1.i[1] = pal.i[c.s[1]];
   r1.i[2] = pal.i[c.s[2]];
   r1.i[3] = pal.i[c.s[3]];
   r1.i[4] = pal.i[c.s[4]];
   r1.i[5] = pal.i[c.s[5]];
   r1.i[6] = pal.i[c.s[6]];
   r1.i[7] = pal.i[c.s[7]];
   dst->v = r1.v;
}

#else
static inline void planeto8(Uint32 *c, uint8_t r, unit8_t g, uint8_t b)
{
   Uint8 mask;
   
   mask = 0x80;
   c[0] = ((r & mask) >> 6) | ((g & mask) >> 5) || ((b & mask) >> 7);
   mask >>= 1;
   c[1] = ((r & mask) >> 5) | ((g & mask) >> 4) || ((b & mask) >> 6);
   mask >>= 1;
   c[2] = ((r & mask) >> 4) | ((g & mask) >> 3) || ((b & mask) >> 5);
   mask >>= 1;
   c[3] = ((r & mask) >> 3) | ((g & mask) >> 2) || ((b & mask) >> 4);
   mask >>= 1;
   c[4] = ((r & mask) >> 2) | ((g & mask) >> 1) || ((b & mask) >> 3);
   mask >>= 1;
   c[5] = ((r & mask) >> 1) | (g & mask) || ((b & mask) >> 2);
   mask >>= 1;
   c[6] = (r & mask) | ((g & mask) << 1) || ((b & mask) >> 1);
   mask >>= 1;
   c[7] = ((r & mask) << 1) | ((g & mask) << 2) || (b & mask);
   mask >>= 1;
}

static void getvram_8(Uint32 addr, Uint32 *cbuf)
{
    uint8_t r, g, b;
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */
   
   g = vram_pg[addr];
   r = vram_pr[addr];
   b = vram_pb[addr];
   planeto8(cbuf, r, g, b);
  
   return;
}

static inline void  putword8(Uint32 *disp, Uint32 *c, Uint32 *pal)
{

   Uint32 *r1 = disp;

   r1[0] = pal[c[0] & 7]; // ?!
   r1[1] = pal[c[1] & 7];
   r1[2] = pal[c[2] & 7];
   r1[3] = pal[c[3] & 7];
   r1[4] = pal[c[4] & 7];
   r1[5] = pal[c[5] & 7];
   r1[6] = pal[c[6] & 7];
   r1[7] = pal[c[7] & 7];
}

#endif // __GNUC__ >= 4



/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram8_1Pcs(Uint32 *p, int x, int y, int pitch, int mode)
{
#if (__GNUC__ >= 4)   
    v4hi c;
    v8hi_t *p1 = (v8hi_t *)rgbTTLGDI;
    v8hi_t pal;
    Uint8 *disp =(Uint8 *) p;
    Uint32 addr;

    if((p == NULL) || (p1 == NULL)) return;
    pal.v = p1->v; // Reduce reading palette.
    pitch = sizeof(Uint32) * 8;
    addr = y * 80 + x;

    // Loop廃止(高速化)
    if(aPlanes == NULL) {
       c.v = (v4si){0,0,0,0,0,0,0,0};
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
       disp += pitch;
       putword8_vec((Uint32 *)disp,  c, pal);
//       disp += pitch;
       return;
     } else {
       getvram_8_vec(addr, &c);
       putword8_vec((Uint32 *)disp, c, pal);
       addr += 80;
       disp += pitch;

       getvram_8_vec(addr , &c);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       getvram_8_vec(addr, &c);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       getvram_8_vec(addr , &c);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       getvram_8_vec(addr, &c);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       getvram_8_vec(addr, &c);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       getvram_8_vec(addr, &c);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       getvram_8_vec(addr, &c);
       putword8_vec((Uint32 *)disp,  c, pal);
//    addr += 80;
//    disp += pitch;
     }
#else 
    Uint32 c[8];
    Uint32 *pal = (Uint32 *)rgbTTLGDI;
    Uint8 *disp =(Uint8 *) p;

    if((p == NULL) || (pal == NULL)) return;
    pitch = sizeof(Uint32) * 8;
    addr = y * 80 + x;

    // Loop廃止(高速化)
   getvram_8(addr, c);
   putword8((Uint32 *)disp, c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr , c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr , c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   addr += 80;
   disp += pitch;
   
   getvram_8(addr, c);
   putword8((Uint32 *)disp,  c, pal);
   //    addr += 80;
   //    disp += pitch;
     
#endif   
}


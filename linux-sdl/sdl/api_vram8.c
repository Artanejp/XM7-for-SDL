/*
 * api_vram8.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
#include "api_vram.h"
#include "sdl_cpuid.h"


Uint8 *vram_pb;
Uint8 *vram_pr;
Uint8 *vram_pg;


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
    rgbTTLGDI[index] = ds;
//    UnlockVram();
}

static inline void  putword8_vec(Uint32 *disp, volatile v4hi c, volatile v8hi_t pal)
{
   v8hi_t *dst = (v8hi_t *)disp;
   v8hi_t r1, r2, r3;
   
   if(disp == NULL) return;
   
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


/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram8_1Pcs(Uint32 *p, int x, int y, int pitch, int mode)
{
    volatile v4hi c;
    v8hi_t *p1 = (v8hi_t *)rgbTTLGDI;
    v8hi_t pal;
    Uint8 *disp =(Uint8 *) p;
    Uint32 addr;

    pal.v = p1->v; // Reduce reading palette.
    pitch = sizeof(Uint32) * 8;
    addr = y * 80 + x;

    // Loop廃止(高速化)
     {
       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp, c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
       addr += 80;
       disp += pitch;

       c = getvram_8_vec(addr);
       putword8_vec((Uint32 *)disp,  c, pal);
//    addr += 80;
//    disp += pitch;
    }


}


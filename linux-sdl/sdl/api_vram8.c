/*
 * api_vram8.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
#include "api_vram.h"

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
	ds =r + (g << 8)+ (b << 16) + 255<<24;
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
    rgbTTLGDI[index] = ds;
//    UnlockVram();
}

static void  putword8_vec(volatile Uint32 *disp, volatile v8hi c)
{
   v8hi *pal =(v8hi *)rgbTTLGDI;
   v8hi *dst = (v8hi *)disp;

//   dst->v =  __builtin_shuffle (pal->v, c.v);
    dst->i[0] = rgbTTLGDI[c.i[0]];
    dst->i[1] = rgbTTLGDI[c.i[1]];
    dst->i[2] = rgbTTLGDI[c.i[2]];
    dst->i[3] = rgbTTLGDI[c.i[3]];
    dst->i[4] = rgbTTLGDI[c.i[4]];
    dst->i[5] = rgbTTLGDI[c.i[5]];
    dst->i[6] = rgbTTLGDI[c.i[6]];
    dst->i[7] = rgbTTLGDI[c.i[7]];
}



/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram8_1Pcs(Uint32 *p, int x, int y, int pitch, int mode)
{
    v8hi c;
    Uint8 *disp =(Uint8 *) p;
    Uint32 addr;

    addr = y * 80 + x;

    // Loop廃止(高速化)
    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp, c);
    addr += 80;
    disp += pitch;

    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    c = getvram_8_vec(addr);
    putword8_vec((Uint32 *)disp,  c);
//    addr += 80;
//    disp += pitch;

}


/*
 * api_vram4096.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
#include "api_vram.h"



void CalcPalette_4096Colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    Uint32 ds;
//     if((index > 4095) || (index < 0)) return;
#ifdef SDL_LIL_ENDIAN
	ds =r + (g << 8)+ (b << 16) + (a<<24);
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
    rgbAnalogGDI[index] = ds;
}

static inline void putword2_vec(Uint32 *disp, volatile v8hi cbuf)
{
   v8hi *dst = (v8hi *)disp;
   dst->i[0] = rgbAnalogGDI[cbuf.i[0]];
   dst->i[1] = rgbAnalogGDI[cbuf.i[1]];
   dst->i[2] = rgbAnalogGDI[cbuf.i[2]];
   dst->i[3] = rgbAnalogGDI[cbuf.i[3]];
   dst->i[4] = rgbAnalogGDI[cbuf.i[4]];
   dst->i[5] = rgbAnalogGDI[cbuf.i[5]];
   dst->i[6] = rgbAnalogGDI[cbuf.i[6]];
   dst->i[7] = rgbAnalogGDI[cbuf.i[7]];
}


/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram4096_1Pcs(Uint32 *p, int x, int y, int pitch, int mode)
{
//    Uint32 c[8];
    v8hi c;
    Uint8 *disp = (Uint8 *)p;
    Uint32 addr;

    addr = y * 40 + x;
    // Loop廃止(高速化)

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);
    addr += 40;
    disp += pitch;

    c = getvram_4096_vec(addr);
    putword2_vec((Uint32 *)disp,  c);

}


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
	ds =r | (g << 8) | (b << 16) | (a<<24);
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
    rgbAnalogGDI[index] = ds;
}

static inline void putword2_vec(Uint32 *disp, volatile v4hi cbuf)
{
   v8hi_t *dst = (v8hi_t *)disp;
   v8hi_t r1;
   
   r1.i[0] = rgbAnalogGDI[cbuf.s[0]];
   r1.i[1] = rgbAnalogGDI[cbuf.s[1]];
   r1.i[2] = rgbAnalogGDI[cbuf.s[2]];
   r1.i[3] = rgbAnalogGDI[cbuf.s[3]];
   r1.i[4] = rgbAnalogGDI[cbuf.s[4]];
   r1.i[5] = rgbAnalogGDI[cbuf.s[5]];
   r1.i[6] = rgbAnalogGDI[cbuf.s[6]];
   r1.i[7] = rgbAnalogGDI[cbuf.s[7]];
   dst->v = r1.v;
}


/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram4096_1Pcs(Uint32 *p, int x, int y, int pitch, int mode)
{
//    Uint32 c[8];
    v4hi c;
    Uint8 *disp = (Uint8 *)p;
    Uint32 addr;

    pitch = sizeof(Uint32) * 8;
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


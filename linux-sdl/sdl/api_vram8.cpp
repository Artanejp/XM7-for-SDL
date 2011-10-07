/*
 * api_vram8.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
//#include <GL/gl.h>
//#include <GL/glext.h>
#include "api_draw.h"
#include "api_scaler.h"
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


void CalcPalette_8colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
     Uint32 ds;

//     if((index > 10) || (index < 0)) return;
     LockVram();
#ifdef AG_LITTLE_ENDIAN
	ds =r + (g << 8)+ (b << 16) + (a<<24);
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
    rgbTTLGDI[index] = ds;
    UnlockVram();
}

static inline void putword(Uint32 *disp, Uint8 *cbuf)
{
    disp[0] = (Uint32)(cbuf[7] & 0x0f);
    disp[1] = (Uint32)(cbuf[6] & 0x0f);
    disp[2] = (Uint32)(cbuf[5] & 0x0f);
    disp[3] = (Uint32)(cbuf[4] & 0x0f);
    disp[4] = (Uint32)(cbuf[3] & 0x0f);
    disp[5] = (Uint32)(cbuf[2] & 0x0f);
    disp[6] = (Uint32)(cbuf[1] & 0x0f);
    disp[7] = (Uint32)(cbuf[0] & 0x0f);
}

static inline void putword8(Uint32 *disp, Uint8 *cbuf)
{
    disp[0] = rgbTTLGDI[cbuf[7] & 0x0f];
    disp[1] = rgbTTLGDI[cbuf[6] & 0x0f];
    disp[2] = rgbTTLGDI[cbuf[5] & 0x0f];
    disp[3] = rgbTTLGDI[cbuf[4] & 0x0f];
    disp[4] = rgbTTLGDI[cbuf[3] & 0x0f];
    disp[5] = rgbTTLGDI[cbuf[2] & 0x0f];
    disp[6] = rgbTTLGDI[cbuf[1] & 0x0f];
    disp[7] = rgbTTLGDI[cbuf[0] & 0x0f];
}

static inline void getvram_8to8(Uint32 addr, Uint8 *cbuf)
{
       Uint8    cb,
                cr,
                cg;
        cb = vram_pb[addr];
        cr = vram_pr[addr];
        cg = vram_pg[addr];
        cbuf[0] =   (cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2);
        cbuf[1] =   ((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1);
        cbuf[2] =   ((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04);
        cbuf[3] =   ((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +((cg & 0x08) >> 1);
        cbuf[4] =   ((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) + ((cg & 0x10) >> 2);
        cbuf[5] =   ((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) + ((cg & 0x20) >> 3);
        cbuf[6] =   ((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) + ((cg & 0x40) >> 4);
        cbuf[7] =   ((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) + ((cg & 0x80) >> 5);
}

void getvram_400l(Uint32 addr,Uint32 *p, Uint32 mpage)
{
  getvram_8to8(addr, (Uint8 *)p);
}

void getvram_200l(Uint32 addr,Uint32 *p, Uint32 mpage)
{
  getvram_8to8(addr, (Uint8 *)p);
}

void CreateVirtualVram8(Uint32 *p, int x, int y, int w, int h, int mode)
{
	int ww, hh;
	int xx, yy;
	Uint32 addr;
	Uint32 *disp;
	Uint8 c[8];

	LockVram();

    switch(mode) {
    case SCR_200LINE:
        break;
    case SCR_400LINE:
        break;
    default:
        UnlockVram();
        return;
        break;
    }
	ww = (w>>3) + (x>>3);
	hh = h + y;
	if(p == NULL) {
		UnlockVram();
		return;
	}
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * 80 + xx ;
			getvram_8to8(addr, c);
			disp = &p[xx * 8 + 640 * yy];
			putword8(disp,  c);
			addr++;
			}
	}
   UnlockVram();
   return;
}

/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram8_1Pcs(Uint32 *p, int x, int y, int pitch, int mode)
{
    Uint8 c[8];
    Uint8 *disp = (Uint8 *)p;
    Uint32 addr;

    addr = y * 80 + x;
    // Loop廃止(高速化)
#if 1
    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword8((Uint32 *)disp,  c);
#else
    // Pattern TEST
    p[0] = 0xffffffff;
    p[1] = 0xffffffff;
    p[2] = 0xffffffff;
    p[3] = 0xffffffff;
    p[4] = 0xffffffff;
    p[5] = 0xffffffff;
    p[6] = 0xffffffff;
    p[7] = 0xffffffff;
#endif
}

/*
 * 8x8のピースをVRAMから作成する(パレットなし)：VramLockしない事に注意
 */
void CreateVirtualVram8_1Pcs_Nopal(Uint32 *p, int x, int y, int pitch, int mode)
{
    Uint8 c[8];
    Uint8 *disp = (Uint8 *)p;
    Uint32 addr;

    addr = y * 80 + x;
    // Loop廃止(高速化)

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);
    addr += 80;
    disp += pitch;

    getvram_8to8(addr, c);
    putword((Uint32 *)disp,  c);

}



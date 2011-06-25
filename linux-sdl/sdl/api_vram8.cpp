/*
 * api_vram8.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include "api_draw.h"
#include "api_scaler.h"
#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"

extern Uint32 *pVram2;
extern  GLuint uVramTextureID;
extern void DiscardTexture(GLuint tid);
extern "C"
{
    extern void LockVram(void);
    extern void UnlockVram(void);
}

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


static inline void putword8(Uint8 *disp, Uint8 *cbuf)
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


static void copy8to32(Uint32 *p, int w, int h)
{
    int size = w * h;
    int i;
    Uint8 *pp;

    pp = (Uint8 *)p;
    for(i = 0; i < size; i+=8 ) {
        pVram2[i] =(Uint32) rgbTTLGDI[*pp];
        pVram2[i + 1] =(Uint32) rgbTTLGDI[*(pp + 1)];
        pVram2[i + 2] =(Uint32) rgbTTLGDI[*(pp + 2)];
        pVram2[i + 3] =(Uint32) rgbTTLGDI[*(pp + 3)];
        pVram2[i + 4] =(Uint32) rgbTTLGDI[*(pp + 4)];
        pVram2[i + 5] =(Uint32) rgbTTLGDI[*(pp + 5)];
        pVram2[i + 6] =(Uint32) rgbTTLGDI[*(pp + 6)];
        pVram2[i + 7] =(Uint32) rgbTTLGDI[*(pp + 7)];
        pp+=8;
    }
}

/*
 * TextureをUpdateモードで作成する(8色, 16色パレット付き)
 */
GLuint UpdateTexture8(Uint32 *p, GLuint tid, int w, int h)
{
    GLuint ttid;

    if((w < 0) || (h < 0)) return 0;
    if(p == NULL) return 0;
    if(pVram2 == NULL) return 0;

    if(tid == 0) {
        glGenTextures(1, &ttid);
    } else {
        ttid = tid;
    }
    glBindTexture(GL_TEXTURE_2D, ttid);
    copy8to32(p, w, h);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pVram2);
    return ttid;
}

GLuint CreateVirtualVram8(Uint32 *p, int x, int y, int w, int h, int mode)
{
	int ww, hh;
	int vramwidth, vramheight;
	int xx, yy;
	Uint32 addr;
	Uint8 *bitmap;
	Uint8 *disp;
	Uint8 c[8];

	LockVram();

    switch(mode) {
    case SCR_200LINE:
        vramwidth = 640;
        vramheight = 200;
        break;
    case SCR_400LINE:
        vramwidth = 640;
        vramheight = 400;
        break;
    default:
        UnlockVram();
        return 0;
        break;
    }
	ww = (w>>3) + (x>>3);
	hh = h + y;
	bitmap = (Uint8 *)p;
	if(bitmap == NULL) {
		UnlockVram();
		return 0;
	}
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * 80 + xx ;
			getvram_8to8(addr, c);
			disp = bitmap + xx * 8 + 640 * yy;
			putword8((Uint8 *)disp,  c);
			addr++;
			}
	}
   DiscardTexture(uVramTextureID);
   uVramTextureID = UpdateTexture8(p, 0, vramwidth, vramheight);
   UnlockVram();
   return uVramTextureID;
}


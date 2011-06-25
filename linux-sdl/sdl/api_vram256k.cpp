/*
 * api_vram256k.cpp
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


extern  GLuint uVramTextureID;
extern Uint32 *pVram2;
extern Uint8 *vram_pb;
extern Uint8 *vram_pr;
extern Uint8 *vram_pg;

extern void DiscardTexture(GLuint tid);
extern "C"
{
    extern void LockVram(void);
    extern void UnlockVram(void);
}

GLuint UpdateTexture256k(Uint32 *p, GLuint tid , int w, int h)
{
    GLuint ttid;

    if((w < 0) || (h < 0)) return 0;
    if(p == NULL) return 0;
    if(tid == 0) {
        glGenTextures(1, &ttid);
    } else {
        ttid = tid;
    }
    glBindTexture(GL_TEXTURE_2D, ttid);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 p);
    return ttid;
}

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

static void getvram256k(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
    Uint8           b[6],
                    r[6],
                    g[6];
    Uint32 	rr, gg, bb, aa;
    int i;
    Uint8 *vram_p = vram_pb;

    /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う
     */

    cbuf[0] = 0x00000000;
    cbuf[1] = 0x00000000;
    cbuf[2] = 0x00000000;
    cbuf[3] = 0x00000000;
    cbuf[4] = 0x00000000;
    cbuf[5] = 0x00000000;
    cbuf[6] = 0x00000000;
    cbuf[7] = 0x00000000;

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
    	r[1] = vram_p[addr + 0x28000];
    	r[0] = vram_p[addr + 0x2a000];
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
}


GLuint CreateVirtualVram256k(Uint32 *p, int x, int y, int w, int h, int mode, Uint32 mpage)
{
	int ww, hh;
	int xx, yy;
	Uint32 addr;
	Uint32 *disp;
	Uint32 c[8];

    if(p == NULL) return 0;
	LockVram();
	ww = (w>>3) + (x>>3);
	hh = h + y;
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * 40 + xx ;
			getvram256k(addr, c, mpage);
			disp = &p[xx * 8 + 320 * yy];
			putword(disp,  c);
			addr++;
			}
	}
   DiscardTexture(uVramTextureID);
   uVramTextureID = UpdateTexture256k(p, 0, 320, 200);
   UnlockVram();
   return uVramTextureID;
}

/*
 * api_vram256k.cpp
 * Convert VRAM -> VirtualVram
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
//#include "api_scaler.h"
#include "api_vram.h"


static void putword(Uint32 *disp, Uint32 *cx)
{
//    v4hi *dst = (v4hi *)disp;
//    v4hi *src = (v4hi *)src;
//    Uint32 *c;
//    c = cx;

	
//    dst->v = src->v;
//    src++;
//    dst++;
//    dst->v = src->v;
    disp[0] = cx[0];
    disp[1] = cx[1];
    disp[2] = cx[2];
    disp[3] = cx[3];
    disp[4] = cx[4];
    disp[5] = cx[5];
    disp[6] = cx[6];
    disp[7] = cx[7];
}



static v4hi gpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   v4hi v;
   v4hi v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x40)){
        v.b[5] = vram_p[addr + 0x10000]; 
        v.b[4] = vram_p[addr + 0x12000]; 
        v.b[3] = vram_p[addr + 0x14000]; 
        v.b[2] = vram_p[addr + 0x16000]; 
        v.b[1] = vram_p[addr + 0x28000]; 
        v.b[0] = vram_p[addr + 0x2a000]; 
        v1 = lshift_6bit8v(&v);
        return v1;
    
    } else {
       v4hi r;
       r.v = (v4si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
   

}

static v4hi rpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   v4hi v;
   v4hi v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x20)){
        v.b[5] = vram_p[addr + 0x08000]; 
        v.b[4] = vram_p[addr + 0x0a000]; 
        v.b[3] = vram_p[addr + 0x0c000]; 
        v.b[2] = vram_p[addr + 0x0e000]; 
        v.b[1] = vram_p[addr + 0x20000]; 
        v.b[0] = vram_p[addr + 0x22000]; 
        v1 = lshift_6bit8v(&v);
        return v1;
   } else {
       v4hi r;
       r.v = (v4si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
}

static v4hi bpixel2cbuf(Uint32 addr, Uint32 mpage)
{
   Uint8 ret = 0;
   v4hi v;
   v4hi v1;
   Uint8 *vram_p = vram_pb;
   
    v.i[0] = v.i[1] = v.i[2] = v.i[3] = 0;
    if(!(mpage & 0x10)){
        v.b[5] = vram_p[addr + 0x00000]; 
        v.b[4] = vram_p[addr + 0x02000]; 
        v.b[3] = vram_p[addr + 0x04000]; 
        v.b[2] = vram_p[addr + 0x06000]; 
        v.b[1] = vram_p[addr + 0x18000]; 
        v.b[0] = vram_p[addr + 0x1a000]; 
        

        v1 = lshift_6bit8v(&v);
//        v1.v <<= 16;
        return v1;
   } else {
       v4hi r;
       r.v = (v4si){0, 0, 0, 0, 0, 0, 0, 0};
       return r;
   }
}




static void getvram_256k(Uint32 addr, Uint32 mpage, Uint32 *cbuf)
{
   v4hi r, g, b;
   /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う
     */
   
   b = bpixel2cbuf(addr, mpage);
   r = rpixel2cbuf(addr, mpage);
   g = gpixel2cbuf(addr, mpage);
#ifdef AG_LITTLE_ENDIAN   
   cbuf[0] = (b.s[0] << 16) | (g.s[0] << 8) | r.s[0] | 0xff000000;
   cbuf[1] = (b.s[1] << 16) | (g.s[1] << 8) | r.s[1] | 0xff000000;
   cbuf[2] = (b.s[2] << 16) | (g.s[2] << 8) | r.s[2] | 0xff000000;
   cbuf[3] = (b.s[3] << 16) | (g.s[3] << 8) | r.s[3] | 0xff000000;
   cbuf[4] = (b.s[4] << 16) | (g.s[4] << 8) | r.s[4] | 0xff000000;
   cbuf[5] = (b.s[5] << 16) | (g.s[5] << 8) | r.s[5] | 0xff000000;
   cbuf[6] = (b.s[6] << 16) | (g.s[6] << 8) | r.s[6] | 0xff000000;
   cbuf[7] = (b.s[7] << 16) | (g.s[7] << 8) | r.s[7] | 0xff000000;
#else   
#endif
   return ;
}



/*
 * 8x8のピースをVRAMから作成する：VramLockしない事に注意
 */
void CreateVirtualVram256k_1Pcs(Uint32 *p, int x, int y, int pitch, int mpage)
{
    Uint32 c[8];
    Uint8 *disp = (Uint8 *)p;
    Uint32 addr;
    pitch = sizeof(Uint32) * 8;
   
    addr = y * 40 + x;
    // Loop廃止(高速化)

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);
    addr += 40;
    disp += pitch;

    getvram_256k(addr, mpage, (Uint32 *)&c);
    putword((Uint32 *)disp, (Uint32 *)&c);

}


/*
 * api_vramvec.cpp
 * Convert VRAM -> VirtualVram(Vector Version)
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
#include "api_vram.h"

static inline uint16_t vshift4096(v8hi *src)
{
   v8hi tmp;
   v8si tmp2 = (v8si){1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0};
   uint16_t s;
   v8si *c = (v8si *)&(src->v);   
   
//   tmp.v = *c & (v8si){1,1,1,1,1,1,1,1,1,1,1,1};
   tmp.v = *c & tmp2;
   *c = *c >> tmp2;
   s = tmp.s[PLAING3] << 11 | tmp.s[PLAING2] << 10 | tmp.s[PLAING1] << 9 | tmp.s[PLAING0] <<8;
   s |= tmp.s[PLAINR3] << 7 | tmp.s[PLAINR2] << 6 | tmp.s[PLAINR1] << 5 | tmp.s[PLAINR0] <<4;
   s |= tmp.s[PLAINB3] << 3 | tmp.s[PLAINB2] << 2 | tmp.s[PLAINB1] << 1 | tmp.s[PLAINB0];
   return s;
}

void getvram_4096_vec(Uint32 addr, v4hi *cbuf)
{
        v8hi dat;
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上を考慮して、
         * インライン展開と細かいループの廃止を同時に行う
         */

        dat.s[PLAING3] = vram_pg[addr + 0x00000];
        dat.s[PLAING2] = vram_pg[addr + 0x02000];
        dat.s[PLAING1] = vram_pg[addr + 0x04000];
        dat.s[PLAING0] = vram_pg[addr + 0x06000];

        dat.s[PLAINR3] = vram_pr[addr + 0x00000];
        dat.s[PLAINR2] = vram_pr[addr + 0x02000];
        dat.s[PLAINR1] = vram_pr[addr + 0x04000];
        dat.s[PLAINR0] = vram_pr[addr + 0x06000];

        dat.s[PLAINB3] = vram_pb[addr + 0x00000];
        dat.s[PLAINB2] = vram_pb[addr + 0x02000];
        dat.s[PLAINB1] = vram_pb[addr + 0x04000];
        dat.s[PLAINB0] = vram_pb[addr + 0x06000];

        cbuf->s[7] = vshift4096(&dat);
        cbuf->s[6] = vshift4096(&dat);
        cbuf->s[5] = vshift4096(&dat);
        cbuf->s[4] = vshift4096(&dat);
        cbuf->s[3] = vshift4096(&dat);
        cbuf->s[2] = vshift4096(&dat);
        cbuf->s[1] = vshift4096(&dat);
        cbuf->s[0] = vshift4096(&dat);
}



static inline uint16_t vshift8(v4hi *src)
{
   v4hi tmp;
   v4si tmp2 = (v4si){1,1,1,0};
   uint16_t s;
   v4si *c = (v4si *)&(src->v);   
   
   tmp.v = *c & tmp2;
   *c = *c >> tmp2;
   s = tmp.s[PLAING] << 2 | tmp.s[PLAINR] << 1 | tmp.s[PLAINB];
   return s;
}

void getvram_8_vec(Uint32 addr, v4hi *cbuf)
{
        v4hi dat;
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上を考慮して、
         * インライン展開と細かいループの廃止を同時に行う
         */

        dat.s[PLAING] = vram_pg[addr];
        dat.s[PLAINR] = vram_pr[addr];
        dat.s[PLAINB] = vram_pb[addr];
        dat.s[PLAINW] = 0;

        cbuf->s[7] = vshift8(&dat);
        cbuf->s[6] = vshift8(&dat);
        cbuf->s[5] = vshift8(&dat);
        cbuf->s[4] = vshift8(&dat);
        cbuf->s[3] = vshift8(&dat);
        cbuf->s[2] = vshift8(&dat);
        cbuf->s[1] = vshift8(&dat);
        cbuf->s[0] = vshift8(&dat);
}

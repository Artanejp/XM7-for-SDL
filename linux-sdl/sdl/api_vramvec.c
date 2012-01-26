/*
 * api_vramvec.cpp
 * Convert VRAM -> VirtualVram(Vector Version)
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "api_draw.h"
#include "api_vram.h"

/*
* Definition of Convertsion Tables.
*/
v4hi aPlaneG[256];
v4hi aPlaneR[256];
v4hi aPlaneB[256];

v4hi aPlaneB0[256];
v4hi aPlaneB1[256];
v4hi aPlaneB2[256];
v4hi aPlaneB3[256];

v4hi aPlaneR0[256];
v4hi aPlaneR1[256];
v4hi aPlaneR2[256];
v4hi aPlaneR3[256];

v4hi aPlaneG0[256];
v4hi aPlaneG1[256];
v4hi aPlaneG2[256];
v4hi aPlaneG3[256];


static inline void initvramtblsub_vec(int x, v4hi *p)
{
    int shift;
    int mask = 0x80;
    p->s[0] = (mask & x)>>7;
    mask >>= 1;
    p->s[1] = (mask & x)>>6;
    mask >>= 1;
    p->s[2] = (mask & x)>>5;
    mask >>= 1;
    p->s[3] = (mask & x)>>4;
    mask >>= 1;
    p->s[4] = (mask & x)>>3;
    mask >>= 1;
    p->s[5] = (mask & x)>>2;
    mask >>= 1;
    p->s[6] = (mask & x)>>1;
    mask >>= 1;
    p->s[7] = (mask & x);
}

void initvramtbl_8_vec(void)
{
    int i;
    v4si shift = (v4si){1,1,1,1,1,1,1,1};

    for(i = 0; i < 256; i++){
        initvramtblsub_vec(i, &aPlaneB[i]);
        aPlaneR[i].v = aPlaneB[i].v << shift;
        aPlaneG[i].v = aPlaneR[i].v << shift;
    }
}

void initvramtbl_4096_vec(void)
{
    int i;
    v4si shift = (v4si){1,1,1,1,1,1,1,1};

    for(i = 0; i < 256; i++){
        initvramtblsub_vec(i, &aPlaneB0[i]);
        aPlaneB1[i].v = aPlaneB0[i].v << shift;
        aPlaneB2[i].v = aPlaneB1[i].v << shift;
        aPlaneB3[i].v = aPlaneB2[i].v << shift;

        aPlaneR0[i].v = aPlaneB3[i].v << shift;
        aPlaneR1[i].v = aPlaneR0[i].v << shift;
        aPlaneR2[i].v = aPlaneR1[i].v << shift;
        aPlaneR3[i].v = aPlaneR2[i].v << shift;

        aPlaneG0[i].v = aPlaneR3[i].v << shift;
        aPlaneG1[i].v = aPlaneG0[i].v << shift;
        aPlaneG2[i].v = aPlaneG1[i].v << shift;
        aPlaneG3[i].v = aPlaneG2[i].v << shift;
    }
}


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

#if 0
        cbuf->s[7] = vshift4096(&dat);
        cbuf->s[6] = vshift4096(&dat);
        cbuf->s[5] = vshift4096(&dat);
        cbuf->s[4] = vshift4096(&dat);
        cbuf->s[3] = vshift4096(&dat);
        cbuf->s[2] = vshift4096(&dat);
        cbuf->s[1] = vshift4096(&dat);
        cbuf->s[0] = vshift4096(&dat);
#else
    cbuf->v =  aPlaneB0[dat.s[PLAINB0]].v +
            aPlaneB1[dat.s[PLAINB1]].v +
            aPlaneB2[dat.s[PLAINB2]].v +
            aPlaneB3[dat.s[PLAINB3]].v +
            aPlaneR0[dat.s[PLAINR0]].v +
            aPlaneR1[dat.s[PLAINR1]].v +
            aPlaneR2[dat.s[PLAINR2]].v +
            aPlaneR3[dat.s[PLAINR3]].v +
            aPlaneG0[dat.s[PLAING0]].v +
            aPlaneG1[dat.s[PLAING1]].v +
            aPlaneG2[dat.s[PLAING2]].v +
            aPlaneG3[dat.s[PLAING3]].v;
#endif
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

#if 0
        cbuf->s[7] = vshift8(&dat);
        cbuf->s[6] = vshift8(&dat);
        cbuf->s[5] = vshift8(&dat);
        cbuf->s[4] = vshift8(&dat);
        cbuf->s[3] = vshift8(&dat);
        cbuf->s[2] = vshift8(&dat);
        cbuf->s[1] = vshift8(&dat);
        cbuf->s[0] = vshift8(&dat);
#else
    cbuf->v =   aPlaneB[dat.s[PLAINB]].v +
                aPlaneR[dat.s[PLAINR]].v +
                aPlaneG[dat.s[PLAING]].v;
#endif
}

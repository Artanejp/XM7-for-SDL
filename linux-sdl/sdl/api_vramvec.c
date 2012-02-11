/*
 * api_vramvec.cpp
 * Convert VRAM -> VirtualVram(Vector Version)
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "types.h"
#include "api_draw.h"
#include "api_vram.h"

/*
* Definition of Convertsion Tables.
*/
// Reduce Tables 20120131
static v4hi aPlaneB0[256];
static v4hi aPlaneB1[256];
static v4hi aPlaneB2[256];
static v4hi aPlaneB3[256];

static v4hi aPlaneR0[256];
static v4hi aPlaneR1[256];
static v4hi aPlaneR2[256];
static v4hi aPlaneR3[256];

static v4hi aPlaneG0[256];
static v4hi aPlaneG1[256];
static v4hi aPlaneG2[256];
static v4hi aPlaneG3[256];

static inline void initvramtblsub_vec(int x, v4hi *p)
{
    v4si mask =(v4si){0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
    p->v = (v4si){x,x,x,x,x,x,x,x};

    p->v = p->v & mask;
    p->s[0] >>= 7;
    p->s[1] >>= 6;
    p->s[2] >>= 5;
    p->s[3] >>= 4;
    p->s[4] >>= 3;
    p->s[5] >>= 2;
    p->s[6] >>= 1;

    // 8 Colors
}

void initvramtbl_8_vec(void)
{
}

void initvramtbl_4096_vec(void)
{
    int i;
    v4si shift = (v4si){2,2,2,2,2,2,2,2};
    // Init Mask Table
    // 20120131 Shift op is unstable, change to multiply.
   for(i = 0; i < 256; i++){
        initvramtblsub_vec(i, &aPlaneB0[i]);
        aPlaneB1[i].v = aPlaneB0[i].v * shift;
        aPlaneB2[i].v = aPlaneB1[i].v * shift;
        aPlaneB3[i].v = aPlaneB2[i].v * shift;

        aPlaneR0[i].v = aPlaneB3[i].v * shift;
        aPlaneR1[i].v = aPlaneR0[i].v * shift;
        aPlaneR2[i].v = aPlaneR1[i].v * shift;
        aPlaneR3[i].v = aPlaneR2[i].v * shift;

        aPlaneG0[i].v = aPlaneR3[i].v * shift;
        aPlaneG1[i].v = aPlaneG0[i].v * shift;
        aPlaneG2[i].v = aPlaneG1[i].v * shift;
        aPlaneG3[i].v = aPlaneG2[i].v * shift;
    }
}

void getvram_4096_vec(Uint32 addr, v4hi *cbuf)
{
    uint8_t dat[12];
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */

    dat[PLAING3] = vram_pg[addr + 0x00000];
    dat[PLAING2] = vram_pg[addr + 0x02000];
    dat[PLAING1] = vram_pg[addr + 0x04000];
    dat[PLAING0] = vram_pg[addr + 0x06000];

    dat[PLAINR3] = vram_pr[addr + 0x00000];
    dat[PLAINR2] = vram_pr[addr + 0x02000];
    dat[PLAINR1] = vram_pr[addr + 0x04000];
    dat[PLAINR0] = vram_pr[addr + 0x06000];

    dat[PLAINB3] = vram_pb[addr + 0x00000];
    dat[PLAINB2] = vram_pb[addr + 0x02000];
    dat[PLAINB1] = vram_pb[addr + 0x04000];
    dat[PLAINB0] = vram_pb[addr + 0x06000];
    cbuf->v =
        aPlaneB0[dat[PLAINB0]].v +
        aPlaneB1[dat[PLAINB1]].v +
        aPlaneB2[dat[PLAINB2]].v +
        aPlaneB3[dat[PLAINB3]].v +
        aPlaneR0[dat[PLAINR0]].v +
        aPlaneR1[dat[PLAINR1]].v +
        aPlaneR2[dat[PLAINR2]].v +
        aPlaneR3[dat[PLAINR3]].v +
        aPlaneG0[dat[PLAING0]].v +
        aPlaneG1[dat[PLAING1]].v +
        aPlaneG2[dat[PLAING2]].v +
        aPlaneG3[dat[PLAING3]].v ;
}




void getvram_8_vec(Uint32 addr, v4hi *cbuf)
{
    uint8_t dat[4];
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */

    dat[PLAING] = vram_pg[addr];
    dat[PLAINR] = vram_pr[addr];
    dat[PLAINB] = vram_pb[addr];
    dat[PLAINW] = 0;

    cbuf->v =
        aPlaneB0[dat[PLAINB]].v+
        aPlaneB1[dat[PLAINR]].v+
        aPlaneB2[dat[PLAING]].v;
}

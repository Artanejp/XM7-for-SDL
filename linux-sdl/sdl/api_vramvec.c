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

static v4hi *aPlanes;
enum {
   B0 = 0,
   B1 = 256,
   B2 = 512,
   B3 = 768,
   R0 = 1024,
   R1 = 1280,
   R2 = 1536,
   R3 = 1792,
   G0 = 2048,
   G1 = 2304,
   G2 = 2560,
   G3 = 2816
};

static inline void initvramtblsub_vec(int x, v4hi *p)
{
    v4si mask __attribute__((align(32))); 
    mask =(v4si){0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
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

static v4hi *initvramtblsub(int size)
{
   v4hi *p;
#ifndef _WINDOWS
   if(posix_memalign((void **)&p, 32, sizeof(v4hi) * size) != 0) return NULL;
#else
   p = (v4hi *)__mingw_aligned_malloc(sizeof(v4hi) * size, 32, 0);
   if(p == NULL) return NULL;
#endif
   return p;
}


void initvramtbl_4096_vec(void)
{
    int i;
    v4si shift = (v4si){2,2,2,2,2,2,2,2};
    aPlanes = (v4hi *)initvramtblsub(12 * 256);
    if(aPlanes == NULL) return;
    // Init Mask Table
    // 20120131 Shift op is unstable, change to multiply.
   for(i = 0; i < 256; i++){
        initvramtblsub_vec(i, &(aPlanes[B0 + i]));
        aPlanes[B1 + i].v = aPlanes[B0 + i].v * shift;
        aPlanes[B2 + i].v = aPlanes[B1 + i].v * shift;
        aPlanes[B3 + i].v = aPlanes[B2 + i].v * shift;

        aPlanes[R0 + i].v = aPlanes[B3 + i].v * shift;
        aPlanes[R1 + i].v = aPlanes[R0 + i].v * shift;
        aPlanes[R2 + i].v = aPlanes[R1 + i].v * shift;
        aPlanes[R3 + i].v = aPlanes[R2 + i].v * shift;

        aPlanes[G0 + i].v = aPlanes[R3 + i].v * shift;
        aPlanes[G1 + i].v = aPlanes[G0 + i].v * shift;
        aPlanes[G2 + i].v = aPlanes[G1 + i].v * shift;
        aPlanes[G3 + i].v = aPlanes[G2 + i].v * shift;
    }
}

void detachvramtbl_8_vec(void)
{
   
}

void detachvramtbl_4096_vec(void)
{
   if(aPlanes != NULL) {
#ifndef _WINDOWS
      free(aPlanes);
#else
      __mingw_aligned_free(aPlanes);
#endif
      aPlanes = NULL;
   }
}


v4hi getvram_4096_vec(Uint32 addr)
{
    v4hi cbuf __attribute__((aligned(32)));
    uint8_t dat[12];
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */
    if(aPlanes == NULL) {
       cbuf.v = (v4si){0,0,0,0,0,0,0,0};
       return cbuf;
    }
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
    cbuf.v =
        aPlanes[B0 + dat[PLAINB0]].v |
        aPlanes[B1 + dat[PLAINB1]].v |
        aPlanes[B2 + dat[PLAINB2]].v |
        aPlanes[B3 + dat[PLAINB3]].v |
        aPlanes[R0 + dat[PLAINR0]].v |
        aPlanes[R1 + dat[PLAINR1]].v |
        aPlanes[R2 + dat[PLAINR2]].v |
        aPlanes[R3 + dat[PLAINR3]].v |
        aPlanes[G0 + dat[PLAING0]].v |
        aPlanes[G1 + dat[PLAING1]].v |
        aPlanes[G2 + dat[PLAING2]].v |
        aPlanes[G3 + dat[PLAING3]].v ;
   return cbuf;
}




v4hi getvram_8_vec(Uint32 addr)
{
    uint8_t dat[4];
    v4hi cbuf __attribute__((aligned(32)));
    if(aPlanes == NULL) {
       cbuf.v = (v4si){0,0,0,0,0,0,0,0};
       return cbuf;
     }
        /*
         * R,G,Bについて8bit単位で描画する。
         * 高速化…キャッシュヒット率の向上とVector演算(MMXetc)の速度効果を考慮して、
         * ループの廃止を同時に行う
         */

    dat[PLAING] = vram_pg[addr];
    dat[PLAINR] = vram_pr[addr];
    dat[PLAINB] = vram_pb[addr];
//    dat[PLAINW] = 0;

    cbuf.v = aPlanes[B0 + dat[PLAINB]].v |
             aPlanes[B1 + dat[PLAINR]].v |
             aPlanes[B2 + dat[PLAING]].v;
   return cbuf;
}

/*
 * api_vramvec.cpp
 * Convert VRAM -> VirtualVram(Vector Version)
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL.h>
#include "xm7_types.h"
#include "api_draw.h"
#include "api_vram.h"

/*
* Definition of Convertsion Tables.
*/
// Reduce Tables 20120131

static v4si *aPlanes;
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

static void initvramtblsub_vec(volatile unsigned char x, volatile v4hi *p)
{
    p->v = (v4si){x & 0x80, x & 0x40, x & 0x20, x & 0x10, x & 0x08, x & 0x04, x & 0x02, x & 0x01};
    
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

static v4si *initvramtblsub(int size)
{
   v4si *p;
#ifndef _WINDOWS
   if(posix_memalign((void **)&p, 32, sizeof(v4si) * size) != 0) return NULL;
#else
   p = (v4si *)__mingw_aligned_malloc(sizeof(v4si) * size, 32);
   if(p == NULL) return NULL;
#endif
   return p;
}


void initvramtbl_4096_vec(void)
{
    int i;
//    v8si shift = (v8si){2,2,2,2,2,2,2,2};
    volatile v4hi r;
//    v8si shift = (v8si){1,1,1,1,1,1,1,1};
    aPlanes = initvramtblsub(12 * 256);
    if(aPlanes == NULL) return;
    printf("DBG: Table OK\n");
    // Init Mask Table
    // 20120131 Shift op is unstable, change to multiply.
   for(i = 0; i <= 255; i++){
        initvramtblsub_vec((i & 255), &r);

        aPlanes[B0 + i] = r.v;
        r.v <<= 1;
        aPlanes[B1 + i] = r.v;
        r.v <<= 1;
        aPlanes[B2 + i] = r.v;
        r.v <<= 1;
        aPlanes[B3 + i] = r.v;
        r.v <<= 1;


        aPlanes[R0 + i] = r.v;
        r.v <<= 1;
        aPlanes[R1 + i] = r.v;
        r.v <<= 1;
        aPlanes[R2 + i] = r.v;
        r.v <<= 1;
        aPlanes[R3 + i] = r.v;
        r.v <<= 1;
      
        aPlanes[G0 + i] = r.v;
        r.v <<= 1;
        aPlanes[G1 + i] = r.v;
        r.v <<= 1;
        aPlanes[G2 + i] = r.v;
        r.v <<= 1;
        aPlanes[G3 + i] = r.v;
//        r.v <<= 1;
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
    volatile v4hi cbuf __attribute__((aligned(32)));
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
        aPlanes[B0 + dat[PLAINB0]] |
        aPlanes[B1 + dat[PLAINB1]] |
        aPlanes[B2 + dat[PLAINB2]] |
        aPlanes[B3 + dat[PLAINB3]] |
        aPlanes[R0 + dat[PLAINR0]] |
        aPlanes[R1 + dat[PLAINR1]] |
        aPlanes[R2 + dat[PLAINR2]] |
        aPlanes[R3 + dat[PLAINR3]] |
        aPlanes[G0 + dat[PLAING0]] |
        aPlanes[G1 + dat[PLAING1]] |
        aPlanes[G2 + dat[PLAING2]] |
        aPlanes[G3 + dat[PLAING3]] ;
   return cbuf;
}

v4hi getvram_8_vec(Uint32 addr)
{
    uint8_t dat[4];
    volatile v4hi cbuf __attribute__((aligned(32)));
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

    cbuf.v = aPlanes[B0 + dat[PLAINB]] |
             aPlanes[B1 + dat[PLAINR]] |
             aPlanes[B2 + dat[PLAING]];
//   cbuf.v = (v8si){0,1,2,3,4,5,6,7};
   return cbuf;
}


v4hi lshift_6bit8v(v4hi *v)
{
   v4hi r;
   v4hi cbuf __attribute__((aligned(32)));
   v4hi mask;
   mask.v = (v4si){0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8};
   cbuf.v =
        aPlanes[B2 + v->b[0]] |
        aPlanes[B3 + v->b[1]] |
        aPlanes[R0 + v->b[2]] |
        aPlanes[R1 + v->b[3]] |
        aPlanes[R2 + v->b[4]] |
        aPlanes[R3 + v->b[5]];
   
   mask.v = mask.v & cbuf.v;
   if(mask.s[0] != 0) cbuf.s[0] |= 0x03;
   if(mask.s[1] != 0) cbuf.s[1] |= 0x03;
   if(mask.s[2] != 0) cbuf.s[2] |= 0x03;
   if(mask.s[3] != 0) cbuf.s[3] |= 0x03;
   if(mask.s[4] != 0) cbuf.s[4] |= 0x03;
   if(mask.s[5] != 0) cbuf.s[5] |= 0x03;
   if(mask.s[6] != 0) cbuf.s[6] |= 0x03;
   if(mask.s[7] != 0) cbuf.s[7] |= 0x03;
//   r.v = (mask.v != (v4si){0, 0, 0, 0, 0, 0, 0, 0});
//   r.v &= (v4si) {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
//   cbuf.v = cbuf.v |  r.v;
	
  return cbuf;
}





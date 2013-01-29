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

v4si *aPlanes;
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





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

extern v4si *aPlanes;


v8hi_t lshift_6bit8v_SSE2(v4hi *v)
{
   v4hi r;
   v4hi cbuf;
   v4hi mask;
   v8hi_t ret;
   mask.v = (v4si){0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8};
   cbuf.v =
        aPlanes[B2 + v->b[0]] |
        aPlanes[B3 + v->b[1]] |
        aPlanes[R0 + v->b[2]] |
        aPlanes[R1 + v->b[3]] |
        aPlanes[R2 + v->b[4]] |
        aPlanes[R3 + v->b[5]];
   
   mask.v = mask.v & cbuf.v;
#if ((__GNUC__ == 4) && (__GCC_MINOR__ >= 7)) || (__GNUC__ > 4) //GCC 4.7 or later.
   r.v = mask.v != (v4si){0, 0, 0, 0, 0, 0, 0, 0};
   r.v = r.v & (v4si) {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};
   cbuf.v = cbuf.v |  r.v;
#else
   if(mask.s[0] != 0) cbuf.s[0] |= 0x03;
   if(mask.s[1] != 0) cbuf.s[1] |= 0x03;
   if(mask.s[2] != 0) cbuf.s[2] |= 0x03;
   if(mask.s[3] != 0) cbuf.s[3] |= 0x03;
   if(mask.s[4] != 0) cbuf.s[4] |= 0x03;
   if(mask.s[5] != 0) cbuf.s[5] |= 0x03;
   if(mask.s[6] != 0) cbuf.s[6] |= 0x03;
   if(mask.s[7] != 0) cbuf.s[7] |= 0x03;
#endif	
   ret.i[0] = cbuf.s[0];
   ret.i[1] = cbuf.s[1];
   ret.i[2] = cbuf.s[2];
   ret.i[3] = cbuf.s[3];
   ret.i[4] = cbuf.s[4];
   ret.i[5] = cbuf.s[5];
   ret.i[6] = cbuf.s[6];
   ret.i[7] = cbuf.s[7];
   return ret;
}





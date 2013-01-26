/*
 * Copying Sound Buffer to Mix_Chunk.
 */

#include "xm7_types.h"
#include "xm7.h"
static inline Sint16 _clamp(Sint32 b)
{
    if(b < -0x7fff) return -0x7fff;
    if(b > 0x7fff) return 0x7fff;
    return (Sint16) b;
}

#ifndef v2si
typedef int v2si __attribute__ ((__vector_size__(8), aligned(8)));
typedef int16_t v4si_s __attribute__ ((__vector_size__(8), aligned(8)));
#endif

void CopySoundBufferGeneric(DWORD * from, WORD * to, int size)
{
    int         i, j;
    Sint32       *p = (Sint32 *) from;
    Sint16       *t = (Sint16 *) to;
    Sint32       tmp1;
    v8hi_t tmp2;
#ifdef __MMX__
    v4si_s *h;
    v4si_s *l;
    v4si_s r, s;
    v4hi tmp3;
    v4hi tmp4;
#else
    v4hi tmp3;
    v4hi *l;
    v8hi_t *h;
#endif

    if (p == NULL) {
        return;
    }
    if (t == NULL) {
        return;
    }
#ifdef __MMX__
    h = (v4si_s *)p;
    l = (v4si_s *)t;
    i = (size >> 3) << 3;
    for (j = 0; j < i; j += 8) {
       r = *h++;
       r = __builtin_ia32_paddsw((v4si_s){0, 0, 0, 0}, r);
       r = __builtin_ia32_psubsw(r, (v4si_s){0, 0, 0, 0});
       s = *h++;
       s = __builtin_ia32_paddsw((v4si_s){0, 0, 0, 0}, s);
       s = __builtin_ia32_psubsw(s, (v4si_s){0, 0, 0, 0});
       s = __builtin_shuffle(r, s, (v4si_s){0,2,4,6});
       *l++ = s;		    
       r = *h++;
       r = __builtin_ia32_paddsw((v4si_s){0, 0, 0, 0}, r);
       r = __builtin_ia32_psubsw(r, (v4si_s){0, 0, 0, 0});
       s = *h++;
       s = __builtin_ia32_paddsw((v4si_s){0, 0, 0, 0}, s);
       s = __builtin_ia32_psubsw(s, (v4si_s){0, 0, 0, 0});
       s = __builtin_shuffle(r, s, (v4si_s){0,2,4,6});
       *l++ = s;		    
    }
   
#else       
    h = (v8hi_t *)p;
    l = (v4hi *)t;
    i = (size >> 3) << 3;
    for (j = 0; j < i; j += 8) {
        tmp2 = *h++;
        tmp3.ss[0] =_clamp(tmp2.si[0]);
        tmp3.ss[1] =_clamp(tmp2.si[1]);
        tmp3.ss[2] =_clamp(tmp2.si[2]);
        tmp3.ss[3] =_clamp(tmp2.si[3]);
        tmp3.ss[4] =_clamp(tmp2.si[4]);
        tmp3.ss[5] =_clamp(tmp2.si[5]);
        tmp3.ss[6] =_clamp(tmp2.si[6]);
        tmp3.ss[7] =_clamp(tmp2.si[7]);
        *l++ = tmp3;
   }
#endif
   p = (Sint32 *)h;
   t = (Sint16 *)l;
   if(i >= size) return;
   for (j = 0; j < (size - i); j++) {
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
   }
}


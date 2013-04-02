/*
 * Copying Sound Buffer to Mix_Chunk.
 */

#include "xm7_types.h"
#include "xm7.h"
#include "sdl_cpuid.h"

extern struct XM7_CPUID *pCpuID;

static inline Sint16 _clamp(Sint32 b)
{
    if(b < -0x7fff) return -0x7fff;
    if(b > 0x7fff) return 0x7fff;
    return (Sint16) b;
}
#if defined(__x86_64__) || defined(__i386__)
 #if defined(__MMX__)
void CopySoundBuffer_MMX(DWORD *from, WORD *to, int size)
{
    v2ii *h ;
    v2ii r, s;
    v2hi *l;
    v2hi tt;
    Sint32 tmp1;
    Sint32 *p;
    Sint16 *t;
    int i, j;
   
    h = (v2ii *)from;
    l = (v2hi *)to;
    i = (size >> 2) << 2;
    for (j = 0; j < i; j += 4) {
      r = *h++;
      s = *h++;
      tt.v = __builtin_ia32_packssdw(r, s);
      *l++ = tt;
   }
   p = (Sint32 *)h;
   t = (Sint16 *)l;
   if(i >= size) return;
   for (j = 0; j < (size - i); j++) {
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
   }
   
}
 #else // NOT MMX
void CopySoundBuffer_MMX(DWORD *from, WORD *to, int size)
{
 // NOOP :)
}
 #endif // __MMX__
 #if 0
// By optimization, this occures assertion in gcc-4.8.0(why...)
volatile void CopySoundBuffer_SSE2(DWORD *from, WORD *to, int size)
{
    v4hi *h ;
    v4hi r, s;
    v4hi *l;
    v4hi tt;
    Sint32 tmp1;
    Sint32 *p;
    Sint16 *t;
    int i, j;
   
    h = (v4hi *)from;
    l = (v4hi *)to;
    i = (size >> 3) << 3;
    for (j = 0; j < i; j += 8) {
      r = *h++;
      s = *h++;
      tt.vv = __builtin_ia32_packssdw128(r.vv, s.vv);
      *l++ = tt;
   }
   p = (Sint32 *)h;
   t = (Sint16 *)l;
   if(i >= size) return;
   for (j = 0; j < (size - i); j++) {
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
   }
   
}
 #else // NOT MMX
volatile void CopySoundBuffer_SSE2(DWORD *from, WORD *to, int size)
{
   CopySoundBuffer_MMX(from, to, size);
 // NOOP :)
}
 #endif // __SSE2__
#endif // X86_64 or i386

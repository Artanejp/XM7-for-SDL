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
static void CopySoundBuffer_MMX(DWORD *from, WORD *to, int size)
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
static void CopySoundBuffer_MMX(DWORD *from, WORD *to, int size)
{
 // NOOP :)
}
 #endif // __MMX__
 #if defined(__SSE2__)
// By optimization, this occures assertion in gcc-4.8.0(why...)
static void CopySoundBuffer_SSE2(DWORD *from, WORD *to, int size)
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
static void CopySoundBuffer_SSE2(DWORD *from, WORD *to, int size)
{
 // NOOP :)
}
 #endif // __SSE2__
#endif // X86_64 or i386
void CopySoundBufferGeneric(DWORD * from, WORD * to, int size)
{
    int         i, j;
    Sint32       *p = (Sint32 *) from;
    Sint16       *t = (Sint16 *) to;
    Sint32       tmp1;
    struct XM7_CPUID cpuid;

#if (__GNUC__ >= 4)
    v8hi_t tmp2;
    v4hi tmp3;
    v4hi *l;
    v8hi_t *h;
    if (p == NULL) {
        return;
    }
    if (t == NULL) {
        return;
    }
 if(pCpuID != NULL) {
  #if defined(__SSE2__)
//    if(pCpuID->use_sse2) {
//	CopySoundBuffer_SSE2(from, to, size);
//        return;
//    }
 #endif
 #if defined(__MMX__)
    if(pCpuID->use_mmx) {
	CopySoundBuffer_MMX(from, to, size);
        return;
    }
 #endif
 }
   
 // Not using MMX or SSE2
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
   p = (Sint32 *)h;
   t = (Sint16 *)l;
   if(i >= size) return;
   for (j = 0; j < (size - i); j++) {
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
   }
#else // GCC <= 3.x
//   p = (Sint32 *)h;
//   t = (Sint16 *)l;
   i = size >> 3;
   for (j = 0; j < i; j++) {
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
   }
   i = j << 3;
   if(i >= size) return;
   for (j = 0; j < (size - i); j++) {
      tmp1 = *p++;
      *t++ = _clamp(tmp1);
   }
#endif   
}


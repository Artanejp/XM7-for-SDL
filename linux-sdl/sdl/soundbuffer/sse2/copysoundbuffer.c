/*
 * Copying Sound Buffer to Mix_Chunk.
 */

#include "xm7_types.h"
#include "xm7.h"
#include "sdl_cpuid.h"

extern struct XM7_CPUID *pCpuID;
extern void CopySoundBuffer_MMX(DWORD *from, WORD *to, int size);

static inline Sint16 _clamp(Sint32 b)
{
    if(b < -0x7fff) return -0x7fff;
    if(b > 0x7fff) return 0x7fff;
    return (Sint16) b;
}
#if defined(__x86_64__) || defined(__i386__)
 #if defined(__SSE2__)
 int AddSoundBuffer_SSE2(Sint16 *dst, Sint32 *opnsrc, Sint16 *beepsrc, Sint16 *cmtsrc, Sint16 *wavsrc, int samples)
 {
   int len1, len2;
   int i;
   if(samples <= 0) return 0;
   if(dst == NULL) return 0;
//   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL) || (wavsrc == NULL)) return 0;
   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL)) return 0;
   
   len1 = samples / 8;
   len2 = samples % 8;
#if (__GNUC__ >= 4)
    v4hi t1, t2;
    v4hi tt;
    v4hi *l;
    v4hi *opn  = (v4hi *)opnsrc;
    v4hi *beep = (v4hi *)beepsrc;
    v4hi *cmt  = (v4hi *)cmtsrc;
    v4hi *wav  = (v4hi *)wavsrc;
    v4hi *p    = (v4hi *)dst;
   for(i = 0; i < len1; i++) {
        t1 = *opn;
        opn++;
        t2 = *opn;
        opn++;
        tt.vv = __builtin_ia32_packssdw128(t1.vv, t2.vv);
        tt.vv = __builtin_ia32_paddsw128(tt.vv, beep->vv);
        tt.vv = __builtin_ia32_paddsw128(tt.vv, cmt->vv);
//        tt.vv = __builtin_ia32_paddsw128(tt.vv, wav->vv);
        beep++;
        cmt++;
//      wav++;
        p->vv = tt.vv;
        __builtin_prefetch(p, 1, 1);
        p++;
   }
#endif   
   if(len2 <= 0) return len1 * 8;
   {
      Sint32 tmp4;
      Sint16 tmp5;
      Sint32 *opn2 = (Sint32 *)opn;
      Sint16 *beep2 = (Sint16 *)beep;
      Sint16 *cmt2 = (Sint16 *)cmt;
      Sint16 *wav2 = (Sint16 *)wav;
      Sint16 *dst2 = (Sint16 *)p;
      for (i = 0; i < len2; i++) {
	 tmp4 = *opn2++;
	 tmp5 = _clamp(tmp4);
	 tmp5 = tmp5 + *beep2++;
	 tmp5 = tmp5 + *cmt2++;
//	 tmp5 = tmp5 + *wav2++;
        __builtin_prefetch(dst2, 1, 1);
	 *dst2++ = tmp5;
      }
   }
   return len2 + len1 * 8;
 }


 #endif
#endif

#if defined(__x86_64__) || defined(__i386__)
 #if defined(__SSE2__)
// By optimization, this occures assertion in gcc-4.8.0(why...)
void CopySoundBuffer_SSE2(DWORD *from, WORD *to, int size)
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
        __builtin_prefetch(l, 1, 1);
      *l++ = tt;
   }
   p = (Sint32 *)h;
   t = (Sint16 *)l;
   if(i >= size) return;
   for (j = 0; j < (size - i); j++) {
      tmp1 = *p++;
        __builtin_prefetch(p, 1, 1);
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

/*
 * Copying Sound Buffer to Mix_Chunk.
 */

#include "xm7_types.h"
#include "xm7.h"
#include "sdl_cpuid.h"

extern struct XM7_CPUID *pCpuID;

#ifdef USE_SSE2
extern volatile void CopySoundBuffer_SSE2(DWORD *from, WORD *to, int size);
extern int AddSoundBuffer_MMX(Sint16 *dst, Sint32 *opnsrc, Sint16 *beepsrc, Sint16 *cmtsrc, Sint16 *wavsrc, int samples);
#endif
#ifdef USE_MMX
extern void void CopySoundBuffer_MMX(DWORD *from, WORD *to, int size);
extern int AddSoundBuffer_MMX(Sint16 *dst, Sint32 *opnsrc, Sint16 *beepsrc, Sint16 *cmtsrc, Sint16 *wavsrc, int samples);
#endif



static inline Sint16 _clamp(Sint32 b)
{
    if(b < -0x7fff) return -0x7fff;
    if(b > 0x7fff) return 0x7fff;
    return (Sint16) b;
}

int AddSoundBuffer(Sint16 *dst, Sint32 *opnsrc, Sint16 *beepsrc, Sint16 *cmtsrc, Sint16 *wavsrc, int samples)
{
   int len1, len2;
   int i;
   if(samples <= 0) return 0;
   if(dst == NULL) return 0;
//   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL) || (wavsrc == NULL)) return 0;
   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL)) return 0;
 if(pCpuID != NULL) {
 #if defined(USE_SSE2)
    if(pCpuID->use_sse2) {
	return AddSoundBuffer_MMX(dst, opnsrc, beepsrc, cmtsrc, wavsrc, samples);
    }
 #endif
 #if defined(USE_MMX)
    if(pCpuID->use_mmx) {
       return AddSoundBuffer_MMX(dst, opnsrc, beepsrc, cmtsrc, wavsrc, samples);
    }
 #endif
 }
   
   
   len1 = samples / 8;
   len2 = samples % 8;
#if (__GNUC__ >= 4)
    v8hi_t tmp2;
    v4hi tmp3;
    v4hi *l;
    v8hi_t *opn  = (v8hi_t *)opnsrc;
    v4hi *beep = (v4hi *)beepsrc;
    v4hi *cmt  = (v4hi *)cmtsrc;
    v4hi *wav  = (v4hi *)wavsrc;
    v4hi *p    = (v4hi *)dst;
   for(i = 0; i < len1; i++) {
        tmp2 = *opn++;
        tmp3.ss[0] =_clamp(tmp2.si[0]);
        tmp3.ss[1] =_clamp(tmp2.si[1]);
        tmp3.ss[2] =_clamp(tmp2.si[2]);
        tmp3.ss[3] =_clamp(tmp2.si[3]);
        tmp3.ss[4] =_clamp(tmp2.si[4]);
        tmp3.ss[5] =_clamp(tmp2.si[5]);
        tmp3.ss[6] =_clamp(tmp2.si[6]);
        tmp3.ss[7] =_clamp(tmp2.si[7]);
        tmp3.v = tmp3.v + beep->v;
        beep++;
        tmp3.v = tmp3.v + cmt->v;
        cmt++;
//        tmp3.v = tmp3.v + wav->v;
//        wav++;
        p->v = tmp3.v;
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
	 *dst2++ = tmp5;
      }
   }
   return len2 + len1 * 8;
}


void CopySoundBufferGeneric(DWORD * from, WORD * to, int size)
{
    int         i,j;
    Sint32 tmp1;
    Sint32 *p = (Sint32 *)from;
    Sint16 *t = (Sint16 *)to;
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
  #if defined(USE_SSE2)
    if(pCpuID->use_sse2) {
	CopySoundBuffer_SSE2(from, to, size);
        return;
    }
 #endif
 #if defined(USE_MMX)
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


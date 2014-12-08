/*
 * Copying Sound Buffer to Mix_Chunk.
 */

#include "xm7_types.h"
#include "xm7.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern struct XM7_CPUID *pCpuID;
extern void CopySoundBuffer_MMX(DWORD *from, WORD *to, int size);

static inline Sint16 _clamp(Sint32 b)
{
    if(b < -0x7fff) return -0x7fff;
    if(b > 0x7fff) return 0x7fff;
    return (Sint16) b;
}

static v4hi _get_unaligned_int16(Sint16 *p)
{
   v4hi v __attribute__((aligned(16)));
   
//   if(__builtin_expect((((uint64_t)p & 0x0f) == 0), 1)){
      v = *((v4hi *)p);
//   } else {
//      v.ss[0] = *p++;
//      v.ss[1] = *p++;
//      v.ss[2] = *p++;
//      v.ss[3] = *p++;
//      v.ss[4] = *p++;
//      v.ss[5] = *p++;
//      v.ss[6] = *p++;
//      v.ss[7] = *p++;
//   }
   return v;
}

static v4hi _get_unaligned_int32(Sint32 *p)
{
   v4hi v __attribute__((aligned(16)));
#if 0   
   if(__builtin_expect((((uint64_t)p & 0x0f) == 0), 1)){
      v = *((v4hi *)p);
   } else {
      v.si[0] = p[0];
      v.si[1] = p[1];
      v.si[2] = p[2];
      v.si[3] = p[3];
   }
#else
   v4hi *s = (v4hi *)p;
   asm volatile("movdqu %[x], %%xmm0\n\t"
		"movdqu %%xmm0, %[y]"
		: [y] "=rm" (v)
		: [x] "rm" (*s)
		: "xmm0");
   
#endif
   return v;
}

static void  _put_unaligned_int16(Sint16 *dst, v4hi v)
{
#if 0   
   if(__builtin_expect((((uint64_t)dst & 0x0f) == 0), 1)){
      *((v4hi *)dst) = v;
   } else {
      dst[0] = v.ss[0];
      dst[1] = v.ss[1];
      dst[2] = v.ss[2];
      dst[3] = v.ss[3];
      dst[4] = v.ss[4];
      dst[5] = v.ss[5];
      dst[6] = v.ss[6];
      dst[7] = v.ss[7];
   }
#else
   v4hi *d = (v4hi *)dst;
   asm volatile("movdqu %[y], %%xmm0\n\t"
		"movdqu %%xmm0, %[x]"
		: [x] "=rm" (*d)
		: [y] "rm" (v)
		: "xmm0"
		);
#endif
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
#if 0
    Sint32 *opn  = opnsrc;
    Sint16 *beep = beepsrc;
    Sint16 *cmt  = cmtsrc;
    Sint16 *wav  = wavsrc;
    Sint16 *p    = dst;
    v4hi vtmp;
#else
    v4hi *opn = (v4hi *)opnsrc;
    v4hi *beep = (v4hi *)beepsrc;
    v4hi *cmt = (v4hi *)cmtsrc;
    v4hi *wav = (v4hi *)wavsrc;
    v4hi *opnb = &opn[1];
    v4hi *p    = (v4hi *)dst;

#endif
   _prefetch_data_write_l1(p, samples * sizeof(Sint16));
   _prefetch_data_read_l2(opn, sizeof(Sint32) * samples);
   _prefetch_data_read_l2(beep, sizeof(Sint16) * samples);
   _prefetch_data_read_l2(cmt, sizeof(Sint16) * samples);
   _prefetch_data_read_l2(wav, sizeof(Sint16) * samples);
   for(i = 0; i < len1; i++) {
#if 0
        t1 = _get_unaligned_int32(opn);
        t2 = _get_unaligned_int32(opn + 4);
        opn += 8;
        tt.vv = __builtin_ia32_packssdw128(t1.vv, t2.vv);
        vtmp = _get_unaligned_int16(beep);
        tt.vv = __builtin_ia32_paddsw128(tt.vv, vtmp.vv);
        vtmp = _get_unaligned_int16(cmt);
        tt.vv = __builtin_ia32_paddsw128(tt.vv, vtmp.vv);
        vtmp = _get_unaligned_int16(wav);
        tt.vv = __builtin_ia32_paddsw128(tt.vv, vtmp.vv);
        _put_unaligned_int16(p, tt);
        opn += 8;
        beep += 8;
        cmt += 8;
        wav += 8;
        p += 8;

#else
      // Assembler syntax is ATT, not MS, NASM, YSAM.
      asm volatile("movdqu %[opn1],  %%xmm1\n\t"
		   "movdqu %[opn2],  %%xmm0\n\t"
		   "packssdw %%xmm0, %%xmm1\n\t"
		   "movdqu %[beep1], %%xmm0\n\t"
		   "paddsw %%xmm0, %%xmm1\n\t"
		   "movdqu %[cmt1],  %%xmm0\n\t"
		   "paddsw %%xmm0, %%xmm1\n\t"
		   "movdqu %[wav1],  %%xmm0\n\t"
		   "paddsw %%xmm0, %%xmm1\n\t"
		   "movdqu %%xmm1, %[dstp]"
		   : [dstp] "=rm" (*p)
		   : [opn1] "rm" (*opn),
		     [opn2] "rm" (*opnb),
		     [beep1] "rm" (*beep),
		     [cmt1] "rm" (*cmt),
		     [wav1] "rm" (*wav)
		   : "xmm0", "xmm1");
      p++;
      opn  = &opn[2];
      opnb = &opnb[2];
      beep++;
      cmt++;
      wav++;
#endif
   }
#endif   
//   if(len2 <= 0) return len1 * 8;
   if(__builtin_expect((len2 > 0), 0))
   {
      Sint32 tmp4;
      Sint32 *opn2 = (Sint32 *)opn;
      Sint16 *beep2 = (Sint16 *)beep;
      Sint16 *cmt2 = (Sint16 *)cmt;
      Sint16 *wav2 = (Sint16 *)wav;
      Sint16 *dst2 = (Sint16 *)p;
      for (i = 0; i < len2; i++) {
	 tmp4 = *opn2++;
	 tmp4 += (Sint32)*beep2++;
	 tmp4 += (Sint32)*cmt2++;
	 tmp4 += (Sint32)*wav2++;	 
	 *dst2++ = _clamp(tmp4);
      }
      return len2 + len1 * 8;
   } else {
      return len1 * 8;
   }
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
    _prefetch_data_write_l1(l, sizeof(Uint16) * size);
    _prefetch_data_read_l2(h, sizeof(Uint32) * size);
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

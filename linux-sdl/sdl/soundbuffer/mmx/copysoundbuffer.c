/*
 * Copying Sound Buffer to Mix_Chunk.
 */

#include "xm7_types.h"
#include "xm7.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern struct XM7_CPUID *pCpuID;

static inline Sint16 _clamp(Sint32 b)
{
    if(b < -0x7fff) return -0x7fff;
    if(b > 0x7fff) return 0x7fff;
    return (Sint16) b;
}
#if defined(__x86_64__) || defined(__i386__)
 #if defined(__MMX__)
int AddSoundBuffer_MMX(Sint16 *dst, Sint32 *opnsrc, Sint16 *beepsrc, Sint16 *cmtsrc, Sint16 *wavsrc, int samples)
{
   int len1, len2;
   int i;
   if(samples <= 0) return 0;
   if(dst == NULL) return 0;
//   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL) || (wavsrc == NULL)) return 0;
   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL)) return 0;
   
   len1 = samples / 4;
   len2 = samples % 4;
#if (__GNUC__ >= 4)
    v2ii t1, t2;
    v2hi tt;
    v4hi *l;
    v2hi *opn  = (v2hi *)opnsrc;
    v2hi *beep = (v2hi *)beepsrc;
    v2hi *cmt  = (v2hi *)cmtsrc;
    v2hi *wav  = (v2hi *)wavsrc;
    v2hi *p    = (v2hi *)dst;
   _prefetch_data_write_l1(p, sizeof(v4hi) * len1 + sizeof(Sint16) * len2);
   _prefetch_data_read_l2(opn, sizeof(Sint32) * samples);
   _prefetch_data_read_l2(beep, sizeof(Sint16) * samples);
   _prefetch_data_read_l2(cmt, sizeof(Sint16) * samples);
//   _prefetch_data_read_l2(wav, sizeof(Sint16) * samples);
   for(i = 0; i < len1; i++) {
        t1 = opn->v;
        opn++;
        t2 = opn->v;
        opn++;
        tt.v = __builtin_ia32_packssdw(t1, t2);

        tt.v = tt.v + beep->v;
        beep++;
        tt.v = tt.v + cmt->v;
        cmt++;
//      tt.v = tt.v + *wav;
//      wav++;
        p->v = tt.v;
        p++;
   }
#endif   
   if(len2 <= 0) return len1 * 4;
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
   return len2 + len1 * 4;
}
 #endif // __MMX__
#endif

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
    _prefetch_data_write_l1(l, sizeof(Uint16) * size);
    _prefetch_data_read_l2(h, sizeof(Uint32) * size);
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
#endif // X86_64 or i386

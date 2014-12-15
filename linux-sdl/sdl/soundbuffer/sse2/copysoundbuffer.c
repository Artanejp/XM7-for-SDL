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
#ifndef __x86_64__   
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
   asm volatile("movq  %[x], %%rsi\n\t"
		"movdqu 0(%%rsi), %%xmm0\n\t"
		"movdqu %%xmm0, %[y]"
		: [y] "=rm" (v)
		: [x] "rm" (s)
		: "xmm0", "rsi");
   
#endif
   return v;
}

static void  _put_unaligned_int16(Sint16 *dst, v4hi v)
{
#ifndef __x86_64__   
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
   asm volatile("movq %[x], %%rdi\n\t"
		"movq %[y], %%rsi\n\t"
		"movdqu 0(%%rsi), %%xmm0\n\t"
		"movdqu %%xmm0, 0(%%rdi)\n\t"
		: 
		: [y] "rm" (v), [x] "rm" (d)
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
   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL) || (wavsrc == NULL)) return 0;
//   if((opnsrc == NULL) || (beepsrc == NULL) || (cmtsrc == NULL)) return 0;

    
   len1 = samples / 8;
   len2 = samples % 8;
#if (__GNUC__ >= 4)
    v4hi t1, t2;
    v4hi tt;
#ifndef __x86_64__
    Sint32 *opn  = opnsrc;
    Sint16 *beep = beepsrc;
    Sint16 *cmt  = cmtsrc;
    Sint16 *wav  = wavsrc;
    Sint16 *p    = dst;
    v4hi vtmp;
#endif
#ifndef __x86_64__
   _prefetch_data_write_l1(dst, samples * sizeof(Sint16));
   _prefetch_data_read_l2(opnsrc, sizeof(Sint32) * samples);
   _prefetch_data_read_l2(beepsrc, sizeof(Sint16) * samples);
   _prefetch_data_read_l2(cmtsrc, sizeof(Sint16) * samples);
   _prefetch_data_read_l2(wavsrc, sizeof(Sint16) * samples);
   for(i = 0; i < len1; i++) {
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
   }
   opnsrc =  (Sint32 *)opn;
   beepsrc = (Sint16 *)beep;
   cmtsrc =  (Sint16 *)cmt;
   wavsrc =  (Sint16 *)wav;
   dst    =  (Sint16 *)p;
#else
	 
   _prefetch_data_write_l1(dst, samples * sizeof(Sint16));
//   _prefetch_data_read_l2(opnsrc, sizeof(Sint32) * samples);
//   _prefetch_data_read_l2(beepsrc, sizeof(Sint16) * samples);
//   _prefetch_data_read_l2(cmtsrc, sizeof(Sint16) * samples);
//   _prefetch_data_read_l2(wavsrc, sizeof(Sint16) * samples);
      // Assembler syntax is GAS/ATT, not MS, NASM, YSAM.
      // 
   if(len1 > 0) {
      asm volatile(
		   "movl %[len], %%ecx\n\t"
		   "cmpl $0, %%ecx\n\t"
		   "movl %%ecx, %%eax\n\t"
		   "andl $1, %%eax\n\t"
		   "movq %[dst], %%rdi\n\t"
		   "movq %[opn1], %%r8\n\t"
		   "movq %[beep1], %%r9\n\t"
		   "movq %[cmt1], %%r10\n\t"
		   "movq %[wav1], %%r11\n"
		   "shrl $1, %%ecx\n\t"
		   "cmpl $0, %%ecx\n\t"
		   "jz _l2\n\t"
		   
		   "_l1:\n\t"
		   "movdqu  0(%%r8),  %%xmm1\n\t"
		   "movdqu 16(%%r8),  %%xmm0\n\t"
		   "packssdw %%xmm0,  %%xmm1 ; /* OPN */\n\t"
		   "movdqu 0(%%r9),   %%xmm2 ; /* BEEP */\n\t"
		   "paddsw %%xmm2, %%xmm1\n\t"
		   "movdqu 0(%%r10),  %%xmm3 ; /* CMT */\n\t"
		   "paddsw %%xmm3, %%xmm1\n\t"
		   "movdqu 0(%%r11),  %%xmm4 ; /* WAV */\n\t"
		   "paddsw %%xmm4, %%xmm1\n\t"
		   "movdqu %%xmm1, 0(%%rdi)  ; /* store */"

		   "movdqu 32(%%r8),  %%xmm5\n\t"
		   "movdqu 48(%%r8),  %%xmm6\n\t"
		   "packssdw %%xmm5,  %%xmm6 ; /* OPN */\n\t"
		   "movdqu 16(%%r9),   %%xmm7 ; /* BEEP */\n\t"
		   "paddsw %%xmm7, %%xmm6\n\t"
		   "movdqu 16(%%r10),  %%xmm8 ; /* CMT */\n\t"
		   "paddsw %%xmm8, %%xmm6\n\t"
		   "movdqu 16(%%r11),  %%xmm9 ; /* WAV */\n\t"
		   "paddsw %%xmm9, %%xmm6\n\t"
		   "movdqu %%xmm6, 16(%%rdi)  ; /* store */"
		   "addq $64, %%r8\n\t"
		   "addq $32, %%r9\n\t"
		   "addq $32, %%r10\n\t"
		   "addq $32, %%r11\n\t"
		   "addq $32, %%rdi\n\t"
		   "dec %%ecx\n\t"
		   "jnz _l1\n\t"
		   
		   "cmpl $0, %%eax\n\t"
		   "je _l3\n\t"
		   "_l2:\n\t"
		   "movdqu  0(%%r8),  %%xmm1\n\t"
		   "movdqu 16(%%r8),  %%xmm0\n\t"
		   "packssdw %%xmm0,  %%xmm1 ; /* OPN */\n\t"
		   "movdqu 0(%%r9),   %%xmm2 ; /* BEEP */\n\t"
		   "paddsw %%xmm2, %%xmm1\n\t"
		   "movdqu 0(%%r10),  %%xmm3 ; /* CMT */\n\t"
		   "paddsw %%xmm3, %%xmm1\n\t"
		   "movdqu 0(%%r11),  %%xmm4 ; /* WAV */\n\t"
		   "paddsw %%xmm4, %%xmm1\n\t"
		   "movdqu %%xmm1, 0(%%rdi)\n\t  ; /* store */"
		   "_l3:\n\t"
		   : 
		   : [dst]  "rm"  (dst), [opn1] "rm" (opnsrc), [beep1] "rm" (beepsrc), 
		     [cmt1] "rm" (cmtsrc),[wav1] "rm" (wavsrc), [len] "rm" (len1)
		   : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", 
		     "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", 
		     "rax", "rcx", "rdi", "r8", "r9", "r10", "r11");
   }
    dst     = &dst[len1 * 8];
    opnsrc  = &opnsrc[len1 * 8];
    beepsrc = &beepsrc[len1 * 8];
    cmtsrc  = &cmtsrc[len1 * 8];
    wavsrc  = &wavsrc[len1 * 8];
#endif
#endif   
//   if(len2 <= 0) return len1 * 8;
   if(__builtin_expect((len2 > 0), 0))
   {
      Sint32 tmp4;
      Sint32 *opn2 = (Sint32 *)opnsrc;
      Sint16 *beep2 = (Sint16 *)beepsrc;
      Sint16 *cmt2 = (Sint16 *)cmtsrc;
      Sint16 *wav2 = (Sint16 *)wavsrc;
      Sint16 *dst2 = (Sint16 *)dst;
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

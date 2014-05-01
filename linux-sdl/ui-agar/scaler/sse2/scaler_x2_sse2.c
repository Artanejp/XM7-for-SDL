/*
 * Zoom x2x2
 * (C) 2013 K.Ohta
 * 
 * History:
 *  2013-04-02 Move from scaler_x2.c
 */
#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"

extern struct XM7_CPUID *pCpuID;

extern void pVram2RGB_x2(Uint32 *src, Uint32 *dst, int x, int y, int yrep);

#if defined(__SSE2__)
void pVram2RGB_x2_SSE2(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   register v4hi *b;
   AG_Surface *Surface = GetDrawSurface();
   
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   int w = 0;
   int h = 0;
   int yy;
   int xx;
   int hh;
   int ww;
   int i;
   int pitch;
   Uint32 black;

   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;
#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
   if(yrep < 2) {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 2 * Surface->format->BytesPerPixel
                        + y * Surface->pitch);
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 2 * Surface->format->BytesPerPixel
                        + y * (yrep >> 1) * Surface->pitch);
   }

   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   if(w < (x * 2 + 15)) {
    int j;
    register Uint32 d0;

    p = src;
    ww = w - x * 2;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 2, i++){
            d2 = d1;
            d0 = p[i];
            for(j = 0; j < (yrep >> 1); j++){
	       if((j >= (yrep >> 2)) && !bFullScan){
		  d2[xx] = d2[xx +1] = black;
	       } else {
		  d2[xx] = d0;
		  d2[xx + 1] = d0;
	       }
                d2 += pitch;
            }
        }
        d1 += (pitch * (yrep >> 1));
        if(yrep & 1) d1 += pitch;
        p += 8;
      }
   } else { // inside align
    int j;
    register v4hi b2;
    register v4hi b3;
    register v4hi b4;
    register v4hi b5;
    register v4hi bb;
    register v4hi btmp;
    v4hi *b2p;
    b = (v4hi *)src;
    bb.vv = (v4ii){black, black, black, black};
       switch(yrep) {
	case 0:
	case 1:
	case 2:
	  for(yy = 0; yy < hh; yy++){
	     b2p = (v4hi *)d1;
	     btmp = b[0];
	     b2.v = __builtin_ia32_pshufd(btmp.v, 0x50);
	     b3.v = __builtin_ia32_pshufd(btmp.v, 0xfa);
	     *(b2p++) = b2;
	     *(b2p++) = b3;
	     
	     btmp = b[1];
	     b4.v = __builtin_ia32_pshufd(btmp.v, 0x50);
	     b5.v = __builtin_ia32_pshufd(btmp.v, 0xfa);
	     *(b2p++) = b4;
	     *b2p = b5;
	     d1 += pitch;
	     b += 2;
	  }
	  break;
	default:
	  for(yy = 0; yy < hh; yy++){
	     b2.v = __builtin_ia32_pshufd(b[0].v, 0x50);
	     b3.v = __builtin_ia32_pshufd(b[0].v, 0xfa);
	     b4.v = __builtin_ia32_pshufd(b[1].v, 0x50);
	     b5.v = __builtin_ia32_pshufd(b[1].v, 0xfa);
	     for(j = 0; j < (yrep >> 1); j++) {
		b2p = (v4hi *)d1;
		if(!bFullScan && (j >= (yrep >> 2))) {
		   b2p[0] = 
		   b2p[1] = 
		   b2p[2] = 
		   b2p[3] = bb;
		 } else {
		    *(b2p++) = b2;
		    *(b2p++) = b3;
		    *(b2p++) = b4;
		    *b2p = b5;
		}
		d1 += pitch;
	     }
	     b += 2;
	  }

	  break;
       }

   }
}

void pVram2RGB_x2_Line_SSE2(Uint32 *src, int xbegin, int xend, int y, int yrep)
{
   register v4hi *b;
   AG_Surface *Surface = GetDrawSurface();
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   int w;
   int h;
   int yy;
   int xx;
   int hh;
   int ww;
   int i;
   int x = xbegin;
   unsigned  pitch;
   Uint32 black;
   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;
   
   ww = xend - xbegin;
   if(ww <= 0) return;
   
#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
   if(yrep < 2) {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 2 * Surface->format->BytesPerPixel
                        + y * Surface->pitch);
      d2 = &src[x + y * 640];
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 2 * Surface->format->BytesPerPixel
                        + y * (yrep >> 1) * Surface->pitch);
      d2 = &src[x + y * 640];
   }

   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   for(i = 0; i < ww; i++) __builtin_prefetch(&d2[i], 1, 1);
   { // Not thinking align ;-(
	
    int j;
    v4hi b2;
    v4hi b3;
    v4hi b4;
    v4hi b5;
    register v4hi bb;
    v4hi *b2p;
    Uint32 *d0;
      
    b = (v4hi *)d2;
    bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] = black;
       switch(yrep) {
	case 0:
	case 1:
	case 2:
	  for(xx = 0; xx < ww; xx += 8) {
	     b2p = (v4hi *)d1;
	     b2.i[0] = b2.i[1] = b[0].i[0];
	     b2.i[2] = b2.i[3] = b[0].i[1];
	     b3.i[0] = b3.i[1] = b[0].i[2];
	     b3.i[2] = b3.i[3] = b[0].i[3];

	     b4.i[0] = b4.i[1] = b[1].i[0];
	     b4.i[2] = b4.i[3] = b[1].i[1];
	     b5.i[0] = b5.i[1] = b[1].i[2];
	     b5.i[2] = b5.i[3] = b[1].i[3];
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     b2p[3] = b5;
	     d1 += 16;
	     b += 2;
	  }
	  break;
	default:
	  d0 = d1;
	  for(xx = 0; xx < ww; xx += 8){
	     d1 = d0;
	     b2.i[0] = b2.i[1] = b[0].i[0];
	     b2.i[2] = b2.i[3] = b[0].i[1];
	     b3.i[0] = b3.i[1] = b[0].i[2];
	     b3.i[2] = b3.i[3] = b[0].i[3];
	     
	     b4.i[0] = b4.i[1] = b[1].i[0];
	     b4.i[2] = b4.i[3] = b[1].i[1];
	     b5.i[0] = b5.i[1] = b[1].i[2];
	     b5.i[2] = b5.i[3] = b[1].i[3];

	     for(j = 0; j < (yrep >> 1); j++) {
		b2p = (v4hi *)d1;
		if(!bFullScan && (j >= (yrep >> 2))) {
		   b2p[0] = 
		   b2p[1] = 
		   b2p[2] = 
		   b2p[3] = bb;
		 } else {
		   b2p[0] = b2;
		   b2p[1] = b3;
		   b2p[2] = b4;
		   b2p[3] = b5;
		}
		d1 += pitch;
	     }
	     d0 += 16;
	     b += 2;
	  }

	  break;
       }

   }
}


#else 
void pVram2RGB_x2_SSE(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   pVram2RGB_x2(src, dst, x, y, yrep);
}

void pVram2RGB_x2_Line_SSE2(Uint32 *src, int xbegin,  int xend, int y, int yrep)
{
   pVram2RGB_x2_Line(src, dst, x, y, yrep);
}

#endif // __SSE2__
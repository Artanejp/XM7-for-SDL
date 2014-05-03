/*
 * Zoom x1x1
 * (C) 2014 K.Ohta
 * 
 * History:
 *  2013-01-26 Move from agar_sdlscaler.cpp
 */

#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"

extern struct XM7_CPUID *pCpuID;

void pVram2RGB_x1_SSE2(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi_t *b;
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
   int pitch;
   Uint32 black;
   AG_Surface *Surface = GetDrawSurface();
    
   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;
   
#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif

   if(yrep < 2) {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * Surface->format->BytesPerPixel
                        + y * Surface->pitch);
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * Surface->format->BytesPerPixel
                        + y * (yrep >> 1) * Surface->pitch);
   }


   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   if(w < (x  + 7)) {
    int j;
    Uint32 d0;

    p = src;
    ww = w - x;

    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx ++, i++){
            d2 = d1;
            d0 = p[i];
            for(j = 0; j < (yrep >> 1); j++){
	       if((j > (yrep >> 2)) && !bFullScan){
		  d2[xx] = black;
	       } else {
		  d2[xx] = d0;
	       }

                d2 += pitch;
            }
        }
        d1 += (pitch * (yrep >> 1));
        if(yrep & 1) d1 += pitch;
        p += 8;
      }
   } else { // inside align
      v8hi_t *bv;
      v8hi_t bb;
      int j;
      bb.i[0] = bb.i[1] =
      bb.i[2] = bb.i[3] =
      bb.i[4] = bb.i[5] =
      bb.i[6] = bb.i[7] = black;
        b = (v8hi_t *)src;
        for(yy = 0; yy < hh; yy++){
	   switch(yrep) {
	    case 0:
	    case 1:
	    case 2:
	      bv = (v8hi_t *)d1;
	      *bv = *b;
	      d1 += pitch;
	      break;
	    default:
	      if(yrep & 1) {
		 bv = (v8hi_t *)d1;
		 *bv = *b;
		 d1 += pitch;
		 yrep--;
	      }
	      for(j = 0; j < (yrep >> 1); j++) {
		 bv = (v8hi_t *)d1;
		 if(!bFullScan && (j >= (yrep >> 2))) {
		    *bv = bb;
		 } else {
		    *bv = *b;
		 }

		 d1 += pitch;
	      }
	      break;
	   }
        b++;
        }
    }
}

void pVram2RGB_x1_Line_SSE2(Uint32 *src, int xbegin, int xend, int y, int yrep)
{
   register v8hi_t *b;
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
   if(yrep < 2) {
      if(y >= h) return;
   } else {
      if(y >= (h / (yrep >> 1))) return;
   }
   
   
#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
   if(yrep < 2) {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * Surface->format->BytesPerPixel
                        + y * Surface->pitch);
      d2 = &src[x + y * 640];
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * Surface->format->BytesPerPixel
                        + y * (yrep >> 1) * Surface->pitch);
      d2 = &src[x + y * 640];
   }

   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   { // Not thinking align ;-(
	
    int j;
    v8hi_t b2;
    register v8hi_t bb;
    v8hi_t *b2p;
    Uint32 *d0;
      
    b = (v8hi_t *)d2;
    
    bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] =
    bb.i[4] = bb.i[5] = bb.i[6] = bb.i[7] = black;
      switch(yrep) {
	case 0:
	case 1:
	case 2:
	  for(xx = 0; xx < ww; xx += 8) {
	     b2p = (v8hi_t *)d1;
	     *b2p = *b;
	     d1 += 8;
	     b++;
	  }
	  break;
	default:
	  d0 = d1;
	  for(xx = 0; xx < ww; xx += 8){
	     d1 = d0;
	     b2 = *b;

	     for(j = 0; j < (yrep >> 1); j++) {
		b2p = (v8hi_t *)d1;
		if(!bFullScan && (j >= (yrep >> 2))) {
		   *b2p = bb;
		 } else {
		   *b2p = b2;
		}
		d1 += pitch;
	     }
	     d0 += 8;
	     b++;
	  }

	  break;
       }

   }
}


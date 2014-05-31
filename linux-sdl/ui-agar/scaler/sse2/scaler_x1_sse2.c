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
#include "cache_wrapper.h"

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
        _prefetch_data_write_l2(d1, sizeof(Uint32) * ww);
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
	      _prefetch_data_write_l2(d1, sizeof(v8hi_t));
	      bv = (v8hi_t *)d1;
	      *bv = *b;
	      d1 += pitch;
	      break;
	    default:
	      if(yrep & 1) {
		 _prefetch_data_write_l2(d1, sizeof(v8hi_t));
		 bv = (v8hi_t *)d1;
		 *bv = *b;
		 d1 += pitch;
		 yrep--;
	      }
	      for(j = 0; j < (yrep >> 1); j++) {
		 _prefetch_data_write_l2(d1, sizeof(v8hi_t));
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

static void Scaler_DrawLine(v4hi *dst, Uint32 *src, int ww, int repeat, int rep0, int pitch)
{
   int xx;
   int yy;
   int yrep2;
   int yrep3;
   int blank = rep0 - repeat;
   register v4hi *b2p;
   register v4hi r1, r2;
   register v4hi *d0;
   register v4hi *b;
   register v4hi bb2;
#if AG_BIG_ENDIAN != 1
   const v4ui bb = {0xff000000, 0xff000000, 0xff000000, 0xff000000};
#else
   const v4ui bb = {0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff};
#endif
     
   if(repeat < 0) return;
   b = (v4hi *)src;
    
   b2p = dst;
   if(rep0 < 3) {
      for(xx = 0; xx < ww; xx += 8) {
	    r1 = b[0];
	    r2 = b[1];
	    b2p[0] = r1;
	    b2p[1] = r2;
	    b2p += 2;
	    b += 2;
      }
   } else {
      if(rep0 < 2) {
	 _prefetch_data_write_l2(b2p, sizeof(v4hi) * 2 * ww);
	 for(xx = 0; xx < ww; xx += 8) {
	    r1 = b[0];
	    r2 = b[1];
	    b2p[0] = r1;
	    b2p[1] = r2;
	    b2p += 2;
	    b += 2;
	 }
      } else {
	 if(bFullScan) {
	    yrep2 = repeat;
	 } else {
	    yrep2 = repeat >> 1;
	    blank = blank >> 1;
	 }
	 d0 = b2p;
	 pitch = pitch / (sizeof(v4hi));
	 for(xx = 0; xx < ww; xx += 8) {
	    b2p = d0;
	    r1 = b[0];
	    r2 = b[1];
	    for(yy = 0; yy < yrep2; yy++) {
	       _prefetch_data_write_l2(b2p, sizeof(v4hi) * 2);
	       b2p[0] = r1;
	       b2p[1] = r2;
	       b2p += pitch;
	    }
	    d0 += 2;
	    b += 2;
	 }
	 if(bFullScan) return;
	 d0 = dst;
	 d0 = d0 + pitch * yrep2;
	 bb2.uv = bb;
	 for(yy = 0; yy < blank; yy++) {
	    b2p = d0;
	    for(xx = 0; xx < ww; xx += 8) {
	       _prefetch_data_write_l2(b2p, sizeof(v4hi) * 2);
	       b2p[0] = bb2;
	       b2p[1] = bb2;
	       b2p += 2;
	    }
	    d0 += pitch;
	 }
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
   AG_SurfaceLock(Surface);   
   
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
   Scaler_DrawLine((v4hi *)d1, (Uint32 *)d2, ww, yrep >> 1, yrep, Surface->pitch);
   AG_SurfaceUnlock(Surface);
   return;
}


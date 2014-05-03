/*
 * Zoom x4x4
 * (C) 2013 K.Ohta
 * 
 * History:
 *  2013-01-26 Move from agar_sdlscaler.cpp
 *  2013-09-17 Move from ui-agar/
 */

#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"


void pVram2RGB_x4(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi_t *b;
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   AG_Surface *Surface = GetDrawSurface();
   int w;
   int h;
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
      d1 = (Uint32 *)((Uint8 *)Surface->pixels + x * 4 * Surface->format->BytesPerPixel + y * Surface->pitch);
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)Surface->pixels + x * 4 * Surface->format->BytesPerPixel + y * (yrep >> 1) * Surface->pitch);
   }
   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   if(w <= (x * 4 + 31)) {
       int j;
       Uint32 d0;
      p = src;

      ww = w - x * 4;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 4, i++){
            d2 = d1;
            d0 = p[i];
	  if(yrep & 1) {
	     d2[xx + 0] = d2[xx + 1] = d2[xx + 2] = d2[xx + 3] = d0;
	     yrep--;
	     d2 += pitch;
	  }
            for(j = 0; j < (yrep / 2); j++) {
	       if(!bFullScan && (j >= (yrep / 4))) {
		    d2[xx] = d2[xx + 1] = d2[xx + 2] = d2[xx + 3] = black;
		 } else {
		    d2[xx] = d2[xx + 1] = d2[xx + 2] = d2[xx + 3] = d0;
		 }

                d2 += pitch;
            }
        }
        d1 += (pitch * (yrep >> 1));
        if(yrep & 1) d1 += pitch;
	 p += 8;
    }
   } else { // inside align
      v8hi_t b2, b3, b4, b5;
      v8hi_t *b2p;
      int j;
      v8hi_t bb;
     bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] =
     bb.i[4] = bb.i[5] = bb.i[6] = bb.i[7] = black;

     b = (v8hi_t *)src;
       switch(yrep){
	case 0:
	case 1:
	case 2:
	  for(yy = 0; yy < hh; yy++){
	     b2.i[0] = b2.i[1] =
	       b2.i[2] = b2.i[3] = b->i[0];
	     b2.i[4] = b2.i[5] =
	       b2.i[7] = b2.i[6] = b->i[1];

	     b3.i[0] = b3.i[1] =
	       b3.i[2] = b3.i[3] = b->i[2];
	     b3.i[4] = b3.i[5] =
	       b3.i[7] = b3.i[6] = b->i[3];

	     b4.i[0] = b4.i[1] =
	       b4.i[2] = b4.i[3] = b->i[4];
	     b4.i[4] = b4.i[5] =
	       b4.i[7] = b4.i[6] = b->i[5];

	     b5.i[0] = b5.i[1] =
	       b5.i[2] = b5.i[3] = b->i[6];
	     b5.i[4] = b5.i[5] =
	       b5.i[7] = b5.i[6] = b->i[7];

	     b2p = (v8hi_t *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     b2p[3] = b5;
	     d1 += pitch;
	     b++;
	  }
	  break;
	default:
	  for(yy = 0; yy < hh; yy++){
	     b2.i[0] = b2.i[1] =
	       b2.i[2] = b2.i[3] = b->i[0];
	     b2.i[4] = b2.i[5] =
	       b2.i[7] = b2.i[6] = b->i[1];

	     b3.i[0] = b3.i[1] =
	       b3.i[2] = b3.i[3] = b->i[2];
	     b3.i[4] = b3.i[5] =
	       b3.i[7] = b3.i[6] = b->i[3];

	     b4.i[0] = b4.i[1] =
	       b4.i[2] = b4.i[3] = b->i[4];
	     b4.i[4] = b4.i[5] =
	       b4.i[7] = b4.i[6] = b->i[5];

	     b5.i[0] = b5.i[1] =
	       b5.i[2] = b5.i[3] = b->i[6];
	     b5.i[4] = b5.i[5] =
	       b5.i[7] = b5.i[6] = b->i[7];

	     if(yrep & 1) {
		b2p = (v8hi_t *)d1;
		b2p[0] = b2;
		b2p[1] = b3;
		b2p[2] = b4;
		b2p[3] = b5;
		d1 += pitch;
		yrep--;
	     }
	     for(j = 0; j < (yrep >> 1); j++) {
		b2p = (v8hi_t *)d1;
		if(!bFullScan && (j >= (yrep >> 2))) {
		   b2p[0] = b2p[1] = b2p[2] = b2p[3] = bb;
		} else {
		   b2p[0] = b2;
		   b2p[1] = b3;
		   b2p[2] = b4;
		   b2p[3] = b5;
		}
		d1 += pitch;
	     }
	     b++;
	  }
	break;
       }
   }
}

void pVram2RGB_x4_Line(Uint32 *src, int xbegin, int xend, int y, int yrep)
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
   if((ww * 4) >= w) ww = w / 4;
   ww = ww - 7;
   if(ww <= 0) return;
   
#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
   if(yrep < 2) {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 4 * Surface->format->BytesPerPixel
                        + y * Surface->pitch);
      d2 = &src[x + y * 640];
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 4 * Surface->format->BytesPerPixel
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
    v4hi b2;
    v4hi b3;
    v4hi b4;
    v4hi b5;
    v4hi b6;
    v4hi b7;
    v4hi b8;
    v4hi b9;
    register v4hi bb;
    v4hi *b2p;
    Uint32 *d0;
    Uint32 dd;
      
    b = (v4hi *)d2;
    bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] = black;
       switch(yrep) {
	case 0:
	case 1:
	case 2:
	  for(xx = 0; xx < ww; xx += 8) {
	     b2p = (v4hi *)d1;
	     b2.i[0] = b2.i[1] = b2.i[2] = b2.i[3] = b[0].i[0];
	     b3.i[0] = b3.i[1] = b3.i[2] = b3.i[3] = b[0].i[1];
	     b4.i[0] = b4.i[1] = b4.i[2] = b4.i[3] = b[0].i[2];
	     b5.i[0] = b5.i[1] = b5.i[2] = b5.i[3] = b[0].i[3];

	     b6.i[0] = b6.i[1] = b6.i[2] = b6.i[3] = b[1].i[0];
	     b7.i[0] = b7.i[1] = b7.i[2] = b7.i[3] = b[1].i[1];
	     b8.i[0] = b8.i[1] = b8.i[2] = b8.i[3] = b[1].i[2];
	     b9.i[0] = b9.i[1] = b9.i[2] = b9.i[3] = b[1].i[3];

	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     b2p[3] = b5;
	     b2p[4] = b6;
	     b2p[5] = b7;
	     b2p[6] = b8;
	     b2p[7] = b9;
	     d1 += 32;
	     b += 2;
	  }
	  if((ww % 8) != 0){
	     j = 0;
	     d0 = (Uint32 *)b;
	     b2p = (v4hi *)d1;
	     for(j = 0;j < (ww % 8); j++) {
		b2.i[0] = b2.i[1] = b2.i[3] = b2.i[4] = *d0;
		*b2p = b2;
		d0++;
		b2p++;
	     }
	  }
	  break;
	default:
	  d0 = d1;
	  for(xx = 0; xx < ww; xx += 8){
	     d1 = d0;
	     b2.i[0] = b2.i[1] = b2.i[2] = b2.i[3] = b[0].i[0];
	     b3.i[0] = b3.i[1] = b3.i[2] = b3.i[3] = b[0].i[1];
	     b4.i[0] = b4.i[1] = b4.i[2] = b4.i[3] = b[0].i[2];
	     b5.i[0] = b5.i[1] = b5.i[2] = b5.i[3] = b[0].i[3];

	     b6.i[0] = b6.i[1] = b6.i[2] = b6.i[3] = b[1].i[0];
	     b7.i[0] = b7.i[1] = b7.i[2] = b7.i[3] = b[1].i[1];
	     b8.i[0] = b8.i[1] = b8.i[2] = b8.i[3] = b[1].i[2];
	     b9.i[0] = b9.i[1] = b9.i[2] = b9.i[3] = b[1].i[3];


	     for(j = 0; j < (yrep >> 1); j++) {
		b2p = (v4hi *)d1;
		if(!bFullScan && (j > (yrep >> 2))) {
		   b2p[0] = 
		   b2p[1] = 
		   b2p[2] = 
		   b2p[3] = 
		   b2p[4] = 
		   b2p[5] = 
		   b2p[6] = 
		   b2p[7] = bb;
		 } else {
		    b2p[0] = b2;
		    b2p[1] = b3;
		    b2p[2] = b4;
		    b2p[3] = b5;
		    b2p[4] = b6;
		    b2p[5] = b7;
		    b2p[6] = b8;
		    b2p[7] = b9;
		}
		d1 += pitch;
	     }
	     d0 += 32;
	     b += 2;
	  }
	  if((ww % 8) != 0){
	     d2 = (Uint32 *)b;
	     d0 = d1;
	     for(j = 0;j < (ww % 8); j++) {
		d1 = d0;
		b2.i[0] = b2.i[1] = b2.i[3] = b2.i[4] = *d2;
		for(i = 0; i < (yrep >> 1); i++) {
		   b2p = (v4hi *)d1;
		   if(!bFullScan && (j > (yrep >> 2))) {
		      *b2p = bb;
		   } else {
		      *b2p = b2;
		   }
		   d1 += pitch;
		}
		d0 += 4;
		d2++;
	     }
	  }
	  break;
       }

   }
}


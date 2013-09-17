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


void pVram2RGB_x4(XM7_SDLView *my, Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi_t *b;
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   int w = my->Surface->w;
   int h = my->Surface->h;
   int yy;
   int xx;
   int hh;
   int ww;
   int i;
   int pitch;
   Uint32 black;

#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif

   if(yrep < 2) {
      d1 = (Uint32 *)((Uint8 *)(my->Surface->pixels) + x * 4 * my->Surface->format->BytesPerPixel
                        + y * my->Surface->pitch);
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(my->Surface->pixels) + x * 4 * my->Surface->format->BytesPerPixel
                        + y * (yrep >> 1) * my->Surface->pitch);
   }
   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = my->Surface->pitch / sizeof(Uint32);
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


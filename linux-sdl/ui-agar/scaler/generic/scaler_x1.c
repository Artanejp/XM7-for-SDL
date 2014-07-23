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

void pVram2RGB_x1_Line(Uint32 *src, int xbegin, int xend, int y, float yrep)
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
   int yrep2;
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
   yrep2 = (int)(yrep * 16.0f);
   if(yrep <= 1.0f) {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * Surface->format->BytesPerPixel
                        + y * Surface->pitch);
      d2 = &src[x + y * 640];
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * Surface->format->BytesPerPixel
                        + ((y * yrep2) >> 4) * Surface->pitch);
      d2 = &src[x + y * 640];
   }

   if((((y * yrep2) % 16) == 0) && ((yrep2 % 16) != 0)) yrep2 += 16;
   yrep2 >>= 4;

   pitch = Surface->pitch / sizeof(Uint32);
   { // Not thinking align ;-(
	
    int j;
    v4hi b2;
    v4hi b3;
    register v4hi bb;
    v4hi *b2p;
    Uint32 *d0;
      
    b = (v4hi *)d2;
    bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] = black;
       switch(yrep2) {
	case 0:
	case 1:
//	case 2:
	  for(xx = 0; xx < ww; xx += 8) {
	     b2p = (v4hi *)d1;
	     b2p[0] = b[0];
	     b2p[1] = b[1];
	     d1 += 8;
	     b += 2;
	  }
	  break;
	default:
	  d0 = d1;
	  for(xx = 0; xx < ww; xx += 8){
	     d1 = d0;
	     b2 = b[0];
	     b3 = b[1];

	     for(j = 0; j < yrep2; j++) {
		b2p = (v4hi *)d1;
		if(!bFullScan && (j >= (yrep2 >> 1))) {
		   b2p[0] = 
		   b2p[1] = bb;
		 } else {
		   b2p[0] = b2;
		   b2p[1] = b3;
		}
		d1 += pitch;
	     }
	     d0 += 8;
	     b += 2;
	  }

	  break;
       }

   }
}


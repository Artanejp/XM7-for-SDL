/*
 * Zoom x0.5
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

// 0.5
void pVram2RGB_x05(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi_t *b;
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   int w;
   int h;
   int yy;
   int yy2;
   int xx;
   int hh;
   int ww;
   int i;
   int pitch;
   v8hi_t rmask1, gmask1, bmask1, amask1;
   v4hi rmask2, gmask2, bmask2, amask2;
   Uint32 black;
   AG_Surface *Surface = GetDrawSurface();
    
   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;
   
#if AG_BIG_ENDIAN != 1
   rmask1.i[0] = rmask1.i[1] = rmask1.i[2] = rmask1.i[3] =
   rmask1.i[4] = rmask1.i[5] = rmask1.i[6] = rmask1.i[7] = 0x000000ff;

   gmask1.i[0] = gmask1.i[1] = gmask1.i[2] = gmask1.i[3] =
   gmask1.i[4] = gmask1.i[5] = gmask1.i[6] = gmask1.i[7] = 0x0000ff00;

   bmask1.i[0] = bmask1.i[1] = bmask1.i[2] = bmask1.i[3] =
   bmask1.i[4] = bmask1.i[5] = bmask1.i[6] = bmask1.i[7] = 0x00ff0000;

   amask1.i[0] = amask1.i[1] = amask1.i[2] = amask1.i[3] =
   amask1.i[4] = amask1.i[5] = amask1.i[6] = amask1.i[7] = 0xff000000;

   amask2.i[0] = amask2.i[1] = amask2.i[2] = amask2.i[3] = 0xff000000;
   bmask2.i[0] = bmask2.i[1] = bmask2.i[2] = bmask2.i[3] = 0x00ff0000;
   gmask2.i[0] = gmask2.i[1] = gmask2.i[2] = gmask2.i[3] = 0x0000ff00;
   rmask2.i[0] = rmask2.i[1] = rmask2.i[2] = rmask2.i[3] = 0x000000ff;

#else
   rmask1.i[0] = rmask1.i[1] = rmask1.i[2] = rmask1.i[3] =
   rmask1.i[4] = rmask1.i[5] = rmask1.i[6] = rmask1.i[7] = 0xff000000;

   gmask1.i[0] = gmask1.i[1] = gmask1.i[2] = gmask1.i[3] =
   gmask1.i[4] = gmask1.i[5] = gmask1.i[6] = gmask1.i[7] = 0x00ff0000;

   bmask1.i[0] = bmask1.i[1] = bmask1.i[2] = bmask1.i[3] =
   bmask1.i[4] = bmask1.i[5] = bmask1.i[6] = bmask1.i[7] = 0x0000ff00;

   amask1.i[0] = amask1.i[1] = amask1.i[2] = amask1.i[3] =
   amask1.i[4] = amask1.i[5] = amask1.i[6] = amask1.i[7] = 0x000000ff;

   rmask2.i[0] = rmask2.i[1] = rmask2.i[2] = rmask2.i[3] = 0xff000000;
   gmask2.i[0] = gmask2.i[1] = gmask2.i[2] = gmask2.i[3] = 0x00ff0000;
   bmask2.i[0] = bmask2.i[1] = bmask2.i[2] = bmask2.i[3] = 0x0000ff00;
   amask2.i[0] = amask2.i[1] = amask2.i[2] = amask2.i[3] = 0x000000ff;
#endif
   if(yrep < 4) {
      if(yrep == 0) {
	 d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (x >> 1) * Surface->format->BytesPerPixel
                        + (y >> 1)  * Surface->pitch);
      } else {
	 d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (x >> 1) * Surface->format->BytesPerPixel
                        + y  * Surface->pitch);
      }

      if(yrep == 0) yrep = 1;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (x >> 1) * Surface->format->BytesPerPixel
                        + y * (yrep >> 2) * Surface->pitch);
   }
   if(((x >>1) + 4) >= w) {
	Uint32 amask, rmask, gmask, bmask;
        Uint32 bd1, bd2;
        Uint32 r, g, b, a;
        int yrep2;
        int j;

#if AG_BIG_ENDIAN != 1
      amask = 0xff000000;
      bmask = 0x00ff0000;
      gmask = 0x0000ff00;
      rmask = 0x000000ff;
#else
      rmask = 0xff000000;
      gmask = 0x00ff0000;
      bmask = 0x0000ff00;
      amask = 0x000000ff;
#endif
      yrep2 = yrep >> 1;
      if(yrep < 1) {
	Uint32 r2, g2, b2;

        ww = w - (x >> 1);
        p = src;
	yy2 = y;
	for(yy = 0; yy < 4; yy++, yy2++) {
	 if(yy2 >= h) break;
	   for(i = 0, xx = x; xx < w; xx++, i++ ) {
	      bd1 = p[0];
	      bd2 = p[1];
	      r = (((bd1 & rmask) >> 1) + ((bd2 & rmask) >> 1)) & rmask;
	      g = (((bd1 & gmask) >> 1) + ((bd2 & gmask) >> 1)) & gmask;
	      b = (((bd1 & bmask) >> 1) + ((bd2 & bmask) >> 1)) & bmask;
	      bd1 = p[8];
	      bd2 = p[9];
	      r2 = (((bd1 & rmask) >> 1) + ((bd2 & rmask) >> 1)) & rmask;
	      g2 = (((bd1 & gmask) >> 1) + ((bd2 & gmask) >> 1)) & gmask;
	      b2 = (((bd1 & bmask) >> 1) + ((bd2 & bmask) >> 1)) & bmask;
	      r = ((r >> 1) + (r2 >> 1)) & rmask;
	      g = ((g >> 1) + (g2 >> 1)) & gmask;
	      b = ((b >> 1) + (b2 >> 1)) & bmask;

	      d2 = &d1[xx];
	      *d2 = r | g | b | amask;
	      p += 2;
	   }
	   d1 += pitch;
	   p += 16;
	}
      } else {
//        ww = w - (x >> 1);
        p = src;
	yy2 = y;
	for(yy = 0; yy < 4; yy++, yy2++) {
	 if(yy2 >= h) break;
	   for(i = 0, xx = x; xx < w; xx++, i++ ) {
	      bd1 = p[0];
	      bd2 = p[1];
	      r = (((bd1 & rmask) >> 1) + ((bd2 & rmask) >> 1)) & rmask;
	      g = (((bd1 & gmask) >> 1) + ((bd2 & gmask) >> 1)) & gmask;
	      b = (((bd1 & bmask) >> 1) + ((bd2 & bmask) >> 1)) & bmask;
	      d2 = &d1[i];
	      for(j = 0; j < yrep2; j++) {
		 *d2 = r | g  | b | amask;
		 d2 += pitch;
	      }
	      p += 2;
	   }
	   d1 += pitch;
	   p += 8;
	}

      }
      return;
   }


   pitch = Surface->pitch / sizeof(Uint32);
   if(yrep < 1) {
      v8hi_t *p2;
      v4hi *pd;
      v8hi_t b1;
      v8hi_t b2;
      v8hi_t br, bb, bg;
      v8hi_t br2, bb2, bg2;
      v4hi cr, cb, cg;
      p = src;
      yy2 = y;
      for(yy  = 0; yy < 4; yy++, yy2++) {
	 if(yy2 >= h) break;
	 d2 = d1;
	 b = (v8hi_t *)p;
	 b1 = b[0];
	 b2 = b[1];
	 br.v  = (b1.v & rmask1.v);
	 br2.v = (b2.v & rmask1.v);
	 bg.v  = (b1.v & gmask1.v);
	 bg2.v = (b2.v & gmask1.v);
	 bb.v  = (b1.v & bmask1.v);
	 bb2.v = (b2.v & bmask1.v);
	 cr.i[0] = (br.i[0] >> 1) + (br.i[1] >> 1);
	 cr.i[1] = (br.i[2] >> 1) + (br.i[3] >> 1);
	 cr.i[2] = (br.i[4] >> 1) + (br.i[5] >> 1);
	 cr.i[3] = (br.i[6] >> 1) + (br.i[7] >> 1);

	 cb.i[0] = (bb.i[0] + bb.i[1]) >> 1;
	 cb.i[1] = (bb.i[2] + bb.i[3]) >> 1;
	 cb.i[2] = (bb.i[4] + bb.i[5]) >> 1;
	 cb.i[3] = (bb.i[6] + bb.i[7]) >> 1;

	 cg.i[0] = (bg.i[0] + bg.i[1]) >> 1;
	 cg.i[1] = (bg.i[2] + bg.i[3]) >> 1;
	 cg.i[2] = (bg.i[4] + bg.i[5]) >> 1;
	 cg.i[3] = (bg.i[6] + bg.i[7]) >> 1;
	 cr.v = cr.v & rmask2.v;
	 cg.v = cg.v & gmask2.v;
	 cb.v = cb.v & bmask2.v;

	 pd = (v4hi *)d1;
	 pd->v = cr.v | cg.v | cb.v | amask2.v;

	 p += 16;
	 d1 += pitch;
      }
   } else {
      v4hi *pd;
      v4hi cr, cg, cb, cd;
      v8hi_t *b;
      v8hi_t br,bg, bb;
      int yrep2 = yrep >> 1;

      if(yrep2 <= 1) yrep2 = 1;
      if(h <= ((y + 8) * (yrep >> 2))) {
	 hh = (h - y * (yrep >> 2)) / (yrep >> 2);
      } else {
	 hh = 8;
      }
      p = src;
      yy2 = y;
      for(yy = 0; yy < 8; yy++, yy2++) {
	 if(yy2 >= h) break;

 	 b = (v8hi_t *)p;
	 br.v = b->v & rmask1.v;
	 bg.v = b->v & gmask1.v;
	 bb.v = b->v & bmask1.v;
	 cr.i[0] = (br.i[0] >> 1) + (br.i[1] >> 1);
	 cr.i[1] = (br.i[2] >> 1) + (br.i[3] >> 1);
	 cr.i[2] = (br.i[4] >> 1) + (br.i[5] >> 1);
	 cr.i[3] = (br.i[6] >> 1) + (br.i[7] >> 1);

	 cb.i[0] = (bb.i[0] + bb.i[1]) >> 1;
	 cb.i[1] = (bb.i[2] + bb.i[3]) >> 1;
	 cb.i[2] = (bb.i[4] + bb.i[5]) >> 1;
	 cb.i[3] = (bb.i[6] + bb.i[7]) >> 1;

	 cg.i[0] = (bg.i[0] + bg.i[1]) >> 1;
	 cg.i[1] = (bg.i[2] + bg.i[3]) >> 1;
	 cg.i[2] = (bg.i[4] + bg.i[5]) >> 1;
	 cg.i[3] = (bg.i[6] + bg.i[7]) >> 1;
	 cr.v = cr.v & rmask2.v;
	 cg.v = cg.v & gmask2.v;
	 cb.v = cb.v & bmask2.v;
	 cd.v = cr.v | cg.v | cb.v | amask2.v;
	 for(i = 0; i < yrep2; i++) {
	    pd = (v4hi *)d1;
	    *pd = cd;
	    d1 += pitch;
	 }
	 p += 8;
      }
   }
}


void pVram2RGB_x05_Line(Uint32 *src, int xbegin, int xend, int y, int yrep)
{
   Uint32 *d1;
   Uint32 *d2;
   Uint32 *p;
   int w;
   int h;
   int yy;
   int yy2;
   int xx;
   int hh;
   int ww;
   int i;
   int pitch;
   v8hi_t rmask1, gmask1, bmask1, amask1;
   v4hi rmask2, gmask2, bmask2, amask2;
   Uint32 black;
   AG_Surface *Surface = GetDrawSurface();
   Uint32 amask, rmask, gmask, bmask;
   Uint32 bd1, bd2;
   Uint32 r, g, b, a;
   int yrep2;
   int j;
    
   if(Surface == NULL) return;
   w = Surface->w;
   h = Surface->h;
   
#if AG_BIG_ENDIAN != 1
   rmask1.i[0] = rmask1.i[1] = rmask1.i[2] = rmask1.i[3] =
   rmask1.i[4] = rmask1.i[5] = rmask1.i[6] = rmask1.i[7] = 0x000000ff;

   gmask1.i[0] = gmask1.i[1] = gmask1.i[2] = gmask1.i[3] =
   gmask1.i[4] = gmask1.i[5] = gmask1.i[6] = gmask1.i[7] = 0x0000ff00;

   bmask1.i[0] = bmask1.i[1] = bmask1.i[2] = bmask1.i[3] =
   bmask1.i[4] = bmask1.i[5] = bmask1.i[6] = bmask1.i[7] = 0x00ff0000;

   amask1.i[0] = amask1.i[1] = amask1.i[2] = amask1.i[3] =
   amask1.i[4] = amask1.i[5] = amask1.i[6] = amask1.i[7] = 0xff000000;

   amask2.i[0] = amask2.i[1] = amask2.i[2] = amask2.i[3] = 0xff000000;
   bmask2.i[0] = bmask2.i[1] = bmask2.i[2] = bmask2.i[3] = 0x00ff0000;
   gmask2.i[0] = gmask2.i[1] = gmask2.i[2] = gmask2.i[3] = 0x0000ff00;
   rmask2.i[0] = rmask2.i[1] = rmask2.i[2] = rmask2.i[3] = 0x000000ff;

#else
   rmask1.i[0] = rmask1.i[1] = rmask1.i[2] = rmask1.i[3] =
   rmask1.i[4] = rmask1.i[5] = rmask1.i[6] = rmask1.i[7] = 0xff000000;

   gmask1.i[0] = gmask1.i[1] = gmask1.i[2] = gmask1.i[3] =
   gmask1.i[4] = gmask1.i[5] = gmask1.i[6] = gmask1.i[7] = 0x00ff0000;

   bmask1.i[0] = bmask1.i[1] = bmask1.i[2] = bmask1.i[3] =
   bmask1.i[4] = bmask1.i[5] = bmask1.i[6] = bmask1.i[7] = 0x0000ff00;

   amask1.i[0] = amask1.i[1] = amask1.i[2] = amask1.i[3] =
   amask1.i[4] = amask1.i[5] = amask1.i[6] = amask1.i[7] = 0x000000ff;

   rmask2.i[0] = rmask2.i[1] = rmask2.i[2] = rmask2.i[3] = 0xff000000;
   gmask2.i[0] = gmask2.i[1] = gmask2.i[2] = gmask2.i[3] = 0x00ff0000;
   bmask2.i[0] = bmask2.i[1] = bmask2.i[2] = bmask2.i[3] = 0x0000ff00;
   amask2.i[0] = amask2.i[1] = amask2.i[2] = amask2.i[3] = 0x000000ff;
#endif
   if(yrep < 4) {
      if(yrep == 0) {
	 d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (xbegin >> 1) * Surface->format->BytesPerPixel
                        + (y >> 1)  * Surface->pitch);
      } else {
	 d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (xbegin >> 1) * Surface->format->BytesPerPixel
                        + y  * Surface->pitch);
      }

      if(yrep == 0) yrep = 1;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (xbegin >> 1) * Surface->format->BytesPerPixel
                        + y * (yrep >> 2) * Surface->pitch);
   }

#if AG_BIG_ENDIAN != 1
      amask = 0xff000000;
      bmask = 0x00ff0000;
      gmask = 0x0000ff00;
      rmask = 0x000000ff;
#else
      rmask = 0xff000000;
      gmask = 0x00ff0000;
      bmask = 0x0000ff00;
      amask = 0x000000ff;
#endif
      yrep2 = yrep >> 1;
      
      ww = w;
      if(ww > ((xend - xbegin) >> 1)) ww = (xend - xbegin) >> 1;
      if(yrep < 2) {
	Uint32 r2, g2, b2;
	
        p = src;
	yy2 = y;
	   p = src;
	   for(xx = 0; xx < ww; xx++) {
	      bd1 = p[0];
	      bd2 = p[1];
	      r = (((bd1 & rmask) >> 1) + ((bd2 & rmask) >> 1)) & rmask;
	      g = (((bd1 & gmask) >> 1) + ((bd2 & gmask) >> 1)) & gmask;
	      b = (((bd1 & bmask) >> 1) + ((bd2 & bmask) >> 1)) & bmask;
	      r = ((r >> 1) + (r2 >> 1)) & rmask;
	      g = ((g >> 1) + (g2 >> 1)) & gmask;
	      b = ((b >> 1) + (b2 >> 1)) & bmask;

	      d2 = &d1[xx];
	      *d2 = r | g | b | amask;
	      d2++;
	      p += 2;
	   }

            
      } else {
//        ww = w - (x >> 1);
        Uint32 pixel;
	Uint32 r2, g2, b2;
        p = src;
	yy2 = y;
	black = amask;
	 for(xx = 0; xx < ww; xx++) {
	      bd1 = p[0];
	      bd2 = p[1];
	      r = (((bd1 & rmask) >> 1) | ((bd2 & rmask) >> 1)) & rmask;
	      g = (((bd1 & gmask) >> 1) | ((bd2 & gmask) >> 1)) & gmask;
	      b = (((bd1 & bmask) >> 1) | ((bd2 & bmask) >> 1)) & bmask;
	      pixel = r | g  | b | amask;
	      d2 = &d1[xx];
	      for(j = 0; j < yrep2; j++) {
		 if(!bFullScan && (j >= (yrep >> 2))) {
		    *d2 = black;
		 } else {
		    *d2 = pixel;
		 }
		 d2 += pitch;
	      }
	      p += 4;
	   }
      }
}




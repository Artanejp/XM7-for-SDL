/*
 * Zoom x2.25x2 i.e. 1440x900.
 * (C) 2013 K.Ohta
 * 
 * History:
 *  2013-04-02 Move from scaler_x2.c
 */
#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern struct XM7_CPUID *pCpuID;

extern void pVram2RGB_x225(Uint32 *src, Uint32 *dst, int x, int y, int yrep);

#if defined(__SSE2__)

static void Scaler_DrawLine(Uint32 *dst, Uint32 *src, int ww, int repeat, int pitch)
{
   int xx;
   int yy;
   int yrep2;
   int yrep3;
   int blank;
   v2hi *b2p;
   register v2hi r1, r2, r3, r4;
   v2hi r5v[(640 * 9) / 8 + 1];
   v4hi *d0;
   register v2hi *b;
   int pitch2;
   int ip = 0;
#if AG_BIG_ENDIAN != 1
   const v2ui bb = (v2ui){0xff000000, 0xff000000};
#else
   const v2ui bb = (v2ui){0x000000ff, 0x000000ff};
#endif
     
   if(repeat <= 0) return;
   b = (v2hi *)src;
   pitch2 = pitch / sizeof(v2hi);

   _prefetch_data_write_l1(r5v, sizeof(r5v));
   if((bFullScan) || (repeat < 2)) {
	 // 76543210 -> 776655444332211000
      for(xx = 0; xx < ww; xx += 8) {
	 r1 = b[0];
	 r2 = b[1];
	 r3 = b[2];
	 r4 = b[3];
	 r5v[ip + 0].uv = (v2ui){r1.i[0], r1.i[0]}; //00
	 r5v[ip + 1].uv = (v2ui){r1.i[0], r1.i[1]}; //01
	 
	 r5v[ip + 2].uv = (v2ui){r1.i[1], r2.i[0]}; //12
	 r5v[ip + 3].uv = (v2ui){r2.i[0], r2.i[1]}; //23
	 
	 r5v[ip + 4].uv = (v2ui){r2.i[1], r3.i[0]}; //34
	 r5v[ip + 5].uv = (v2ui){r3.i[0], r3.i[0]}; //44
	 r5v[ip + 6].uv = (v2ui){r3.i[1], r3.i[1]}; //55

	 r5v[ip + 7].uv = (v2ui){r4.i[0], r4.i[0]}; //66
	 r5v[ip + 8].uv = (v2ui){r4.i[1], r4.i[1]}; //77	 
	 ip += 9;
	 b += 4;
      }
      b2p = (v2hi *)dst;      
      _prefetch_data_read_l1(r5v, sizeof(r5v));
      for(yy = 0; yy < repeat; yy++) {
	 memcpy((void *)b2p, (void *)r5v, ip * sizeof(v2hi));
	 b2p = b2p + pitch2;
      }
   } else {
	 // 76543210 -> 776655444332211000
      for(xx = 0; xx < ww; xx += 8) {
	 r1 = b[0];
	 r2 = b[1];
	 r3 = b[2];
	 r4 = b[3];
	 r5v[ip + 0].uv = (v2ui){r1.i[0], r1.i[0]}; //00
	 r5v[ip + 1].uv = (v2ui){r1.i[0], r1.i[1]}; //01
	 
	 r5v[ip + 2].uv = (v2ui){r1.i[1], r2.i[0]}; //12
	 r5v[ip + 3].uv = (v2ui){r2.i[0], r2.i[1]}; //23
	 
	 r5v[ip + 4].uv = (v2ui){r2.i[1], r3.i[0]}; //34
	 r5v[ip + 5].uv = (v2ui){r3.i[0], r3.i[0]}; //44
	 r5v[ip + 6].uv = (v2ui){r3.i[1], r3.i[1]}; //55

	 r5v[ip + 7].uv = (v2ui){r4.i[0], r4.i[0]}; //66
	 r5v[ip + 8].uv = (v2ui){r4.i[1], r4.i[1]}; //77	 
	 ip += 9;
	 b += 4;
      }
      b2p = (v2hi *)dst;      
      _prefetch_data_read_l1(r5v, sizeof(r5v));
      for(yy = 0; yy < repeat - 1; yy++) {
	 memcpy((void *)b2p, (void *)r5v, ip * sizeof(v2hi));
	 b2p = b2p + pitch2;
      }
      for(xx = 0; xx < ip; xx++) b2p[xx].uv = bb;
   }
   
}



void pVram2RGB_x225_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep)
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
   if(ww > (w / 2)) ww = w / 2;
   ww = (ww / 8) * 8;
   if(ww <= 0) return;


#if AG_BIG_ENDIAN != 1
   black = 0xff000000;
#else
   black = 0x000000ff;
#endif
//   yrep = yrep * 16.0f;

   yrep2 = yrep;

   d1 = (Uint32 *)((Uint8 *)dst + ((x * 18) / 8) * Surface->format->BytesPerPixel);
   d2 = &src[x + y * 640];
   Scaler_DrawLine(d1, (Uint32 *)d2, ww, yrep2, Surface->pitch);
//   AG_SurfaceUnlock(Surface);
   return;
}


#else 

void pVram2RGB_x2_Line_SSE2(Uint32 *src, int xbegin,  int xend, int y, int yrep)
{
   pVram2RGB_x2_Line(src, dst, x, y, yrep);
}

#endif // __SSE2__
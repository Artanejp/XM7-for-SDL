/*
* FM-7 Emulator "XM7"
* Virtual Vram Display(Agar widget version)
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* History:
* Jan 18,2012 From demos/customwidget/mywidget.[c|h]
* Jan 20,2012 Separete subroutines.
*/

#include "agar_sdlview.h"
#include "agar_cfg.h"
#include "api_vram.h"
#include "api_draw.h"
//#include "api_scaler.h"
#include "api_kbd.h"
#include "sdl_cpuid.h"
#include "cache_wrapper.h"

extern "C" {
extern struct XM7_CPUID *pCpuID;
extern BOOL bUseSIMD;
}

extern "C" { // Define Headers
   // scaler/generic
   extern void pVram2RGB_x05_Line(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x05.c , raster render
   extern void pVram2RGB_x1_Line(Uint32 *src,  Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x1.c , raster render
   extern void pVram2RGB_x2_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.
   extern void pVram2RGB_x225_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.
   extern void pVram2RGB_x4_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.

#if defined(USE_SSE2) // scaler/sse2/
   extern void pVram2RGB_x1_Line_SSE2(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x1.c , raster render
   extern void pVram2RGB_x2_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.
   extern void pVram2RGB_x225_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.
   extern void pVram2RGB_x4_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.
#endif
}

static int iScaleFactor = 1;
static void *pDrawFn = NULL;
static void *pDrawFn2 = NULL;
static int iOldW = 0;
static int iOldH = 0;


static inline Uint32 pVram_XtoHalf(Uint32 d1, Uint32 d2)
{
   Uint32 d0;
   Uint16 r,g,b,a;
#if AG_BIG_ENDIAN
   r = (d1 & 0x000000ff) + (d2 & 0x000000ff);
   g = ((d1 & 0x0000ff00) >> 8) + ((d2 & 0x0000ff00) >> 8);
   b = ((d1 & 0x00ff0000) >> 16) + ((d2 & 0x00ff0000) >> 16);
   d0 = 0xff000000 | (r >> 1) | ((b << 15) & 0x00ff0000) | ((g << 7) & 0x0000ff00);
#else
   r = ((d1 & 0xff000000) >> 24) + ((d2 & 0xff000000) >> 24);
   g = ((d1 & 0x00ff0000) >> 16) + ((d2 & 0x00ff0000) >> 16);
   b = ((d1 & 0x0000ff00) >> 8) + ((d2 & 0x0000ff00) >> 8);
   d0 = 0x000000ff | ((r << 23) & 0xff000000) | ((g << 15) & 0x00ff0000) | ((b << 7) & 0x0000ff00);
#endif
   return d0;
}


// Zoom 1.25 (640->800)
void pVram2RGB_x125(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
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
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (x * 5 * Surface->format->BytesPerPixel) / 4
                        + y * Surface->pitch);
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (x * 5  * Surface->format->BytesPerPixel) / 4
                        + y * (yrep >> 1) * Surface->pitch);
   }

   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   if(w < ((x * 5)/ 4 + 8)) {
    int j;
    Uint32 d0;

    p = src;
    ww = w - (x * 5) / 4;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx ++, i++){
            d2 = d1;
            d0 = p[i];
            for(j = 0; j < (yrep >> 1); j++){
	       if((j > ((yrep >> 1))) && !bFullScan){
		  d2[xx] = black;
		  if((xx & 3) == 0) {
		     d2[xx + 1] = black;
		  }
       	       } else {
		  d2[xx] = d0;
		  if((xx & 3) == 0) {
		     d2[xx + 1] = d0;
		  }
	       }
               d2 += pitch;
            }
	   if((xx & 3) == 0) xx++;
        }
        d1 += (pitch * (yrep >> 1));
        if(yrep & 1) d1 += pitch;
        p += 8;
      }
   } else { // inside align
    int j;
    v8hi_t b2;
    v8hi_t bb;
    Uint32 b28 ,b29;
    Uint32 b38 ,b39;

    v8hi_t *b2p;
    b = (v8hi_t *)src;
    for(yy = 0; yy < hh; yy++){
       b2.i[0] = b2.i[1] = b->i[0];
       b2.i[2] = b->i[1];
       b2.i[3] = b->i[2];
       b2.i[4] = b->i[3];

       b2.i[5] = b2.i[6] = b->i[4];
       b2.i[7] = b->i[5];
       b28 = b->i[6];
       b29 = b->i[7];

       bb.i[0] = bb.i[1] =
       bb.i[2] = bb.i[3] =
       bb.i[4] = bb.i[5] =
       bb.i[6] = bb.i[7] = black;
       b38 = b39 = black;
       switch(yrep) {
	case 0:
	case 1:
	case 2:
	  b2p = (v8hi_t *)d1;
	  *b2p = b2;
	  d1[8] = b28;
	  d1[9] = b29;
	  d1 += pitch;
	  break;
	default:
	  for(j = 0; j < (yrep >> 1); j++) {
	     if(yrep & 1){
		b2p = (v8hi_t *)d1;
		*b2p = b2;
		d1[8] = b28;
		d1[9] = b29;
		yrep--;
		d1 += pitch;
	     }

	     b2p = (v8hi_t *)d1;
	     if(!bFullScan && (j >= (yrep >> 2))) {
		*b2p = bb;
		d1[8] = b38;
		d1[9] = b39;
	     } else {
		*b2p = b2;
		d1[8] = b28;
		d1[9] = b29;
	     }

	  d1 += pitch;
	  }
	  break;
       }

       b++;
     }
   }
}


   
// Zoom 2x2

// Zoom 2.5
void pVram2RGB_x25(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
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
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (x * 5 * Surface->format->BytesPerPixel) / 2
                        + y * Surface->pitch);
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + (x * 5 * Surface->format->BytesPerPixel) / 2
                        + y * (yrep >> 1) * Surface->pitch);
   }

   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   if(w < ((x * 5) / 2 + 10)) {
    int j;
    Uint32 d0;

    p = src;
    ww = w - (x * 5) / 2;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 2, i++){
            d2 = d1;
            d0 = p[i];
	  if(yrep & 1) {
	     d2[xx + 0] = d2[xx + 1] = d0;
	     if((xx & 1) == 0) d2[xx + 2] = d0;
	     yrep--;
	     d2 += pitch;
	  }
            for(j = 0; j < (yrep >> 1); j++){
	       if((j > (yrep >> 2)) && !bFullScan){
		  d2[xx] = d2[xx +1] = black;
		  if((xx & 1) == 0) {
		     d2[xx + 2] = black;
		  }
	       } else {
		  d2[xx] = d2[xx + 1] = d0;
		  if((xx & 1) == 0) {
		     d2[xx + 2] = d0;
		  }
	       }
                d2 += pitch;
            }
	   if((xx & 1) == 0) xx++;

        }
        d1 += (pitch * (yrep >> 1));
        if(yrep & 1) d1 += pitch;
        p += 8;
      }
   } else { // inside align
    int j;
    v8hi_t b2;
    v8hi_t b3;
    v4hi b4;
    v8hi_t bb;
    v4hi bb4;

    v8hi_t *b2p;
    v4hi *b4p;

    b = (v8hi_t *)src;
    for(yy = 0; yy < hh; yy++){
       b2.i[0] = b2.i[1] = b2.i[2] = b->i[0];
       b2.i[3] = b2.i[4] = b->i[1];
       b2.i[5] = b2.i[6] = b2.i[7] = b->i[2];
       b3.i[0] = b3.i[1] = b->i[3];

       b3.i[2] = b3.i[3] = b3.i[4] = b->i[4];
       b3.i[5] = b3.i[6] = b->i[5];
       b3.i[7] = b4.i[0] = b4.i[1] = b->i[6];
       b4.i[2] = b4.i[3] = b->i[7];

       bb.i[0] = bb.i[1] =
       bb.i[2] = bb.i[3] =
       bb.i[4] = bb.i[5] =
       bb.i[6] = bb.i[7] = black;
       bb4.i[0] = bb4.i[1] =
       bb4.i[2] = bb4.i[2] = black;

       switch(yrep) {
	case 0:
	case 1:
	case 2:
	  b2p = (v8hi_t *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b4p = (v4hi *)(&d1[16]);
	  *b4p = b4;
	  d1 += pitch;
	  break;
	default:
	  if(yrep & 1) {
	     b2p = (v8hi_t *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b4p = (v4hi *)(&d1[16]);
	     *b4p = b4;
	     yrep--;
	     d1 += pitch;
	  }

	  for(j = 0; j < (yrep >> 1); j++) {
	     b2p = (v8hi_t *)d1;
	     if(!bFullScan && (j >= (yrep >> 2))) {
		b2p[0] = b2p[1] = bb;
		b4p = (v4hi *)(&d1[16]);
		*b4p = bb4;
	     } else {
		b2p[0] = b2;
		b2p[1] = b3;
		b4p = (v4hi *)(&d1[16]);
		*b4p = b4;
	     }

	  d1 += pitch;
	  }
	  break;
       }

       b++;
     }
   }
}


void pVram2RGB_x3(Uint32 *src, Uint32 *dst, int x, int y, int yrep)
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
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 3 * Surface->format->BytesPerPixel
                        + y * Surface->pitch);
      yrep = 2;
   } else {
      d1 = (Uint32 *)((Uint8 *)(Surface->pixels) + x * 3 * Surface->format->BytesPerPixel
                        + y * (yrep >> 1) * Surface->pitch);
   }

   if(h <= ((y + 8) * (yrep >> 1))) {
      hh = (h - y * (yrep >> 1)) / (yrep >> 1);
   } else {
      hh = 8;
   }

   pitch = Surface->pitch / sizeof(Uint32);
   if(w <= (x * 3 + 23)) {
       int j;
       Uint32 d0;
      p = src;

      ww = w - x * 3;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 3, i++){
            d2 = d1;
            d0 = p[i];
	  if(yrep & 1) {
	     d2[xx + 0] = d2[xx + 1] = d2[xx + 2]  = d0;
	     yrep--;
	     d2 += pitch;
	  }
            for(j = 0; j < (yrep >> 1); j++) {
	       if(!bFullScan && (j > (yrep >> 2))) {
		d2[xx] =
		d2[xx + 1] =
                d2[xx + 2] = black;
               } else {
		d2[xx] =
                d2[xx + 1] =
                d2[xx + 2] = d0;
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
      v8hi_t b2,b3,b4;
      v8hi_t *b2p;
      v8hi_t bb;

      bb.i[0] = bb.i[1] = bb.i[2] = bb.i[3] =
      bb.i[4] = bb.i[5] = bb.i[6] = bb.i[7] = black;

     b = (v8hi_t *)src;
     for(yy = 0; yy < hh; yy++){
	b2.i[0] = b2.i[1] = b2.i[2] = b->i[0];
	b2.i[3] = b2.i[4] = b2.i[5] = b->i[1];
	b2.i[6] = b2.i[7] = b3.i[0] = b->i[2];
	b3.i[1] = b3.i[2] = b3.i[3] = b->i[3];
	b3.i[4] = b3.i[5] = b3.i[6] = b->i[4];
	b3.i[7] = b4.i[0] = b4.i[1] = b->i[5];
	b4.i[2] = b4.i[3] = b4.i[4] = b->i[6];
	b4.i[5] = b4.i[6] = b4.i[7] = b->i[7];


       switch(yrep) {
	case 0:
	case 1:
	case 2:
	     b2p = (v8hi_t *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	     break;
	default:
	  if(yrep & 1) {
	     b2p = (v8hi_t *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     yrep--;
	     d1 += pitch;
	  }

	  for(j = 0; j < (yrep >> 1); j++) {
	     b2p = (v8hi_t *)d1;
	     if(!bFullScan && (j >= (yrep >> 2))) {
		b2p[0] = b2p[1] = b2p[2] = bb;
	     } else {
		b2p[0] = b2;
		b2p[1] = b3;
		b2p[2] = b4;
	     }
	     d1 += pitch;
	  }
	  break;
       }
       b++;
     }
   }
}


#if defined(USE_SSE2)
// w0, h0 = Console
// w1, h1 = DrawMode
static void *XM7_SDLViewSelectScaler_Line_SSE2(int w0 ,int h0, int w1, int h1)
{
    int wx0 = w0 >> 1; // w1/4
    int hy0 = h0 >> 1;
    int xfactor;
    int yfactor;
    int xth;
    void (*DrawFn)(Uint32 *src, Uint32 *dst, int xbegin, int xend, int y, int yrep);
    DrawFn = NULL;
   
    xfactor = w1 % wx0;
    yfactor = h1 % hy0;
    xth = wx0 >> 1;
    if(iScaleFactor == (w1 / w0) && (pDrawFn2 != NULL)
      && (w1 == iOldW) && (h1 == iOldH))  return (void *)pDrawFn2;
    iScaleFactor = w1 / w0;
    iOldW = w1;
    iOldH = h1;
    switch(iScaleFactor){
     case 0:
            if(w0 > 480){
	        if((w1 < 480) || (h1 < 150)){
		   DrawFn = pVram2RGB_x05_Line;
		} else {
		   DrawFn = pVram2RGB_x1_Line_SSE2;
		}
            } else {
                DrawFn = pVram2RGB_x1_Line_SSE2;
            }
            break;

     case 1:
            if(xfactor < xth){
	       if(w1 > 720) {
		  DrawFn = pVram2RGB_x2_Line_SSE2;
	       } else {
		  DrawFn = pVram2RGB_x1_Line_SSE2;
	       }
            } else { // xfactor != 0
	       DrawFn = pVram2RGB_x1_Line_SSE2;
	    }
            break;
     case 2:
//            if(xfactor < xth){
	      if((w1 > 720) && (w0 <= 480)) {
		 DrawFn = pVram2RGB_x2_Line_SSE2;  // x2.5
	      } else if((w1 > 1360) && (w1 <= 1520)){
		 DrawFn = pVram2RGB_x225_Line_SSE2; // x2.25
	      } else if(w1 > 1520){
		 DrawFn = pVram2RGB_x4_Line_SSE2; // x3
	      } else {
		 DrawFn = pVram2RGB_x2_Line_SSE2; // x2
	      }
            break;
     case 3:
            if(xfactor < xth){
	       DrawFn = pVram2RGB_x4_Line_SSE2; // x3
            } else { // xfactor != 0
	       DrawFn = pVram2RGB_x4_Line_SSE2;
	    }
            break;
     case 4:
     case 5:
     case 6:
     case 7:
     case 8:
	      DrawFn = pVram2RGB_x4_Line_SSE2;
            break;
     default:
	      DrawFn = pVram2RGB_x1_Line_SSE2;
            break;
        }
        pDrawFn2 = (void *)DrawFn;
        return (void *)DrawFn;
}
#endif // USE_SSE2


// w0, h0 = Console
// w1, h1 = DrawMode
static void *XM7_SDLViewSelectScaler_Line(int w0 ,int h0, int w1, int h1)
{
    int wx0 = w0 >> 1; // w1/4
    int hy0 = h0 >> 1;
    int xfactor;
    int yfactor;
    int xth;
    void (*DrawFn)(Uint32 *src, Uint32 *dst, int xbegin, int xend, int y, int yrep);
    DrawFn = NULL;

#if defined(USE_SSE2)
   if(pCpuID != NULL){
      if(pCpuID->use_sse2) {
	 return XM7_SDLViewSelectScaler_Line_SSE2(w0, h0, w1, h1);
      }
   }
#endif
   
    xfactor = w1 % wx0;
    yfactor = h1 % hy0;
    xth = wx0 >> 1;
    if(iScaleFactor == (w1 / w0) && (pDrawFn2 != NULL)
      && (w1 == iOldW) && (h1 == iOldH))  return (void *)pDrawFn2;
    iScaleFactor = w1 / w0;
    iOldW = w1;
    iOldH = h1;
    switch(iScaleFactor){
     case 0:
            if(w0 > 480){
	        if((w1 < 480) || (h1 < 150)){
		   DrawFn = pVram2RGB_x05_Line;
		} else {
		   DrawFn = pVram2RGB_x1_Line;
		}
            } else {
                DrawFn = pVram2RGB_x1_Line;
            }
            break;

     case 1:
            if(xfactor < xth){
	       if(w1 > 720) {
		  DrawFn = pVram2RGB_x2_Line;
	       } else {
		  DrawFn = pVram2RGB_x1_Line;
	       }
            } else { // xfactor != 0
	       DrawFn = pVram2RGB_x2_Line;
	    }
            break;
     case 2:
//            if(xfactor < xth){
	      if((w1 > 720) && (w0 <= 480)) {
		 DrawFn = pVram2RGB_x2_Line;  // x2.5
	      } else if((w1 > 1360) && (w1 <= 1520)){
		 DrawFn = pVram2RGB_x225_Line; // x2.25
	      }else if(w1 > 1520){
		 DrawFn = pVram2RGB_x2_Line; // x3
	      } else {
		 DrawFn = pVram2RGB_x2_Line;
	      }
            break;
     case 3:
            if(xfactor < xth){
	       DrawFn = pVram2RGB_x4_Line; // x3
            } else { // xfactor != 0
	       DrawFn = pVram2RGB_x4_Line;
	    }
            break;
     case 4:
     case 5:
     case 6:
     case 7:
     case 8:
	      DrawFn = pVram2RGB_x4_Line;
            break;
     default:
	      DrawFn = pVram2RGB_x1_Line;
            break;
        }
        pDrawFn2 = (void *)DrawFn;
        return (void *)DrawFn;
}



void XM7_SDLViewUpdateSrc(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   void *Fn = NULL;
   void (*DrawFn2)(Uint32 *, Uint8 *, int , int , int, int);
   AG_Surface *Surface;
   
   Uint8 *pb;
   Uint32 *disp;
   Uint32 *src;
   Uint8 *dst;
   int yrep2;
   int y2, y3;
   int w;
   int h;
   int ww;
   int hh;
   int xx;
   int yy;
   int pitch;
   int bpp;
   int of;
   int yrep;
   int ymod;
   int yfact;
   int lcount;
   int xcache;
   BOOL flag = FALSE;

   Fn = AG_PTR(1);
   if(my == NULL) return;
   Surface = my->Surface;
   
   if(Surface == NULL) return;
   DrawSurface = Surface;
   w = Surface->w;
   h = Surface->h;
   pb = (Uint8 *)(Surface->pixels);
   pitch = Surface->pitch;
   bpp = Surface->format->BytesPerPixel;
   

   if(pVram2 == NULL) return;
   if(crt_flag == FALSE) {
      AG_Rect rr;
      AG_Color cc;
      
      cc.r = 0x00;
      cc.g = 0x00;
      cc.b = 0x00;
      cc.a = 0xff;
      
      LockVram();
      AG_ObjectLock(AGOBJECT(my));
      AG_SurfaceLock(Surface);
      AG_FillRect(Surface, NULL, cc);
      AG_ObjectUnlock(AGOBJECT(my));
      UnlockVram();
      return;
   }
   
   switch(bMode){
    case SCR_200LINE:
        ww = 640;
        hh = 200;
        break;
    case SCR_400LINE:
        ww = 640;
        hh = 400;
        break;
    default:
        ww = 320;
        hh = 200;
        break;
   }
   Fn = XM7_SDLViewSelectScaler_Line(ww , hh, w, h);
   if(Fn != NULL) {
      DrawFn2 = (void (*)(Uint32 *, Uint8 *, int , int , int, int))Fn;
   }

   
   if(h > hh) {
      ymod = h % hh;
      yrep = h / hh;
   } else {
      ymod = h % hh;
      yrep = 1;
   }
   
    if(Fn == NULL) return; 
    src = pVram2;
    LockVram();
    AG_ObjectLock(AGOBJECT(my));

   if(nRenderMethod == RENDERING_RASTER) {
      if(my->forceredraw != 0){
	  for(yy = 0; yy < hh; yy++) {
	     bDrawLine[yy] = TRUE;
	  }
	  my->forceredraw = 0;
       }
       Surface = GetDrawSurface();
       if(Surface == NULL)       goto _end1;
       AG_SurfaceLock(Surface);
       dst = (Uint8 *)(Surface->pixels);
#ifdef _OPENMP
// #pragma omp parallel for shared(hh, bDrawLine, yrep, ww, src, Surface) private(dst, y2, y3)
#endif
      for(yy = 0 ; yy < hh; yy++) {
/*
*  Virtual VRAM -> Real Surface:
*/
	 if(bDrawLine[yy] == TRUE){
	    _prefetch_data_read_l1(&src[yy * 80], ww);
	    y2 = (h * yy ) / hh;
	    y3 = (h * (yy + 1)) / hh;
	    dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
	    yrep2 = y3 - y2;
	    if(yrep2 < 1) yrep2 = 1;
	    DrawFn2(src, dst, 0, ww, yy, yrep2);
	    bDrawLine[yy] = FALSE;
	    flag = TRUE;
	 }
	 dst = dst + (yrep2 * Surface->pitch);
      }
      AG_SurfaceUnlock(Surface);
      // BREAK.
      goto _end1;
   } else { // Block
      if(my->forceredraw != 0){
	 for(yy = 0; yy < hh; yy += 8) {
            for(xx = 0; xx < ww; xx +=8 ){
	       SDLDrawFlag.write[xx >> 3][yy >> 3] = TRUE;
            }
	 }
	 my->forceredraw = 0;
      }
   }
   
/*
 * Below is BLOCK or FULL.
 * Not use from line-rendering.
 */

   Surface = GetDrawSurface();
   if(Surface == NULL) goto _end1;
   AG_SurfaceLock(Surface);

#ifdef _OPENMP
// # pragma omp parallel for shared(pb, SDLDrawFlag, ww, hh, src) private(disp, of, xx, lcount, xcache, y2, y3, dst)
#endif
    for(yy = 0 ; yy < hh; yy += 8) {
       _prefetch_data_read_l1(&src[(yy + 0) * 80], ww);
       _prefetch_data_read_l1(&src[(yy + 1) * 80], ww);
       _prefetch_data_read_l1(&src[(yy + 2) * 80], ww);
       _prefetch_data_read_l1(&src[(yy + 3) * 80], ww);
       _prefetch_data_read_l1(&src[(yy + 4) * 80], ww);
       _prefetch_data_read_l1(&src[(yy + 5) * 80], ww);
       _prefetch_data_read_l1(&src[(yy + 6) * 80], ww);
       _prefetch_data_read_l1(&src[(yy + 7) * 80], ww);
       lcount = 0;
       xcache = 0;
       dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
        for(xx = 0; xx < ww; xx += 8) {
/*
*  Virtual VRAM -> Real Surface:
*                disp = (Uint32 *)(pb + xx  * bpp + yy * pitch);
*                of = (xx % 8) + (xx / 8) * (8 * 8)
*                    + (yy % 8) * 8 + (yy / 8) * 640 * 8;
*                *disp = src[of];
** // xx,yy = 1scale(not 8)
*/
//            if(xx >= w) continue;
	   if(SDLDrawFlag.write[xx >> 3][yy >> 3]) {
	      lcount += 8;
	      SDLDrawFlag.write[xx >> 3][yy >> 3] = FALSE;
	   } else {
	      if(lcount > 0) {
		 int yy2;
		 //	      disp = (Uint32 *)pb;
		 //	      of = (xx *8) + yy * ww;
		 //	      DrawFn(&src[of], disp, xx, yy, yrep);
		 for(yy2 = 0; yy2 < 8; yy2++) {
		    y2 = (h * (yy + yy2)) / hh;
		    y3 = (h * (yy + yy2 + 1)) / hh;
		    dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
		    yrep2 = y3 - y2;
		    if(yrep2 < 1) yrep2 = 1;
		    DrawFn2(src, dst, xcache, xcache + lcount, yy + yy2 , yrep2);
		    flag = TRUE;
		 }
	      }
	      xcache = xx + 8;
	      lcount = 0;
	   }
	}
       
       if(lcount > 0) {
	  int yy2;
	  //	      disp = (Uint32 *)pb;
	  //	      of = (xx *8) + yy * ww;
	  //	      DrawFn(&src[of], disp, xx, yy, yrep);
	  for(yy2 = 0; yy2 < 8; yy2++) {
	     y2 = (h * (yy + yy2)) / hh;
	     y3 = (h * (yy + yy2 + 1)) / hh;
	     dst = (Uint8 *)(Surface->pixels + Surface->pitch * y2);
	     yrep2 = y3 - y2;
	     if(yrep2 < 1) yrep2 = 1;
	     DrawFn2(src, dst, xcache, xcache + lcount, yy + yy2 , yrep2);
	     flag = TRUE;
	  }
       }
//			if(yy >= h) continue;
    }
   AG_SurfaceUnlock(Surface);
      
_end1:   
   AG_ObjectUnlock(AGOBJECT(my));

//   AG_PixmapUpdateCurrentSurface(my);
//   my->mySurface = AG_WidgetMapSurfaceNODUP(my, Surface);
//	AG_WidgetUpdateSurface(my, my->mySurface);
   UnlockVram();
   return;
}

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
   extern void pVram2RGB_x125_Line(Uint32 *src,  Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x125.c , raster render
   extern void pVram2RGB_x2_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2.c , raster render.
   extern void pVram2RGB_x225_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x225.c , raster render.
   extern void pVram2RGB_x4_Line(Uint32 *src,  Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x4.c , raster render.

#if defined(USE_SSE2) // scaler/sse2/
   extern void pVram2RGB_x1_Line_SSE2(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x1_sse2.c , raster render
   extern void pVram2RGB_x125_Line_SSE2(Uint32 *src, Uint8 *dst, int x, int xend, int y, int yrep); // scaler_x125_sse2.c , raster render
   extern void pVram2RGB_x2_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x2_sse2.c , raster render.
   extern void pVram2RGB_x225_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x225_sse2.c , raster render.
   extern void pVram2RGB_x4_Line_SSE2(Uint32 *src, Uint8 *dst, int xbegin, int xend, int y, int yrep); // scaler_x4_sse2.c , raster render.
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
	       if(w1 > 900) {
		  DrawFn = pVram2RGB_x2_Line_SSE2; // 1.5?
	       } else if(w1 > 720) {
		  DrawFn = pVram2RGB_x125_Line_SSE2; // 1.25
	       } else {
		  DrawFn = pVram2RGB_x1_Line_SSE2; // 1.0
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
       if(w1 > 900) {
	  DrawFn = pVram2RGB_x2_Line; // 1.5?
       } else if(w1 > 720) {
	  DrawFn = pVram2RGB_x125_Line; // 1.25
       } else {
	  DrawFn = pVram2RGB_x1_Line; // 1.0
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

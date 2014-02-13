/*
 * XM7 : 断片化VRAMでの書き込み
 * 2011 (C) K.Ohta <whatithis.sowhat@gmail.com>
 */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
//#include <agar/gui/opengl.h>

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>

#ifdef _OPENMP
#include <omp.h>
#endif // _OPENMP

#include "api_draw.h"
//#include "api_scaler.h"
#include "agar_xm7.h"
#include "agar_cfg.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "sdl_cpuid.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"

extern "C" {
extern BOOL bUseOpenCL;
extern BOOL bUseSIMD;
extern AG_Surface *DrawSurface;
extern struct  XM7_CPUID *pCpuID;
}


#ifdef USE_OPENGL
#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
#endif // _USE_OPENCL
#endif // USE_OPENGL

static void BuildVirtualVram(Uint32 *pp, int x, int y, int w, int h, int mode)
{
    int xx;
    int yy;
    int ww;
    int hh;
    Uint32 *p;
   if(pp == NULL) return;
   if(pVirtualVramBuilder == NULL) return;
    LockVram();
    if(SDLDrawFlag.DPaletteChanged) { // Palette changed
#ifdef _OPENMP
       #pragma omp parallel for shared(pp, SDLDrawFlag, hh, ww, mode) private(p, xx)
#endif
	for(yy = (y >> 3); yy < hh ; yy++) {
	   p = &pp[64 * ((x >> 3) + 80 * yy)];
	   for(xx = (x >> 3); xx < ww ; xx++) {
	      pVirtualVramBuilder->vram_block(p, xx , yy << 3, sizeof(Uint32) * 8, mode);
	      SDLDrawFlag.write[xx][yy] = TRUE;
	      SDLDrawFlag.read[xx][yy]  = FALSE;
	      p += 64;
	   }
	}
	SDLDrawFlag.Drawn = TRUE;
	SDLDrawFlag.DPaletteChanged = FALSE;
     } else { // Palette not changed
#ifdef _OPENMP
       #pragma omp parallel for shared(pp, SDLDrawFlag, hh, ww, mode) private(p, xx)
#endif
	for(yy = (y >> 3); yy < hh ; yy++) {
   	   p = &pp[64 * ((x >> 3) + 80 * yy)];
	   for(xx = (x >> 3); xx < ww ; xx++) {
	      if(SDLDrawFlag.read[xx][yy]) {
		 pVirtualVramBuilder->vram_block(p, xx , yy << 3, sizeof(Uint32) * 8, mode);
		 SDLDrawFlag.write[xx][yy] = TRUE;
		 SDLDrawFlag.read[xx][yy]  = FALSE;
		 SDLDrawFlag.Drawn = TRUE;
	      }
	      p += 64;
	   }
	}
     }
   
    UnlockVram();
}


void BuildVirtualVram_Raster(Uint32 *pp, int xbegin, int xend, int y, int mode)
{
    int xx;
    int xwidth;
    Uint32 *p;
   

   if(pp == NULL) return;
   if(pVirtualVramBuilder == NULL) return;
   if((bMode == SCR_4096) || (bMode == SCR_262144)) { 
	xwidth = 320;
   } else {
	xwidth = 640;
   }
   
   
//    LockVram();
    if(bDirtyLine[y] != TRUE) return;
    if((xbegin != 0) || (xend < (xwidth >> 3))){
       p = &pp[y * xwidth + xbegin * 8];
       pVirtualVramBuilder->vram_windowline(p, y, y + 1, xbegin, xend, mode);
    } else { // Not Windowed mode
       p = &pp[xwidth * y];
       pVirtualVramBuilder->vram_line(p, y, y + 1, mode);
     }
//    bDirtyLine[y] = FALSE;
//       UnlockVram();
 
}



static void BuildVirtualVram4096_SSE2(Uint32 *pp, int x, int y ,int  w, int h, int mode)
{
    int xx;
    int yy;
    int ww;
    int hh;
    Uint32 *p;
   
#ifndef USE_SSE2
   BuildVirtualVram4096(pp, x, y , w, h, mode);
   return;
#endif
   if(pp == NULL) return;
   ww = (w + x) >> 3;
   hh = (h + y) >> 3;

   LockVram();
//    p = pp;
    if(SDLDrawFlag.APaletteChanged) { // Palette changed
#ifdef _OPENMP
       #pragma omp parallel for shared(pp, SDLDrawFlag, hh, ww, mode) private(p, xx)
#endif
       for(yy = (y >> 3); yy < hh ; yy++) {
	  p = &pp[64 * ((x >> 3) + 40 * yy)];
	  for(xx = (x >> 3); xx < ww ; xx++) {
	     CreateVirtualVram4096_1Pcs_SSE2(p, xx, yy << 3, 8 * sizeof(Uint32), mode);
	     SDLDrawFlag.write[xx][yy] = TRUE;
	     SDLDrawFlag.read[xx][yy] = FALSE;
	     p += 64;
	  }
       }
       SDLDrawFlag.Drawn = TRUE;
       SDLDrawFlag.APaletteChanged = FALSE;
    } else {
	// Palette not changed   
#ifdef _OPENMP
       #pragma omp parallel for shared(pp, SDLDrawFlag, hh, ww, mode) private(p, xx)
#endif
       for(yy = (y >> 3); yy < hh ; yy++) {
	  p = &pp[64 * ((x >> 3) + 40 * yy)];
	  for(xx = (x >> 3); xx < ww ; xx++) {
	     if(SDLDrawFlag.read[xx][yy]) {
                CreateVirtualVram4096_1Pcs_SSE2(p, xx, yy << 3, 8 * sizeof(Uint32), mode);
                SDLDrawFlag.write[xx][yy] = TRUE;
                SDLDrawFlag.read[xx][yy] = FALSE;
		SDLDrawFlag.Drawn = TRUE;
	     }
	     p += 64;
	  }
       }
    }
   
    UnlockVram();
}

static void BuildVirtualVram256k_SSE2(Uint32 *pp, int x, int y, int  w, int h, int mpage)
{
    int xx;
    int yy;
    int ww;
    int hh;
    Uint32 *p;

#ifndef USE_SSE2
   BuildVirtualVram256k(pp, x, y, w, h, mpage);
   return;
#endif
   if(pp == NULL) return;
   ww = (w + x) >> 3;
   hh = (h + y) >> 3;
   LockVram();
//    p = pp;
#ifdef _OPENMP
       #pragma omp parallel for shared(pp, SDLDrawFlag, hh, ww) private(p, xx)
#endif
    for(yy = (y >> 3); yy < hh ; yy++) {
        for(xx = (x >> 3); xx < ww ; xx++) {
            if(SDLDrawFlag.read[xx][yy]) {
                p = &pp[64 * (xx + 40 * yy)];
		 CreateVirtualVram256k_1Pcs_SSE2(p, xx , yy << 3, sizeof(Uint32) * 8, mpage);
                SDLDrawFlag.read[xx][yy] = FALSE;
                SDLDrawFlag.write[xx][yy] = TRUE;
                SDLDrawFlag.Drawn = TRUE;
            }
        }
    }
    UnlockVram();
}

void PutVram_AG_SP(AG_Surface *p, int x, int y, int w, int h,  Uint32 mpage)
{
	int xx, yy;
	int hh, ww;
	int addr;
	int ofset;
	int size;
	Uint32 *pp;

	// Test

    if((vram_pb == NULL) || (vram_pg == NULL) || (vram_pr == NULL)) return;
    if(pVirtualVramBuilder == NULL) return;
   
    LockVram();
#ifdef _USE_OPENCL
   if((cldraw != NULL) && (GLDrawArea != NULL)){ // Snip builing-viryual-vram if GLCL mode.
//        bVramUpdateFlag = TRUE;
        SDLDrawFlag.Drawn = TRUE;
        UnlockVram();
        return;
   }
#endif   
   if(pVram2 == NULL) return;
   pp = pVram2;

   if(pp == NULL) {
      UnlockVram();
      return;
   }
   
   if((bClearFlag)) {
      memset(pp, 0x00, 640 * 400 * sizeof(Uint32)); // モードが変更されてるので仮想VRAMクリア
      if(nRenderMethod == RENDERING_RASTER) {
	 SetDirtyFlag(0, 400, TRUE);
//	 Palet320();
//	 Palet640();
      } else {
	 SetDrawFlag(TRUE);
      }
      bClearFlag = FALSE;
   }
//   if((pCpuID == NULL) || (bUseSIMD != TRUE)){
      BuildVirtualVram(pp, x, y, w, h, bMode);
      UnlockVram();
      return;
//   } 
   UnlockVram();
}

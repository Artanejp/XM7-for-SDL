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
#include "cache_wrapper.h"

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
    int xfactor;
    Uint32 *p;
   
   if(pp == NULL) return;
   if(pVirtualVramBuilder == NULL) return;
   ww = (w + x) >> 3;
   hh = (h + y) >> 3;
   if((bMode == SCR_4096) || (bMode == SCR_262144)) {
      _prefetch_data_read_l1(rgbAnalogGDI, sizeof(Uint32) * 4096);
//      xfactor = 40;
      xfactor = 80;
   } else {
      xfactor = 80;
      _prefetch_data_read_l1(rgbTTLGDI, sizeof(Uint32) * 8);
   }
      
   LockVram();
   _prefetch_data_read_l1(&SDLDrawFlag, sizeof(SDLDrawFlag));
   _prefetch_data_write_l1(&SDLDrawFlag, sizeof(SDLDrawFlag));
   _prefetch_data_read_l1(aPlanes, sizeof(Uint32) * 256 * 8 * 12); // 98KB (!), priority = 1.

   if(SDLDrawFlag.DPaletteChanged || SDLDrawFlag.APaletteChanged) { // Palette changed
#ifdef _OPENMP
       #pragma omp parallel for shared(pp, SDLDrawFlag, hh, ww, mode, xfactor) private(p, xx)
#endif
	for(yy = (y >> 3); yy < hh ; yy++) {
	   p = &pp[(xfactor * (yy << 3) + (x >> 3)) << 3];
	   for(xx = (x >> 3); xx < ww ; xx++) {
	      pVirtualVramBuilder->vram_block(p, xx , yy << 3, xfactor << 3, mode);
	      SDLDrawFlag.write[xx][yy] = TRUE;
	      SDLDrawFlag.read[xx][yy]  = FALSE;
	      p += 8;
	   }
	}
	SDLDrawFlag.Drawn = TRUE;
	SDLDrawFlag.DPaletteChanged = FALSE;
	SDLDrawFlag.APaletteChanged = FALSE;
     } else { // Palette not changed
//#ifdef _OPENMP
//       #pragma omp parallel for shared(pp, SDLDrawFlag, hh, ww, mode, xfactor) private(p, xx)
//#endif
	for(yy = (y >> 3); yy < hh ; yy++) {
   	   p = &pp[(xfactor * (yy << 3) + (x >> 3)) << 3];
	   for(xx = (x >> 3); xx < ww ; xx++) {
	      if(SDLDrawFlag.read[xx][yy]) {
		 pVirtualVramBuilder->vram_block(p, xx , yy << 3, xfactor << 3, mode);
		 SDLDrawFlag.write[xx][yy] = TRUE;
		 SDLDrawFlag.read[xx][yy]  = FALSE;
		 SDLDrawFlag.Drawn = TRUE;
	      }
	      p += 8;
	   }
	}
     }
    UnlockVram();
}

void BuildVirtualVram_RasterWindow(Uint32 *pp, int xbegin, int xend, int y, int mode)
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
   p = &pp[xwidth * y];

   pVirtualVramBuilder->vram_windowline(p, y, y + 1, xbegin, xend, mode);
//       UnlockVram();
}

void BuildVirtualVram_Raster(Uint32 *pp, int y, int mode)
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
   p = &pp[xwidth * y];
   pVirtualVramBuilder->vram_line(p, y, y + 1, mode);
//       UnlockVram();
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
//        SDLDrawFlag.Drawn = TRUE;
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
      BuildVirtualVram(pp, x, y, w, h, mpage);
      UnlockVram();
      return;
//   } 
   UnlockVram();
}

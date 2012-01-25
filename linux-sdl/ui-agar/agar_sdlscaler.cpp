/*
* FM-7 Emulator "XM7"
* Virtual Vram Display(Agar widget version)
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* History:
* Jan 18,2012 From demos/customwidget/mywidget.[c|h]
* Jan 20,2012 Separete subroutines.
*/

#include "agar_sdlview.h"
#include "api_vram.h"
#include "api_draw.h"
//#include "api_scaler.h"
#include "api_kbd.h"


static void pVram2RGB(XM7_SDLView *my, Uint32 *src, Uint32 *dst, int x, int y)
{
    Uint32 *dbase;
    Uint32 *dsrc = src;
    Uint8 *pb = (Uint8 *)dst;
    int of;
    int pitch;
    int yy;

//    of = x + y * 640;
    of = 0;
    dbase = (Uint32 *)(pb + x  * my->Surface->format->BytesPerPixel
                        + y * my->Surface->pitch);
    pitch = my->Surface->pitch / sizeof(Uint32);

    for(yy = 0; yy < 8; yy++){
        dbase[0] = dsrc[of];
        dbase[1] = dsrc[of + 1];
        dbase[2] = dsrc[of + 2];
        dbase[3] = dsrc[of + 3];
        dbase[4] = dsrc[of + 4];
        dbase[5] = dsrc[of + 5];
        dbase[6] = dsrc[of + 6];
        dbase[7] = dsrc[of + 7];
        dbase += pitch;
        of += 8;
    }

}

void XM7_SDLViewUpdateSrc(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   Uint8 *pb;
   Uint32 *disp;
   Uint32 *src;
   int w;
   int h;
   int ww;
   int hh;
   int xx;
   int yy;
   int pitch;
   int bpp;
   int of;

   if(my == NULL) return;
   if(my->Surface == NULL) return;
   w = my->Surface->w;
   h = my->Surface->h;

   if(pVirtualVram == NULL) return;
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
   if(ww >= w) ww = (w / 8) * 8;
   if(hh >= h) hh = (h / 8) * 8;
    pb = (Uint8 *)(my->Surface->pixels);
    pitch = my->Surface->pitch;
    bpp = my->Surface->format->BytesPerPixel;
    src = &(pVirtualVram->pVram[0][0]);

    LockVram();
    AG_SurfaceLock(my->Surface);
    if(my->forceredraw != 0){
        for(yy = 0; yy < hh; yy += 8) {
            for(xx = 0; xx < ww; xx +=8 ){
                SDLDrawFlag.write[xx >> 3][yy >> 3] = TRUE;
            }
        }
        my->forceredraw = 0;
    }

#ifdef _OPENMP
       #pragma omp parallel for shared(pb, SDLDrawFlag, ww, hh, src) private(disp, of, xx)
#endif
    for(yy = 0 ; yy < hh; yy+=8) {
        for(xx = 0; xx < ww; xx+=8) {
/*
*  Virtual VRAM -> Real Surface:
*                disp = (Uint32 *)(pb + xx  * bpp + yy * pitch);
*                of = (xx % 8) + (xx / 8) * (8 * 8)
*                    + (yy % 8) * 8 + (yy / 8) * 640 * 8;
*                *disp = src[of];
** // xx,yy = 1scale(not 8)
*/
            if(xx >= w) continue;
                if(SDLDrawFlag.write[xx >> 3][yy >> 3]){
                    disp = (Uint32 *)pb;
                    of = (xx *8) + yy * ww;
                    pVram2RGB(my, &src[of], disp, xx, yy);
                    SDLDrawFlag.write[xx >> 3][yy >> 3] = FALSE;
                }
			}
			if(yy >= h) continue;
	}
	AG_SurfaceUnlock(my->Surface);
//    my->mySurface = AG_WidgetMapSurfaceNODUP(my, my->Surface);
	AG_WidgetUpdateSurface(my, my->mySurface);
    UnlockVram();

}

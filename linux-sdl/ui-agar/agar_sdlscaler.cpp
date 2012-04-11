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


void pVram2RGB(XM7_SDLView *my, Uint32 *src, Uint32 *dst, int x, int y, int yrep)
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


void pVram2RGB_x1(XM7_SDLView *my, Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi *b;
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
   d1 = (Uint32 *)((Uint8 *)(my->Surface->pixels) + x * my->Surface->format->BytesPerPixel
                        + y * yrep * my->Surface->pitch);
   if(h <= ((y + 8) * yrep)) {
      hh = (h - y * yrep) / yrep;
   } else {
      hh = 8;
   }

   pitch = my->Surface->pitch / sizeof(Uint32);
   if(w < (x  + 8)) {
    int j;
    p = src;
    ww = w - x;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 2, i++){
        d2 = d1;
            for(j = 0; j < yrep; j++)
             d2[xx + pitch * j] = p[i];
        }
        d1 += (pitch * yrep);
      }
   } else { // inside align
      v8hi *bv;
      int j;

        b = (v8hi *)src;
        for(yy = 0; yy < hh; yy++){
            for(j = 0; j < yrep; j++) {
                bv = (v8hi *)d1;
                *bv = *b;
                d1 += pitch;
            }
        b++;
        }
    }
}

// Zoom 2x2
void pVram2RGB_x2(XM7_SDLView *my, Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi *b;

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
   d1 = (Uint32 *)((Uint8 *)(my->Surface->pixels) + x * 2 * my->Surface->format->BytesPerPixel
                        + y * yrep * my->Surface->pitch);
   if(h <= ((y + 8) * yrep)) {
      hh = (h - y * yrep) / yrep;
   } else {
      hh = 8;
   }

   pitch = my->Surface->pitch / sizeof(Uint32);
   if(w < (x * 2 + 16)) {
    int j;
    Uint32 d0;

    p = src;
    ww = w - x * 2;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 2, i++){
            d2 = d1;
            d0 = p[i];
            for(j = 0; j < yrep; j++){
                d2[xx] = d0;
                d2[xx + 1] = d0;
                d2 += pitch;
            }
        }
        d1 += (pitch * yrep);
        p += 8;
      }
   } else { // inside align
    int j;
    v8hi b2;
    v8hi b3;
    v8hi *b2p;
    b = (v8hi *)src;
    for(yy = 0; yy < hh; yy++){
       b2.i[0] = b2.i[1] = b->i[0];
       b2.i[2] = b2.i[3] = b->i[1];
       b2.i[4] = b2.i[5] = b->i[2];
       b2.i[7] = b2.i[6] = b->i[3];

       b3.i[0] = b3.i[1] = b->i[4];
       b3.i[2] = b3.i[3] = b->i[5];
       b3.i[4] = b3.i[5] = b->i[6];
       b3.i[7] = b3.i[6] = b->i[7];


       switch(yrep) {
	case 0:
	case 1:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;
	  break;
	case 2:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;
	  break;
	case 3:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;
	  
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;
	  break;
        case 4:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;
	  break;
	default:
	  for(j = 0; j < yrep; j++) {
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  d1 += pitch;
	  }
	  break;
       }
       
       b++;
     }
   }
}


void pVram2RGB_x3(XM7_SDLView *my, Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi *b;
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
   d1 = (Uint32 *)((Uint8 *)(my->Surface->pixels) + x * 4 * my->Surface->format->BytesPerPixel
                        + y * yrep * my->Surface->pitch);
   if(h <= ((y + 8) * yrep)) {
      hh = (h - y * yrep) / yrep;
   } else {
      hh = 8;
   }

   pitch = my->Surface->pitch / sizeof(Uint32);
   if(w <= (x * 3 + 24)) {
       int j;
       Uint32 d0;
      p = src;

      ww = w - x * 3;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 3, i++){
            d2 = d1;
            d0 = p[i];
            for(j = 0; j < yrep; j++) {
                d2[xx] = d0;
                d2[xx + 1] = d0;
                d2[xx + 2] = d0;
                d2 += pitch;
            }
        }
	 d1 += (pitch * yrep);
	 p += 8;
    }
   } else { // inside align
      int j;
      v8hi b2,b3,b4;
      v8hi *b2p;
      b = (v8hi *)src;
     for(yy = 0; yy < hh; yy++){
	b2.i[0] = b2.i[1] = b2.i[2] = b->i[0];
	b2.i[3] = b2.i[4] = b2.i[5] = b->i[1];
	b2.i[6] = b2.i[7] = b3.i[0] = b->i[2];
	b3.i[1] = b3.i[2] = b3.i[3] = b->i[3];
	b3.i[4] = b3.i[5] = b3.i[6] = b->i[4];
	b3.i[7] = b4.i[0] = b4.i[1] = b->i[5];
	b4.i[2] = b2.i[3] = b4.i[4] = b->i[6];
	b4.i[5] = b2.i[6] = b4.i[7] = b->i[7];


       switch(yrep) {
	case 0:
	case 1:
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	     break;
	case 2:
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;

	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	     break;
	case 3:
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	  
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	  
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	     break;
	case 4:
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	  
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	  
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	  
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	     break;
	default:
	  for(j = 0; j < yrep; j++) {
	     b2p = (v8hi *)d1;
	     b2p[0] = b2;
	     b2p[1] = b3;
	     b2p[2] = b4;
	     d1 += pitch;
	  }
	  break;
       }
       b++;
     }
   }
}

void pVram2RGB_x4(XM7_SDLView *my, Uint32 *src, Uint32 *dst, int x, int y, int yrep)
{
   v8hi *b;
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
   d1 = (Uint32 *)((Uint8 *)(my->Surface->pixels) + x * 4 * my->Surface->format->BytesPerPixel
                        + y * yrep * my->Surface->pitch);
   if(h <= ((y + 8) * yrep)) {
      hh = (h - y * yrep) / yrep;
   } else {
      hh = 8;
   }

   pitch = my->Surface->pitch / sizeof(Uint32);
   if(w <= (x * 4 + 32)) {
       int j;
       Uint32 d0;
      p = src;

      ww = w - x * 4;
    for(yy = 0; yy < hh ; yy++) {
        i = 0;
        for(xx = 0; xx < ww; xx += 4, i++){
            d2 = d1;
            d0 = p[i];
            for(j = 0; j < yrep; j++) {
                d2[xx] = d0;
                d2[xx + 1] = d0;
                d2[xx + 2] = d0;
                d2[xx + 3] = d0;
                d2 += pitch;
            }
        }
	 d1 += (pitch * yrep);
	 p += 8;
    }
   } else { // inside align
      v8hi b2, b3, b4, b5;
      v8hi *b2p;
      int j;
      b = (v8hi *)src;
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
       switch(yrep){
	case 0:
	case 1:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;
	  break;
        case 2:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;

  	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;
	  break;
	case 3:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;
	  
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;
	  
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;
	  break;
	case 4:
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;

	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;
	  break;
	default:
	for(j = 0; j < yrep; j++) {
	  b2p = (v8hi *)d1;
	  b2p[0] = b2;
	  b2p[1] = b3;
	  b2p[2] = b4;
	  b2p[3] = b5;
	  d1 += pitch;
	}
       }
	
       b++;
     }
   }
}

// w0, h0 = Console
// w1, h1 = DrawMode
static void *XM7_SDLViewSelectScaler(int w0 ,int h0, int w1, int h1)
{
    int wx0 = w0 >> 1; // w1/4
    int hy0 = h0 >> 1;
    int xfactor;
    int yfactor;
    int xth;
    void (*DrawFn)(XM7_SDLView *, Uint32 *, Uint32 *, int , int, int);


    xfactor = w1 % wx0;
    yfactor = h1 % hy0;
    xth = wx0 >> 1;
    switch(w1 / w0){
            case 0:
            if(xfactor < xth){
                DrawFn = NULL;
            } else {
                DrawFn = pVram2RGB_x1;
            }
            break;
            case 1:
            if(xfactor < xth){
              DrawFn = pVram2RGB_x1;
            } else { // xfactor != 0
              DrawFn = pVram2RGB_x2;
            }
            break;
            case 2:
            if(xfactor < xth){
              DrawFn = pVram2RGB_x2;
            } else { // xfactor != 0
              DrawFn = pVram2RGB_x3;
            }
            break;
            case 3:
            if(xfactor < xth){
              DrawFn = pVram2RGB_x3;
            } else { // xfactor != 0
              DrawFn = pVram2RGB_x4;
            }
            break;
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                DrawFn = pVram2RGB_x4;
            break;
            default:
                DrawFn = pVram2RGB_x1;
                break;
        }
        return (void *)DrawFn;
}
void XM7_SDLViewUpdateSrc(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   void *Fn = AG_PTR(1);
   void (*DrawFn)(XM7_SDLView *, Uint32 *, Uint32 *, int , int, int);

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
   int yrep;
   int tmp;

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
    if(Fn == NULL){
        Fn = XM7_SDLViewSelectScaler(ww , hh, w, h);
    }
    if(Fn == NULL){
        Fn =(void *) pVram2RGB;
    }
    DrawFn =(void (*)(XM7_SDLView *, Uint32 *, Uint32 *, int , int, int))Fn;
    tmp = h % hh;
    yrep = h / hh;
    if(tmp > (hh >> 1)){
        yrep++;
    }

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
                    DrawFn(my, &src[of], disp, xx, yy, yrep);
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

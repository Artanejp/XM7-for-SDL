/*
 * Draw Console (NON-GL Mode)
 *
 * K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 *    06/05/2011 Initial
 */


 #include <SDL.h>
#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
#include "api_draw.h"
#include "api_scaler.h"
#include "agar_xm7.h"
#include "agar_gldraw.h"


static AG_Surface *pixvram;
static Uint32 vramwidth;
static Uint32 vramheight;
static void (*getvram)(Uint32, Uint32 *, Uint32);
static BOOL InitVideo;
extern SDL_semaphore *VramSem;

static inline void putdot32(Uint8 *addr, Uint32 c)
{
	Uint32 *addr32 = (Uint32 *)addr;
	*addr32 = c;
}


static inline void putword32(Uint32 *disp, Uint32 *cbuf)
{
		putdot32((Uint8 *)&disp[0], cbuf[7]);
		putdot32((Uint8 *)&disp[1], cbuf[6]);
		putdot32((Uint8 *)&disp[2], cbuf[5]);
		putdot32((Uint8 *)&disp[3], cbuf[4]);
		putdot32((Uint8 *)&disp[4], cbuf[3]);
		putdot32((Uint8 *)&disp[5], cbuf[2]);
		putdot32((Uint8 *)&disp[6], cbuf[1]);
		putdot32((Uint8 *)&disp[7], cbuf[0]);
}

static inline void putdot16(Uint8 *addr, Uint16 c)
{
	Uint16 *addr16 = (Uint16 *)addr;
	*addr16 = c;
}


static inline void putword16(Uint16 *disp, Uint32 *cbuf)
{
		putdot16((Uint8 *)&disp[0], cbuf[7]);
		putdot16((Uint8 *)&disp[1], cbuf[6]);
		putdot16((Uint8 *)&disp[2], cbuf[5]);
		putdot16((Uint8 *)&disp[3], cbuf[4]);
		putdot16((Uint8 *)&disp[4], cbuf[3]);
		putdot16((Uint8 *)&disp[5], cbuf[2]);
		putdot16((Uint8 *)&disp[6], cbuf[1]);
		putdot16((Uint8 *)&disp[7], cbuf[0]);
}


void CalcPalette_AG(Uint32 *palette, Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint32 ds;
	AG_Surface *dst = pixvram;
	if(palette == NULL) return;
	if(dst == NULL) return;

    ds = ( r << dst->format->Rshift) & dst->format->Rmask +
     ( r << dst->format->Gshift) & dst->format->Gmask +
     ( r << dst->format->Bshift) & dst->format->Bmask +
     ( r << dst->format->Ashift) & dst->format->Amask;
	palette[src] = ds;
}

void InitDraw_AG(int w, int h)
{
	Uint32 flags;

	if(InitVideo) return;
    InitVideo = TRUE;

	if((pixvram == NULL) &&(w != 0) &&(h != 0)) {
	    pixvram = AG_SurfaceStdRGB(w, h);
	}
	if(VramSem == NULL) {
		VramSem = SDL_CreateSemaphore(1);
		if(VramSem) SDL_SemPost(VramSem);
	}
	return;
}


void DetachDraw_AG()
{
	if(VramSem != NULL) {
		SDL_SemWait(VramSem);
		SDL_DestroySemaphore(VramSem);
		VramSem = NULL;
	}
	if(pixvram){
		AG_SurfaceFree(pixvram);
		pixvram = NULL;
	}
}


/*
 * agar_gldraw.cpp
 *
 *  Created on: 2011/01/21
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */

//
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
static GLuint textureid;
static 	int viewport_x;
static int viewport_y;
static int viewport_h;
static int viewport_w;
static int offset_x;
static int offset_y;
static int osd_w;
static int osd_h;
static AG_PixelFormat format;
static void (*getvram)(Uint32, Uint32 *, Uint32);
static BOOL InitVideo;
static SDL_semaphore *VramSem;
extern AG_GLView *OsdArea;

static inline void putdot(Uint8 *addr, Uint32 c)
{
	Uint32 *addr32 = (Uint32 *)addr;
	*addr32 = c;
}


static inline void putword(Uint32 *disp, Uint32 *cbuf)
{
		putdot((GLubyte *)&disp[0], cbuf[7]);
		putdot((GLubyte *)&disp[1], cbuf[6]);
		putdot((GLubyte *)&disp[2], cbuf[5]);
		putdot((GLubyte *)&disp[3], cbuf[4]);
		putdot((GLubyte *)&disp[4], cbuf[3]);
		putdot((GLubyte *)&disp[5], cbuf[2]);
		putdot((GLubyte *)&disp[6], cbuf[1]);
		putdot((GLubyte *)&disp[7], cbuf[0]);
}


static void SetOffset(int x, int y)
{
	offset_x = x;
	offset_y = y;
}

void CalcPalette_AG_GL(Uint32 *palette, Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint32 ds;
	if(palette == NULL) return;

#if AG_LITTLE_ENDIAN
	ds =r + (g << 8)+ (b << 16) + (a<<24);
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
	palette[src] = ds;
}

void InitGL_AG_GL(int w, int h)
{
	Uint32 flags;
	int bpp = 32;
	int rgb_size[3];

	if(InitVideo) return;
    InitVideo = TRUE;
	flags = SDL_OPENGL | SDL_RESIZABLE;
    switch (bpp) {
         case 8:
             rgb_size[0] = 3;
             rgb_size[1] = 3;
             rgb_size[2] = 2;
             break;
         case 15:
         case 16:
             rgb_size[0] = 5;
             rgb_size[1] = 5;
             rgb_size[2] = 5;
             break;
         default:
             rgb_size[0] = 8;
             rgb_size[1] = 8;
             rgb_size[2] = 8;
             break;
     }

	// Surfaceつくる
	format.BitsPerPixel = 32;
	format.BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
	format.Rmask = 0x0000ff00; // R
	format.Gmask = 0x00ff0000; // G
	format.Bmask = 0x000000ff; // B
	format.Amask = 0xff000000; // A
#else
	format.Rmask = 0x00ff0000; // R
	format.Gmask = 0x0000ff00; // G
	format.Bmask = 0xff000000; // B
	format.Amask = 0x000000ff; // A
#endif
	format.Rshift = 8;
	format.Gshift = 16;
	format.Bshift = 0;
	format.Ashift = 24;
	format.palette = NULL;
	if((pixvram == NULL) &&(w != 0) &&(h != 0)) {
		pixvram = AG_SurfaceNew(AG_SURFACE_PACKED, (Uint)w, (Uint)h , &format, AG_SRCCOLORKEY);
	}
	if(VramSem == NULL) {
		VramSem = SDL_CreateSemaphore(1);
		if(VramSem) SDL_SemPost(VramSem);
	}
	return;
}

void Detach_AG_GL()
{
	if(VramSem != NULL) {
		SDL_SemWait(VramSem);
		SDL_DestroySemaphore(VramSem);
		VramSem = NULL;
	}
}

extern "C" {
void LockVram(void)
{
	if(VramSem != NULL) SDL_SemWait(VramSem);
}

void UnLockVram(void)
{
	if(VramSem != NULL) SDL_SemPost(VramSem);
}
}

static GLuint CreateTexture(AG_Surface *p)
{
	GLuint textureid;

	if(agDriverOps == NULL) return 0;
	if(p == NULL) return 0;
	LockVram();
    glGenTextures(1, &textureid);
    glBindTexture(GL_TEXTURE_2D, textureid);
    if(!bSmoosing) {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } else {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 pixvram->w, pixvram->h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixvram->pixels);
    UnLockVram();
	return textureid;
}



static void DiscardTextures(int n, GLuint *id)
{
	if(DrawArea == NULL) return;
	if(agDriverOps == NULL) return;
	glDeleteTextures(n, id);

}

static void DiscardTexture(GLuint tid)
{
	DiscardTextures(1, &tid);
}

static void SetViewPort(int x, int y, int w, int h, int ow, int oh)
{
	viewport_x = x;
	viewport_y = y;
	viewport_h = h;
	viewport_w = w;
	osd_w = ow;
	osd_h = oh;
}


void SetVramReader_AG_GL(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
	getvram = p;
	vramwidth = w;
	vramheight = h;
}

AG_Surface *GetVramSurface_AG_GL()
{
	return pixvram;
}

void SetDrawArea_AG_GL(AG_Widget *p, int x, int y, int w, int h)
{
}

void Flip_AG_GL(void)
{
	if(!InitVideo) return;
//	SDL_GL_SwapBuffers();
}


// Create GL Handler(Main)
void PutVram_AG_GL(AG_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	int xx, yy;
	int hh, ww;
	int addr;
	int size;
	int ofset;
	Uint32 c[8];
	Uint8 *bitmap;
	Uint8 *disp;
	AG_Driver *drv;
	float xratio;
	float yratio;

//	if(!InitVideo) return;
	if(DrawArea == NULL) return;
	// Test
	if(agDriverOps == NULL) return;
	if(agDriverSw) {
		drv = &agDriverSw->_inherit;
	} else {
		drv = AGWIDGET(DrawArea)->drv;
	}
	if(drv == NULL) return;
	if((vramwidth <= 0) || (vramheight <=0)) return;
	LockVram();

	size = vramwidth * vramheight * 8 * 4;
	if((pixvram == NULL) &&(vramwidth != 0) &&(vramheight != 0)) {
		pixvram = AG_SurfaceNew(AG_SURFACE_PACKED, (Uint)vramwidth * 8, (Uint)vramheight , &format, AG_SRCCOLORKEY);
	}
	if((pixvram->w != ((Uint)vramwidth * 8)) || (pixvram->h != (Uint)vramheight)){
		AG_SurfaceFree(pixvram);
		pixvram = AG_SurfaceNew(AG_SURFACE_PACKED, (Uint)vramwidth * 8, (Uint)vramheight , &format, AG_SRCCOLORKEY);
	}

//	glClearColor(0, 0, 0, 0);
	ww = (w>>3) + (x>>3);
	hh = h + y;
	bitmap = (Uint8 *)pixvram->pixels;
	if(bitmap == NULL) {
		UnLockVram();
		return;
	}
	ofset = 0;
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * vramwidth + xx ;
			getvram(addr, c, mpage);
			disp = bitmap + pixvram->pitch * yy + xx * 8 * pixvram->format->BytesPerPixel;
			putword((Uint32 *)disp,  c);
			addr++;
			}
	}
	UnLockVram();
	xratio = (float)pixvram->w / (float)AGWIDGET(DrawArea)->w;
	yratio =  (float)pixvram->h / (float)AGWIDGET(DrawArea)->h ;
}

/*
 * Event Functins
 */

void AGEventOverlayGL(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();
}


void AGEventScaleGL(AG_Event *event)
{
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram ;

	pixvram = GetVramSurface_AG_GL();
	if(pixvram == NULL) return;
}

void AGEventDrawGL(AG_Event *event)
{
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	float xbegin;
	float xend;
	float ybegin;
	float yend;
	float aspect;

	if(pixvram == NULL) return;
	aspect =  (float)nDrawHeight / (float)nDrawWidth;
	/*
	 * 開始座標系はVRAMに合わせる。但し、Paddingする
	 */
	xbegin = (float)0.0;
	xend = (float) pixvram->w - 2.0;
	ybegin = (float)0.0;
	yend = (float) pixvram->h - 10.0;

	textureid = CreateTexture(pixvram);
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    /* This allows alpha blending of 2D textures with the scene */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

//    glViewport(0, viewport_y, viewport_w,  viewport_h + viewport_y);
    glViewport(0, 0, nDrawWidth, nDrawHeight);

    /*
     * 座標系はディスプレイに合わせる
     */
    glOrtho(0.0, pixvram->w ,
   		pixvram->h, 0.0,
    		0.0,  1.0);

    glBindTexture(GL_TEXTURE_2D, textureid);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(xbegin, ybegin, -1.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(xbegin, yend, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(xend, ybegin, -1.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(xend, yend, -1.0);
    glEnd();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
	DiscardTexture(textureid);
}

void AGEventMouseMove_AG_GL(AG_Event *event)
{
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int xrel = AG_INT(3);
	int yrel = AG_INT(4);
	int buttons = AG_INT(5);
		//int x, int y, int xRel, int yRel, int buttons
	printf("Mouse-Motion (%d,%d) (%d,%d) %08x\n", x, y, yrel, yrel, buttons);
}

void AGEventKeyPress_AG_GL(AG_Event *event)
{
	//int key, int mod, Ulong unicode
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	int key = AG_INT(1);
	int mod = AG_INT(2);
	int unicode = AG_INT(3);
	printf("Key Press %d %04x %08x\n", key, mod, unicode);
}

void AGEventKeyRelease_AG_GL(AG_Event *event)
{
	//int key, int mod, Ulong unicode
	AG_GLView *wid = (AG_GLView *)AG_SELF();
	int key = AG_INT(1);
	int mod = AG_INT(2);
	int unicode = AG_INT(3);
	printf("Key Release %d %04x %08x\n", key, mod, unicode);
}


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
static BOOL InitVideo = FALSE;
static SDL_semaphore *VramSem;

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

	flags = SDL_OPENGL | SDL_RESIZABLE;
#if 1
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
#endif

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
    InitVideo = TRUE;
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
	Uint8 *pix;
	int w;
	int h;
	GLuint textureid;

	if(agDriverOps == NULL) return 0;
	if(p == NULL) return 0;
	w = p->w;
	h = p->h;
	pix = (Uint8 *)p->pixels;
	if(pix == NULL) return 0;
	LockVram();
    glGenTextures(1, &textureid);
    glBindTexture(GL_TEXTURE_2D, textureid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pix);
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

static void Enter2DMode(void)
{
	int x = viewport_x;
	int y = viewport_y;
	int w = viewport_w + osd_w;
	int h = viewport_h + osd_h;

	        /* Note, there may be other things you need to change,
	           depending on how you have your OpenGL state set up.
	        */
	        glPushAttrib(GL_ENABLE_BIT);
	        glDisable(GL_DEPTH_TEST);
	        glDisable(GL_CULL_FACE);
	        glEnable(GL_TEXTURE_2D);
	        glPushMatrix();
	        /* This allows alpha blending of 2D textures with the scene */
	        glEnable(GL_BLEND);
	        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	        glMatrixMode(GL_PROJECTION);
	        glPushMatrix();
	        glLoadIdentity();
	        /*
	         * ビューポートは表示する画面の大きさ
	         */
	        glViewport(x, y , w + x,  h + y );
	        /*
	         * 座標系は(0,0)-(0,1)
	         */
	        glOrtho(0.0, 1.0 ,
	        		1.0, 0.0,
	        		0.0,  1.0);
	//        glOrtho(0.0, (GLdouble)w, (GLdouble)h, 0.0, 0.0, 2.0);
	        glMatrixMode(GL_MODELVIEW);
	        glPushMatrix();
	        glLoadIdentity();
	        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

static void Leave2DMode()
{
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
}



static void DrawTexture(GLuint tid)
{
	float xbegin = 0.0f;
	float xend = 1.0f;
	float ybegin = 0.0f;
	float yend = 1.0f;

#if 1
	if(viewport_w != 0) {
		xbegin = (float)offset_x / (float)viewport_w;
		xend = 0.5;
	} else {
		xbegin = 0.0;
		xend = 1.0;
	}
	if(viewport_h != 0) {
		ybegin = (float)offset_y  / ((float)viewport_h + (float)osd_h );
		yend = (float)viewport_h  / ((float)viewport_h + (float)osd_h );
	} else {
		xbegin = 0.0;
		xend = 1.0;
	}
#endif

	Enter2DMode();
    glBindTexture(GL_TEXTURE_2D, tid);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(xbegin, ybegin, -1.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(xbegin, yend, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(xend, ybegin, -1.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(xend, yend, -1.0);
 //   glViewport(0, 0 , viewport_w, viewport_h);
    glEnd();
    Leave2DMode();
	DiscardTexture(tid);
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
	if(bitmap == NULL) return;
	ofset = 0;
	LockVram();
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
//	pixvram = GetVramSurface();
	if(pixvram == NULL) return;
	SetViewPort(wid->wid.x, wid->wid.y, nDrawWidth, nDrawHeight, nDrawWidth, OSD_HEIGHT);
	SetOffset(0, DRAW_OFSET);
	textureid = CreateTexture(pixvram);
	DrawTexture(textureid);
	DiscardTexture(textureid);
}


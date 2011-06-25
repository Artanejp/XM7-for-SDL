/* agar_gldraw.cpp
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
#include "agar_draw.h"
#include "agar_gldraw.h"

extern "C" {
    AG_GLView *GLDrawArea;
    extern void LockVram(void);
    extern void UnlockVram(void);
}
static AG_Surface *pixvram;
static Uint32 vramwidth;
static Uint32 vramheight;
static GLuint textureid;
static GLuint blanktextureid;
static AG_PixelFormat format;
static void (*getvram)(Uint32, Uint32 *, Uint32);
static BOOL InitVideo;
static SDL_semaphore *VramSem;

extern void DrawOSDGL(AG_GLView *w);
void DiscardTexture(GLuint tid);


/*
 * OSD
 */
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



static void InitBlankLine()
{

   Uint32 blank = 0x00000000;
   glGenTextures(1, &blanktextureid);
   glBindTexture(GL_TEXTURE_2D, blanktextureid);
   glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 1, 1,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 &blank);

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
        InitBlankLine();
	return;
}

void Detach_AG_GL()
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
	DiscardTexture(textureid);
//        DiscardTexture(blanktextureid);
}


static GLuint CreateTexture(AG_Surface *p)
{
	GLuint tid;


	if(agDriverOps == NULL) return 0;
	if(p == NULL) return 0;

   LockVram();

   glGenTextures(1, &tid);
    glBindTexture(GL_TEXTURE_2D, tid);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 pixvram->w, pixvram->h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixvram->pixels);
   UnlockVram();
   return tid;
}



static void DiscardTextures(int n, GLuint *id)
{
	if(GLDrawArea == NULL) return;
	if(agDriverOps == NULL) return;
	glDeleteTextures(n, id);

}

void DiscardTexture(GLuint tid)
{
	DiscardTextures(1, &tid);
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

//void SetDrawArea_AG_GL(AG_Widget *p, int x, int y, int w, int h)
//{
//}

void Flip_AG_GL(void)
{
	if(!InitVideo) return;
//	SDL_GL_SwapBuffers();
}


// Create GL Handler(Main)
void PutVram_AG_GL(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	int xx, yy;
	int hh, ww;
	int addr;
	int ofset;
	int size;
	Uint32 c[8];
	Uint8 *bitmap;
	Uint8 *disp;
	AG_Driver *drv;
    BOOL newFlag = FALSE;


	if(GLDrawArea == NULL) return;
	// Test
	if(agDriverOps == NULL) return;
	if(agDriverSw) {
		drv = &agDriverSw->_inherit;
	} else {
		drv = AGWIDGET(GLDrawArea)->drv;
	}
	if(drv == NULL) return;
	if((vramwidth <= 0) || (vramheight <=0)) return;
	LockVram();

	size = vramwidth * vramheight * 8 * 4;
	if((pixvram == NULL) &&(vramwidth != 0) &&(vramheight != 0)) {
		pixvram = AG_SurfaceNew(AG_SURFACE_PACKED, (Uint)vramwidth * 8, (Uint)vramheight , &format, AG_SRCCOLORKEY);
	        newFlag = TRUE;
	}
	if((pixvram->w != ((Uint)vramwidth * 8)) || (pixvram->h != (Uint)vramheight)){
		AG_SurfaceFree(pixvram);
		pixvram = AG_SurfaceNew(AG_SURFACE_PACKED, (Uint)vramwidth * 8, (Uint)vramheight , &format, AG_SRCCOLORKEY);
	        newFlag = TRUE;
	}
	ww = (w>>3) + (x>>3);
	hh = h + y;
	bitmap = (Uint8 *)pixvram->pixels;
	if(bitmap == NULL) {
		UnlockVram();
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
	   DiscardTexture(textureid);
	   textureid = CreateTexture(pixvram);


	UnlockVram();
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
	AG_GLView *glv = (AG_GLView *)AG_SELF();

	glViewport(glv->wid.rView.x1, glv->wid.rView.y1 , glv->wid.rView.w, glv->wid.rView.h );
    glLoadIdentity();
    glOrtho(-1.0, 1.0,	1.0, -1.0, -1.0,  1.0);

}

void AGEventDrawGL(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram ;
	int h;
	int i;
	float width;
	float ybegin;
	float yend;


	pixvram = GetVramSurface_AG_GL();

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_TEXTURE_2D);
//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*
     * VRAMの表示:テクスチャ貼った四角形
     */
    glBindTexture(GL_TEXTURE_2D, textureid);
    if(!bSmoosing) {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } else {
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glBegin(GL_POLYGON);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -0.99);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -0.99);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -0.99);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -0.99);
    glEnd();

	if(pixvram){
		h = pixvram->h;
	} else {
		h = 200;
	}
#if 1
    if(!bFullScan) {
       glBindTexture(GL_TEXTURE_2D, blanktextureid);

    	width = 1.0f;
        glLineWidth(width);
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
    	for(i = 0; i < (h - 1); i++) {
    		ybegin = ((float)i  + 1.0f) * 2.0f / (float)h - 1.0f;
	        glVertex3f(-1.0f, ybegin, -0.98);
    		glVertex3f(1.0f , ybegin, -0.98);
    	}
        glEnd();
    }
#endif
    DrawOSDGL(glv);

//    glMatrixMode(GL_PROJECTION);
//    glPopMatrix();
    glPopAttrib();
}

void AGEventMouseMove_AG_GL(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();
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
	AG_GLView *glv = (AG_GLView *)AG_SELF();
	int key = AG_INT(1);
	int mod = AG_INT(2);
	int unicode = AG_INT(3);
	printf("Key Press %d %04x %08x\n", key, mod, unicode);
}

void AGEventKeyRelease_AG_GL(AG_Event *event)
{
	//int key, int mod, Ulong unicode
	AG_GLView *glv = (AG_GLView *)AG_SELF();
	int key = AG_INT(1);
	int mod = AG_INT(2);
	int unicode = AG_INT(3);
	printf("Key Release %d %04x %08x\n", key, mod, unicode);
}


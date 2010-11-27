/*
 * EmuAgarGL.cpp
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 *      EmuUtil: Agar version w/OpenGL
 *      see also : http://
 */

#include <SDL.h>
#include "EmuAgarGL.h"


EmuAgarGL::EmuAgarGL() {
	// TODO Auto-generated constructor stub
	minX = 0.0f;
	minY = 0.0f;
	maxX = 1.0f;
	maxY = 1.0f;
	InitVideo = FALSE;
    ScanLine = FALSE;
    ScanLineWidth = 4.0f;
    UseTexture = FALSE;
    video = NULL;
    pixvram = NULL;
    drawSem = SDL_CreateSemaphore(1);
    SDL_SemPost(drawSem);
}

EmuAgarGL::~EmuAgarGL() {
	// TODO Auto-generated destructor stub
	if(video != NULL) 		AG_SurfaceFree(video);

	if(drawSem != NULL) {
		SDL_DestroySemaphore(drawSem);
	}
}

void EmuAgarGL::InitUI(char *name, Uint Flags)
{
	InitUI(name, Flags);
}

void EmuAgarGL::InitUI(char *name)
{
	InitUI(name, AG_VERBOSE | AG_CREATE_DATADIR | AG_NO_CFG_AUTOLOAD);
}

void EmuAgarGL::SetDrawArea(AG_GLView *p, int x, int y, int w, int h)
{
	drawarea = p;
	SetDrawArea(x, y, w, h);
	SetViewPort(x, y, w, h);
}

void EmuAgarGL::SetViewPort(void)
{
	SetViewPort(0, 0, vramwidth * 8, vramheight);
}

void EmuAgarGL::SetViewPort(AG_Widget *wid)
{
	SetViewPort( wid->x, wid->y, wid->w, wid->h);
}

void EmuAgarGL::SetViewPort(int x, int y, int w, int h)
{
	viewport_x = x;
	viewport_y = y;
	viewport_h = h;
	viewport_w = w;
//	minX = 0.0f;
//	minY = 0.0f;
	minX = (float) x /  (float)viewport_w;
	minY = (float) y /  (float)viewport_h;
	maxX =  ((float)vramwidth * 8.0f) / (float)viewport_w ;
	maxY = (float)vramheight / (float)viewport_h;
#if 0
	printf("VIEWPORT %d,%d %d,%d %f,%f - %f,%f\n",
			viewport_x, viewport_y, viewport_w, viewport_h,
			minX, minY, maxX, maxY);
#endif
}

/*
 * 20101128追加:右下に表示部分を設定する場合
 */
void EmuAgarGL::SetViewPort(int x, int y, int w, int h, int osd_w, int osd_h)
{
	viewport_x = x;
	viewport_y = y;
	viewport_h = h;
	viewport_w = w;
	minX = (float) 0 /  ((float)viewport_w + (float)osd_w);
	minY = (float) 0 /  ((float)viewport_h + (float)osd_h);
	maxX =  ((float)vramwidth * 8.0f) / ((float)viewport_w + (float)osd_w) ;
	maxY = (float)vramheight / ((float)viewport_h + (float)osd_h);
#if 0
	printf("VIEWPORT %d,%d %d,%d %f,%f - %f,%f\n",
			viewport_x, viewport_y, viewport_w, viewport_h,
			minX, minY, maxX, maxY);
#endif
}

void EmuAgarGL::SetDrawArea(int x, int y, int w, int h)
{
//	SDL_SemWait(drawSem);

//	AG_WidgetMapSurface(drawarea, video);
//	SDL_SemPost(drawSem);
}


void EmuAgarGL::InitGL(int w, int h)
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

	return;
}

void EmuAgarGL::CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint32 ds;

	if(palette == NULL) return;

#if AG_LITTLE_ENDIAN
	ds =r + (g << 8)+ (b << 16) + (a<<24);
#else
	ds = r<<24 + g<<16 + b<<8 + 255<<0;
#endif
//			(a << 0);
	palette[src] = ds;
}

/*
 * パレットテーブルを設定する
 */
void EmuAgarGL::SetPaletteTable(Uint32 *p)
{
	palette = p;
}

void EmuAgarGL::Flip(void)
{
	if(!InitVideo) return;
//	SDL_GL_SwapBuffers();
}


static inline void putdot(Uint8 *addr, Uint32 c)
{
	Uint32 *addr32 = (Uint32 *)addr;
	*addr32 = c;
}

static inline void SetByte(Uint8 *addr, Uint32 *c)
{
	putdot(&addr[0], c[7]);
	putdot(&addr[4], c[6]);
	putdot(&addr[8], c[5]);
	putdot(&addr[12], c[4]);
	putdot(&addr[16], c[3]);
	putdot(&addr[20], c[2]);
	putdot(&addr[24], c[1]);
	putdot(&addr[28], c[0]);
}

void EmuAgarGL::PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage)
{
	PutVram((AG_Surface *)p, x, y, w, h, mpage);
}

void EmuAgarGL::PutVram(AG_Surface *p, int x, int y, int w, int h, Uint32 mpage)
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

	if(!InitVideo) return;
	if(drawarea == NULL) return;
	if(putword == NULL) return;
	// Test
	if(agDriverOps == NULL) return;
//	drv = AGWIDGET(drawarea)->drv;
	if(agDriverSw) {
		drv = &agDriverSw->_inherit;
	} else {
		drv = AGWIDGET(drawarea)->drv;
	}
	if(drv == NULL) return;
	//AG_ObjectLock(agDriverOps);
#if 0
	surface = SDL_GetVideoSurface();
	printf("Video w: %d h:%d FLAGS:%02x\n", surface->w, surface->h, surface->flags);
#endif
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
	ofset = 0;
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * vramwidth + xx ;
			getvram(addr, c, mpage);
			disp = bitmap + vramwidth * yy * 32+ xx * 32;
			putword((Uint32 *)disp, 32, c);
			addr++;
			}
	}
#if 0
	printf("Transfer: %08x bytes \n", ofset);
#endif
	xratio = (float)pixvram->w / (float)AGWIDGET(drawarea)->w;
	yratio =  (float)pixvram->h / (float)AGWIDGET(drawarea)->h ;
#if 0
	printf("Draw: %08x bytes TID=%08x\n", ofset, textureid);
#endif
//	AG_ObjectUnlock(drawarea);
//	AG_ObjectUnlock(agDriverOps);
}

GLuint EmuAgarGL::CreateTexture(AG_Surface *p)
{
	Uint8 *pix;
	int w;
	int h;

	if(agDriverOps == NULL) return 0;
	if(p == NULL) return 0;
	w = p->w;
	h = p->h;
	pix = (Uint8 *)p->pixels;

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

	return textureid;
}

void EmuAgarGL::DiscardTexture()
{
	DiscardTexture(textureid);
}

void EmuAgarGL::DiscardTexture(GLuint tid)
{
	DiscardTextures(1, &tid);
}

void EmuAgarGL::DiscardTextures(int n, GLuint *id)
{
	if(drawarea == NULL) return;
	if(agDriverOps == NULL) return;
	glDeleteTextures(n, id);

}

void EmuAgarGL::Enter2DMode(void)
{
	int x = viewport_x;
	int y = viewport_y;
	int w = viewport_w;
	int h = viewport_h;

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

void EmuAgarGL::Leave2DMode()
{
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
}


void EmuAgarGL::DrawTexture()
{
	DrawTexture(textureid);
}

void EmuAgarGL::DrawTexture(GLuint tid)
{
	float xbegin = 0.0f;
	float xend = 1.0f;
	float ybegin = 0.0f;
	float yend = 1.0f;

#if 1
	if(viewport_w != 0) {
		xbegin = (float)offset_x / (float)viewport_w;
		xend = 1.0;
//		viewport_x = -offset_x;
//		viewport_w += offset_x;
	} else {
		xbegin = 0.0;
		xend = 1.0;
	}
	if(viewport_h != 0) {
		ybegin = (float)offset_y / (float)viewport_h;
		yend = 1.0;
//		viewport_y = -offset_y;
//		viewport_h += offset_y;
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
	DiscardTexture();
}

void EmuAgarGL::SetTextureID(GLuint id)
{
	textureid = id;
}

GLuint EmuAgarGL::GetTextureID(void)
{
	return textureid;
}

AG_Surface *EmuAgarGL::GetVramSurface(void)
{
	return pixvram;
}

void EmuAgarGL::SetOffset(int x, int y)
{
	offset_x = x;
	offset_y = y;
}

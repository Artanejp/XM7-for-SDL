/*
 * EmuAgarGL.cpp
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 *      EmuUtil: Agar version w/OpenGL
 *      see also : http://
 */

#include "EmuAgarGL.h"


EmuAgarGL::EmuAgarGL() {
	// TODO Auto-generated constructor stub
	minX = 0.0f;
	minY = 0.0f;
	maxX = 1.0f;
	maxY = 1.0f;
	InitVideo = TRUE;
    ScanLine = FALSE;
    ScanLineWidth = 4.0f;
    UseTexture = FALSE;
    video = NULL;
}

EmuAgarGL::~EmuAgarGL() {
	// TODO Auto-generated destructor stub
	if(video != NULL) AG_SurfaceFree(video);
}

void EmuAgarGL::InitUI(char *name, Uint Flags)
{
	InitUI(name, Flags);
}

void EmuAgarGL::InitUI(char *name)
{
	InitUI(name, AG_VERBOSE | AG_CREATE_DATADIR | AG_NO_CFG_AUTOLOAD);
}

void EmuAgarGL::SetDrawArea(AG_Window *p, int x, int y, int w, int h)
{
	if(drawarea == NULL) {
		drawarea = AG_BoxNew(p, AG_BOX_HORIZ,0);
	}
	if(drawarea != NULL){
		AG_WidgetSetPosition(&(drawarea->wid), x, y);
		AG_WidgetSetSize(&(drawarea->wid), w, h);
	}
}

void EmuAgarGL::InitGL(int w, int h)
{
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
	video = AG_SurfaceNew(AG_SURFACE_PACKED, (Uint)w, (Uint)h , &format, AG_SRCCOLORKEY);
}

void EmuAgarGL::CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint32 ds;

	if(palette == NULL) return;

#if AG_LITTLE_ENDIAN
	ds =r<<8 + g<<16 + b + a<<24;
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
	SDL_GL_SwapBuffers();
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
	int xx, yy;
	int hh, ww;
	int addr;
	int size;
	int ofset;
	int i;
	AG_Surface *pp =(AG_Surface *)p;
	Uint32 c[8];
	Uint8 *bitmap;
	Uint8 *disp;
	AG_Driver *drv;


	if(!InitVideo) return;
	if(drawarea == NULL) return;
	if(putword == NULL) return;
	// Test
	if(agDriverOps == NULL) return;
	drv = AG_DriverOpen(agDriverOps);
	if(drv == NULL) return;
#if 0
	surface = SDL_GetVideoSurface();
	printf("Video w: %d h:%d FLAGS:%02x\n", surface->w, surface->h, surface->flags);
#endif
	size = vramwidth * vramheight * 8 * 4;
	glClearColor(0, 0, 0, 0);
	ww = w >>3;
	hh = h + y;

	if(video == NULL) {
		AG_DriverClose(drv);
		return;
	}
	bitmap = (Uint8 *)video->pixels;
	ofset = 0;
	for(yy = y; yy < hh; yy++) {
		for(xx = 0; xx < ww; xx++) {
			addr = yy  * vramwidth + xx + x;
			ofset = yy * vramwidth * 32 + xx * 32;
			getvram(addr, c, mpage);
			disp = &bitmap[ofset];
			putword((Uint32 *)disp, 32, c);
			ofset+=32;
			addr++;
			}
	}
#if 0
	printf("Transfer: %08x bytes \n", ofset);
#endif
	agDriverOps->blitSurfaceGL(drv, &(drawarea->wid), video, 1.0, 1.0);

//   if(ScanLine) {
//	   DrawScanLine();
//   }

#if 0
	printf("Draw: %08x bytes TID=%08x\n", ofset, textureid);
#endif
	AG_DriverClose(drv);

}

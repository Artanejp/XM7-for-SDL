/*
 * agar_gldraw2.cpp
 * Using Indexed palette @8Colors.
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
//#include <agar/gui/opengl.h>

#include <SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include "api_draw.h"
#include "api_scaler.h"
#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"


//extern "C" {
//    AG_GLView *GLDrawArea;
//}


struct VirtualVram {
    Uint32 pVram[320][200];
};

static Uint32 *pVram2;
static struct VirtualVram *pVirtualVram;
static GLuint textureid;
static GLuint blanktextureid;
static BOOL InitVideo;
static SDL_sem *pVideoSem;
static void (*pGetVram)(Uint32, Uint32 *, Uint32);
static Uint8 *vram_pb;
static Uint8 *vram_pr;
static Uint8 *vram_pg;

void SetVramReader_GL2(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
    pGetVram = p;
}



static void Lock(void)
{
    if(pVideoSem == NULL) return;
    SDL_SemWait(pVideoSem);
}

static void Unlock(void)
{
    if(pVideoSem == NULL) return;
    SDL_SemPost(pVideoSem);
}


void SetVram_200l(Uint8 *p)
{
    vram_pb = p + 0;
    vram_pg = p + 0x10000;
    vram_pr = p + 0x8000;
}


static void InitVirtualVram()
{
    if(pVirtualVram != NULL) return;
    pVirtualVram = (struct VirtualVram *)malloc(640*400*sizeof(Uint8) + 256 * sizeof(Uint32));
    if(pVirtualVram == NULL) return;
    pVram2 = (Uint32 *)malloc(640*400*sizeof(Uint32));
    if(pVram2 == NULL) {
        free(pVirtualVram);
        pVirtualVram = NULL;
    }

   // Phase 1
    memset(pVirtualVram, 0x00, sizeof(struct VirtualVram) );
    memset(pVram2, 0x00, sizeof(640*400*sizeof(Uint32)));
    // Phase 2
}

static void InitBlankLine()
{
   Uint32 blank = 0x00000000;
   if(blanktextureid == 0) {
        glGenTextures(1, &blanktextureid);
   }
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

static void DetachBlankLine(void)
{
    if(blanktextureid == 0) return;
   	glDeleteTextures(1, &blanktextureid);
   	blanktextureid = 0;
}

static void DetachTexture(void)
{
    if(textureid == 0) return;
   	glDeleteTextures(1, &textureid);
   	textureid = 0;
   	return;
}

static void DetachVirtualVram(void)
{
    if(pVirtualVram == NULL) {
        if(pVram2 != NULL) free(pVram2);
        pVram2 = NULL;
        return;
    }
    free((void *)pVirtualVram);
    pVirtualVram = NULL;
    if(pVram2 == NULL) return;
    free(pVram2);
    pVram2 = NULL;
}

void DetachGL_AG2(void)
{
    DetachTexture();
    DetachBlankLine();
    DetachVirtualVram();
    if(pVideoSem != NULL) {
    SDL_DestroySemaphore(pVideoSem);
    pVideoSem = NULL;
    }
    pGetVram = NULL;
}


void InitGL_AG2(int w, int h)
{
	Uint32 flags;
	int bpp = 32;
	int rgb_size[3];

	if(InitVideo) return;
    InitVideo = TRUE;

    vram_pb = NULL;
    vram_pg = NULL;
    vram_pr = NULL;

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

	if(pVideoSem == NULL) {
		pVideoSem = SDL_CreateSemaphore(1);
		if(pVideoSem) SDL_SemPost(pVideoSem);
	}
	textureid = 0;
	blanktextureid = 0;
	pVirtualVram = NULL;
	pGetVram = NULL;

	InitVirtualVram();
    InitBlankLine();
	return;
}



 void CalcPalette_8colors(int index, Uint8 R, Uint8 G, Uint8 B, Uint8 A)
 {
     if((index > 10) || (index < 0)) return;
     Lock();
 #ifdef AG_LITTLE_ENDIAN
     rgbTTLGDI[index] = A << 24 + R << 0 + G << 8 + B << 16;
 #else
     rgbTTLGDI[index] = A << 0 + R << 24 + G << 16 + B << 8;
 #endif
     Unlock();
 }

 void CalcPalette_4096Colors(int index, Uint8 R, Uint8 G, Uint8 B, Uint8 A)
 {
     if((index > 4095) || (index < 0)) return;
     Lock();
 #ifdef AG_LITTLE_ENDIAN
     rgbAnalogGDI[index] = A << 24 + R << 0 + G << 8 + B << 16;
 #else
     rgbAnalogGDI[index] = A << 0 + R << 24 + G << 16 + B << 8;
 #endif
    Unlock();
 }

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

static inline void putword8(Uint8 *disp, Uint8 *cbuf)
{
    disp[0] = cbuf[7];
    disp[1] = cbuf[6];
    disp[2] = cbuf[5];
    disp[3] = cbuf[4];
    disp[4] = cbuf[3];
    disp[5] = cbuf[2];
    disp[6] = cbuf[1];
    disp[7] = cbuf[0];
}


static inline void getvram_8to8(Uint32 addr, Uint8 *cbuf)
{
       Uint8    cb,
                cr,
                cg;
        cb = vram_pb[addr];
        cr = vram_pr[addr];
        cg = vram_pg[addr];
        cbuf[0] =   (cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2);
        cbuf[1] =   ((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1);
        cbuf[2] =   ((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04);
        cbuf[3] =   ((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +((cg & 0x08) >> 1);
        cbuf[4] =   ((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) + ((cg & 0x10) >> 2);
        cbuf[5] =   ((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) + ((cg & 0x20) >> 3);
        cbuf[6] =   ((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) + ((cg & 0x40) >> 4);
        cbuf[7] =   ((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) + ((cg & 0x80) >> 5);
}

void getvram_400l(Uint32 addr,Uint32 *p, Uint32 mpage)
{
  getvram_8to8(addr, (Uint8 *)p);
}

void getvram_200l(Uint32 addr,Uint32 *p, Uint32 mpage)
{
  getvram_8to8(addr, (Uint8 *)p);
}

static void DiscardTexture(GLuint tid)
{
	glDeleteTextures(1, &tid);
}


static void copy8to32(int w, int h)
{
    int size = w * h;
    int i;
    Uint8 *p;
    Uint8 t;

    p = (Uint8 *)(&pVirtualVram->pVram);
    for(i = 0; i < size; i+=8 ) {
        pVram2[i] =(Uint32) rgbTTLGDI[*p];
        pVram2[i + 1] =(Uint32) rgbTTLGDI[*(p + 1)];
        pVram2[i + 2] =(Uint32) rgbTTLGDI[*(p + 2)];
        pVram2[i + 3] =(Uint32) rgbTTLGDI[*(p + 3)];
        pVram2[i + 4] =(Uint32) rgbTTLGDI[*(p + 4)];
        pVram2[i + 5] =(Uint32) rgbTTLGDI[*(p + 5)];
        pVram2[i + 6] =(Uint32) rgbTTLGDI[*(p + 6)];
        pVram2[i + 7] =(Uint32) rgbTTLGDI[*(p + 7)];
        p+=8;
    }
}
/*
 * TextureをUpdateモードで作成する(8色, 16色パレット付き)
 */
static GLuint UpdateTexture8(GLuint tid, int w, int h)
{
    GLuint ttid;
    int size;

    if((w < 0) || (h < 0)) return 0;
    if(pVirtualVram == NULL) return 0;
    if(&pVirtualVram->pVram == NULL) return 0;
    if(pVram2 == NULL) return 0;

    if(tid == 0) {
        glGenTextures(1, &ttid);
    } else {
        ttid = tid;
    }
    glBindTexture(GL_TEXTURE_2D, ttid);
    copy8to32(w, h);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pVram2);
    return ttid;
}

static GLuint CreateVirtualVram8(int x, int y, int w, int h, int mode)
{
	int size;
	int ww, hh;
	int vramwidth, vramheight;
	int xx, yy;
	Uint32 addr;
	Uint8 *bitmap;
	Uint8 *disp;
	Uint8 c[8];

	Lock();

    switch(mode) {
    case SCR_200LINE:
        size = 80 * 200 * 8;
        vramwidth = 640;
        vramheight = 200;
        break;
    case SCR_400LINE:
        size = 80 * 400 * 8;
        vramwidth = 640;
        vramheight = 400;
        break;
    default:
        Unlock();
        return 0;
        break;
    }
	ww = (w>>3) + (x>>3);
	hh = h + y;
	bitmap = (Uint8 *)(&pVirtualVram->pVram);
	if(bitmap == NULL) {
		Unlock();
		return 0;
	}
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * 80 + xx ;
//			c = (Uint8 *)(&pVirtualVram->pVram[addr * 8]);
			getvram_8to8(addr, c);
			disp = bitmap + xx * 8 + 640 * yy;
			putword8((Uint8 *)disp,  c);
			addr++;
			}
	}
   DiscardTexture(textureid);
   textureid = UpdateTexture8(0, vramwidth, vramheight);
   Unlock();
}

static GLuint UpdateTexture32(GLuint tid , int w, int h)
{
    GLuint ttid;
    int size;

    if((w < 0) || (h < 0)) return 0;
    if(pVirtualVram == NULL) return 0;
    if(&pVirtualVram->pVram == NULL) return 0;
    if(tid == 0) {
        glGenTextures(1, &ttid);
    } else {
        ttid = tid;
    }
    glBindTexture(GL_TEXTURE_2D, ttid);
    size = w * h * sizeof(Uint32);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 &pVirtualVram->pVram);
}


static void CreateVirtualVram320(int x, int y, int w, int h, int mode, Uint32 mpage)
{
	int size;
	int ww, hh;
	int vramwidth, vramheight;
	int xx, yy;
	Uint32 addr;
	Uint32 *bitmap;
	Uint32 *disp;
	Uint32 c[8];

    if(pGetVram == NULL) return;
	Lock();
	ww = (w>>3) + (x>>3);
	hh = h + y;
	bitmap = (Uint32 *)(&pVirtualVram->pVram);
	for(yy = y; yy < hh; yy++) {
		for(xx = x>>3 ; xx < ww; xx++) {
			addr = yy  * 40 + xx ;
//			c = (Uint8 *)(&pVirtualVram->pVram[addr * 8]);
			pGetVram(addr, c, mpage);
			disp = &bitmap[xx * 8 + 320 * yy];
			putword(disp,  c);
			addr++;
			}
	}
   DiscardTexture(textureid);
   textureid = UpdateTexture32(0, 320, 200);
   Unlock();
}

 // Create GL Handler(Main)
void PutVram_AG_GL2(SDL_Surface *p, int x, int y, int w, int h,  Uint32 mpage)
{
	int xx, yy;
	int hh, ww;
	int addr;
	int ofset;
	int size;
	Uint32 c[8];
	AG_Driver *drv;
    GLuint tid;

	if(GLDrawArea == NULL) return;
	// Test
	if(agDriverOps == NULL) return;
	if(agDriverSw) {
		drv = &agDriverSw->_inherit;
	} else {
		drv = AGWIDGET(GLDrawArea)->drv;
	}
	if(drv == NULL) return;
	if(AG_UsingGL(drv) == 0) return; // Non-GL
	if((vram_pb == NULL) || (vram_pg == NULL) || (vram_pr == NULL)) return;
	switch (bMode) {
	case SCR_400LINE:
        CreateVirtualVram8(x, y, w, h, bMode);
		break;
	case SCR_262144:
        CreateVirtualVram320(x, y, w, h, bMode, mpage);
		break;
	case SCR_4096:
        CreateVirtualVram320(x, y, w, h, bMode, mpage);
		break;
	case SCR_200LINE:
        CreateVirtualVram8(x, y, w, h, bMode);
		break;
	}
}

/*
 * "Draw"イベントハンドラ
 */
void AGEventDrawGL2(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram ;
	int h;
	int i;
	float width;
	float ybegin;
	float yend;


    glPushAttrib(GL_ALL_ATTRIB_BITS);
//    glMatrixMode(GL_PROJECTION);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    /*
     * VRAMの表示:テクスチャ貼った四角形
     */
     if(textureid != 0) {
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
     }

     switch(bMode) {
        case SCR_400LINE:
            h = 400;
            break;
        case SCR_200LINE:
        case SCR_262144:
        case SCR_4096:
        default:
            h = 200;
            break;
     }
    if((!bFullScan)  && (blanktextureid != 0)){
       glBindTexture(GL_TEXTURE_2D, blanktextureid);
    	width = 1.0f;
        glLineWidth(width);
        glBegin(GL_LINES);
    	for(i = 0; i < (h - 1); i++) {
    		ybegin = ((float)i  + 1.0f) * 2.0f / (float)h - 1.0f;
            glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, ybegin, -0.98);
    		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f , ybegin, -0.98);
    	}
        glEnd();
    }
    glEnable(GL_BLEND);
    DrawOSDGL(glv);
//    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

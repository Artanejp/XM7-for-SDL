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
#include <GL/glx.h>
#include <GL/glxext.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP

#include "api_draw.h"
#include "api_scaler.h"
#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"


GLuint uVramTextureID;

static void (*pGetVram)(Uint32, Uint32 *, Uint32);
static int bModeOld;

void SetVramReader_GL2(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
    pGetVram = p;
}

static void DetachTexture(void)
{
    if(uVramTextureID == 0) return;
   	glDeleteTextures(1, &uVramTextureID);
   	uVramTextureID = 0;
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

static void DetachGridVertexs(void);
void DetachGL_AG2(void)
{
    DetachTexture();
    DetachVirtualVram();
    DetachGridVertexs();
    DetachVramSemaphore();
    pGetVram = NULL;
}

// Grids
static GLfloat *GridVertexs200l;
static GLfloat *GridVertexs400l;
static GLfloat MainTexcoods[4];

// FBO API
PFNGLVERTEXPOINTEREXTPROC glVertexPointerEXT;
PFNGLDRAWARRAYSEXTPROC glDrawArraysEXT;
PFNGLTEXCOORDPOINTEREXTPROC glTexCoordPointerEXT;


static void InitGridVertexsSub(int h, GLfloat *vertex)
{
  int i;
  GLfloat ybegin;
  GLfloat yofset;
  GLfloat yinc;
  GLfloat y;
  int base;

    yinc = 400.0f / (float)h;
    ybegin = yinc / (float)h;
    yofset = -1.0f + 41.0f / 440.0f;
  if(vertex == NULL) return;
  y = ybegin + yofset;
  for(i = 0; i < h; i++, y += ybegin){
      base = i * 6;
      vertex[base] = -1.0f; // x
      vertex[base + 1] = y; // y
      vertex[base + 2] = 0.98f; // z

      vertex[base + 3] = 1.0f; // x
      vertex[base + 4] = y; // y
      vertex[base + 5] = 0.98f; // z

  }
}

static void InitGridVertexs(void)
{
    GridVertexs200l = (GLfloat *)malloc(201 * 6 * sizeof(GLfloat));
    if(GridVertexs200l != NULL) {
        InitGridVertexsSub(200, GridVertexs200l);
    }
    GridVertexs400l = (GLfloat *)malloc(401 * 6 * sizeof(GLfloat));
    if(GridVertexs400l != NULL) {
        InitGridVertexsSub(400, GridVertexs400l);
    }
}

static void DetachGridVertexs(void)
{
    if(GridVertexs200l != NULL) {
        free(GridVertexs200l);
        GridVertexs200l = NULL;
    }
    if(GridVertexs400l != NULL) {
        free(GridVertexs400l);
        GridVertexs400l = NULL;
    }
}

static void InitFBO(void)
{
    glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)glXGetProcAddress((const GLubyte *)"glVertexPointerEXT");
    if(glVertexPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
    glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)glXGetProcAddress((const GLubyte *)"glDrawArraysEXT");
    if(glDrawArraysEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
    glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)glXGetProcAddress((const GLubyte *)"glTexCoordPointerEXT");
    if(glTexCoordPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
}

void InitGL_AG2(int w, int h)
{
	Uint32 flags;
	int bpp = 32;
	int rgb_size[3];
	char *ext;

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
    /*
     * GL 拡張の取得 20110907-
     */
	InitVramSemaphore();
	uVramTextureID = 0;
	pVirtualVram = NULL;
	pGetVram = NULL;
	InitVirtualVram();
    InitGLExtensionVars();
    InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
               // FBOの有無を受けて、拡張の有無変数を変更する（念のために）
    InitGridVertexs(); // Grid初期化
    return;
}


Uint32 *GetVirtualVram(void)
{
    if(pVirtualVram == NULL) return NULL;
    return &(pVirtualVram->pVram[0][0]);
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
	Uint32 *pp;
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

    if(pVirtualVram == NULL) return;
    pp = &(pVirtualVram->pVram[0][0]);

    if(pp == NULL) return;
	if((vram_pb == NULL) || (vram_pg == NULL) || (vram_pr == NULL)) return;

    if(bClearFlag) {
        LockVram();
        memset(pp, 0x00, 640 * 400 * sizeof(Uint32)); // モードが変更されてるので仮想VRAMクリア
        bClearFlag = FALSE;
        UnlockVram();
    }
	switch (bMode) {
	case SCR_400LINE:
        CreateVirtualVram8(pp, x, y, w, h, bMode);
		break;
	case SCR_262144:
        CreateVirtualVram256k(pp, x, y, w, h, bMode, mpage);
		break;
	case SCR_4096:
        CreateVirtualVram4096(pp, x, y, w, h, bMode, mpage);
		break;
	case SCR_200LINE:
        CreateVirtualVram8(pp, x, y, w, h, bMode);
		break;
	}
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

   glViewport(glv->wid.rView.x1, glv->wid.rView.y1, glv->wid.rView.w, glv->wid.rView.h);
    //glLoadIdentity();
    //glOrtho(-1.0, 1.0,	1.0, -1.0, -1.0,  1.0);

}


/*
 * "Draw"イベントハンドラ
 */
void AGEventDrawGL2(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();
	AG_Surface *pixvram ;
	int w;
	int h;
	int i;
	float width;
	float ybegin;
	float yend;
	Uint32 *p;
	Uint32 *pp;
	int x;
	int y;
    GLfloat TexCoords[4][2];
    GLfloat Vertexs[4][3];

   if(pVirtualVram == NULL) return;
   p = &(pVirtualVram->pVram[0][0]);
   if(p == NULL) return;

    TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
    TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin

    TexCoords[2][0] = TexCoords[1][0] = 1.0f; // Xend
    TexCoords[2][1] = TexCoords[3][1] = 1.0f; // Yend

    // OSD を外に追い出す
    ybegin = 400.0f / 440.0f;
    yend = 1.0f;
            // Z Axis
    Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = -0.99f;
    Vertexs[0][0] = Vertexs[3][0] = -1.0f; // Xbegin
    Vertexs[0][1] = Vertexs[1][1] = 1.0f;  // Yend
    Vertexs[2][0] = Vertexs[1][0] = 1.0f; // Xend
    Vertexs[2][1] = Vertexs[3][1] = -ybegin; // Ybegin


     switch(bMode) {
        case SCR_400LINE:
            w = 640;
            h = 400;
            break;
        case SCR_200LINE:
            w = 640;
            h = 200;
            break;
        case SCR_262144:
        case SCR_4096:
        default:
            w = 320;
            h = 200;
            break;
     }
     /*
     * 20110904 OOPS! Updating-Texture must be in Draw-Event-Handler(--;
     */
    LockVram();
    if(SDLDrawFlag.Drawn) {
       Uint32 *pu;
       Uint32 *pq;
       int xx;
       int yy;
       int ww;
       int hh;
       int ofset;

            if(uVramTextureID == 0){
                uVramTextureID = CreateNullTexture(w + 2, h + 2);
            }
//           printf("DBG: Vram Texture Updated %08x\n");
            glPushAttrib(GL_TEXTURE_BIT);
            glBindTexture(GL_TEXTURE_2D, uVramTextureID);
//            pu = p;
//#ifdef _OPENMP
//       #pragma omp parallel for shared(p, SDLDrawFlag) private(pu)
//#endif       
       ww = w >> 3;
       hh = h >> 3;
//       pu = p;
       for(yy = 0; yy < hh; yy++) {
               for(xx = 0; xx < ww; xx++) {
		   pu = &p[(xx + yy * ww) * 64];
                    if(SDLDrawFlag.write[xx][yy]) {
                       UpdateTexturePiece(pu, uVramTextureID, xx << 3, yy << 3, 8, 8);
		       SDLDrawFlag.write[xx][yy] = FALSE;
                    }
//		   pu += 64;
                }
            }
            glPopAttrib();
       //uVramTextureID = UpdateTexture(p, uVramTextureID, w, h);
    }
    SDLDrawFlag.Drawn = FALSE;
    UnlockVram();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
//    glDisable(GL_DEPTH_TEST);
    /*
     * VRAMの表示:テクスチャ貼った四角形
     */
     if(uVramTextureID != 0) {
       	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, uVramTextureID);
        if(!bSmoosing) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
//        LockVram();
        if(bGL_EXT_VERTEX_ARRAY) {
            glTexCoordPointerEXT(2, GL_FLOAT, 0, 4, TexCoords);
            glVertexPointerEXT(3, GL_FLOAT, 0, 4, Vertexs);
            glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
            glEnable(GL_VERTEX_ARRAY_EXT);

            glDrawArraysEXT(GL_POLYGON, 0, 4);
            glDisable(GL_VERTEX_ARRAY_EXT);
            glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
        } else {
            glBegin(GL_POLYGON);
            glTexCoord2f(TexCoords[0][0], TexCoords[0][1]);
            glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);

            glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
            glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);

            glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
            glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);

            glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
            glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
            glEnd();
        }
//        UnlockVram();
     }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    if(!bFullScan){
    	width = 1.0f;
    	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        glLineWidth(width);
        {
           GLfloat *vertex;
           int base;

        switch(bMode) {
        case SCR_400LINE:
            vertex = GridVertexs400l;
            break;
        default:
            vertex = GridVertexs200l;
            break;
        }
        if(vertex == NULL) goto e1;

        if(bGL_EXT_VERTEX_ARRAY) {
                glVertexPointerEXT(3, GL_FLOAT, 0, h * 2, vertex);
                glEnable(GL_VERTEX_ARRAY_EXT);
                glDrawArraysEXT(GL_LINES, 0, h  * 2);
                glDisable(GL_VERTEX_ARRAY_EXT);
        } else {
            GLfloat *v;
            // VERTEX_ARRAYなしの場合
            glBegin(GL_LINES);
            for(i = 0; i < (h - 1); i++) {
                v = &vertex[i * 6];
                glVertex3f(v[0], v[1], v[2]);
                glVertex3f(v[3], v[4], v[5]);
            }
            glEnd();
        }
    }

    }
e1:
    DrawOSDGL(glv);
    glPopAttrib();
}

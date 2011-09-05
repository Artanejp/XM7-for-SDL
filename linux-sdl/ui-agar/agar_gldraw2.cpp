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

void DetachGL_AG2(void)
{
    DetachTexture();
    DetachVirtualVram();
    DetachVramSemaphore();
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

	InitVramSemaphore();
	uVramTextureID = 0;
	pVirtualVram = NULL;
	pGetVram = NULL;
	InitVirtualVram();
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

   if(pVirtualVram == NULL) return;
   p = &(pVirtualVram->pVram[0][0]);
   if(p == NULL) return;

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
       uVramTextureID = UpdateTexture(p, uVramTextureID, w, h);
    }
//    memset(SDLDrawFlag.drawn, 0x00, sizeof(SDLDrawFlag.drawn));
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
        glBegin(GL_POLYGON);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -0.99f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, -0.99f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, -0.99f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -0.99f);
        glEnd();
//        UnlockVram();
     }
//    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    if(!bFullScan){
    	width = 1.0f;
    	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
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
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

/*
 * agar_gldraw2.cpp
 * Using Indexed palette @8Colors.
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
//#include <agar/gui/opengl.h>

#include <SDL/SDL.h>
#ifdef _WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#ifdef _USE_OPENCL
# include "agar_glcl.h"
#endif

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP

#include "api_draw.h"
//#include "api_scaler.h"
#include "api_kbd.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_cfg.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"


GLuint uVramTextureID;
#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
#endif

extern BOOL bInitCL;
extern void InitContextCL(void);
extern void InitGL_AG2(int w, int h);
extern void DetachGL_AG2(void);

Uint32 *pFrameBuffer;
// Grids
extern GLuint GridVertexs200l;
extern GLuint GridVertexs400l;

// Brights
float fBrightR;
float fBrightG;
float fBrightB;
static int bModeOld;

void SetVramReader_GL2(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
//    pGetVram = p;
}

void SetBrightRGB_AG_GL2(float r, float g, float b)
{
   fBrightR = r;
   fBrightG = g;
   fBrightB = b;
}

Uint32 *GetVirtualVram(void)
{
    return pVram2;
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

static void UpdateFramebufferPiece(Uint32 *p, int x, int y)
{
   v4hi *addr1;// 256bit->128bit for older CPUs.
   v4hi *src;
   int ofset;
   int yy;

   if((x < 0) || (x >= 640)) return;
   if((y < 0) || (y >= 400)) return;
//   if(pFrameBuffer == NULL) return;
   ofset = x + y * 640;

   src = (v4hi *)p;
   addr1 = (v4hi *)(&pFrameBuffer[ofset]);
   addr1[0] = src[0];
   addr1[1] = src[1];

   addr1[160] = src[2];
   addr1[161] = src[3];

   addr1[320] = src[4];
   addr1[321] = src[5];

   addr1[480] = src[6];
   addr1[481] = src[7];

   addr1[640] = src[8];
   addr1[641] = src[9];

   addr1[800] = src[10];
   addr1[801] = src[11];

   addr1[960] = src[12];
   addr1[961] = src[13];

   addr1[1120] = src[14];
   addr1[1121] = src[15];
}

static void drawGrids(void *pg,int w, int h)
{
    AG_GLView *glv = (AG_GLView *)pg;

   
}


static void drawUpdateTexture(Uint32 *p, int w, int h)
{
//    LockVram();
   

    if(uVramTextureID != 0){
       Uint32 *pu;
       Uint32 *pq;
       int xx;
       int yy;
       int ww;
       int hh;
       int ofset;


//       glPushAttrib(GL_TEXTURE_BIT);
       ww = w >> 3;
       hh = h >> 3;

#ifdef _USE_OPENCL
       if((cldraw != NULL) && bGL_PIXEL_UNPACK_BUFFER_BINDING) {
	  cl_int ret;
	  LockVram();
	  ret = cldraw->GetVram(bMode);
	  UnlockVram();
          if(ret != CL_SUCCESS) {
	     SDLDrawFlag.Drawn = FALSE;
	     return;
	  }
	  
	  glBindTexture(GL_TEXTURE_2D, uVramTextureID);
	  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, cldraw->GetPbo());
	  // Copy pbo to texture 
	  glTexSubImage2D(GL_TEXTURE_2D, 
			  0,
			  0,
			  0,
			  w,
			  h,
			  GL_RGBA,
			  GL_UNSIGNED_BYTE,
			  NULL);
	  glBindTexture(GL_TEXTURE_2D, 0);
	  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
       } else {
#endif
	  LockVram();
	  if(nRenderMethod == RENDERING_RASTER) {
	     if(p != NULL) {
		glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		UpdateTexturePiece(p, uVramTextureID, 0, 0, 640, h);
		glBindTexture(GL_TEXTURE_2D, 0); // 20111023 チラつきなど抑止
	     }
	  } else if((pFrameBuffer != NULL) && (p != NULL) && (SDLDrawFlag.Drawn) ) {
#ifdef _OPENMP
//            # pragma omp parallel for shared(p, SDLDrawFlag, ww, hh) private(pu, xx)
            #pragma omp parallel for shared(p, SDLDrawFlag, ww, hh) private(pu, xx)
#endif
	       for(yy = 0; yy < hh; yy++) { // 20120411 分割アップデートだとGLドライバによっては遅くなる
		  for(xx = 0; xx < ww; xx++) {
		     if(SDLDrawFlag.write[xx][yy]) {
			pu = &p[(xx + yy * ww) << 6];
			UpdateFramebufferPiece(pu, xx << 3, yy << 3);
			SDLDrawFlag.write[xx][yy] = FALSE;
		     }
		  }
	       }
  	       
	       glBindTexture(GL_TEXTURE_2D, uVramTextureID);
	       UpdateTexturePiece(pFrameBuffer, uVramTextureID, 0, 0, 640, h);
	       glBindTexture(GL_TEXTURE_2D, 0); // 20111023 チラつきなど抑止
	  }
	  UnlockVram();
#ifdef _USE_OPENCL
       }
#endif       
    SDLDrawFlag.Drawn = FALSE;
    }
}

   




/*
 * "Draw"イベントハンドラ
 */

void AGEventDrawGL2(AG_Event *event)
{
   AG_GLView *glv = (AG_GLView *)AG_SELF();
   int w;
   int h;
   int i;
   float width;
   float yf;
   Uint32 *p;
   Uint32 *pp;
   int x;
   int y;
   GLfloat TexCoords[4][2];
   GLfloat Vertexs[4][3];
   GLfloat TexCoords2[4][2];
   GLuint gridtid;

   p = pVram2;
   if(p == NULL) return;

     switch(bMode) {
        case SCR_400LINE:
            w = 640;
            h = 400;
            TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
            TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin

            TexCoords[2][0] = TexCoords[1][0] = 640.0f / 640.0f; // Xend
            TexCoords[2][1] = TexCoords[3][1] = 400.0f / 400.0f; // Yend
	    gridtid = GridVertexs400l;
            break;
        case SCR_200LINE:
            w = 640;
            h = 200;
            TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
            TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin

            TexCoords[2][0] = TexCoords[1][0] = 640.0f / 640.0f; // Xend
            TexCoords[2][1] = TexCoords[3][1] = 199.0f / 400.0f; // Yend
	    gridtid = GridVertexs200l;
            break;
        case SCR_262144:
        case SCR_4096:
        default:
            w = 320;
            h = 200;
            TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
            TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin

            TexCoords[2][0] = TexCoords[1][0] = 320.0f / 640.0f; // Xend
            TexCoords[2][1] = TexCoords[3][1] = 199.0f / 400.0f; // Yend
	    gridtid = GridVertexs200l;
            break;
     }

    Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = -0.98f;
    Vertexs[0][0] = Vertexs[3][0] = -1.0f; // Xbegin
    Vertexs[0][1] = Vertexs[1][1] = 1.0f;  // Yend
    Vertexs[2][0] = Vertexs[1][0] = 1.0f; // Xend
    Vertexs[2][1] = Vertexs[3][1] = -1.0f; // Ybegin


    if(uVramTextureID == 0) {
//        uVramTextureID = CreateNullTexture(642, 402); //  ドットゴーストを防ぐ
        uVramTextureID = CreateNullTexture(640, 400); //  ドットゴーストを防ぐ
    }
     /*
     * 20110904 OOPS! Updating-Texture must be in Draw-Event-Handler(--;
     */
    InitContextCL();   

    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_TRANSFORM_BIT);
    glPushAttrib(GL_ENABLE_BIT);

    drawUpdateTexture(p, w, h);
   
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);

   
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
   
    /*
     * VRAMの表示:テクスチャ貼った四角形
     */
     if(uVramTextureID != 0) {
        glBindTexture(GL_TEXTURE_2D, uVramTextureID);
        if(!bSmoosing) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        if(bGL_EXT_VERTEX_ARRAY) {
            glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
            glEnable(GL_VERTEX_ARRAY_EXT);

            glTexCoordPointerEXT(2, GL_FLOAT, 0, 4, TexCoords);
            glVertexPointerEXT(3, GL_FLOAT, 0, 4, Vertexs);
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
     }
     // 20120502 輝度調整
    glBindTexture(GL_TEXTURE_2D, 0); // 20111023
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
   
    glColor3f(fBrightR , fBrightG, fBrightB);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
//    glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        if(bGL_EXT_VERTEX_ARRAY) {
            glEnable(GL_VERTEX_ARRAY_EXT);
            glVertexPointerEXT(3, GL_FLOAT, 0, 4, Vertexs);
            glDrawArraysEXT(GL_POLYGON, 0, 4);
            glDisable(GL_VERTEX_ARRAY_EXT);
        } else {
            glBegin(GL_POLYGON);
            glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);
            glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);
            glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);
            glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
            glEnd();
        }

    glBlendFunc(GL_ONE, GL_ZERO);
   
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    if((glv->wid.rView.h >= (h * 2)) && (bFullScan == 0)) {
       glLineWidth((float)(glv->wid.rView.h) / (float)(h * 2));
       glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
       glBegin(GL_LINES);
       for(y = 0; y < h; y++) {
	  yf = -1.0f + (float) y * 2.0f / (float)h;
	  glVertex3f(-1.0f, yf, 0.96f);  
	  glVertex3f(+1.0f, yf, 0.96f);  
       }
       glEnd();
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
#ifdef USE_OPENGL
    DrawOSDGL(glv);
#endif
    glPopAttrib();
    glPopAttrib();
    glPopAttrib();
    glFlush();
   
}

void AGEventKeyUpGL(AG_Event *event)
{
    int key = AG_INT(1);
    int mod = AG_INT(2);
    Uint32 ucs = AG_ULONG(3);
	OnKeyReleaseAG(key, mod, ucs);
}

void AGEventKeyDownGL(AG_Event *event)
{
    int key = AG_INT(1);
    int mod = AG_INT(2);
    Uint32 ucs = AG_ULONG(3);
	OnKeyPressAG(key, mod, ucs);

}

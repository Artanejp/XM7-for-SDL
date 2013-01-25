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
# include <agar_glcl.h>
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
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"


GLuint uVramTextureID;
#ifdef _USE_OPENCL
class GLCLDraw *cldraw = NULL;
extern "C" 
{
extern BOOL bUseOpenCL;
}

#endif

static int bModeOld;
static Uint32 *pFrameBuffer;
static BOOL bInitCL;
extern const char *cl_render;

void SetVramReader_GL2(void p(Uint32, Uint32 *, Uint32), int w, int h)
{
//    pGetVram = p;
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
    if(pFrameBuffer != NULL) {
       free(pFrameBuffer);
       pFrameBuffer = NULL;
    }
#ifdef _USE_OPENCL
   if(cldraw != NULL) delete cldraw;
#endif
//    pGetVram = NULL;
}

// Grids
static GLfloat *GridVertexs200l;
static GLfloat *GridVertexs400l;
static GLfloat MainTexcoods[4];
// Brights
static float fBrightR;
static float fBrightG;
static float fBrightB;



static void InitGridVertexsSub(int h, GLfloat *vertex)
{
  int i;
  int j;
  GLfloat ybegin;
  GLfloat yofset;
  GLfloat yinc;
  GLfloat y;
  int base;
    j = h + 1;
    yinc = -4.0f / 400.0f;
    ybegin = 1.0f;
    yofset = -5.0f / 400.0f;
  if(vertex == NULL) return;
//  y = ybegin + yofset;
  y = ybegin + yofset / 4.0f;
  for(i = 0; j > 0 ; j--, i++, y += yinc){
      base = i * 6;
      vertex[base] = -1.0f; // x
      vertex[base + 1] = y; // y
      vertex[base + 2] = -0.99f; // z

      vertex[base + 3] = 1.0f; // x
      vertex[base + 4] = y; // y
      vertex[base + 5] = -0.99f; // z

  }
}

static void InitGridVertexs(void)
{
    GridVertexs200l = (GLfloat *)malloc(202 * 6 * sizeof(GLfloat));
    if(GridVertexs200l != NULL) {
        InitGridVertexsSub(200, GridVertexs200l);
    }
    GridVertexs400l = (GLfloat *)malloc(402 * 6 * sizeof(GLfloat));
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
#ifdef _USE_OPENCL
   cldraw = NULL;
#endif
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
	pFrameBuffer = (Uint32 *)malloc(sizeof(Uint32) * 640 * 400);
        if(pFrameBuffer == NULL) return;
        memset(pFrameBuffer, 0x00, sizeof(Uint32) * 640 * 400);
	InitVramSemaphore();
	uVramTextureID = 0;
	pVirtualVram = NULL;
        bInitCL = FALSE;
	InitVirtualVram();
        InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
               // FBOの有無を受けて、拡張の有無変数を変更する（念のために）
        InitGLExtensionVars();
//    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // Double buffer
    InitGridVertexs(); // Grid初期化
    fBrightR = 1.0; // 輝度の初期化
    fBrightG = 1.0;
    fBrightB = 1.0;

    return;
}


void SetBrightRGB_AG_GL2(float r, float g, float b)
{
   fBrightR = r;
   fBrightG = g;
   fBrightB = b;
}

Uint32 *GetVirtualVram(void)
{
    if(pVirtualVram == NULL) return NULL;
    return &(pVirtualVram->pVram[0][0]);
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
   if(pFrameBuffer == NULL) return;
   ofset = x + y * 640;

   src = (v4hi *)p;
   addr1 = (v4hi *)(&pFrameBuffer[ofset]);
   addr1[0] = src[0];
   addr1[1] = src[1];
   addr1 += 160;

   addr1[0] = src[2];
   addr1[1] = src[3];
   addr1 += 160;

   addr1[0] = src[4];
   addr1[1] = src[5];
   addr1 += 160;

   addr1[0] = src[6];
   addr1[1] = src[7];
   addr1 += 160;

   addr1[0] = src[8];
   addr1[1] = src[9];
   addr1 += 160;

   addr1[0] = src[10];
   addr1[1] = src[11];
   addr1 += 160;

   addr1[0] = src[12];
   addr1[1] = src[13];
   addr1 += 160;

   addr1[0] = src[14];
   addr1[1] = src[15];
//   addr1 += 160;
}

static void drawGrids(void *pg,int w, int h)
{
    AG_GLView *glv = (AG_GLView *)pg;
    float width;
   
    if(!bFullScan){
        int sh = AGWIDGET(glv)->h;
    	width = (float)sh / 800.0f * 1.5f;
    	if(width < 0.70f) goto e1;
        glLineWidth(width);
        {
           GLfloat *vertex;
           int base;

        switch(bMode) {
        case SCR_400LINE:
            goto e1;
            break;
        default:
            vertex = GridVertexs200l;
            break;
        }
        if(vertex == NULL) goto e1;
    	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

        if(bGL_EXT_VERTEX_ARRAY) {
                glVertexPointerEXT(3, GL_FLOAT, 0, h + 1, vertex);
                glEnable(GL_VERTEX_ARRAY_EXT);
                glDrawArraysEXT(GL_LINES, 0, h + 1);
                glDisable(GL_VERTEX_ARRAY_EXT);
        } else {
	    int i;
            GLfloat *v;
            // VERTEX_ARRAYなしの場合
            glBegin(GL_LINES);
            for(i = 0; i < (h + 1); i++) {
                v = &vertex[i * 6];
                glVertex3f(v[0], v[1], v[2]);
                glVertex3f(v[3], v[4], v[5]);
            }
            glEnd();
        }
    }

    }
e1:
   return;
}


static void drawUpdateTexture(Uint32 *p, int w, int h)
{
//    LockVram();

    if((SDLDrawFlag.Drawn) && (uVramTextureID != 0)){
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

//#ifdef _OPENMP
//       #pragma omp parallel for shared(p, SDLDrawFlag, ww, hh) private(pu, xx)
//#endif
#ifdef _USE_OPENCL
       if((cldraw != NULL) && bGL_PIXEL_UNPACK_BUFFER_BINDING) {
	  LockVram();
	  cldraw->GetVram(bMode);
	  UnlockVram();

	  glBindTexture(GL_TEXTURE_2D, uVramTextureID);
	  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, cldraw->GetPbo());
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
	  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	  bVramUpdateFlag = TRUE;

       } else {
#endif
	  LockVram();
	  for(yy = 0; yy < hh; yy++) { // 20120411 分割アップデートだとGLドライバによっては遅くなる
	     for(xx = 0; xx < ww; xx++) {
                    if(SDLDrawFlag.write[xx][yy]) {
                    pu = &p[(xx + yy * ww) << 6];
                    UpdateFramebufferPiece(pu, xx << 3, yy << 3);
                    SDLDrawFlag.write[xx][yy] = FALSE;
                    }
                }
            }

            if(pFrameBuffer != NULL) {
	       glBindTexture(GL_TEXTURE_2D, uVramTextureID);
	       UpdateTexturePiece(pFrameBuffer, uVramTextureID, 0, 0, 640, h);
	       glBindTexture(GL_TEXTURE_2D, 0); // 20111023 チラつきなど抑止
	    }
	  UnlockVram();
#ifdef _USE_OPENCL
       }
#endif       
       


    }
    SDLDrawFlag.Drawn = FALSE;

   

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
   GLfloat TexCoords2[4][2];
   
   if(pVirtualVram == NULL) return;
   p = &(pVirtualVram->pVram[0][0]);
   if(p == NULL) return;

//   printf("DrawGL2: %d\n", AG_GetTicks());

    ybegin = 1.0f;
//    ybegin = 400.0f/460.0f;
    yend = 1.0f;
            // Z Axis
   
     switch(bMode) {
        case SCR_400LINE:
            w = 640;
            h = 400;
            TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
            TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin

            TexCoords[2][0] = TexCoords[1][0] = 640.0f / 640.0f; // Xend
            TexCoords[2][1] = TexCoords[3][1] = 400.0f / 400.0f; // Yend
            break;
        case SCR_200LINE:
            w = 640;
            h = 200;
            TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
            TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin

            TexCoords[2][0] = TexCoords[1][0] = 640.0f / 640.0f; // Xend
            TexCoords[2][1] = TexCoords[3][1] = 199.0f / 400.0f; // Yend
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
            break;
     }

    Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = -0.98f;
    Vertexs[0][0] = Vertexs[3][0] = -1.0f; // Xbegin
    Vertexs[0][1] = Vertexs[1][1] = yend;  // Yend
    Vertexs[2][0] = Vertexs[1][0] = 1.0f; // Xend
    Vertexs[2][1] = Vertexs[3][1] = -ybegin; // Ybegin


    if(uVramTextureID == 0) {
//        uVramTextureID = CreateNullTexture(642, 402); //  ドットゴーストを防ぐ
        uVramTextureID = CreateNullTexture(640, 400); //  ドットゴーストを防ぐ
    }
#ifdef _USE_OPENCL
     if(bUseOpenCL && (cldraw == NULL) && bGL_PIXEL_UNPACK_BUFFER_BINDING && (!bInitCL)) {
	    cl_int r;
	    cldraw = new GLCLDraw;
	    if(cldraw != NULL) {
	       r = cldraw->InitContext();
	       printf("CTX: STS = %d \n", r);
	       if(r == CL_SUCCESS){  
		 r = cldraw->BuildFromSource(cl_render);
		  printf("Build: STS = %d \n", r);
	         if(r == CL_SUCCESS) {
		    r = cldraw->SetupBuffer(uVramTextureID);
		    r |= cldraw->SetupTable();
		    if(r != CL_SUCCESS){
		       delete cldraw;
		       cldraw = NULL;
		    }
		 } else {
		    delete cldraw;
		    cldraw = NULL;
		 }
	       } else {
		  delete cldraw;
		  cldraw = NULL;
	       }
	       
	
	    }
	bInitCL = TRUE;
    }
#endif // _USE_OPENCL   
     /*
     * 20110904 OOPS! Updating-Texture must be in Draw-Event-Handler(--;
     */
   

    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_TRANSFORM_BIT);
    glPushAttrib(GL_ENABLE_BIT);

    drawUpdateTexture(p, w, h);
   
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);

   
    glDisable(GL_DEPTH_TEST);
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
   
    glEnable(GL_BLEND);
    glColor3f(fBrightR , fBrightG, fBrightB);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
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
    glEnable(GL_DEPTH_TEST);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

    drawGrids(glv, w, h);
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

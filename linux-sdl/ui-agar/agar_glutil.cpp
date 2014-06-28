/*
 * Agar: OpenGLUtils
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include "agar_glutil.h"
#ifdef _USE_OPENCL
#include "agar_glcl.h"
#endif

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP

extern "C" {
    AG_GLView *GLDrawArea;
    extern BOOL bUseOpenCL;
    BOOL bInitCL;
}

GLuint GridVertexs200l;
GLuint GridVertexs400l;

extern Uint32 *pFrameBuffer;
// Brights
extern float fBrightR;
extern float fBrightG;
extern float fBrightB;
extern const char *cl_render;
extern GLuint uVramTextureID;

#ifdef _USE_OPENCL
class GLCLDraw *cldraw = NULL;
#endif


GLuint UpdateTexturePiece(Uint32 *p, GLuint texid, int x, int y, int w, int h)
{
//    glBindTexture(GL_TEXTURE_2D, texid);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    x, y + 1, w, h,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,(GLvoid *)p);
}

GLuint CreateNullTexture(int w, int h)
{
    GLuint ttid;
    Uint32 *p;

    p =(Uint32 *)malloc((w + 2)*  (h  + 2) * sizeof(Uint32));
    if(p == NULL) return 0;

    memset(p, 0x00, (w + 2) * (h + 2) * sizeof(Uint32));
    glGenTextures(1, &ttid);
    glBindTexture(GL_TEXTURE_2D, ttid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1); // Limit mipmap level , reduce resources.
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h + 2,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 p);
    free(p);
    return ttid;
}

GLuint UpdateTexture(Uint32 *p, GLuint texid, int w, int h)
{
    GLuint ttid;

    if((w < 0) || (h < 0)) return 0;
    if(p == NULL) return 0;

    LockVram();
    ttid = texid;
    if(texid == 0) {
        glGenTextures(1, &ttid);
        glBindTexture(GL_TEXTURE_2D, ttid);
        glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 p);
    } else {
#if 1 // texSubImage2D()で置き換えるとAgar側がちらつく(--;
       glBindTexture(GL_TEXTURE_2D, ttid);
       glTexSubImage2D(GL_TEXTURE_2D,
                         0,  // level
                         0, 0, // offset
                         w, h,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         p );
#else
       glDeleteTextures(1, &texid);
       glGenTextures(1, &ttid);
       glBindTexture(GL_TEXTURE_2D, ttid);
       glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 w, h,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 p);
#endif
    }
    UnlockVram();
    return ttid;
}

void Flip_AG_GL(void)
{
	if(!InitVideo) return;
//	SDL_GL_SwapBuffers();
}

void DiscardTextures(int n, GLuint *id)
{
	if(GLDrawArea == NULL) return;
	if(agDriverOps == NULL) return;
	glDeleteTextures(n, id);

}

void DiscardTexture(GLuint tid)
{
	DiscardTextures(1, &tid);
}


void InitContextCL(void)
{
      if(GLDrawArea == NULL) return; // Context not created yet.
#ifdef _USE_OPENCL
     if(bUseOpenCL && (cldraw == NULL) && 
	bGL_PIXEL_UNPACK_BUFFER_BINDING && (!bInitCL)) {
	    cl_int r;
	    cldraw = new GLCLDraw;
	    if(cldraw != NULL) {
	       r = cldraw->InitContext();
	       printf("CTX: STS = %d \n", r);
	       if(r == CL_SUCCESS){
		 printf("CL: GLCTX=%08x\n", glXGetCurrentContext());
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
    }
   bInitCL = TRUE;     

#endif // _USE_OPENCL   
}


static GLuint InitGridVertexsSub(int h)
{
   Uint32 *p;
   int y;
   int x;
   GLuint ttid;
   
   return 0;
}


void InitGridVertexs(void)
{
   GridVertexs200l =  InitGridVertexsSub(200);
   GridVertexs400l =  InitGridVertexsSub(400);
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
	pVram2 = NULL;
#ifdef _OPENCL
        bInitCL = FALSE;
#endif
	InitVirtualVram();
        //if(AG_UsingSDL(NULL)) {
	   InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
	   // FBOの有無を受けて、拡張の有無変数を変更する（念のために）
	   InitGLExtensionVars();
	   InitGridVertexs(); // Grid初期化
	//}
   
    fBrightR = 1.0; // 輝度の初期化
    fBrightG = 1.0;
    fBrightB = 1.0;

    return;
}


extern "C" {
// OpenGL状態変数
BOOL bGL_ARB_IMAGING; // イメージ操作可能か？
BOOL bGL_ARB_COPY_BUFFER;  // バッファ内コピー（高速化！）サポート
BOOL bGL_EXT_INDEX_TEXTURE; // パレットモードに係わる
BOOL bGL_EXT_COPY_TEXTURE; // テクスチャ間のコピー
BOOL bGL_SGI_COLOR_TABLE; // パレットモード(SGI拡張)
BOOL bGL_SGIS_PIXEL_TEXTURE; // テクスチャアップデート用
BOOL bGL_EXT_PACKED_PIXEL; // PackedPixelを使ってアップデートを高速化？
BOOL bGL_EXT_VERTEX_ARRAY; // 頂点を配列化して描画を高速化
BOOL bGL_EXT_PALETTED_TEXTURE; // パレットモード（更に別拡張)
BOOL bGL_PIXEL_UNPACK_BUFFER_BINDING; // ピクセルバッファがあるか？

   
// FBO API
PFNGLVERTEXPOINTEREXTPROC glVertexPointerEXT;
PFNGLDRAWARRAYSEXTPROC glDrawArraysEXT;
PFNGLTEXCOORDPOINTEREXTPROC glTexCoordPointerEXT;
//#ifndef _WINDOWS
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLGENBUFFERSPROC glGenBuffers;
//#endif

BOOL QueryGLExtensions(const char *str)
{
    char *ext;
    char *p;
    int i;
    int j;
    int k;
    int l;
    int ll;
//#ifndef _WINDOWS

    if(str == NULL) return FALSE;
    ll = strlen(str);
    if(ll <= 0) return FALSE;

    ext =(char *)glGetString(GL_EXTENSIONS);
    if(ext == NULL) return FALSE;
    l = strlen(ext);
    if(l <= 0) return FALSE;
    p = ext;
    for(i = 0; i < l ; ){
        int j = strcspn(p, " ");
        if((ll == j) && (strncmp(str, p, j) == 0)) {
            return TRUE;
        }
        p += (j + 1);
        i += (j + 1);
    }
//#endif
    return FALSE;
}

void InitGLExtensionVars(void)
{
    bGL_ARB_IMAGING = QueryGLExtensions("GL_ARB_imaging");
    bGL_ARB_COPY_BUFFER = QueryGLExtensions("GL_ARB_copy_buffer");
    bGL_EXT_INDEX_TEXTURE = QueryGLExtensions("GL_EXT_index_texture");
    bGL_EXT_COPY_TEXTURE = QueryGLExtensions("GL_EXT_copy_texture");
    bGL_SGI_COLOR_TABLE = QueryGLExtensions("GL_SGI_color_table");
    bGL_SGIS_PIXEL_TEXTURE = QueryGLExtensions("GL_SGIS_pixel_texture");
    bGL_EXT_PACKED_PIXEL = QueryGLExtensions("GL_EXT_packed_pixel");
    bGL_EXT_PALETTED_TEXTURE = QueryGLExtensions("GL_EXT_paletted_texture");
    bGL_EXT_VERTEX_ARRAY = QueryGLExtensions("GL_EXT_vertex_array");
//    bGL_PIXEL_UNPACK_BUFFER_BINDING = QueryGLExtensions("GL_pixel_unpack_buffer_binding");
    bGL_PIXEL_UNPACK_BUFFER_BINDING = TRUE;

}

   
#ifdef _WINDOWS
#include <windef.h>
extern PROC WINAPI wglGetProcAddress(LPCSTR lpszProc);
//#else 
//extern void *glXGetProcAddress(const GLubyte *);
#endif
   
void InitFBO(void)
{
//#ifndef _WINDOWS // glx is for X11.
// Use SDL for wrapper. 20130128
    if(AG_UsingSDL(NULL)) {
       glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)SDL_GL_GetProcAddress("glVertexPointerEXT");
       if(glVertexPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)SDL_GL_GetProcAddress("glDrawArraysEXT");
       if(glDrawArraysEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)SDL_GL_GetProcAddress("glTexCoordPointerEXT");
       if(glTexCoordPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
       if(glBindBuffer == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
       if(glBufferData == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
       if(glGenBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
    } else { // glx, wgl
#ifndef _WINDOWS
       glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)glXGetProcAddress((const GLubyte *)"glVertexPointerEXT");
       if(glVertexPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)glXGetProcAddress((const GLubyte *)"glDrawArraysEXT");
       if(glDrawArraysEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)glXGetProcAddress((const GLubyte *)"glTexCoordPointerEXT");
       if(glTexCoordPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindBuffer");
       if(glBindBuffer == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte *)"glBufferData");
       if(glBufferData == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenBuffers");
       if(glGenBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
#else
       glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)wglGetProcAddress("glVertexPointerEXT");
       if(glVertexPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)wglGetProcAddress("glDrawArraysEXT");
       if(glDrawArraysEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)wglGetProcAddress("glTexCoordPointerEXT");
       if(glTexCoordPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
       if(glBindBuffer == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
       if(glBufferData == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
       if(glGenBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;

#endif // _WINDOWS    
    }
   
}

}

/*
 * Agar: OpenGLUtils
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include "agar_glutil.h"
extern "C" {
    AG_GLView *GLDrawArea;
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
#if 0 // texSubImage2D()で置き換えるとAgar側がちらつく(--;
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


BOOL QueryGLExtensions(char *str)
{
    char *ext;
    char *p;
    int i;
    int j;
    int k;
    int l;
    int ll;

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

}
}

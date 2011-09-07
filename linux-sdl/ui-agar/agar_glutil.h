#ifndef AGAR_GLUTIL_H_INCLUDED
#define AGAR_GLUTIL_H_INCLUDED


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
#include "agar_vramutil.h"
#include "agar_draw.h"
#include "agar_gldraw.h"

extern "C" {
extern  AG_GLView *GLDrawArea;
extern BOOL bGL_ARB_IMAGING; // イメージ操作可能か？
extern BOOL bGL_ARB_COPY_BUFFER;  // バッファ内コピー（高速化！）サポート
extern BOOL bGL_EXT_INDEX_TEXTURE; // パレットモードに係わる

extern void InitGLExtensionVars(void);
extern BOOL QueryGLExtensions(char *str);
}

extern GLuint UpdateTexture(Uint32 *p, GLuint texid, int w, int h);
extern void Flip_AG_GL(void);
extern void DiscardTextures(int n, GLuint *id);
extern void DiscardTexture(GLuint id);


#endif // AGAR_GLUTIL_H_INCLUDED

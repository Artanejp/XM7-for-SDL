/*
 * Agar: OpenGLUtils
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include "agar_glutil.h"
extern "C" {
    AG_GLView *GLDrawArea;
}
/*
 *　GL Utility Routines
 */
GLuint UpdateTexture(Uint32 *p, GLuint texid, int w, int h)
{
    GLuint ttid;

    if((w < 0) || (h < 0)) return 0;
    if(p == NULL) return 0;

    LockVram();
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
        ttid = texid;
        glBindTexture(GL_TEXTURE_2D, ttid);
        glTexSubImage2D(GL_TEXTURE_2D,
                         0,  // level
                         0, 0, // offset
                         w, h,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         p );
    }
//    uVramTextureID = ttid;
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


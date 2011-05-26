/*
 * EmuAgarGL.h
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 */

#ifndef EMUAGARGL_H_
#define EMUAGARGL_H_


#include "EmuGLUtils.h"
#include <SDL.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>


#ifndef Uint
#define Uint unsigned int
#endif

class EmuAgarGL: public EmuGLUtils {
public:
	EmuAgarGL();
	virtual ~EmuAgarGL();
	void InitUI(char *name);
	void InitUI(char *name, Uint Flags);
	void InitGL(int w, int h);
	void SetDrawArea(int x, int y, int w, int h);
	void SetDrawArea(AG_GLView *p, int x, int y, int w, int h);
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void PutVram(AG_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void Flip(void);
	void SetViewPort(void);
	void SetViewPort(AG_Widget *wid);
    void SetViewPort(int x, int y, int w, int h);
    void SetViewPort(int x, int y, int w, int h, int osd_w, int osd_h);
    void CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void SetPaletteTable(Uint32 *p);
    void DiscardTexture(GLuint tid);
    void DiscardTextures(int n, GLuint *tids);
    GLuint CreateTexture(AG_Surface *p);
    void DrawTexture(GLuint tid);
    void SetTextureID(GLuint id);
    GLuint GetTextureID(void);
    AG_Surface *GetVramSurface(void);
    void SetOffset(int x, int y);
protected:
    void DiscardTexture(void);
    void Enter2DMode(void);
    void Leave2DMode(void);
    void DrawTexture(void);
    AG_Surface *video;   // スケーリング後
    AG_Surface *pixvram; // スケーリング前
    Uint32 *palette;
    BOOL   InitVideo;
    AG_GLView *drawarea;
    AG_PixelFormat format;
    SDL_semaphore *drawSem;
	int offset_x;
	int offset_y;
	int osd_w;
	int osd_h;

private:
    BOOL  UseTexture;
	AG_TexCoord texcoord;

};

#endif /* EMUAGARGL_H_ */

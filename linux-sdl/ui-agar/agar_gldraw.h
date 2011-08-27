/*
 * agar_gldraw.h
 *
 *  Created on: 2011/01/21
 *      Author: whatisthis
 */

#ifndef AGAR_GLDRAW_H_
#define AGAR_GLDRAW_H_

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
#include "EmuAgarGL.h"
#include "api_draw.h"
#include "api_scaler.h"

#include "agar_draw.h"

extern "C" {
extern AG_GLView *GLDrawArea;
}
extern BOOL EventSDL(AG_Driver *drv);
extern BOOL EventGUI(AG_Driver *drv);
extern void DrawOSDGL(AG_GLView *w);

extern void AGEventScaleGL(AG_Event *event);
extern void AGEventDrawGL(AG_Event *event);

extern void AGEventOverlayGL(AG_Event *event);
extern void AGEventMouseMove_AG_GL(AG_Event *event);
extern void AGEventKeyRelease_AG_GL(AG_Event *event);
extern void AGEventKeyPress_AG_GL(AG_Event *event);

extern void InitGL_AG_GL(int w, int h);
extern void Detach_AG_GL();

extern "C" {
extern void LockVram(void);
extern void UnlockVram(void);
}

//extern void CalcPalette_AG_GL(Uint32 *palette, Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

//extern void SetVramReader_AG_GL(void p(Uint32, Uint32 *, Uint32), int w, int h);
//extern void SetDrawArea_AG_GL(AG_Widget *p, int x, int y, int w, int h);
extern void Flip_AG_GL(void);

//extern void PutVram_AG_GL(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern AG_Surface *GetVramSurface_AG_GL();

/*
 * agar_gldraw2.cpp
 */
extern void InitGL_AG2(int w, int h);
extern void DetachGL_AG2(void);
extern void SetVramReader_GL2(void p(Uint32, Uint32 *, Uint32), int w, int h);
extern void SetVram_200l(Uint8 *p);

extern void PutVram_AG_GL2(SDL_Surface *p, int x, int y, int w, int h,  Uint32 mpage);
extern void AGEventDrawGL2(AG_Event *event);
extern void CalcPalette_8colors(Uint32 index, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
extern void CalcPalette_4096Colors(Uint32 index, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
extern GLuint UpdateTexture(Uint32 *p, int w, int h);
extern void DiscardTexture(GLuint tid);
extern  GLuint uVramTextureID;


#endif /* AGAR_GLDRAW_H_ */

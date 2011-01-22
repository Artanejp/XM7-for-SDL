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
#include <libemugrph/EmuAgarGL.h>
#include "api_draw.h"
#include "api_scaler.h"

extern "C" {
extern AG_GLView *DrawArea;
extern AG_Window *MainWindow;
extern AG_Menu  *MenuBar;
extern void DrawStatus(void);
}
extern void EventSDL(AG_Driver *drv);
extern void EventGUI(AG_Driver *drv);
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
extern void UnLockVram(void);
}

extern void CalcPalette_AG_GL(Uint32 *palette, Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

extern void SetVramReader_AG_GL(void p(Uint32, Uint32 *, Uint32), int w, int h);
extern void SetDrawArea_AG_GL(AG_Widget *p, int x, int y, int w, int h);
extern void Flip_AG_GL(void);

extern void PutVram_AG_GL(AG_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern AG_Surface *GetVramSurface_AG_GL();


#endif /* AGAR_GLDRAW_H_ */

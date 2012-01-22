#ifndef AGAR_DRAW_H_INCLUDED
#define AGAR_DRAW_H_INCLUDED

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
//#include "EmuAgarGL.h"
#include "api_draw.h"
//#include "api_scaler.h"

#include "agar_vramutil.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"

//#include "DrawAGNonGL.h"

#include <SDL.h>
#ifdef __cplusplus
extern "C" {
#endif
    extern XM7_SDLView *DrawArea;
    extern AG_Window *MainWindow;
    extern AG_Menu  *MenuBar;
    extern AG_Box *pStatusBar;
    extern void DrawStatus(void);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern Uint32 nDrawTick1E;


extern void InitGL(int w, int h);
extern void InitNonGL(int w, int h);
extern SDL_Surface *GetDrawSurface(void);
extern void ResizeWindow_Agar(int w, int h);
extern void AGDrawTaskMain(void);
#endif
#endif // AGAR_DRAW_H_INCLUDED

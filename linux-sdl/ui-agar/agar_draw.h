#ifndef AGAR_DRAW_H_INCLUDED
#define AGAR_DRAW_H_INCLUDED

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
//#include "EmuAgarGL.h"
#include "api_draw.h"
#include "api_scaler.h"

#include "agar_vramutil.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdldraw.h"

//#include "DrawAGNonGL.h"

#include <SDL.h>

extern "C" {
    extern AG_Box *DrawArea;
    extern AG_Window *MainWindow;
    extern AG_Menu  *MenuBar;
    extern void DrawStatus(void);
}

extern Uint32 nDrawTick1E;


extern void InitDrawArea(int w, int h);
extern void DetachDrawArea(void);
extern void LinkDrawArea(AG_Widget *w);
extern SDL_Surface *GetDrawSurface(void);
extern void ResizeWindow_Agar(int w, int h);
extern void AGDrawTaskMain(void);
#endif // AGAR_DRAW_H_INCLUDED

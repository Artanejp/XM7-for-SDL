#ifndef DRAWAGNONGL_H
#define DRAWAGNONGL_H


#define MAXLINKWIDGETS 8

#include <SDL.h>
#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
#include "api_draw.h"
#include "api_scaler.h"
#include "agar_xm7.h"

class DrawAGNonGL
{
    public:
        DrawAGNonGL();
        virtual ~DrawAGNonGL();

        void CalcPalette(Uint32 *palette, Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
        void InitDraw(int w, int h);
        void Lock(void);
        void Unlock(void);
        BOOL LinkVram(AG_Widget *w);
        BOOL UnlinkVram(AG_Widget *w);
        BOOL IsLinked(AG_Widget *w);
        SDL_Surface *GetSDLSurface(void);
        AG_Surface *GetVram(void);
        void Flip(void);
    protected:
        BOOL InitVideo;
        SDL_sem *VramSem;
        AG_Surface *pixvram;
        SDL_Surface *ShadowSurface; // pinvramのSDL_Surface型変換後の値
        int SurfaceIDs[MAXLINKWIDGETS];
        AG_Widget *Widgets[MAXLINKWIDGETS]; // list型がいいのかなぁ…

        int GetMaxWidgets(void);
        SDL_Surface *MakeShadowSurface(void);
        void FreeShadowSurface(void);
};

#endif // DRAWAGNONGL_H

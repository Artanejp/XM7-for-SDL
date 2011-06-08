/*
 * Draw Console (NON-GL Mode)
 *
 * K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 *    06/05/2011 Initial
 */

#include "DrawAGNonGL.h"

DrawAGNonGL::DrawAGNonGL()
{
    //ctor
    int i;
    InitVideo = FALSE;
    VramSem = NULL;
    pixvram = NULL;
    ShadowSurface = NULL;
    for(i = 0; i < MAXLINKWIDGETS; i++) {
        Widgets[i] = NULL;
        SurfaceIDs[i] = 0;
    }
}

DrawAGNonGL::~DrawAGNonGL()
{
    //dtor
    int i;
    for(i = 0; i < MAXLINKWIDGETS; i++) {
        if(Widgets[i] != NULL) {
            AG_WidgetUnmapSurface(Widgets[i], SurfaceIDs[i]);
        }
    }
    if(ShadowSurface != NULL) {
        FreeShadowSurface();
    }
	if(VramSem != NULL) {
		SDL_SemWait(VramSem);
		SDL_DestroySemaphore(VramSem);
		VramSem = NULL;
	}
	if(pixvram){
		AG_SurfaceFree(pixvram);
		pixvram = NULL;
	}
}


static inline void putdot32(Uint8 *addr, Uint32 c)
{
	Uint32 *addr32 = (Uint32 *)addr;
	*addr32 = c;
}


static inline void putword32(Uint32 *disp, Uint32 *cbuf)
{
		putdot32((Uint8 *)&disp[0], cbuf[7]);
		putdot32((Uint8 *)&disp[1], cbuf[6]);
		putdot32((Uint8 *)&disp[2], cbuf[5]);
		putdot32((Uint8 *)&disp[3], cbuf[4]);
		putdot32((Uint8 *)&disp[4], cbuf[3]);
		putdot32((Uint8 *)&disp[5], cbuf[2]);
		putdot32((Uint8 *)&disp[6], cbuf[1]);
		putdot32((Uint8 *)&disp[7], cbuf[0]);
}

static inline void putdot16(Uint8 *addr, Uint16 c)
{
	Uint16 *addr16 = (Uint16 *)addr;
	*addr16 = c;
}


static inline void putword16(Uint16 *disp, Uint32 *cbuf)
{
		putdot16((Uint8 *)&disp[0], cbuf[7]);
		putdot16((Uint8 *)&disp[1], cbuf[6]);
		putdot16((Uint8 *)&disp[2], cbuf[5]);
		putdot16((Uint8 *)&disp[3], cbuf[4]);
		putdot16((Uint8 *)&disp[4], cbuf[3]);
		putdot16((Uint8 *)&disp[5], cbuf[2]);
		putdot16((Uint8 *)&disp[6], cbuf[1]);
		putdot16((Uint8 *)&disp[7], cbuf[0]);
}

/*
 * パレットの計算
 */

void DrawAGNonGL::CalcPalette(Uint32 *palette, Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint32 ds;
	AG_Surface *dst = pixvram;
	if(palette == NULL) return;
	if(dst == NULL) return;

    ds = ( r << dst->format->Rshift) & dst->format->Rmask +
     ( r << dst->format->Gshift) & dst->format->Gmask +
     ( r << dst->format->Bshift) & dst->format->Bmask +
     ( r << dst->format->Ashift) & dst->format->Amask;
	palette[src] = ds;
}


SDL_Surface *DrawAGNonGL::MakeShadowSurface(void)
{
    SDL_Surface *p;
    SDL_PixelFormat *f;

    p = (SDL_Surface *)malloc(sizeof(SDL_Surface));
    if(p == NULL) return NULL;
    f = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
    if(f == NULL) {
        free(p);
        return NULL;
    }
    ShadowSurface = p;

    if(p == NULL) return NULL;
    if(pixvram == NULL) return NULL;
    if(p->format == NULL) return NULL;
    if(pixvram->format == NULL) return NULL;

    p->w = pixvram->w;
    p->h = pixvram->h;
    p->pixels = pixvram->pixels;
    p->pitch = pixvram->pitch;
    p->flags = SDL_SWSURFACE;
    p->clip_rect.x = pixvram->clipRect.x;
    p->clip_rect.y = pixvram->clipRect.y;
    p->clip_rect.w = pixvram->clipRect.w;
    p->clip_rect.h = pixvram->clipRect.h;
    p->refcount = 0;

    p->format->Aloss = pixvram->format->Aloss;
    p->format->Amask = pixvram->format->Amask;
    p->format->Ashift = pixvram->format->Ashift;

    p->format->Rloss = pixvram->format->Rloss;
    p->format->Rmask = pixvram->format->Rmask;
    p->format->Rshift = pixvram->format->Rshift;

    p->format->Gloss = pixvram->format->Gloss;
    p->format->Gmask = pixvram->format->Gmask;
    p->format->Gshift = pixvram->format->Gshift;

    p->format->Bloss = pixvram->format->Bloss;
    p->format->Bmask = pixvram->format->Bmask;
    p->format->Bshift = pixvram->format->Bshift;

    p->format->alpha = pixvram->format->alpha;
    p->format->colorkey = pixvram->format->colorkey;
    p->format->BytesPerPixel = pixvram->format->BytesPerPixel;
    p->format->BitsPerPixel = pixvram->format->BitsPerPixel;
    return p;
}

void DrawAGNonGL::FreeShadowSurface(void)
{
    if(ShadowSurface == NULL) return;
    if(ShadowSurface->format != NULL) {
        free(ShadowSurface->format);
        ShadowSurface->format = NULL;
    }
    free(ShadowSurface);
    ShadowSurface = NULL;
}
/*
 * VRAMをアロケートする
 */
void DrawAGNonGL::InitDraw(int w, int h)
{

    AG_PixelFormat fmt;

	if(InitVideo) return;
    InitVideo = TRUE;

	if((pixvram == NULL) &&(w != 0) &&(h != 0)) {
	    pixvram = AG_SurfaceRGBA(w, h, 32, AG_SRCALPHA, 0x00ff000, 0x0000ff00, 0x000000ff, 0xff000000);
	    if(ShadowSurface != NULL) {
	        FreeShadowSurface();
	    }
        ShadowSurface = MakeShadowSurface();
	} else {
	    AG_SurfaceFree(pixvram);
	    pixvram = AG_SurfaceRGBA(w, h, 32, AG_SRCALPHA, 0x00ff00000, 0x0000ff00, 0x000000ff, 0xff000000);
	}
	if(VramSem == NULL) {
		VramSem = SDL_CreateSemaphore(1);
		if(VramSem) SDL_SemPost(VramSem);
	}
	return;
}


/*
 * VRAMをロックする
 */
void DrawAGNonGL::Lock(void)
{
    if(VramSem == NULL) return;
    SDL_SemWait(VramSem);
}

/*
 * VRAMをアンロックする
 */
void DrawAGNonGL::Unlock(void)
{
    if(VramSem == NULL) return;
    SDL_SemPost(VramSem);
}


/*
 * VRAMを取得する
 */
AG_Surface *DrawAGNonGL::GetVram(void)
{
	return pixvram;
}

/*
 * VramをWidgetにリンクする
 */
BOOL DrawAGNonGL::LinkVram(AG_Widget *w)
{
    int i;
    if(pixvram == NULL) return FALSE;
    if(w == NULL) return FALSE;

    if(VramSem == NULL) return FALSE;
    SDL_SemWait(VramSem);

    for(i = 0; i < GetMaxWidgets(); i++) {
        if(Widgets[i] == NULL) {
            SurfaceIDs[i] = AG_WidgetMapSurface(w, pixvram);
            SDL_SemPost(VramSem);
            return TRUE;
        }
    }
    SDL_SemPost(VramSem);
    return FALSE;
}

/*
 * Widgetをリンクから外す
 */

BOOL DrawAGNonGL::UnlinkVram(AG_Widget *w)
{
    BOOL found = FALSE;
    int i;

    if(w == NULL) return FALSE;
    if(VramSem == NULL) return FALSE;
    SDL_SemWait(VramSem);

    for(i = 0; i < GetMaxWidgets(); i++) {
        if(Widgets[i] == w) {
            AG_WidgetUnmapSurface(Widgets[i], SurfaceIDs[i]);
            Widgets[i] = NULL;
            SurfaceIDs[i] = 0;
            found = TRUE;
        }
    }
    SDL_SemPost(VramSem);
    return found;
}

/*
 * Widgetがリンクされてるか問い合わせる
 */
BOOL DrawAGNonGL::IsLinked(AG_Widget *w)
{
    int i;

    if(w == NULL) return FALSE;
    if(VramSem == NULL) return FALSE;
    SDL_SemWait(VramSem);

    for(i = 0; i < GetMaxWidgets(); i++) {
        if(Widgets[i] == w) {
            SDL_SemPost(VramSem);
            return TRUE;
        }
    }
    SDL_SemPost(VramSem);
    return FALSE;
}

/*
 * Surfaceを更新した後、Widget描画に反映させる
 */
void DrawAGNonGL::Flip(void)
{
    int i;
    if(VramSem == NULL) return;
    SDL_SemWait(VramSem);
    for(i = 0; i < GetMaxWidgets(); i++) {
        if(Widgets[i] != NULL) {
            AG_WidgetUpdateSurface(Widgets[i], SurfaceIDs[i]);
        }
    }
    SDL_SemPost(VramSem);
}


int DrawAGNonGL::GetMaxWidgets(void)
{
    return MAXLINKWIDGETS; // List型がいいのかなぁ…
}


SDL_Surface *DrawAGNonGL::GetSDLSurface(void)
{
    return ShadowSurface;
//    return NULL;
}

/*
 * スケーラはソフトウェアの場合libemugrph等の外部関数なので書かない
 */

#ifndef __XM7_SDL_VIEW
#define __XM7_SDL_VIEW

# ifdef __cplusplus
extern "C" {
#endif

/* Structure describing an instance of the XM7_SDLView. */
typedef struct  XM7_SDLView {
	struct ag_widget _inherit;	/* Inherit from AG_Widget */
	int mySurface;			/* Surface handle */
	AG_Surface *Surface;
	const char *param;		/* Some parameter */
} XM7_SDLView;

extern AG_WidgetClass XM7_SDLViewClass;
extern XM7_SDLView *XM7_SDLViewNew(void *, AG_Surface *, const char *);

extern void XM7_SDLViewLinkSurface(void *p, AG_Surface *src);
extern AG_Surface *XM7_SDLViewSurfaceNew(void *p, int w, int h);
extern void XM7_SDLViewSurfaceDetach(void *p);
extern AG_Surface *XM7_SDLViewGetSrcSurface(void *p);
extern void XM7_SDLViewUpdateSrc(void *p);

#ifdef __cplusplus
}
#endif
#endif /* __XM7_SDL_VIEW */

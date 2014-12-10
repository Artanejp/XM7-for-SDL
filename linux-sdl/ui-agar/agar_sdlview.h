/*
* FM-7 Emulator "XM7"
* Virtual Vram Display(Agar widget version)
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* License: CC-BY-SA
* History:
* Jan 18,2012 From demos/customwidget/mywidget.[c|h]
*
*/
#ifndef __XM7_SDL_VIEW
#define __XM7_SDL_VIEW

# ifdef __cplusplus
extern "C" {
#endif

//#include <sys/types.h>
//#include <agar/core/string_compat.h>
#include <agar/core.h>
//#include <agar/core/types.h>
#include <agar/gui.h>

/*
* Do compatibility for widget
*/
#define Strlcat AG_Strlcat
#define Strlcpy AG_Strlcpy
#define Strsep AG_Strsep
#define Strdup AG_Strdup
#define TryStrdup AG_TryStrdup
#define Strcasecmp AG_Strcasecmp
#define Strncasecmp AG_Strncasecmp
#define Strcasestr AG_Strcasestr
#define StrlcatUCS4 AG_StrlcatUCS4
#define StrlcpyUCS4 AG_StrlcpyUCS4
#define StrsepUCS4 AG_StrsepUCS4
#define StrdupUCS4 AG_StrdupUCS4
#define TryStrdupUCS4 AG_TryStrdupUCS4
#define StrReverse AG_StrReverse
#define StrlcpyInt AG_StrlcpyInt
#define StrlcatInt AG_StrlcatInt
#define StrlcpyUint AG_StrlcpyUint
#define StrlcatUint AG_StrlcatUint


/* Structure describing an instance of the XM7_SDLView. */
typedef struct  XM7_SDLView {
	struct ag_widget _inherit;	/* Inherit from AG_Widget */
	int mySurface;			/* Surface handle : CURRENT */
	AG_Event *draw_ev;     // draw handler event
	int forceredraw;
        int dirty;
	const char *param;		/* Some parameter */
} XM7_SDLView;

extern AG_WidgetClass XM7_SDLViewClass;
extern XM7_SDLView *XM7_SDLViewNew(void *, AG_Surface *, const char *);

extern int XM7_SDLViewLinkSurface(void *p, AG_Surface *src);
extern int XM7_SDLViewSurfaceNew(void *p, int w, int h);
extern void XM7_SDLViewSurfaceDetach(void *p);
extern AG_Surface *XM7_SDLViewGetSurface(void *p, int index);
extern AG_Surface *XM7_SDLViewGetSrcSurface(void *p);
extern void XM7_SDLViewSetSurfaceNum(void *p, int num);

extern void XM7_SDLViewDrawFn(void *p, AG_EventFn fn, const char *fmt, ...);
extern void XM7_SDLViewSetDirty(void *p);

#ifdef __cplusplus
}
#endif
#endif /* __XM7_SDL_VIEW */

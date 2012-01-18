/*	Public domain	*/

/*
 * Implementation of a typical Agar widget which uses surface mappings to
 * efficiently draw surfaces, regardless of the underlying graphics system.
 *
 * If you are not familiar with the way the Agar object system handles
 * inheritance, see demos/objsystem.
 */

#include <agar/core.h>
#include <agar/gui.h>

#include "agar_sdldraw.h"

/*
 * This is a generic constructor function. It is completely optional, but
 * customary of FooNew() functions to allocate, initialize and attach an
 * instance of the class.
 */
XM7_SDLView *XM7_SDLViewNew(void *parent, AG_Surface *src, const char *param)
{
	XM7_SDLView *my;

	/* Create a new instance of the MyWidget class */
	my = malloc(sizeof(XM7_SDLViewNew));
	AG_ObjectInit(my, &XM7_SDLViewClass);

	/* Set some constructor arguments */
	my->param = param;
        my->Surface = src;

	/* Attach the object to the parent (no-op if parent is NULL) */
	AG_ObjectAttach(parent, my);

	return (my);
}

void XM7_SDLViewLinkSurface(void *p, AG_Surface *src)
{
   XM7_SDLView *my = p;
   my->Surface = src;
   my->mySurface = AG_WidgetMapSurfaceNODUP(my, src);
   
}

AG_Surface *XM7_SDLViewSurfaceNew(void *p, int w, int h)
{
   XM7_SDLView *my = p;
   AG_Surface *src;
   AG_PixelFormat fmt;
   
   fmt.BitsPerPixel = 32;
   fmt.BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
   fmt.Rmask = 0x000000ff; // R
   fmt.Gmask = 0x0000ff00; // G
   fmt.Bmask = 0x00ff0000; // B
   fmt.Amask = 0xff000000; // A
#else
   fmt.Rmask = 0x00ff0000; // R
   fmt.Gmask = 0x0000ff00; // G
   fmt.Bmask = 0xff000000; // B
   fmt.Amask = 0x000000ff; // A
#endif
   fmt.Rshift = 0;
   fmt.Gshift = 8;
   fmt.Bshift = 16;
   fmt.Ashift = 24;
   fmt.Rloss = 0;
   fmt.Gloss = 0;
   fmt.Bloss = 0;
   fmt.Aloss = 0;
   fmt.palette = NULL;
   
   
   src = AG_SurfaceNew(AG_SURFACE_PACKED, w, h, &fmt, 0);
   my->mySurface = AG_WidgetMapSurfaceNODUP(my, src);
   if(src != NULL) my->Surface = src;
   return src;
}


void XM7_SDLViewSurfaceDetach(void *p)
{
   XM7_SDLView *my = p;

   if(my->Surface != NULL) {
      AG_WidgetUnmapSurface(my, my->mySurface);
      AG_SurfaceFree(my->Surface);
      my->Surface = NULL;
      my->mySurface = -1;
   }
}

AG_Surface *XM7_SDLViewGetSrcSurface(void *p)
{
   XM7_SDLView *my = p;
   if(my != NULL) {
      return my->Surface;
   }
   return NULL;
}

void XM7_SDLViewUpdateSrc(void *p)
{
   XM7_SDLView *my = p;
   if(my != NULL) {
	AG_WidgetUpdateSurface(my, my->mySurface);
   }
}



/*
 * This function requests a minimal geometry for displaying the widget.
 * It is expected to return the width and height in pixels into r.
 *
 * Note: Some widgets will provide FooSizeHint() functions to allow the
 * programmer to request an initial size in pixels or some other metric
 * FooSizeHint() typically sets some structure variable, which are then
 * used here.
 */
static void SizeRequest(void *p, AG_SizeReq *r)
{
	XM7_SDLView *my = p;
	
	if (my->mySurface == -1) {
		/*
		 * We can use AG_TextSize() to return the dimensions of rendered
		 * text, without rendering it.
		 */
		AG_TextFont(AG_FetchFont(NULL, 24, 0));
		AG_TextSize("Custom widget!", &r->w, &r->h);
	} else {
		/*
		 * We can use the geometry of the rendered surface. The
		 * AGWIDGET_SURFACE() macro returns the AG_Surface given a
		 * Widget surface handle.
		 */
		r->w = AGWIDGET_SURFACE(my,my->mySurface)->w;
		r->h = AGWIDGET_SURFACE(my,my->mySurface)->h;
	}
}

/*
 * This function is called by the parent widget after it decided how much
 * space to allocate to this widget. It is mostly useful to container
 * widgets, but other widgets generally use it to check if the allocated
 * geometry can be handled by Draw().
 */
static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	XM7_SDLView *my = p;

	/* If we return -1, Draw() will not be called. */
	if (a->w < 5 || a->h < 5)
		return (-1);
	
	printf("Allocated %dx%d pixels\n", a->w, a->h);
	return (0);
}

/*
 * Draw function. Invoked from GUI rendering context to draw the widget
 * at its current location. All primitive and surface operations operate
 * on widget coordinates.
 */
static void Draw(void *p)
{
	XM7_SDLView *my = p;
	
	/*
	 * Draw a box spanning the widget area. In order to allow themeing,
	 * you would generally use a STYLE() call here instead, see AG_Style(3)
	 * for more information on styles.
	 */
        
	AG_DrawBox(my,
	    AG_RECT(0, 0, AGWIDGET(my)->w, AGWIDGET(my)->h), 1,
	    agColors[BUTTON_COLOR]);

	/*
	 * Render some text into a new surface. In OpenGL mode, the
	 * AG_WidgetMapSurface() call involves a texture upload.
	 */
	if (my->mySurface == -1) {
		AG_TextFont(AG_FetchFont(NULL, 24, 0));
		my->mySurface = AG_WidgetMapSurface(my,
		    AG_TextRender("Custom widget!"));
	}

	/* Blit the mapped surface at [0,0]. */
	AG_WidgetBlitSurface(my, my->mySurface, 0, 0);
}

/* Mouse motion event handler */
static void MouseMotion(AG_Event *event)
{
	XM7_SDLView *my = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	/* ... */
}

/* Mouse click event handler */
static void MouseButtonDown(AG_Event *event)
{
	XM7_SDLView *my = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	if (button != AG_MOUSE_LEFT) {
		return;
	}
	printf("Click at %d,%d\n", x, y);
	AG_WidgetFocus(my);
}

/* Mouse click event handler */
static void MouseButtonUp(AG_Event *event)
{
	XM7_SDLView *my = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	/* ... */
}

/* Keystroke event handler */
static void KeyDown(AG_Event *event)
{
	XM7_SDLView *my = AG_SELF();
	int keysym = AG_INT(1);

	printf("Keystroke: 0x%x\n", keysym);
}

/* Keystroke event handler */
static void KeyUp(AG_Event *event)
{
	XM7_SDLView *my = AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of the parent classes first.
 */
static void Init(void *obj)
{
	XM7_SDLView *my = obj;

	/* Allow this widget to grab focus. */
	AGWIDGET(my)->flags |= AG_WIDGET_FOCUSABLE;

	/* Initialize instance variables. */
	my->param = "";

	/*
	 * We'll eventually need to create and map a surface, but we cannot
	 * do this from Init(), because it involves texture operations in
	 * GL mode which are thread-unsafe. We wait until Draw() to do that.
	 */
	my->mySurface = -1;

	/*
	 * Map our event handlers. For a list of all meaningful events
	 * we can handle, see AG_Object(3), AG_Widget(3) and AG_Window(3).
	 *
	 * Here we register handlers for the common AG_Window(3) events.
	 */
         my->Surface = NULL;
   
	AG_SetEvent(my, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(my, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(my, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(my, "key-up", KeyUp, NULL);
	AG_SetEvent(my, "key-down", KeyDown, NULL);
}

/*
 * This structure describes our widget class. It inherits from AG_ObjectClass.
 * Any of the function members may be NULL. See AG_Widget(3) for details.
 */
AG_WidgetClass XM7_SDLViewClass = {
	{
		"AG_Widget:XM7_SDLView",	/* Name of class */
		sizeof(XM7_SDLView),	/* Size of structure */
		{ 0,0 },		/* Version for load/save */
		Init,			/* Initialize dataset */
		NULL,			/* Free dataset */
		NULL,			/* Destroy widget */
		NULL,			/* Load widget (for GUI builder) */
		NULL,			/* Save widget (for GUI builder) */
		NULL			/* Edit (for GUI builder) */
	},
	Draw,				/* Render widget */
	SizeRequest,			/* Default size requisition */
	SizeAllocate			/* Size allocation callback */
};

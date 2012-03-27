/*
* Debugger for XM7
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* History:
* 21 Mar,2012 Initial
*/

#include "agar_surfaceconsole.h"
#include "agar_debugger.h"


/*
* WIDGET
*/
extern void DBG_PrintYSum(char *str, int *sum, int totalSum, int width);
extern void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD addr, int sum, int bytes, BOOL AddrFlag);
extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 b);
extern void DBG_Bin2Hex4(char *str, Uint32 b);

void XM7_DbgDumpDetach(void *p)
{
   XM7_DbgDump *my =(XM7_DbgDump *) p;

    if(my == NULL) return;
    if(my->dump != NULL) {
        delete my->dump;
    }
    printf("XM7_DbgDumpDetach()\n");
}

void XM7_DbgDumpMem(void *p, int addr)
{
    int a;
    int Hb,Wb;
    int i,j;
    int wd,hd;
    char *strbuf;
    BYTE *buf;
    unsigned int *ysum;
    unsigned int xsum;
    unsigned int sum;
    XM7_DbgDump *obj;

    obj = (XM7_DbgDump *)p;

    if(obj == NULL) return;
    if(obj->dump == NULL) return;
    if(obj->rb == NULL) return;

    Wb = obj->dump->GetWidth();
    Hb = obj->dump->GetHeight();
    wd = 16;
    hd = 16;
    strbuf = new char [Wb + 4];
    obj->dump->MoveDrawPos(0, 0);
    if(wd >= (Wb / 5)) wd = Wb / 5;
    if(hd >= (Hb - 4)) hd = Hb - 4;
    buf = new BYTE [wd + 2];
    ysum = new unsigned int [hd + 2];


    obj->dump->PutString("ADDR  ");
    for(i = 0; i < wd; i++){
        sprintf(strbuf, "+%x", i);
        obj->dump->PutString(strbuf);
    }
    obj->dump->MoveDrawPos(0,1);
    for(i = 0; i < Wb - 2; i++){
        obj->dump->PutChar('-');
    }
    sum = 0;
    for(i = 0;i < wd; i++){
        ysum[i] = 0;
    }

    for(i = 0;i < hd; i++){
        xsum = 0;
        for(j = 0; j < wd; j++) {
            buf[j] = obj->rb((WORD)((addr + j) & 0xffff));
            sum += (unsigned int)buf[j];
            xsum += (unsigned int)buf[j];
        }
        obj->dump->MoveDrawPos(0, 2 + i);
        DBG_HexDumpMemory(strbuf, buf, (WORD)addr, xsum & 0xff, wd, TRUE);
    }
    // Footer
    obj->dump->MoveDrawPos(0,3+i);
    for(i = 0; i < Wb - 2; i++){
        obj->dump->PutChar('-');
    }
    // Sum
    obj->dump->MoveDrawPos(0, 3+i);
    DBG_PrintYSum(strbuf, (int *)ysum, (int) (sum & 0xff), wd);
    obj->dump->PutString(strbuf);


    delete [] buf;
    delete [] strbuf;
    delete [] ysum;
}


extern "C"{
XM7_DbgDump *XM7_DbgDumpNew(void *parent, BYTE (*rb)(WORD), void (*wb)(WORD, BYTE), int W, int H, const char *arg)
{
	XM7_DbgDump *my;
	if(parent == NULL) return NULL;

	/* Create a new instance of the MyWidget class */
	my =(XM7_DbgDump *)malloc(sizeof(XM7_DbgDump));
	AG_ObjectInit(my, &XM7_DbgDumpClass);
//	AG_ObjectInit(my, NULL);

	/* Set some constructor arguments */
	my->param = arg;
    my->rb = rb;
    my->wb = wb;
    my->addr = 0;
    my->dump = new DumpObject;
	my->forceredraw = 1;
    my->dump->InitConsole(W, H);
	/* Attach the object to the parent (no-op if parent is NULL) */
	AG_ObjectAttach(parent, my);
	return (my);
}


// Resist Draw Function
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
	XM7_DbgDump *my =(XM7_DbgDump *) p;
	AG_SizeAlloc a;
	AG_Surface *s;
	int w, h;

    AG_ObjectLock(my);
	if (my->mySurface == -1) {
		/*
		 * We can use AG_TextSize() to return the dimensions of rendered
		 * text, without rendering it.
		 */
		 s = my->dump->GetScreen();
		 if(s != NULL){
		     w = s->w;
		     h = s->h;
		 } else {
		     w = 0;
		     h = 0;
		 }
	} else {
		/*
		 * We can use the geometry of the rendered surface. The
		 * AGWIDGET_SURFACE() macro returns the AG_Surface given a
		 * Widget surface handle.
		 */
		w = AGWIDGET_SURFACE(my,my->mySurface)->w;
		h = AGWIDGET_SURFACE(my,my->mySurface)->h;
	}
	a.h = h;
	a.w = w;
	my->dump->SizeAlloc(&a);
    AG_ObjectUnlock(my);
}

/*
 * This function is called by the parent widget after it decided how much
 * space to allocate to this widget. It is mostly useful to container
 * widgets, but other widgets generally use it to check if the allocated
 * geometry can be handled by Draw().
 */
static void Draw(void *p);

static int SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	XM7_DbgDump *my =(XM7_DbgDump *)p;

	if(my == NULL) return -1;
    if(my->dump == NULL) return -1;
	/* If we return -1, Draw() will not be called. */
	if (a->w < 5 || a->h < 5)
		return (-1);

    AG_ObjectLock(my);
    my->dump->SizeAlloc((AG_SizeAlloc *)a);
    AG_ObjectUnlock(my);
    Draw(my); // Dirty?
	return (0);
}

/*
 * Draw function. Invoked from GUI rendering context to draw the widget
 * at its current location. All primitive and surface operations operate
 * on widget coordinates.
 */
static void Draw(void *p)
{
	XM7_DbgDump *my = (XM7_DbgDump *)p;
	AG_Surface *s;
	/*
	 * Draw a box spanning the widget area. In order to allow themeing,
	 * you would generally use a STYLE() call here instead, see AG_Style(3)
	 * for more information on styles.
	 */
    AG_ObjectLock(my);
	/*
	 * Render some text into a new surface. In OpenGL mode, the
	 * AG_WidgetMapSurface() call involves a texture upload.
	 */

	/* Blit the mapped surface at [0,0]. */
	if(my->dump != NULL) {
	    my->dump->Draw(my->forceredraw);
	    s = my->dump->GetScreen();
	    if(s != NULL) AG_WidgetBlit(my, s, 0, 0);
	}
    AG_ObjectUnlock(my);
}

/* Mouse motion event handler */
static void MouseMotion(AG_Event *event)
{
	XM7_DbgDump *my = (XM7_DbgDump *)AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);

	/* ... */
}

/* Mouse click event handler */
static void MouseButtonDown(AG_Event *event)
{
	XM7_DbgDump *my = (XM7_DbgDump *)AG_SELF();
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
	XM7_DbgDump *my = (XM7_DbgDump *)AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	/* ... */
}

/* Keystroke event handler */
static void KeyDown(AG_Event *event)
{
	XM7_DbgDump *my = (XM7_DbgDump *)AG_SELF();
	int keysym = AG_INT(1);

//	printf("Keystroke: 0x%x\n", keysym);
}

/* Keystroke event handler */
static void KeyUp(AG_Event *event)
{
	XM7_DbgDump *my = (XM7_DbgDump *)AG_SELF();
	int keysym = AG_INT(1);

	/* ... */
}

/*
 * Initialization routine. Note that the object system will automatically
 * invoke the initialization routines of the parent classes first.
 */
static void Init(void *obj)
{
	XM7_DbgDump *my = (XM7_DbgDump *)obj;

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
//	 if(my->dump == NULL){
//        my->dump = new DumpObject;
//	 }
    my->mySurface = AG_WidgetMapSurfaceNODUP(my, my->dump->GetScreen());

	AG_SetEvent(my, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(my, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(my, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(my, "key-up", KeyUp, NULL);
	AG_SetEvent(my, "key-down", KeyDown, NULL);
}

static void Detach(void *obj)
{
	XM7_DbgDump *my = (XM7_DbgDump *)obj;
    if(my == NULL) return;
    if(my->dump != NULL){
        delete my->dump;
    }
}
/*
 * This structure describes our widget class. It inherits from AG_ObjectClass.
 * Any of the function members may be NULL. See AG_Widget(3) for details.
 */
AG_WidgetClass XM7_DbgDumpClass = {
	{
		"AG_Widget:XM7_DbgDump",	/* Name of class */
		sizeof(XM7_DbgDump),	/* Size of structure */
		{ 0,0 },		/* Version for load/save */
		Init,			/* Initialize dataset */
		Detach,			/* Free dataset */
		NULL,			/* Destroy widget */
		NULL,			/* Load widget (for GUI builder) */
		NULL,			/* Save widget (for GUI builder) */
		NULL			/* Edit (for GUI builder) */
	},
	Draw,				/* Render widget */
	SizeRequest,			/* Default size requisition */
	SizeAllocate			/* Size allocation callback */
};



}

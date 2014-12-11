/*
 * Debugger for XM-7/SDL : Register-Dump
 * (C) 2012 K.Ohta <whatisthis.sowhar@gmail.com>
 * History:
 *        12 Dec 2014 : Branch from agar_debugger.cpp
 */

#include "agar_surfaceconsole.h"
#include "agar_debugger.h"
#include "xm7.h"
#include "agar_surfaceconsole.h"
#include "agar_sdlview.h"

void XM7_ConsoleUpdate(XM7_SDLView *view, DumpObject *cons, BOOL forceredraw)
{
//   forceredraw = TRUE;
   if(cons->Draw(forceredraw) == FALSE) return;
   if(AG_UsingGL(NULL)) {
	 XM7_SDLViewLinkSurface(view, AGWIDGET_SURFACE(view, view->mySurface));
   }
   XM7_SDLViewSetDirty(view);
}

extern "C" {
static void ForceRedraw(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   DumpObject *cons = (DumpObject *)AG_PTR(1);
   XM7_ConsoleUpdate(my, cons, TRUE);
   AG_Redraw(AGWIDGET(my));
}
}


void XM7_ConsoleSetup(XM7_SDLView *view, DumpObject *cons, void *obj, AG_EventFn fn, int w, int h, int flag)
{
    AG_Surface *s;
    AG_SizeAlloc a;
    cons->InitConsole(w, h); // ステータス表示分
    s = cons->GetScreen();
    a.w = s->w;
    a.h = s->h;
    a.x = 0;
    a.y = 0;
    
    XM7_SDLViewDrawFn(view, fn, "%p,%i", (void *)obj, TRUE); //
    XM7_SDLViewLinkSurface(view, s);
    AG_WidgetSizeAlloc(AGWIDGET(view), &a);
//    AG_SetEvent(AGOBJECT(view), "widget-moved", ForceRedraw, "%p", (void *)cons);
    AG_RedrawOnTick(AGWIDGET(view), 100);
//    AG_ObjectLock(AGOBJECT(view));
//    view->forceredraw = 1;
//    AG_ObjectUnlock(AGOBJECT(view));
    XM7_SDLViewSetDirty(view);
}

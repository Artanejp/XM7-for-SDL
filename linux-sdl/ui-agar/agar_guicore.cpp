/*
 * agar_guicore.cpp
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 */
#include <SDL/SDL.h>
#include <libintl.h>
extern "C" {
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
}
#include "xm7.h"
#include "fdc.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#include "agar_toolbox.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"
#include "agar_osd.h"
#else
#include "xm7_sdl.h"
#include "sdl_cfg.h"
#endif

#include "api_kbd.h"
#include "sdl_inifile.h"
#include "api_draw.h"
#include "sdl_cpuid.h"
extern struct XM7_CPUID *pCpuID;

extern "C" {
void InitInstance(void);
extern void OnDestroy(AG_Event *event);
}

Uint32 nDrawTick1D;
BOOL   bResizeGUIFlag;
extern Uint32 nDrawTick1E;

extern void Create_AGMainBar(AG_Widget *Parent);


BOOL EventGuiSingle(AG_Driver *drv, AG_DriverEvent *ev)
{
	int w;
	int h;
	BOOL bi;

	bi = FALSE;
        if(MainWindow != NULL) {
        if(AG_WindowIsFocused(MainWindow)) {
            bi = TRUE;
        }
    } else {
	return FALSE;
    }
   if(drv == NULL) return FALSE;
   if(ev == NULL) return FALSE;
	/* Retrieve the next queued event. */
	switch (ev->type) {
	case AG_DRIVER_KEY_UP:
//        if(bi) {
//            OnKeyReleaseAG(ev->data.key.ks, drv->kbd->modState , ev->data.key.ucs);
//        }
    break;
	case AG_DRIVER_KEY_DOWN:
//        if(bi) {
//            OnKeyPressAG(ev->data.key.ks, drv->kbd->modState, ev->data.key.ucs);
//        }
    break;
	case AG_DRIVER_VIDEORESIZE:
		w = ev->data.videoresize.w;
		h = ev->data.videoresize.h;
		ResizeWindow_Agar2(w, h);
		break;
	default:
		break;
	}
	if (AG_ProcessEvent(drv, ev) == -1) 	return FALSE;
		//	if(drv == NULL) return;
		/* Forward the event to Agar. */
		return TRUE;
}

BOOL EventGUI(AG_Driver *drv)
{
	AG_DriverEvent dev;
	BOOL r;
	if(AG_PendingEvents(drv) <= 0) return FALSE;
	do {
		if (AG_GetNextEvent(drv, &dev) == 1) {
            r = EventGuiSingle(drv, &dev);
    		if(!r) return FALSE;
		}
	} while (AG_PendingEvents(drv) > 0);
	return TRUE;
}


void AGDrawTaskEvent(BOOL flag)
{
   Uint32 nDrawTick2D;
   AG_Window *win;
   AG_Driver *drv;
   Uint32 fps;
   Uint32 oldfps = nDrawFPS;
   bResizeGUIFlag = FALSE;

    if(nDrawFPS > 2) {
#ifdef USE_OPENGL
       if(DrawArea != NULL) {
            AG_RedrawOnTick(DrawArea, 1000 / nDrawFPS);
        } else if(GLDrawArea != NULL){
            AG_RedrawOnTick(GLDrawArea, 1000 / nDrawFPS);
        }
#else
        if(DrawArea != NULL) {
            AG_RedrawOnTick(DrawArea, 1000 / nDrawFPS);
        }
#endif
    }
   for(;;) {
      if(nDrawFPS > 2) {
	 fps = 1000 / nDrawFPS;
      } else {
	 fps = 500;
      }
      if(oldfps != nDrawFPS){ // FPS Change 20120120
	 oldfps = nDrawFPS;
#ifdef USE_OPENGL
	   if(DrawArea != NULL) {
                    AG_RedrawOnTick(DrawArea, 1000 / nDrawFPS);
                } else if(GLDrawArea != NULL){
                    AG_RedrawOnTick(GLDrawArea, 1000 / nDrawFPS);
                }
#else
	   if(DrawArea != NULL) {
	      AG_RedrawOnTick(DrawArea, 1000 / nDrawFPS);
	   }
#endif
      }
      nDrawTick2D = AG_GetTicks();
      if(bResizeGUIFlag) continue;

      if(nDrawTick2D < nDrawTick1D) nDrawTick1D = 0; // オーバーフロー対策
      if((nDrawTick2D - nDrawTick1D) > fps) {
	 // ここにGUIの処理入れる
	 AG_LockVFS(&agDrivers);
	 if (agDriverSw) {
	    drv = &agDriverSw->_inherit;
	    /* With single-window drivers (e.g., sdlfb). */
	    AG_BeginRendering(agDriverSw);
	    AG_FOREACH_WINDOW(win, agDriverSw) {
	       AG_ObjectLock(win);
	       AG_WindowDraw(win);
	       AG_ObjectUnlock(win);
	    }
	    nDrawTick1D = nDrawTick2D;
	    AG_EndRendering(agDriverSw);
	 } else  {
//	    AG_Window *miniwin;
	    /* With multiple-window drivers (e.g., glx). */
	    AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
	       if (!AGDRIVER_MULTIPLE(drv)) {
		  continue;
	       }
	       win = AGDRIVER_MW(drv)->win;
	       if (!win->visible || !win->dirty){
		  continue;
	       }
	       AG_BeginRendering(drv);
	       AG_ObjectLock(win);
	       AG_WindowDraw(win);
	       AG_ObjectUnlock(win);
	       AG_EndRendering(drv);
	    }
	 }
	 
	 AG_UnlockVFS(&agDrivers);
      }
//      if(AG_PendingEvents(NULL) > 0) {
	 if(agDriverSw) { // Single Window
	    drv = &agDriverSw->_inherit;
	    if (AG_PendingEvents(drv) > 0){
	       if(EventSDL(drv) == FALSE) return;
	       //EventGUI(drv);
	    }
	 } else { // Multi windows
	    BOOL b;
	    b = FALSE;
	    EventSDL(NULL);
	    AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver){
	       if (AG_PendingEvents(drv) > 0){
//		  if(EventSDL(drv) == FALSE) {
//		     b = TRUE;
//		     continue;
//		  }
		  if(EventGUI(drv) == FALSE) {
		     b = TRUE;
		     continue;
		  }
	       }
	    }
	    if(b == TRUE) return;
	 }
      
//      }       
      // 20120109 - Timer Event
	 if (AG_TIMEOUTS_QUEUED()){
	    Uint32 tim = 0;
	    tim = AG_GetTicks();
	    AG_ProcessTimeouts(tim);
	    AG_Delay(1);
	 } else {
	    AG_Delay(1);
	 }

      
   }	// Process Event per 1Ticks;
   

}





void ProcessKeyDown(AG_Event *event)
{
	// キーハンドラー
	int sym = AG_INT(1);
	int mod = AG_INT(2);
	Uint32  unicode = (Uint32)AG_ULONG(3);
	OnKeyPressAG(sym, mod, unicode);
}



void ProcessKeyUp(AG_Event *event)
{
	// キーハンドラー
	int sym = AG_INT(1);
	int mod = AG_INT(2);
	Uint32  unicode = (Uint32)AG_ULONG(3);
	OnKeyReleaseAG(sym, mod, unicode);
}

extern void AG_initsub(void);
extern void AG_detachsub(void);
extern void ResizeWindow(int w, int h);
extern Uint32 nDrawTick1;



#ifdef __cplusplus
extern "C" {
#endif



static void InitFont(void)
{
}


void InitInstance(void)
{
	AG_HBox *hb;
        AG_VBox *vb;
	AG_Window *win;
//	AG_Driver *drv;

#ifdef USE_OPENGL
    GLDrawArea = NULL;
#endif /* USE_OPENGL */
    DrawArea = NULL;
    pStatusBar = NULL;

	InitFont();

	//  最初にカスタムウイジェットをつける
    AG_RegisterClass(&XM7_SDLViewClass);
   if(agDriverSw) {
      MainWindow = AG_WindowNew(AG_WINDOW_NOTITLE |  AG_WINDOW_NOBORDERS | AG_WINDOW_KEEPBELOW | AG_WINDOW_NOBACKGROUND | AG_WINDOW_MODKEYEVENTS);
   } else {
      MainWindow = AG_WindowNew(AG_WINDOW_DIALOG | AG_WINDOW_MODKEYEVENTS );
//        MainWindow = AG_WindowNew(AG_WINDOW_NOTITLE |  AG_WINDOW_NOBORDERS | AG_WINDOW_KEEPBELOW | AG_WINDOW_NOBACKGROUND | AG_WINDOW_MODKEYEVENTS);
   }
	AG_WindowSetGeometry (MainWindow, 0, 0 , nDrawWidth, nDrawHeight);
	AG_SetEvent(MainWindow , "window-close", OnDestroy, NULL);
        AG_WindowSetCloseAction(MainWindow, AG_WINDOW_DETACH);
        MenuBar = AG_MenuNew(AGWIDGET(MainWindow), 0);
	Create_AGMainBar(AGWIDGET(NULL));
   	AG_WidgetSetPosition(MenuBar, 0, 0);
	AG_WidgetShow(AGWIDGET(MenuBar));

	AG_WindowShow(MainWindow);
	AG_WindowFocus(MainWindow);
#if USE_OPENGL
    if(AG_UsingGL(NULL) != 0) {
        hb = AG_HBoxNew(AGWIDGET(MainWindow), AG_BOX_HFILL);
          /*
         * OpenGL Capability
         */
        GLDrawArea = AG_GLViewNew(AGWIDGET(hb) , 0);
        GLDrawArea->wid.flags |= AG_WIDGET_CATCH_TAB;
        AG_WidgetSetSize(GLDrawArea, 640,400);
        AG_GLViewSizeHint(GLDrawArea, 640, 400);
        AG_GLViewDrawFn (GLDrawArea, AGEventDrawGL2, NULL);
        AG_GLViewKeydownFn (GLDrawArea, AGEventKeyDownGL, NULL);
        AG_GLViewKeyupFn (GLDrawArea, AGEventKeyUpGL, NULL);
        AG_GLViewScaleFn (GLDrawArea, AGEventScaleGL, NULL);
        //AG_GLViewOverlayFn (GLDrawArea, AGEventOverlayGL, NULL);
        //	AG_GLViewMotionFn(GLDrawArea, AGEventMouseMove_AG_GL, NULL);
	bUseOpenGL = TRUE;
	DrawArea = NULL;
        AG_WidgetShow(GLDrawArea);
        AG_WidgetFocus(AGWIDGET(GLDrawArea));
    } else
#endif /* USE_OPENGL */
     {
        // Non-GL
        hb = AG_HBoxNew(AGWIDGET(MainWindow), AG_BOX_HFILL);
        DrawArea = XM7_SDLViewNew(AGWIDGET(hb), NULL, NULL);
        AGWIDGET(DrawArea)->flags |= AG_WIDGET_CATCH_TAB;
        XM7_SDLViewDrawFn(DrawArea, XM7_SDLViewUpdateSrc, "%p", NULL);
        XM7_SDLViewSurfaceNew(DrawArea, 640, 400);
        AG_SetEvent(DrawArea, "key-up", ProcessKeyUp, NULL);
        AG_SetEvent(DrawArea, "key-down", ProcessKeyDown, NULL);

        AG_WidgetSetSize(DrawArea, 640, 400);
        bUseOpenGL = FALSE;
#ifdef USE_OPENGL
        GLDrawArea = NULL;
#endif /* USE_OPENGL */
        AG_WidgetShow(DrawArea);
        AG_WidgetFocus(AGWIDGET(DrawArea));
        ResizeWindow_Agar2(nDrawWidth, nDrawHeight);
    }
    {
      // hb = AG_HBoxNew(AGWIDGET(MainWindow), 0);
       pStatusBar = AG_HBoxNew(AGWIDGET(MainWindow), AG_BOX_HFILL);
       AG_WidgetSetSize(pStatusBar, 640, 40);
        CreateStatus(AGWIDGET(pStatusBar));
        AG_WidgetShow(pStatusBar);
    }
#if 0 // GUI-DEBUG
     {
     win = AG_GuiDebugger(AGWIDGET(MainWindow));
     AG_WindowShow(win);
    }
#endif
   
}

/*
 *  Update UI
 */
void ui_update(void)
{
	AG_WindowUpdate(MainWindow);
}

#ifdef __cplusplus
}
#endif

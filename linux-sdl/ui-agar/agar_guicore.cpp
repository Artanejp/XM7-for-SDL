/*
 * agar_guicore.cpp
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 */
#include <SDL.h>
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
#else
#include "sdl.h"
#include "sdl_cfg.h"
#endif

#include "sdl_bar.h"
#include "api_kbd.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_inifile.h"
#include "api_draw.h"
//#include "sdl_gtkdlg.h"

extern "C" {
void InitInstance(void);
void OnDestroy(AG_Event *event);
void OnDestroy2(void);
}

Uint32 nDrawTick1D;
extern Uint32 nDrawTick1E;

extern void Create_AGMainBar(AG_Widget *Parent);
extern void CreateStatus(AG_Widget *parent);
extern void DestroyStatus(void);

extern void DrawOSDEv(AG_Event *e);

//static BOOL bKeyboardSnooped;





void KeyBoardSnoop(BOOL Flag)
{
//	bKeyboardSnooped = Flag;
}

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
	/* Retrieve the next queued event. */
	switch (ev->type) {
	case AG_DRIVER_KEY_UP:
        if(bi) {
            OnKeyReleaseAG(ev->data.key.ks, drv->kbd->modState , ev->data.key.ucs);
        }
    break;
	case AG_DRIVER_KEY_DOWN:
        if(bi) {
            OnKeyPressAG(ev->data.key.ks, drv->kbd->modState, ev->data.key.ucs);
        }
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

    if(nDrawFPS > 2) {
        if(DrawArea != NULL) {
            AG_RedrawOnTick(DrawArea, 1000 / nDrawFPS);
        } else if(GLDrawArea != NULL){
            AG_RedrawOnTick(GLDrawArea, 1000 / nDrawFPS);
        }
    }

	for(;;) {
		if(nDrawFPS > 2) {
			fps = 1000 / nDrawFPS;
		} else {
			fps = 500;
		}
        if(oldfps != nDrawFPS){ // FPS Change 20120120
                oldfps = nDrawFPS;
                if(DrawArea != NULL) {
                    AG_RedrawOnTick(DrawArea, 1000 / nDrawFPS);
                } else if(GLDrawArea != NULL){
                    AG_RedrawOnTick(GLDrawArea, 1000 / nDrawFPS);
                }
        }
		nDrawTick2D = AG_GetTicks();
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
			/* With multiple-window drivers (e.g., glx). */
			AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver)
			  {
			     if (!AGDRIVER_MULTIPLE(drv)) {
				  continue;
			       }

			     win = AGDRIVER_MW(drv)->win;
			     if (win->visible) {
				  AG_BeginRendering(drv);
				  AG_ObjectLock(win);
				  AG_WindowDraw(win);
				  AG_ObjectUnlock(win);
				  AG_EndRendering(drv);
			       }
			  }
		     }
		   AG_UnlockVFS(&agDrivers);
		}	// Process Event per 1Ticks;
        if(agDriverSw) { // Single Window
	   drv = &agDriverSw->_inherit;

            if (AG_PendingEvents(drv) > 0){
                if(EventSDL(drv) == FALSE) return;
//                if(EventGUI(drv) == FALSE) return;
            }
        } else { // Multi windows
       		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver){
                    if (AG_PendingEvents(drv) > 0){
                    if(EventSDL(drv) == FALSE) return;
                    if(EventGUI(drv) == FALSE) return;
                }
             }
        }
		// 20120109 - Timer Event
        if (AG_TIMEOUTS_QUEUED())
                AG_ProcessTimeouts(AG_GetTicks());
		AG_Delay(1);
	}
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

extern "C" {
int  RootVideoWidth;
int  RootVideoHeight;
}

void MainLoop(int argc, char *argv[])
{
	int c;
	char *drivers = NULL;
	char *optArg;
        char strbuf[2048];
	const SDL_VideoInfo *inf;
	SDL_Surface *s;

//	AG_InitCore("xm7", AG_VERBOSE | AG_NO_CFG_AUTOLOAD);
	AG_InitCore("xm7", AG_VERBOSE);

	AG_ConfigLoad();
    AG_SetInt(agConfig, "font.size", UI_PT);

    while ((c = AG_Getopt(argc, argv, "?fWd:w:h:T:t:c:T:F:S:o:O:l:s:i:", &optArg, NULL))
          != -1) {
              switch (c) {
              case 'd':
                      drivers = optArg;
                      break;
              case 'f':
                      /* Force full screen */
                      AG_SetBool(agConfig, "view.full-screen", 1);
                      break;
              case 'W':
                      /* Force Window */
                      AG_SetBool(agConfig, "view.full-screen", 0);
                      break;
              case 'T':
                      /* Set an alternate font directory */
                      AG_SetString(agConfig, "font-path", optArg);
                      break;
              case 'F':
                      /* Set an alternate font face */
                      AG_SetString(agConfig, "font.face", optArg);
                      break;
              case 'S':
                  /* Set an alternate font face */
                  AG_SetInt(agConfig, "font.size", atoi(optArg));
                  break;
              case 'o':
                  /* Set an alternate font face */
                  AG_SetString(agConfig, "osdfont.face", optArg);
                  break;
              case 'O':
                  /* Set an alternate font face */
                  AG_SetInt(agConfig, "osdfont.size", atoi(optArg));
                  break;
              case 'l':
                  /* Set an alternate font face */
                  AG_SetString(agConfig, "load-path", optArg);
                  break;
              case 's':
                  /* Set an alternate font face */
                  AG_SetString(agConfig, "save-path", optArg);
                  break;
              case 'i':
                  /* Set an alternate font face */
                  AG_SetString(agConfig, "save-path", optArg);
                  AG_SetString(agConfig, "load-path", optArg);
                  break;
              case 't':
                  /* Change the default font */
                  AG_TextParseFontSpec(optArg);
                  break;
          case '?':
          default:
                  printf("%s [-v] [-f|-W] [-d driver] [-r fps] [-t fontspec] "
                         "[-w width] [-h height] "
                	 "[-F font.face] [-S font.size]"
			 "[-o osd-font.face] [-O osd-font.size]"
			 "[-s SavePath] [-l LoadPath] "
                         "[-T font-path]\n\n"
			 "Usage:\n"
			 "-f : FullScreen\n-W:Window Mode\n",
                         agProgName);
                  exit(0);
          }
    }
    AG_GetString(agConfig, "font.face", strbuf, 511);
    if(strlen(strbuf) <= 0)
    {
        AG_SetString(agConfig, "font.face", UI_FONT);
    }

    AG_GetString(agConfig, "font-path", strbuf, 2047);
    if(strlen(strbuf) <= 0)
    {
     AG_PrtString(agConfig, "font-path", "%s:%s/.xm7:%s:.", getenv("HOME"), getenv("HOME"), FONTPATH);
    }

    stopreq_flag = FALSE;
    run_flag = TRUE;
    // Debug
#ifdef _XM7_FB_DEBUG
    drivers = "sdlgl:width=1280:height=880:depth=32";
//    drivers = "glx";
#endif
	/*
	 * Agar のメインループに入る
	 */
//    SDL_Init(SDL_INIT_VIDEO);

    if(drivers == NULL)  {
    	AG_InitVideo(640, 480, 32, AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF | AG_VIDEO_ASYNCBLIT |
    			AG_VIDEO_RESIZABLE | AG_VIDEO_OPENGL_OR_SDL );
    } else {
        if (AG_InitGraphics(drivers) == -1) {
                fprintf(stderr, "%s\n", AG_GetError());
                return;
        }
    }
    OnCreate((AG_Widget *)NULL);
	InitInstance();
	stopreq_flag = FALSE;
	run_flag = TRUE;
	AG_DrawInitsub();

	inf = SDL_GetVideoInfo();
    if(inf != NULL) {
	   RootVideoWidth = inf->current_w;
	   RootVideoHeight = inf->current_h;
	} else {
	   RootVideoWidth = 640;
	   RootVideoHeight = 400;
	}

//	ResizeWindow_Agar(640, 400);
	newResize = FALSE;
	nDrawTick1D = AG_GetTicks();
	nDrawTick1E = AG_GetTicks();
	ResizeWindow_Agar(nDrawWidth, nDrawHeight);
	AGDrawTaskEvent(TRUE);
//	AG_Quit();
}



void Create_DebugMenu(void)
{

}

void Create_ToolsMenu(void)
{

}

void Create_HelpMenu(void)
{

}

void Create_AboutMenu(void)
{

}


void Destroy_AGMainBar(void)
{

}

#ifdef __cplusplus
extern "C" {
#endif


void OnDestroy2(void)
{
	OnDestroy(NULL);
}


void OnDestroy(AG_Event *event)
{

        /*
         * サウンド停止
         */
       StopSnd();
       /*
        * コンポーネント クリーンアップ
        */
#ifdef FDDSND
        CleanFDDSnd();
#endif				/*  */
        CleanSch();
        CleanKbd();
        CleanSnd();
        DestroyStatus();

        CleanDraw();
        SaveCfg();
    	AG_ConfigSave();

        /*
         * 仮想マシン クリーンアップ
         */
        system_cleanup();
        AG_Quit();
}

static void InitFont(void)
{
}

void InitInstance(void)
{
	AG_HBox *hb;
    AG_VBox *vb;
	AG_Window *win;
//	AG_Driver *drv;


    GLDrawArea = NULL;
    DrawArea = NULL;

	InitFont();

	//  最初にカスタムウイジェットをつける
    AG_RegisterClass(&XM7_SDLViewClass);
	if(agDriverSw) {
        MainWindow = AG_WindowNew(AG_WINDOW_NOTITLE |  AG_WINDOW_NOBORDERS | AG_WINDOW_KEEPBELOW | AG_WINDOW_NOBACKGROUND | AG_WINDOW_MODKEYEVENTS);
	} else {
        MainWindow = AG_WindowNew(AG_WINDOW_DIALOG );
	}
//	AG_WindowSetGeometry (MainWindow, 0, 0 , 640, 480);
	AG_WindowSetGeometry (MainWindow, 0, 0 , nDrawWidth, nDrawHeight);
	AG_SetEvent(MainWindow , "window-close", OnDestroy, NULL);
    AG_WindowSetCloseAction(MainWindow, AG_WINDOW_DETACH);
//    hb = AG_HBoxNew(AGWIDGET(MainWindow), 0);
//    vb = AG_VBoxNew(AGWIDGET(MainWindow), 0);
//   MenuBar = AG_MenuNew(AGWIDGET(hb), 0);
    MenuBar = AG_MenuNew(AGWIDGET(MainWindow), 0);
	Create_AGMainBar(AGWIDGET(NULL));
   	AG_WidgetSetPosition(MenuBar, 0, 0);
	AG_WidgetShow(AGWIDGET(MenuBar));

	AG_WindowShow(MainWindow);
	AG_WindowFocus(MainWindow);

    if(AG_UsingGL(NULL) != 0) {
          /*
         * OpenGL Capability
         */
//        GLDrawArea = AG_GLViewNew(AGWIDGET(hb) , 0);
        GLDrawArea = AG_GLViewNew(AGWIDGET(MainWindow) , 0);
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
    } else {
        // Non-GL
//        DrawArea = XM7_SDLViewNew(AGWIDGET(hb), NULL, NULL);
        DrawArea = XM7_SDLViewNew(AGWIDGET(MainWindow), NULL, NULL);
        XM7_SDLViewDrawFn(DrawArea, XM7_SDLViewUpdateSrc, NULL);
        XM7_SDLViewSurfaceNew(DrawArea, 640, 400);
        AG_WidgetSetSize(DrawArea, 640, 400);
        bUseOpenGL = FALSE;
        GLDrawArea = NULL;
	    AG_WidgetShow(DrawArea);
        AG_WidgetFocus(AGWIDGET(DrawArea));
        ResizeWindow_Agar2(nDrawWidth, nDrawHeight);
    }
    {
//        hb = AG_HBoxNew(AGWIDGET(MainWindow), 0);
        pStatusBar = AG_BoxNewHoriz(AGWIDGET(MainWindow), AG_BOX_HFILL);
        CreateStatus(AGWIDGET(pStatusBar));
        AG_WidgetShow(pStatusBar);
    }

	win = AG_GuiDebugger();
    AG_WindowShow(win);

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

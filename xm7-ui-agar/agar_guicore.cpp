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
#include "agar_toolbox.h"
#include "agar_gldraw.h"

extern "C" {
void InitInstance(void);
void OnDestroy(AG_Event *event);
void OnDestroy2(void);
extern void InitGL(int w, int h);

extern AG_Window *MainWindow;
extern AG_Menu *MenuBar;
extern AG_GLView *DrawArea;
extern AG_GLView *OsdArea;
}

Uint32 nDrawTick1D;
extern Uint32 nDrawTick1E;

extern void Create_AGMainBar(AG_Widget *Parent);
extern void CreateStatus(void);
extern int DrawThreadMain(void *);
extern void DestroyStatus(void);

extern void DrawOSDEv(AG_Event *e);

static BOOL bKeyboardSnooped;





void KeyBoardSnoop(BOOL Flag)
{
	bKeyboardSnooped = Flag;
}

BOOL EventGuiSingle(AG_Driver *drv, AG_DriverEvent *ev)
{
		/* Retrieve the next queued event. */
			switch (ev->type) {
			case AG_DRIVER_KEY_UP:
				if(	!bKeyboardSnooped) {
					OnKeyReleaseAG(ev->data.key.ks, 0, ev->data.key.ucs);
				}
				break;
			case AG_DRIVER_KEY_DOWN:
				if(!bKeyboardSnooped) {
					OnKeyPressAG(ev->data.key.ks, 0, ev->data.key.ucs);
				}
				break;
			case AG_DRIVER_VIDEORESIZE:
				nDrawWidth = ev->data.videoresize.w;
				nDrawHeight = ev->data.videoresize.h;
				if(nDrawHeight < 50) {
					nDrawHeight = 50;
				}
				ResizeWindow_Agar2(nDrawWidth, nDrawHeight);
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

	for(;;) {
		if(nDrawFPS > 2) {
			fps = 1000 / nDrawFPS;
		} else {
			fps = 500;
		}
		if(agDriverSw) {
			drv = &agDriverSw->_inherit;
		}
		if(drv == NULL) {
			AG_Delay(10);
			continue;
		}

		nDrawTick2D = AG_GetTicks();
		if(nDrawTick2D < nDrawTick1D) nDrawTick1D = 0; // オーバーフロー対策
		if((nDrawTick2D - nDrawTick1D) > fps) {
			AGDrawTaskMain();
			// ここにGUIの処理入れる
			AG_LockVFS(&agDrivers);
			if (agDriverSw) {
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
		}	else if (AG_PendingEvents(drv) > 0){
			AGDrawTaskMain();
			if(EventSDL(drv) == FALSE) return;
			if(EventGUI(drv) == FALSE) return;
		}
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
	const SDL_VideoInfo *inf;
	SDL_Surface *s;

//	AG_InitCore("xm7", AG_VERBOSE | AG_NO_CFG_AUTOLOAD);
	AG_InitCore("xm7", AG_VERBOSE);

	AG_ConfigLoad();
    AG_PrtString(agConfig, "font-path", "%s:%s/.xm7:%s:.", getenv("HOME"), getenv("HOME"), FONTPATH);
    AG_SetString(agConfig, "font.face", UI_FONT);
    AG_SetInt(agConfig, "font.size", UI_PT);

    while ((c = AG_Getopt(argc, argv, "?d:w:h:ft:T:c:T:F:S:l:s:i:", &optArg, NULL))
          != -1) {
              switch (c) {
              case 'd':
                      drivers = optArg;
                      break;
              case 'f':
                      /* Force full screen */
                      AG_SetBool(agConfig, "view.full-screen", 1);
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
                  AG_SetString(agConfig, "font.size", optArg);
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
                  printf("%s [-vgsDdfR] [-d driver] [-r fps] [-t fontspec] "
                         "[-w width] [-h height] "
                		  "[-F font.face] [-S font.size] "
                         "[-T font-path]\n",
                         agProgName);
                  exit(0);
          }
    }
    stopreq_flag = FALSE;
    run_flag = TRUE;
	/*
	 * Agar のメインループに入る
	 */
//    SDL_Init(SDL_INIT_VIDEO);

    if(drivers == NULL)  {
    	AG_InitVideo(640, 480, 32, AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF |
    			AG_VIDEO_RESIZABLE | AG_VIDEO_OPENGL_OR_SDL );
    } else {
        if (AG_InitGraphics(drivers) == -1) {
                fprintf(stderr, "%s\n", AG_GetError());
                return;
        }
    }
	InitGL(640, 400);
   OnCreate((AG_Widget *)NULL);
	InitInstance();
	bKeyboardSnooped = FALSE;
	stopreq_flag = FALSE;
	run_flag = TRUE;
	AG_initsub();
   
	inf = SDL_GetVideoInfo();
	RootVideoWidth = inf->current_w;
	RootVideoHeight = inf->current_h;
   
	ResizeWindow_Agar(nDrawWidth, nDrawHeight);
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
        AG_QuitGUI();
}

static void InitFont(void)
{
}

void InitInstance(void)
{
	AG_Box *hb;
	AG_Window *win;

	InitFont();
	MenuBar = AG_MenuNewGlobal(AG_MENU_HFILL);
	Create_AGMainBar(AGWIDGET(NULL));
        MainWindow = AG_WindowNew(AG_WINDOW_NOTITLE |  AG_WINDOW_NOBORDERS | AG_WINDOW_NOBACKGROUND);
//        MainWindow = AG_WindowNew(AG_WINDOW_NOTITLE | AG_WINDOW_NOBORDERS );
	AG_WindowSetGeometry (MainWindow, 0, 0, 640, 440);
	AG_SetEvent(MainWindow , "window-close", OnDestroy, NULL);
   
//   	MenuBar = AG_MenuNewGlobal(AG_MENU_HFILL);
//	Create_AGMainBar(AGWIDGET(MainWindow));
//	AG_WidgetSetPosition(MenuBar, 0, 0);

	DrawArea = AG_GLViewNew(AGWIDGET(MainWindow) , 0);
        AG_WidgetSetSize(DrawArea, 640,440);
	AG_GLViewSizeHint(DrawArea, 640, 440);
	AG_WidgetSetPosition(DrawArea, 0, 0);
	AG_GLViewDrawFn (DrawArea, AGEventDrawGL, NULL);
	AG_GLViewScaleFn (DrawArea, AGEventScaleGL, NULL);
	AG_GLViewOverlayFn (DrawArea, AGEventOverlayGL, NULL);
//	AG_GLViewMotionFn(DrawArea, AGEventMouseMove_AG_GL, NULL);
	CreateStatus();
//	AG_WidgetDisable(AGWIDGET(DrawArea));
	AG_WidgetShow(DrawArea);
	AG_WindowShow(MainWindow);

//	win = AG_GuiDebugger();
//    AG_WindowShow(win);
	AG_WidgetShow(AGWIDGET(MenuBar));
	AG_WidgetFocus(AGWIDGET(MenuBar));
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

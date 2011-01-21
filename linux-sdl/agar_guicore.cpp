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

extern void Create_AGMainBar(AG_Widget *Parent);
extern void CreateStatus(void);
extern int DrawThreadMain(void *);
extern void AGEventDrawGL(AG_Event *event);
extern void AGEventScaleGL(AG_Event *event);
extern void AGEventOverlayGL(AG_Event *event);
extern void DestroyStatus(void);

extern void DrawOSDEv(AG_Event *e);

static BOOL bKeyboardSnooped;





void KeyBoardSnoop(BOOL Flag)
{
	bKeyboardSnooped = Flag;
}

void EventGuiSingle(AG_Driver *drv, AG_DriverEvent *ev)
{
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
		newDrawWidth = ev->data.videoresize.w;
		newDrawHeight = ev->data.videoresize.h;
		if(newDrawHeight < 50) {
			newDrawHeight = 50;
		}
		newResize = TRUE;
		break;
	default:
		break;
	}

	/* Forward the event to Agar. */
	if (AG_ProcessEvent(drv, ev) == -1)
		return;

}

void EventGUI(AG_Driver *drv)
{
	AG_DriverEvent ev;

	if(drv == NULL) return;
	 if (AG_PendingEvents(drv) > 0) {
				/*
				 * Case 2: There are events waiting to be processed.
				 */
				do {
					/* Retrieve the next queued event. */
					if (AG_GetNextEvent(drv, &ev) == 1) {
						EventGuiSingle(drv, &ev);
					}
				} while (AG_PendingEvents(drv) > 0);
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

void MainLoop(int argc, char *argv[])
{
	int c;
	char *drivers = NULL;
	char *optArg;

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

    if(drivers == NULL)  {
    	AG_InitVideo(640, 480, 32, AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF |
    			AG_VIDEO_RESIZABLE | AG_VIDEO_OPENGL_OR_SDL );
    } else {
        if (AG_InitGraphics(drivers) == -1) {
                fprintf(stderr, "%s\n", AG_GetError());
                return;
        }
    }
    OnCreate((AG_Widget *)NULL);
	InitInstance();
	bKeyboardSnooped = FALSE;
	stopreq_flag = FALSE;
	run_flag = TRUE;
//	DrawThreadMain(NULL);
	AG_initsub();
	ResizeWindow(640,480);

	nDrawTick1 = AG_GetTicks();
	while(1) {
	AG_Delay(5);
	AGDrawTaskEvent(TRUE);
		if(newResize) {
			nDrawWidth = newDrawWidth;
			nDrawHeight = newDrawHeight;
			ResizeWindow_Agar(nDrawWidth, nDrawHeight);
			newResize = FALSE;
			if(DrawArea) {
				AG_WidgetSetSize(AGWIDGET(DrawArea), newDrawWidth, newDrawHeight);
			}
		}
	}
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
	AG_Box *hb2;
	InitFont();
	MainWindow = AG_WindowNew(AG_WINDOW_NOTITLE |  AG_WINDOW_NOBORDERS | AG_WINDOW_NOBACKGROUND);
	AG_WindowSetGeometry (MainWindow, 0, 0, 640, 480);

    hb = AG_BoxNewHoriz(MainWindow, AG_BOX_HFILL);
    AG_WidgetSetSize(MainWindow, 640,480);

    Create_AGMainBar(AGWIDGET(hb));
    AG_SetEvent(MainWindow , "window-close", OnDestroy, NULL);
    AG_AtExitFunc(OnDestroy2);
	AG_WidgetSetSize(MenuBar, 640, 32);
	AG_WidgetEnable(AGWIDGET(MenuBar));

    hb2 = AG_BoxNewHoriz(MainWindow, AG_BOX_HFILL);
    AG_WidgetSetSize(hb2, 640,400);
	AG_WidgetEnable(hb2);

	DrawArea = AG_GLViewNew(AGWIDGET(hb2) , AG_GLVIEW_EXPAND);
	AG_WidgetEnable(DrawArea);
	AG_GLViewSizeHint(DrawArea, 640, 480);
	AG_WidgetSetPosition(DrawArea, 0, 32);
	AG_GLViewDrawFn (DrawArea, AGEventDrawGL, NULL);
	AG_GLViewScaleFn (DrawArea, AGEventScaleGL, NULL);
//	AG_SetEvent(DrawArea, "key-down" , ProcessKeyDown, NULL);
    hb2 = AG_BoxNewHoriz(MainWindow, AG_BOX_HFILL);
    AG_WidgetSetSize(hb2, 640,40);
	AG_WidgetEnable(hb2);
	OsdArea = AG_GLViewNew(AGWIDGET(hb2) , AG_GLVIEW_EXPAND);
	AG_WidgetEnable(OsdArea);
	AG_GLViewSizeHint(OsdArea, 640, 40);
	AG_WidgetSetPosition(OsdArea, 0, 0);
	AG_GLViewDrawFn (OsdArea, DrawOSDEv, NULL);
	CreateStatus();
	InitGL(640, 480);
//	if(MenuBar != NULL) {
//		AG_WidgetFocus(AGWIDGET(MenuBar));
//	}
	AG_WindowShow(MainWindow);
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

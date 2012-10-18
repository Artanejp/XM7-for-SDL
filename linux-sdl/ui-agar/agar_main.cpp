/*
 * agar_main.cpp
 *  AGAR Toolkit用メイン
 *  Created on: 2010/11/08
 *      Author: K.Ohta <whatisthis.soiwhat@gmail.com>
 */

#include <SDL/SDL.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <locale.h>
#include <libintl.h>

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>

#include "xm7.h"
#include "mouse.h"
#include "tapelp.h"
#include "keyboard.h"
#include "agar_xm7.h"
#include "agar_osd.h"

#include "api_draw.h"
#include "api_kbd.h"
#include "api_js.h"
#include "api_mouse.h"
#include "api_snd.h"

#include "sdl_sch.h"

#include "agar_cfg.h"
#include "agar_draw.h"
#include "agar_gldraw.h"

#include "sdl_inifile.h"
#include "sdl_cpuid.h"

extern Uint32 nDrawTick1D;
extern Uint32 nDrawTick1E;
extern void Detach_DebugMenu(void);


extern "C" {
extern void InitInstance(void);
void OnDestroy(AG_Event *event);

   
/*
 *  グローバル ワーク
 */
char            ModuleDir[MAXPATHLEN];	/* XM7実行モジュールディレクトリ  */

SDL_Surface     *drawArea = NULL;       /* スクリーン描画エリア */
int             nErrorCode;             /* エラーコード */
BOOL            bMenuLoop;	      /* メニューループ中 */
BOOL            bCloseReq;              /* 終了要求フラグ */
BOOL            bSync;	      /* 実行に同期 */
BOOL            bSyncDisasm[2];         /* 逆アセンブルをPCに同期 */
BOOL            bActivate;              /* アクティベートフラグ */
int             nAppIcon;               /* アイコン番号(1,2,3) */
BOOL            bMMXflag;               /* MMXサポートフラグ(未使用) */
BOOL            bCMOVflag;              /* CMOVサポートフラグ(現状未使用) */
struct  XM7_CPUID *pCpuID;           /* CPUフラグ */

#if ((XM7_VER <= 2) && defined(FMTV151))
BOOL            bFMTV151;               /* チャンネルコールフラグ */
#endif				/*  */

}

static AG_Mutex VMMutex;


/*-[ 同期 ]-----------------------------------------------------------------*/

extern "C" {


int  RootVideoWidth;
int  RootVideoHeight;

   
/*
 *  VMをロック
*/
void LockVM(void)
{
//	if(VMMutex == NULL) return;
	AG_MutexLock(&VMMutex);
}
/*
 *  VMをアンロック
 */
void
UnlockVM(void)
{
//	if(VMMutex == NULL) return;
	AG_MutexUnlock(&VMMutex);
}
/*-[ ドローウインドウ ]-----------------------------------------------------*/


void CreateDraw(void)
{

}




/*-[ メインウインドウ ]-----------------------------------------------------*/

    /*
     *  ウインドウ作成
     */
void OnCreate(AG_Widget *parent)
{
        BOOL        flag;
/*
 * ワークエリア初期化
 */
        nErrorCode = 0;
        bMenuLoop = FALSE;
        bCloseReq = FALSE;
        bSync = TRUE;
        bSyncDisasm[0] = TRUE;
        bSyncDisasm[1] = TRUE;
        bActivate = FALSE;
        AG_MutexInit(&VMMutex);
/*
 * コンポーネント初期化
 */
        LoadCfg();
        InitDraw();
        InitSnd();
        InitKbd();
        InitJoy();
        InitSch();
//        CreateStatus();

/*
 * 仮想マシン初期化
 */
        if (!system_init()) {
                nErrorCode = 1;
                return;
        }
/*
 * 直後、リセット
 */
        ApplyCfg();
        system_reset();

/*
 * コンポーネントセレクト
 */
        flag = TRUE;
        if (!SelectDraw()) {
                flag = FALSE;
        }
        if (!SelectSnd()) {
                flag = FALSE;
        }
        if (!SelectKbd()) {
                flag = FALSE;
        }
        if (!SelectSch()) {
                flag = FALSE;
        }
        PaintStatus();

/*
 * エラーコードをセットさせ、スタート
 */
        if (!flag) {
                nErrorCode = 2;
        }
}

void OnDestroy(AG_Event *event)
{
   // 20120610 GUI関連処理
   Detach_DebugMenu();
        /*
         * サウンド停止
         */
       StopSnd();
        SaveCfg();
    	AG_ConfigSave();
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

        /*
         * 仮想マシン クリーンアップ
         */
        if(pCpuID != NULL) {
	   detachCpuID(pCpuID);
	   pCpuID = NULL;
	}

        system_cleanup();
        AG_MutexDestroy(&VMMutex);
//	AG_Destroy();
        AG_Quit();

}

}

extern void OnPushCancel(AG_Event *event);
static void ErrorPopup(char *message)
{
   AG_Window *win;
   win = AG_WindowNew(0);
   if(win == NULL) {
        if(message == NULL) return;
	fprintf(stderr,"ERR: %s\n", message);
        return;
   } else {
      AG_VBox *vb;
      AG_Textbox *tb;
        vb = AG_VBoxNew(AGWIDGET(win), AG_HBOX_HFILL);
        if(message != NULL) tb = AG_TextboxNew(hb, AG_TEXTBOX_MULTILINE, "%s", message);
        AG_ButtonNewFn(AGWIDGET(vb), 0, gettext("Close"), OnPushCancel, NULL);
   }
   
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
	AG_InitCore("xm7", AG_VERBOSE | AG_CREATE_DATADIR);

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

//    AG_GetString(agConfig, "font-path", strbuf, 2047);
//    if(strlen(strbuf) <= 0)
    {
     AG_PrtString(agConfig, "font-path", "%s/.fonts:%s:%s/.xm7:%s:.", 
		  getenv("HOME"), getenv("HOME"), getenv("HOME"), FONTPATH);
    }

    stopreq_flag = FALSE;
    run_flag = TRUE;
    // Debug
#ifdef _XM7_FB_DEBUG
drivers = "sdlfb:width=1280:height=880:depth=32";
//    drivers = "glx";
#endif
	/*
	 * Agar のメインループに入る
	 */
    SDL_Init(SDL_INIT_VIDEO);

    if(drivers == NULL)  {
#ifdef USE_OPENGL
       if(AG_InitGraphics(NULL) == -1){
                fprintf(stderr, "%s\n", AG_GetError());
                return;
        }
#else
       if(AG_InitGraphics("cocoa,sdlfb") == -1){
                fprintf(stderr, "%s\n", AG_GetError());
                return;
        }
#endif
    } else {
        if (AG_InitGraphics(drivers) == -1) {
                fprintf(stderr, "%s\n", AG_GetError());
                return;
        }
    }
    SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO);
    
    OnCreate((AG_Widget *)NULL);
    InitInstance();
    switch(nErrorCode) 
     {
      case 0:
	break;
      case 1:
	ErrorPopup(gettext("Error init VM.\nPlease check ROMS and system-memory.\n"));
	break;
      case 2:
	ErrorPopup(gettext("Error init Emulator.\nPlease check window system, display and more.\n"));
	break;
      default:
	ErrorPopup(gettext("Unknown error on setup.\nPlease email to author."));
	break;
     }
       
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
	nDrawTick1E = nDrawTick1D;

	ResizeWindow_Agar(nDrawWidth, nDrawHeight);
	AGDrawTaskEvent(TRUE);
//	AG_Quit();
}

/*
 * Main Function
 * (C) 2012-06-21 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: CC-BY-SA
 * History:
 * 2012-06-21 Moved from ui-agar/agar_main.cpp
 */
 #include <SDL.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#ifdef _WINDOWS
#include <windows.h>
#include <tchar.h>
#endif

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


 /*
 *  メイン関数
 */
extern int MainLoop(int argc, char *argv[]);
extern struct  XM7_CPUID *pCpuID;           /* CPUフラグ */
extern "C" {
int main(int argc, char *argv[])
{
   int rgb_size[3];
   int flags;
/*
 * 実行モジュールのディレクトリを取得
 */
        char    *p;
       /*
	* Check CPUID
	*/
       pCpuID = initCpuID();
       p = getenv("HOME");
        if(p == NULL) {
                perror("Warning : Can't get HOME directory...Making ./.xm7/ .");
                strcpy(ModuleDir, "./.xm7/");
        } else {
                strcpy(ModuleDir, p);
                strcat(ModuleDir, "/.xm7/");
        }
        if(opendir(ModuleDir) == NULL) {
#ifdef _WINDOWS
		mkdir(ModuleDir);
#else
        mkdir(ModuleDir, 0777);
#endif
        }
        /* Gettext */
        setlocale(LC_ALL, "");
        bindtextdomain("messages", RSSDIR);
        textdomain("messages");

        //SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_TIMER);

#if ((XM7_VER <= 2) && defined(FMTV151))
        bFMTV151 = TRUE;
#endif				/*  */
/*
 * アプリケーション初期化
 */
        MainLoop(argc, argv);
        return nErrorCode;
}


}

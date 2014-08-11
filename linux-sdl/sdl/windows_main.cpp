/*
 * Main Function
 * (C) 2012-06-21 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: CC-BY-SA
 * History:
 * 2012-06-21 Moved from ui-agar/agar_main.cpp
 */

#include <SDL/SDL.h>
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
#include "agar_logger.h"

#include "sdl_inifile.h"
#include "sdl_cpuid.h"


 /*
 *  メイン関数
 */
extern int MainLoop(int argc, char *argv[]);

extern "C" {
extern struct  XM7_CPUID *pCpuID;           /* CPUフラグ */
BOOL            bLogSTDOUT = FALSE;
BOOL            bLogSYSLOG = FALSE;

   
int main(int argc, char *argv[])
{
   int rgb_size[3];
   int flags;
   char simdstr[512];
   char archstr[32];

/*
 * 実行モジュールのディレクトリを取得
 */
   char    *p;

   int simdid = 0;
       /*
	* Check CPUID
	*/
       pCpuID = initCpuID();
       p = SDL_getenv("HOME");
       if(p == NULL) {
#ifdef _WINDOWS
                perror("Warning : Can't get HOME directory...Making .\\xm7\\ .");
               strcpy(ModuleDir, ".\\xm7\\");
#else
                perror("Warning : Can't get HOME directory...Making ./.xm7/ .");
                strcpy(ModuleDir, "./.xm7/");
#endif
        } else {
                strcpy(ModuleDir, p);
#ifdef _WINDOWS
                strcat(ModuleDir, "\\xm7\\");
#else
                strcat(ModuleDir, "/.xm7/");
#endif
        }
   
        AG_InitCore("xm7", AG_VERBOSE | AG_CREATE_DATADIR);
        AG_ConfigLoad();

        if(AG_GetVariable(agConfig, "logger.syslog", NULL) == NULL) { 
	   AG_SetInt(agConfig, "logger.syslog", FALSE);
	}
        if(AG_GetVariable(agConfig, "logger.stdout", NULL) == NULL) { 
	   AG_SetInt(agConfig, "logger.stdout", TRUE);
	}
        bLogSYSLOG = (BOOL)AG_GetInt(agConfig, "logger.syslog");
        bLogSTDOUT = (BOOL)AG_GetInt(agConfig, "logger.stdout");
        XM7_OpenLog(bLogSYSLOG, bLogSTDOUT); // Write to syslog, console

   strncpy(archstr, "Generic", 30);
#if defined(__x86_64__)
   strncpy(archstr, "amd64", 30);
#endif
#if defined(__i386__)
   strncpy(archstr, "ia32", 30);
#endif
        printf("XM7/SDL %s%s %s %s\n", VERSION, LEVEL, LOCALVER, DATE);
        printf("(C) Ryu Takegami / SDL Version K.Ohta <whatisthis.sowhat@gmail.com>\n");
        printf("Architecture: %s\n", archstr);
        printf(" -? is print help(s).\n");
   
        /* Print SIMD features */ 
        simdstr[0] = '\0';
#if defined(__x86_64__) || defined(__i386__)
        if(pCpuID != NULL) {
	   
	   if(pCpuID->use_mmx) {
	      strcat(simdstr, " MMX");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_mmxext) {
	      strcat(simdstr, " MMXEXT");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse) {
	      strcat(simdstr, " SSE");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse2) {
	      strcat(simdstr, " SSE2");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse3) {
	      strcat(simdstr, " SSE3");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_ssse3) {
	      strcat(simdstr, " SSSE3");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse41) {
	      strcat(simdstr, " SSE4.1");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse42) {
	      strcat(simdstr, " SSE4.2");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_sse4a) {
	      strcat(simdstr, " SSE4a");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_abm) {
	      strcat(simdstr, " ABM");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_avx) {
	      strcat(simdstr, " AVX");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_3dnow) {
	      strcat(simdstr, " 3DNOW");
	      simdid |= 0xffffffff;
	   }
	   if(pCpuID->use_3dnowp) {
	      strcat(simdstr, " 3DNOWP");
	      simdid |= 0xffffffff;
	   }
	   if(simdid == 0) strcpy(simdstr, "NONE");
	} else {
	   strcpy(simdstr, "NONE");
	}
#else // Not ia32 or amd64
	   strcpy(simdstr, "NONE");
#endif
   printf("Supported SIMD: %s\n", simdstr);

   
   XM7_DebugLog(XM7_LOG_INFO, "Start XM7 %s%s %s %s", VERSION, LEVEL, LOCALVER, DATE);
   XM7_DebugLog(XM7_LOG_INFO, "(C) Ryu Takegami / SDL Version K.Ohta");
   XM7_DebugLog(XM7_LOG_INFO, "Architecture: %s", archstr);

   XM7_DebugLog(XM7_LOG_DEBUG, "Start XM7 %s%s %s %s", VERSION, LEVEL, LOCALVER, DATE);
   XM7_DebugLog(XM7_LOG_DEBUG, "(C) Ryu Takegami / SDL Version K.Ohta");
   XM7_DebugLog(XM7_LOG_DEBUG, "Architecture: %s", archstr);
   XM7_DebugLog(XM7_LOG_DEBUG, "Supported SIMD: %s", simdstr);
   XM7_DebugLog(XM7_LOG_DEBUG, " -? is print help(s).");
   XM7_DebugLog(XM7_LOG_DEBUG, "Moduledir = %s home = %s", ModuleDir, p); // Debug

   
   if(opendir(ModuleDir) == NULL) {
#ifdef _WINDOWS
	   mkdir(ModuleDir);
#else
	   mkdir(ModuleDir, 0777);
#endif
	   XM7_DebugLog(XM7_LOG_DEBUG, "Created directory: %s", ModuleDir);
        }
        /* Gettext */
        setlocale(LC_ALL, "");
        bindtextdomain("messages", RSSDIR);
        textdomain("messages");
        XM7_DebugLog(XM7_LOG_DEBUG, "I18N via gettext initialized.");
        XM7_DebugLog(XM7_LOG_DEBUG, "I18N resource dir: %s", RSSDIR);
        

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

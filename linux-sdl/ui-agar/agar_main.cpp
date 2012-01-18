/*
 * agar_main.cpp
 *  AGAR Toolkit用メイン
 *  Created on: 2010/11/08
 *      Author: K.Ohta <whatisthis.soiwhat@gmail.com>
 */

#include <SDL.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>

#include "xm7.h"
#include "mouse.h"
#include "tapelp.h"
#include "keyboard.h"
#include "agar_xm7.h"
#include "sdl_bar.h"
#include "api_draw.h"
#include "api_kbd.h"
#include "api_js.h"
#include "api_mouse.h"

#include "sdl_sch.h"
#include "sdl_snd.h"
#include "agar_cfg.h"
#include "agar_draw.h"
#include "agar_gldraw.h"

#include "sdl_inifile.h"

#ifdef __cplusplus
extern "C" {
#endif
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

#if ((XM7_VER <= 2) && defined(FMTV151))
BOOL            bFMTV151;               /* チャンネルコールフラグ */
#endif				/*  */

#ifdef __cplusplus
}
#endif

static AG_Mutex *VMMutex;


/*-[ 同期 ]-----------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  VMをロック
*/
void LockVM(void)
{
//	if(VMMutex == NULL) return;
//	AG_MutexLock(VMMutex);
}
/*
 *  VMをアンロック
 */
void
UnlockVM(void)
{
//	if(VMMutex == NULL) return;
//	AG_MutexUnlock(VMMutex);
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
#ifdef __cplusplus
}
#endif

extern void MainLoop(int argc, char *argv[]);

/*
 *  メイン関数
 */
int main(int argc, char *argv[])
{
   int rgb_size[3];
   int flags;
/*
 * 実行モジュールのディレクトリを取得
 */
        char    *p;

       p = getenv("HOME");
        if(p == NULL) {
                perror("Warning : Can't get HOME directory...Making ./.xm7/ .");
                strcpy(ModuleDir, "./.xm7/");
        } else {
                strcpy(ModuleDir, p);
                strcat(ModuleDir, "/.xm7/");
        }
        if(opendir(ModuleDir) == NULL) {
                mkdir(ModuleDir, 0777);
        }

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

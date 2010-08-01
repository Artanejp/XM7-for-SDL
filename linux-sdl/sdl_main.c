/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta  
 * [ XWIN/SDLメインプログラム ] 
 */  
    
    
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/param.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
    
#include "xm7.h"
#include "mouse.h"
#include "tapelp.h"
#include "keyboard.h"
#include "sdl.h"
#include "sdl_bar.h"
#include "sdl_draw.h"
#include "sdl_kbd.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_cfg.h"
#include "sdl_inifile.h"
#ifdef _XWIN
    
/*
 *  グローバル ワーク 
 */ 
char            ModuleDir[MAXPATHLEN];	/* XM7実行モジュールディレクトリ 
					 */


SDL_Surface     *drawArea = NULL;       /* スクリーン描画エリア */
SDL_Surface     *displayArea = NULL;
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
    
/*
 *  スタティック ワーク 
 */ 
    
/*
 *  アセンブラ関数のためのプロトタイプ宣言 ->削除
 */ 
    
/*-[ 同期 ]-----------------------------------------------------------------*/ 
    
/*
 *  VMをロック 
*/ 
void
LockVM(void) 
{
} 
/*
 *  VMをアンロック 
 */ 
void
UnlockVM(void) 
{
} 
/*-[ ドローウインドウ ]-----------------------------------------------------*/ 
    
    /*
     *  SDL関連フラグ 
     */ 
    
#define XM7_DRAWMODE_SDL SDL_HWSURFACE
    
    /*
     *  ドローウインドウ作成(SDL) 
     */ 
void
CreateDrawSDL(void) 
{
    SDL_Surface * ret;
    ret = SDL_SetVideoMode(640, 480, 16,
	 SDL_HWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE |
	 SDL_DOUBLEBUF | SDL_ASYNCBLIT | SDL_HWPALETTE | 0);
    if (ret == NULL) {
	perror("SDL can't get display area.");	/* 最終的にはWidget独立でPopup出す */
	SDL_FreeSurface(drawArea);
	exit(1);
    }
    displayArea = SDL_GetVideoSurface();
/*
 * マウス処理やWIndowとの接続をここに入れる 
 */ 
// SDL_UpdateRect(displayArea ,0 ,0 ,640 ,480);
}




/*-[ メインウインドウ ]-----------------------------------------------------*/ 
    
    /*
     *  ウインドウ作成 
     */
#ifdef USE_GTK 
void
OnCreate(GtkWidget *parent)
#else
void
OnCreate(void *parent)
#endif
{
        BOOL        flag;
#ifdef USE_GTK    
    CreateMenu(parent);
    CreateDrawGTK(parent);
//    CreateStatus();

#else
//    CreateStatus();

#endif
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
        InitSch();
        CreateStatus();
 
    
//#ifdef FDDSND
// InitFDDSnd();
//#endif
	
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



static void 
InitInstance(void)
{

#ifdef USE_GTK
    InitInstanceGtk();
#endif

}



/*
 *  Update UI 
 */ 
void
ui_update(void) 
{
#ifdef USE_GTK
        ui_update_gtk();
#endif
}


/*
 *  メイン関数 
 */ 
int
main(int argc, char *argv[]) 
{
    
/*
 * 実行モジュールのディレクトリを取得 
 */ 


        char    *p;
        p = getenv("HOME");
        if(p == NULL) {
                perror("Werning : Can't get HOME directory...Making ./.xm7/ .");
                strcpy(ModuleDir, "./.xm7/");
        } else {
                strcpy(ModuleDir, p);
                strcat(ModuleDir, "/.xm7/");
        }
        if(opendir(ModuleDir) == NULL) {
                mkdir(ModuleDir, 0777);
        }

/*
 * 各種フラグを取得・設定 
 */ 
	// bMMXflag = CheckMMX();
	// bCMOVflag = CheckCMOV();
/*
 * x86依存命令除去 
 */ 
        bMMXflag = FALSE;
        bCMOVflag = FALSE;
    
#if ((XM7_VER <= 2) && defined(FMTV151))
        bFMTV151 = TRUE;
#endif				/*  */

#ifdef USE_GTK
        InitGtk(argc, argv);
#endif
/*
 * アプリケーション初期化 
 */ 
        InitInstance();
#ifdef USE_GTK
        MainLoopGtk(argc, argv);
#endif
        return nErrorCode;
}


#endif	/* _XWIN */

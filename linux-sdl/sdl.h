/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN ] 
 */  
    
#ifdef _XWIN
    
#ifndef _sdl_h_
#define _sdl_h_
    
#ifndef __STDLIB_H
#include <stdlib.h>
#endif				/*  */
    
#ifndef _SYS_PARAM_H
#include <sys/param.h>
#endif				/*  */

#ifdef  USE_GTK    
#ifndef __GTK_H__
#include <gtk/gtk.h>
#endif
#endif /* USE_GTK */				/*  */
    
#ifndef _SDL_H
#include <SDL/SDL.h>
#endif				/*  */
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
/*
 *  定数、型定義 
 */ 
        typedef unsigned int UINT;
                 
/*
 *  主要エントリ 
 */            
        extern  void    LockVM(void);
/*
 * VMロック 
 */            
        extern void     UnlockVM(void);
                   
/*
 * VMアンロック 
 */            
        extern void     CreateDrawSDL(void);
#ifdef USE_GTK
        extern void     OnCreate(GtkWidget *parent);
#else
        extern void     OnCreate(void *parent);
#endif
/*
 * 以下、Toolkit依存部分
 */
#ifdef USE_GTK
        extern void     ChangeResolutionGTK(int width, int height, int oldwidth, int oldheight);
        extern void     InitGtk(int argc, char *argv[]);
        extern void     InitInstanceGtk(void);
        extern void     OnCreateGtk(GtkWidget *parent);
        extern void     CreateDrawGTK(GtkWidget * parent);       
        extern void     SetIconGtk(int ver);
        extern void     ui_update_gtk(void);
        extern void     InitGtk(int argc, char *argv[]);
        extern void     MainLoopGtk(int argc, char *argv[]);
#endif


/*
 *  主要ワーク 
 */            
        extern char             ModuleDir[MAXPATHLEN];  /* XM7実行モジュールパス */
#ifdef USE_GTK
        extern GtkWidget        *wndMain;	     /* メインウィンドウ  */
        extern GtkWidget        *gtkDrawArea;   
        extern GtkBuilder       *gbuilderMain;   
#endif
        extern SDL_Surface      *drawArea;	     /* スクリーン描画エリア */
        extern SDL_Surface      *displayArea;           /* スクリーン表示エリア */ 
        extern char             InitialDir[5][MAXPATHLEN]; /* 初期ディレクトリ */
        extern int              nErrorCode;             /* エラーコード */
        extern BOOL             bMenuLoop;              /* メニューループ中 */
        extern BOOL             bCloseReq;              /* 終了要求フラグ */            
        extern BOOL             bSync;                  /* 実行に同期  */
        extern BOOL             bSyncDisasm[2];         /* 逆アセンブルをPCに同期 */
        extern BOOL             bActivate;              /* アクティベートフラグ */
        extern BOOL             bMMXflag;               /* MMXサポートフラグ */
                   
#if ((XM7_VER <= 2) && defined(FMTV151))
        extern BOOL             bFMTV151;               /* チャンネルコールエミュレーション */
#endif	/*  */
                 
#ifdef __cplusplus
}              
#endif	/*  */
               
#endif	/* _sdl_h_ */
#endif	/* _XWIN */

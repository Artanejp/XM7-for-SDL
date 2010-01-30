/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_h_
#define _xw_h_
    
#ifndef __STDLIB_H
#include <stdlib.h>
#endif				/*  */
    
#ifndef _SYS_PARAM_H
#include <sys/param.h>
#endif				/*  */
    
#ifndef __GTK_H__
#include <gtk/gtk.h>
#endif				/*  */
    
#ifndef _SDL_H
#include <SDL/SDL.h>
#endif				/*  */
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
    
	/*
	 *  定数、型定義 
	 */ 
    typedef unsigned int UINT;
                 
	/*
	 *  主要エントリ 
	 */            
    extern void FASTCALL LockVM(void);
                   
	/*
	 * VMロック 
	 */            
    extern void FASTCALL UnlockVM(void);
                   
	/*
	 * VMアンロック 
	 */            
                   
	/*
	 *  主要ワーク 
	 */            
    extern char     ModuleDir[MAXPATHLEN];	/* XM7実行モジュールパス 
						 */
                  extern GtkWidget *wndMain;	/* メインウィンドウ 
						 */
                  extern SDL_Surface *drawArea;	/* スクリーン描画エリア 
							 */
                   extern SDL_Surface *displayArea;	/* スクリーン表示エリア 
							 */
                  extern char InitialDir[5][MAXPATHLEN];
                   
	/*
	 * 初期ディレクトリ 
	 */            
    extern int      nErrorCode;
                   
	/*
	 * エラーコード 
	 */            
    extern BOOL     bMenuLoop;
                   
	/*
	 * メニューループ中 
	 */            
    extern BOOL     bCloseReq;
                   
	/*
	 * 終了要求フラグ 
	 */            
    extern BOOL     bSync;
                   
	/*
	 * 実行に同期 
	 */            
    extern BOOL     bSyncDisasm[2];
                   
	/*
	 * 逆アセンブルをPCに同期 
	 */            
    extern BOOL     bActivate;
                   
	/*
	 * アクティベートフラグ 
	 */            
    extern BOOL     bMMXflag;
                   
	/*
	 * MMXサポートフラグ 
	 */            
                   
#if ((XM7_VER <= 2) && defined(FMTV151))
    extern BOOL     bFMTV151;
                   
	/*
	 * チャンネルコールエミュレーション 
	 */            
#endif				/*  */
                 
#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_h_ */
#endif	/* _XWIN */

/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN ]
 */


#ifndef _xm7_sdl_h_
#define _xm7_sdl_h_

#ifndef __STDLIB_H
#include <stdlib.h>
#endif				/*  */

#ifndef _SYS_PARAM_H
#include <sys/param.h>
#endif				/*  */

#ifndef _SDL_H
#ifndef _WINDOWS
# include <SDL.h>
#endif
#endif				/*  */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#ifdef __cplusplus
extern          "C" {

#endif				/*  */
/*
 *  定数、型定義
 */
typedef unsigned int UINT;
//#ifndef BOOL
//typedef int BOOL;
//#endif

/*
 *  主要エントリ
 */
  //      extern  void    LockVM(void);
/*
 * VMロック
 */
//        extern void     UnlockVM(void);

/*
 * VMアンロック
 */
   //    extern void     OnCreate(AG_Widget *parent);
/*
 * 以下、Toolkit依存部分
 */

/*
 *  主要ワーク
 */
   extern char             ModuleDir[MAXPATHLEN];  /* XM7実行モジュールパス */
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

#endif	/* _xm7_sdl_h_ */

/*
 * agar_xm7.h
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */

#ifndef AGAR_XM7_H_
#define AGAR_XM7_H_
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

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

//#ifndef _SDL_H
//#include <SDL.h>
//#endif				/*  */

/*
 *  定数、型定義
 */
typedef unsigned int UINT;
//#ifndef BOOL
//#define BOOL int
//#endif

#ifdef __cplusplus
extern "C"
{
#endif	/*  */

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
extern void     OnCreate(AG_Widget *parent);
/*
 * 以下、Toolkit依存部分
 */
/*
 *  主要ワーク
 */
        extern char             ModuleDir[MAXPATHLEN];  /* XM7実行モジュールパス */
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


#endif /* AGAR_XM7_H_ */

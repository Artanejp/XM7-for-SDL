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
#include "xm7.h"

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

/* Gettext */
#include <libintl.h>

#define _(String) gettext(String)
#define N_(String) gettext_noop(String)
#define gettext_noop(String) (String)


/* デフォルトフォントが設定されてないときはフォントとしてIPAゴシックを使う */
#ifndef FONTPATH

#endif

//#ifndef DIALOG_WINDOW_DEFAULT
//#define DIALOG_WINDOW_DEFAULT AG_WINDOW_KEEPABOVE | AG_WINDOW_NOCLOSE
//#endif
//#ifndef FILEDIALOG_WINDOW_DEFAULT
//#define FILEDIALOG_WINDOW_DEFAULT AG_WINDOW_KEEPABOVE | AG_WINDOW_DIALOG
//#endif
#define DIALOG_WINDOW_DEFAULT 0
#define FILEDIALOG_WINDOW_DEFAULT 0
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
 * 外部変数
 */
extern unsigned char* _mbsrchr(const unsigned char* string, unsigned int c);
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

// Macros
#ifndef AGDRIVER_IS_GLX
#define AGDRIVER_IS_GLX(drv) \
        (strcmp("glx", AGDRIVER_CLASS(drv)->name) == 0)
#endif
#ifdef __cplusplus
}
#endif	/*  */


#endif /* AGAR_XM7_H_ */

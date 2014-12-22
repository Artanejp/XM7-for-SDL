/*
 * ag_cmd.cpp
 *
 *  Created on: 2010/11/10
 *      Author: whatisthis
 */

/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 *[ GTK メニューコマンド]
 */

#ifdef USE_GTK
#include <gtk/gtk.h>
#endif

#ifdef USE_AGAR
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <SDL/SDL.h>
#endif

#include "xm7.h"
#include "fdc.h"
#include "tapelp.h"
#include "tools.h"
#include "mouse.h"
#include "rtc.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#else
#include "xm7_sdl.h"
#include "sdl_cfg.h"
#include "sdl_prop.h"
#endif

#include "agar_cmd.h"
#include "agar_toolbox.h"


#ifdef USE_GTK
#include "gtk_toolbox.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
    /*
     *  グローバル ワーク
     */
char InitialDir[5][MAXPATHLEN];
    /*
     *  スタティック ワーク
     */


/*-[ ファイルメニュー ]-----------------------------------------------------*/

/*
 *  リセット(R)
 */
void OnReset(AG_Event *ev)
{
    LockVM();
    system_reset();
    UnlockVM();

	/*
	 * 再描画
	 */
	// OnRefresh();
}
/*
 *  ホットリセット(H)
 */
void OnHotReset(AG_Event *ev)
{
    LockVM();
    system_hotreset();
    UnlockVM();

	/*
	 * 再描画
	 */
	// OnRefresh();
}
    /*
     *  BASICモード(B)
     */
void OnBasic(AG_Event *ev)
{

    LockVM();
    boot_mode = BOOT_BASIC;
    if (fm7_ver < 2) {
	mainmem_transfer_boot();
    }
    else {
	system_reset();
    }
    UnlockVM();
}


    /*
     *  DOSモード(D)
     */
void OnDos(AG_Event *ev)
{

    LockVM();
    boot_mode = BOOT_DOS;
    if (fm7_ver < 2) {
	mainmem_transfer_boot();
    }

    else {
	system_reset();
    }
    UnlockVM();
}


    /*
     *  終了(X)
     */
void OnExit(AG_Event *event)
{
/*
 * フラグアップ
 */
    LockVM();
    bCloseReq = TRUE;
    UnlockVM();
    //gtk_widget_destroy(GTK_WIDGET(data));
    /* ここに、Exitするかどうかのダイアログを入れるか？ */
    AG_Destroy();
}



    /*
     *  実行(X)
     */
void OnExec(void)
{

/*
 * 既に実行中なら、何もしない
 */
        if (run_flag) {
                return;
        }
/*
 * スタート
 */
        LockVM();
        stopreq_flag = FALSE;
        run_flag = TRUE;
        UnlockVM();
}


/*
 *  停止(B)
 */
void OnBreak(void)
{
        /*
         * 既に停止状態なら、何もしない
         */
        if (!run_flag) {
                return;
        }

/*
 * 停止
 */
        LockVM();
        stopreq_flag = TRUE;
        UnlockVM();
}




/*
 *  マウスモード切り換え(M)
 */
#ifdef MOUSE
void OnMouseMode(AG_Event *event)
{

/*
 * マウスキャプチャフラグを反転させてモード切り替え
 */
        mos_capture = (!mos_capture);
        SetMouseCapture(mos_capture);
}
#endif				/*  */
    /*
     *  サウンド出力モード切り替え
     */
void OnChgSound(AG_Event *event)
{
    LockVM();

	/*
	 * サウンドモード変更
	 */
	nStereoOut = (nStereoOut + 1) % 4;

	/*
	 * 適用
	 */
	ApplySnd();
    UnlockVM();
}





#ifdef __cplusplus
}
#endif

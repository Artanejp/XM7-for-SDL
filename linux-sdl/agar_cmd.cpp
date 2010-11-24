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
#include "sdl.h"
#include "sdl_cfg.h"
#include "sdl_prop.h"
#endif

#include "agar_cmd.h"
#include "agar_toolbox.h"

#include "sdl_snd.h"
#include "sdl_sch.h"
#include "api_kbd.h"
#include "api_js.h"


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
void OnReset(AG_Event *event)
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
void OnHotReset(AG_Event *event)
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
void OnBasic(AG_Event *event)
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
void OnDos(AG_Event *event)
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
     //   SetMouseCapture(bActivate);
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


    /*
     *  ファイルドロップサブ
     */
void OnDropSub(char *path)
{
    char           InitDir[MAXPATHLEN];
    char          *ext = NULL;
    char          *p = NULL;

    /*
     * 拡張子だけ分離
     */
    p = strrchr(path, '.');
    if (p != NULL) {
	ext = p;
    }
    strcpy(InitDir, path);
    p = strrchr(InitDir, '/');
    if (p != NULL) {
	p[1] = '\0';
    } else {
	InitDir[0] = '\0';
    }
    if (ext != NULL) {

	    /*
	     * D77
	     */
	    if (stricmp(ext, ".D77") == 0) {
	    strcpy(InitialDir[0], InitDir);
	    LockVM();
	    StopSnd();
	    fdc_setdisk(0, path);
	    fdc_setdisk(1, NULL);
	    if ((fdc_ready[0] != FDC_TYPE_NOTREADY)
		 && (fdc_medias[0] >= 2)) {
		fdc_setdisk(1, path);
		fdc_setmedia(1, 1);
	    }
	    system_reset();
	    PlaySnd();
	    ResetSch();
	    UnlockVM();
	}

	    /*
	     * 2D/VFD
	     */
	    if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".VFD") == 0)) {
	    strcpy(InitialDir[0], InitDir);
	    LockVM();
	    StopSnd();
	    fdc_setdisk(0, path);
	    fdc_setdisk(1, NULL);
	    system_reset();
	    PlaySnd();
	    ResetSch();
	    UnlockVM();
	}

	    /*
	     * T77
	     */
	    if (stricmp(ext, ".T77") == 0) {
	    strcpy(InitialDir[1], InitDir);
	    LockVM();
	    tape_setfile(path);
	    UnlockVM();
	}

	    /*
	     * XM7
	     */
	    if (stricmp(ext, ".XM7") == 0) {
	    strcpy(InitialDir[2], InitDir);
	    LockVM();
	    StopSnd();
	    StateLoad(path);
	    PlaySnd();
	    ResetSch();
	    UnlockVM();
	}
    }
}


/*
 *  ファイルドロップ
 */
void OnDropFiles(void)
{
    char           path[MAXPATHLEN];

	/*
	 * 処理
	 */
	OnDropSub(path);
}
    /*
     *  コマンドライン処理
     */
void OnCmdLine(char *arg)
{
/*
 * 処理
 */
        OnDropSub(arg);
}


#ifdef __cplusplus
}
#endif

/*
 * agar_toolbox.cpp
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */

/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 *[ Agar メニューコマンド]
 */

#include <libintl.h>
#include <iconv.h>
#include <agar/core.h>
#include <agar/gui.h>

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
#endif
#include "agar_cmd.h"
#include "sdl_prop.h"
#include "sdl_snd.h"
#include "sdl_sch.h"
#include "api_kbd.h"

#include "agar_toolbox.h"

extern void KeyBoardSnoop(BOOL Flag);



static Disk   disk[2][FDC_MEDIAS];
static char     StatePath[MAXPATHLEN];
static char    DiskTitle[16 + 1];
static BOOL    DiskMedia;
static BOOL    DiskFormat;

//static guint   hidWavCapture;

#ifdef MOUSE
//static guint   hidMouseCapture;
#endif				/*  */


static AG_MenuItem *debug_menu;
static AG_MenuItem *debug_item;
static AG_MenuItem *debug_stop;
static AG_MenuItem *debug_restart;
static AG_MenuItem *debug_disasm_main;
static AG_MenuItem *debug_disasm_sub;
static AG_MenuItem *miExec;
static AG_MenuItem *miBreak;
static AG_MenuItem *tool_menu;
static AG_MenuItem *tool_item;
static AG_MenuItem *miWavCapture;
#ifdef MOUSE
static AG_MenuItem *miMouseCapture;
#endif
static AG_MenuItem *help_menu;
static AG_MenuItem *help_item;

void OnPushCancel(AG_Event *event)
{
	AG_Button *self = (AG_Button *)AG_SELF();
	KeyBoardSnoop(FALSE);
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}

    /*
     *  ステートロード処理
     */
void StateLoad(char *path)
{
    int            state;
    state = system_load(path);
    if (state == STATELOAD_ERROR) {

	    /*
	     * 本体読み込み中のエラー発生時のみリセット
	     */
	    system_reset();
    }
    if (state != STATELOAD_SUCCESS) {
    }

    else {
	strcpy(StatePath, path);
	SetMachineVersion();
    }
}

/*
 *  開く(O)
 */
/*
 * ファイル選択
 */
static void OnLoadStatusSub(char *filename)
{
    char          *p;
    if(filename == NULL) return;
    /*
     * ステートロード
     */
    LockVM();
    StopSnd();
    StateLoad(filename);
    PlaySnd();
    ResetSch();
    UnlockVM();
    /*
     * 画面再描画
     */
    // OnRefresh();
    p = strrchr(filename, '/');
    if (p != NULL) {
            p[1] = '\0';
            strcpy(InitialDir[2], filename);
    }
}

static void OnLoadStatusSubEv(AG_Event *event)
{
    char  *sFilename = AG_STRING(1);
    KeyBoardSnoop(FALSE);
    OnLoadStatusSub(sFilename);
}


void OnLoadStatus(AG_Event *event)
{
    AG_Window *dlgWin;
    AG_FileDlg *dlg;
    dlgWin = AG_WindowNew(FILEDIALOG_WINDOW_DEFAULT);
    if(dlgWin == NULL) return;
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
    if(dlg == NULL) return;
    KeyBoardSnoop(TRUE);
    AG_FileDlgSetDirectory(dlg, InitialDir[2]);
    AG_FileDlgAddType(dlg, "XM7 Status", "*.xm7,*.XM7", OnLoadStatusSubEv, NULL);
    AG_WidgetFocus(dlg);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_ActionFn(AGWIDGET(dlgWin), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
    AG_WindowShow(dlgWin);
}

void OnQuickLoad(AG_Event *event)
{
#if 0
	if(strlen(s) <= 0){
		OnLoadStatus();
		return;
	}
	OnLoadStatusSub(s);
#endif
}


/*
 *  保存(A)
 */
static void OnSaveStatusSub(char *filename)
{
    char          *p;
    KeyBoardSnoop(FALSE);
    if(filename == NULL) return;
	/*
	 * ファイル選択
	 */

    /*
     * ステートセーブ
     */
    LockVM();
    StopSnd();
    if (!system_save(filename)) {
    } else {
    	strcpy(StatePath, filename);
    }
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(filename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[2], filename);
    }
}

static void OnSaveStatusSubEv(AG_Event *event)
{
    char  *sFilename = AG_STRING(1);
    OnSaveStatusSub(sFilename);
    KeyBoardSnoop(FALSE);

}


void OnSaveAs(AG_Event *event)
{
	/*
	 * ファイル選択
	 */
    AG_Window *dlgWin;
    AG_FileDlg *dlg;
    dlgWin = AG_WindowNew(FILEDIALOG_WINDOW_DEFAULT);
//    dlgWin = MainWindow;
    if(dlgWin == NULL) return;
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
    if(dlg == NULL) return;
    KeyBoardSnoop(TRUE);
    AG_FileDlgSetDirectory(dlg, InitialDir[2]);
    AG_FileDlgAddType(dlg, "XM7 Status", "*.xm7,*.XM7", OnSaveStatusSubEv, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_ActionFn(AGWIDGET(dlgWin), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
    AG_WidgetFocus(dlg);
    AG_WindowShow(dlgWin);
}

void OnQuickSave(AG_Event *event)
{
#if 0
	if(strlen(s) <= 0){
		OnSaveAs();
		return;
	}
	OnSaveStatusSub(s);
#endif
}





/*-[ テープメニュー ]-------------------------------------------------------*/

    /*
     *  テープ(A)メニュー更新
     */


/*-[ ヘルプメニュー ]-----------------------------------------------------*/

#if 0


/*-[ デバッグメニュー ]-----------------------------------------------------*/

    /*
     *  デバッグメニュー更新
     */
void
OnDebugPopup(GtkWidget * widget, gpointer data)
{
    if (run_flag) {
	gtk_widget_set_sensitive(debug_restart, FALSE);
	gtk_widget_set_sensitive(debug_stop, TRUE);
    } else {
	gtk_widget_set_sensitive(debug_restart, TRUE);
	gtk_widget_set_sensitive(debug_stop, FALSE);
    }
}

/*
 * 逆アセンブル
 */
void
OnMainDisAsmPopup(GtkWidget * widget, gpointer data)
{
	if(disasm_main_flag)
	{
		//gtk_widget_set_sensitive(debug_disasm_main, FALSE);
		disasm_main_flag = FALSE;
	} else {
		//gtk_widget_set_sensitive(debug_disasm_main, FALSE);
		disasm_main_flag = TRUE;
	}
}

void
OnSubDisAsmPopup(GtkWidget * widget, gpointer data)
{
	if(disasm_sub_flag)
	{
		//gtk_widget_set_sensitive(debug_disasm_main, FALSE);
		disasm_sub_flag = FALSE;
	} else {
		//gtk_widget_set_sensitive(debug_disasm_main, FALSE);
		disasm_sub_flag = TRUE;
	}
}



    /*
     *  VFD→D77変換(V)
     */
static void
OnVFD2D77(GtkWidget * widget, gpointer data)
{
    char          *p;
    FileSelectDialog sdlg, ddlg;
    DiskTitleDialog tdlg;
    char           src[MAXPATHLEN];
    char           dst[MAXPATHLEN];

	/*
	 * ファイル選択
	 */
	sdlg = OpenFileSelectDialog(InitialDir[0]);
    if (sdlg.bResult != DLG_OK) {
	return;
    }
    strcpy(src, sdlg.sFilename);

	/*
	 * タイトル入力
	 */
	tdlg = OpenDiskTitleDialog();
    if (tdlg.bResult != DLG_OK) {
	return;
    }
    strcpy(DiskTitle, tdlg.sTitle);

	/*
	 * ファイル選択
	 */
	ddlg = OpenFileSelectDialog(InitialDir[0]);
    if (ddlg.bResult != DLG_OK) {
	return;
    }
    strcpy(dst, ddlg.sFilename);

	/*
	 * 作成
	 */
	LockVM();
    StopSnd();
    if (conv_vfd_to_d77(src, dst, DiskTitle)) {
    }
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(ddlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[0], ddlg.sFilename);
    }
}


    /*
     *  2D→D77変換(2)
     */
static void
On2D2D77(GtkWidget * widget, gpointer data)
{
    char          *p;
    FileSelectDialog sdlg, ddlg;
    DiskTitleDialog tdlg;
    char           src[MAXPATHLEN];
    char           dst[MAXPATHLEN];

	/*
	 * ファイル選択
	 */
	sdlg = OpenFileSelectDialog(InitialDir[0]);
    if (sdlg.bResult != DLG_OK) {
	return;
    }
    strcpy(src, sdlg.sFilename);

	/*
	 * タイトル入力
	 */
	tdlg = OpenDiskTitleDialog();
    if (tdlg.bResult != DLG_OK) {
	return;
    }
    strcpy(DiskTitle, tdlg.sTitle);

	/*
	 * ファイル選択
	 */
	ddlg = OpenFileSelectDialog(InitialDir[0]);
    if (ddlg.bResult != DLG_OK) {
	return;
    }
    strcpy(dst, ddlg.sFilename);

	/*
	 * 作成
	 */
	LockVM();
    StopSnd();
    if (conv_2d_to_d77(src, dst, DiskTitle)) {
    }
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(ddlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[0], ddlg.sFilename);
    }
}


    /*
     *  VTP→T77変換(P)
     */
static void
OnVTP2T77(GtkWidget * widget, gpointer data)
{
    char          *p;
    FileSelectDialog sdlg, ddlg;
    char           src[MAXPATHLEN];
    char           dst[MAXPATHLEN];

	/*
	 * ファイル選択
	 */
	sdlg = OpenFileSelectDialog(InitialDir[1]);
    if (sdlg.bResult != DLG_OK) {
	return;
    }
    strcpy(src, sdlg.sFilename);

	/*
	 * ファイル選択
	 */
	ddlg = OpenFileSelectDialog(InitialDir[1]);
    if (ddlg.bResult != DLG_OK) {
	return;
    }
    strcpy(dst, ddlg.sFilename);

	/*
	 * 作成
	 */
	LockVM();
    StopSnd();
    if (conv_vtp_to_t77(src, dst)) {
    }
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(ddlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[1], ddlg.sFilename);
    }
}



/*-[ ツールメニュー ]-------------------------------------------------------*/


#endif
/*
 *  メニューバーの生成
 */
void CreateMenu(void)
{
    memset(InitialDir, 0x00, 5 * MAXPATHLEN);
}

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

#include <sys/stat.h>

#include <SDL/SDL.h>
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
#include "xm7_sdl.h"
#endif
#include "agar_cmd.h"

#include "sdl_prop.h"
#include "api_snd.h"
#include "sdl_sch.h"
#include "api_kbd.h"

#include "agar_toolbox.h"

//extern AG_Mutex nRunMutex;
extern "C" {
   extern DWORD XM7_timeGetTime(void);	/* timeGetTime互換関数 */
   extern void  XM7_Sleep(DWORD t);	/* Sleep互換関数 */
}



static Disk   disk[2][FDC_MEDIAS];
static char   StatePath[MAXPATHLEN];
static char   DiskTitle[16 + 1];
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
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}

void OnPushCancel2(AG_Event *event)
{
	AG_Button *self = (AG_Button *)AG_SELF();
        void *cfg = AG_PTR(1);
   
        if(cfg != NULL) free(cfg);
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}

void OnConfigApplyMain(configdat_t *p, AG_Button *self)
{

        int ver;
	LockVM();
        
	memcpy(&configdat, p, sizeof(configdat_t));
	ver = fm7_ver;
	ApplyCfg();
	/*
	 * VMヴァージョンが違ったら強制リセット
	 */
	if(ver != fm7_ver){
		system_reset();
	}
	/*
	 * ここにアイコン変更入れる
	 */

	/*
	 * 終了処理
	 */
	UnlockVM();
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self);
//	AG_WindowDetach(self->wid.window);
}

configdat_t localconfig;

void OnConfigApply(AG_Event *event)
{
	int ver;
        int i;
	AG_Button *self = (AG_Button *)AG_SELF();

        OnConfigApplyMain(&localconfig, self);
}



void OnConfigApply2(AG_Event *event)
{
	int ver;
        int i;
	AG_Button *self = (AG_Button *)AG_SELF();
        configdat_t *cfg = AG_PTR(1);

        if(cfg == NULL) return;
         OnConfigApplyMain(cfg, self);
        free(cfg);
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
    } else {
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
//    AG_MutexLock(&nRunMutex);
    StopSnd();
    StateLoad(filename);
    PlaySnd();
    ResetSch();
    UnlockVM();
//    AG_MutexUnlock(&nRunMutex);
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
    OnLoadStatusSub(sFilename);
}


void OnLoadStatus(AG_Event *event)
{
    AG_Window *dlgWin;
    AG_FileDlg *dlg;
    dlgWin = AG_WindowNew(FILEDIALOG_WINDOW_DEFAULT);
    if(dlgWin == NULL) return;
    AG_WindowSetCaption(dlgWin, "%s", gettext("Load status as"));
    dlg = AG_FileDlgNew(dlgWin, FILEDLG_DEFAULT | AG_FILEDLG_LOAD);
    if(dlg == NULL) return;
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
    struct stat st;
    char stmp[MAXPATHLEN];

    stmp[0] = '\0';
	if(strlen(StatePath) > 0){
	    strcpy(stmp, StatePath);
	    if(stat(stmp, &st) == 0) {
            OnLoadStatusSub(stmp);
            return;
	    }
    }
    OnLoadStatus(event);
    return;
}


/*
 *  保存(A)
 */
static void OnSaveStatusSub(char *filename)
{
    char          *p;
    if(filename == NULL) return;
	/*
	 * ファイル選択
	 */

    /*
     * ステートセーブ
     */
    LockVM();
//    AG_MutexLock(&nRunMutex);
    StopSnd();
    XM7_Sleep(10);
    if (!system_save(filename)) {
    } else {
    	strcpy(StatePath, filename);
    }
//    run_flag = TRUE; 
    PlaySnd();
    ResetSch();
    UnlockVM();
//    AG_MutexUnlock(&nRunMutex);
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
    AG_WindowSetCaption(dlgWin, "%s", gettext("Save status as"));
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_CLOSEWIN | AG_FILEDLG_SAVE | AG_FILEDLG_MASK_EXT);
    if(dlg == NULL) return;
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
    char stmp[MAXPATHLEN];

    stmp[0] ='\0';
	if(strlen(StatePath) > 0){
	    strcpy(stmp, StatePath);
        OnSaveStatusSub(stmp);
        return;
	}
	OnSaveAs(event);
	return;
}





/*-[ テープメニュー ]-------------------------------------------------------*/

    /*
     *  テープ(A)メニュー更新
     */


/*-[ ヘルプメニュー ]-----------------------------------------------------*/
/*
 *  メニューバーの生成
 */
void XM7_CreateMenu(void)
{
    memset(InitialDir, 0x00, 5 * MAXPATHLEN);
}

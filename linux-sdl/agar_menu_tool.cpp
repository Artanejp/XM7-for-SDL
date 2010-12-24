/*
 * agar_menu_too.cpp
 *
 *  Created on: 2010/11/24
 *      Author: whatisthis
 */

/*
 * agar_gui_tapemenu.cpp
 *
 *  Created on: 2010/11/24
 *      Author: whatisthis
 */

#include <SDL.h>
#include <libintl.h>
extern "C" {
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
}
#include "xm7.h"
#include "tools.h"
#include "rtc.h"


#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#else
#include "sdl.h"
#include "sdl_cfg.h"
#endif

#include "sdl_bar.h"
#include "api_kbd.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_inifile.h"
#include "api_draw.h"
#include "agar_toolbox.h"

extern void OnPushCancel(AG_Event *event);
extern void OnConfigEmulationMenu(AG_Event *event);
extern void OnConfigInputMenu(AG_Event *event);
extern void OnConfigSoundMenu(AG_Event *event);

extern void KeyBoardSnoop(BOOL t);

static BOOL b2DD ;
static BOOL bFormat ;
static char DiskTitle[128];


/*
 *  新規ディスク作成(D)
 */
static void OnNewDiskCreate(AG_Event *event)
{
    AG_Button *btn = (AG_Button *)AG_SELF();
	char *title = DiskTitle;
	char *filename = AG_STRING(1);
	int err;
	char *p;

	LockVM();
	StopSnd();
	if (bFormat) {
		err = make_new_userdisk(filename, title, b2DD);
	}	else {
		err = make_new_d77(filename, title, b2DD);
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
	p = strrchr(filename, '/');
	if (p != NULL) {
		p[1] = '\0';
		strcpy(InitialDir[0], filename);
	}
    KeyBoardSnoop(FALSE);
    //	AG_FILEDLG_CLOSEWIN指定してるので後始末要らない
}


static void OnNewDisk(AG_Event *event)
{
	AG_Window *w;
	AG_Box *box;
	AG_Box *box2;
	AG_Label *lbl;
	AG_Textbox *textbox;
	AG_Checkbox *check;
	AG_FileDlg *dlg;


    KeyBoardSnoop(TRUE);

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(w, 280, 120);
    AG_WindowSetPadding(w, 10, 10, 10, 10);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s", gettext("Make Disk Image") );
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	textbox = AG_TextboxNew(w,  AG_TEXTBOX_STATIC | AG_TEXTBOX_HFILL, "%s:", gettext("Title"));
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXXXXXXX");
	AG_TextboxPrintf(textbox, "Default");
	AG_TextboxBindUTF8(textbox, DiskTitle, sizeof(DiskTitle));
	AG_WidgetFocus(AGWIDGET(textbox));
	check = AG_CheckboxNewInt(w, 0, "2DD", &b2DD);
	check = AG_CheckboxNewInt(w, 0, gettext("Disk Format"), &bFormat);

	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 16);
	box2 = AG_BoxNewVert(box, 0);

	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 200);
    dlg = AG_FileDlgNew(w, AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN);
	AG_FileDlgSetDirectory (dlg, "%s", InitialDir[0]);
	AG_FileDlgAddType(dlg, "D77 Image File", "*.d77,*.D77", OnNewDiskCreate, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);

	AG_WindowSetCaption(w, gettext("Create Disk "));
	AG_WindowShow(w);
}


static void OnNewTapeCreate(AG_Event *event)
{
    AG_Button *btn = (AG_Button *)AG_SELF();
	char *filename = AG_STRING(1);
	int err;
	char *p;

	LockVM();
	StopSnd();
	if (make_new_t77(filename)) {
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
	p = strrchr(filename, '/');
	if (p != NULL) {
		p[1] = '\0';
		strcpy(InitialDir[1], filename);
	}

    KeyBoardSnoop(FALSE);
    //	AG_FILEDLG_CLOSEWIN指定してるので後始末要らない
}

static void OnNewTape(AG_Event *event)
{
	AG_Window *w;
	AG_Box *box;
	AG_Label *lbl;
	AG_FileDlg *dlg;


    KeyBoardSnoop(TRUE);

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(w, 280, 120);
    AG_WindowSetPadding(w, 10, 10, 10, 10);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s", gettext("Make Disk Image") );
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 16);
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 200);
    dlg = AG_FileDlgNew(w, AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN);
	AG_FileDlgSetDirectory (dlg, "%s", InitialDir[1]);
	AG_FileDlgAddType(dlg, "T77 Image File", "*.t77,*.T77", OnNewTapeCreate, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_WidgetFocus(dlg);
	AG_WindowSetCaption(w, gettext("Create Tape"));
	AG_WindowShow(w);
}


static void ScreenCaptureSub(AG_Event *event)
{
	char *sFilename = AG_STRING(1);
	AG_Button *btn = (AG_Button *)AG_SELF();
	char *p;
	/*
	 * キャプチャ
	 */
	if((sFilename == NULL) || (strlen(sFilename) <= 0)) return;
	LockVM();
    StopSnd();
    capture_to_bmp(sFilename, FALSE);
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[3], sFilename);
    }
    KeyBoardSnoop(FALSE);
    //	AG_FILEDLG_CLOSEWIN指定してるので後始末要らない
}
/*
 *  画面キャプチャ(C)
 */
static void OnGrpCapture(AG_Event *event)
{
    AG_Widget *parent = (AG_Widget *)AG_SENDER();
    AG_FileDlg *dlg;
    AG_Window *w;
	/*
	 * ファイル選択
	 */
	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(w, 280, 120);
    KeyBoardSnoop(TRUE);
	dlg = AG_FileDlgNew(w, AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN);
	if(dlg == NULL) return;
	AG_FileDlgSetDirectory(dlg, "%s",InitialDir[3]);
	AG_FileDlgAddType(dlg, "BMP file","*.bmp,*.BMP", ScreenCaptureSub, NULL);
    AG_WidgetFocus(dlg);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
	AG_WindowSetCaption(w, gettext("Capture to BMP"));
	AG_WindowShow(w);
}


static void ScreenCaptureSub2(AG_Event *event)
{
	AG_Button *btn = (AG_Button *)AG_SELF();
	char *sFilename = AG_STRING(1);
	char *p;
	/*
	 * キャプチャ
	 */
	if((sFilename == NULL) || (strlen(sFilename) <= 0)) return;
	LockVM();
    StopSnd();
    capture_to_bmp2(sFilename);
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
		strcpy(InitialDir[3], sFilename);
    }
    KeyBoardSnoop(FALSE);
    //	AG_FILEDLG_CLOSEWIN指定してるので後始末要らない
}
/*
 *  画面キャプチャ2
 */
static void OnGrpCapture2(AG_Event *event)
{
    AG_Widget *parent = (AG_Widget *)AG_SENDER();
    AG_FileDlg *dlg;
    AG_Window *w;
	/*
	 * ファイル選択
	 */
    KeyBoardSnoop(TRUE);
	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(w, 280, 120);
	dlg = AG_FileDlgNew(w, AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN);
	AG_FileDlgSetDirectory(dlg, "%s",InitialDir[3]);
	AG_FileDlgAddType(dlg, "BMP file","*.bmp,*.BMP", ScreenCaptureSub2, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_WidgetFocus(dlg);
	AG_WindowSetCaption(w, gettext("Capture to BMP 2"));
	AG_WindowShow(w);
}

static void OnWavCaptureSub(AG_Event *event)
{
	AG_Button *btn = (AG_Button *)AG_SELF();
	char *sFilename = AG_STRING(1);
	char *p;

	if((sFilename == NULL) || (strlen(sFilename) <= 0)) return;
	/*
	 * キャプチャ
	 */
	LockVM();
    OpenCaptureSnd(sFilename);
    UnlockVM();

	/*
	 * 条件判定
	 */
	if (hWavCapture < 0) {
	LockVM();
	StopSnd();
	PlaySnd();
	ResetSch();
	UnlockVM();
    }
    p = strrchr(sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[4], sFilename);
    }
    KeyBoardSnoop(FALSE);
    //	AG_FILEDLG_CLOSEWIN指定してるので後始末要らない
}
    /*
     *  WAVキャプチャ(W)
     */
static void OnWavCapture(AG_Event *event)
{
    AG_Widget *parent = (AG_Widget *)AG_SENDER();
    AG_FileDlg *dlg;
    AG_Window *w;

	/*
	 * 既にキャプチャ中なら、クローズ
	 */
	if (hWavCapture >= 0) {
	LockVM();
	CloseCaptureSnd();
	UnlockVM();
	return;
    }

	/*
	 * ファイル選択
	 */
	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(w, 280, 120);
	KeyBoardSnoop(TRUE);
	dlg = AG_FileDlgNew(parent, AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN);
	AG_FileDlgSetDirectory(dlg, "%s",InitialDir[4]);
	AG_FileDlgAddType(dlg, "WAV Sound file","*.wav,*.WAV", OnWavCaptureSub, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_WidgetFocus(dlg);
	AG_WindowSetCaption(w, gettext("Capture Sound"));
	AG_WindowShow(w);
}


/*
 *  時刻アジャスト
 */
static void OnTimeAdjust(AG_Event *event)
{

	/*
	 * 時刻を再設定する
	 */
	rtc_time_adjust();

	/*
	 * 念のためスケジュールを初期化
	 */
	rtc_reset();
}


void Create_ToolsMenu(AG_MenuItem *parent)
{
	AG_MenuItem *item;

	item = AG_MenuAction(parent , gettext("Capture Screen "), NULL, OnGrpCapture, NULL);
	item = AG_MenuAction(parent , gettext("Capture Screen (Non-Scaled)"), NULL, OnGrpCapture2, NULL);
	item = AG_MenuAction(parent , gettext("Capture Sound"), NULL, OnWavCapture, NULL);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent , gettext("Adjust Clock"), NULL, OnTimeAdjust, NULL);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent , gettext("Create Virtual Disk"), NULL, OnNewDisk, NULL);
	item = AG_MenuAction(parent , gettext("Create Virtual Tape"), NULL, OnNewTape, NULL);

	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent , gettext("Configure Emulation"), NULL, OnConfigEmulationMenu, NULL);
	item = AG_MenuAction(parent , gettext("Configure Inputs"), NULL, OnConfigInputMenu, NULL);
	item = AG_MenuAction(parent , gettext("Configure Sound"), NULL, OnConfigSoundMenu, NULL);

}

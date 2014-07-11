/*
 * agar_menu_tool.cpp
 *
 *  Created on: 2010/11/24
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
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
#include "xm7_sdl.h"
#include "sdl_cfg.h"
#endif

#include "api_draw.h"
#include "agar_toolbox.h"

extern void OnPushCancel(AG_Event *event);
extern void OnConfigEmulationMenu(AG_Event *event);
extern void OnConfigInputMenu(AG_Event *event);
extern void OnConfigSoundMenu(AG_Event *event);


struct D77Attr {
   char name[18];
   BOOL b2DD;
   BOOL bFormat;
};


static void OnPushCancelDisk(AG_Event *event)
{
	AG_Button *self = (AG_Button *)AG_SELF();
        struct D77Attr *alloc = (struct D77Attr *)AG_PTR(1);
        if(alloc != NULL) free(alloc);
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}


/*
 *  新規ディスク作成(D)
 */
static void OnNewDiskCreate(AG_Event *event)
{
    AG_Button *btn = (AG_Button *)AG_SELF();
	char *filename = AG_STRING(2);
        struct D77Attr *pVDisk = (struct D77Attr*)AG_PTR(1);
        char DiskTitle[18];
        BOOL b2DD;
        BOOL bFormat;
	int err;
	char *p;

        if(pVDisk == NULL) return;
        strncpy(DiskTitle, pVDisk->name, 17);
        b2DD = pVDisk->b2DD;
        bFormat = pVDisk->bFormat;
   
	LockVM();
	StopSnd();
	if (bFormat) {
		err = make_new_userdisk(filename, DiskTitle, b2DD);
	}	else {
		err = make_new_d77(filename, DiskTitle, b2DD);
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
	p = strrchr(filename, '/');
	if (p != NULL) {
		p[1] = '\0';
		strcpy(InitialDir[0], filename);
	}
        if(pVDisk != NULL) free(pVDisk);
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
        struct D77Attr *pVDisk;
        char *DiskTitle;
   

	if((InitialDir[0] == NULL) || strlen(InitialDir[0]) <= 0) {
		strcpy(InitialDir[0], "./");
	}
   
        pVDisk =(struct D77Attr *)malloc(sizeof(struct D77Attr));
        if(pVDisk == NULL) return;
        memset(pVDisk, 0x00, sizeof(struct D77Attr));
   
	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT );
	AG_WindowSetMinSize(w, 280, 120);
    AG_WindowSetPadding(w, 10, 10, 10, 10);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s", gettext("Make Disk Image") );
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
        pVDisk->b2DD = FALSE; 
        pVDisk->bFormat = FALSE;
        pVDisk->name[0] = NULL;
  
        textbox = AG_TextboxNew(w,  AG_TEXTBOX_STATIC | AG_TEXTBOX_HFILL, "%s:", gettext("Title"));
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXXXXXXX");
        strcpy(pVDisk->name, "Default");
	AG_TextboxPrintf(textbox, pVDisk->name);
	AG_TextboxBindUTF8(textbox, &(pVDisk->name[0]), sizeof(pVDisk->name));
	AG_WidgetFocus(AGWIDGET(textbox));
   
	check = AG_CheckboxNewInt(w, 0, gettext("2DD"), &(pVDisk->b2DD));
	check = AG_CheckboxNewInt(w, 0, gettext("Disk Format"), &(pVDisk->bFormat));

	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 16);
	box2 = AG_BoxNewVert(box, 0);

	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 200);
    dlg = AG_FileDlgNew(w, FILEDLG_DEFAULT | AG_FILEDLG_SAVE);
	AG_FileDlgSetDirectory (dlg, "%s", InitialDir[0]);
	AG_FileDlgAddType(dlg, "D77 Image File", "*.d77,*.D77", OnNewDiskCreate, "%p", pVDisk);
    AG_FileDlgCancelAction (dlg, OnPushCancelDisk, "%p", pVDisk);
    AG_ActionFn(AGWIDGET(w), "window-close", OnPushCancelDisk, "%p", pVDisk);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancelDisk, "%p", pVDisk);

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

    //	AG_FILEDLG_CLOSEWIN指定してるので後始末要らない
}

static void OnNewTape(AG_Event *event)
{
	AG_Window *w;
	AG_Box *box;
	AG_Label *lbl;
	AG_FileDlg *dlg;


	if((InitialDir[1] == NULL) || strlen(InitialDir[1]) <= 0) {
		strcpy(InitialDir[1], "./");
	}

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 280, 120);
    AG_WindowSetPadding(w, 10, 10, 10, 10);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s", gettext("Make Disk Image") );
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 16);
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 280, 200);
    dlg = AG_FileDlgNew(w, FILEDLG_DEFAULT | AG_FILEDLG_SAVE);
	AG_FileDlgSetDirectory (dlg, "%s", InitialDir[1]);
	AG_FileDlgAddType(dlg, "T77 Image File", "*.t77,*.T77", OnNewTapeCreate, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_ActionFn(AGWIDGET(w), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
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
	if((InitialDir[3] == NULL) || strlen(InitialDir[3]) <= 0) {
		strcpy(InitialDir[3], "./");
	}

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 280, 120);
	dlg = AG_FileDlgNew(w, FILEDLG_DEFAULT | AG_FILEDLG_SAVE);
	if(dlg == NULL) return;
	AG_FileDlgSetDirectory(dlg, "%s",InitialDir[3]);
	AG_FileDlgAddType(dlg, "BMP file","*.bmp,*.BMP", ScreenCaptureSub, NULL);
    AG_ActionFn(AGWIDGET(w), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
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
	if((InitialDir[3] == NULL) || strlen(InitialDir[3]) <= 0) {
		strcpy(InitialDir[3], "./");
	}

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 280, 120);
	dlg = AG_FileDlgNew(w, FILEDLG_DEFAULT | AG_FILEDLG_SAVE);
	AG_FileDlgSetDirectory(dlg, "%s",InitialDir[3]);
	AG_FileDlgAddType(dlg, "BMP file","*.bmp,*.BMP", ScreenCaptureSub2, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_ActionFn(AGWIDGET(w), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
    AG_WidgetFocus(dlg);
	AG_WindowSetCaption(w, gettext("Capture to BMP without Scaling."));
	AG_WindowShow(w);
}

static void OnWavCaptureSub(AG_Event *event)
{
	AG_Button *btn = (AG_Button *)AG_SELF();
	char *sFilename = AG_STRING(1);
	char *p;

	if((sFilename == NULL) || (strlen(sFilename) <= 0)) return;
	/*
	 * WAV取り込みはCapture中に新しいファイルを作らない
	 */
    if (!bWavCapture) {
		LockVM();
		StopSnd();
	        OpenCaptureSnd(sFilename);
		PlaySnd();
		ResetSch();
		UnlockVM();
    } 
	
    p = strrchr(sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[4], sFilename);
    }
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
	if(bWavCapture) {
		LockVM();
//	        bWavCapture = FALSE;
		CloseCaptureSnd();
		UnlockVM();
		// ダイアログ表示入れる？
		return;
	}

	/*
	 * ファイル選択
	 */
	if((InitialDir[4] == NULL) || strlen(InitialDir[4]) <= 0) {
		strcpy(InitialDir[4], "./");
	}
	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 280, 120);
	dlg = AG_FileDlgNew(w, FILEDLG_DEFAULT | AG_FILEDLG_SAVE);
	AG_FileDlgSetDirectory(dlg, "%s",InitialDir[4]);
	AG_FileDlgAddType(dlg, "Wav Sound","*.wav,*.WAV", OnWavCaptureSub, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_ActionFn(AGWIDGET(w), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
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
	item = AG_MenuAction(parent , gettext("Capture WAV"), NULL, OnWavCapture, NULL);
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

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

extern AG_MenuItem *Menu_File;
extern AG_MenuItem *Menu_Drive0;
extern AG_MenuItem *Menu_Drive1;
extern AG_MenuItem *Menu_DiskImages[FDC_DRIVES];

extern AG_MenuItem *Menu_Tape;
extern AG_MenuItem *Menu_Debug;
extern AG_MenuItem *Menu_Tools;
extern AG_MenuItem *Menu_Help;
extern AG_MenuItem *Menu_About;

static Disk   disk[2][FDC_MEDIAS];
static char     StatePath[MAXPATHLEN];
static char    DiskTitle[16 + 1];
static BOOL    DiskMedia;
static BOOL    DiskFormat;

//static guint   hidWavCapture;

#ifdef MOUSE
//static guint   hidMouseCapture;
#endif				/*  */


static AG_MenuItem *drive_menu[2];
static AG_MenuItem *drive_item[2];
static AG_MenuItem *midrive_open[2];
static AG_MenuItem *midrive_openboth[2];
static AG_MenuItem *midrive_eject[2];
static AG_MenuItem *midrive_teject[2];
static AG_MenuItem *midrive_writep[2];
static AG_MenuItem *midrive_medias[2][FDC_MEDIAS];
static AG_MenuItem *tape_menu;
static AG_MenuItem *tape_item;
static AG_MenuItem *mitape_open;
static AG_MenuItem *mitape_eject;
static AG_MenuItem *mitape_sep[2];
static AG_MenuItem *mitape_rew;
static AG_MenuItem *mitape_ff;
static AG_MenuItem *mitape_rec;
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

static void OnPushCancel(AG_Event *event)
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
    OnLoadStatusSub(sFilename);
}


void OnLoadStatus(AG_Event *event)
{
    AG_Window *dlgWin;
    AG_FileDlg *dlg;
    dlgWin = AG_WindowNew(0);
    if(dlgWin == NULL) return;
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_ASYNC|AG_FILEDLG_CLOSEWIN);
    if(dlg == NULL) return;
    KeyBoardSnoop(TRUE);
    AG_FileDlgSetDirectory(dlg, InitialDir[2]);
    AG_FileDlgAddType(dlg, "XM7 Status", "*.xm7,*.XM7", OnLoadStatusSubEv, NULL);
    AG_WidgetFocus(dlg);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
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
    if(filename == NULL) return;
	/*
	 * ファイル選択
	 */

    /*
     * ステートセーブ
     */
    LockVM();
    StopSnd();
    SDL_Delay(100);		/* テスト */
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
    dlgWin = AG_WindowNew(0);
//    dlgWin = MainWindow;
    if(dlgWin == NULL) return;
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC|AG_FILEDLG_CLOSEWIN);
    if(dlg == NULL) return;
    KeyBoardSnoop(TRUE);
    AG_FileDlgSetDirectory(dlg, InitialDir[2]);
    AG_FileDlgAddType(dlg, "XM7 Status", "*.xm7,*.XM7", OnSaveStatusSubEv, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
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


/*
 *  「ファイル」メニュー生成
 */
/*-[ ディスクメニュー ]-----------------------------------------------------*/
void OnSelectDiskImage(AG_Event *event)
{
	AG_Menu *self = (AG_Menu *)AG_SELF();
	AG_MenuItem *itemr = (AG_MenuItem *)AG_SENDER();

	int Drive = AG_INT(1);

}

static void OnOpenDiskSub(int Drive, char *sFilename)
{
	char *p;
    /*
     * セット
     */
    LockVM();
    fdc_setdisk(Drive, sFilename);
    ResetSch();
    UnlockVM();
    p = strrchr(sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[0], sFilename);
    }
}


static void OnOpenDiskSubEv(AG_Event *event)
{
    AG_FileDlg *dlg = (AG_FileDlg *)AG_SELF();
    char  *sFilename = AG_STRING(2);
    AG_FileType *ft = (AG_FileType *)AG_PTR(3);
    AG_MenuItem *parent;
    int Drv = AG_INT(1);

    if(Drv == 0) {
    	parent = Menu_Drive0;
    } else if(Drv == 1) {
    	parent = Menu_Drive1;
    } else {
    	parent = NULL;
    }

    OnOpenDiskSub(Drv, sFilename);
    KeyBoardSnoop(FALSE);
}

static void OnOpenDiskBothSub(char *sFilename)
{
	char *p;
    LockVM();
    fdc_setdisk(0, sFilename);
    fdc_setdisk(1, NULL);
    if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
	fdc_setdisk(1, sFilename);
	fdc_setmedia(1, 1);
    }
    ResetSch();
    UnlockVM();
    p = strrchr(sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[0], sFilename);
    }

}

static void OnOpenDiskBothSubEv(AG_Event *event)
{
    AG_FileDlg *dlg = (AG_FileDlg *)AG_SELF();
    char  *sFilename = AG_STRING(1);
    AG_FileType *ft = (AG_FileType *)AG_PTR(2);

    OnOpenDiskBothSub(sFilename);
    KeyBoardSnoop(FALSE);
}


void OnOpenDisk(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *dlgWin;
	AG_FileDlg *dlg;
	int Drive = AG_INT(1);
	dlgWin = AG_WindowNew(0);
	if(dlgWin == NULL) return;
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC|AG_FILEDLG_CLOSEWIN);
	if(dlg == NULL) return;
	AG_FileDlgSetDirectory (dlg, "%s", InitialDir[0]);
	KeyBoardSnoop(TRUE);
	AG_WidgetFocus(dlg);
	AG_FileDlgAddType(dlg, "D77 Disk Image File", "*.d77,*.D77", OnOpenDiskSubEv, "%i", Drive);
	AG_FileDlgAddType(dlg, "D88 Disk Image File", "*.d88,*.D88", OnOpenDiskSubEv, "%i", Drive);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_WindowShow(dlgWin);
}

void OnOpenDiskBoth(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *dlgWin;
	AG_FileDlg *dlg;
//	int Drive = AG_INT(1);
	dlgWin = AG_WindowNew(0);
	if(dlgWin == NULL) return;
	dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC|AG_FILEDLG_CLOSEWIN);
	if(dlg == NULL) return;
	AG_FileDlgSetDirectory (dlg, "%s", InitialDir[0]);
	KeyBoardSnoop(TRUE);
	AG_WidgetFocus(dlg);
	AG_FileDlgAddType(dlg, "D77 Disk Image File", "*.d77,*.D77", OnOpenDiskBothSubEv, NULL);
	AG_FileDlgAddType(dlg, "D88 Disk Image File", "*.d88,*.D88", OnOpenDiskBothSubEv, NULL);
	AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
	AG_WindowShow(dlgWin);
}


void DisplayWriteProtectDisk(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
	char Label[128];
	int Drive = AG_INT(1);

	strcpy(Label,gettext("Write Protect"));
	strcat(Label, " ");
	if(fdc_writep[Drive]) {
		strcat(Label, "ON");
	} else {
		strcat(Label, "Off");
	}
	AG_MenuSetLabel(item, Label);
}


static void SetWriteProtectDisk(AG_Event *event)
{
	BOOL flag = AG_INT(1);
	int Drive = AG_INT(2);
	AG_Button *self = (AG_Button *)AG_SELF();

    LockVM();
	fdc_writep[Drive] = flag;
    ResetSch();
    UnlockVM();
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}

void OnWriteProtectDisk(AG_Event *event)
{
	AG_Menu *self = (AG_Menu *)AG_SELF();
	AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
	AG_Window *w;
	AG_Button   *btn[3];
	AG_Box *box;
	AG_Box *box2;
	char Label[128];
	int Drive = AG_INT(1);
	char *caption;
	AG_Label *lbl;
	int id;

	if(fdc_ready[Drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	sprintf(Label, "Drive %d:", Drive);
	caption = gettext("Switch Write Protect on");

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(w, 230, 80);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s %d:", caption, Drive );
	AG_LabelSizeHint (lbl, 2, caption);
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 8);
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);
	box2 = AG_BoxNewVert(box, 0);
	btn[0] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("ON"), SetWriteProtectDisk, "%i,%i", TRUE, Drive);
	box2 = AG_BoxNewVert(box, 0);
	btn[1] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("OFF"), SetWriteProtectDisk, "%i,%i", FALSE, Drive);
	box2 = AG_BoxNewVert(box, 0);
	btn[2] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("Cancel"), OnPushCancel, NULL);

	AG_WindowSetCaption(w, gettext(Label));
	AG_WindowShow(w);
}

void OnSelectDiskMedia(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	int Drive = AG_INT(1);
	AG_Window *w;
	AG_Box *box;
	AG_Box *box2;
	AG_Label *lbl;
	AG_Button *btn;
	char *caption;
	int i;

	if(fdc_ready[Drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	caption = gettext("Select Disk Image on ");
	w = AG_WindowNew(0);
	AG_WindowSetMinSize(w, 250, 80);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s %d:", caption, Drive );
	AG_LabelSizeHint (lbl, 2, caption);
	{
	 char utf8[256];
	 char          *pIn, *pOut;
	 iconv_t       hd;
	 size_t        in, out;

	 for(i = 0; i < fdc_medias[Drive]; i++) {
		 box2 = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
         pIn =  fdc_name[Drive][i];;
         pOut = utf8;
          in = strlen(pIn);
          out = 256;
          hd = iconv_open("utf8", "cp932");
          if(hd >= 0) {
                  while(in>0) {
                          iconv(hd, &pIn, &in, &pOut, &out);
                  }
                  iconv_close(hd);
          }
		 btn = AG_ButtonNewFn (AGWIDGET(box2), 0, utf8 , OnMediaChange, "%i,%i",  Drive, i);

		}
	}
	AG_WindowSetCaption(w, gettext("Select Image"));
	AG_WindowShow(w);
}


/*
 *  ディスク(1)(0)メニュー更新
 */


/*-[ テープメニュー ]-------------------------------------------------------*/

    /*
     *  テープ(A)メニュー更新
     */
void OnTapePopup(void)
{
#if 0
	/*
	 * メニューすべて削除
	 */
    gtk_widget_hide(mitape_open);
    gtk_widget_hide(mitape_eject);
    gtk_widget_hide(mitape_sep[0]);
    gtk_widget_hide(mitape_sep[1]);
    gtk_widget_hide(mitape_rew);
    gtk_widget_hide(mitape_ff);
    gtk_widget_hide(mitape_rec);

	/*
	 * オープン
	 */
	gtk_widget_show(mitape_open);

	/*
	 * テープがセットされていなければ、ここまで
	 */
	if (tape_fileh == -1) {
	return;
    }

	/*
	 * イジェクト
	 */
	gtk_widget_show(mitape_eject);

	/*
	 * セパレータ挿入
	 */
	gtk_widget_show(mitape_sep[0]);

	/*
	 * 巻き戻し
	 */
	gtk_widget_show(mitape_rew);

	/*
	 * 早送り
	 */
	gtk_widget_show(mitape_ff);

	/*
	 * セパレータ挿入
	 */
	gtk_widget_show(mitape_sep[1]);

	/*
	 * 録音
	 */
	if (tape_writep) {
	gtk_widget_set_sensitive(mitape_rec, FALSE);
    } else {
	gtk_widget_set_sensitive(mitape_rec, TRUE);
	if (tape_rec) {
	    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
					    (mitape_rec), TRUE);
	} else {
	    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
					    (mitape_rec), FALSE);
	}
    }
    gtk_widget_show(mitape_rec);
#endif
}

#if 0
    /*
     *  テープオープン
     */
void
OnTapeOpen(GtkWidget * widget, gpointer data)
{
    char          *p;

	/*
	 * ファイル選択
	 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[1]);
    if (dlg.bResult != DLG_OK) {
            return;
    }

	/*
	 * セット
	 */
	LockVM();
    tape_setfile(dlg.sFilename);
    ResetSch();
    UnlockVM();
    p = strrchr(dlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[1], dlg.sFilename);
    }
}
/*
 *  「テープ」メニューを作成
 */
void
CreateTapeMenu(GtkBuilder *gbuilder)
{

	/*
	 * テープメニューの作成
	 */
	tape_menu = gtk_menu_new();

	/*********************************************************/

	/*
	 * 「開く」ボタンを作成
	 */
	//mitape_open = gtk_menu_item_new_with_label("開く");
    //gtk_menu_append(GTK_MENU(tape_menu), mitape_open);
    mitape_open = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tape_Open"));
    gtk_signal_connect(GTK_OBJECT(mitape_open), "activate",
			GTK_SIGNAL_FUNC(OnTapeOpen),
                       NULL);

    gtk_widget_show (mitape_open);

	/*********************************************************/

	/*
	 * 「取り外す」ボタンを作成
	 */
    mitape_eject = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tape_eject"));
    gtk_signal_connect(GTK_OBJECT(mitape_eject), "activate",
			GTK_SIGNAL_FUNC(OnTapeEject),
                       NULL);
    gtk_widget_show(mitape_eject);

	/*********************************************************/

	/*
	 * 「巻き戻し」ボタンを作成
	 */
	mitape_rew = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tape_rewind"));
    gtk_signal_connect(GTK_OBJECT(mitape_rew), "activate",
                       GTK_SIGNAL_FUNC(OnRew), NULL);
    gtk_widget_show(mitape_rew);

	/*********************************************************/

	/*
	 * 「早送り」ボタンを作成
	 */
	mitape_ff = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tape_FastForward"));
    gtk_signal_connect(GTK_OBJECT(mitape_ff), "activate",
			GTK_SIGNAL_FUNC(OnFF), NULL);

    gtk_widget_show (mitape_ff);

	/*********************************************************/


	/*
	 * 「録音」ボタンを作成
	 */
	mitape_rec = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tape_record"));
    gtk_signal_connect(GTK_OBJECT(mitape_rec), "activate",
			GTK_SIGNAL_FUNC(OnRec), NULL);

	gtk_widget_show (mitape_rec);

	/*********************************************************/
    tape_menu = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tape"));
    gtk_signal_connect (GTK_OBJECT(tape_menu), "activate",
                        GTK_SIGNAL_FUNC (OnTapePopup), NULL);

	return;
}
#endif

/*-[ ヘルプメニュー ]-----------------------------------------------------*/

#if 0
/*
    *  バージョン情報
     */
void
OnVersion(GtkWidget * widget, gpointer data)
{
    GtkWidget * dlgVersion;
    GtkWidget * dialog_vbox;
    GtkWidget * hbox;
    GtkWidget * image;
    GtkWidget * textviewProduct;
    GtkWidget * textviewAuthor;
    GtkWidget * dialog_action_area;
    GtkWidget * okbutton;
    char           icon_path[MAXPATHLEN];
    dlgVersion = gtk_dialog_new();
    gtk_widget_set_usize(dlgVersion, 300, 320);
    gtk_window_set_resizable(GTK_WINDOW(dlgVersion), FALSE);
    gtk_window_set_title(GTK_WINDOW(dlgVersion),
			  "XM7バージョン情報");
    gtk_window_set_modal(GTK_WINDOW(dlgVersion), TRUE);
    dialog_vbox = GTK_DIALOG(dlgVersion)->vbox;
    gtk_widget_show(dialog_vbox);
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(dialog_vbox), hbox, FALSE, FALSE, 0);
#ifdef RSSDIR
    strcpy(icon_path, RSSDIR);
#else
    strcpy(icon_path, ModuleDir);
#endif
    switch (fm7_ver) {
    case 1:
	strcat(icon_path, "tamori.ico");
	break;
    case 2:
	strcat(icon_path, "app_av.ico");
	break;
    case 3:
	strcat(icon_path, "app_ex.ico");
	break;
    default:
	icon_path[0] = '\0';
    }
#ifdef RSSDIR
    if (icon_path[0] != '\0' && strcmp(icon_path, RSSDIR) != 0) {
	image = gtk_image_new_from_file(icon_path);
    } else {
	image =
	    gtk_image_new_from_stock("gtk-dialog-info",
				     GTK_ICON_SIZE_DIALOG);
    }
#else
    if (icon_path[0] != '\0' && strcmp(icon_path, ModuleDir) != 0) {
	image = gtk_image_new_from_file(icon_path);
    } else {
	image =
	    gtk_image_new_from_stock("gtk-dialog-info",
				     GTK_ICON_SIZE_DIALOG);
    }
#endif
    gtk_widget_show(image);
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, TRUE, 0);
    textviewProduct = gtk_text_view_new();
    gtk_widget_show(textviewProduct);
    gtk_box_pack_start(GTK_BOX(hbox), textviewProduct, TRUE, TRUE, 0);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textviewProduct), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textviewProduct),
				      FALSE);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(textviewProduct),
				     GTK_JUSTIFY_CENTER);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textviewProduct),
				 GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textviewProduct),
				      FALSE);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer
			      (GTK_TEXT_VIEW(textviewProduct)), VERSTR,
			      -1 );
    textviewAuthor = gtk_text_view_new();
    gtk_widget_show(textviewAuthor);
    gtk_box_pack_start(GTK_BOX(dialog_vbox), textviewAuthor, TRUE, TRUE,
			0);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textviewAuthor), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textviewAuthor),
				      FALSE);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(textviewAuthor),
				     GTK_JUSTIFY_CENTER);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textviewAuthor),
				 GTK_WRAP_WORD);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textviewAuthor),
				      FALSE);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer
			      (GTK_TEXT_VIEW(textviewAuthor)), AUTSTR,
			      -1 );
    dialog_action_area = GTK_DIALOG(dlgVersion)->action_area;
    gtk_widget_show(dialog_action_area);
    okbutton = gtk_button_new_from_stock("gtk-ok");
    gtk_widget_show(okbutton);
    gtk_dialog_add_action_widget(GTK_DIALOG(dlgVersion), okbutton,
				  GTK_RESPONSE_OK);
    GTK_WIDGET_SET_FLAGS(okbutton, GTK_CAN_DEFAULT);
    gtk_signal_connect_object(GTK_OBJECT(okbutton), "clicked",
				GTK_SIGNAL_FUNC(gtk_widget_destroy),
				dlgVersion);
    gtk_widget_show(dlgVersion);
}


    /*
     *  「ヘルプ」メニューを作成
     */
void
CreateHelpMenu(GtkBuilder *gbuilder)
{
    GtkWidget * sub_item;

/*
 * ヘルプメニューの作成
 */
	help_menu = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Help"));

	/*********************************************************/

	/*
	 * 「バージョン情報」ボタンを作成
	 */
    sub_item = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Help_About"));
    gtk_signal_connect(GTK_OBJECT(sub_item), "activate",
			GTK_SIGNAL_FUNC(OnVersion), NULL);
    gtk_widget_show(sub_item);

    return;
}


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
 *  「デバッグ」メニューを作成
 */
void
CreateDebugMenu (GtkBuilder *gbuilder)
{

    GtkWidget *sub_item;
    GSList *ModeGroup = NULL;
    GtkWidget *debug_menu, *file_item;
    debug_menu = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu5"));


	/*
	 * デバッグメニューの作成
	 */
    /*
     * 停止
     */
    debug_stop = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Debug_STOP"));
    gtk_signal_connect(GTK_OBJECT(debug_stop), "activate",
			GTK_SIGNAL_FUNC(OnBreak),
                       NULL);

    gtk_widget_show (debug_stop);
/*
 * 再開
 */
    debug_restart = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Debug_Restart"));
    gtk_signal_connect(GTK_OBJECT(debug_restart), "activate",
			GTK_SIGNAL_FUNC(OnExec),
                       NULL);

    gtk_widget_show (debug_restart);
	/*********************************************************/
    /* 「逆アセンブル」ボタンを作成 */
    sub_item = gtk_radio_menu_item_new_with_label (ModeGroup, "逆アセンブル - メイン");

    gtk_menu_append (GTK_MENU(debug_menu), sub_item);
    gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
            GTK_SIGNAL_FUNC (OnMainDisAsmPopup), wndMain);
    gtk_widget_show (sub_item);
//    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (sub_item), TRUE);

	/*********************************************************/
    /* 「逆アセンブル」ボタンを作成 */
    sub_item = gtk_radio_menu_item_new_with_label (ModeGroup, "逆アセンブル - サブ");

    gtk_menu_append (GTK_MENU(debug_menu), sub_item);
    gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
            GTK_SIGNAL_FUNC (OnSubDisAsmPopup), wndMain);
    gtk_widget_show (sub_item);
//    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (sub_item), TRUE);

	/*********************************************************/
    gtk_signal_connect (GTK_OBJECT(debug_menu), "activate",
                        GTK_SIGNAL_FUNC (OnDebugPopup), NULL);



	return;
}

    /*
     *  新規ディスク作成(D)
     */
static void
OnNewDisk(GtkWidget * widget, gpointer data)
{
    char          *p;
    BOOL err;
    DiskImageDialog ddlg;
    FileSelectDialog fdlg;

	/*
	 * タイトル入力
	 */
    ddlg = OpenDiskImageDialog();
    if (ddlg.bResult != DLG_OK) {
	return;
    }
    strcpy(DiskTitle, ddlg.sTitle);
    DiskFormat = ddlg.bUserDisk;

#if XM7_VER >= 3
	DiskMedia = ddlg.b2DDDisk;

#else				/*  */
	DiskMedia = FALSE;

#endif				/*  */

	/*
	 * ファイル選択
	 */
	fdlg = OpenFileSelectDialog(InitialDir[0]);
    if (fdlg.bResult != DLG_OK) {
	return;
    }

	/*
	 * 作成
	 */
	LockVM();
    StopSnd();
    if (DiskFormat) {
	err = make_new_userdisk(fdlg.sFilename, DiskTitle, DiskMedia);
    }

    else {
	err = make_new_d77(fdlg.sFilename, DiskTitle, DiskMedia);
    }
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(fdlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[0], fdlg.sFilename);
    }
}


    /*
     *  新規テープ作成(T)
     */
static void
OnNewTape(GtkWidget * widget, gpointer data)
{
    char          *p;

	/*
	 * ファイル選択
	 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[1]);
    if (dlg.bResult != DLG_OK) {
	return;
    }

	/*
	 * 作成
	 */
	LockVM();
    StopSnd();
    if (make_new_t77(dlg.sFilename)) {
    }
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(dlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[1], dlg.sFilename);
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

    /*
     *  ツールメニュー更新
     */
void
OnToolPopup(GtkWidget * widget, gpointer data)
{

#ifdef MOUSE
	gtk_signal_handler_block(GTK_OBJECT(miMouseCapture),
				 hidMouseCapture);
    if (mos_capture) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
					(miMouseCapture), TRUE);
    } else {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM
					(miMouseCapture), FALSE);
    }
    gtk_signal_handler_unblock(GTK_OBJECT(miMouseCapture),
				hidMouseCapture);

#endif				/*  */
}


/*
 *  「ツール」メニューを作成
 */
static void
CreateToolMenu(GtkBuilder *gbuilder)
{
    GtkWidget * sub_item;

    /*
     * ヘルプメニューの作成
     */
    tool_menu = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tool"));

/*********************************************************/

/*
 * 「設定」ボタンを作成
 */
    sub_item = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_properties"));
    g_signal_connect(GTK_OBJECT(sub_item), "activate",
                       GTK_SIGNAL_FUNC(OnConfig), NULL);
    gtk_widget_show(sub_item);
    sub_item = GTK_WIDGET(gtk_builder_get_object(gbuilder, "button_BAR_Options"));
    g_signal_connect(G_OBJECT(sub_item), "clicked",
                       GTK_SIGNAL_FUNC(OnConfig), NULL);


	/*********************************************************/
#ifdef MOUSE
/*
 * 「マウスモード」ボタンを作成
 */
    miMouseCapture = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_mouse"));
    //gtk_menu_append(GTK_MENU(tool_menu), miMouseCapture);
    hidMouseCapture =
	g_signal_connect(GTK_OBJECT(miMouseCapture), "activate",
			   GTK_SIGNAL_FUNC(OnMouseMode), NULL);
    //gtk_widget_show(miMouseCapture);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miMouseCapture),
                                   FALSE);

#endif				/*  */
    /*********************************************************/

    /*
     * 「時刻アジャスト」ボタンを作成
     */
    sub_item = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_adjust"));
    g_signal_connect(GTK_OBJECT(sub_item), "activate",
                       GTK_SIGNAL_FUNC(OnTimeAdjust), NULL);
    /*********************************************************/

    /*
     * 「画面キャプチャ」ボタンを作成
     */
    sub_item = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_Capture"));
    g_signal_connect(GTK_OBJECT(sub_item), "activate",
			GTK_SIGNAL_FUNC(OnGrpCapture), NULL);


/*********************************************************/

/*
 * 「縮小画像キャプチャ」ボタンを作成
 */
    sub_item = GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_Capturemini"));
    g_signal_connect(GTK_OBJECT(sub_item), "activate",
			GTK_SIGNAL_FUNC(OnGrpCapture2), NULL);


    /*********************************************************/

    /*
     * 「WAVキャプチャ」ボタンを作成
     */
    miWavCapture =
            GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_WAVCapture"));
    hidWavCapture =
            g_signal_connect(GTK_OBJECT(miWavCapture), "activate",
                               GTK_SIGNAL_FUNC(OnWavCapture), NULL);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miWavCapture),
                                   FALSE);

    /*********************************************************/

    /*
     * 「新規ディスク作成」ボタンを作成
     */
    sub_item =
            GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_NewD77"));
    g_signal_connect(GTK_OBJECT(sub_item), "activate",
			GTK_SIGNAL_FUNC(OnNewDisk), NULL);

    /*********************************************************/

    /*
     * 「新規テープ作成」ボタンを作成
     */
    sub_item =
            GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_NewTape"));

    g_signal_connect(GTK_OBJECT(sub_item), "activate",
			GTK_SIGNAL_FUNC(OnNewTape), NULL);

    /*********************************************************/

    /*
     * 「VFD→D77変換」ボタンを作成
     */
    sub_item =
            GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_CVTVFD"));
    g_signal_connect(GTK_OBJECT(sub_item), "activate",
                       GTK_SIGNAL_FUNC(OnVFD2D77), NULL);

	/*********************************************************/

	/*
	 * 「2D→D77変換」ボタンを作成
	 */
    sub_item =
            GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_CVT2D"));

    g_signal_connect(GTK_OBJECT(sub_item), "activate",
                       GTK_SIGNAL_FUNC(On2D2D77), NULL);


    /*********************************************************/

    /*
     * 「VTP→T77変換」ボタンを作成
     */
    sub_item =
            GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tools_CVTVTP"));
    g_signal_connect(GTK_OBJECT(sub_item), "activate",
                       GTK_SIGNAL_FUNC(OnVTP2T77), NULL);


    /*********************************************************/
    /*********************************************************/

    /*
     * ツールメニューをのせるメニューアイテムの作成
     */
    tool_item =
            GTK_WIDGET(gtk_builder_get_object(gbuilder, "menu_Tool"));
    //  gtk_widget_show(tool_item);

    g_signal_connect(GTK_OBJECT(tool_item), "activate",
                       GTK_SIGNAL_FUNC(OnToolPopup), NULL);

    /*********************************************************/
    return;
}

/*-[ メニューバー ]-----------------------------------------------------*/
#endif
/*
 *  メニューバーの生成
 */
void CreateMenu(void)
{
    memset(InitialDir, 0x00, 5 * MAXPATHLEN);
/*
 * 「ファイル」メニューを作成する関数を呼び出す
 */
    //CreateFileMenu();
/*
 * 「ドライブ0」メニューを作成する関数を呼び出す
 */
//    CreateDiskMenu_0();
/*
 * 「ドライブ1」メニューを作成する関数を呼び出す
 */
//    CreateDiskMenu_1();
/*
 * 「テープ」メニューを作成する関数を呼び出す
 */
#if 0
    CreateTapeMenu(gbuilderMain);

/*
 * 「デバッグ」メニューを作成する関数を呼び出す
 */
	CreateDebugMenu(gbuilderMain);

/*
 * 「ツール」メニューを作成する関数を呼び出す
 */
    CreateToolMenu(gbuilderMain);

    /*
     * 「ヘルプ」メニューを作成する関数を呼び出す
     */
    CreateHelpMenu(gbuilderMain);
#endif
}

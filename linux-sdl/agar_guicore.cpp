/*
 * agar_guicore.cpp
 *
 *  Created on: 2010/11/08
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
//#include "sdl_gtkdlg.h"
#include "agar_toolbox.h"

extern "C" {
void InitInstance(void);
void OnDestroy(AG_Event *event);
void OnDestroy2(void);
extern void InitGL(int w, int h);
extern void OnReset(AG_Event *event);
extern void OnHotReset(AG_Event *event);

AG_Window *MainWindow;
AG_Menu *MenuBar;
AG_GLView *DrawArea;
}

extern int DrawThreadMain(void *);
extern void AGEventDrawGL(AG_Event *event);
extern void AGEventScaleGL(AG_Event *event);
extern void AGEventOverlayGL(AG_Event *event);


static BOOL bKeyboardSnooped;


static void Create_AGMainBar(AG_Widget *Parent);
static void Create_FileMenu(void);

void KeyBoardSnoop(BOOL Flag)
{
	bKeyboardSnooped = Flag;
}

void EventGuiSingle(AG_Driver *drv, AG_DriverEvent *ev)
{
	switch (ev->type) {
#if 1
	case AG_DRIVER_KEY_UP:
		if(	!bKeyboardSnooped) {
			OnKeyReleaseAG(ev->data.key.ks, 0, ev->data.key.ucs);
		}
		break;
	case AG_DRIVER_KEY_DOWN:
		if(!bKeyboardSnooped) {
			OnKeyPressAG(ev->data.key.ks, 0, ev->data.key.ucs);
		}
		break;
#endif
	case AG_DRIVER_VIDEORESIZE:
		newDrawWidth = ev->data.videoresize.w;
		newDrawHeight = ev->data.videoresize.h;
		newResize = TRUE;
		break;
	default:
		break;
	}

	/* Forward the event to Agar. */
	if (AG_ProcessEvent(drv, ev) == -1)
		return;

}

void EventGUI(AG_Driver *drv)
{
	AG_DriverEvent ev;

	if(drv == NULL) return;
	 if (AG_PendingEvents(drv) > 0) {
				/*
				 * Case 2: There are events waiting to be processed.
				 */
				do {
					/* Retrieve the next queued event. */
					if (AG_GetNextEvent(drv, &ev) == 1) {
						EventGuiSingle(drv, &ev);
					}
				} while (AG_PendingEvents(drv) > 0);
	 }
}


void ProcessKeyDown(AG_Event *event)
{
	// キーハンドラー
	int sym = AG_INT(1);
	int mod = AG_INT(2);
	Uint32  unicode = (Uint32)AG_ULONG(3);
	OnKeyPressAG(sym, mod, unicode);
}



void ProcessKeyUp(AG_Event *event)
{
	// キーハンドラー
	int sym = AG_INT(1);
	int mod = AG_INT(2);
	Uint32  unicode = (Uint32)AG_ULONG(3);
	OnKeyReleaseAG(sym, mod, unicode);

}


static void FileMenu_Reset(AG_Event *ev)
{

}

static void FileMenu_HotReset(AG_Event *ev)
{

}

static void FileMenu_BootMode(AG_Event *ev)
{

}

static void FileMenu_QuickSave(AG_Event *ev)
{

}

static void FileMenu_QuickLoad(AG_Event *ev)
{

}

static void FileMenu_Load(AG_Event *ev)
{

}

static void FileMenu_Save(AG_Event *ev)
{

}

static void FileMenu_Quit(AG_Event *ev)
{

}

AG_Toolbar *MenuToolBar;
AG_MenuItem *Menu_File;
AG_MenuItem *Menu_Drive0;
AG_MenuItem *Menu_Drive1;
AG_MenuItem *Menu_Tape;
AG_MenuItem *Menu_Debug;
AG_MenuItem *Menu_Tools;
AG_MenuItem *Menu_Help;
AG_MenuItem *Menu_About;




void MainLoop(int argc, char *argv[])
{
	AG_Box *hb;
	/*
	 * エラーコード別
	 */
	switch (nErrorCode) {
	/*
	 * エラーなし
	 */

	case 0:
		/*
		 * 実行開始
		 */
		stopreq_flag = FALSE;
		run_flag = TRUE;
		/*
		 * コマンドライン処理
		 */
		if (argc > 1) {
//			OnCmdLine(argv[1]);
		}
		break;
		/*
		 * VM初期化エラー
		 */
	case 1:
//		OpenErrorMessageDialog("XM7", "仮想マシンを初期化できません");
		break;

		/*
		 * コンポーネント初期化エラー
		 */
	case 2:
//		OpenErrorMessageDialog("XM7","コンポーネントを初期化できません");
		break;
	}
//	AG_AtExitFunc(SDL_Quit);
	/*
	 * Agar のメインループに入る
	 */
    AG_InitCore("xm7", AG_VERBOSE | AG_NO_CFG_AUTOLOAD);
	AG_InitVideo(640, 480, 32, AG_VIDEO_HWSURFACE | AG_VIDEO_DOUBLEBUF |
			AG_VIDEO_RESIZABLE | AG_VIDEO_OPENGL_OR_SDL );
    OnCreate((AG_Widget *)NULL);
	InitInstance();
	bKeyboardSnooped = FALSE;
	stopreq_flag = FALSE;
	run_flag = TRUE;
	DrawThreadMain(NULL);
}

void Create_Drive0Menu(void);
void Create_Drive1Menu(void);

static void Create_AGMainMenu(AG_Widget *parent)
{

}

static void Create_AGMainBar(AG_Widget *Parent)
{
	MenuBar = AG_MenuNew(Parent, AG_MENU_HFILL);

//	MenuBar = AG_MenuNewGlobal(AG_MENU_HFILL);
	if(!MenuBar) return;
#if 1
	AG_LockVFS(AGOBJECT(MenuBar));
	Menu_File = AG_MenuNode(MenuBar->root , "File", NULL);
	Create_FileMenu();

	Menu_Drive1 = AG_MenuNode(MenuBar->root, "Drive 1", NULL);
	Create_Drive1Menu();
	Menu_Drive0 = AG_MenuNode(MenuBar->root, "Drive 0", NULL);
	Create_Drive0Menu();

 	Menu_Tape = AG_MenuNode(MenuBar->root, "Tape", NULL);
 	Menu_Debug = AG_MenuNode(MenuBar->root, "Debug", NULL);
 	Menu_Tools = AG_MenuNode(MenuBar->root, "Tools", NULL);
 	Menu_Help = AG_MenuNode(MenuBar->root, "Help", NULL);
 	Menu_About = AG_MenuNode(MenuBar->root, "About", NULL);
	AG_UnlockVFS(AGOBJECT(MenuBar));
#endif
}

static  AG_MenuItem *Menu_File_QuickSave;
static AG_MenuItem *Menu_File_QuickLoad;
static AG_MenuItem *Menu_File_Save;
static AG_MenuItem *Menu_File_Load;
static AG_MenuItem *Menu_File_Reset;
static AG_MenuItem *Menu_File_HotReset;
static AG_MenuItem *Menu_File_BootMode;
static AG_MenuItem *Menu_File_Quit;


static void Create_FileMenu(void)
{
	Menu_File_QuickSave = AG_MenuAction(Menu_File , "Quick Save", NULL, OnQuickSave, NULL);
	Menu_File_QuickLoad = AG_MenuAction(Menu_File , "Quick Load", NULL, OnQuickLoad, NULL);
	AG_MenuSeparator(Menu_File);
	Menu_File_Save = AG_MenuAction(Menu_File , "Save As...", NULL, OnSaveAs, NULL);
	Menu_File_Load = AG_MenuAction(Menu_File , "Load", NULL, OnLoadStatus, NULL);
	AG_MenuSeparator(Menu_File);
	Menu_File_Reset = AG_MenuAction(Menu_File, "Cold Reset", NULL, OnReset, NULL);
	Menu_File_HotReset = AG_MenuAction(Menu_File, "Hot Reset", NULL, OnHotReset, NULL);
	AG_MenuSeparator(Menu_File);
	Menu_File_Quit = AG_MenuAction(Menu_File , "Quit", NULL, OnDestroy, NULL);

}

extern void OnOpenDisk(AG_Event *event);
extern void OnOpenDiskBoth(AG_Event *event);
extern void OnEjectDisk(AG_Event *event);
extern void OnEjectDiskTemp(AG_Event *event);
extern void OnWriteProtectDisk(AG_Event *event);
extern void OnSelectDiskMedia(AG_Event *event);



void CreateDiskMenu(AG_MenuItem *self, int Drive)
{
	AG_MenuItem *item;

	item = AG_MenuAction(self, gettext("Open"), NULL, OnOpenDisk, "%i", Drive);
	item = AG_MenuAction(self, gettext("Open Both"), NULL, OnOpenDiskBoth, NULL);
	AG_MenuSeparator(self);
	item = AG_MenuAction(self, gettext("Eject"), NULL, OnEjectDisk, "%i", Drive);
	/*
	 * セパレータ挿入
	 */
	AG_MenuSeparator(self);

	/*
	 * 一時取り出し
	 */
//	item = AG_MenuDynamicItem(self, gettext("Eject Temp"), NULL, OnEjectDiskTemp, "%i,%i", Drive, fdc_teject[Drive]);
	/*
	 * ライトプロテクト
	 */
//	item = AG_MenuDynamicItem(self, gettext("Write Protect"), NULL, OnWriteProtectDisk, "%i,%i", Drive, !fdc_fwritep[Drive]);

	/*
	 * セパレータ挿入
	 */
	AG_MenuSeparator(self);

}

void Create_Drive0Menu(void)
{
	CreateDiskMenu(Menu_Drive0, 0);
}

void Create_Drive1Menu(void)
{
	CreateDiskMenu(Menu_Drive1, 1);
}

void Create_TapeMenu(void)
{

}

void Create_DebugMenu(void)
{

}

void Create_ToolsMenu(void)
{

}

void Create_HelpMenu(void)
{

}

void Create_AboutMenu(void)
{

}


void Destroy_AGMainBar(void)
{

}

#ifdef __cplusplus
extern "C" {
#endif


void OnDestroy2(void)
{
	OnDestroy(NULL);
}
void OnDestroy(AG_Event *event)
{

        /*
         * サウンド停止
         */
       StopSnd();
       /*
        * コンポーネント クリーンアップ
        */
#ifdef FDDSND
        CleanFDDSnd();
#endif				/*  */
        CleanSch();
        CleanKbd();
        CleanSnd();
        CleanDraw();
        SaveCfg();
        /*
         * 仮想マシン クリーンアップ
         */
        system_cleanup();
        AG_QuitGUI();
}

void InitInstance(void)
{
	AG_Box *hb;
	MainWindow = AG_WindowNew(AG_WINDOW_NOTITLE |  AG_WINDOW_NOBORDERS | AG_WINDOW_NOBACKGROUND);
	AG_WindowSetGeometry (MainWindow, 0, 0, 640, 480);

    hb = AG_BoxNewHoriz(MainWindow, AG_BOX_EXPAND);
    AG_WidgetSetSize(MainWindow, 640,480);
    AG_WidgetSetSize(hb, 640, 32);

    Create_AGMainBar(AGWIDGET(hb));
    AG_SetEvent(MainWindow , "window-close", OnDestroy, NULL);
    AG_AtExitFunc(OnDestroy2);
	AG_WidgetSetSize(MenuBar, 640, 32);
	AG_WidgetEnable(AGWIDGET(MenuBar));

	DrawArea = AG_GLViewNew(AGWIDGET(MainWindow) , AG_GLVIEW_EXPAND);
	AG_WidgetEnable(DrawArea);
	AG_GLViewSizeHint(DrawArea, 640, 400);
	AG_WidgetSetPosition(DrawArea, 0, 32);
	AG_GLViewDrawFn (DrawArea, AGEventDrawGL, NULL);
	AG_GLViewScaleFn (DrawArea, AGEventScaleGL, NULL);
//	AG_GLViewDrawFn (DrawArea, AGEventDrawGL, NULL);

	//	AG_SetEvent(DrawArea, "key-down" , ProcessKeyDown, NULL);
//	AG_SetEvent(DrawArea, "key-up" , ProcessKeyUp, NULL);
	InitGL(640, 400);
	AG_WindowShow(MainWindow);


}

/*
 *  Update UI
 */
void ui_update(void)
{
	AG_WindowUpdate(MainWindow);
}

void OpenErrorMessageDialog(char *strMessage)
{

}




/*-[ディスクイメージダイアログ]-----------------------------------------------------*/

/*
 *  OKボタンイベント
  */
#ifdef USE_GTK
static void     FASTCALL
OnDiskImageDialogOk(GtkWidget * widget, gpointer user_data)
{
    DiskImageDialog * dlg = (DiskImageDialog *) user_data;
    dlg->bResult = DLG_OK;
    strcpy(dlg->sTitle, gtk_entry_get_text(GTK_ENTRY(dlg->entTitle)));

#if XM7_VER >= 3
	dlg->b2DDDisk = FALSE;
    if (gtk_toggle_button_get_active
	 (GTK_TOGGLE_BUTTON(dlg->radMediaType2DD))) {
	dlg->b2DDDisk = TRUE;
    }

#endif				/*  */
	dlg->bUserDisk =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->chkUserDisk));
    gtk_main_quit();
}


    /*
     *  CENCELボタンイベント
     */
static void     FASTCALL
OnDiskImageDialogCancel(GtkWidget * widget, gpointer user_data)
{
    DiskImageDialog * dlg = (DiskImageDialog *) user_data;
    dlg->bResult = DLG_CANCEL;
    gtk_main_quit();
}
    /*
     *  ディスクイメージダイアログを作成する
     */
    DiskImageDialog FASTCALL OpenDiskImageDialog(void)
{
    DiskImageDialog dlg;
    GtkWidget * vbox1;
    GtkWidget * hbox1;
    GtkWidget * vbox2;
    GtkWidget * alignment1;
    GtkWidget * vbox3;
    GtkWidget * lblTitle;
    GtkWidget * entTitle;
    GtkWidget * frmMediaType;
    GtkWidget * hbox2;

#if XM7_VER >= 3
	GtkWidget * radMediaType2D;
    GtkWidget * radMediaType2DD;
    GSList * radMediaType2D_group = NULL;
    GtkWidget * lblMediaType;

#endif				/*  */
	GtkWidget * vbox4;
    GtkWidget * chkUserDisk;
    GtkWidget * btnOk;
    GtkWidget * btnCancel;
    dlg.bResult = DLG_NONE;
    dlg.dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(dlg.dlg), 4);
    gtk_window_set_title(GTK_WINDOW(dlg.dlg),
			  "新規ディスクイメージ作成");
    gtk_window_set_position(GTK_WINDOW(dlg.dlg), GTK_WIN_POS_CENTER);
    gtk_window_set_modal(GTK_WINDOW(dlg.dlg), TRUE);
    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(dlg.dlg), vbox1);
    hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox1);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 0);
    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox2);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox2, TRUE, TRUE, 0);
    alignment1 = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(alignment1);
    gtk_box_pack_start(GTK_BOX(vbox2), alignment1, FALSE, FALSE, 0);
    vbox3 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox3);
    gtk_container_add(GTK_CONTAINER(alignment1), vbox3);
    lblTitle = gtk_label_new("タイトル16文字まで");
    gtk_widget_show(lblTitle);
    gtk_box_pack_start(GTK_BOX(vbox3), lblTitle, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblTitle), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(lblTitle), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(lblTitle), 0, 5);
    entTitle = gtk_entry_new();
    dlg.entTitle = entTitle;
    gtk_widget_show(entTitle);
    gtk_box_pack_start(GTK_BOX(vbox3), entTitle, FALSE, FALSE, 0);
    gtk_entry_set_text(GTK_ENTRY(entTitle), "Default");
    frmMediaType = gtk_frame_new(NULL);
    gtk_widget_show(frmMediaType);
    gtk_box_pack_start(GTK_BOX(vbox2), frmMediaType, FALSE, FALSE, 0);
    hbox2 = gtk_hbox_new(TRUE, 0);
    gtk_widget_show(hbox2);
    gtk_container_add(GTK_CONTAINER(frmMediaType), hbox2);

#if XM7_VER >= 3
	radMediaType2D = gtk_radio_button_new_with_mnemonic(NULL, "2D");
    dlg.radMediaType2D = radMediaType2D;
    gtk_widget_show(radMediaType2D);
    gtk_box_pack_start(GTK_BOX(hbox2), radMediaType2D, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(radMediaType2D),
				radMediaType2D_group);
    radMediaType2D_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(radMediaType2D));
    radMediaType2DD = gtk_radio_button_new_with_mnemonic(NULL, "2DD");
    dlg.radMediaType2DD = radMediaType2DD;
    gtk_widget_show(radMediaType2DD);
    gtk_box_pack_start(GTK_BOX(hbox2), radMediaType2DD, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(radMediaType2DD),
				radMediaType2D_group);
    radMediaType2D_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(radMediaType2DD));
    lblMediaType = gtk_label_new("メディアタイプ");
    gtk_widget_show(lblMediaType);
    gtk_frame_set_label_widget(GTK_FRAME(frmMediaType), lblMediaType);
    gtk_label_set_justify(GTK_LABEL(lblMediaType), GTK_JUSTIFY_LEFT);
    gtk_misc_set_padding(GTK_MISC(lblMediaType), 0, 5);

#endif				/*  */
	vbox4 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox4);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox4, TRUE, TRUE, 0);
    btnOk = gtk_button_new_with_mnemonic("    OK    ");
    gtk_widget_show(btnOk);
    gtk_box_pack_start(GTK_BOX(vbox4), btnOk, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btnOk), 4);
    btnCancel = gtk_button_new_with_mnemonic("キャンセル");
    gtk_widget_show(btnCancel);
    gtk_box_pack_start(GTK_BOX(vbox4), btnCancel, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btnCancel), 4);
    chkUserDisk =
	gtk_check_button_new_with_mnemonic
	("F-BASICのユーザディスクを作成する");
    dlg.chkUserDisk = chkUserDisk;
    gtk_widget_show(chkUserDisk);
    gtk_box_pack_start(GTK_BOX(vbox1), chkUserDisk, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(chkUserDisk), 8);

	/*
	 * OKボタン押下時のアクションを指定する
	 */
	gtk_signal_connect(GTK_OBJECT(btnOk), "clicked",
			   GTK_SIGNAL_FUNC(OnDiskImageDialogOk),
			   (gpointer) & dlg);

	/*
	 * CANCELボタン押下時のアクションを指定する
	 */
	gtk_signal_connect(GTK_OBJECT(btnCancel), "clicked",
			   GTK_SIGNAL_FUNC(OnDiskImageDialogCancel),
			   (gpointer) & dlg);

	/*
	 * ボタンをクリックしたらdestroy
	 */
	gtk_signal_connect_object(GTK_OBJECT(btnOk), "clicked",
				  GTK_SIGNAL_FUNC(gtk_widget_destroy),
				  (gpointer) dlg.dlg);
    gtk_signal_connect_object(GTK_OBJECT(btnCancel), "clicked",
				GTK_SIGNAL_FUNC(gtk_widget_destroy),
				(gpointer) dlg.dlg);

	/*
	 * ダイアログを表示する
	 */
	gtk_widget_show(dlg.dlg);
    gtk_main();
    return dlg;
}


/*-[ディスクタイトルダイアログ]-----------------------------------------------------*/

    /*
     *  OKボタンイベント
     */
static void     FASTCALL
OnDiskTitleDialogOk(GtkWidget * widget, gpointer user_data)
{
    DiskTitleDialog * dlg = (DiskTitleDialog *) user_data;
    dlg->bResult = DLG_OK;
    strcpy(dlg->sTitle, gtk_entry_get_text(GTK_ENTRY(dlg->entTitle)));
    gtk_main_quit();
}
    /*
     *  CENCELボタンイベント
     */
static void     FASTCALL
OnDiskTitleDialogCancel(GtkWidget * widget, gpointer user_data)
{
    DiskTitleDialog * dlg = (DiskTitleDialog *) user_data;
    dlg->bResult = DLG_CANCEL;
    gtk_main_quit();
}
/*
 *  ディスクタイトルダイアログを作成する
 */
DiskTitleDialog FASTCALL OpenDiskTitleDialog(void)
{
#if 0
	DiskTitleDialog dlg;
    GtkWidget * hbox1;
    GtkWidget * vbox1;
    GtkWidget * lblTitle;
    GtkWidget * entTitle;
    GtkWidget * vbox2;
    GtkWidget * btnOk;
    GtkWidget * btnCancel;
    dlg.bResult = DLG_NONE;
    dlg.dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(dlg.dlg), 4);
    gtk_window_set_title(GTK_WINDOW(dlg.dlg),
			  "ディスクタイトル入力");
    gtk_window_set_position(GTK_WINDOW(dlg.dlg), GTK_WIN_POS_CENTER);
    gtk_window_set_modal(GTK_WINDOW(dlg.dlg), TRUE);
    hbox1 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox1);
    gtk_container_add(GTK_CONTAINER(dlg.dlg), hbox1);
    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox1, FALSE, FALSE, 0);
    lblTitle = gtk_label_new("タイトル16文字まで");
    gtk_widget_show(lblTitle);
    gtk_box_pack_start(GTK_BOX(vbox1), lblTitle, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblTitle), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment(GTK_MISC(lblTitle), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(lblTitle), 0, 5);
    entTitle = gtk_entry_new();
    dlg.entTitle = entTitle;
    gtk_widget_show(entTitle);
    gtk_box_pack_start(GTK_BOX(vbox1), entTitle, FALSE, FALSE, 0);
    gtk_entry_set_text(GTK_ENTRY(entTitle), "Default");
    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox2);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox2, FALSE, FALSE, 0);
    btnOk = gtk_button_new_with_mnemonic("    OK    ");
    gtk_widget_show(btnOk);
    gtk_box_pack_start(GTK_BOX(vbox2), btnOk, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btnOk), 4);
    btnCancel = gtk_button_new_with_mnemonic("キャンセル");
    gtk_widget_show(btnCancel);
    gtk_box_pack_start(GTK_BOX(vbox2), btnCancel, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btnCancel), 4);

	/*
	 * OKボタン押下時のアクションを指定する
	 */
	gtk_signal_connect(GTK_OBJECT(btnOk), "clicked",
			   GTK_SIGNAL_FUNC(OnDiskTitleDialogOk),
			   (gpointer) & dlg);

	/*
	 * CANCELボタン押下時のアクションを指定する
	 */
	gtk_signal_connect(GTK_OBJECT(btnCancel), "clicked",
			   GTK_SIGNAL_FUNC(OnDiskTitleDialogCancel),
			   (gpointer) & dlg);

	/*
	 * ボタンをクリックしたらdestroy
	 */
	gtk_signal_connect_object(GTK_OBJECT(btnOk), "clicked",
				  GTK_SIGNAL_FUNC(gtk_widget_destroy),
				  (gpointer) dlg.dlg);
    gtk_signal_connect_object(GTK_OBJECT(btnCancel), "clicked",
				GTK_SIGNAL_FUNC(gtk_widget_destroy),
				(gpointer) dlg.dlg);

	/*
	 * ダイアログを表示する
	 */
	gtk_widget_show(dlg.dlg);
    gtk_main();
#endif
    return dlg;
}
#endif


#if 0
void
OpenErrorMessageDialog(char *strTitle, char *strMessage)
{
GtkWidget * dlgMessage;
GtkWidget * vbox;
GtkWidget * hbox;
GtkWidget * imgMessage;
GtkWidget * lblMessage;
GtkWidget * actArea;
GtkWidget * btnClose;
dlgMessage = gtk_dialog_new();
gtk_window_set_title(GTK_WINDOW(dlgMessage), strTitle);
gtk_window_set_modal(GTK_WINDOW(dlgMessage), TRUE);
gtk_window_set_resizable(GTK_WINDOW(dlgMessage), FALSE);
vbox = GTK_DIALOG(dlgMessage)->vbox;
gtk_widget_show(vbox);
hbox = gtk_hbox_new(FALSE, 0);
gtk_widget_show(hbox);
gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
imgMessage =
gtk_image_new_from_stock("gtk-dialog-error", GTK_ICON_SIZE_DND);
gtk_widget_show(imgMessage);
gtk_box_pack_start(GTK_BOX(hbox), imgMessage, FALSE, FALSE, 0);
gtk_misc_set_padding(GTK_MISC(imgMessage), 10, 10);
lblMessage = gtk_label_new(strMessage);
gtk_widget_show(lblMessage);
gtk_box_pack_start(GTK_BOX(hbox), lblMessage, TRUE, TRUE, 0);
gtk_label_set_justify(GTK_LABEL(lblMessage), GTK_JUSTIFY_LEFT);
actArea = GTK_DIALOG(dlgMessage)->action_area;
gtk_widget_show(actArea);
btnClose = gtk_button_new_with_mnemonic("閉じる");
gtk_widget_show(btnClose);
gtk_dialog_add_action_widget(GTK_DIALOG(dlgMessage), btnClose, 0);
GTK_WIDGET_SET_FLAGS(btnClose, GTK_CAN_DEFAULT);

/*
 * ボタンをクリックしたらdestroy
 */
gtk_signal_connect_object(GTK_OBJECT(btnClose), "clicked",
			  GTK_SIGNAL_FUNC(gtk_main_quit),
			  (gpointer) dlgMessage);
gtk_widget_show(dlgMessage);
}
/*-[ファイル選択ダイアログ]-----------------------------------------------------*/

/*
 *  OKボタンイベント
 */
static void     FASTCALL
OnFileSelectDialogOk(GtkFileSelection * selector, gpointer user_data)
{
FileSelectDialog * dlg = (FileSelectDialog *) user_data;
G_CONST_RETURN gchar * fname =
gtk_file_selection_get_filename(GTK_FILE_SELECTION(dlg->dlg));
dlg->bResult = DLG_OK;
strcpy(dlg->sFilename, fname);
gtk_main_quit();
}
/*
 *  CANCELボタンイベント
 */
static void     FASTCALL
OnFileSelectDialogCancel(GtkFileSelection * selector, gpointer user_data)
{
FileSelectDialog * dlg = (FileSelectDialog *) user_data;
dlg->bResult = DLG_CANCEL;
dlg->sFilename[0] = '\0';
gtk_main_quit();
}
/*
 *  ファイル選択ダイアログを作成する
 */
FileSelectDialog FASTCALL OpenFileSelectDialog(char *dir)
{

/*
 * 初期化
 */
FileSelectDialog dlg;
dlg.sFilename[0] = '\0';
dlg.bResult = DLG_NONE;

/*
 * ファイル選択ダイアログを生成する
 */
dlg.dlg = gtk_file_selection_new("ファイル選択");
if (dir != NULL) {
if (strlen(dir) > 0) {
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(dlg.dlg),
				     (gchar *) dir);
}
}

/*
 * OKボタン押下時のアクションを指定する
 */
gtk_signal_connect(GTK_OBJECT
		   (GTK_FILE_SELECTION(dlg.dlg)->ok_button),
		   "clicked",
		   GTK_SIGNAL_FUNC(OnFileSelectDialogOk),
		   (gpointer) & dlg);

/*
 * CANCELボタン押下時のアクションを指定する
 */
gtk_signal_connect(GTK_OBJECT
		   (GTK_FILE_SELECTION(dlg.dlg)->cancel_button),
		   "clicked",
		   GTK_SIGNAL_FUNC(OnFileSelectDialogCancel),
		   (gpointer) & dlg);

/*
 * ボタンをクリックしたらdestroy
 */
gtk_signal_connect_object(GTK_OBJECT
			  (GTK_FILE_SELECTION(dlg.dlg)->ok_button),
			  "clicked",
			  GTK_SIGNAL_FUNC(gtk_widget_destroy),
			  (gpointer) dlg.dlg);
gtk_signal_connect_object(GTK_OBJECT
			(GTK_FILE_SELECTION
			 (dlg.dlg)->cancel_button), "clicked",
			GTK_SIGNAL_FUNC(gtk_widget_destroy),
			(gpointer) dlg.dlg);

/*
 * ダイアログを表示する
 */
gtk_widget_show(dlg.dlg);
gtk_main();
return dlg;
}
#endif

/*-[ディスクイメージダイアログ]-----------------------------------------------------*/

/*
 *  OKボタンイベント
 */
#if 0
static void     FASTCALL
OnDiskImageDialogOk(GtkWidget * widget, gpointer user_data)
{
DiskImageDialog * dlg = (DiskImageDialog *) user_data;
dlg->bResult = DLG_OK;
strcpy(dlg->sTitle, gtk_entry_get_text(GTK_ENTRY(dlg->entTitle)));

#if XM7_VER >= 3
dlg->b2DDDisk = FALSE;
if (gtk_toggle_button_get_active
 (GTK_TOGGLE_BUTTON(dlg->radMediaType2DD))) {
dlg->b2DDDisk = TRUE;
}

#endif				/*  */
dlg->bUserDisk =
gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->chkUserDisk));
gtk_main_quit();
}


/*
 *  CENCELボタンイベント
 */
static void     FASTCALL
OnDiskImageDialogCancel(GtkWidget * widget, gpointer user_data)
{
DiskImageDialog * dlg = (DiskImageDialog *) user_data;
dlg->bResult = DLG_CANCEL;
gtk_main_quit();
}
/*
 *  ディスクイメージダイアログを作成する
 */
DiskImageDialog FASTCALL OpenDiskImageDialog(void)
{
DiskImageDialog dlg;
GtkWidget * vbox1;
GtkWidget * hbox1;
GtkWidget * vbox2;
GtkWidget * alignment1;
GtkWidget * vbox3;
GtkWidget * lblTitle;
GtkWidget * entTitle;
GtkWidget * frmMediaType;
GtkWidget * hbox2;

#if XM7_VER >= 3
GtkWidget * radMediaType2D;
GtkWidget * radMediaType2DD;
GSList * radMediaType2D_group = NULL;
GtkWidget * lblMediaType;

#endif				/*  */
GtkWidget * vbox4;
GtkWidget * chkUserDisk;
GtkWidget * btnOk;
GtkWidget * btnCancel;
dlg.bResult = DLG_NONE;
dlg.dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_container_set_border_width(GTK_CONTAINER(dlg.dlg), 4);
gtk_window_set_title(GTK_WINDOW(dlg.dlg),
		  "新規ディスクイメージ作成");
gtk_window_set_position(GTK_WINDOW(dlg.dlg), GTK_WIN_POS_CENTER);
gtk_window_set_modal(GTK_WINDOW(dlg.dlg), TRUE);
vbox1 = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox1);
gtk_container_add(GTK_CONTAINER(dlg.dlg), vbox1);
hbox1 = gtk_hbox_new(FALSE, 0);
gtk_widget_show(hbox1);
gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 0);
vbox2 = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox2);
gtk_box_pack_start(GTK_BOX(hbox1), vbox2, TRUE, TRUE, 0);
alignment1 = gtk_alignment_new(0.5, 0.5, 1, 1);
gtk_widget_show(alignment1);
gtk_box_pack_start(GTK_BOX(vbox2), alignment1, FALSE, FALSE, 0);
vbox3 = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox3);
gtk_container_add(GTK_CONTAINER(alignment1), vbox3);
lblTitle = gtk_label_new("タイトル16文字まで");
gtk_widget_show(lblTitle);
gtk_box_pack_start(GTK_BOX(vbox3), lblTitle, FALSE, FALSE, 0);
gtk_label_set_justify(GTK_LABEL(lblTitle), GTK_JUSTIFY_LEFT);
gtk_misc_set_alignment(GTK_MISC(lblTitle), 0, 0.5);
gtk_misc_set_padding(GTK_MISC(lblTitle), 0, 5);
entTitle = gtk_entry_new();
dlg.entTitle = entTitle;
gtk_widget_show(entTitle);
gtk_box_pack_start(GTK_BOX(vbox3), entTitle, FALSE, FALSE, 0);
gtk_entry_set_text(GTK_ENTRY(entTitle), "Default");
frmMediaType = gtk_frame_new(NULL);
gtk_widget_show(frmMediaType);
gtk_box_pack_start(GTK_BOX(vbox2), frmMediaType, FALSE, FALSE, 0);
hbox2 = gtk_hbox_new(TRUE, 0);
gtk_widget_show(hbox2);
gtk_container_add(GTK_CONTAINER(frmMediaType), hbox2);

#if XM7_VER >= 3
radMediaType2D = gtk_radio_button_new_with_mnemonic(NULL, "2D");
dlg.radMediaType2D = radMediaType2D;
gtk_widget_show(radMediaType2D);
gtk_box_pack_start(GTK_BOX(hbox2), radMediaType2D, FALSE, FALSE, 0);
gtk_radio_button_set_group(GTK_RADIO_BUTTON(radMediaType2D),
			radMediaType2D_group);
radMediaType2D_group =
gtk_radio_button_get_group(GTK_RADIO_BUTTON(radMediaType2D));
radMediaType2DD = gtk_radio_button_new_with_mnemonic(NULL, "2DD");
dlg.radMediaType2DD = radMediaType2DD;
gtk_widget_show(radMediaType2DD);
gtk_box_pack_start(GTK_BOX(hbox2), radMediaType2DD, FALSE, FALSE, 0);
gtk_radio_button_set_group(GTK_RADIO_BUTTON(radMediaType2DD),
			radMediaType2D_group);
radMediaType2D_group =
gtk_radio_button_get_group(GTK_RADIO_BUTTON(radMediaType2DD));
lblMediaType = gtk_label_new("メディアタイプ");
gtk_widget_show(lblMediaType);
gtk_frame_set_label_widget(GTK_FRAME(frmMediaType), lblMediaType);
gtk_label_set_justify(GTK_LABEL(lblMediaType), GTK_JUSTIFY_LEFT);
gtk_misc_set_padding(GTK_MISC(lblMediaType), 0, 5);

#endif				/*  */
vbox4 = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox4);
gtk_box_pack_start(GTK_BOX(hbox1), vbox4, TRUE, TRUE, 0);
btnOk = gtk_button_new_with_mnemonic("    OK    ");
gtk_widget_show(btnOk);
gtk_box_pack_start(GTK_BOX(vbox4), btnOk, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(btnOk), 4);
btnCancel = gtk_button_new_with_mnemonic("キャンセル");
gtk_widget_show(btnCancel);
gtk_box_pack_start(GTK_BOX(vbox4), btnCancel, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(btnCancel), 4);
chkUserDisk =
gtk_check_button_new_with_mnemonic
("F-BASICのユーザディスクを作成する");
dlg.chkUserDisk = chkUserDisk;
gtk_widget_show(chkUserDisk);
gtk_box_pack_start(GTK_BOX(vbox1), chkUserDisk, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(chkUserDisk), 8);

/*
 * OKボタン押下時のアクションを指定する
 */
gtk_signal_connect(GTK_OBJECT(btnOk), "clicked",
		   GTK_SIGNAL_FUNC(OnDiskImageDialogOk),
		   (gpointer) & dlg);

/*
 * CANCELボタン押下時のアクションを指定する
 */
gtk_signal_connect(GTK_OBJECT(btnCancel), "clicked",
		   GTK_SIGNAL_FUNC(OnDiskImageDialogCancel),
		   (gpointer) & dlg);

/*
 * ボタンをクリックしたらdestroy
 */
gtk_signal_connect_object(GTK_OBJECT(btnOk), "clicked",
			  GTK_SIGNAL_FUNC(gtk_widget_destroy),
			  (gpointer) dlg.dlg);
gtk_signal_connect_object(GTK_OBJECT(btnCancel), "clicked",
			GTK_SIGNAL_FUNC(gtk_widget_destroy),
			(gpointer) dlg.dlg);

/*
 * ダイアログを表示する
 */
gtk_widget_show(dlg.dlg);
gtk_main();
return dlg;
}


/*-[ディスクタイトルダイアログ]-----------------------------------------------------*/

/*
 *  OKボタンイベント
 */
static void     FASTCALL
OnDiskTitleDialogOk(GtkWidget * widget, gpointer user_data)
{
DiskTitleDialog * dlg = (DiskTitleDialog *) user_data;
dlg->bResult = DLG_OK;
strcpy(dlg->sTitle, gtk_entry_get_text(GTK_ENTRY(dlg->entTitle)));
gtk_main_quit();
}
/*
 *  CENCELボタンイベント
 */
static void     FASTCALL
OnDiskTitleDialogCancel(GtkWidget * widget, gpointer user_data)
{
DiskTitleDialog * dlg = (DiskTitleDialog *) user_data;
dlg->bResult = DLG_CANCEL;
gtk_main_quit();
}
/*
 *  ディスクタイトルダイアログを作成する
 */
DiskTitleDialog FASTCALL OpenDiskTitleDialog(void)
{

DiskTitleDialog dlg;
GtkWidget * hbox1;
GtkWidget * vbox1;
GtkWidget * lblTitle;
GtkWidget * entTitle;
GtkWidget * vbox2;
GtkWidget * btnOk;
GtkWidget * btnCancel;
dlg.bResult = DLG_NONE;
dlg.dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_container_set_border_width(GTK_CONTAINER(dlg.dlg), 4);
gtk_window_set_title(GTK_WINDOW(dlg.dlg),
		  "ディスクタイトル入力");
gtk_window_set_position(GTK_WINDOW(dlg.dlg), GTK_WIN_POS_CENTER);
gtk_window_set_modal(GTK_WINDOW(dlg.dlg), TRUE);
hbox1 = gtk_hbox_new(FALSE, 0);
gtk_widget_show(hbox1);
gtk_container_add(GTK_CONTAINER(dlg.dlg), hbox1);
vbox1 = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox1);
gtk_box_pack_start(GTK_BOX(hbox1), vbox1, FALSE, FALSE, 0);
lblTitle = gtk_label_new("タイトル16文字まで");
gtk_widget_show(lblTitle);
gtk_box_pack_start(GTK_BOX(vbox1), lblTitle, FALSE, FALSE, 0);
gtk_label_set_justify(GTK_LABEL(lblTitle), GTK_JUSTIFY_LEFT);
gtk_misc_set_alignment(GTK_MISC(lblTitle), 0, 0.5);
gtk_misc_set_padding(GTK_MISC(lblTitle), 0, 5);
entTitle = gtk_entry_new();
dlg.entTitle = entTitle;
gtk_widget_show(entTitle);
gtk_box_pack_start(GTK_BOX(vbox1), entTitle, FALSE, FALSE, 0);
gtk_entry_set_text(GTK_ENTRY(entTitle), "Default");
vbox2 = gtk_vbox_new(FALSE, 0);
gtk_widget_show(vbox2);
gtk_box_pack_start(GTK_BOX(hbox1), vbox2, FALSE, FALSE, 0);
btnOk = gtk_button_new_with_mnemonic("    OK    ");
gtk_widget_show(btnOk);
gtk_box_pack_start(GTK_BOX(vbox2), btnOk, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(btnOk), 4);
btnCancel = gtk_button_new_with_mnemonic("キャンセル");
gtk_widget_show(btnCancel);
gtk_box_pack_start(GTK_BOX(vbox2), btnCancel, FALSE, FALSE, 0);
gtk_container_set_border_width(GTK_CONTAINER(btnCancel), 4);

/*
 * OKボタン押下時のアクションを指定する
 */
gtk_signal_connect(GTK_OBJECT(btnOk), "clicked",
		   GTK_SIGNAL_FUNC(OnDiskTitleDialogOk),
		   (gpointer) & dlg);

/*
 * CANCELボタン押下時のアクションを指定する
 */
gtk_signal_connect(GTK_OBJECT(btnCancel), "clicked",
		   GTK_SIGNAL_FUNC(OnDiskTitleDialogCancel),
		   (gpointer) & dlg);

/*
 * ボタンをクリックしたらdestroy
 */
gtk_signal_connect_object(GTK_OBJECT(btnOk), "clicked",
			  GTK_SIGNAL_FUNC(gtk_widget_destroy),
			  (gpointer) dlg.dlg);
gtk_signal_connect_object(GTK_OBJECT(btnCancel), "clicked",
			GTK_SIGNAL_FUNC(gtk_widget_destroy),
			(gpointer) dlg.dlg);

/*
 * ダイアログを表示する
 */
gtk_widget_show(dlg.dlg);
gtk_main();
return dlg;
}
#endif

#ifdef __cplusplus
}
#endif

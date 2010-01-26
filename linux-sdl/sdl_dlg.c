/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN 各種ダイアログ ]
 */

#ifdef _XWIN

#include<gtk/gtk.h>
#include "xm7.h"
#include "sdl.h"
#include "sdl_cfg.h"
#include "sdl_dlg.h"

/*-[メッセージダイアログ]-----------------------------------------------------*/

void OpenErrorMessageDialog(char *strTitle, char *strMessage)
{
	GtkWidget *dlgMessage;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *imgMessage;
	GtkWidget *lblMessage;
	GtkWidget *actArea;
	GtkWidget *btnClose;

	dlgMessage = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (dlgMessage), strTitle);
	gtk_window_set_modal (GTK_WINDOW (dlgMessage), TRUE);
	gtk_window_set_resizable (GTK_WINDOW (dlgMessage), FALSE);

	vbox = GTK_DIALOG (dlgMessage)->vbox;
	gtk_widget_show (vbox);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

	imgMessage = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DND);
	gtk_widget_show (imgMessage);
	gtk_box_pack_start (GTK_BOX (hbox), imgMessage, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (imgMessage), 10, 10);

	lblMessage = gtk_label_new (strMessage);
	gtk_widget_show (lblMessage);
	gtk_box_pack_start (GTK_BOX (hbox), lblMessage, TRUE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (lblMessage), GTK_JUSTIFY_LEFT);

	actArea = GTK_DIALOG (dlgMessage)->action_area;
	gtk_widget_show (actArea);

	btnClose = gtk_button_new_with_mnemonic ("閉じる");
	gtk_widget_show (btnClose);
	gtk_dialog_add_action_widget (GTK_DIALOG (dlgMessage), btnClose, 0);
	GTK_WIDGET_SET_FLAGS (btnClose, GTK_CAN_DEFAULT);

	/* ボタンをクリックしたらdestroy */
	gtk_signal_connect_object (GTK_OBJECT(btnClose),
						"clicked", GTK_SIGNAL_FUNC (gtk_main_quit),
						(gpointer)dlgMessage);

	gtk_widget_show(dlgMessage);
}

/*-[ファイル選択ダイアログ]-----------------------------------------------------*/

/*
 * OKボタンイベント
 */
static void FASTCALL OnFileSelectDialogOk(GtkFileSelection *selector, gpointer user_data) {
	FileSelectDialog *dlg = (FileSelectDialog *)user_data;

	G_CONST_RETURN gchar *fname = 
		gtk_file_selection_get_filename (GTK_FILE_SELECTION(dlg->dlg));
   dlg->bResult = DLG_OK;
   strcpy(dlg->sFilename, fname);
   gtk_main_quit();
}

/*
 * CANCELボタンイベント
 */
static void FASTCALL OnFileSelectDialogCancel(GtkFileSelection *selector, gpointer user_data) {
	FileSelectDialog *dlg = (FileSelectDialog *)user_data;

	dlg->bResult = DLG_CANCEL;
	dlg->sFilename[0] = '\0';
	gtk_main_quit();
}

/*
 * ファイル選択ダイアログを作成する
 */
FileSelectDialog FASTCALL OpenFileSelectDialog(char *dir) {

	/* 初期化 */
	FileSelectDialog dlg;

	dlg.sFilename[0] = '\0';
	dlg.bResult = DLG_NONE;

	/* ファイル選択ダイアログを生成する */
	dlg.dlg = gtk_file_selection_new("ファイル選択");
	if ( dir != NULL ) {
		if ( strlen(dir) > 0 ) {
			gtk_file_selection_set_filename(GTK_FILE_SELECTION(dlg.dlg), (gchar *)dir);
		}
	}

	/* OKボタン押下時のアクションを指定する */	
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dlg.dlg)->ok_button),
						"clicked", GTK_SIGNAL_FUNC (OnFileSelectDialogOk),
						(gpointer) &dlg);

	/* CANCELボタン押下時のアクションを指定する */
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dlg.dlg)->cancel_button),
						"clicked", GTK_SIGNAL_FUNC (OnFileSelectDialogCancel),
						(gpointer) &dlg);

	/* ボタンをクリックしたらdestroy */
	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dlg.dlg)->ok_button),
						"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
						(gpointer) dlg.dlg);

	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(dlg.dlg)->cancel_button),
						"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
						(gpointer) dlg.dlg);

	/* ダイアログを表示する */
	gtk_widget_show (dlg.dlg);
	gtk_main();

	return dlg;
}


/*-[ディスクイメージダイアログ]-----------------------------------------------------*/


/*
 * OKボタンイベント
 */
static void FASTCALL OnDiskImageDialogOk(GtkWidget *widget, gpointer user_data) {
	DiskImageDialog *dlg = (DiskImageDialog *)user_data;

	dlg->bResult = DLG_OK;
	strcpy(dlg->sTitle, gtk_entry_get_text(GTK_ENTRY(dlg->entTitle)));
#if XM7_VER >= 3
	dlg->b2DDDisk = FALSE;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->radMediaType2DD))) {
		dlg->b2DDDisk = TRUE;
	}
#endif
	dlg->bUserDisk = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dlg->chkUserDisk));
	gtk_main_quit();
}

/*
 * CENCELボタンイベント
 */
static void FASTCALL OnDiskImageDialogCancel(GtkWidget *widget, gpointer user_data) {
	DiskImageDialog *dlg = (DiskImageDialog *)user_data;

	dlg->bResult = DLG_CANCEL;
	gtk_main_quit();
}

/*
 * ディスクイメージダイアログを作成する
 */
DiskImageDialog FASTCALL OpenDiskImageDialog(void)
{
	DiskImageDialog dlg;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *vbox2;
	GtkWidget *alignment1;
	GtkWidget *vbox3;
	GtkWidget *lblTitle;
	GtkWidget *entTitle;
	GtkWidget *frmMediaType;
	GtkWidget *hbox2;
#if XM7_VER >= 3
	GtkWidget *radMediaType2D;
	GtkWidget *radMediaType2DD;
	GSList *radMediaType2D_group = NULL;
	GtkWidget *lblMediaType;
#endif
	GtkWidget *vbox4;
	GtkWidget *chkUserDisk;
	GtkWidget *btnOk;
	GtkWidget *btnCancel;

	dlg.bResult = DLG_NONE;

	dlg.dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (dlg.dlg), 4);
	gtk_window_set_title (GTK_WINDOW (dlg.dlg), "新規ディスクイメージ作成");
	gtk_window_set_position (GTK_WINDOW (dlg.dlg), GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (dlg.dlg), TRUE);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (dlg.dlg), vbox1);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

	alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment1);
	gtk_box_pack_start (GTK_BOX (vbox2), alignment1, FALSE, FALSE, 0);

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_container_add (GTK_CONTAINER (alignment1), vbox3);

	lblTitle = gtk_label_new ("タイトル16文字まで");
	gtk_widget_show (lblTitle);
	gtk_box_pack_start (GTK_BOX (vbox3), lblTitle, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lblTitle), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblTitle), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (lblTitle), 0, 5);

	entTitle = gtk_entry_new ();
	dlg.entTitle = entTitle;
	gtk_widget_show (entTitle);
	gtk_box_pack_start (GTK_BOX (vbox3), entTitle, FALSE, FALSE, 0);
	gtk_entry_set_text (GTK_ENTRY (entTitle), "Default");

	frmMediaType = gtk_frame_new (NULL);
	gtk_widget_show (frmMediaType);
	gtk_box_pack_start (GTK_BOX (vbox2), frmMediaType, FALSE, FALSE, 0);

	hbox2 = gtk_hbox_new (TRUE, 0);
	gtk_widget_show (hbox2);
	gtk_container_add (GTK_CONTAINER (frmMediaType), hbox2);

#if XM7_VER >= 3
	radMediaType2D = gtk_radio_button_new_with_mnemonic (NULL, "2D");
	dlg.radMediaType2D = radMediaType2D;
	gtk_widget_show (radMediaType2D);
	gtk_box_pack_start (GTK_BOX (hbox2), radMediaType2D, FALSE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radMediaType2D), radMediaType2D_group);
	radMediaType2D_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radMediaType2D));

	radMediaType2DD = gtk_radio_button_new_with_mnemonic (NULL, "2DD");
	dlg.radMediaType2DD = radMediaType2DD;
	gtk_widget_show (radMediaType2DD);
	gtk_box_pack_start (GTK_BOX (hbox2), radMediaType2DD, FALSE, FALSE, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radMediaType2DD), radMediaType2D_group);
	radMediaType2D_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radMediaType2DD));

	lblMediaType = gtk_label_new ("メディアタイプ");
	gtk_widget_show (lblMediaType);
	gtk_frame_set_label_widget (GTK_FRAME (frmMediaType), lblMediaType);
	gtk_label_set_justify (GTK_LABEL (lblMediaType), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding (GTK_MISC (lblMediaType), 0, 5);
#endif

	vbox4 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox4);
	gtk_box_pack_start (GTK_BOX (hbox1), vbox4, TRUE, TRUE, 0);

	btnOk = gtk_button_new_with_mnemonic ("    OK    ");
	gtk_widget_show (btnOk);
	gtk_box_pack_start (GTK_BOX (vbox4), btnOk, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (btnOk), 4);

	btnCancel = gtk_button_new_with_mnemonic ("キャンセル");
	gtk_widget_show (btnCancel);
	gtk_box_pack_start (GTK_BOX (vbox4), btnCancel, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (btnCancel), 4);

	chkUserDisk = gtk_check_button_new_with_mnemonic ("F-BASICのユーザディスクを作成する");
	dlg.chkUserDisk = chkUserDisk;
	gtk_widget_show (chkUserDisk);
	gtk_box_pack_start (GTK_BOX (vbox1), chkUserDisk, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (chkUserDisk), 8);

	/* OKボタン押下時のアクションを指定する */
	gtk_signal_connect (GTK_OBJECT (btnOk),
   			   "clicked", GTK_SIGNAL_FUNC (OnDiskImageDialogOk),(gpointer)&dlg);
   			   
	/* CANCELボタン押下時のアクションを指定する */
	gtk_signal_connect (GTK_OBJECT (btnCancel),
						"clicked", GTK_SIGNAL_FUNC (OnDiskImageDialogCancel),
						(gpointer)&dlg);

	/* ボタンをクリックしたらdestroy */
	gtk_signal_connect_object (GTK_OBJECT (btnOk),
						"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
						(gpointer)dlg.dlg);

	gtk_signal_connect_object (GTK_OBJECT (btnCancel),
						"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
						(gpointer)dlg.dlg);

	/* ダイアログを表示する */
	gtk_widget_show (dlg.dlg);
	gtk_main();

	return dlg;
}

/*-[ディスクタイトルダイアログ]-----------------------------------------------------*/

/*
 * OKボタンイベント
 */
static void FASTCALL OnDiskTitleDialogOk(GtkWidget *widget, gpointer user_data) {
	DiskTitleDialog *dlg = (DiskTitleDialog *)user_data;

	dlg->bResult = DLG_OK;
	strcpy(dlg->sTitle, gtk_entry_get_text(GTK_ENTRY(dlg->entTitle)));
	gtk_main_quit();
}

/*
 * CENCELボタンイベント
 */
static void FASTCALL OnDiskTitleDialogCancel(GtkWidget *widget, gpointer user_data) {
	DiskTitleDialog *dlg = (DiskTitleDialog *)user_data;

	dlg->bResult = DLG_CANCEL;
	gtk_main_quit();
}

/*
 * ディスクタイトルダイアログを作成する
 */
DiskTitleDialog FASTCALL OpenDiskTitleDialog(void)
{
	DiskTitleDialog dlg;
	GtkWidget *hbox1;
	GtkWidget *vbox1;
	GtkWidget *lblTitle;
	GtkWidget *entTitle;
	GtkWidget *vbox2;
	GtkWidget *btnOk;
	GtkWidget *btnCancel;

	dlg.bResult = DLG_NONE;

	dlg.dlg = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (dlg.dlg), 4);
	gtk_window_set_title (GTK_WINDOW (dlg.dlg), "ディスクタイトル入力");
	gtk_window_set_position (GTK_WINDOW (dlg.dlg), GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (dlg.dlg), TRUE);
													  
	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (dlg.dlg), hbox1);
													  
	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_box_pack_start (GTK_BOX (hbox1), vbox1, FALSE, FALSE, 0);
													  
	lblTitle = gtk_label_new ("タイトル16文字まで");
	gtk_widget_show (lblTitle);
	gtk_box_pack_start (GTK_BOX (vbox1), lblTitle, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lblTitle), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lblTitle), 0, 0.5);
	gtk_misc_set_padding (GTK_MISC (lblTitle), 0, 5);
													  
	entTitle = gtk_entry_new ();
	dlg.entTitle = entTitle;
	gtk_widget_show (entTitle);
	gtk_box_pack_start (GTK_BOX (vbox1), entTitle, FALSE, FALSE, 0);
	gtk_entry_set_text (GTK_ENTRY (entTitle), "Default");
													  
	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), vbox2, FALSE, FALSE, 0);
													  
	btnOk = gtk_button_new_with_mnemonic ("    OK    ");
	gtk_widget_show (btnOk);
	gtk_box_pack_start (GTK_BOX (vbox2), btnOk, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (btnOk), 4);
													  
	btnCancel = gtk_button_new_with_mnemonic ("キャンセル");
	gtk_widget_show (btnCancel);
	gtk_box_pack_start (GTK_BOX (vbox2), btnCancel, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (btnCancel), 4);

	/* OKボタン押下時のアクションを指定する */
	gtk_signal_connect (GTK_OBJECT (btnOk),
						"clicked", GTK_SIGNAL_FUNC (OnDiskTitleDialogOk),
						(gpointer)&dlg);

	/* CANCELボタン押下時のアクションを指定する */
	gtk_signal_connect (GTK_OBJECT (btnCancel),
						"clicked", GTK_SIGNAL_FUNC (OnDiskTitleDialogCancel),
						(gpointer)&dlg);

	/* ボタンをクリックしたらdestroy */
	gtk_signal_connect_object (GTK_OBJECT (btnOk),
						"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
						(gpointer)dlg.dlg);

	gtk_signal_connect_object (GTK_OBJECT (btnCancel),
						"clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
						(gpointer)dlg.dlg);

	/* ダイアログを表示する */
	gtk_widget_show (dlg.dlg);
	gtk_main();

	return dlg;
}

#endif	/* _XWIN */

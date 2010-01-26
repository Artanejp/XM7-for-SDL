/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN メニューコマンド ]
 */

#ifdef _XWIN

#include <gtk/gtk.h>
#include "xm7.h"
#include "fdc.h"
#include "tapelp.h"
#include "tools.h"
#include "mouse.h"
#include "rtc.h"
#include "sdl.h"
#include "sdl_cmd.h"
#include "sdl_cfg.h"
#include "sdl_gtkdlg.h"
#include "sdl_prop.h"
#include "sdl_snd.h"
#include "sdl_sch.h"
#include "sdl_kbd.h"

/*
 *	グローバル ワーク
 */
char InitialDir[5][MAXPATHLEN];

/*
 *	スタティック ワーク
 */
static char StatePath[MAXPATHLEN];
static char DiskTitle[16 + 1];
static BOOL DiskMedia;
static BOOL DiskFormat;

static GtkWidget *drive_menu[2];
static GtkWidget *drive_item[2];
static GtkWidget *midrive_open[2];
static GtkWidget *midrive_openboth[2];
static GtkWidget *midrive_eject[2];
static GtkWidget *midrive_sep[2][2];
static GtkWidget *midrive_teject[2];
static GtkWidget *midrive_writep[2];
static GtkWidget *midrive_medias[2][FDC_MEDIAS];

static Disk disk[2][FDC_MEDIAS];

static GtkWidget *tape_menu;
static GtkWidget *tape_item;
static GtkWidget *mitape_open;
static GtkWidget *mitape_eject;
static GtkWidget *mitape_sep[2];
static GtkWidget *mitape_rew;
static GtkWidget *mitape_ff;
static GtkWidget *mitape_rec;

static GtkWidget *debug_menu;
static GtkWidget *debug_item;
static GtkWidget *miExec;
static GtkWidget *miBreak;

static GtkWidget *tool_menu;
static GtkWidget *tool_item;
static GtkWidget *miWavCapture;
static guint hidWavCapture;
#ifdef MOUSE
static GtkWidget *miMouseCapture;
static guint hidMouseCapture;
#endif

static GtkWidget *help_menu;
static GtkWidget *help_item;


/*-[ ファイルメニュー ]-----------------------------------------------------*/

/*
 *	ステートロード処理
 */
static void FASTCALL StateLoad(char *path)
{
	int state;

	state = system_load(path);
	if (state == STATELOAD_ERROR) {
		/* 本体読み込み中のエラー発生時のみリセット */
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
 *	開く(O)
 */
static void FASTCALL OnOpen(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[2]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

		/* ステートロード */
	LockVM();
	StopSnd();
	StateLoad(dlg.sFilename);
	PlaySnd();
	ResetSch();
	UnlockVM();
	/* 画面再描画 */
	//OnRefresh();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[2], dlg.sFilename);
	}
}

/*
 *	保存(A)
 */
static void FASTCALL OnSaveAs(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[2]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	/* ステートセーブ */
	LockVM();
	StopSnd();
        SDL_Delay(100); /* テスト */
	if (!system_save(dlg.sFilename)) {
	}
	else {
		strcpy(StatePath, dlg.sFilename);
	}
	PlaySnd();
	ResetSch();
	UnlockVM();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[2], dlg.sFilename);
	}
}

/*
 *	名前を付けて保存(S)
 */
static void FASTCALL OnSave(GtkWidget *widget, gpointer data)
{
	/* まだ保存されていなければ、名前をつける */
	if (StatePath[0] == '\0') {
		OnSaveAs(widget, data);
		return;
	}

	/* ステートセーブ */
	LockVM();
	StopSnd();
	if (!system_save(StatePath)) {
	}
	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	リセット(R)
 */
static void FASTCALL OnReset(GtkWidget *widget, gpointer data)
{
	LockVM();
	system_reset();
	UnlockVM();

	/* 再描画 */
	//OnRefresh();
}

/*
 *	ホットリセット(H)
 */
static void FASTCALL OnHotReset(GtkWidget *widget, gpointer data)
{
	LockVM();
	system_hotreset();
	UnlockVM();

	/* 再描画 */
	//OnRefresh();
}

/*
 *	BASICモード(B)
 */
static void FASTCALL OnBasic(GtkWidget *widget, gpointer data)
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
 *	DOSモード(D)
 */
static void FASTCALL OnDos(GtkWidget *widget, gpointer data)
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
 *	終了(X)
 */
static void FASTCALL OnExit(GtkWidget *widget, gpointer data)
{
	/* フラグアップ */
	LockVM();
	bCloseReq = TRUE;
	UnlockVM();

	gtk_widget_destroy(GTK_WIDGET(data));
}


/*
 * 「ファイル」メニュー生成
 */
static void FASTCALL CreateFileMenu 
	(GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
	GtkWidget *file_menu;
	GtkWidget *file_item;
	GtkWidget *sub_item;
	GSList *ModeGroup = NULL;

	/* ファイルメニューの作成 */
	file_menu = gtk_menu_new();

	/*********************************************************/

	/*「開く」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("開く");
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnOpen), wndMain);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* 「上書き保存」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("上書き保存");
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnSave), wndMain);
	gtk_widget_show (sub_item);
	
	/*********************************************************/

	/* 「名前を付けて保存」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("名前を付けて別名保存");
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnSaveAs), wndMain);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* セパレータを作成 */
	sub_item = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* 「リセット」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("リセット");
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnReset), wndMain);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* 「ホットリセット」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("ホットリセット");
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnHotReset), wndMain);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* セパレータを作成 */
	sub_item = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* 「BASICモード」ボタンを作成 */
	sub_item = gtk_radio_menu_item_new_with_label (ModeGroup, "BASICモード");
	ModeGroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (sub_item));
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnBasic), wndMain);
	gtk_widget_show (sub_item);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (sub_item), TRUE);

	/*********************************************************/

	/* 「DOSモード」ボタンを作成 */
	sub_item = gtk_radio_menu_item_new_with_label (ModeGroup, "DOSモード");
	ModeGroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (sub_item));
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnDos), wndMain);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* セパレータを作成 */
	sub_item = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* 「終了」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("終了");
	gtk_menu_append (GTK_MENU(file_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnExit), wndMain);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*ファイルメニューをのせるメニューアイテムの作成 */
	file_item = gtk_menu_item_new_with_label ("ファイル");
	gtk_widget_show (file_item);

	/* ファイルメニューアイテムにファイルメニューをのせる */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(file_item), file_menu);

	/* メニュバーにファイルメニューアイテムをのせる */
	gtk_menu_bar_append (GTK_MENU_BAR(menu_bar), file_item);

	/*********************************************************/
	return;
}


/*-[ ディスクメニュー ]-----------------------------------------------------*/

/*
 *      ディスク(1)(0)メニュー更新
 */
static void FASTCALL OnDiskPopup(GtkWidget *widget, gpointer data)
{
	int Drive = ((Disk *)data)->drive;
	int i;

	gtk_widget_hide(midrive_open[Drive]);
	gtk_widget_hide(midrive_openboth[Drive]);
	gtk_widget_hide(midrive_eject[Drive]);
	gtk_widget_hide(midrive_sep[Drive][0]);
	gtk_widget_hide(midrive_sep[Drive][1]);
	gtk_widget_hide(midrive_sep[Drive][2]);
	gtk_widget_hide(midrive_teject[Drive]);
	gtk_widget_hide(midrive_writep[Drive]);

	for (i=0; i<FDC_MEDIAS; i++) {
		gtk_widget_hide(midrive_medias[Drive][i]);
	}

	gtk_widget_show(midrive_open[Drive]);
	gtk_widget_show(midrive_openboth[Drive]);

	/* ディスクが挿入されていなければ、ここまで */
	if (fdc_ready[Drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	/* イジェクト */
	gtk_widget_show(midrive_eject[Drive]);
                                                                                                                                           
	/* セパレータ挿入 */
	gtk_widget_show(midrive_sep[Drive][0]);
                                                                                                                                           
	/* 一時取り出し */
	if (fdc_teject[Drive]) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(midrive_teject[Drive]), TRUE);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(midrive_teject[Drive]), FALSE);
	}
	gtk_widget_show(midrive_teject[Drive]);
                                                                                                                                           
	/* ライトプロテクト */
	if (fdc_fwritep[Drive]) {
		gtk_widget_set_sensitive(midrive_writep[Drive], FALSE);
	} else {
		gtk_widget_set_sensitive(midrive_writep[Drive], TRUE);
		if (fdc_writep[Drive]) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(midrive_writep[Drive]), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(midrive_writep[Drive]), FALSE);
		}
	}
	gtk_widget_show(midrive_writep[Drive]);
                                                                                                                                           
	/* セパレータ挿入 */
	gtk_widget_show(midrive_sep[Drive][1]);
                                                                                                                                           
	/* メディアを回す */
	for (i=0; i<fdc_medias[Drive]; i++) {
		if (fdc_media[Drive] == i) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(midrive_medias[Drive][i]), TRUE);
		}
                                                                                                                                           
		if (GTK_BIN (midrive_medias[Drive][i])->child) {
			GtkWidget *child = GTK_BIN (midrive_medias[Drive][i])->child;
			if (GTK_IS_LABEL (child)) {
				gchar *utf8 = g_convert (
								fdc_name[Drive][i],
								strlen (fdc_name[Drive][i]),
								"UTF-8",
								"CP932",
								NULL,
								NULL,
								NULL
							);

				gtk_label_set_text(GTK_LABEL (child), utf8);
				g_free(utf8);
			}
		}

		gtk_widget_show(midrive_medias[Drive][i]);

	}
                                                                                                                             
}

/*
 *	ドライブを開く
 */
static void FASTCALL OnDiskOpen(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[0]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	int Drive = ((Disk *)data)->drive;

	/* セット */
	LockVM();
	fdc_setdisk(Drive, dlg.sFilename);
	ResetSch();
	UnlockVM();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[0], dlg.sFilename);
	}
}

/*
 *	両ドライブを開く
 */
static void FASTCALL OnDiskBoth(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[0]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	/* セット */
	LockVM();
	fdc_setdisk(0, dlg.sFilename);
	fdc_setdisk(1, NULL);
	if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
		fdc_setdisk(1, dlg.sFilename);
		fdc_setmedia(1, 1);
	}
	ResetSch();
	UnlockVM();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[0], dlg.sFilename);
	}
}

/*
 *	ディスクイジェクト
 */
static void FASTCALL OnDiskEject(GtkWidget *widget, gpointer data)
{
	int Drive = ((Disk *)data)->drive;

	/* イジェクト */
	LockVM();
	fdc_setdisk(Drive, NULL);
	UnlockVM();
}

/*
 *	ディスク一時取り出し
 */
static void FASTCALL OnDiskTemp(GtkWidget *widget, gpointer data)
{
	int Drive = ((Disk *)data)->drive;

	/* 書き込み禁止切り替え */
	LockVM();
	if (fdc_teject[Drive]) {
		fdc_teject[Drive] = FALSE;
	}
	else {
		fdc_teject[Drive] = TRUE;
	}
	UnlockVM();
}

/*
 *	ディスク書き込み禁止
 */
static void FASTCALL OnDiskProtect(GtkWidget *widget, gpointer data)
{
	int Drive = ((Disk *)data)->drive;

	/* 書き込み禁止切り替え */
	LockVM();
	if (fdc_writep[Drive]) {
		fdc_setwritep(Drive, FALSE);
	}
	else {
		fdc_setwritep(Drive, TRUE);
	}
	ResetSch();
	UnlockVM();
}

/*
 *	メディア切り替え
 */
static void FASTCALL OnMediaChange(GtkWidget *widget, gpointer data)
{
	int Drive = ((Disk *)data)->drive;
	int Media = ((Disk *)data)->media;

	/* 書き込み禁止切り替え */
	LockVM();
	fdc_setmedia(Drive, Media);
	ResetSch();
	UnlockVM();
}

/*
 * 「ディスク」メニューを作成
 */
static void FASTCALL CreateDiskMenu 
	(GtkWidget *menu_bar, GtkAccelGroup *accel_group, int d)
{
	int i;
	gchar label[6];
	GSList *MediaGroup = NULL;

	/* Disk構造体の初期化 */
	for (i=0; i < FDC_MEDIAS; i++) {
		disk[d][i].drive = d;
		disk[d][i].media = i;
	}

	/* ディスクメニューの作成 */
	drive_menu[d] = gtk_menu_new();

	/*********************************************************/

	/* 「開く」ボタンを作成 */
	midrive_open[d] = gtk_menu_item_new_with_label ("開く");
	gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_open[d]);
	gtk_signal_connect (GTK_OBJECT(midrive_open[d]), "activate",
		GTK_SIGNAL_FUNC (OnDiskOpen), (gpointer)&disk[d][0]);
	//gtk_widget_show (midrive_open[d]);

	/*********************************************************/

	/* 「両ドライブで開く」ボタンを作成 */
	midrive_openboth[d] = gtk_menu_item_new_with_label ("両ドライブで開く");
	gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_openboth[d]);
	gtk_signal_connect (GTK_OBJECT(midrive_openboth[d]), "activate",
		GTK_SIGNAL_FUNC (OnDiskBoth), (gpointer)NULL);
	//gtk_widget_show (midrive_openboth[d]);

	/*********************************************************/

	/* 「取り外す」ボタンを作成 */
	midrive_eject[d] = gtk_menu_item_new_with_label ("取り外す");
	gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_eject[d]);
	gtk_signal_connect (GTK_OBJECT(midrive_eject[d]), "activate",
		GTK_SIGNAL_FUNC (OnDiskEject), (gpointer)&disk[d][0]);
	//gtk_widget_show (midrive_eject[d]);

	/*********************************************************/

	/* セパレータを作成 */
	midrive_sep[d][0] = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_sep[d][0]);
	//gtk_widget_show (midrive_sep[d][0]);

	/*********************************************************/

	/* 「一時取り出し」ボタンを作成 */
	midrive_teject[d] = gtk_check_menu_item_new_with_label ("一時取り出し");
	gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_teject[d]);
	gtk_signal_connect (GTK_OBJECT(midrive_teject[d]), "activate",
		GTK_SIGNAL_FUNC (OnDiskTemp), (gpointer)&disk[d][0]);
	gtk_widget_show (midrive_teject[d]);


	/*********************************************************/

	/*「書き込み禁止」ボタンを作成 */
	midrive_writep[d] = gtk_check_menu_item_new_with_label ("書き込み禁止");
	gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_writep[d]);
	gtk_signal_connect (GTK_OBJECT(midrive_writep[d]), "activate",
		GTK_SIGNAL_FUNC (OnDiskProtect), (gpointer)&disk[d][0]);
	//gtk_widget_show (midrive_writep[d]);

	/*********************************************************/

	/* セパレータを作成 */
	midrive_sep[d][1] = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_sep[d][1]);
	gtk_widget_show (midrive_sep[d][1]);

	/*********************************************************/

	/* メディア選択用のラジオボタンを作成 */
	for (i=0; i < FDC_MEDIAS; i++) {
		midrive_medias[d][i] = gtk_radio_menu_item_new_with_label (MediaGroup, "-");
		MediaGroup = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (midrive_medias[d][i]));
		gtk_menu_append (GTK_MENU(drive_menu[d]), midrive_medias[d][i]);
		gtk_signal_connect (GTK_OBJECT(midrive_medias[d][i]), "activate",
			GTK_SIGNAL_FUNC (OnMediaChange), (gpointer)&disk[d][i]);
		//gtk_widget_show (midrive_medias[d][i]);
	}

	/*********************************************************/

	/*ディスクメニューをのせるメニューアイテムの作成 */
	g_sprintf(label, "ドライブ%d", d);
	drive_item[d] = gtk_menu_item_new_with_label (label);
	gtk_widget_show (drive_item[d]);

	/* ディスクメニューアイテムにディスクメニューをのせる */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(drive_item[d]), drive_menu[d]);

	/* メニュバーにディスクメニューアイテムをのせる */
	gtk_menu_bar_append (GTK_MENU_BAR(menu_bar), drive_item[d]);

	gtk_signal_connect (GTK_OBJECT(drive_item[d]), "activate",
		GTK_SIGNAL_FUNC (OnDiskPopup), (gpointer)&disk[d][0]);

	/*********************************************************/

	return;
}

/*-[ テープメニュー ]-------------------------------------------------------*/

/*
 *      テープ(A)メニュー更新
 */
static void FASTCALL OnTapePopup(GtkWidget *widget, gpointer data)
{
	/* メニューすべて削除 */
	gtk_widget_hide(mitape_open);
	gtk_widget_hide(mitape_eject);
	gtk_widget_hide(mitape_sep[0]);
	gtk_widget_hide(mitape_sep[1]);
	gtk_widget_hide(mitape_rew);
	gtk_widget_hide(mitape_ff);
	gtk_widget_hide(mitape_rec);

	/* オープン */
	gtk_widget_show(mitape_open);

	/* テープがセットされていなければ、ここまで */
	if (tape_fileh == -1) {
		return;
	}

	/* イジェクト */
	gtk_widget_show(mitape_eject);

	/* セパレータ挿入 */
	gtk_widget_show(mitape_sep[0]);

	/* 巻き戻し */
	gtk_widget_show(mitape_rew);

	/* 早送り */
	gtk_widget_show(mitape_ff);

	/* セパレータ挿入 */
	gtk_widget_show(mitape_sep[1]);

	/* 録音 */
	if (tape_writep) {
		gtk_widget_set_sensitive(mitape_rec, FALSE);
	} else {
		gtk_widget_set_sensitive(mitape_rec, TRUE);
		if (tape_rec) {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitape_rec), TRUE);
		} else {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitape_rec), FALSE);
		}
	}

	gtk_widget_show(mitape_rec);
}


/*
 *  テープオープン
 */
static void FASTCALL OnTapeOpen(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[1]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	/* セット */
	LockVM();
	tape_setfile(dlg.sFilename);
	ResetSch();
	UnlockVM();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[1], dlg.sFilename);
	}
}

/*
 *	テープイジェクト
 */
static void FASTCALL OnTapeEject(GtkWidget *widget, gpointer data)
{
	/* イジェクト */
	LockVM();
	tape_setfile(NULL);
	UnlockVM();
}

/*
 *	巻き戻し
 */
static void FASTCALL OnRew(GtkWidget *widget, gpointer data)
{
	/* 巻き戻し */
	LockVM();
	StopSnd();

	tape_rew();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	早送り
 */
static void FASTCALL OnFF(GtkWidget *widget, gpointer data)
{
	/* 巻き戻し */
	LockVM();
	StopSnd();

	tape_ff();

	PlaySnd();
	ResetSch();
	UnlockVM();
}

/*
 *	録音
 */
static void FASTCALL OnRec(GtkWidget *widget, gpointer data)
{
	/* 録音 */
	LockVM();
	if (tape_rec) {
		tape_setrec(FALSE);
	}
	else {
		tape_setrec(TRUE);
	}
	UnlockVM();
}

/*
 * 「テープ」メニューを作成
 */
static void FASTCALL CreateTapeMenu 
	(GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
	/* テープメニューの作成 */
	tape_menu = gtk_menu_new();

	/*********************************************************/

	/* 「開く」ボタンを作成 */
	mitape_open = gtk_menu_item_new_with_label ("開く");
	gtk_menu_append (GTK_MENU(tape_menu), mitape_open);
	gtk_signal_connect (GTK_OBJECT(mitape_open), "activate",
		GTK_SIGNAL_FUNC (OnTapeOpen), NULL);
	//gtk_widget_show (mitape_open);

	/*********************************************************/

	/* 「取り外す」ボタンを作成 */
	mitape_eject = gtk_menu_item_new_with_label ("取り外す");
	gtk_menu_append (GTK_MENU(tape_menu), mitape_eject);
	gtk_signal_connect (GTK_OBJECT(mitape_eject), "activate",
		GTK_SIGNAL_FUNC (OnTapeEject), NULL);
	//gtk_widget_show (mitape_eject);

	/*********************************************************/

	/* セパレータを作成 */
	mitape_sep[0] = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(tape_menu), mitape_sep[0]);
	//gtk_widget_show (mitape_sep[0]);

	/*********************************************************/

	/* 「巻き戻し」ボタンを作成 */
	mitape_rew = gtk_menu_item_new_with_label ("巻き戻し");
	gtk_menu_append (GTK_MENU(tape_menu), mitape_rew);
	gtk_signal_connect (GTK_OBJECT(mitape_rew), "activate",
		GTK_SIGNAL_FUNC (OnRew), NULL);
	gtk_widget_show (mitape_rew);


	/*********************************************************/

	/* 「早送り」ボタンを作成 */
	mitape_ff = gtk_menu_item_new_with_label ("早送り");
	gtk_menu_append (GTK_MENU(tape_menu), mitape_ff);
	gtk_signal_connect (GTK_OBJECT(mitape_ff), "activate",
		GTK_SIGNAL_FUNC (OnFF), NULL);
	//gtk_widget_show (mitape_ff);

	/*********************************************************/

	/* セパレータを作成 */
	mitape_sep[1] = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(tape_menu), mitape_sep[1]);
	//gtk_widget_show (mitape_sep[1]);

	/*********************************************************/


	/*「録音」ボタンを作成 */
	mitape_rec = gtk_check_menu_item_new_with_label ("録音");
	gtk_menu_append (GTK_MENU(tape_menu), mitape_rec);
	gtk_signal_connect (GTK_OBJECT(mitape_rec), "activate",
		GTK_SIGNAL_FUNC (OnRec), NULL);
	//gtk_widget_show (mitape_rec);

	/*********************************************************/

	/*テープメニューをのせるメニューアイテムの作成 */
	tape_item = gtk_menu_item_new_with_label ("テープ");
	gtk_widget_show (tape_item);

	/* テープメニューアイテムにドライブメニューをのせる */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(tape_item), tape_menu);

	/* メニュバーにテープメニューアイテムをのせる */
	gtk_menu_bar_append (GTK_MENU_BAR(menu_bar), tape_item);

	gtk_signal_connect (GTK_OBJECT(tape_item), "activate",
		GTK_SIGNAL_FUNC (OnTapePopup), NULL);

	/*********************************************************/

	return;
}

/*-[ ヘルプメニュー ]-----------------------------------------------------*/



/*
 *	バージョン情報
 */
static void FASTCALL OnVersion(GtkWidget *widget, gpointer data)
{
	GtkWidget *dlgVersion;
	GtkWidget *dialog_vbox;
	GtkWidget *hbox;
	GtkWidget *image;
	GtkWidget *textviewProduct;
	GtkWidget *textviewAuthor;
	GtkWidget *dialog_action_area;
	GtkWidget *okbutton;
	char icon_path[MAXPATHLEN];

	dlgVersion = gtk_dialog_new ();
	gtk_widget_set_usize(dlgVersion, 300, 250);
	gtk_window_set_resizable (GTK_WINDOW (dlgVersion), FALSE);
	gtk_window_set_title (GTK_WINDOW (dlgVersion), "XM7バージョン情報");
	gtk_window_set_modal (GTK_WINDOW (dlgVersion), TRUE);

	dialog_vbox = GTK_DIALOG (dlgVersion)->vbox;
	gtk_widget_show (dialog_vbox);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);

	strcpy(icon_path, ModuleDir);
	switch(fm7_ver) {
		case 1:
			strcat(icon_path,"resource/tamori.ico");
			break;
		case 2:
			strcat(icon_path,"resource/app_av.ico");
			break;
		case 3:
			strcat(icon_path,"resource/app_ex.ico");
			break;
		default:
			icon_path[0] = '\0';
	}

	if ( icon_path[0] != '\0' && strcmp(icon_path, ModuleDir)!=0 ) {
		image = gtk_image_new_from_file (icon_path);
	} else {
		image = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_DIALOG);
	}

	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);

	textviewProduct = gtk_text_view_new ();
	gtk_widget_show (textviewProduct);
	gtk_box_pack_start (GTK_BOX (hbox), textviewProduct, TRUE, TRUE, 0);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (textviewProduct), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textviewProduct), FALSE);
 	gtk_text_view_set_justification (GTK_TEXT_VIEW (textviewProduct), GTK_JUSTIFY_CENTER);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textviewProduct), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textviewProduct), FALSE);
	gtk_text_buffer_set_text (
		gtk_text_view_get_buffer(GTK_TEXT_VIEW (textviewProduct)),
		VERSTR,
		-1
	);

	textviewAuthor = gtk_text_view_new ();
	gtk_widget_show (textviewAuthor);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), textviewAuthor, TRUE, TRUE, 0);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (textviewAuthor), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textviewAuthor), FALSE);
	gtk_text_view_set_justification (GTK_TEXT_VIEW (textviewAuthor), GTK_JUSTIFY_CENTER);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textviewAuthor), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textviewAuthor), FALSE);
 	gtk_text_buffer_set_text (
		gtk_text_view_get_buffer (GTK_TEXT_VIEW (textviewAuthor)),
		AUTSTR,
		-1
	);

	dialog_action_area = GTK_DIALOG (dlgVersion)->action_area;
	gtk_widget_show (dialog_action_area);

	okbutton = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton);
	gtk_dialog_add_action_widget (GTK_DIALOG (dlgVersion), okbutton, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton, GTK_CAN_DEFAULT);

	gtk_signal_connect_object (GTK_OBJECT (okbutton), "clicked",
						GTK_SIGNAL_FUNC (gtk_widget_destroy), dlgVersion);
	gtk_widget_show (dlgVersion);
}

/*
 * 「ヘルプ」メニューを作成
 */
static void FASTCALL CreateHelpMenu 
	(GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
	GtkWidget *sub_item;

	/* ヘルプメニューの作成 */
	help_menu = gtk_menu_new();

	/*********************************************************/

	/* 「バージョン情報」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("バージョン情報");
	gtk_menu_append (GTK_MENU(help_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnVersion), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*ヘルプメニューをのせるメニューアイテムの作成 */
	help_item = gtk_menu_item_new_with_label ("ヘルプ");
	gtk_widget_show (help_item);

	/* テープメニューアイテムにドライブメニューをのせる */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(help_item), help_menu);

	/* メニュバーにヘルプメニューアイテムをのせる */
	gtk_menu_bar_append (GTK_MENU_BAR(menu_bar), help_item);

	/*********************************************************/

	return;
}

/*-[ デバッグメニュー ]-----------------------------------------------------*/

/*
 *	デバッグメニュー更新
 */
static void FASTCALL OnDebugPopup(GtkWidget *widget, gpointer data)
{
	if (run_flag) {
		gtk_widget_set_sensitive(miExec, FALSE);
		gtk_widget_set_sensitive(miBreak, TRUE);
	} else {
		gtk_widget_set_sensitive(miExec, TRUE);
		gtk_widget_set_sensitive(miBreak, FALSE);
	}
}

/*
 *	実行(X)
 */
static void FASTCALL OnExec(void)
{
	/* 既に実行中なら、何もしない */
	if (run_flag) {
		return;
	}

	/* スタート */
	LockVM();
	stopreq_flag = FALSE;
	run_flag = TRUE;
	UnlockVM();
}

/*
 *	停止(B)
 */
static void FASTCALL OnBreak(void)
{
	/* 既に停止状態なら、何もしない */
	if (!run_flag) {
		return;
	}

	/* 停止 */
	LockVM();
	stopreq_flag = TRUE;
	UnlockVM();
}

/*
 * 「デバッグ」メニューを作成
 */
static void FASTCALL CreateDebugMenu 
	(GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
	/* デバッグメニューの作成 */
	debug_menu = gtk_menu_new();

	/*********************************************************/

	/* 「実行」ボタンを作成 */
	miExec = gtk_menu_item_new_with_label ("実行");
	gtk_menu_append (GTK_MENU(debug_menu), miExec);
	gtk_signal_connect (GTK_OBJECT(miExec), "activate",
		GTK_SIGNAL_FUNC (OnExec), NULL);
	gtk_widget_set_sensitive(miExec, FALSE);
	gtk_widget_show (miExec);
	
	/*********************************************************/

	/* 「停止」ボタンを作成 */
	miBreak = gtk_menu_item_new_with_label ("停止");
	gtk_menu_append (GTK_MENU(debug_menu), miBreak);
	gtk_signal_connect (GTK_OBJECT(miBreak), "activate",
		GTK_SIGNAL_FUNC (OnBreak), NULL);
	gtk_widget_set_sensitive(miBreak, TRUE);
	gtk_widget_show (miBreak);

	/*********************************************************/

	/*デバッグメニューをのせるメニューアイテムの作成 */
	debug_item = gtk_menu_item_new_with_label ("デバッグ");
	gtk_widget_show (debug_item);

	/* テープメニューアイテムにドライブメニューをのせる */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(debug_item), debug_menu);

	/* メニュバーにヘルプメニューアイテムをのせる */
	gtk_menu_bar_append (GTK_MENU_BAR(menu_bar), debug_item);

	gtk_signal_connect (GTK_OBJECT(debug_item), "activate",
		GTK_SIGNAL_FUNC (OnDebugPopup), NULL);
	/*********************************************************/

	return;
}

/*-[ ツールメニュー ]-------------------------------------------------------*/

/*
 *      ツールメニュー更新
 */
static void FASTCALL OnToolPopup(GtkWidget *widget, gpointer data)
{
#ifdef MOUSE
	gtk_signal_handler_block(GTK_OBJECT(miMouseCapture), hidMouseCapture);
	if (mos_capture) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miMouseCapture), TRUE);
	} else {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miMouseCapture), FALSE);
	}
	gtk_signal_handler_unblock(GTK_OBJECT(miMouseCapture), hidMouseCapture);
#endif
}

/*
 *	時刻アジャスト
 */
static void FASTCALL OnTimeAdjust(GtkWidget *widget, gpointer data)
{
	/* 時刻を再設定する */
	rtc_time_adjust();

	/* 念のためスケジュールを初期化 */
	rtc_reset();
}

/*
 *	マウスモード切り換え(M)
 */
#ifdef MOUSE
static void FASTCALL OnMouseMode(GtkWidget *widget, gpointer data)
{
	/* マウスキャプチャフラグを反転させてモード切り替え */
	mos_capture = (!mos_capture);
	SetMouseCapture(bActivate);
}
#endif

/*
 *	画面キャプチャ(C)
 */
static void FASTCALL OnGrpCapture(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[3]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	/* キャプチャ */
	LockVM();
	StopSnd();

	capture_to_bmp(dlg.sFilename, FALSE);

	PlaySnd();
	ResetSch();
	UnlockVM();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[3], dlg.sFilename);
	}
}

/*
 *	画面キャプチャ2
 */
static void FASTCALL OnGrpCapture2(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[3]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	/* キャプチャ */
	LockVM();
	StopSnd();

	capture_to_bmp2(dlg.sFilename);

	PlaySnd();
	ResetSch();
	UnlockVM();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[3], dlg.sFilename);
	}
}

/*
 *	WAVキャプチャ(W)
 */
static void FASTCALL OnWavCapture(GtkWidget *widget, gpointer data)
{
	char *p;
	FileSelectDialog dlg;

	/* 既にキャプチャ中なら、クローズ */
	if (hWavCapture >= 0) {
		LockVM();
		CloseCaptureSnd();
		UnlockVM();
		return;
	}

	/* ファイル選択 */
	dlg = OpenFileSelectDialog(InitialDir[4]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	/* キャプチャ */
	LockVM();
	OpenCaptureSnd(dlg.sFilename);
	UnlockVM();

	/* 条件判定 */
	if (hWavCapture < 0) {
		LockVM();
		StopSnd();

		PlaySnd();
		ResetSch();
		UnlockVM();
	}

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[4], dlg.sFilename);
	}
}

/*
 *	新規ディスク作成(D)
 */
static void FASTCALL OnNewDisk(GtkWidget *widget, gpointer data)
{
	char *p;
	int ret;
	BOOL err;
	DiskImageDialog ddlg;
	FileSelectDialog fdlg;

	/* タイトル入力 */
	ddlg = OpenDiskImageDialog();
	if (ddlg.bResult != DLG_OK) {
		return;
	}

	strcpy(DiskTitle, ddlg.sTitle);
	DiskFormat = ddlg.bUserDisk;
#if XM7_VER >= 3
	DiskMedia = ddlg.b2DDDisk;
#else
	DiskMedia = FALSE;
#endif

	/* ファイル選択 */
	fdlg = OpenFileSelectDialog(InitialDir[0]);
	if (fdlg.bResult != DLG_OK ) {
		return;
	}

	/* 作成 */
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
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[0], fdlg.sFilename);
	}
}

/*
 *	新規テープ作成(T)
 */
static void FASTCALL OnNewTape(GtkWidget *widget, gpointer data)
{
	char *p;

	/* ファイル選択 */
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[1]);
	if (dlg.bResult != DLG_OK ) {
		return;
	}

	/* 作成 */
	LockVM();
	StopSnd();

	if (make_new_t77(dlg.sFilename)) {
	}

	PlaySnd();
	ResetSch();
	UnlockVM();

	p = strrchr(dlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[1], dlg.sFilename);
	}
}

/*
 *	VFD→D77変換(V)
 */
static void FASTCALL OnVFD2D77(GtkWidget *widget, gpointer data)
{
	char *p;
	FileSelectDialog sdlg,ddlg;
	DiskTitleDialog tdlg;
	char src[MAXPATHLEN];
	char dst[MAXPATHLEN];
	int ret;

	/* ファイル選択 */
	sdlg = OpenFileSelectDialog(InitialDir[0]);
	if (sdlg.bResult != DLG_OK ) {
		return;
	}
	strcpy(src, sdlg.sFilename);

	/* タイトル入力 */
	tdlg = OpenDiskTitleDialog();
	if (tdlg.bResult != DLG_OK) {
		return;
	}

	strcpy(DiskTitle, tdlg.sTitle);

	/* ファイル選択 */
	ddlg = OpenFileSelectDialog(InitialDir[0]);
	if (ddlg.bResult != DLG_OK ) {
		return;
	}
	strcpy(dst, ddlg.sFilename);

	/* 作成 */
	LockVM();
	StopSnd();

	if (conv_vfd_to_d77(src, dst, DiskTitle)) {
	}

	PlaySnd();
	ResetSch();
	UnlockVM();

	p = strrchr(ddlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[0], ddlg.sFilename);
	}
}

/*
 *	2D→D77変換(2)
 */
static void FASTCALL On2D2D77(GtkWidget *widget, gpointer data)
{
	char *p;
	FileSelectDialog sdlg,ddlg;
	DiskTitleDialog tdlg;
	char src[MAXPATHLEN];
	char dst[MAXPATHLEN];
	int ret;

	/* ファイル選択 */
	sdlg = OpenFileSelectDialog(InitialDir[0]);
	if (sdlg.bResult != DLG_OK ) {
		return;
	}
	strcpy(src, sdlg.sFilename);

	/* タイトル入力 */
	tdlg = OpenDiskTitleDialog();
	if (tdlg.bResult != DLG_OK) {
		return;
	}

	strcpy(DiskTitle, tdlg.sTitle);

	/* ファイル選択 */
	ddlg = OpenFileSelectDialog(InitialDir[0]);
	if (ddlg.bResult != DLG_OK ) {
		return;
	}
	strcpy(dst, ddlg.sFilename);

	/* 作成 */
	LockVM();
	StopSnd();

	if (conv_2d_to_d77(src, dst, DiskTitle)) {
	}

	PlaySnd();
	ResetSch();
	UnlockVM();

	p = strrchr(ddlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[0], ddlg.sFilename);
	}
}

/*
 *	VTP→T77変換(P)
 */
static void FASTCALL OnVTP2T77(GtkWidget *widget, gpointer data)
{
	char *p;
	FileSelectDialog sdlg,ddlg;
	char src[MAXPATHLEN];
	char dst[MAXPATHLEN];

	/* ファイル選択 */
	sdlg = OpenFileSelectDialog(InitialDir[1]);
	if (sdlg.bResult != DLG_OK ) {
		return;
	}
	strcpy(src, sdlg.sFilename);

	/* ファイル選択 */
	ddlg = OpenFileSelectDialog(InitialDir[1]);
	if (ddlg.bResult != DLG_OK ) {
		return;
	}
	strcpy(dst, ddlg.sFilename);

	/* 作成 */
	LockVM();
	StopSnd();

	if (conv_vtp_to_t77(src, dst)) {
	}

	PlaySnd();
	ResetSch();
	UnlockVM();

	p = strrchr(ddlg.sFilename, '/');
	if ( p!=NULL) {
		p[1] = '\0';
		strcpy(InitialDir[1], ddlg.sFilename);
	}
}

/*
 *	サウンド出力モード切り替え
 */
static void FASTCALL OnChgSound(GtkWidget *widget, gpointer data)
{
	LockVM();

	/* サウンドモード変更 */
	nStereoOut = (nStereoOut + 1) % 4;

	/* 適用 */
	ApplySnd();
	UnlockVM();
}


/*
 * 「ツール」メニューを作成
 */
static void FASTCALL CreateToolMenu 
	(GtkWidget *menu_bar, GtkAccelGroup *accel_group)
{
	GtkWidget *sub_item;

	/* ヘルプメニューの作成 */
	tool_menu = gtk_menu_new();

	/*********************************************************/

	/* 「設定」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label ("設定");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnConfig), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* セパレータを作成 */
	sub_item = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_widget_show (sub_item);

	/*********************************************************/
#ifdef MOUSE
	/*「マウスモード」ボタンを作成 */
	miMouseCapture = gtk_check_menu_item_new_with_label("マウスモード");
	gtk_menu_append (GTK_MENU(tool_menu), miMouseCapture);
	hidMouseCapture = gtk_signal_connect (GTK_OBJECT(miMouseCapture), "activate",
		GTK_SIGNAL_FUNC (OnMouseMode), NULL);
	gtk_widget_show (miMouseCapture);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(miMouseCapture), FALSE);
#endif
	/*********************************************************/

	/*「時刻アジャスト」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("時刻アジャスト");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnTimeAdjust), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* セパレータを作成 */
	sub_item = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*「画面キャプチャ」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("画面キャプチャ");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnGrpCapture), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*「縮小画像キャプチャ」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("縮小画像キャプチャ");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnGrpCapture2), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/* 「WAVキャプチャ」ボタンを作成 */
	miWavCapture = gtk_check_menu_item_new_with_label("WAVキャプチャ");
	gtk_menu_append (GTK_MENU(tool_menu), miWavCapture);
	hidWavCapture = gtk_signal_connect (GTK_OBJECT(miWavCapture), "activate",
		GTK_SIGNAL_FUNC (OnWavCapture), NULL);
	gtk_widget_show (miWavCapture);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (miWavCapture), FALSE);

	/*********************************************************/

	/* セパレータを作成 */
	sub_item = gtk_menu_item_new();
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*「新規ディスク作成」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("新規ディスク作成");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnNewDisk), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*「新規テープ作成」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("新規テープ作成");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnNewTape), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*「VFD→D77変換」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("VFD→D77変換");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnVFD2D77), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*「2D→D77変換」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("2D→D77変換");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (On2D2D77), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/

	/*「VTP→T77変換」ボタンを作成 */
	sub_item = gtk_menu_item_new_with_label("VTP→T77変換");
	gtk_menu_append (GTK_MENU(tool_menu), sub_item);
	gtk_signal_connect (GTK_OBJECT(sub_item), "activate",
		GTK_SIGNAL_FUNC (OnVTP2T77), NULL);
	gtk_widget_show (sub_item);

	/*********************************************************/
	/*********************************************************/

	/*ツールメニューをのせるメニューアイテムの作成 */
	tool_item = gtk_menu_item_new_with_label ("ツール");
	gtk_widget_show (tool_item);

	/* ツールメニューアイテムにドライブメニューをのせる */
	gtk_menu_item_set_submenu (GTK_MENU_ITEM(tool_item), tool_menu);

	/* メニュバーにツールメニューアイテムをのせる */
	gtk_menu_bar_append (GTK_MENU_BAR(menu_bar), tool_item);

	gtk_signal_connect (GTK_OBJECT(tool_item), "activate",
		GTK_SIGNAL_FUNC (OnToolPopup), NULL);
	/*********************************************************/

	return;
}

/*-[ メニューバー ]-----------------------------------------------------*/

/*
 *	メニューバーの生成
 */
void FASTCALL CreateMenu(GtkWidget *parent)
{
	GtkWidget *hbox1, *menu_bar;
	GtkAccelGroup *accel_group;

	memset(InitialDir, 0x00, 5*MAXPATHLEN);

	/* メインウィンドウ用のショートカット対応表を作る */
	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group (
		GTK_WINDOW(wndMain), accel_group);

	/* メニューバーをのせる土台となる水平ボックス */
	hbox1 = gtk_hbox_new( FALSE, 0 );
	gtk_box_pack_start (GTK_BOX(parent), hbox1, FALSE, FALSE, 0);
	gtk_widget_show( hbox1 );

	/* メニューバーの作成 */
	menu_bar = gtk_menu_bar_new();
	gtk_container_add (GTK_CONTAINER (hbox1), GTK_WIDGET(menu_bar));
	gtk_widget_show (menu_bar);

	/* 「ファイル」メニューを作成する関数を呼び出す */
	CreateFileMenu(menu_bar, accel_group);

	/* 「ドライブ1」メニューを作成する関数を呼び出す */
	CreateDiskMenu(menu_bar, accel_group, 1);

	/* 「ドライブ0」メニューを作成する関数を呼び出す */
	CreateDiskMenu(menu_bar, accel_group, 0);

	/* 「テープ」メニューを作成する関数を呼び出す */
	CreateTapeMenu(menu_bar, accel_group);

	/* 「デバッグ」メニューを作成する関数を呼び出す */
	CreateDebugMenu(menu_bar, accel_group);

	/* 「ツール」メニューを作成する関数を呼び出す */
	CreateToolMenu(menu_bar, accel_group);

	/* 「ヘルプ」メニューを作成する関数を呼び出す */
	CreateHelpMenu(menu_bar, accel_group);
}

/*
 *	ファイルドロップサブ
 */
void FASTCALL OnDropSub(char *path)
{
	char InitDir[MAXPATHLEN];
	char *ext = NULL;
	char *p = NULL;

	/* 拡張子だけ分離 */
	p = strrchr(path, '.');
	if ( p!=NULL) {
		ext = p;
	}

	strcpy(InitDir, path);
	p = strrchr(InitDir, '/');
	if ( p!=NULL) {
		p[1] = '\0';
	} else {
		InitDir[0] = '\0';
	}

	if ( ext!=NULL) {

		/* D77 */
		if (stricmp(ext, ".D77") == 0) {
			strcpy(InitialDir[0], InitDir);
			LockVM();
			StopSnd();
			fdc_setdisk(0, path);
			fdc_setdisk(1, NULL);
			if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
				fdc_setdisk(1, path);
				fdc_setmedia(1, 1);
			}
			system_reset();
			PlaySnd();
			ResetSch();
			UnlockVM();
		}

		/* 2D/VFD */
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

		/* T77 */
		if (stricmp(ext, ".T77") == 0) {
			strcpy(InitialDir[1], InitDir);
			LockVM();
			tape_setfile(path);
			UnlockVM();
		}

		/* XM7 */
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
 *	ファイルドロップ
 */
void FASTCALL OnDropFiles(void)
{
	char path[MAXPATHLEN];

	/* 処理 */
	OnDropSub(path);
}

/*
 *	コマンドライン処理
 */
void FASTCALL OnCmdLine(char *arg)
{
	/* 処理 */
	OnDropSub(arg);
}


#endif	/* _XWIN */

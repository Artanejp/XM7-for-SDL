/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 2004 GIMONS
 * [ XWINプロパティウィンドウ ] 
 */
#ifdef USE_GTK

#include <gtk/gtk.h>
#include "xm7.h"
#include "sdl.h"
#include "sdl_cfg.h"
#include "sdl_prop.h"

/*
 *  グローバル ワーク 
 */
GtkWidget      *winProperty;
GtkWidget      *GP_FM7;
GtkWidget * GP_FM77AV;
#if XM7_VER >= 3
GtkWidget      *GP_AV40EX;
#endif				/*  */
GtkWidget      *GP_HIGHSPEED;
GtkWidget * GP_LOWSPEED;
GtkWidget * GP_CPUCOMBO;
GtkWidget * GP_MAINCPU;
GtkWidget * GP_MAINMMR;

#if XM7_VER >= 3
    GtkWidget * GP_FASTMMR;
#endif				/*  */
GtkWidget      *GP_SUBCPU;
GtkWidget * GP_CPUTEXT;
GtkWidget * GP_CPUDEFAULT;
GtkWidget * GP_TAPESPEED;
GtkWidget * GP_TAPESPEEDMODE;
#ifdef FDDSND
GtkWidget      *GP_FDDWAIT;
#endif				/*  */
GtkWidget      *GP_FULLSPEED;
GtkWidget * GP_AUTOSPEEDADJUST;
GtkWidget * SP_96K;
GtkWidget * SP_88K;
GtkWidget * SP_48K;
GtkWidget * SP_44K;
GtkWidget * SP_22K;
GtkWidget * SP_NONE;
GtkWidget * SP_HQMODE;
GtkWidget * SP_BUFSPIN;
GtkWidget * SP_STEREO;
GtkWidget * SP_MONO;
GtkWidget * SP_STEREOQ;
GtkWidget * SP_STEREOQ_REV;
GtkWidget * SP_STEREOQ_THG;
GtkWidget * SP_BEEPSPIN;
GtkWidget * SP_TAPEMON;
#ifdef FDDSND
GtkWidget      *SP_FDDSOUND;
#endif				/*  */
GtkWidget      *KP_ARROW8DIR;
GtkWidget * KP_USEARROWFOR10;
GtkWidget * KP_KBDREAL;
GtkWidget * JOY1_UNUSED;
GtkWidget      *JOY1_PORT1;
GtkWidget * JOY1_PORT2;
GtkWidget * JOY1_KBD;
GtkWidget * JOY1_DEMPA;
GtkWidget * JOY2_UNUSED;
GtkWidget      *JOY2_PORT1;
GtkWidget * JOY2_PORT2;
GtkWidget * JOY2_KBD;
GtkWidget * JOY2_DEMPA;
GtkWidget * JOY_RAPID[2][4];
GtkWidget      *JOY_RAPID0[2][4];
GtkWidget      *JOY_RAPID1[2][4];
GtkWidget      *JOY_RAPID2[2][4];
GtkWidget      *JOY_RAPID3[2][4];
GtkWidget      *JOY_RAPID4[2][4];
GtkWidget      *JOY_RAPID5[2][4];
GtkWidget      *JOY_RAPID6[2][4];
GtkWidget      *JOY_RAPID8[2][4];
GtkWidget      *JOY_RAPID12[2][4];
GtkWidget      *JOY_RAPID25[2][4];

GtkWidget      *SCP_24K;
GtkWidget * SCP_640X400;
GtkWidget * SCP_1280X800;
GtkWidget * OP_OPNB;
GtkWidget * OP_WHGB;
GtkWidget * OP_THGB;
#ifdef MOUSE
GtkWidget      *OP_MOUSEEM;
GtkWidget * OP_MOUSE_PORT1;
GtkWidget * OP_MOUSE_PORT2;
#endif				/*  */
GtkWidget      *OP_DIGITIZEB;
#if XM7_VER >= 3
GtkWidget      *OP_RAMB;
#endif				/*  */
int             GP_CPUCOMBO_IDX;

/*-[ サポート関数 ]-----------------------------------------------------*/ 
    
    /*
     *  チェック、ラジオボタンのチェック 
     */
void            FASTCALL
CheckDlgButton(GtkWidget * widget, BOOL b)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), (gboolean) b);
}
    /*
     *  オプションメニューのインデックス移動 
     */
void            FASTCALL
SelectOptionMenu(GtkWidget * widget, guint i)
{
    gtk_option_menu_set_history(GTK_OPTION_MENU(widget), i);
}
    /*
     *  オプションメニューのインデックス取得 
     */
int             FASTCALL
GetIdxOptionMenu(GtkWidget * widget)
{
    return gtk_option_menu_get_history(GTK_OPTION_MENU(widget));
}


    /*
     *  テキストBOXのテキスト設定 
     */ 
void            FASTCALL
SetTextBox(GtkWidget * widget, char *txt)
{
    gtk_entry_set_text(GTK_ENTRY(widget), txt);
} 
    /*
     *  スピンボタンの数値設定 
     */ 
void            FASTCALL
SetSpin(GtkWidget * widget, gint v)
{
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), v);
} 
    /*
     *  スピンボタンの数値取得 
     */ 
    gint FASTCALL GetSpin(GtkWidget * widget)
{
    return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}


    /*
     *  ウィジェットのアクティブ状態を設定 
     */ 
void            FASTCALL
SetEnable(GtkWidget * widget, BOOL b)
{
    gtk_widget_set_sensitive(widget, b);
} 
    /*
     *  チェック状態の取得 
     */ 
    BOOL FASTCALL IsDlgButtonChecked(GtkWidget * widget)
{
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}


/*-[ イベント関数 ]-----------------------------------------------------*/ 
    
    /*
     *  テープスピード状態の連動 
     */ 
static void     FASTCALL
OnGP_TAPESPEEDClicked(GtkWidget * widget, gpointer data) 
{
    SetEnable(GP_TAPESPEEDMODE, IsDlgButtonChecked(GP_TAPESPEED));
} 
    /*
     *  テンキー状態の連動 
     */ 
static void     FASTCALL
OnKP_USEARROWFOR10Clicked(GtkWidget * widget, gpointer data) 
{
    SetEnable(KP_ARROW8DIR, IsDlgButtonChecked(KP_USEARROWFOR10));
} 
/*-[ 設定ダイアログ ]-----------------------------------------------------*/ 
    
    /*
     *  プロパティウィンドウの生成 
     */ 
    GtkWidget * FASTCALL OpenPropertyPage(void) 
{
    int            i,
                    j;
    char            stmp[64];
    GtkWidget      *vbox1;
    GtkWidget * propertySheet;
    GtkWidget * vbox2;
    GtkWidget * hbox1;
    GtkWidget * frmFm7_ver;
    GtkWidget * vbox3;
    GSList * GP_FM7_group = NULL;
    GtkWidget * lblFm7_ver;
    GtkWidget * frmCyclesteal;
    GtkWidget * vbox4;
    GSList * GP_HIGHSPEED_group = NULL;
    GtkWidget * lblCyclesteal;
    GtkWidget * frmSpeed;
    GtkWidget * vbox5;
    GtkWidget * hbox2;
    GtkWidget * menu2;
    GtkObject * GP_CPUTEXT_adj;
    GtkWidget * lblCycle;
    GtkWidget * hbox3;
    GtkWidget * lblSpeed;
    GtkWidget * lblGeneral;
    GtkWidget * hbox4;
    GtkWidget * frmSamplingrate;
    GtkWidget * vbox6;
    GSList * SP_96K_group = NULL;
    GtkWidget * lblSamplingrate;
    GtkWidget * vbox7;
    GtkWidget * frmSoundbuffer;
    GtkWidget * vbox8;
    GtkWidget * hbox5;
    GtkWidget * lblBuffersize;
    GtkObject * SP_BUFSPIN_adj;
    GtkWidget * lblMs;
    GtkWidget * hbox6;
    GtkWidget * lblOutputmode;
    GtkWidget * menu3;
    GtkWidget * lblSoundbuffer;
    GtkWidget * frmSoundoption;
    GtkWidget * vbox9;
    GtkWidget * hbox7;
    GtkWidget * lblBeeprate;
    GtkObject * SP_BEEPSPIN_adj;
    GtkWidget * lblHz;
    GtkWidget * lblSoundoption;
    GtkWidget * lblSound;
    GtkWidget * frmKeyboardmap;
    GtkWidget * vbox10;
    GtkWidget * lblKeyboardmap;
    GtkWidget * lblKeyboard;
    GtkWidget * frmJS1;
    GtkWidget      *frmJS2;

    GtkWidget      *vbox20;
    GtkWidget      *vbox21;
    GtkWidget      *vbox22;
    GtkWidget      *hbox30;
    GtkWidget      *hbox31;
    GtkWidget      *hbox32;

    GtkWidget      *lblJoy1;
    GtkWidget      *lblJoy2;
    GtkWidget      *lblJoyStick1;
    GtkWidget      *lblJoyStick2;
    GSList         *JOY1_group = NULL;
    GSList * JOY2_group = NULL;
    GtkWidget * frmJS1b[2];

    GtkWidget      *menu_js_rapid[2][4];
    GtkWidget      *vbox23[2];
    GtkWidget      *vbox24;
    GtkWidget      *vbox25;
    GtkWidget      *lblJoyrapidButton[2][4];	/* 4ボタンまでリザーブしておく 
						 */
    GtkWidget      *lblRapid[2];

    GtkWidget      *frmScreen;
    GtkWidget * lblScreen;
    GtkWidget * lblScreen2;
    GSList * RESO_group = NULL;
    GtkWidget * hbox40;
    GtkWidget      *vbox30;
    GtkWidget      *vbox31;
    GtkWidget      *vbox32;
    GtkWidget      *frmScpReso;
    GtkWidget      *lblScpReso;
    GtkWidget      *vbox14;
    GtkWidget * hbox19;
    GtkWidget * frmOpn;
    GtkWidget * vbox15;
    GtkWidget * lblOpn;
#ifdef MOUSE
    GtkWidget      *frmMouse;
    GtkWidget * vbox16;
    GSList * OP_MOUSE_PORT1_group = NULL;
    GtkWidget * lblMouse;
#endif				/*  */
    GtkWidget      *frmDigitize;
    GtkWidget * lblDigitize;
    
#if XM7_VER >= 3
	GtkWidget * frmExtram;
    GtkWidget * lblExtram;
    
#endif				/*  */
	GtkWidget * lblOption;
    GtkWidget * hbox20;
    GtkWidget * btnCancel;
    GtkWidget * btnOk;
    winProperty = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(winProperty), "XM7の動作設定");
    gtk_window_set_position(GTK_WINDOW(winProperty), GTK_WIN_POS_CENTER);
    gtk_window_set_modal(GTK_WINDOW(winProperty), TRUE);
    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(winProperty), vbox1);
    propertySheet = gtk_notebook_new();
    gtk_widget_show(propertySheet);
    gtk_box_pack_start(GTK_BOX(vbox1), propertySheet, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(propertySheet), 8);
    vbox2 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox2);
    gtk_container_add(GTK_CONTAINER(propertySheet), vbox2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
    hbox1 = gtk_hbox_new(FALSE, 9);
    gtk_widget_show(hbox1);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox1, TRUE, TRUE, 0);
    frmFm7_ver = gtk_frame_new(NULL);
    gtk_widget_show(frmFm7_ver);
    gtk_box_pack_start(GTK_BOX(hbox1), frmFm7_ver, TRUE, TRUE, 0);
    vbox3 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox3);
    gtk_container_add(GTK_CONTAINER(frmFm7_ver), vbox3);
    gtk_container_set_border_width(GTK_CONTAINER(vbox3), 4);
    GP_FM7 = gtk_radio_button_new_with_mnemonic(NULL, "FM-7");
    gtk_widget_show(GP_FM7);
    gtk_box_pack_start(GTK_BOX(vbox3), GP_FM7, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_FM7), GP_FM7_group);
    GP_FM7_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_FM7));
    GP_FM77AV = gtk_radio_button_new_with_mnemonic(NULL, "FM77AV");
    gtk_widget_show(GP_FM77AV);
    gtk_box_pack_start(GTK_BOX(vbox3), GP_FM77AV, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_FM77AV), GP_FM7_group);
    GP_FM7_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_FM77AV));
    
#if XM7_VER >= 3
	GP_AV40EX = gtk_radio_button_new_with_mnemonic(NULL, "FM77AV40EX");
    gtk_widget_show(GP_AV40EX);
    gtk_box_pack_start(GTK_BOX(vbox3), GP_AV40EX, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_AV40EX), GP_FM7_group);
    GP_FM7_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_AV40EX));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_AV40EX), TRUE);
#else				/*  */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_FM77AV), TRUE);
#endif
    lblFm7_ver = gtk_label_new("動作機種");
    gtk_widget_show(lblFm7_ver);
    gtk_frame_set_label_widget(GTK_FRAME(frmFm7_ver), lblFm7_ver);
    gtk_label_set_justify(GTK_LABEL(lblFm7_ver), GTK_JUSTIFY_LEFT);
    frmCyclesteal = gtk_frame_new(NULL);
    gtk_widget_show(frmCyclesteal);
    gtk_box_pack_start(GTK_BOX(hbox1), frmCyclesteal, TRUE, TRUE, 0);
    vbox4 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox4);
    gtk_container_add(GTK_CONTAINER(frmCyclesteal), vbox4);
    gtk_container_set_border_width(GTK_CONTAINER(vbox4), 4);
    GP_HIGHSPEED =
	gtk_radio_button_new_with_mnemonic(NULL, "高速(FM-77以降)");
    gtk_widget_show(GP_HIGHSPEED);
    gtk_box_pack_start(GTK_BOX(vbox4), GP_HIGHSPEED, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_HIGHSPEED),
				GP_HIGHSPEED_group);
    GP_HIGHSPEED_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_HIGHSPEED));
    GP_LOWSPEED =
	gtk_radio_button_new_with_mnemonic(NULL, "低速(FM-7互換)");
    gtk_widget_show(GP_LOWSPEED);
    gtk_box_pack_start(GTK_BOX(vbox4), GP_LOWSPEED, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_LOWSPEED),
				GP_HIGHSPEED_group);
    GP_HIGHSPEED_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_LOWSPEED));
    lblCyclesteal = gtk_label_new("動作モード");
    gtk_widget_show(lblCyclesteal);
    gtk_frame_set_label_widget(GTK_FRAME(frmCyclesteal), lblCyclesteal);
    gtk_label_set_justify(GTK_LABEL(lblCyclesteal), GTK_JUSTIFY_LEFT);
    frmSpeed = gtk_frame_new(NULL);
    gtk_widget_show(frmSpeed);
    gtk_box_pack_start(GTK_BOX(vbox2), frmSpeed, FALSE, TRUE, 0);
    vbox5 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox5);
    gtk_container_add(GTK_CONTAINER(frmSpeed), vbox5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox5), 4);
    hbox2 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox2);
    gtk_box_pack_start(GTK_BOX(vbox5), hbox2, FALSE, TRUE, 0);
    GP_CPUCOMBO = gtk_option_menu_new();
    gtk_widget_show(GP_CPUCOMBO);
    gtk_box_pack_start(GTK_BOX(hbox2), GP_CPUCOMBO, FALSE, FALSE, 0);
    menu2 = gtk_menu_new();
    GP_MAINCPU = gtk_menu_item_new_with_mnemonic("メインCPU");
    gtk_widget_show(GP_MAINCPU);
    gtk_container_add(GTK_CONTAINER(menu2), GP_MAINCPU);
    GP_MAINMMR = gtk_menu_item_new_with_mnemonic("MMR使用時");
    gtk_widget_show(GP_MAINMMR);
    gtk_container_add(GTK_CONTAINER(menu2), GP_MAINMMR);
    
#if XM7_VER >= 3
	GP_FASTMMR =
	gtk_menu_item_new_with_mnemonic("MMR高速モード時");
    gtk_widget_show(GP_FASTMMR);
    gtk_container_add(GTK_CONTAINER(menu2), GP_FASTMMR);
    
#endif
	GP_SUBCPU = gtk_menu_item_new_with_mnemonic("サブCPU");
    gtk_widget_show(GP_SUBCPU);
    gtk_container_add(GTK_CONTAINER(menu2), GP_SUBCPU);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(GP_CPUCOMBO), menu2);
    GP_CPUTEXT_adj = gtk_adjustment_new(0, 0, 1e+09, 1, 10, 10);
    GP_CPUTEXT =
	gtk_spin_button_new(GTK_ADJUSTMENT(GP_CPUTEXT_adj), 1, 0);
    gtk_widget_show(GP_CPUTEXT);
    gtk_box_pack_start(GTK_BOX(hbox2), GP_CPUTEXT, FALSE, FALSE, 0);
    lblCycle = gtk_label_new("サイクル/ms");
    gtk_widget_show(lblCycle);
    gtk_box_pack_start(GTK_BOX(hbox2), lblCycle, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblCycle), GTK_JUSTIFY_LEFT);
    GP_CPUDEFAULT = gtk_button_new_with_mnemonic("Default");
    gtk_widget_show(GP_CPUDEFAULT);
    gtk_box_pack_start(GTK_BOX(hbox2), GP_CPUDEFAULT, FALSE, FALSE, 0);
    GP_TAPESPEED =
	gtk_check_button_new_with_mnemonic
	("テープモータオン時はフルスピード動作");
    gtk_widget_show(GP_TAPESPEED);
    gtk_box_pack_start(GTK_BOX(vbox5), GP_TAPESPEED, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_TAPESPEED), TRUE);
    GP_TAPESPEEDMODE =
	gtk_check_button_new_with_mnemonic
	("テープモータオン時の動作を速度調整省略にする");
    gtk_widget_show(GP_TAPESPEEDMODE);
    gtk_box_pack_start(GTK_BOX(vbox5), GP_TAPESPEEDMODE, FALSE, FALSE, 0);
    
#ifdef FDDSND
	GP_FDDWAIT =
	gtk_check_button_new_with_mnemonic
	("フロッピーアクセス時にウェイトを挿入する");
    gtk_widget_show(GP_FDDWAIT);
    gtk_box_pack_start(GTK_BOX(vbox5), GP_FDDWAIT, FALSE, FALSE, 0);
    
#endif
	hbox3 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox3);
    gtk_box_pack_start(GTK_BOX(vbox5), hbox3, TRUE, TRUE, 0);
    GP_FULLSPEED = gtk_check_button_new_with_mnemonic("全力駆動");
    gtk_widget_show(GP_FULLSPEED);
    gtk_box_pack_start(GTK_BOX(hbox3), GP_FULLSPEED, FALSE, FALSE, 0);
    GP_AUTOSPEEDADJUST =
	gtk_check_button_new_with_mnemonic("自動速度調整");
    gtk_widget_show(GP_AUTOSPEEDADJUST);
    gtk_box_pack_start(GTK_BOX(hbox3), GP_AUTOSPEEDADJUST, TRUE, FALSE,
			0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_AUTOSPEEDADJUST),
				  TRUE);
    lblSpeed = gtk_label_new("動作速度");
    gtk_widget_show(lblSpeed);
    gtk_frame_set_label_widget(GTK_FRAME(frmSpeed), lblSpeed);
    gtk_label_set_justify(GTK_LABEL(lblSpeed), GTK_JUSTIFY_LEFT);
    lblGeneral = gtk_label_new("全般");
    gtk_widget_show(lblGeneral);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(propertySheet),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK
							   (propertySheet),
							   0), lblGeneral);
    gtk_label_set_justify(GTK_LABEL(lblGeneral), GTK_JUSTIFY_LEFT);
    hbox4 = gtk_hbox_new(FALSE, 8);
    gtk_widget_show(hbox4);
    gtk_container_add(GTK_CONTAINER(propertySheet), hbox4);
    frmSamplingrate = gtk_frame_new(NULL);
    gtk_widget_show(frmSamplingrate);
    gtk_box_pack_start(GTK_BOX(hbox4), frmSamplingrate, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frmSamplingrate), 4);
    vbox6 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox6);
    gtk_container_add(GTK_CONTAINER(frmSamplingrate), vbox6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox6), 4);
    SP_96K = gtk_radio_button_new_with_mnemonic(NULL, "96.000kHz");
    gtk_widget_show(SP_96K);
    gtk_box_pack_start(GTK_BOX(vbox6), SP_96K, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_96K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_96K));
    SP_88K = gtk_radio_button_new_with_mnemonic(NULL, "88.200kHz");
    gtk_widget_show(SP_88K);
    gtk_box_pack_start(GTK_BOX(vbox6), SP_88K, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_88K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_88K));
    SP_48K = gtk_radio_button_new_with_mnemonic(NULL, "48.000kHz");
    gtk_widget_show(SP_48K);
    gtk_box_pack_start(GTK_BOX(vbox6), SP_48K, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_48K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_48K));
    SP_44K = gtk_radio_button_new_with_mnemonic(NULL, "44.100kHz");
    gtk_widget_show(SP_44K);
    gtk_box_pack_start(GTK_BOX(vbox6), SP_44K, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_44K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_44K));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SP_44K), TRUE);
    SP_22K = gtk_radio_button_new_with_mnemonic(NULL, "22.050kHz");
    gtk_widget_show(SP_22K);
    gtk_box_pack_start(GTK_BOX(vbox6), SP_22K, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_22K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_22K));
    SP_NONE =
	gtk_radio_button_new_with_mnemonic(NULL, "合成しない");
    gtk_widget_show(SP_NONE);
    gtk_box_pack_start(GTK_BOX(vbox6), SP_NONE, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_NONE), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_NONE));
    SP_HQMODE = gtk_check_button_new_with_mnemonic("線型補間");
    gtk_widget_show(SP_HQMODE);
    gtk_box_pack_start(GTK_BOX(vbox6), SP_HQMODE, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SP_HQMODE), TRUE);
    lblSamplingrate = gtk_label_new("サンプリングレート");
    gtk_widget_show(lblSamplingrate);
    gtk_frame_set_label_widget(GTK_FRAME(frmSamplingrate),
				lblSamplingrate);
    gtk_label_set_justify(GTK_LABEL(lblSamplingrate), GTK_JUSTIFY_LEFT);
    vbox7 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox7);
    gtk_box_pack_start(GTK_BOX(hbox4), vbox7, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox7), 4);
    frmSoundbuffer = gtk_frame_new(NULL);
    gtk_widget_show(frmSoundbuffer);
    gtk_box_pack_start(GTK_BOX(vbox7), frmSoundbuffer, FALSE, TRUE, 0);
    vbox8 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox8);
    gtk_container_add(GTK_CONTAINER(frmSoundbuffer), vbox8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox8), 4);
    hbox5 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox5);
    gtk_box_pack_start(GTK_BOX(vbox8), hbox5, FALSE, TRUE, 0);
    lblBuffersize = gtk_label_new("バッファサイズ");
    gtk_widget_show(lblBuffersize);
    gtk_box_pack_start(GTK_BOX(hbox5), lblBuffersize, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblBuffersize), GTK_JUSTIFY_LEFT);
    SP_BUFSPIN_adj = gtk_adjustment_new(100, 80, 1000, 10, 10, 10);
    SP_BUFSPIN =
	gtk_spin_button_new(GTK_ADJUSTMENT(SP_BUFSPIN_adj), 1, 0);
    gtk_widget_show(SP_BUFSPIN);
    gtk_box_pack_start(GTK_BOX(hbox5), SP_BUFSPIN, FALSE, FALSE, 0);
    lblMs = gtk_label_new("ms");
    gtk_widget_show(lblMs);
    gtk_box_pack_start(GTK_BOX(hbox5), lblMs, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblMs), GTK_JUSTIFY_LEFT);
    hbox6 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox6);
    gtk_box_pack_start(GTK_BOX(vbox8), hbox6, FALSE, TRUE, 0);
    lblOutputmode = gtk_label_new("出力モード");
    gtk_widget_show(lblOutputmode);
    gtk_box_pack_start(GTK_BOX(hbox6), lblOutputmode, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblOutputmode), GTK_JUSTIFY_LEFT);
    SP_STEREO = gtk_option_menu_new();
    gtk_widget_show(SP_STEREO);
    gtk_box_pack_start(GTK_BOX(hbox6), SP_STEREO, FALSE, FALSE, 0);
    menu3 = gtk_menu_new();
    SP_MONO = gtk_menu_item_new_with_mnemonic("モノラル");
    gtk_widget_show(SP_MONO);
    gtk_container_add(GTK_CONTAINER(menu3), SP_MONO);
    SP_STEREOQ = gtk_menu_item_new_with_mnemonic("ステレオ(WHG)");
    gtk_widget_show(SP_STEREOQ);
    gtk_container_add(GTK_CONTAINER(menu3), SP_STEREOQ);
    SP_STEREOQ_REV =
	gtk_menu_item_new_with_mnemonic("ステレオ(反転)");
    gtk_widget_show(SP_STEREOQ_REV);
    gtk_container_add(GTK_CONTAINER(menu3), SP_STEREOQ_REV);
    SP_STEREOQ_THG =
	gtk_menu_item_new_with_mnemonic("ステレオ(THG)");
    gtk_widget_show(SP_STEREOQ_THG);
    gtk_container_add(GTK_CONTAINER(menu3), SP_STEREOQ_THG);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(SP_STEREO), menu3);
    lblSoundbuffer = gtk_label_new("サウンドバッファ");
    gtk_widget_show(lblSoundbuffer);
    gtk_frame_set_label_widget(GTK_FRAME(frmSoundbuffer), lblSoundbuffer);
    gtk_label_set_justify(GTK_LABEL(lblSoundbuffer), GTK_JUSTIFY_LEFT);
    frmSoundoption = gtk_frame_new(NULL);
    gtk_widget_show(frmSoundoption);
    gtk_box_pack_start(GTK_BOX(vbox7), frmSoundoption, TRUE, TRUE, 0);
    vbox9 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox9);
    gtk_container_add(GTK_CONTAINER(frmSoundoption), vbox9);
    gtk_container_set_border_width(GTK_CONTAINER(vbox9), 4);
    hbox7 = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox7);
    gtk_box_pack_start(GTK_BOX(vbox9), hbox7, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(hbox7), 4);
    lblBeeprate = gtk_label_new("BEEP発振周波数");
    gtk_widget_show(lblBeeprate);
    gtk_box_pack_start(GTK_BOX(hbox7), lblBeeprate, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblBeeprate), GTK_JUSTIFY_LEFT);
    SP_BEEPSPIN_adj = gtk_adjustment_new(1200, 100, 9999, 10, 10, 10);
    SP_BEEPSPIN =
	gtk_spin_button_new(GTK_ADJUSTMENT(SP_BEEPSPIN_adj), 1, 0);
    gtk_widget_show(SP_BEEPSPIN);
    gtk_box_pack_start(GTK_BOX(hbox7), SP_BEEPSPIN, FALSE, FALSE, 0);
    lblHz = gtk_label_new("Hz");
    gtk_widget_show(lblHz);
    gtk_box_pack_start(GTK_BOX(hbox7), lblHz, FALSE, FALSE, 0);
    gtk_label_set_justify(GTK_LABEL(lblHz), GTK_JUSTIFY_LEFT);
    SP_TAPEMON =
	gtk_check_button_new_with_mnemonic("テープ音モニタ");
    gtk_widget_show(SP_TAPEMON);
    gtk_box_pack_start(GTK_BOX(vbox9), SP_TAPEMON, FALSE, FALSE, 0);
    
#ifdef FDDSND
	SP_FDDSOUND =
	gtk_check_button_new_with_mnemonic
	("FDDシーク音/CMTリレー音");
    gtk_widget_show(SP_FDDSOUND);
    gtk_box_pack_start(GTK_BOX(vbox9), SP_FDDSOUND, FALSE, FALSE, 0);
    
#endif
	lblSoundoption = gtk_label_new("その他オプション");
    gtk_widget_show(lblSoundoption);
    gtk_frame_set_label_widget(GTK_FRAME(frmSoundoption), lblSoundoption);
    gtk_label_set_justify(GTK_LABEL(lblSoundoption), GTK_JUSTIFY_LEFT);
    lblSound = gtk_label_new("サウンド");
    gtk_widget_show(lblSound);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(propertySheet),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK
							   (propertySheet),
							   1), lblSound);
    gtk_label_set_justify(GTK_LABEL(lblSound), GTK_JUSTIFY_LEFT);
    frmKeyboardmap = gtk_frame_new(NULL);
    gtk_widget_show(frmKeyboardmap);
    gtk_container_add(GTK_CONTAINER(propertySheet), frmKeyboardmap);
    gtk_container_set_border_width(GTK_CONTAINER(frmKeyboardmap), 4);
    vbox10 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox10);
    gtk_container_add(GTK_CONTAINER(frmKeyboardmap), vbox10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox10), 4);
    KP_USEARROWFOR10 =
	gtk_check_button_new_with_mnemonic
	("カーソルキーをテンキーに対応させる");
    gtk_widget_show(KP_USEARROWFOR10);
    gtk_box_pack_start(GTK_BOX(vbox10), KP_USEARROWFOR10, FALSE, FALSE,
			0);
    KP_ARROW8DIR =
	gtk_check_button_new_with_mnemonic
	("カーソルキーの2つ同時押しを斜め方向に対応させる");
    gtk_widget_show(KP_ARROW8DIR);
    gtk_box_pack_start(GTK_BOX(vbox10), KP_ARROW8DIR, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(KP_ARROW8DIR, FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(KP_ARROW8DIR), TRUE);
    KP_KBDREAL =
	gtk_check_button_new_with_mnemonic
	("擬似リアルタイムキースキャン");
    gtk_widget_show(KP_KBDREAL);
    gtk_box_pack_start(GTK_BOX(vbox10), KP_KBDREAL, FALSE, FALSE, 0);
    lblKeyboardmap = gtk_label_new("キーボードマップ");
    gtk_widget_show(lblKeyboardmap);
    gtk_frame_set_label_widget(GTK_FRAME(frmKeyboardmap), lblKeyboardmap);
    gtk_label_set_justify(GTK_LABEL(lblKeyboardmap), GTK_JUSTIFY_LEFT);
    lblKeyboard = gtk_label_new("キーボード");
    gtk_widget_show(lblKeyboard);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(propertySheet),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK
							   (propertySheet),
							   2),
				lblKeyboard);
    gtk_label_set_justify(GTK_LABEL(lblKeyboard), GTK_JUSTIFY_LEFT);
    
	/*
	 * JOYSTICK 
	 */
	// js1menu = gtk_menu_new ();
	/*
	 * JOYSTICK1 設定メニュー 
	 */
	hbox30 = gtk_hbox_new(FALSE, 8);
    gtk_widget_show(hbox30);
    gtk_container_add(GTK_CONTAINER(propertySheet), hbox30);
    gtk_container_set_border_width(GTK_CONTAINER(hbox30), 4);
    frmJS1 = gtk_frame_new(NULL);
    gtk_widget_show(frmJS1);
    gtk_box_pack_start(GTK_BOX(hbox30), frmJS1, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frmJS1), 4);
    vbox21 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox21);
    gtk_container_add(GTK_CONTAINER(frmJS1), vbox21);
    gtk_container_set_border_width(GTK_CONTAINER(vbox21), 4);
    JOY1_UNUSED = gtk_radio_button_new_with_mnemonic(NULL, "UNUSED");
    gtk_widget_show(JOY1_UNUSED);
    gtk_box_pack_start(GTK_BOX(vbox21), JOY1_UNUSED, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_UNUSED), JOY1_group);
    JOY1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_UNUSED));
    JOY1_PORT1 = gtk_radio_button_new_with_mnemonic(NULL, "Port1");
    gtk_widget_show(JOY1_PORT1);
    gtk_box_pack_start(GTK_BOX(vbox21), JOY1_PORT1, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_PORT1), JOY1_group);
    JOY1_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_PORT1));
    JOY1_PORT2 = gtk_radio_button_new_with_mnemonic(NULL, "Port2");
    gtk_widget_show(JOY1_PORT2);
    gtk_box_pack_start(GTK_BOX(vbox21), JOY1_PORT2, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_PORT2), JOY1_group);
    JOY1_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_PORT2));
    JOY1_KBD = gtk_radio_button_new_with_mnemonic(NULL, "JOYKEY");
    gtk_widget_show(JOY1_KBD);
    gtk_box_pack_start(GTK_BOX(vbox21), JOY1_KBD, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_KBD), JOY1_group);
    JOY1_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_KBD));
    JOY1_DEMPA = gtk_radio_button_new_with_mnemonic(NULL, "DEMPA");
    gtk_widget_show(JOY1_DEMPA);
    gtk_box_pack_start(GTK_BOX(vbox21), JOY1_DEMPA, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_DEMPA), JOY1_group);
    JOY1_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_DEMPA));
    lblJoy1 = gtk_label_new("JOYSTICK#1");
    gtk_widget_show(lblJoy1);
    gtk_frame_set_label_widget(GTK_FRAME(frmJS1), lblJoy1);
    gtk_label_set_justify(GTK_LABEL(lblJoy1), GTK_JUSTIFY_LEFT);
    
	/*
	 * JOYSTICK2 設定メニュー 
	 */
	frmJS2 = gtk_frame_new(NULL);
    gtk_widget_show(frmJS2);
    gtk_container_add(GTK_CONTAINER(hbox30), frmJS2);
    gtk_container_set_border_width(GTK_CONTAINER(frmJS2), 4);
    vbox22 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox22);
    gtk_container_add(GTK_CONTAINER(frmJS2), vbox22);
    gtk_container_set_border_width(GTK_CONTAINER(vbox22), 4);
    JOY2_UNUSED = gtk_radio_button_new_with_mnemonic(NULL, "UNUSED");
    gtk_widget_show(JOY2_UNUSED);
    gtk_box_pack_start(GTK_BOX(vbox22), JOY2_UNUSED, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_UNUSED), JOY2_group);
    JOY2_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_UNUSED));
    JOY2_PORT1 = gtk_radio_button_new_with_mnemonic(NULL, "Port1");
    gtk_widget_show(JOY2_PORT1);
    gtk_box_pack_start(GTK_BOX(vbox22), JOY2_PORT1, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_PORT1), JOY2_group);
    JOY2_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_PORT1));
    JOY2_PORT2 = gtk_radio_button_new_with_mnemonic(NULL, "Port2");
    gtk_widget_show(JOY2_PORT2);
    gtk_box_pack_start(GTK_BOX(vbox22), JOY2_PORT2, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_PORT2), JOY2_group);
    JOY2_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_PORT2));
    JOY2_KBD = gtk_radio_button_new_with_mnemonic(NULL, "JOYKEY");
    gtk_widget_show(JOY2_KBD);
    gtk_box_pack_start(GTK_BOX(vbox22), JOY2_KBD, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_KBD), JOY2_group);
    JOY2_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_KBD));
    JOY2_DEMPA = gtk_radio_button_new_with_mnemonic(NULL, "DEMPA");
    gtk_widget_show(JOY2_DEMPA);
    gtk_box_pack_start(GTK_BOX(vbox22), JOY2_DEMPA, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_DEMPA), JOY2_group);
    JOY2_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_DEMPA));
    lblJoy2 = gtk_label_new("JOYSTICK#2");
    gtk_widget_show(lblJoy2);
    gtk_frame_set_label_widget(GTK_FRAME(frmJS2), lblJoy2);
    gtk_label_set_justify(GTK_LABEL(lblJoy2), GTK_JUSTIFY_LEFT);
    lblJoyStick1 = gtk_label_new("ジョイスティック");
    gtk_widget_show(lblJoyStick1);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(propertySheet),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK
							   (propertySheet),
							   3),
				lblJoyStick1);
    gtk_label_set_justify(GTK_LABEL(lblJoyStick1), GTK_JUSTIFY_LEFT);
    
	/*
	 * JOYSTICK B (連射関連) 
	 */
	hbox31 = gtk_hbox_new(FALSE, 8);
    gtk_widget_show(hbox31);
    gtk_container_add(GTK_CONTAINER(propertySheet), hbox31);
    gtk_container_set_border_width(GTK_CONTAINER(hbox31), 4);
    for (j = 0; j < 2; j++) {
	frmJS1b[j] = gtk_frame_new(NULL);
	gtk_widget_show(frmJS1b[j]);
	gtk_box_pack_start(GTK_BOX(hbox31), frmJS1b[j], TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frmJS1b[j]), 4);
	vbox23[j] = gtk_vbox_new(FALSE, 8);
	gtk_widget_show(vbox23[j]);
	gtk_container_add(GTK_CONTAINER(frmJS1b[j]), vbox23[j]);
	gtk_container_set_border_width(GTK_CONTAINER(vbox23[j]), 4);
	for (i = 0; i < 2; i++) {
	    sprintf(stmp, "ボタン%d", i);
	    lblJoyrapidButton[j][i] = gtk_label_new(stmp);
	    gtk_widget_show(lblJoyrapidButton[j][i]);
	    gtk_box_pack_start(GTK_BOX(vbox23[j]),
				lblJoyrapidButton[j][i], FALSE, FALSE, 0);
	    gtk_label_set_justify(GTK_LABEL(lblJoyrapidButton[j][i]),
				   GTK_JUSTIFY_LEFT);
	    
		/*
		 * 連射関連 
		 */
		JOY_RAPID[j][i] = gtk_option_menu_new();
	    gtk_widget_show(JOY_RAPID[j][i]);
	    gtk_box_pack_start(GTK_BOX(vbox23[j]), JOY_RAPID[j][i], FALSE,
				FALSE, 0);
	    menu_js_rapid[j][i] = gtk_menu_new();
	    JOY_RAPID0[j][i] =
		gtk_menu_item_new_with_mnemonic("連射なし");
	    gtk_widget_show(JOY_RAPID0[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID0[j][i]);
	    JOY_RAPID1[j][i] = gtk_menu_item_new_with_mnemonic("1連射");
	    gtk_widget_show(JOY_RAPID1[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID1[j][i]);
	    JOY_RAPID2[j][i] = gtk_menu_item_new_with_mnemonic("2連射");
	    gtk_widget_show(JOY_RAPID2[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID2[j][i]);
	    JOY_RAPID3[j][i] = gtk_menu_item_new_with_mnemonic("3連射");
	    gtk_widget_show(JOY_RAPID3[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID3[j][i]);
	    JOY_RAPID4[j][i] = gtk_menu_item_new_with_mnemonic("4連射");
	    gtk_widget_show(JOY_RAPID4[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID4[j][i]);
	    JOY_RAPID5[j][i] = gtk_menu_item_new_with_mnemonic("5連射");
	    gtk_widget_show(JOY_RAPID5[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID5[j][i]);
	    JOY_RAPID6[j][i] = gtk_menu_item_new_with_mnemonic("6連射");
	    gtk_widget_show(JOY_RAPID6[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID6[j][i]);
	    JOY_RAPID8[j][i] = gtk_menu_item_new_with_mnemonic("8連射");
	    gtk_widget_show(JOY_RAPID8[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID8[j][i]);
	    JOY_RAPID12[j][i] =
		gtk_menu_item_new_with_mnemonic("12連射");
	    gtk_widget_show(JOY_RAPID12[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID12[j][i]);
	    JOY_RAPID25[j][i] =
		gtk_menu_item_new_with_mnemonic("25連射");
	    gtk_widget_show(JOY_RAPID25[j][i]);
	    gtk_container_add(GTK_CONTAINER(menu_js_rapid[j][i]),
			       JOY_RAPID25[j][i]);
	    gtk_option_menu_set_menu(GTK_OPTION_MENU(JOY_RAPID[j][i]),
				      menu_js_rapid[j][i]);
	}
	sprintf(stmp, "#%d", j);
	lblRapid[j] = gtk_label_new(stmp);
	gtk_widget_show(lblRapid[j]);
	gtk_frame_set_label_widget(GTK_FRAME(frmJS1b[j]), lblRapid[j]);
	gtk_label_set_justify(GTK_LABEL(lblRapid[j]), GTK_JUSTIFY_LEFT);
    }
    lblJoyStick2 = gtk_label_new("連射");
    gtk_widget_show(lblJoyStick2);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(propertySheet),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK
							   (propertySheet),
							   4),
				lblJoyStick2);
    gtk_label_set_justify(GTK_LABEL(lblJoyStick2), GTK_JUSTIFY_LEFT);
    
	/*
	 * 解像度 
	 */
	vbox30 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox30);
    gtk_container_add(GTK_CONTAINER(propertySheet), vbox30);
    gtk_container_set_border_width(GTK_CONTAINER(vbox30), 4);
    vbox31 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox31);
    gtk_container_add(GTK_CONTAINER(vbox30), vbox31);
    gtk_container_set_border_width(GTK_CONTAINER(vbox31), 4);
    frmScpReso = gtk_frame_new(NULL);
    gtk_widget_show(frmScpReso);
    gtk_container_add(GTK_CONTAINER(vbox31), frmScpReso);
    gtk_container_set_border_width(GTK_CONTAINER(frmScpReso), 4);
    vbox32 = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox32);
    gtk_container_add(GTK_CONTAINER(frmScpReso), vbox32);
    gtk_container_set_border_width(GTK_CONTAINER(vbox32), 4);
    SCP_640X400 = gtk_radio_button_new_with_mnemonic(NULL, "640x400");
    gtk_widget_show(SCP_640X400);
    gtk_box_pack_start(GTK_BOX(vbox32), SCP_640X400, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SCP_640X400), RESO_group);
    RESO_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(SCP_640X400));
    SCP_1280X800 = gtk_radio_button_new_with_mnemonic(NULL, "1280x800");
    gtk_widget_show(SCP_1280X800);
    gtk_box_pack_start(GTK_BOX(vbox32), SCP_1280X800, FALSE, FALSE, 0);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SCP_1280X800),
				RESO_group);
    RESO_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(SCP_1280X800));
    frmScreen = gtk_frame_new(NULL);
    gtk_widget_show(frmScreen);
    gtk_container_add(GTK_CONTAINER(vbox30), frmScreen);
    gtk_container_set_border_width(GTK_CONTAINER(frmScreen), 4);
    SCP_24K =
	gtk_check_button_new_with_mnemonic
	("ウィンドウモード時に24Hz表示をエミュレート");
    gtk_widget_show(SCP_24K);
    gtk_container_add(GTK_CONTAINER(frmScreen), SCP_24K);
    gtk_container_set_border_width(GTK_CONTAINER(SCP_24K), 4);
    lblScpReso = gtk_label_new("描画サイズ");
    gtk_widget_show(lblScpReso);
    gtk_frame_set_label_widget(GTK_FRAME(frmScpReso), lblScpReso);
    gtk_label_set_justify(GTK_LABEL(lblScpReso), GTK_JUSTIFY_LEFT);
    lblScreen = gtk_label_new("その他");
    gtk_widget_show(lblScreen);
    gtk_frame_set_label_widget(GTK_FRAME(frmScreen), lblScreen);
    gtk_label_set_justify(GTK_LABEL(lblScreen), GTK_JUSTIFY_LEFT);
    lblScreen2 = gtk_label_new("スクリーン");
    gtk_widget_show(lblScreen2);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(propertySheet),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK
							   (propertySheet),
							   5), lblScreen2);
    gtk_label_set_justify(GTK_LABEL(lblScreen2), GTK_JUSTIFY_LEFT);
    vbox14 = gtk_vbox_new(FALSE, 4);
    gtk_widget_show(vbox14);
    gtk_container_add(GTK_CONTAINER(propertySheet), vbox14);
    gtk_container_set_border_width(GTK_CONTAINER(vbox14), 4);
    hbox19 = gtk_hbox_new(TRUE, 8);
    gtk_widget_show(hbox19);
    gtk_box_pack_start(GTK_BOX(vbox14), hbox19, TRUE, TRUE, 0);
    frmOpn = gtk_frame_new(NULL);
    gtk_widget_show(frmOpn);
    gtk_box_pack_start(GTK_BOX(hbox19), frmOpn, TRUE, TRUE, 0);
    vbox15 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox15);
    gtk_container_add(GTK_CONTAINER(frmOpn), vbox15);
    gtk_container_set_border_width(GTK_CONTAINER(vbox15), 4);
    OP_OPNB = gtk_check_button_new_with_mnemonic("標準FM音源有効");
    gtk_widget_show(OP_OPNB);
    gtk_box_pack_start(GTK_BOX(vbox15), OP_OPNB, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(OP_OPNB), 4);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OP_OPNB), TRUE);
    OP_WHGB = gtk_check_button_new_with_mnemonic("WHG音源有効");
    gtk_widget_show(OP_WHGB);
    gtk_box_pack_start(GTK_BOX(vbox15), OP_WHGB, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(OP_WHGB), 4);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OP_WHGB), TRUE);
    OP_THGB = gtk_check_button_new_with_mnemonic("THG音源有効");
    gtk_widget_show(OP_THGB);
    gtk_box_pack_start(GTK_BOX(vbox15), OP_THGB, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(OP_THGB), 4);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OP_THGB), TRUE);
    lblOpn = gtk_label_new("FM音源");
    gtk_widget_show(lblOpn);
    gtk_frame_set_label_widget(GTK_FRAME(frmOpn), lblOpn);
    gtk_label_set_justify(GTK_LABEL(lblOpn), GTK_JUSTIFY_LEFT);
    
#ifdef MOUSE
	frmMouse = gtk_frame_new(NULL);
    gtk_widget_show(frmMouse);
    gtk_box_pack_start(GTK_BOX(hbox19), frmMouse, TRUE, TRUE, 0);
    vbox16 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox16);
    gtk_container_add(GTK_CONTAINER(frmMouse), vbox16);
    gtk_container_set_border_width(GTK_CONTAINER(vbox16), 4);
    OP_MOUSEEM =
	gtk_check_button_new_with_mnemonic("マウスを使用する");
    gtk_widget_show(OP_MOUSEEM);
    gtk_box_pack_start(GTK_BOX(vbox16), OP_MOUSEEM, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(OP_MOUSEEM), 4);
    OP_MOUSE_PORT1 =
	gtk_radio_button_new_with_mnemonic(NULL,
					   "ポート1にマウスを接続");
    gtk_widget_show(OP_MOUSE_PORT1);
    gtk_box_pack_start(GTK_BOX(vbox16), OP_MOUSE_PORT1, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(OP_MOUSE_PORT1), 4);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT1),
				OP_MOUSE_PORT1_group);
    OP_MOUSE_PORT1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT1));
    OP_MOUSE_PORT2 =
	gtk_radio_button_new_with_mnemonic(NULL,
					   "ポート2にマウスを接続");
    gtk_widget_show(OP_MOUSE_PORT2);
    gtk_box_pack_start(GTK_BOX(vbox16), OP_MOUSE_PORT2, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(OP_MOUSE_PORT2), 4);
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT2),
				OP_MOUSE_PORT1_group);
    OP_MOUSE_PORT1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT2));
    lblMouse = gtk_label_new("インテリジェントマウス");
    gtk_widget_show(lblMouse);
    gtk_frame_set_label_widget(GTK_FRAME(frmMouse), lblMouse);
    gtk_label_set_justify(GTK_LABEL(lblMouse), GTK_JUSTIFY_LEFT);
    
#endif
	frmDigitize = gtk_frame_new(NULL);
    gtk_widget_show(frmDigitize);
    gtk_box_pack_start(GTK_BOX(vbox14), frmDigitize, TRUE, TRUE, 0);
    OP_DIGITIZEB = gtk_check_button_new_with_mnemonic("有効");
    gtk_widget_show(OP_DIGITIZEB);
    gtk_container_add(GTK_CONTAINER(frmDigitize), OP_DIGITIZEB);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(OP_DIGITIZEB), TRUE);
    lblDigitize = gtk_label_new("ビデオディジタイズカード");
    gtk_widget_show(lblDigitize);
    gtk_frame_set_label_widget(GTK_FRAME(frmDigitize), lblDigitize);
    gtk_label_set_justify(GTK_LABEL(lblDigitize), GTK_JUSTIFY_LEFT);
    
#if XM7_VER >= 3
	frmExtram = gtk_frame_new(NULL);
    gtk_widget_show(frmExtram);
    gtk_box_pack_start(GTK_BOX(vbox14), frmExtram, TRUE, TRUE, 0);
    OP_RAMB = gtk_check_button_new_with_mnemonic("有効");
    gtk_widget_show(OP_RAMB);
    gtk_container_add(GTK_CONTAINER(frmExtram), OP_RAMB);
    lblExtram = gtk_label_new("拡張RAMカード(768KB)");
    gtk_widget_show(lblExtram);
    gtk_frame_set_label_widget(GTK_FRAME(frmExtram), lblExtram);
    gtk_label_set_justify(GTK_LABEL(lblExtram), GTK_JUSTIFY_LEFT);
    
#endif
	lblOption = gtk_label_new("オプション");
    gtk_widget_show(lblOption);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(propertySheet),
				gtk_notebook_get_nth_page(GTK_NOTEBOOK
							   (propertySheet),
							   6), lblOption);
    gtk_label_set_justify(GTK_LABEL(lblOption), GTK_JUSTIFY_LEFT);
    hbox20 = gtk_hbox_new(FALSE, 8);
    gtk_widget_show(hbox20);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox20, FALSE, FALSE, 0);
    btnCancel = gtk_button_new_with_mnemonic("キャンセル");
    gtk_widget_show(btnCancel);
    gtk_box_pack_end(GTK_BOX(hbox20), btnCancel, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btnCancel), 4);
    btnOk = gtk_button_new_with_mnemonic("    OK    ");
    gtk_widget_show(btnOk);
    gtk_box_pack_end(GTK_BOX(hbox20), btnOk, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(btnOk), 5);
    gtk_signal_connect((gpointer) GP_CPUDEFAULT, "clicked",
			  GTK_SIGNAL_FUNC(OnGP_CPUDEFAULTClicked), NULL);
    gtk_signal_connect((gpointer) GP_CPUCOMBO, "changed",
			 GTK_SIGNAL_FUNC(OnGP_CPUCOMBOChanged), NULL);
    gtk_signal_connect((gpointer) GP_TAPESPEED, "clicked",
			 GTK_SIGNAL_FUNC(OnGP_TAPESPEEDClicked), NULL);
    gtk_signal_connect((gpointer) KP_USEARROWFOR10, "clicked",
			 GTK_SIGNAL_FUNC(OnKP_USEARROWFOR10Clicked), NULL);
    gtk_signal_connect((gpointer) btnOk, "clicked",
			 GTK_SIGNAL_FUNC(OnConfig_OK), NULL);
    g_signal_connect(winProperty, "destroy",
		       GTK_SIGNAL_FUNC(gtk_widget_destroy), NULL);
    gtk_signal_connect_object((gpointer) btnCancel, "clicked",
				 GTK_SIGNAL_FUNC(gtk_widget_destroy),
				 (gpointer) winProperty);
    gtk_signal_connect_object((gpointer) btnOk, "clicked",
				GTK_SIGNAL_FUNC(gtk_widget_destroy),
				(gpointer) winProperty);
    return winProperty;
}


#endif	/* _XWIN */

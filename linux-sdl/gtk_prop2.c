/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 2004 GIMONS  [ XWIN
 * プロパティウィンドウ ] 
 */  

#ifdef _XWIN

#include<gtk/gtk.h>
#include "xm7.h"
#include "sdl.h"
#include "sdl_cfg.h"
#include "sdl_prop.h"
#include "gtk_propkeyboard.h"

/*
 *  グローバル ワーク
 */
GtkWidget       *winProperty;
GtkWidget       *GP_FM7;
GtkWidget       *GP_FM77AV;
#if XM7_VER >= 3
GtkWidget       *GP_AV40EX;
#endif				/*  */
GtkWidget       *GP_HIGHSPEED;
GtkWidget       *GP_LOWSPEED;
GtkWidget       *GP_CPUCOMBO;
GtkWidget       *GP_MAINCPU;
GtkWidget       *GP_MAINMMR;

#if XM7_VER >= 3
GtkWidget       *GP_FASTMMR;
#endif				/*  */
GtkWidget       *GP_SUBCPU;
GtkWidget       *GP_CPUTEXT;
GtkWidget       *GP_CPUDEFAULT;
GtkWidget       *GP_TAPESPEED;
GtkWidget       *GP_TAPESPEEDMODE;
#ifdef FDDSND
GtkWidget       *GP_FDDWAIT;
#endif				/*  */
GtkWidget       *GP_FULLSPEED;
GtkWidget       *GP_AUTOSPEEDADJUST;
GtkWidget       *SP_96K;
GtkWidget       *SP_88K;
GtkWidget       *SP_48K;
GtkWidget       *SP_44K;
GtkWidget       *SP_22K;
GtkWidget       *SP_NONE;
GtkWidget       *SP_HQMODE;
GtkWidget       *SP_BUFSPIN;
GtkWidget       *SP_STEREO;
GtkWidget       *SP_MONO;
GtkWidget       *SP_STEREOQ;
GtkWidget       *SP_STEREOQ_REV;
GtkWidget       *SP_STEREOQ_THG;
GtkWidget       *SP_BEEPSPIN;
GtkWidget       *SP_TAPEMON;
#ifdef FDDSND
GtkWidget       *SP_FDDSOUND;
#endif				/*  */
GtkWidget       *KP_ARROW8DIR;
GtkWidget       *KP_USEARROWFOR10;
GtkWidget       *KP_KBDREAL;
GtkWidget       *JOY1_UNUSED;
GtkWidget       *JOY1_PORT1;
GtkWidget       *JOY1_PORT2;
GtkWidget       *JOY1_KBD;
GtkWidget       *JOY1_DEMPA;
GtkWidget       *JOY2_UNUSED;
GtkWidget       *JOY2_PORT1;
GtkWidget       *JOY2_PORT2;
GtkWidget       *JOY2_KBD;
GtkWidget       *JOY2_DEMPA;
GtkWidget       *JOY_RAPID[2][4];
GtkWidget       *JOY_RAPID0[2][4];
GtkWidget       *JOY_RAPID1[2][4];
GtkWidget       *JOY_RAPID2[2][4];
GtkWidget       *JOY_RAPID3[2][4];
GtkWidget       *JOY_RAPID4[2][4];
GtkWidget       *JOY_RAPID5[2][4];
GtkWidget       *JOY_RAPID6[2][4];
GtkWidget       *JOY_RAPID8[2][4];
GtkWidget       *JOY_RAPID12[2][4];
GtkWidget       *JOY_RAPID25[2][4];

GtkWidget       *SCP_24K;
GtkWidget       *SCP_320X200;
GtkWidget       *SCP_640X400;
GtkWidget       *SCP_1280X800;
GtkWidget 		*SCP_FPS;
GtkWidget       *OP_OPNB;
GtkWidget       *OP_WHGB;
GtkWidget       *OP_THGB;
#ifdef MOUSE
GtkWidget       *OP_MOUSEEM;
GtkWidget       *OP_MOUSE_PORT1;
GtkWidget       *OP_MOUSE_PORT2;
#endif				/*  */
GtkWidget       *OP_DIGITIZEB;
#if XM7_VER >= 3
GtkWidget       *OP_RAMB;
#endif
GtkWidget       *VOLUME_FM;
GtkWidget       *VOLUME_PSG;
GtkWidget       *VOLUME_BEEP;
GtkWidget       *VOLUME_TAPE;
GtkWidget       *VOLUME_WAV;
GtkWidget       *VOLUME_SEP;

/*  */
int             GP_CPUCOMBO_IDX;


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
 *  テキストBOXのテキスト設定
 */
void            FASTCALL
SetTextBox(GtkWidget * widget, char *txt)
{
	gtk_entry_set_text(GTK_ENTRY(widget), txt);
}
/*
 *  スピンボタンの数値設定
 */
void            FASTCALL
SetSpin(GtkWidget * widget, gint v)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), v);
}
/*
 *  スピンボタンの数値取得
 */
gint FASTCALL GetSpin(GtkWidget * widget)
{
	return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}


/*
 *  ウィジェットのアクティブ状態を設定
 */
void            FASTCALL
SetEnable(GtkWidget * widget, BOOL b)
{
	gtk_widget_set_sensitive(widget, b);
}
/*
 *  チェック状態の取得
 */
BOOL IsDlgButtonChecked(GtkWidget * widget)
{
	return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}


/*-[ イベント関数 ]-----------------------------------------------------*/

/*
 *  テープスピード状態の連動
 */
void
OnGP_TAPESPEEDClicked(GtkWidget * widget, gpointer data)
{
	SetEnable(GP_TAPESPEEDMODE, IsDlgButtonChecked(GP_TAPESPEED));
}
/*
 *  テンキー状態の連動
 */
static void
OnKP_USEARROWFOR10Clicked(GtkWidget * widget, gpointer data)
{
	SetEnable(KP_ARROW8DIR, IsDlgButtonChecked(KP_USEARROWFOR10));
}
/*-[ 設定ダイアログ ]-----------------------------------------------------*/



/*-[ サポート関数 ]-----------------------------------------------------*/ 

/*
 * ジョイスティックグループの設定
 */


#define AddRadioGroup(w,g,l) \
		w = \
				GTK_WIDGET(gtk_builder_get_object(gbuilderMain, l)); \
		gtk_radio_button_set_group(GTK_RADIO_BUTTON(w), g); \
		g = gtk_radio_button_get_group(GTK_RADIO_BUTTON(w));

#define AddMenu(w, m, l) \
		w = gtk_menu_item_new_with_mnemonic(l); \
		gtk_widget_show(w); \
		gtk_container_add(GTK_CONTAINER(m), w);


/*
 * Page 0
 */
static void SetEmulationPage()
{
	GSList      *GP_FM7_group = NULL;
	GSList      *GP_HIGHSPEED_group = NULL;
	GtkWidget   *menu2;
	GtkWidget   *hbox2;

	AddRadioGroup(GP_FM7, GP_FM7_group, "radiobutton_EMU_FM7");
	AddRadioGroup(GP_FM77AV, GP_FM7_group, "radiobutton_EMU_77AV");
#if XM7_VER >= 3
	AddRadioGroup(GP_AV40EX, GP_FM7_group, "radiobutton_EMU_77AVEX");
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_FM77AV), TRUE);
#endif

	AddRadioGroup(GP_HIGHSPEED, GP_HIGHSPEED_group, "radiobutton_emumode_fast");
	AddRadioGroup(GP_LOWSPEED, GP_HIGHSPEED_group, "radiobutton_emumode_slow");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_HIGHSPEED), TRUE);

	hbox2 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_cpucycle"));

	GP_CPUCOMBO = gtk_option_menu_new();
	gtk_widget_show(GP_CPUCOMBO);
	gtk_box_pack_start(GTK_BOX(hbox2), GP_CPUCOMBO, FALSE, FALSE, 0);

	menu2 = gtk_menu_new();
	AddMenu(GP_MAINCPU, menu2, "メインCPU");
	AddMenu(GP_MAINMMR, menu2, "MMR使用時");
#if XM7_VER >= 3
	AddMenu(GP_FASTMMR, menu2, "MMR高速モード時");
#endif
	AddMenu(GP_SUBCPU, menu2, "サブCPU");
	gtk_option_menu_set_menu(GTK_OPTION_MENU(GP_CPUCOMBO), menu2);
	gtk_widget_show(GP_CPUCOMBO);

	GP_CPUTEXT =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "spinbutton_CYCLES"));

	GP_CPUDEFAULT =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_SPEED_SETDEFAULT"));
	GP_TAPESPEED =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_tapeon_full"));

	GP_TAPESPEEDMODE =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_tapeon_nowait"));
#ifdef FDDSND
	GP_FDDWAIT =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_fdd_wait"));
#endif
	GP_FULLSPEED =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_fullspeed"));
	GP_AUTOSPEEDADJUST =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_autospeed"));

	g_signal_connect((gpointer) GP_CPUDEFAULT, "clicked",
			GTK_SIGNAL_FUNC(OnGP_CPUDEFAULTClicked), NULL);
	g_signal_connect((gpointer) GP_CPUCOMBO, "changed",
			GTK_SIGNAL_FUNC(OnGP_CPUCOMBOChanged), NULL);
	g_signal_connect((gpointer) GP_TAPESPEED, "clicked",
			GTK_SIGNAL_FUNC(OnGP_TAPESPEEDClicked), NULL);

}

/*
 * Page 1
 */

static void SetSoundPage()
{
	GSList      *SP_96K_group = NULL;
	GtkWidget   *hbox3;
	GtkWidget   *menu3;
	/*
	 * Page 1
	 */
	AddRadioGroup(SP_96K, SP_96K_group, "radiobutton_96k");
	AddRadioGroup(SP_88K, SP_96K_group, "radiobutton_88k");
	AddRadioGroup(SP_48K, SP_96K_group, "radiobutton_48k");
	AddRadioGroup(SP_44K, SP_96K_group, "radiobutton_44k");
	AddRadioGroup(SP_22K, SP_96K_group, "radiobutton_22k");
	AddRadioGroup(SP_NONE, SP_96K_group, "radiobutton_pcm_none");
	SP_HQMODE =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_pcm_hq"));


	hbox3 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_stereomode"));
	SP_STEREO = gtk_option_menu_new();
	gtk_widget_show(SP_STEREO);
	gtk_box_pack_start(GTK_BOX(hbox3), SP_STEREO, FALSE, FALSE, 0);
	menu3 = gtk_menu_new();
	AddMenu(SP_MONO, menu3, "モノラル");
	AddMenu(SP_STEREOQ, menu3, "ステレオ(WHG)");
	AddMenu(SP_STEREOQ_REV, menu3, "ステレオ(反転)");
	AddMenu(SP_STEREOQ_THG, menu3, "ステレオ(THG)");
	gtk_option_menu_set_menu(GTK_OPTION_MENU(SP_STEREO), menu3);

	SP_BUFSPIN =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "spinbutton_SND_BUFFERSIZE"));
	SP_BEEPSPIN =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "spinbutton_SND_BEEP"));
	SP_TAPEMON =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_tapemon"));
#ifdef FDDSND
	SP_FDDSOUND =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_wav"));
#endif
}
/*
 * Page 2
 */
static void SetKeyboardPage(void)
{
	GtkWidget *btnCallKeyMap;

	/*
	 * Page 2
	 */
	KP_USEARROWFOR10 =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_ApplyTenKey"));

	KP_ARROW8DIR =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_DoublePress"));

	KP_KBDREAL =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_PseudoRealtimeKeyScan"));
	btnCallKeyMap =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_CallKeyMap"));

	g_signal_connect((gpointer) btnCallKeyMap, "clicked",
			GTK_SIGNAL_FUNC(OnClick_KeyMap), NULL);
	g_signal_connect((gpointer) KP_USEARROWFOR10, "clicked",
			GTK_SIGNAL_FUNC(OnKP_USEARROWFOR10Clicked), NULL);
}

/*
 * Page 3
 */
static void SetJsPage()
{
	GSList      *JOY1_group = NULL;
	GSList      *JOY2_group = NULL;

	AddRadioGroup(JOY1_UNUSED, JOY1_group, "radiobutton_js1_unused");
	AddRadioGroup(JOY1_PORT1, JOY1_group, "radiobutton_js1_port1");
	AddRadioGroup(JOY1_PORT2, JOY1_group, "radiobutton_js1_port2");
	AddRadioGroup(JOY1_KBD, JOY1_group, "radiobutton_js1_joykey");
	AddRadioGroup(JOY1_DEMPA, JOY1_group, "radiobutton_js1_dempa");

	AddRadioGroup(JOY2_UNUSED, JOY2_group, "radiobutton_js2_unused");
	AddRadioGroup(JOY2_PORT1, JOY2_group, "radiobutton_js2_port1");
	AddRadioGroup(JOY2_PORT2, JOY2_group, "radiobutton_js2_port2");
	AddRadioGroup(JOY2_KBD, JOY2_group, "radiobutton_js2_joykey");
	AddRadioGroup(JOY2_DEMPA, JOY2_group, "radiobutton_js2_dempa");

}

/*
 * Page 4
 */
static void SetJsRapidPage()
{
	int i, j;
	char stmp[256];
	GtkWidget   *menu_js_rapid[2][4];
	GtkWidget   *hbox23[2];

	for (j = 0; j < 2; j++) {
		sprintf(stmp, "vbox%d0", j);

		for (i = 0; i < 2; i++) {
			sprintf(stmp, "hbox_rapid_%d%d", j, i);
			//JOY_RAPID[j][i] = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, stmp));
			hbox23[j] = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, stmp));
			JOY_RAPID[j][i] = gtk_option_menu_new();
			gtk_widget_show(JOY_RAPID[j][i]);
			gtk_box_pack_start(GTK_BOX(hbox23[j]), JOY_RAPID[j][i], FALSE,
					FALSE, 0);
			menu_js_rapid[j][i] = gtk_menu_new();
			AddMenu(JOY_RAPID0[j][i], menu_js_rapid[j][i], "連射なし");
			AddMenu(JOY_RAPID1[j][i], menu_js_rapid[j][i], "1連射");
			AddMenu(JOY_RAPID2[j][i], menu_js_rapid[j][i], "2連射");
			AddMenu(JOY_RAPID3[j][i], menu_js_rapid[j][i], "3連射");
			AddMenu(JOY_RAPID4[j][i], menu_js_rapid[j][i], "4連射");
			AddMenu(JOY_RAPID5[j][i], menu_js_rapid[j][i], "5連射");
			AddMenu(JOY_RAPID6[j][i], menu_js_rapid[j][i], "6連射");
			AddMenu(JOY_RAPID8[j][i], menu_js_rapid[j][i], "8連射");
			AddMenu(JOY_RAPID12[j][i], menu_js_rapid[j][i], "12連射");
			AddMenu(JOY_RAPID25[j][i], menu_js_rapid[j][i], "25連射");
			gtk_option_menu_set_menu(GTK_OPTION_MENU(JOY_RAPID[j][i]),
					menu_js_rapid[j][i]);
		}
	}
}

/*
 * Page 5
 */

static void SetVideoPage(void)
{
	GSList      *RESO_group = NULL;

	AddRadioGroup(SCP_320X200, RESO_group, "radiobutton20");
	AddRadioGroup(SCP_640X400, RESO_group, "radiobutton21");
	AddRadioGroup(SCP_1280X800, RESO_group, "radiobutton22");

	SCP_24K = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_fullscan"));
	SCP_FPS =
			GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "spinbutton_FPS"));
}
/*
 * Page 6
 */
static void SetMiscPage(void)
{
	/*
	 * Page 6
	 */
#ifdef MOUSE
	GSList      *OP_MOUSE_PORT1_group = NULL;
#endif				/*  */
	OP_OPNB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_stdfm"));
	OP_WHGB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_whg"));
	OP_THGB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_thg"));
	OP_MOUSEEM =GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_mouse"));

	AddRadioGroup(OP_MOUSE_PORT1, OP_MOUSE_PORT1_group, "radiobutton_mouse1");
	AddRadioGroup(OP_MOUSE_PORT2, OP_MOUSE_PORT1_group, "radiobutton_mouse2");

	OP_DIGITIZEB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_digitize"));
	OP_RAMB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_ramboard"));

}

/*
 * Page 7
 */
static void SetVolumePage(void)
{
	/*
	 * Page 7
	 */

	VOLUME_FM = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hscale_FM"));
	g_signal_connect(VOLUME_FM, "value-changed",
			GTK_SIGNAL_FUNC(OnGP_FMVolumeChanged), NULL);

	VOLUME_PSG = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hscale_PSG"));
	g_signal_connect(VOLUME_PSG, "value-changed",
			GTK_SIGNAL_FUNC(OnGP_PSGVolumeChanged), NULL);

	VOLUME_BEEP = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hscale_BEEP"));
	g_signal_connect(VOLUME_BEEP, "value-changed",
			GTK_SIGNAL_FUNC(OnGP_BEEPVolumeChanged), NULL);

	VOLUME_TAPE = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hscale_TAPE"));
	g_signal_connect(VOLUME_TAPE, "value-changed",
			GTK_SIGNAL_FUNC(OnGP_CMTVolumeChanged), NULL);


	VOLUME_SEP = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hscale_SEP"));
	g_signal_connect(VOLUME_SEP, "value-changed",
			GTK_SIGNAL_FUNC(OnGP_ChSepVolumeChanged), NULL);

#ifdef FDDSND
	VOLUME_WAV = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hscale_WAV"));
	g_signal_connect(VOLUME_WAV, "value-changed",
			GTK_SIGNAL_FUNC(OnGP_WAVVolumeChanged), NULL);

#endif

}

/*
 *  プロパティウィンドウの生成
 */
GtkWidget 
*OpenPropertyPage(void) 
{


#if XM7_VER >= 3
#endif				/*  */
	/*
	 * Page 7
	 */

	/*
	 * Cancel,OK
	 */
	GtkWidget *btnCancel;
	GtkWidget *btnOk;
	GtkWidget *window;

	//    winProperty = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	window = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "window_prop"));
	gtk_window_set_title(GTK_WINDOW(window), "XM7の動作設定");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_deletable(GTK_WINDOW(window), FALSE);
	//    gtk_widget_show(GTK_WIDGET(winProperty));
	SetEmulationPage();
	SetSoundPage();
	SetKeyboardPage();
	SetJsPage();
	SetJsRapidPage();
	SetVideoPage();
	SetMiscPage();
	SetVolumePage();
	/*
	 *    スペーシング用ダミー
	 */

	/*
	 * OK or キャンセル...共通ボタン
	 */
	btnCancel = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_CANCEL"));
	btnOk = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_OK"));
	/* This is important */
//	gtk_builder_connect_signals (gbuilderMain, NULL);

#if 1
	g_signal_connect((gpointer) btnOk, "clicked",
			GTK_SIGNAL_FUNC(OnConfig_OK), NULL);
#endif
	g_signal_connect(window, "destroy",
			GTK_SIGNAL_FUNC(OnCancelPressed),
			(gpointer)window);

	g_signal_connect(btnCancel, "clicked",
			GTK_SIGNAL_FUNC(OnCancelPressed),
			(gpointer)window);

	g_signal_connect(btnOk, "clicked",
			GTK_SIGNAL_FUNC(OnCancelPressed),
			(gpointer)window);
	return window;
}
#endif	/* _XWIN */

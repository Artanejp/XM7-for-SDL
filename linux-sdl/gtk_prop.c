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

#if XM7_VER >= 3
GtkWidget      *GP_AV40EX;
#endif				/* 
GtkWidget      *GP_HIGHSPEED;





#if XM7_VER >= 3
    GtkWidget * GP_FASTMMR;
#endif				/* 
GtkWidget      *GP_SUBCPU;




#ifdef FDDSND
GtkWidget      *GP_FDDWAIT;
#endif				/* 
GtkWidget      *GP_FULLSPEED;
















#ifdef FDDSND
GtkWidget      *SP_FDDSOUND;
#endif				/* 
GtkWidget      *KP_ARROW8DIR;



GtkWidget      *JOY1_PORT1;




GtkWidget      *JOY2_PORT1;




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





#ifdef MOUSE
GtkWidget      *OP_MOUSEEM;


#endif				/* 
GtkWidget      *OP_DIGITIZEB;
#if XM7_VER >= 3
GtkWidget      *OP_RAMB;
#endif				/* 
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
    



    /*
     * 
     */ 
void            FASTCALL
SetTextBox(GtkWidget * widget, char *txt)
{
    

    /*
     * 
     */ 
void            FASTCALL
SetSpin(GtkWidget * widget, gint v)
{
    

    /*
     * 
     */ 
    gint FASTCALL GetSpin(GtkWidget * widget)
{
    



    /*
     * 
     */ 
void            FASTCALL
SetEnable(GtkWidget * widget, BOOL b)
{
    

    /*
     * 
     */ 
    BOOL FASTCALL IsDlgButtonChecked(GtkWidget * widget)
{
    



/*-[ イベント関数 ]-----------------------------------------------------*/ 
    
    /*
     * 
     */ 
static void     FASTCALL
OnGP_TAPESPEEDClicked(GtkWidget * widget, gpointer data) 
{
    

    /*
     * 
     */ 
static void     FASTCALL
OnKP_USEARROWFOR10Clicked(GtkWidget * widget, gpointer data) 
{
    

/*-[ 設定ダイアログ ]-----------------------------------------------------*/ 
    
    /*
     * 
     */ 
    GtkWidget * FASTCALL OpenPropertyPage(void) 
{
    
                    j;
    char            stmp[64];
    GtkWidget      *vbox1;
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
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
    
    

    GtkWidget      *menu_js_rapid[2][4];
    GtkWidget      *vbox23[2];
    GtkWidget      *vbox24;
    GtkWidget      *vbox25;
    GtkWidget      *lblJoyrapidButton[2][4];	/* 4ボタンまでリザーブしておく 
						 */
    GtkWidget      *lblRapid[2];

    GtkWidget      *frmScreen;
    
    
    
    
    GtkWidget      *vbox30;
    GtkWidget      *vbox31;
    GtkWidget      *vbox32;
    GtkWidget      *frmScpReso;
    GtkWidget      *lblScpReso;
    GtkWidget      *vbox14;
    
    
    
    
#ifdef MOUSE
    GtkWidget      *frmMouse;
    
    
    
#endif				/* 
    GtkWidget      *frmDigitize;
    
    
#if XM7_VER >= 3
	GtkWidget * frmExtram;
    
    
#endif				/* 
	GtkWidget * lblOption;
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_FM77AV));
    
#if XM7_VER >= 3
	GP_AV40EX = gtk_radio_button_new_with_mnemonic(NULL, "FM77AV40EX");
    
    
    
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_AV40EX));
    
#else				/* 
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_FM77AV), TRUE);
#endif
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_radio_button_new_with_mnemonic(NULL, "高速(FM-77以降)");
    
    
    
				GP_HIGHSPEED_group);
    
	
    
	gtk_radio_button_new_with_mnemonic(NULL, "低速(FM-7互換)");
    
    
    
				GP_HIGHSPEED_group);
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_LOWSPEED));
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
#if XM7_VER >= 3
	GP_FASTMMR =
	gtk_menu_item_new_with_mnemonic("MMR高速モード時");
    
    
    
#endif
	
    
    
    
    
    
	gtk_spin_button_new(GTK_ADJUSTMENT(GP_CPUTEXT_adj), 1, 0);
    
    
    
    
    
    
    
    
    
    
	
	("テープモータオン時はフルスピード動作");
    
    
    
    
	
	("テープモータオン時の動作を速度調整省略にする");
    
    
    
#ifdef FDDSND
	GP_FDDWAIT =
	
	("フロッピーアクセス時にウェイトを挿入する");
    
    
    
#endif
	
    
    
    
    
    
    
	gtk_check_button_new_with_mnemonic("自動速度調整");
    
    
			0);
    
				  TRUE);
    
    
    
    
    
    
    
				
							   (propertySheet),
							   0), lblGeneral);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_radio_button_new_with_mnemonic(NULL, "合成しない");
    
    
    
    
    
    
    
    
    
    
    
				lblSamplingrate);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_spin_button_new(GTK_ADJUSTMENT(SP_BUFSPIN_adj), 1, 0);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_menu_item_new_with_mnemonic("ステレオ(反転)");
    
    
    
	gtk_menu_item_new_with_mnemonic("ステレオ(THG)");
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_spin_button_new(GTK_ADJUSTMENT(SP_BEEPSPIN_adj), 1, 0);
    
    
    
    
    
    
    
	gtk_check_button_new_with_mnemonic("テープ音モニタ");
    
    
    
#ifdef FDDSND
	SP_FDDSOUND =
	gtk_check_button_new_with_mnemonic
	("FDDシーク音/CMTリレー音");
    
    
    
#endif
	
    
    
    
    
    
    
				
							   (propertySheet),
							   1), lblSound);
    
    
    
    
    
    
    
    
    
    
	
	("カーソルキーをテンキーに対応させる");
    
    
			0);
    
	
	("カーソルキーの2つ同時押しを斜め方向に対応させる");
    
    
    
    
    
	gtk_check_button_new_with_mnemonic
	("擬似リアルタイムキースキャン");
    
    
    
    
    
    
    
    
    
				
							   (propertySheet),
							   2),
				lblKeyboard);
    
    
	/*
	 * JOYSTICK 
	 */
	// js1menu = gtk_menu_new ();
	/*
	 * JOYSTICK1 設定メニュー 
	 */
	hbox30 = gtk_hbox_new(FALSE, 8);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_UNUSED));
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	/*
	 * JOYSTICK2 設定メニュー 
	 */
	frmJS2 = gtk_frame_new(NULL);
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_UNUSED));
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
				
							   (propertySheet),
							   3),
				lblJoyStick1);
    
    
	/*
	 * JOYSTICK B (連射関連) 
	 */
	hbox31 = gtk_hbox_new(FALSE, 8);
    
    
    
    
	frmJS1b[j] = gtk_frame_new(NULL);
	
	
	
	
	
	
	
	
	    sprintf(stmp, "ボタン%d", i);
	    lblJoyrapidButton[j][i] = gtk_label_new(stmp);
	    
	    
				lblJoyrapidButton[j][i], FALSE, FALSE, 0);
	    
				   GTK_JUSTIFY_LEFT);
	    
		/*
		 * 連射関連 
		 */
		JOY_RAPID[j][i] = gtk_option_menu_new();
	    
	    
				FALSE, 0);
	    
	    
		gtk_menu_item_new_with_mnemonic("連射なし");
	    
	    
			       JOY_RAPID0[j][i]);
	    
	    
	    
			       JOY_RAPID1[j][i]);
	    
	    
	    
			       JOY_RAPID2[j][i]);
	    
	    
	    
			       JOY_RAPID3[j][i]);
	    
	    
	    
			       JOY_RAPID4[j][i]);
	    
	    
	    
			       JOY_RAPID5[j][i]);
	    
	    
	    
			       JOY_RAPID6[j][i]);
	    
	    
	    
			       JOY_RAPID8[j][i]);
	    
		gtk_menu_item_new_with_mnemonic("12連射");
	    
	    
			       JOY_RAPID12[j][i]);
	    
		gtk_menu_item_new_with_mnemonic("25連射");
	    
	    
			       JOY_RAPID25[j][i]);
	    
				      menu_js_rapid[j][i]);
	
	sprintf(stmp, "#%d", j);
	lblRapid[j] = gtk_label_new(stmp);
	
	
	
    
    lblJoyStick2 = gtk_label_new("連射");
    
    
				
							   (propertySheet),
							   4),
				lblJoyStick2);
    
    
	/*
	 * 解像度 
	 */
	vbox30 = gtk_vbox_new(FALSE, 8);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(SCP_640X400));
    
    
    
    
				RESO_group);
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(SCP_1280X800));
    
    
    
    
    
	
	("ウィンドウモード時に24Hz表示をエミュレート");
    
    
    
    
    
    
    
    
    
    
    
    
    
    
				
							   (propertySheet),
							   5), lblScreen2);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
#ifdef MOUSE
	frmMouse = gtk_frame_new(NULL);
    
    
    
    
    
    
    
	gtk_check_button_new_with_mnemonic("マウスを使用する");
    
    
    
    
	gtk_radio_button_new_with_mnemonic(NULL,
					   "ポート1にマウスを接続");
    
    
    
    
				OP_MOUSE_PORT1_group);
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT1));
    
	gtk_radio_button_new_with_mnemonic(NULL,
					   "ポート2にマウスを接続");
    
    
    
    
				OP_MOUSE_PORT1_group);
    
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT2));
    
    
    
    
    
#endif
	
    
    
    
    
    
    
    
    
    
    
    
#if XM7_VER >= 3
	frmExtram = gtk_frame_new(NULL);
    
    
    
    
    
    
    
    
    
    
#endif
	
    
    
				
							   (propertySheet),
							   6), lblOption);
    
    
    
    
    
    
    
    
    
    
    
    
    
			  GTK_SIGNAL_FUNC(OnGP_CPUDEFAULTClicked), NULL);
    
			 GTK_SIGNAL_FUNC(OnGP_CPUCOMBOChanged), NULL);
    
			 GTK_SIGNAL_FUNC(OnGP_TAPESPEEDClicked), NULL);
    
			 GTK_SIGNAL_FUNC(OnKP_USEARROWFOR10Clicked), NULL);
    
			 GTK_SIGNAL_FUNC(OnConfig_OK), NULL);
    
		       
    
				 GTK_SIGNAL_FUNC(gtk_widget_destroy),
				 
    
				GTK_SIGNAL_FUNC(gtk_widget_destroy),
				
    



#endif	/* _XWIN */
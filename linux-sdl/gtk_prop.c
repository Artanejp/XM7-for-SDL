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
static void
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
    
    /*
     *  プロパティウィンドウの生成 
     */ 
GtkWidget 
*OpenPropertyPage(void) 
{
    int        i,
            j;
    char        stmp[64];
    /*
     * Page 0
     */
    GSList      *GP_FM7_group = NULL;
    GSList      *GP_HIGHSPEED_group = NULL;
    GtkWidget   *menu2;
    GtkWidget   *hbox2;
/*
 * Page 1
 */
    GSList      *SP_96K_group = NULL;
    GtkWidget   *hbox3;
    GtkWidget   *menu3;
/*
 * Page 2
 */
/*
 * Page 3,4
 */
    GSList      *JOY1_group = NULL;
    GSList      *JOY2_group = NULL;


    GtkWidget   *menu_js_rapid[2][4];
    GtkWidget   *hbox23[2];
/*
 * Page 5
 */
    GSList      *RESO_group = NULL;
/*
 * Page 6
 */
#ifdef MOUSE
    GSList      *OP_MOUSE_PORT1_group = NULL;
#endif				/*  */
    
#if XM7_VER >= 3
#endif				/*  */
/*
 * Page 7
 */
   GtkWidget   *w;
/*
 * Cancel,OK 
 */
    GtkWidget *btnCancel;
    GtkWidget *btnOk;
    GtkWidget *btnCallKeyMap;
    GtkWidget *window;

//    winProperty = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    window = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "window_prop"));
    gtk_window_set_title(GTK_WINDOW(window), "XM7の動作設定");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_deletable(GTK_WINDOW(window), FALSE);

//    gtk_widget_show(GTK_WIDGET(winProperty));



/*
 * Page0
 */
    GP_FM7 = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_EMU_FM7"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_FM7), GP_FM7_group);
    GP_FM7_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_FM7));

    GP_FM77AV = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_EMU_77AV"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_FM77AV), GP_FM7_group);
    GP_FM7_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_FM77AV));

#if XM7_VER >= 3
    GP_AV40EX = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_EMU_77AVEX"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_AV40EX), GP_FM7_group);
    GP_FM7_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_AV40EX));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_AV40EX), TRUE);
#else				/*  */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_FM77AV), TRUE);
#endif
    GP_HIGHSPEED =
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_emumode_fast"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_HIGHSPEED),
                               GP_HIGHSPEED_group);
    GP_HIGHSPEED_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_HIGHSPEED));

    GP_LOWSPEED =
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_emumode_slow"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(GP_LOWSPEED),
                               GP_HIGHSPEED_group);
    GP_HIGHSPEED_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(GP_LOWSPEED));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GP_HIGHSPEED), TRUE);

    hbox2 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_cpucycle"));
    GP_CPUCOMBO = gtk_option_menu_new();
//    GP_CPUCOMBO =
//            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "combobox_cputype"));
    gtk_widget_show(GP_CPUCOMBO);
    gtk_box_pack_start(GTK_BOX(hbox2), GP_CPUCOMBO, FALSE, FALSE, 0);
#if 1   
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
    gtk_widget_show(GP_CPUCOMBO);    
#endif

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


/*
 * Page 1
 */

    SP_96K = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_96k"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_96K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_96K));

    SP_88K = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_88k"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_88K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_88K));

    SP_48K = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_48k"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_48K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_48K));

    SP_44K = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_44k"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_44K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_44K));

    SP_22K = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_22k"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_22K), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_22K));


    SP_NONE = 
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_pcm_none"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SP_NONE), SP_96K_group);
    SP_96K_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(SP_NONE));

    SP_HQMODE =
            GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_pcm_hq"));

   
    hbox3 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_stereomode"));
    SP_STEREO = gtk_option_menu_new();
    gtk_widget_show(SP_STEREO);
    gtk_box_pack_start(GTK_BOX(hbox3), SP_STEREO, FALSE, FALSE, 0);
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


/*
 * Page 3
 */

    JOY1_UNUSED = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js1_unused"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_UNUSED), JOY1_group);
    JOY1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_UNUSED));

    JOY1_PORT1 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js1_port1"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_PORT1), JOY1_group);
    JOY1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_PORT1));

    JOY1_PORT2 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js1_port2"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_PORT2), JOY1_group);
    JOY1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_PORT2));

    JOY1_KBD = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js1_joykey"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_KBD), JOY1_group);
    JOY1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_KBD));

    JOY1_DEMPA = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js1_dempa"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY1_DEMPA), JOY1_group);
    JOY1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY1_DEMPA));


    JOY2_UNUSED = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js2_unused"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_UNUSED), JOY2_group);
    JOY2_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_UNUSED));

    JOY2_PORT1 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js2_port1"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_PORT1), JOY2_group);
    JOY2_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_PORT1));

    JOY2_PORT2 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js2_port2"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_PORT2), JOY2_group);
    JOY2_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_PORT2));

    JOY2_KBD = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js2_joykey"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_KBD), JOY2_group);
    JOY2_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_KBD));

    JOY2_DEMPA = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_js2_dempa"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(JOY2_DEMPA), JOY2_group);
    JOY2_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(JOY2_DEMPA));


/*
 * Page 4
 */
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
    }

/*
 * Page 5
 */

    SCP_320X200 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton20"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SCP_320X200), RESO_group);
    RESO_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(SCP_320X200));

    SCP_640X400 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton21"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SCP_640X400), RESO_group);
    RESO_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(SCP_640X400));

    SCP_1280X800 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton22"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(SCP_1280X800), RESO_group);
    RESO_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(SCP_1280X800));

    SCP_24K = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_fullscan"));

/*
 * Page 6
 */
    OP_OPNB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_stdfm"));
    OP_WHGB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_whg"));
    OP_THGB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_thg"));
    OP_MOUSEEM =GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_mouse"));

    OP_MOUSE_PORT1 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_mouse1"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT1),
                               OP_MOUSE_PORT1_group);
    OP_MOUSE_PORT1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT1));

    OP_MOUSE_PORT2 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "radiobutton_mouse2"));
    gtk_radio_button_set_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT2),
                               OP_MOUSE_PORT1_group);
    OP_MOUSE_PORT1_group =
	gtk_radio_button_get_group(GTK_RADIO_BUTTON(OP_MOUSE_PORT2));

    OP_DIGITIZEB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_digitize"));
    OP_RAMB = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "checkbutton_ramboard"));
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

/*
 *    スペーシング用ダミー
 */

/*
 * OK or キャンセル...共通ボタン
 */
    btnCancel = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_CANCEL"));
    btnOk = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_OK"));   

    g_signal_connect((gpointer) GP_CPUDEFAULT, "clicked",
                       GTK_SIGNAL_FUNC(OnGP_CPUDEFAULTClicked), NULL);
    g_signal_connect((gpointer) GP_CPUCOMBO, "changed",
                       GTK_SIGNAL_FUNC(OnGP_CPUCOMBOChanged), NULL);
    g_signal_connect((gpointer) GP_TAPESPEED, "clicked",
                       GTK_SIGNAL_FUNC(OnGP_TAPESPEEDClicked), NULL);
    g_signal_connect((gpointer) KP_USEARROWFOR10, "clicked",
                       GTK_SIGNAL_FUNC(OnKP_USEARROWFOR10Clicked), NULL);
    g_signal_connect((gpointer) btnOk, "clicked",
                       GTK_SIGNAL_FUNC(OnConfig_OK), NULL);

    g_signal_connect((gpointer) btnCallKeyMap, "clicked",
                       GTK_SIGNAL_FUNC(OnClick_KeyMap), NULL);



//    gtk_signal_connect((gpointer) btnKeySet, "clicked",
//			 GTK_SIGNAL_FUNC(StartGetKeycodeForProp), NULL);

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

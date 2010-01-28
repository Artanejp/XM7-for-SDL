/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN プロパティウィンドウ ]
 */

#ifdef _XWIN

#ifndef _xw_prop_h_
#define _xw_prop_h_

/*
 *	グローバル ワーク
 */
extern GtkWidget *winProperty;
extern GtkWidget *GP_FM7;
extern GtkWidget *GP_FM77AV;
#if XM7_VER >= 3
extern GtkWidget *GP_AV40EX;
#endif
extern GtkWidget *GP_HIGHSPEED;
extern GtkWidget *GP_LOWSPEED;
extern GtkWidget *GP_CPUCOMBO;
extern GtkWidget *GP_MAINCPU;
extern GtkWidget *GP_MAINMMR;
#if XM7_VER >= 3
extern GtkWidget *GP_FASTMMR;
#endif
extern GtkWidget *GP_SUBCPU;
extern GtkWidget *GP_CPUTEXT;
extern GtkWidget *GP_CPUDEFAULT;
extern GtkWidget *GP_TAPESPEED;
extern GtkWidget *GP_TAPESPEEDMODE;
#ifdef FDDSND
extern GtkWidget *GP_FDDWAIT;
#endif
extern GtkWidget *GP_FULLSPEED;
extern GtkWidget *GP_AUTOSPEEDADJUST;
extern GtkWidget *SP_96K;
extern GtkWidget *SP_88K;
extern GtkWidget *SP_48K;
extern GtkWidget *SP_44K;
extern GtkWidget *SP_22K;
extern GtkWidget *SP_NONE;
extern GtkWidget *SP_HQMODE;
extern GtkWidget *SP_BUFSPIN;
extern GtkWidget *SP_STEREO;
extern GtkWidget *SP_MONO;
extern GtkWidget *SP_STEREOQ;
extern GtkWidget *SP_STEREOQ_REV;
extern GtkWidget *SP_STEREOQ_THG;
extern GtkWidget *SP_BEEPSPIN;
extern GtkWidget *SP_TAPEMON;
#ifdef FDDSND
extern GtkWidget *SP_FDDSOUND;
#endif
extern GtkWidget *KP_ARROW8DIR;
extern GtkWidget *KP_USEARROWFOR10;
extern GtkWidget *KP_KBDREAL;

extern GtkWidget *JOY1_UNUSED;
extern GtkWidget *JOY1_PORT1;
extern GtkWidget *JOY1_PORT2;
extern GtkWidget *JOY1_KBD;
extern GtkWidget *JOY1_DEMPA;
extern GtkWidget *JOY2_UNUSED;
extern GtkWidget *JOY2_PORT1;
extern GtkWidget *JOY2_PORT2;
extern GtkWidget *JOY2_KBD;
extern GtkWidget *JOY2_DEMPA;
extern GtkWidget *JOY_RAPID[2][4];
extern GtkWidget *JOY_RAPID1[2][4];
extern GtkWidget *JOY_RAPID2[2][4];
extern GtkWidget *JOY_RAPID3[2][4];
extern GtkWidget *JOY_RAPID4[2][4];
extern GtkWidget *JOY_RAPID5[2][4];
extern GtkWidget *JOY_RAPID6[2][4];
extern GtkWidget *JOY_RAPID8[2][4];
extern GtkWidget *JOY_RAPID12[2][4];
extern GtkWidget *JOY_RAPID25[2][4];

extern GtkWidget *SCP_24K;
extern GtkWidget *OP_OPNB;
extern GtkWidget *OP_WHGB;
extern GtkWidget *OP_THGB;
#ifdef MOUSE
extern GtkWidget *OP_MOUSEEM;
extern GtkWidget *OP_MOUSE_PORT1;
extern GtkWidget *OP_MOUSE_PORT2;
#endif
extern GtkWidget *OP_DIGITIZEB;
#if XM7_VER >= 3
extern GtkWidget *OP_RAMB;
#endif
extern int	GP_CPUCOMBO_IDX;

/*
 *  主要エントリ
 */

/*-[ サポート関数 ]-----------------------------------------------------*/

/*
 * チェック、ラジオボタンのチェック
 */
void FASTCALL CheckDlgButton(GtkWidget *widget, BOOL b);

/*
 * オプションメニューのインデックス移動
 */
void FASTCALL SelectOptionMenu(GtkWidget *widget, guint i);


/*
 * オプションメニューのインデックス取得
 */
int FASTCALL GetIdxOptionMenu(GtkWidget *widget);

/*
 * テキストBOXのテキスト設定
 */
void FASTCALL SetTextBox(GtkWidget *widget, char *txt);

/*
 * スピンボタンの数値設定
 */
void FASTCALL SetSpin(GtkWidget *widget, gint v);

/*
 * スピンボタンの数値取得
 */
gint FASTCALL GetSpin(GtkWidget *widget);

/*
 * ウィジェットのアクティブ状態を設定
 */
void FASTCALL SetEnable(GtkWidget *widget, BOOL b);

/*
 * プロパティウィンドウの生成
 */
BOOL FASTCALL IsDlgButtonChecked(GtkWidget *widget);


/*-[ 設定ダイアログ ]-----------------------------------------------------*/

/*
 * 設定ダイアログの生成
 */
GtkWidget* FASTCALL OpenPropertyPage(void);

#endif /* _xw_prop_h_ */
#endif /* _XWIN */

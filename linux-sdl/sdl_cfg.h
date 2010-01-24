/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN コンフィギュレーション ]
 */

#ifdef _XWIN

#ifndef _xw_cfg_h_
#define _xw_cfg_h_

#ifdef __cplusplus
extern "C" {
#endif


#define BST_CHECKED	TRUE
#define BST_UNCHECKED FALSE


/*
 *	主要エントリ
 */
void FASTCALL LoadCfg(void);
										/* 設定ロード */

void FASTCALL SaveCfg(void);
										/* 設定セーブ */

void FASTCALL ApplyCfg(void);
										/* 設定適用 */

void FASTCALL SetMachineVersion(void);
										/* 動作機種再設定 */

void FASTCALL OnConfig(GtkWidget *widget, gpointer data);
										/* 設定ダイアログ */

void FASTCALL OnConfig_OK(GtkWidget *widget, gpointer data);
										/* 設定ダイアログ(OKアクション) */

void FASTCALL OnGP_CPUDEFAULTClicked(GtkWidget *widget, gpointer data);
										/* CPU Defaultボタンアクション */

void FASTCALL OnGP_CPUCOMBOChanged(GtkWidget *widget, gpointer data);
										/* CPU コンボボックスアクション */

/*
 *	主要ワーク
 */
#ifdef __cplusplus
}
#endif

#endif	/* _xw_cfg_h_ */
#endif	/* _XWIN */

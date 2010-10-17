/*
 *  FM-7 EMULATOR "XM7"  
 * Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp) 
 * Copyright (C) 2001-2010 Ryu Takegami 
 * Copyright (C) 2004 GIMONS
 * Copyright (C) 2010 K.Ohta <whatisthis.sowhat@gmail.com>  
 * [ GTKコンフィギュレーション / gtk_cfgdlg.c]
 * 
 */  


#include "xm7.h"

#ifdef _XWIN
#ifdef USE_GTK
#include <gtk/gtk.h>
#endif
#include "device.h"
#include "fdc.h"
#include "tapelp.h"
#include "opn.h"
//#include "whg.h"
//#include "thg.h"
#include "keyboard.h"
#include "mmr.h"
#include "mouse.h"
#include "aluline.h"
#include "sdl.h"
#include "sdl_inifile.h"
#include "sdl_cfg.h"
#include "sdl_prop.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "api_kbd.h"
#include "api_js.h"

#include "sdl_draw.h"

/*
 *  設定データ定義 →sdl_cfg.h に。
 */ 
extern configdat_t configdat;	/* コンフィグ用データ */
GtkWidget * winProperty;	/* プロパティウィンドウ */


static configdat_t propdat;	/* プロパティシート用データ */

static UINT     uPropertyState;	/* プロパティシート進行状況 */
static UINT    uPropertyHelp;	/* ヘルプID */

/*
 *  プロトタイプ宣言 
 */ 
static void SheetInit(void);


/*-[ 全般ページ ]-----------------------------------------------------------*/ 

/*
 *  全般ページ ダイアログ初期化
 */
static void
GeneralPageInit(void) 
{
	//    char           string[128];

	/*
	 * 動作機種 
	 */ 
	switch (propdat.fm7_ver) {
	case 1:
		CheckDlgButton(GP_FM7, BST_CHECKED);
		break;
	case 2:
		CheckDlgButton(GP_FM77AV, BST_CHECKED);
		break;

#if XM7_VER >= 3
	case 3:
		CheckDlgButton(GP_AV40EX, BST_CHECKED);
		break;

#endif				/*  */
	}

	/*
	 * 動作モード 
	 */ 
	if (propdat.cycle_steal) {
		CheckDlgButton(GP_HIGHSPEED, BST_CHECKED);
	}

	else {
		CheckDlgButton(GP_LOWSPEED, BST_CHECKED);
	}

	/*
	 * CPU速度 
	 */ 
	SetSpin(GP_CPUTEXT, propdat.main_speed);
	GP_CPUCOMBO_IDX = 0;
	SelectOptionMenu(GP_CPUCOMBO, GP_CPUCOMBO_IDX);

	/*
	 * テープモータフラグ
	 */
	if (propdat.bTapeFull) {
		CheckDlgButton(GP_TAPESPEED, BST_CHECKED);
	}
	else {
		CheckDlgButton(GP_TAPESPEED, BST_UNCHECKED);
	}

	/*
	 * テープモータモードフラグ
	 */
	if (propdat.bTapeMode) {
		CheckDlgButton(GP_TAPESPEEDMODE, BST_CHECKED);
	}   else {
		CheckDlgButton(GP_TAPESPEEDMODE, BST_UNCHECKED);
	}
	SetEnable(GP_TAPESPEEDMODE, propdat.bTapeFull);

	/*
	 * 全力駆動フラグ
	 */
	if (propdat.bCPUFull) {
		CheckDlgButton(GP_FULLSPEED, BST_CHECKED);
	} else {
		CheckDlgButton(GP_FULLSPEED, BST_UNCHECKED);
	}

	/*
	 * 自動速度調整フラグ
	 */
	if (propdat.bSpeedAdjust) {
		CheckDlgButton(GP_AUTOSPEEDADJUST, BST_CHECKED);
	} else {
		CheckDlgButton(GP_AUTOSPEEDADJUST, BST_UNCHECKED);
	}

	/*
	 * FDDウェイトフラグ 
	 */ 
#ifdef FDDSND
	if (propdat.bFddWait) {
		CheckDlgButton(GP_FDDWAIT, BST_CHECKED);
	}

	else {
		CheckDlgButton(GP_FDDWAIT, BST_UNCHECKED);
	}

#endif				/*  */
}


/*
 *  全般ページ
 * CPUスピードデフォルト設定リカバリ
 */
void
OnGP_CPUDEFAULTClicked(GtkWidget * widget, gpointer data) 
{

#if XM7_VER >= 3
	propdat.main_speed = MAINCYCLES;
	propdat.mmr_speed = MAINCYCLES_MMR;
	propdat.fmmr_speed = MAINCYCLES_FMMR;
	propdat.sub_speed = SUBCYCLES;
	switch (GetIdxOptionMenu(GP_CPUCOMBO)) {
	case 0:
		SetSpin(GP_CPUTEXT, propdat.main_speed);
		break;
	case 1:
		SetSpin(GP_CPUTEXT, propdat.mmr_speed);
		break;
	case 2:
		SetSpin(GP_CPUTEXT, propdat.fmmr_speed);
		break;
	case 3:
		SetSpin(GP_CPUTEXT, propdat.sub_speed);
		break;
	default:
		break;
	}

#else				/*  */
	propdat.main_speed = MAINCYCLES;
	propdat.mmr_speed = MAINCYCLES_MMR;
	propdat.sub_speed = SUBCYCLES;
	switch (GetIdxOptionMenu(GP_CPUCOMBO)) {
	case 0:
		SetSpin(GP_CPUTEXT, propdat.main_speed);
		break;
	case 1:
		SetSpin(GP_CPUTEXT, propdat.mmr_speed);
		break;
	case 2:
		SetSpin(GP_CPUTEXT, propdat.sub_speed);
		break;
	default:
		break;
	}

#endif				/*  */
}


/*
 *  全般ページ CPUスピードコンボボックス制御
 */
void
OnGP_CPUCOMBOChanged(GtkWidget * widget, gpointer data) 
{

#if XM7_VER >= 3
	switch (GP_CPUCOMBO_IDX) {
	case 0:
		propdat.main_speed = GetSpin(GP_CPUTEXT);
		break;
	case 1:
		propdat.mmr_speed = GetSpin(GP_CPUTEXT);
		break;
	case 2:
		propdat.fmmr_speed = GetSpin(GP_CPUTEXT);
		break;
	case 3:
		propdat.sub_speed = GetSpin(GP_CPUTEXT);
		break;
	default:
		break;
	}
	switch (GetIdxOptionMenu(GP_CPUCOMBO)) {
	case 0:
		SetSpin(GP_CPUTEXT, propdat.main_speed);
		break;
	case 1:
		SetSpin(GP_CPUTEXT, propdat.mmr_speed);
		break;
	case 2:
		SetSpin(GP_CPUTEXT, propdat.fmmr_speed);
		break;
	case 3:
		SetSpin(GP_CPUTEXT, propdat.sub_speed);
		break;
	default:
		break;
	}

#else				/*  */
	switch (GP_CPUCOMBO_IDX) {
	case 0:
		propdat.main_speed = GetSpin(GP_CPUTEXT);
		break;
	case 1:
		propdat.mmr_speed = GetSpin(GP_CPUTEXT);
		break;
	case 2:
		propdat.sub_speed = GetSpin(GP_CPUTEXT);
		break;
	default:
		break;
	}
	switch (GetIdxOptionMenu(GP_CPUCOMBO)) {
	case 0:
		SetSpin(GP_CPUTEXT, propdat.main_speed);
		break;
	case 1:
		SetSpin(GP_CPUTEXT, propdat.mmr_speed);
		break;
	case 2:
		SetSpin(GP_CPUTEXT, propdat.sub_speed);
		break;
	default:
		break;
	}

#endif				/*  */
	GP_CPUCOMBO_IDX = GetIdxOptionMenu(GP_CPUCOMBO);
}


/*
 *  全般ページ 適用
 */
static void
GeneralPageApply(void) 
{

	/*
	 * ステート変更 
	 */ 
	uPropertyState = 2;

	/*
	 * FM-7バージョン 
	 */ 
	if (IsDlgButtonChecked(GP_FM7) == BST_CHECKED) {
		propdat.fm7_ver = 1;
	}

#if XM7_VER >= 3
	else if (IsDlgButtonChecked(GP_FM77AV) == BST_CHECKED) {
		propdat.fm7_ver = 2;
	}

	else
	{
		propdat.fm7_ver = 3;
	}

#else				/*  */
	else {
		propdat.fm7_ver = 2;
	}

#endif				/*  */

	/*
	 * CPU速度 
	 */ 
#if XM7_VER >= 3
	switch (GP_CPUCOMBO_IDX) {
	case 0:
		propdat.main_speed = GetSpin(GP_CPUTEXT);
		break;
	case 1:
		propdat.mmr_speed = GetSpin(GP_CPUTEXT);
		break;
	case 2:
		propdat.fmmr_speed = GetSpin(GP_CPUTEXT);
		break;
	case 3:
		propdat.sub_speed = GetSpin(GP_CPUTEXT);
		break;
	default:
		break;
	}

#else				/*  */
	switch (GP_CPUCOMBO_IDX) {
	case 0:
		propdat.main_speed = GetSpin(GP_CPUTEXT);
		break;
	case 1:
		propdat.mmr_speed = GetSpin(GP_CPUTEXT);
		break;
	case 2:
		propdat.sub_speed = GetSpin(GP_CPUTEXT);
		break;
	default:
		break;
	}

#endif				/*  */

	/*
	 * サイクルスチール 
	 */ 
	if (IsDlgButtonChecked(GP_HIGHSPEED) == BST_CHECKED) {
		propdat.cycle_steal = TRUE;
	}

	else {
		propdat.cycle_steal = FALSE;
	}

	/*
	 * テープ高速モード 
	 */ 
	if (IsDlgButtonChecked(GP_TAPESPEED) == BST_CHECKED) {
		propdat.bTapeFull = TRUE;
	}

	else {
		propdat.bTapeFull = FALSE;
	}

	/*
	 * テープ高速モード動作タイプ 
	 */ 
	if (IsDlgButtonChecked(GP_TAPESPEEDMODE) == BST_CHECKED) {
		propdat.bTapeMode = TRUE;
	}

	else {
		propdat.bTapeMode = FALSE;
	}

	/*
	 * 全力駆動 
	 */ 
	if (IsDlgButtonChecked(GP_FULLSPEED) == BST_CHECKED) {
		propdat.bCPUFull = TRUE;
	}

	else {
		propdat.bCPUFull = FALSE;
	}

	/*
	 * 自動速度調整 
	 */ 
	if (IsDlgButtonChecked(GP_AUTOSPEEDADJUST) == BST_CHECKED) {
		propdat.bSpeedAdjust = TRUE;
	}

	else {
		propdat.bSpeedAdjust = FALSE;
	}

	/*
	 * FDDウェイト 
	 */ 
#ifdef FDDSND
	if (IsDlgButtonChecked(GP_FDDWAIT) == BST_CHECKED) {
		propdat.bFddWait = TRUE;
	}

	else {
		propdat.bFddWait = FALSE;
	}

#endif				/*  */
}


/*-[ サウンドページ ]-------------------------------------------------------*/ 

/*
 *  サウンドページ ダイアログ初期化
 */
static void
SoundPageInit(void) 
{
	//    char           string[128];

	/*
	 * シート初期化 
	 */ 
	SheetInit();

	/*
	 * サンプリングレート
	 */
	CheckDlgButton(SP_96K, BST_UNCHECKED);
	CheckDlgButton(SP_88K, BST_UNCHECKED);
	CheckDlgButton(SP_48K, BST_UNCHECKED);
	CheckDlgButton(SP_44K, BST_UNCHECKED);
	CheckDlgButton(SP_22K, BST_UNCHECKED);
	CheckDlgButton(SP_NONE, BST_UNCHECKED);


	switch (propdat.nSampleRate) {
	case 96000:
		CheckDlgButton(SP_96K, BST_CHECKED);
		break;
	case 88200:
		CheckDlgButton(SP_88K, BST_CHECKED);
		break;
	case 48000:
		CheckDlgButton(SP_48K, BST_CHECKED);
		break;
	case 44100:
		CheckDlgButton(SP_44K, BST_CHECKED);
		break;
	case 22050:
		CheckDlgButton(SP_22K, BST_CHECKED);
		break;
	case 0:
		CheckDlgButton(SP_NONE, BST_CHECKED);
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	/*
	 * サウンドバッファ
	 */
	 SetSpin(SP_BUFSPIN, propdat.nSoundBuffer);

	/*
	 * BEEP周波数
	 */
	 SetSpin(SP_BEEPSPIN, propdat.nBeepFreq);

	/*
	 * FM高品質合成モード
	 */
	 if (propdat.bFMHQmode) {
		 CheckDlgButton(SP_HQMODE, BST_CHECKED);
	 }

	 else {
		 CheckDlgButton(SP_HQMODE, BST_UNCHECKED);
	 }

	 /*
	  * 出力モード
	  */
	 SelectOptionMenu(SP_STEREO, propdat.nStereoOut % 4);

	 /*
	  * テープ音モニタ
	  */
	 if (propdat.bTapeMon) {
		 CheckDlgButton(SP_TAPEMON, BST_CHECKED);
	 }
	 else {
		 CheckDlgButton(SP_TAPEMON, BST_UNCHECKED);
	 }

	 /*
	  * FDDシーク音
	  */
#ifdef FDDSND
	 if (propdat.bFddSound) {
		 CheckDlgButton(SP_FDDSOUND, BST_CHECKED);
	 }
	 else {
		 CheckDlgButton(SP_FDDSOUND, BST_UNCHECKED);
	 }

#endif				/*  */
}


/*
 *  サウンドページ 適用
 */
static void
SoundPageApply(void) 
{
	UINT uPos;

	/*
	 * ステート変更
	 */
	uPropertyState = 2;

	/*
	 * サンプリングレート
	 */
	if (IsDlgButtonChecked(SP_96K) == BST_CHECKED) {
		propdat.nSampleRate = 96000;
	}
	if (IsDlgButtonChecked(SP_88K) == BST_CHECKED) {
		propdat.nSampleRate = 88200;
	}
	if (IsDlgButtonChecked(SP_48K) == BST_CHECKED) {
		propdat.nSampleRate = 48000;
	}
	if (IsDlgButtonChecked(SP_44K) == BST_CHECKED) {
		propdat.nSampleRate = 44100;
	}
	if (IsDlgButtonChecked(SP_22K) == BST_CHECKED) {
		propdat.nSampleRate = 22050;
	}
	if (IsDlgButtonChecked(SP_NONE) == BST_CHECKED) {
		propdat.nSampleRate = 0;
	}

	/*
	 * サウンドバッファ 
	 */ 
	 uPos = GetSpin(SP_BUFSPIN);
	propdat.nSoundBuffer = uPos;	/* Bug */

	/*
	 * BEEP周波数 
	 */ 
	propdat.nBeepFreq = GetSpin(SP_BEEPSPIN);

	/*
	 * FM高品質合成モード 
	 */ 
	if (IsDlgButtonChecked(SP_HQMODE) == BST_CHECKED) {
		propdat.bFMHQmode = TRUE;
	}

	else {
		propdat.bFMHQmode = FALSE;
	}

	/*
	 * 出力モード 
	 */ 
	propdat.nStereoOut = GetIdxOptionMenu(SP_STEREO) % 4;

	/*
	 * テープ音モニタ 
	 */ 
	if (IsDlgButtonChecked(SP_TAPEMON) == BST_CHECKED) {
		propdat.bTapeMon = TRUE;
	}

	else {
		propdat.bTapeMon = FALSE;
	}

	/*
	 * FDDシーク音 
	 */ 
#ifdef FDDSND
	if (IsDlgButtonChecked(SP_FDDSOUND) == BST_CHECKED) {
		propdat.bFddSound = TRUE;
	}
	else {
		propdat.bFddSound = FALSE;
	}

#endif				/*  */
}


/*-[ キーボードページ ]-----------------------------------------------------*/ 

/*
 *  キーボードページ ダイアログ初期化
 */
static void
KbdPageInit(void) 
{
	//    char           string[128];

	/*
	 * シート初期化 
	 */ 
	SheetInit();

	/*
	 * テンキーエミュレーション 
	 */ 
	if (propdat.bTenCursor) {
		CheckDlgButton(KP_USEARROWFOR10, BST_CHECKED);
	}

	else {
		CheckDlgButton(KP_USEARROWFOR10, BST_UNCHECKED);
	}

	/*
	 * テンキーエミュレーション8方向モード 
	 */ 
	if (propdat.bArrow8Dir) {
		CheckDlgButton(KP_ARROW8DIR, BST_CHECKED);
	}

	else {
		CheckDlgButton(KP_ARROW8DIR, BST_UNCHECKED);
	}
	SetEnable(KP_ARROW8DIR, propdat.bTenCursor);

	/*
	 * 疑似リアルタイムキースキャン 
	 */ 
	if (propdat.bKbdReal) {
		CheckDlgButton(KP_KBDREAL, BST_CHECKED);
	}

	else {
		CheckDlgButton(KP_KBDREAL, BST_UNCHECKED);
	}
}


/*
 *  キーボードページ 適用
 */
static void
KbdPageApply(void) 
{

	/*
	 * ステート変更 
	 */ 
	uPropertyState = 2;

	/*
	 * テンキーエミュレーション 
	 */ 
	if (IsDlgButtonChecked(KP_USEARROWFOR10) == BST_CHECKED) {
		propdat.bTenCursor = TRUE;
	}

	else {
		propdat.bTenCursor = FALSE;
	}

	/*
	 * テンキーエミュレーション 8方向モード 
	 */ 
	if (IsDlgButtonChecked(KP_ARROW8DIR) == BST_CHECKED) {
		propdat.bArrow8Dir = TRUE;
	}

	else {
		propdat.bArrow8Dir = FALSE;
	}

	/*
	 * 疑似リアルタイムキースキャン 
	 */ 
	if (IsDlgButtonChecked(KP_KBDREAL) == BST_CHECKED) {
		propdat.bKbdReal = TRUE;
	}

	else {
		propdat.bKbdReal = FALSE;
	}
}


/*-[ ジョイスティックページ ]-----------------------------------------------------*/ 

/*
 *  キーボードページ ダイアログ初期化
 */
static void
JsPageInit(void) 
{
	//    char           string[128];

	/*
	 * シート初期化 
	 */ 
	SheetInit();

	/*
	 * JOYSTICK1 
	 */ 
	switch (propdat.nJoyType[0]) {
	case 1:			/* Port1 */
		CheckDlgButton(JOY1_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT1, BST_CHECKED);
		CheckDlgButton(JOY1_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY1_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY1_DEMPA, BST_UNCHECKED);
		break;
	case 2:			/* Port2 */
		CheckDlgButton(JOY1_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT2, BST_CHECKED);
		CheckDlgButton(JOY1_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY1_DEMPA, BST_UNCHECKED);
		break;
	case 3:			/* JOY-KBD */
		CheckDlgButton(JOY1_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY1_KBD, BST_CHECKED);
		CheckDlgButton(JOY1_DEMPA, BST_UNCHECKED);
		break;
	case 4:			/* DEMPA */
		CheckDlgButton(JOY1_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY1_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY1_DEMPA, BST_CHECKED);
		break;
	default:			/* Undefined */
		CheckDlgButton(JOY1_UNUSED, BST_CHECKED);
		CheckDlgButton(JOY1_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY1_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY1_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY1_DEMPA, BST_UNCHECKED);
		propdat.nJoyType[0] = 0;
		break;
	}

	/*
	 * JOYSTICK2 
	 */ 
	switch (propdat.nJoyType[1]) {

	/*
	 * テンキーエミュレーション
	 */
	case 1:			/* Port1 */
		CheckDlgButton(JOY2_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT1, BST_CHECKED);
		CheckDlgButton(JOY2_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY2_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY2_DEMPA, BST_UNCHECKED);
		break;
	case 2:			/* Port2 */
		CheckDlgButton(JOY2_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT2, BST_CHECKED);
		CheckDlgButton(JOY2_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY2_DEMPA, BST_UNCHECKED);
		break;
	case 3:			/* JOY-KBD */
		CheckDlgButton(JOY2_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY2_KBD, BST_CHECKED);
		CheckDlgButton(JOY2_DEMPA, BST_UNCHECKED);
		break;
	case 4:			/* DEMPA */
		CheckDlgButton(JOY2_UNUSED, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY2_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY2_DEMPA, BST_CHECKED);
		break;
	default:			/* Undefined -> JS1に */
		CheckDlgButton(JOY2_UNUSED, BST_CHECKED);
		CheckDlgButton(JOY2_PORT1, BST_UNCHECKED);
		CheckDlgButton(JOY2_PORT2, BST_UNCHECKED);
		CheckDlgButton(JOY2_KBD, BST_UNCHECKED);
		CheckDlgButton(JOY2_DEMPA, BST_UNCHECKED);
		propdat.nJoyType[1] = 0;
		break;
	}

	/*
	 * JOY-KBD のコードアサインに付いては後ほど ^^;
	 * 20100127 
	 */ 
}


/*
 *  キーボードページ 適用
 */
static void
JsPageApply(void) 
{

	/*
	 * ステート変更 
	 */ 
	uPropertyState = 2;

	/*
	 * JS1 
	 */ 
	if (IsDlgButtonChecked(JOY1_UNUSED) == BST_CHECKED) {
		propdat.nJoyType[0] = 0;
	} else if (IsDlgButtonChecked(JOY1_PORT1) == BST_CHECKED) {
		propdat.nJoyType[0] = 1;
	} else if (IsDlgButtonChecked(JOY1_PORT2) == BST_CHECKED) {
		propdat.nJoyType[0] = 2;
	} else if (IsDlgButtonChecked(JOY1_KBD) == BST_CHECKED) {
		propdat.nJoyType[0] = 3;
	} else if (IsDlgButtonChecked(JOY1_DEMPA) == BST_CHECKED) {
		propdat.nJoyType[0] = 4;
	} else {			/* どのラジオボタンにもチェックがなかった
	 * →Unused */
		propdat.nJoyType[0] = 0;
	}

	/*
	 * JS2 
	 */ 
	if (IsDlgButtonChecked(JOY2_UNUSED) == BST_CHECKED) {
		propdat.nJoyType[1] = 0;
	} else if (IsDlgButtonChecked(JOY2_PORT1) == BST_CHECKED) {
		propdat.nJoyType[1] = 1;
	} else if (IsDlgButtonChecked(JOY2_PORT2) == BST_CHECKED) {
		propdat.nJoyType[1] = 2;
	} else if (IsDlgButtonChecked(JOY2_KBD) == BST_CHECKED) {
		propdat.nJoyType[1] = 3;
	} else if (IsDlgButtonChecked(JOY2_DEMPA) == BST_CHECKED) {
		propdat.nJoyType[1] = 4;
	} else {			/* どのラジオボタンにもチェックがなかった
	 * →Unused */
		propdat.nJoyType[1] = 0;
	}
}


/*
 *  連射設定ページ
 */
static void
JsRapidPageInit(void) 
{
	int            i,
	j;
	int            rapid;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			switch (propdat.nJoyRapid[i][j])
			{
			case 1:
				rapid = 1;
				break;
			case 2:
				rapid = 2;
				break;
			case 3:
				rapid = 3;
				break;
			case 4:
				rapid = 4;
				break;
			case 5:
				rapid = 5;
				break;
			case 6:
				rapid = 6;
				break;
			case 7:
				rapid = 7;
				break;
			case 8:
				rapid = 8;
				break;
			case 9:
				rapid = 9;
				break;
			case 0:
			default:
				rapid = 0;
				break;
			}
			SelectOptionMenu(JOY_RAPID[i][j], rapid);
		}
	}
}


/*
 *  連射設定
 */
static void
JsRapidPageApply(void) 
{
	int            i,
	j;
	int            rapid;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			rapid = GetIdxOptionMenu(JOY_RAPID[i][j]);
			switch (rapid)
			{
			case 1:
				propdat.nJoyRapid[i][j] = 1;
				break;
			case 2:
				propdat.nJoyRapid[i][j] = 2;
				break;
			case 3:
				propdat.nJoyRapid[i][j] = 3;
				break;
			case 4:
				propdat.nJoyRapid[i][j] = 4;
				break;
			case 5:
				propdat.nJoyRapid[i][j] = 5;
				break;
			case 6:
				propdat.nJoyRapid[i][j] = 6;
				break;
			case 7:
				propdat.nJoyRapid[i][j] = 7;
				break;
			case 8:
				propdat.nJoyRapid[i][j] = 8;
				break;
			case 9:
				propdat.nJoyRapid[i][j] = 9;
				break;
			case 0:
			default:
				propdat.nJoyRapid[i][j] = 0;
				break;
			}
		}
	}
}


/*-[ スクリーンページ ]-----------------------------------------------------*/ 

/*
 *  スクリーンページ ダイアログ初期化
 */
static void
ScrPageInit(void) 
{

	/*
	 * シート初期化 
	 */ 
	SheetInit();

	/*
	 * ウィンドウモード時フルスキャン(24k) 
	 */ 
	if (propdat.bFullScan) {
		CheckDlgButton(SCP_24K, BST_CHECKED);
	}

	else {
		CheckDlgButton(SCP_24K, BST_UNCHECKED);
	}
	switch (propdat.uHeight) {
	case 400:
		CheckDlgButton(SCP_640X400, BST_CHECKED);
		CheckDlgButton(SCP_1280X800, BST_UNCHECKED);
		break;
	case 800:
		CheckDlgButton(SCP_640X400, BST_UNCHECKED);
		CheckDlgButton(SCP_1280X800, BST_CHECKED);
		break;
	default:			/* どれにも当てはまらなければ等倍
	 */
		CheckDlgButton(SCP_640X400, BST_CHECKED);
		CheckDlgButton(SCP_1280X800, BST_UNCHECKED);
		break;
	}
	 SetSpin(SCP_FPS, propdat.nDrawFPS);
}


/*
 *  スクリーンページ 適用
 */
static void
ScrPageApply(void) 
{

	/*
	 * ステート変更 
	 */ 
	uPropertyState = 2;

	/*
	 * ウィンドウモード時フルスキャン(24k) 
	 */ 
	if (IsDlgButtonChecked(SCP_640X400) == BST_CHECKED) {
		propdat.uWidth = 640;
		propdat.uHeight = 400;
	} else if (IsDlgButtonChecked(SCP_1280X800) == BST_CHECKED) {
		propdat.uWidth = 1280;
		propdat.uHeight = 800;
	} else {
		propdat.uWidth = 640;
		propdat.uHeight = 400;
	}

	/*
	 * ウィンドウモード時フルスキャン(24k) 
	 */ 
	if (IsDlgButtonChecked(SCP_24K) == BST_CHECKED) {
		propdat.bFullScan = TRUE;
	}

	else {
		propdat.bFullScan = FALSE;
	}
	 propdat.nDrawFPS = (WORD)GetSpin(SCP_FPS);

}


/*-[ オプションページ ]-----------------------------------------------------*/ 

/*
 *  オプションページ ダイアログ初期化
 */
static void
OptPageInit(void) 
{

	/*
	 * シート初期化 
	 */ 
	SheetInit();

	/*
	 * OPN 
	 */ 
	if (propdat.bOPNEnable) {
		CheckDlgButton(OP_OPNB, BST_CHECKED);
	}

	else {
		CheckDlgButton(OP_OPNB, BST_UNCHECKED);
	}

	/*
	 * WHG 
	 */ 
	if (propdat.bWHGEnable) {
		CheckDlgButton(OP_WHGB, BST_CHECKED);
	}

	else {
		CheckDlgButton(OP_WHGB, BST_UNCHECKED);
	}

	/*
	 * THG 
	 */ 
	if (propdat.bTHGEnable) {
		CheckDlgButton(OP_THGB, BST_CHECKED);
	}

	else {
		CheckDlgButton(OP_THGB, BST_UNCHECKED);
	}

	/*
	 * ビデオディジタイズ 
	 */ 
	if (propdat.bDigitizeEnable) {
		CheckDlgButton(OP_DIGITIZEB, BST_CHECKED);
	}

	else {
		CheckDlgButton(OP_DIGITIZEB, BST_UNCHECKED);
	}

#if ((XM7_VER >= 3) || defined(FMTV151))
	/*
	 * 拡張RAM/FMTV-151 
	 */ 
	if (propdat.bExtRAMEnable) {
		CheckDlgButton(OP_RAMB, BST_CHECKED);
	}

	else {
		CheckDlgButton(OP_RAMB, BST_UNCHECKED);
	}

#endif				/*  */

#ifdef MOUSE
	/*
	 * マウスエミュレーション 
	 */ 
	if (propdat.bMouseCapture) {
		CheckDlgButton(OP_MOUSEEM, BST_CHECKED);
	}

	else {
		CheckDlgButton(OP_MOUSEEM, BST_UNCHECKED);
	}

	/*
	 * マウス接続ポート 
	 */ 
	if (propdat.nMousePort == 1) {
		CheckDlgButton(OP_MOUSE_PORT1, BST_CHECKED);
	}

	else {
		CheckDlgButton(OP_MOUSE_PORT2, BST_CHECKED);
	}

#endif				/*  */
}


/*
 *  オプションページ 適用
 */
static void
OptPageApply(void) 
{

	/*
	 * ステート変更 
	 */ 
	uPropertyState = 2;

	/*
	 * OPN 
	 */ 
	if (IsDlgButtonChecked(OP_OPNB) == BST_CHECKED) {
		propdat.bOPNEnable = TRUE;
	}

	else {
		propdat.bOPNEnable = FALSE;
	}

	/*
	 * WHG 
	 */ 
	if (IsDlgButtonChecked(OP_WHGB) == BST_CHECKED) {
		propdat.bWHGEnable = TRUE;
	}

	else {
		propdat.bWHGEnable = FALSE;
	}

	/*
	 * THG 
	 */ 
	if (IsDlgButtonChecked(OP_THGB) == BST_CHECKED) {
		propdat.bTHGEnable = TRUE;
	}

	else {
		propdat.bTHGEnable = FALSE;
	}

	/*
	 * ビデオディジタイズ 
	 */ 
	if (IsDlgButtonChecked(OP_DIGITIZEB) == BST_CHECKED) {
		propdat.bDigitizeEnable = TRUE;
	}

	else {
		propdat.bDigitizeEnable = FALSE;
	}

#if ((XM7_VER >= 3) || defined(FMTV151))
	/*
	 * 拡張RAM/FMTV-151 
	 */ 
	if (IsDlgButtonChecked(OP_RAMB) == BST_CHECKED) {
		propdat.bExtRAMEnable = TRUE;
	}

	else {
		propdat.bExtRAMEnable = FALSE;
	}

#endif				/*  */

#ifdef MOUSE
	/*
	 * マウスエミュレーション 
	 */ 
	if (IsDlgButtonChecked(OP_MOUSEEM) == BST_CHECKED) {
		propdat.bMouseCapture = TRUE;
	}

	else {
		propdat.bMouseCapture = FALSE;
	}

	/*
	 * マウス接続ポート 
	 */ 
	if (IsDlgButtonChecked(OP_MOUSE_PORT1) == BST_CHECKED) {
		propdat.nMousePort = 1;
	}

	else {
		propdat.nMousePort = 2;
	}

#endif				/*  */
}


/*-[ オプションページ ]-----------------------------------------------------*/ 

/*
 *  音量ページ ダイアログ初期化
 */
static void
VolumePageInit(void) 
{

	/*
	 * シート初期化
	 */
	SheetInit();

	/*
	 * Scale Barに実値を設定する
	 */

	gtk_range_set_value(GTK_RANGE(VOLUME_FM), propdat.nFMVolume);
	gtk_range_set_value(GTK_RANGE(VOLUME_PSG), propdat.nPSGVolume);
	gtk_range_set_value(GTK_RANGE(VOLUME_BEEP), propdat.nBeepVolume);
	gtk_range_set_value(GTK_RANGE(VOLUME_TAPE), propdat.nCMTVolume);
#ifdef FDDSND
	gtk_range_set_value(GTK_RANGE(VOLUME_WAV), propdat.nWaveVolume);
#endif
	gtk_range_set_value(GTK_RANGE(VOLUME_SEP), propdat.uChSeparation);
}


/*
 *  音量ページ 適用 
 */ 
static void
VolumePageApply(void) 
{

	/*
	 * ステート変更
	 */
	uPropertyState = 2;
	propdat.nFMVolume = (int)gtk_range_get_value(GTK_RANGE(VOLUME_FM));
	propdat.nPSGVolume = (int)gtk_range_get_value(GTK_RANGE(VOLUME_PSG));
	propdat.nBeepVolume = (int)gtk_range_get_value(GTK_RANGE(VOLUME_BEEP));
	propdat.nCMTVolume = (int)gtk_range_get_value(GTK_RANGE(VOLUME_TAPE));
#ifdef FDDSND
	propdat.nWaveVolume = (int)gtk_range_get_value(GTK_RANGE(VOLUME_WAV));
#endif
	propdat.uChSeparation = (UINT)gtk_range_get_value(GTK_RANGE(VOLUME_SEP));
	//        propdat.uChSeparation = 9;

}

/*-[ プロパティシート ]-----------------------------------------------------*/ 

/*
 *  プロパティシート 初期化
 */
static void
SheetInit(void) 
{

	/*
	 * 初期化フラグをチェック、シート初期化済みに設定
	 */
	if (uPropertyState > 0) {
		return;
	}
	uPropertyState = 1;
}


/*
 *  設定(C)
 */
void
OnConfig(GtkWidget * widget, gpointer data) 
{
	int i,j;
	/*
	 * データ転送
	 */
	propdat = configdat;

	/*
	 * プロパティシート実行
	 */
	uPropertyState = 0;
	uPropertyHelp = 0;
	/*
	 * WidgetをKILLしていく
	 */
	gtk_widget_destroy(GP_CPUCOMBO);
	gtk_widget_destroy(GP_MAINCPU);
	gtk_widget_destroy(GP_MAINMMR);
#if XM7_VER >= 3
	gtk_widget_destroy(GP_FASTMMR);
#endif
	gtk_widget_destroy(GP_SUBCPU);

	gtk_widget_destroy(SP_STEREO);
	gtk_widget_destroy(SP_MONO);
	gtk_widget_destroy(SP_STEREOQ);
	gtk_widget_destroy(SP_STEREOQ_REV);
	gtk_widget_destroy(SP_STEREOQ_THG);

	for (j = 0; j < 2 ; j++) {
		for(i = 0; i < 2 ; i++) {
			gtk_widget_destroy(JOY_RAPID[j][i]);
			gtk_widget_destroy(JOY_RAPID1[j][i]);
			gtk_widget_destroy(JOY_RAPID2[j][i]);
			gtk_widget_destroy(JOY_RAPID3[j][i]);
			gtk_widget_destroy(JOY_RAPID4[j][i]);
			gtk_widget_destroy(JOY_RAPID5[j][i]);
			gtk_widget_destroy(JOY_RAPID6[j][i]);
			gtk_widget_destroy(JOY_RAPID8[j][i]);
			gtk_widget_destroy(JOY_RAPID12[j][i]);
			gtk_widget_destroy(JOY_RAPID25[j][i]);
		}
	}


	winProperty = OpenPropertyPage();
	gtk_widget_show(winProperty);
	GeneralPageInit();
	SoundPageInit();
	KbdPageInit();
	JsPageInit();
	JsRapidPageInit();
	ScrPageInit();
	OptPageInit();
	VolumePageInit();
} 

/*
 * キャンセルボタン…arg0 に当該Widget, arg1に主ウインドウが来る
 */
void
OnCancelPressed(GtkWidget * widget, gpointer data)
{
	/*
	 * この関数はarg2のWidgetをhideする
	 */
	gtk_widget_hide_on_delete((GtkWidget *) data);

}

/*
 *  設定(C) 
 */ 
void
OnConfig_OK(GtkWidget * widget, gpointer data) 
{
	int            ver;
	char           icon_path[MAXPATHLEN];
	GeneralPageApply();
	SoundPageApply();
	KbdPageApply();
	JsPageApply();
	JsRapidPageApply();
	ScrPageApply();
	OptPageApply();
	VolumePageApply();


	/*
	 * okなので、データ転送
	 */
	 configdat = propdat;

	/*
	 * 適用
	 */
	 LockVM();
	 ver = fm7_ver;
	 ApplyCfg();

	 /*
	  * 動作機種変更を伴う場合、リセットする
	  */
	 if (ver != fm7_ver) {
		 system_reset();
	 }
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
	 if (icon_path[0] != '\0' && strcmp(icon_path, RSSDIR) != 0)
		 gtk_window_set_icon_from_file(GTK_WINDOW(wndMain), icon_path,
				 (GError **) NULL);
#else
	if (icon_path[0] != '\0' && strcmp(icon_path, ModuleDir) != 0)
		gtk_window_set_icon_from_file(GTK_WINDOW(wndMain), icon_path,
				(GError **) NULL);
#endif
UnlockVM();
}

/*
 * サウンド：音源の個別音量設定は即時反映される
 */
void
OnGP_ChSepVolumeChanged(GtkWidget* widget, gpointer data) 
{
	UINT    uSp = uChSeparation;
	int     nFM = nFMVolume;
	int     nPSG = nPSGVolume;
	int     nBeep = nBeepVolume;
	int     nCMT = nCMTVolume;
	int     nWav = nWaveVolume;


	uSp = (UINT)gtk_range_get_value(GTK_RANGE(widget));
	SetSoundVolume2(uSp, nFM, nPSG,
			nBeep, nCMT, nWav);
}


/*
 * サウンド：音源の個別音量設定は即時反映される
 */
void
OnGP_FMVolumeChanged(GtkWidget* widget, gpointer data) 
{
	UINT    uSp = uChSeparation;
	int     nFM = nFMVolume;
	int     nPSG = nPSGVolume;
	int     nBeep = nBeepVolume;
	int     nCMT = nCMTVolume;
	int     nWav = nWaveVolume;


	nFM = gtk_range_get_value(GTK_RANGE(widget));
	SetSoundVolume2(uSp, nFM, nPSG,
			nBeep, nCMT, nWav);
}

void
OnGP_PSGVolumeChanged(GtkWidget* widget, gpointer data) 
{
	UINT    uSp = uChSeparation;
	int     nFM = nFMVolume;
	int     nPSG = nPSGVolume;
	int     nBeep = nBeepVolume;
	int     nCMT = nCMTVolume;
	int     nWav = nWaveVolume;


	nPSG = gtk_range_get_value(GTK_RANGE(widget));
	SetSoundVolume2(uSp, nFM, nPSG,
			nBeep, nCMT, nWav);
}

void
OnGP_BEEPVolumeChanged(GtkWidget* widget, gpointer data) 
{
	UINT    uSp = uChSeparation;
	int     nFM = nFMVolume;
	int     nPSG = nPSGVolume;
	int     nBeep = nBeepVolume;
	int     nCMT = nCMTVolume;
	int     nWav = nWaveVolume;


	nBeep = gtk_range_get_value(GTK_RANGE(widget));
	SetSoundVolume2(uSp, nFM, nPSG,
			nBeep, nCMT, nWav);
}

void
OnGP_CMTVolumeChanged(GtkWidget* widget, gpointer data) 
{
	UINT    uSp = uChSeparation;
	int     nFM = nFMVolume;
	int     nPSG = nPSGVolume;
	int     nBeep = nBeepVolume;
	int     nCMT = nCMTVolume;
	int     nWav = nWaveVolume;


	nCMT = gtk_range_get_value(GTK_RANGE(widget));
	SetSoundVolume2(uSp, nFM, nPSG,
			nBeep, nCMT, nWav);
}

void
OnGP_WAVVolumeChanged(GtkWidget* widget, gpointer data) 
{
	UINT    uSp = uChSeparation;
	int     nFM = nFMVolume;
	int     nPSG = nPSGVolume;
	int     nBeep = nBeepVolume;
	int     nCMT = nCMTVolume;
	int     nWav = nWaveVolume;


	nWav = gtk_range_get_value(GTK_RANGE(widget));
	SetSoundVolume2(uSp, nFM, nPSG,
			nBeep, nCMT, nWav);
}

#endif /* _XWIN */

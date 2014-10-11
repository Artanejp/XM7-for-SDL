/*
 * agar_cfg.h
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */

#ifndef AGAR_CFG_H_
#define AGAR_CFG_H_


#include <SDL/SDL.h>
#include "KbdInterface.h"
#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "xm7_sdl.h"
#endif
#include "sdl_inifile.h"
//#include "agar_cfg.h"

#include "sdl_prop.h"
#include "sdl_sch.h"

#include "api_snd.h"
#include "api_kbd.h"
#include "api_js.h"
#include "api_mouse.h"
#include "api_draw.h"

enum 
{
   RENDERING_FULL = 0,
   RENDERING_BLOCK,
   RENDERING_RASTER,
   RENDERING_END
};


#ifdef __cplusplus
extern          "C" {
#endif				/*  */
/*
 * ここで、configdat_t型を指定する
 * …ToolKitを外に追い出すための布石。
 */
typedef struct {
        /*
         * VM
         */
	int  fm7_ver;	/* ハードウェアバージョン */
	BOOL cycle_steal;	/* サイクルスチールフラグ */
        int lowspeed_mode;            /* 低速モード(FM-7エミュレーション向け) */
	DWORD main_speed;	/* メインCPUスピード */
	DWORD main_speed_low;	/* メインCPUスピード */
	DWORD mmr_speed;	/* メインCPU(MMR)スピード */
#if XM7_VER >= 3
	DWORD fmmr_speed;	/* メインCPU(高速MMR)スピード */
#endif				/*  */
	DWORD sub_speed;	/* サブCPUスピード */
	DWORD sub_speed_low;	/* サブCPUスピード(低速) */
	DWORD uTimerResolution;	/* マルチメディアタイマー精度 */
	BOOL bTapeFull;	/* テープモータ時の速度フラグ */
	BOOL bCPUFull;	/* 全力駆動フラグ */
	BOOL bSpeedAdjust;	/* 自動速度調整 */
	BOOL bTapeMode;	/* テープモータ速度制御タイプ */
#ifdef FDDSND
	BOOL bFddWait;	/* FDDウェイト */
#endif
        /*
	 * Sound
	 */
	int nSampleRate;	/* サンプリングレート */
	int nSoundBuffer;	/* サウンドバッファサイズ */
	int nBeepFreq;	/* BEEP周波数 */
	BOOL bFMHQmode;	/* FM高品質合成 */
	int nStereoOut;	/* 出力モード */
	BOOL bTapeMon;	/* テープ音モニタ */
#ifdef FDDSND
	BOOL bFddSound;	/* FDDシークサウンド */
#endif
	BOOL bOPNEnable;	/* OPN有効フラグ(7 only) */
	BOOL bWHGEnable;	/* WHG有効フラグ */
	BOOL bTHGEnable;	/* THG有効フラグ */

	int iTotalVolume;
	int nFMVolume;  /* FM音源ボリューム */
	int nPSGVolume; /* PSGボリューム */
	int nBeepVolume;/* BEEP音ボリューム */
	int nCMTVolume; /* CMT音モニタボリューム */
	int nWaveVolume; /* 各種効果音ボリューム */
	UINT uChSeparation;
   
        /*
         * Keyboard
         */
	XM7KeyCode KeyMap[256]; /* キーマップ */
	BOOL bKbdReal;	/* 擬似リアルタイムキースキャン */
	BOOL bTenCursor;	/* 方向キーをテンキーに対応 */
	BOOL bArrow8Dir;	/* テンキー変換	 * 8方向モード */
	/*
	 * JOYSTICK
	 */
	int nJoyType[2];
	int nJoyRapid[2][2];
	int nJoyCode[2][7];
        /*
	 * Mouse
	 */
#ifdef MOUSE
	BOOL bMouseCapture;	/* マウスキャプチャフラグ  */
	DWORD nMousePort;	/* マウス接続ポート */
	BYTE nMidBtnMode;	/* 中央ボタン状態取得モード */
#endif				/*  */
        /*
         * Display
	 */
	BOOL bFullScan;	/* 400ラインタイミングモード */
	BOOL bFullScanFS;
	WORD uWidth;	/* 表示サイズ(横) */
	WORD uHeight;	/* 表示サイズ(縦) */
	WORD nDrawFPS; /* 描画FPS */
	WORD nEmuFPS;    /* エミュレーションFPS */
        WORD nBrightness; /* 明度(0-255) */
        unsigned int nRenderMethod;
        /*
         * Misc
	 */
	BOOL bUseOpenCL; /* OpenCLレンダラ(GLのみ?) */
	BOOL bUseSIMD;   /* 描画等にSIMDを使う */
	BOOL bSmoosing; /* スムージング処理する(GLのみ?) */

	BOOL bDigitizeEnable;	/* ディジタイズ有効フラグ */
#if ((XM7_VER >= 3) || defined(FMTV151))
	BOOL bExtRAMEnable;	/* 拡張RAM有効フラグ */
#endif				/*  */
        /* OpenCL */
        int nCLGlobalWorkThreads;
        BOOL bCLSparse; // TRUE=Multi threaded CL,FALSE = Single Thread.
        /* Scheduler */
        BOOL  bHiresTick;               /* Hi resolution 1ms timer */
        DWORD nTickResUs;               /* Wait value for Hi-Resolution tick */
} configdat_t;

struct gui_vm {
	int  fm7_ver;	/* ハードウェアバージョン */
	BOOL cycle_steal;	/* サイクルスチールフラグ
	 */
        int lowspeed_mode;            /* 低速モード(FM-7エミュレーション向け) */
	DWORD main_speed;	/* メインCPUスピード */
	DWORD main_speed_low;	/* メインCPUスピード */
	DWORD mmr_speed;	/* メインCPU(MMR)スピード */
#if XM7_VER >= 3
	DWORD fmmr_speed;	/* メインCPU(高速MMR)スピード	 */
#endif				/*  */
	DWORD sub_speed;	/* サブCPUスピード */
	DWORD sub_speed_low;	/* サブCPUスピード(低速) */
	DWORD uTimerResolution;	/* マルチメディアタイマー精度	 */
	BOOL bTapeFull;	/* テープモータ時の速度フラグ */
	BOOL bCPUFull;	/* 全力駆動フラグ */
	BOOL bSpeedAdjust;	/* 自動速度調整 */
	BOOL bTapeMode;	/* テープモータ速度制御タイプ */
#ifdef FDDSND
	BOOL bFddWait;	/* FDDウェイト */
#endif
	BOOL bDigitizeEnable;	/* ディジタイズ有効フラグ */
#if ((XM7_VER >= 3) || defined(FMTV151))
	BOOL bExtRAMEnable;	/* 拡張RAM有効フラグ */
#endif				/*  */
        /* Scheduler */
        BOOL  bHiresTick;               /* Hi resolution 1ms timer */
        DWORD nTickResUs;               /* Wait value for Hi-Resolution tick */
};

struct gui_disp {
	WORD nDrawFPS; /* 描画FPS */
	WORD nEmuFPS;    /* エミュレーションFPS */
        WORD nBrightness; /* 明度(0-255) */
        unsigned int nRenderMethod;
	WORD uWidth;	/* 表示サイズ(横) */
	WORD uHeight;	/* 表示サイズ(縦) */
	BOOL bFullScan;	/* 400ラインタイミングモード  */
	BOOL bFullScanFS;
	BOOL bSmoosing; /* スムージング処理する(GLのみ?) */
        BOOL bUseSIMD;   /* 描画等にSIMDを使う */
#ifdef _USE_OPENCL
	BOOL bUseOpenCL; /* OpenCLレンダラ(GLのみ?) */
        /* OpenCL */
        int  nCLGlobalWorkThreads;
        BOOL bCLSparse; // TRUE=Multi threaded CL,FALSE = Single Thread.
#endif
};

struct gui_sound {
	int iTotalVolume;
	int nFMVolume;  /* FM音源ボリューム */
	int nPSGVolume; /* PSGボリューム */
	int nBeepVolume;/* BEEP音ボリューム */
	int nCMTVolume; /* CMT音モニタボリューム */
	int nWaveVolume; /* 各種効果音ボリューム */
	UINT uChSeparation;

	int nSampleRate;	/* サンプリングレート */
	int nSoundBuffer;	/* サウンドバッファサイズ
	 */
	int nBeepFreq;	/* BEEP周波数 */
	BOOL bFMHQmode;	/* FM高品質合成 */
	int nStereoOut;	/* 出力モード */
	BOOL bTapeMon;	/* テープ音モニタ */

	BOOL bOPNEnable;	/* OPN有効フラグ(7 only) */
	BOOL bWHGEnable;	/* WHG有効フラグ */
	BOOL bTHGEnable;	/* THG有効フラグ */
#ifdef FDDSND
	BOOL bFddSound;	/* FDDシークサウンド */
#endif
};

struct gui_input {
  	XM7KeyCode KeyMap[256]; /* キーマップ */
	BOOL bKbdReal;	/* 擬似リアルタイムキースキャン */
	BOOL bTenCursor;	/* 方向キーをテンキーに対応
	 */
	BOOL bArrow8Dir;	/* テンキー変換
	 * 8方向モード */

	/*
	 * JOYSTICK
	 */
	int nJoyType[2];
	int nJoyRapid[2][2];
	int nJoyCode[2][7];
#ifdef MOUSE
	BOOL bMouseCapture;	/* マウスキャプチャフラグ  */
	DWORD nMousePort;	/* マウス接続ポート */
	BYTE nMidBtnMode;	/* 中央ボタン状態取得モード */
#endif				/*  */
};



extern configdat_t configdat;	/* コンフィグ用データ */

#define BST_CHECKED     TRUE
#define BST_UNCHECKED   FALSE

/*
 *  主要エントリ
 */
void    LoadCfg(void);
void    SaveCfg(void);
void    ApplyCfg(void);
void    SetMachineVersion(void);
#ifdef USE_GTK
/*
 * 動作機種再設定
 */
void    OnConfig(GtkWidget *widget, gpointer data);
void    OnCancelPressed(GtkWidget * widget, gpointer data);



/*
 * 設定ダイアログ
 */
void    OnConfig_OK(GtkWidget *widget, gpointer data);

/*
 * 設定ダイアログ(OKアクション)
 */
void    OnGP_CPUDEFAULTClicked(GtkWidget *widget, gpointer data);

/*
 * CPU Defaultボタンアクション
 */

void    OnGP_CPUCOMBOChanged(GtkWidget *widget, gpointer data);

/*
 * CPU コンボボックスアクション
 */
void    OnGP_ChSepVolumeChanged(GtkWidget *widget, gpointer data);
void    OnGP_FMVolumeChanged(GtkWidget *widget, gpointer data);
void    OnGP_PSGVolumeChanged(GtkWidget *widget, gpointer data);
void    OnGP_BEEPVolumeChanged(GtkWidget *widget, gpointer data);
void    OnGP_CMTVolumeChanged(GtkWidget *widget, gpointer data);
void    OnGP_WAVVolumeChanged(GtkWidget *widget, gpointer data);
#endif
#ifdef __cplusplus
}
#endif  /*  */



#endif /* AGAR_CFG_H_ */

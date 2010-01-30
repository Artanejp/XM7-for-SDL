/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN
 * コンフィギュレーション ] 
 */  
    
#ifdef _XWIN

#ifdef USE_GTK
#include <gtk/gtk.h>
#endif
#include "xm7.h"
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
#include "sdl_kbd.h"
#include "sdl_draw.h"
    
    /*
     *  設定データ定義 
     */ 
    typedef struct {
    int            fm7_ver;	/* ハードウェアバージョン */
                   BOOL cycle_steal;	/* サイクルスチールフラグ 
					 */
                   DWORD main_speed;	/* メインCPUスピード */
                   DWORD mmr_speed;	/* メインCPU(MMR)スピード */
                   
#if XM7_VER >= 3
                    DWORD fmmr_speed;	/* メインCPU(高速MMR)スピード 
					 */
                   
#endif				/*  */
                    DWORD sub_speed;	/* サブCPUスピード */
                   DWORD uTimerResolution;	/* マルチメディアタイマー精度 
						 */
                   BOOL bTapeFull;	/* テープモータ時の速度フラグ 
					 */
                   BOOL bCPUFull;	/* 全力駆動フラグ */
                   BOOL bSpeedAdjust;	/* 自動速度調整 */
                   BOOL bTapeMode;	/* テープモータ速度制御タイプ 
					 */
                  int nSampleRate;	/* サンプリングレート */
                   int nSoundBuffer;	/* サウンドバッファサイズ 
					 */
                   int nBeepFreq;	/* BEEP周波数 */
                   BOOL bFMHQmode;	/* FM高品質合成 */
                   int nStereoOut;	/* 出力モード */
                   BOOL bForceStereo;	/* 強制ステレオ出力 */
                   BOOL bTapeMon;	/* テープ音モニタ */
                  
	// BYTE KeyMap[256]; /* キーマップ */
                    BOOL bKbdReal;	/* 擬似リアルタイムキースキャン 
					 */
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
                  BOOL bFullScan;	/* 400ラインタイミングモード 
					 */
                   BOOL bFullScanFS;
                  WORD uWidth;	/* 表示サイズ(横) */
                   WORD uHeight;	/* 表示サイズ(縦) */
                  BOOL bOPNEnable;	/* OPN有効フラグ(7 only) */
                   BOOL bWHGEnable;	/* WHG有効フラグ */
                   BOOL bTHGEnable;	/* THG有効フラグ */
                   BOOL bDigitizeEnable;	/* ディジタイズ有効フラグ 
						 */
                   
#if ((XM7_VER >= 3) || defined(FMTV151))
                    BOOL bExtRAMEnable;	/* 拡張RAM有効フラグ */
                   
#endif				/*  */
#ifdef MOUSE
                    BOOL bMouseCapture;	/* マウスキャプチャフラグ 
					 */
                   BYTE nMousePort;	/* マウス接続ポート */
                   BYTE nMidBtnMode;	/* 中央ボタン状態取得モード 
					 */
                   
#endif				/*  */
                   
#ifdef FDDSND
                   BOOL bFddWait;	/* FDDウェイト */
                   BOOL bFddSound;	/* FDDシークサウンド */
         
#endif
                   int nFMVolume;                                          /* FM音源ボリューム */
                   int nPSGVolume;                                         /* PSGボリューム */
                   int nBeepVolume;                                        /* BEEP音ボリューム */
                   int nCMTVolume;                                         /* CMT音モニタボリューム */
                   int nWaveVolume;                                        /* 各種効果音ボリューム */
            UINT uChSeparation;
} configdat_t;

    /*
     *  スタティック ワーク 
     */ 
static UINT     uPropertyState;	/* プロパティシート進行状況 */
static UINT    uPropertyHelp;	/* ヘルプID */
GtkWidget * winProperty;	/* プロパティウィンドウ */
static configdat_t configdat;	/* コンフィグ用データ */
static configdat_t propdat;	/* プロパティシート用データ */
static char   *pszSection;	/* セクション名 */

    /*
     *  プロトタイプ宣言 
     */ 
static void FASTCALL SheetInit(void);

    /*
     *  パス保存用キー名 
     */ 
static const char *InitDirStr[] =
    { "DiskImageDir", "TapeImageDir", "StateFileDir", "BMPFileDir",
    "WAVFileDir" 
};


/*-[ 設定データ ]-----------------------------------------------------------*/ 
    
    /*
     *  設定データ ファイル名指定 
     */ 
static void     FASTCALL
SetCfgFile(void) 
{
    INI_init("XM7.INI");
} 
    /*
     *  設定データ データロード 
     */ 
static void     FASTCALL
LoadCfgFile(void) 
{
    INI_load();
} 
    /*
     *  設定データ データセーブ 
     */ 
static void     FASTCALL
SaveCfgFile(void) 
{
    INI_save();
} 
    /*
     *  設定データ セクション名指定 
     */ 
static void     FASTCALL
SetCfgSection(char *section) 
{
    ASSERT(section);
    
	/*
	 * セクション名設定 
	 */ 
	pszSection = section;
} 
    /*
     *  設定データ ロード(文字列) 
     */ 
static          BOOL
LoadCfgString(char *key, char *buf, int length) 
{
    char          *dat;
    ASSERT(key);
    dat = INI_getString(pszSection, key, "");
    if (strlen(dat) == 0 || strlen(dat) > length) {
	return FALSE;
    }
    strcpy(buf, dat);
    return TRUE;
}


    /*
     *  設定データ ロード(int) 
     */ 
    static int
LoadCfgInt(char *key, int def) 
{
    ASSERT(key);
    return INI_getInt(pszSection, key, def);
}


    /*
     *  設定データ ロード(BOOL) 
     */ 
static BOOL     FASTCALL
LoadCfgBool(char *key, BOOL def) 
{
    ASSERT(key);
    
	/*
	 * 読み込み 
	 */ 
	return INI_getBool(pszSection, key, def);
}


    /*
     *  設定データ ロード 
     */ 
void            FASTCALL
LoadCfg(void) 
{
    int            i;
    int            j;
    char           string[128];
    char           dir[MAXPATHLEN];
    char           InitDir[MAXPATHLEN];
    BOOL flag;
    static const int JoyTable[] =
	{ 0x70, 0x71, 0x72, 0x73, 0, 0x74, 0x75 
    };
    SetCfgFile();
    LoadCfgFile();
    
	/*
	 * Generalセクション 
	 */ 
	SetCfgSection("General");
    InitDir[0] = '\0';
    if (LoadCfgString("Directory", dir, 128)) {
	if (dir[strlen(dir) - 1] == '\\') {
	    
		/*
		 * 最後のパス区切り記号は強制的に削る 
		 */ 
		dir[strlen(dir) - 1] = '\0';
	}
	strcpy(InitDir, dir);
    }
    for (i = 0; i < 5; i++) {
	if (!LoadCfgString
	     ((char *) InitDirStr[i], InitialDir[i], MAXPATHLEN)) {
	    strcpy(InitialDir[i], InitDir);
	}
    } 
#if XM7_VER >= 3
	configdat.fm7_ver = LoadCfgInt("Version", 3);
    if ((configdat.fm7_ver < 1) || (configdat.fm7_ver > 3)) {
	configdat.fm7_ver = 3;
    }
    
#else				/*  */
    configdat.fm7_ver = LoadCfgInt("Version", 2);
    if ((configdat.fm7_ver < 1) || (configdat.fm7_ver > 2)) {
	configdat.fm7_ver = 2;
    }
    
#endif				/*  */
	i = LoadCfgInt("CycleSteel", 9999);
    if ((i < 0) || (i > 1)) {
	configdat.cycle_steal = LoadCfgBool("CycleSteal", TRUE);
    }
    
    else {
	configdat.cycle_steal = (BOOL) i;
    }
    configdat.main_speed = LoadCfgInt("MainSpeed", MAINCYCLES);
    if ((configdat.main_speed < 1) || (configdat.main_speed > 9999)) {
	configdat.main_speed = MAINCYCLES;
    }
    configdat.mmr_speed = LoadCfgInt("MMRSpeed", MAINCYCLES_MMR);
    if ((configdat.mmr_speed < 1) || (configdat.mmr_speed > 9999)) {
	configdat.mmr_speed = MAINCYCLES_MMR;
    }
    
#if XM7_VER >= 3
	configdat.fmmr_speed = LoadCfgInt("FastMMRSpeed", MAINCYCLES_FMMR);
    if ((configdat.fmmr_speed < 1) || (configdat.fmmr_speed > 9999)) {
	configdat.fmmr_speed = MAINCYCLES_FMMR;
    }
    
#endif				/*  */
	configdat.sub_speed = LoadCfgInt("SubSpeed", SUBCYCLES);
    if ((configdat.sub_speed < 1) || (configdat.sub_speed > 9999)) {
	configdat.sub_speed = SUBCYCLES;
    }
    configdat.bTapeFull = LoadCfgBool("TapeFullSpeed", TRUE);
    configdat.bCPUFull = LoadCfgBool("FullSpeed", FALSE);
    configdat.bSpeedAdjust = LoadCfgBool("AutoSpeedAdjust", FALSE);
    configdat.bTapeMode = LoadCfgBool("TapeFullSpeedMode", FALSE);
    
#ifdef FDDSND
	configdat.bFddWait = LoadCfgInt("FDDWait", FALSE);
    
#endif				/*  */
	
	/*
	 * 全般ページ(隠し) 
	 */ 
	configdat.uTimerResolution = LoadCfgInt("TimerResolution", 1);
    if ((configdat.uTimerResolution < 1)
	 || (configdat.uTimerResolution > 10)) {
	configdat.uTimerResolution = 1;
    }
    
	/*
	 * Soundセクション 
	 */ 
	SetCfgSection("Sound");
    configdat.nSampleRate = LoadCfgInt("SampleRate", 44100);
    if ((configdat.nSampleRate != 0) && 
	 (configdat.nSampleRate != 22050) && 
	 (configdat.nSampleRate != 44100) && 
	 (configdat.nSampleRate != 48000) && 
	 (configdat.nSampleRate != 88200)
	 && (configdat.nSampleRate != 96000)) {
	configdat.nSampleRate = 44100;
    }
    configdat.nSoundBuffer = LoadCfgInt("SoundBuffer", 100);
    if ((configdat.nSoundBuffer < 40) || (configdat.nSoundBuffer > 1000)) {
	configdat.nSoundBuffer = 100;
    }
    configdat.nBeepFreq = LoadCfgInt("BeepFreq", 1200);
    if ((configdat.nBeepFreq < 100) || (configdat.nBeepFreq > 9999)) {
	configdat.nBeepFreq = 1200;
    }
    configdat.bFMHQmode = LoadCfgBool("FMHQmode", TRUE);
    configdat.nStereoOut = LoadCfgInt("StereoOut", 0);
    if ((configdat.nStereoOut < 0) || (configdat.nStereoOut > 3)) {
	configdat.nStereoOut = 0;
    }
    configdat.bTapeMon = LoadCfgBool("TapeMon", FALSE);
    configdat.bForceStereo = LoadCfgInt("ForceStereoOutput", FALSE);
    
    configdat.nFMVolume = LoadCfgInt("FMVolume", FMVOLUME_DEFAULT);
    if ((configdat.nFMVolume < -96) || (configdat.nFMVolume > 10)) {
            configdat.nFMVolume = FMVOLUME_DEFAULT;
    }       
    configdat.nPSGVolume = LoadCfgInt("PSGVolume", PSGVOLUME_DEFAULT);
    if ((configdat.nPSGVolume < -96) || (configdat.nPSGVolume > 10)) {
            configdat.nPSGVolume = PSGVOLUME_DEFAULT;
    }       
   configdat.nBeepVolume = LoadCfgInt("BeepVolume", BEEPVOLUME_DEFAULT);
   if ((configdat.nBeepVolume < -96) || (configdat.nBeepVolume > 0)) {
           configdat.nBeepVolume = BEEPVOLUME_DEFAULT;
   }       
   configdat.nCMTVolume = LoadCfgInt("CMTVolume", CMTVOLUME_DEFAULT);
   if ((configdat.nCMTVolume < -96) || (configdat.nCMTVolume > 0)) {
           configdat.nCMTVolume = CMTVOLUME_DEFAULT;
   }       
#ifdef FDDSND
   configdat.nWaveVolume = LoadCfgInt("WaveVolume", WAVEVOLUME_DEFAULT);
   if ((configdat.nWaveVolume < -96) || (configdat.nWaveVolume > 0)) {
           configdat.nWaveVolume = WAVEVOLUME_DEFAULT;
    }       
    configdat.bFddSound = LoadCfgBool("FDDSound", FALSE);
#endif

	
/*
 * Keyboardセクション 
 */ 
        SetCfgSection("Keyboard");
        configdat.bKbdReal = LoadCfgBool("RealTimeKeyScan", FALSE);
        configdat.bTenCursor = LoadCfgBool("UseArrowFor10Key", FALSE);
        configdat.bArrow8Dir = LoadCfgBool("Arrow8Dir", TRUE);
        flag = FALSE;
    
	// /* キーマップ読み込み */
	// for (i=0; i<256; i++) {
	// sprintf(string, "Key%d", i);
	// j = i;
	// configdat.KeyMap[j] = (BYTE)LoadCfgInt(string, 0);
	// 
	// if (configdat.KeyMap[j] != 0) {
	// flag = TRUE;
	// }
	// }
	// /*
	// キーマップ設定なき時はデフォルトキーマップ。 
	// 
	// */
	// if (!flag) {
	// GetDefMapKbd(configdat.KeyMap, 0);
	// }
	/*
	 * JoyStickセクション 
	 */ 
	SetCfgSection("JoyStick");
    for (i = 0; i < 2; i++) {
	sprintf(string, "Type%d", i);
	configdat.nJoyType[i] = LoadCfgInt(string, 0);
	if ((configdat.nJoyType[i] < 0) || (configdat.nJoyType[i] > 4)) {
	    configdat.nJoyType[i] = 0;
	}
	for (j = 0; j < 2; j++) {
	    sprintf(string, "Rapid%d", i * 10 + j);
	    configdat.nJoyRapid[i][j] = LoadCfgInt(string, 0);
	    if ((configdat.nJoyRapid[i][j] < 0)
		 || (configdat.nJoyRapid[i][j] > 9)) {
		configdat.nJoyRapid[i][j] = 0;
	    }
	}
	flag = TRUE;
	for (j = 0; j < 7; j++) {
	    sprintf(string, "Code%d", i * 10 + j);
	    configdat.nJoyCode[i][j] = LoadCfgInt(string, -1);
	    if ((configdat.nJoyCode[i][j] < 0)
		 || (configdat.nJoyCode[i][j] > 0x75)) {
		flag = FALSE;
	    }
	}
	
	    /*
	     * レンジエラーなら初期値設定 
	     */ 
	    if (!flag) {
	    for (j = 0; j < 7; j++) {
		configdat.nJoyCode[i][j] = JoyTable[j];
	    }
	}
    }
    
	/*
	 * Screenセクション 
	 */ 
	SetCfgSection("Screen");
    configdat.bFullScan = LoadCfgBool("FullScan", FALSE);
    
	/*
	 * 現状、uWIDTH, uHeightは初期値固定 
	 */ 
	configdat.uWidth = 640;
    configdat.uHeight = 400;
    
	/*
	 * Optionセクション 
	 */ 
	SetCfgSection("Option");
    configdat.bOPNEnable = LoadCfgBool("OPNEnable", TRUE);
    configdat.bWHGEnable = LoadCfgBool("WHGEnable", TRUE);
    configdat.bTHGEnable = LoadCfgBool("THGEnable", TRUE);
    configdat.bDigitizeEnable = LoadCfgBool("DigitizeEnable", TRUE);
    
#if XM7_VER >= 3
	configdat.bExtRAMEnable = LoadCfgBool("ExtRAMEnable", FALSE);
    
#elif defined(FMTV151)
	configdat.bExtRAMEnable = bFMTV151;
    
#endif				/*  */
#ifdef MOUSE
	configdat.bMouseCapture = LoadCfgBool("MouseEmulation", FALSE);
    configdat.nMousePort = (BYTE) LoadCfgInt("MousePort", 1);
    configdat.nMidBtnMode =
	(BYTE) LoadCfgInt("MidBtnMode", MOSCAP_WMESSAGE);
    
#endif				/*  */
}


    /*
     *  設定データ 削除 
     */ 
static void     FASTCALL
DeleteCfg(char *key) 
{
    ASSERT(key);
    INI_clearKey(pszSection, key);
} 
    /*
     *  設定データ セーブ(文字列) 
     */ 
static void     FASTCALL
SaveCfgString(char *key, char *string) 
{
    ASSERT(key);
    ASSERT(string);
    INI_setString(pszSection, key, string);
} 
    /*
     *  設定データ セーブ(４バイトint) 
     */ 
static void     FASTCALL
SaveCfgInt(char *key, int dat) 
{
    ASSERT(key);
    INI_setInt(pszSection, key, dat);
} 
    /*
     *  設定データ セーブ(BOOL) 
     */ 
static void     FASTCALL
SaveCfgBool(char *key, BOOL dat) 
{
    ASSERT(key);
    INI_setBool(pszSection, key, dat);
} 
    /*
     *  設定データ セーブ 
     */ 
void            FASTCALL
SaveCfg(void) 
{
    int            i;
    int            j;
    char           string[128];
    SetCfgFile();
    
	/*
	 * Generalセクション 
	 */ 
    SetCfgSection("General");
    DeleteCfg("Directory");	/* V3.3L20で種別毎の保存に対応 */
    DeleteCfg("CycleSteel");	/* V3.2L01でCycleStealにキー名称を変更 
				 */
    for (i = 0; i < 5; i++) {
            SaveCfgString((char *) InitDirStr[i], InitialDir[i]);
    } SaveCfgInt("Version", configdat.fm7_ver);
    SaveCfgBool("CycleSteal", configdat.cycle_steal);
    SaveCfgInt("MainSpeed", configdat.main_speed);
    SaveCfgInt("MMRSpeed", configdat.mmr_speed);
    
#if XM7_VER >= 3
    SaveCfgInt("FastMMRSpeed", configdat.fmmr_speed);

#endif				/*  */
    SaveCfgInt("SubSpeed", configdat.sub_speed);
    SaveCfgBool("TapeFullSpeed", configdat.bTapeFull);
    SaveCfgBool("TapeFullSpeedMode", configdat.bTapeMode);
    SaveCfgBool("FullSpeed", configdat.bCPUFull);
    SaveCfgBool("AutoSpeedAdjust", configdat.bSpeedAdjust);
    
#ifdef FDDSND
	SaveCfgBool("FDDWait", configdat.bFddWait);
    
#endif				/*  */
	
	/*
	 * Soundセクション 
	 */ 
	SetCfgSection("Sound");
    SaveCfgInt("SampleRate", configdat.nSampleRate);
    SaveCfgInt("SoundBuffer", configdat.nSoundBuffer);
    SaveCfgInt("BeepFreq", configdat.nBeepFreq);
    SaveCfgBool("FMHQmode", configdat.bFMHQmode);
    SaveCfgInt("StereoOut", configdat.nStereoOut);
    SaveCfgBool("TapeMon", configdat.bTapeMon);
    
#ifdef FDDSND
    SaveCfgBool("FDDSound", configdat.bFddSound);
#endif				/*  */
    SaveCfgInt("ChannelSeparation", configdat.uChSeparation);
    SaveCfgInt("FMVolume", configdat.nFMVolume);
    SaveCfgInt("PSGVolume", configdat.nPSGVolume);
    SaveCfgInt("BeepVolume", configdat.nBeepVolume);
    SaveCfgInt("CMTVolume", configdat.nCMTVolume);
#ifdef FDDSND
    SaveCfgInt("WaveVolume", configdat.nWaveVolume);
#endif

	
	/*
	 * Keyboardセクション 
	 */ 
	SetCfgSection("Keyboard");
    SaveCfgBool("RealTimeKeyScan", configdat.bKbdReal);
    SaveCfgBool("UseArrowFor10Key", configdat.bTenCursor);
    SaveCfgBool("Arrow8Dir", configdat.bArrow8Dir);
    
	/*
	 * JoyStickセクション 
	 */ 
	SetCfgSection("JoyStick");
    for (i = 0; i < 2; i++) {
	sprintf(string, "Type%d", i);
	SaveCfgInt(string, configdat.nJoyType[i]);
	for (j = 0; j < 2; j++) {
	    sprintf(string, "Rapid%d", i * 10 + j);
	    SaveCfgInt(string, configdat.nJoyRapid[i][j]);
	}
	for (j = 0; j < 7; j++) {
	    sprintf(string, "Code%d", i * 10 + j);
	    SaveCfgInt(string, configdat.nJoyCode[i][j]);
	}
    }
    
	/*
	 * Screenセクション 
	 */ 
	SetCfgSection("Screen");
    SaveCfgBool("FullScan", configdat.bFullScan);
    
	/*
	 * Optionセクション 
	 */ 
    SetCfgSection("Option");
    DeleteCfg("SubBusyDelay");	/* V3.1で廃止 */
    SaveCfgBool("OPNEnable", configdat.bOPNEnable);
    SaveCfgBool("WHGEnable", configdat.bWHGEnable);
    SaveCfgBool("THGEnable", configdat.bTHGEnable);
    SaveCfgBool("DigitizeEnable", configdat.bDigitizeEnable);
    
#if XM7_VER >= 3
	SaveCfgBool("ExtRAMEnable", configdat.bExtRAMEnable);
    
#endif				/*  */
#ifdef MOUSE
    SaveCfgBool("MouseEmulation", configdat.bMouseCapture);
    SaveCfgInt("MousePort", configdat.nMousePort);
    
	/*
	 * SaveCfgInt("MidBtnMode", configdat.nMidBtnMode); 
	 */ 
#endif				/*  */
	SaveCfgFile();
}


    /*
     *  設定データ適用
     * ※VMのロックは行っていないので注意 
     */ 
void            FASTCALL
ApplyCfg(void) 
{
    
	/*
	 * Generalセクション 
	 */ 
    fm7_ver = configdat.fm7_ver;
    cycle_steal = configdat.cycle_steal;
    main_speed = configdat.main_speed * 10;
    mmr_speed = configdat.mmr_speed * 10;
    
#if XM7_VER >= 3
    fmmr_speed = configdat.fmmr_speed * 10;
    
#endif				/*  */
    sub_speed = configdat.sub_speed * 10;
    bTapeFullSpeed = configdat.bTapeFull;
    bFullSpeed = configdat.bCPUFull;
    bAutoSpeedAdjust = configdat.bSpeedAdjust;
    uTimerResolution = configdat.uTimerResolution;
    
#ifdef FDDSND
	fdc_waitmode = configdat.bFddWait;
    
#endif				/*  */
	bTapeModeType = configdat.bTapeMode;
    
	/*
	 * Soundセクション 
	 */ 
	nSampleRate = configdat.nSampleRate;
    nSoundBuffer = configdat.nSoundBuffer;
    nBeepFreq = configdat.nBeepFreq;
    bFMHQmode = configdat.bFMHQmode;
    nStereoOut = configdat.nStereoOut;
    bForceStereo = configdat.bForceStereo;
    bTapeMon = configdat.bTapeMon;
    tape_monitor = configdat.bTapeMon;
    
#ifdef FDDSND
    fdc_sound = configdat.bFddSound;
    tape_sound = configdat.bFddSound;
    
#endif				/*  */
	ApplySnd();
    
/*
 * Keyboardセクション 
 */ 
    bKbdReal = configdat.bKbdReal;
    bTenCursor = configdat.bTenCursor;
    bArrow8Dir = configdat.bArrow8Dir;
    
	/*
	 * JoyStickセクション 
	 */ 
	memcpy(nJoyType, configdat.nJoyType, sizeof(nJoyType));
    memcpy(nJoyRapid, configdat.nJoyRapid, sizeof(nJoyRapid));
    memcpy(nJoyCode, configdat.nJoyCode, sizeof(nJoyCode));
    
	/*
	 * Screenセクション 
	 */ 
	bFullScan = configdat.bFullScan;
    nDrawHeight = configdat.uHeight;
    nDrawWidth = configdat.uWidth;
    display_notify();
    
	/*
	 * Optionセクション 
	 */ 
    //opn_enable = configdat.bOPNEnable;
    //whg_enable = configdat.bWHGEnable;
    //thg_enable = configdat.bTHGEnable;
    digitize_enable = configdat.bDigitizeEnable;
    
#if XM7_VER >= 3
	mmr_extram = configdat.bExtRAMEnable;
    
#elif defined(FMTV151)
	bFMTV151 = configdat.bExtRAMEnable;
    
#endif				/*  */
#ifdef MOUSE
	mos_capture = configdat.bMouseCapture;
    mos_port = configdat.nMousePort;
    nMidBtnMode = configdat.nMidBtnMode;
    
#endif				/*  */
}


    /*
     *  動作機種再設定 
     */ 
void            FASTCALL
SetMachineVersion(void) 
{
    configdat.fm7_ver = fm7_ver;
} 
/*-[ 全般ページ ]-----------------------------------------------------------*/ 
    
    /*
     *  全般ページ ダイアログ初期化 
     */ 
static void     FASTCALL
GeneralPageInit(void) 
{
    char           string[128];
    
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
    }
    
    else {
	CheckDlgButton(GP_TAPESPEEDMODE, BST_UNCHECKED);
    }
    SetEnable(GP_TAPESPEEDMODE, propdat.bTapeFull);
    
	/*
	 * 全力駆動フラグ 
	 */ 
	if (propdat.bCPUFull) {
	CheckDlgButton(GP_FULLSPEED, BST_CHECKED);
    }
    
    else {
	CheckDlgButton(GP_FULLSPEED, BST_UNCHECKED);
    }
    
	/*
	 * 自動速度調整フラグ 
	 */ 
	if (propdat.bSpeedAdjust) {
	CheckDlgButton(GP_AUTOSPEEDADJUST, BST_CHECKED);
    }
    
    else {
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
void            FASTCALL
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
void            FASTCALL
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
static void     FASTCALL
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
static void     FASTCALL
SoundPageInit(void) 
{
    char           string[128];
    
	/*
	 * シート初期化 
	 */ 
	SheetInit();
    
	/*
	 * サンプリングレート 
	 */ 
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
	SelectOptionMenu(SP_STEREO, propdat.nStereoOut);
    
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
static void     FASTCALL
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
	propdat.nStereoOut = GetIdxOptionMenu(SP_STEREO);
    
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
static void     FASTCALL
KbdPageInit(void) 
{
    int            index;
    int            i;
    char           string[128];
    
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
static void     FASTCALL
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
static void     FASTCALL
JsPageInit(void) 
{
    int            index;
    int            i;
    char           string[128];
    
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
static void     FASTCALL
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
static void     FASTCALL
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
}


    /*
     *  スクリーンページ 適用 
     */ 
static void     FASTCALL
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
}


/*-[ オプションページ ]-----------------------------------------------------*/ 
    
    /*
     *  オプションページ ダイアログ初期化 
     */ 
static void     FASTCALL
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
static void     FASTCALL
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


/*-[ プロパティシート ]-----------------------------------------------------*/ 
    
    /*
     *  プロパティシート 初期化 
     */ 
static void     FASTCALL
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
void            FASTCALL
OnConfig(GtkWidget * widget, gpointer data) 
{
    
	/*
	 * データ転送 
	 */ 
	propdat = configdat;
    
	/*
	 * プロパティシート実行 
	 */ 
	uPropertyState = 0;
    uPropertyHelp = 0;
    winProperty = OpenPropertyPage();
    gtk_widget_show(winProperty);
    GeneralPageInit();
    SoundPageInit();
    KbdPageInit();
    JsPageInit();
    JsRapidPageInit();
    ScrPageInit();
    OptPageInit();
} 
    /*
     *  設定(C) 
     */ 
void            FASTCALL
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
    strcpy(icon_path, ModuleDir);
    switch (fm7_ver) {
    case 1:
	strcat(icon_path, "resource/tamori.ico");
	break;
    case 2:
	strcat(icon_path, "resource/app_av.ico");
	break;
    case 3:
	strcat(icon_path, "resource/app_ex.ico");
	break;
    default:
	icon_path[0] = '\0';
    }
    if (icon_path[0] != '\0' && strcmp(icon_path, ModuleDir) != 0)
	gtk_window_set_icon_from_file(GTK_WINDOW(wndMain), icon_path,
				       (GError **) NULL);
    UnlockVM();
}


#endif	/* _XWIN */

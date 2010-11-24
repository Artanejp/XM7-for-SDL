/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 * [ Agarコンフィギュレーション...Toolkit非依存部分 ]
 */


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

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif
#include "sdl_inifile.h"
#include "agar_cfg.h"
#include "sdl_prop.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_bar.h"

#include "api_kbd.h"
#include "api_js.h"
#include "api_mouse.h"
#include "api_draw.h"

/*
 *  設定データ定義 →sdl_cfg.h に。
 */

configdat_t configdat;	/* コンフィグ用データ */
/*
 *  スタティック ワーク
 */
static char   *pszSection;	/* セクション名 */

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
static void SetCfgFile(void)
{
    INI_init("XM7.INI");
}
    /*
     *  設定データ データロード
     */
static void LoadCfgFile(void)
{
    INI_load();
}
    /*
     *  設定データ データセーブ
     */
static void SaveCfgFile(void)
{
    INI_save();
}
    /*
     *  設定データ セクション名指定
     */
static void SetCfgSection(char *section)
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
static BOOL LoadCfgString(char *key, char *buf, int length)
{
    char          *dat;
    ASSERT(key);
    dat = INI_getString(pszSection, key, "");
    if ((strlen(dat) == 0) || (strlen(dat) > (size_t)length)) {
	return FALSE;
    }
    strcpy(buf, dat);
    return TRUE;
}


/*
 *  設定データ ロード(int)
 */
static int LoadCfgInt(char *key, int def)
{
    ASSERT(key);
    return INI_getInt(pszSection, key, def);
}


    /*
     *  設定データ ロード(BOOL)
     */
static BOOL LoadCfgBool(char *key, BOOL def)
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
void LoadCfg(void)
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
   if ((configdat.nBeepVolume < -96) || (configdat.nBeepVolume > 6)) { /* 最大値変更 */
           configdat.nBeepVolume = BEEPVOLUME_DEFAULT;
   }
   configdat.nCMTVolume = LoadCfgInt("CMTVolume", CMTVOLUME_DEFAULT);
   if ((configdat.nCMTVolume < -96) || (configdat.nCMTVolume > 6)) { /* 最大値変更 */
           configdat.nCMTVolume = CMTVOLUME_DEFAULT;
   }
#ifdef FDDSND
   configdat.nWaveVolume = LoadCfgInt("WaveVolume", WAVEVOLUME_DEFAULT);
   if ((configdat.nWaveVolume < -96) || (configdat.nWaveVolume > 6)) {/* 最大値変更 */
           configdat.nWaveVolume = WAVEVOLUME_DEFAULT;
    }
    configdat.bFddSound = LoadCfgBool("FDDSound", FALSE);
#endif

   configdat.uChSeparation = LoadCfgInt("ChannelSeparation", CHSEPARATION_DEFAULT);
   if ((configdat.uChSeparation <= 0 ) || (configdat.uChSeparation > 15)) {/* 最大値変更 */
           configdat.uChSeparation = CHSEPARATION_DEFAULT;
    }



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

    configdat.uWidth = LoadCfgInt("DrawWidth", 640);
    if((configdat.uWidth != 640)
       && (configdat.uWidth != 1280)
       && (configdat.uWidth != 320)) {
            configdat.uWidth = 640;
    }
    configdat.uHeight = LoadCfgInt("DrawHeight", 400);
    if((configdat.uHeight != 400)
       && (configdat.uHeight != 200)
       && (configdat.uHeight != 240)
       && (configdat.uHeight != 480)
       && (configdat.uHeight != 800)
       && (configdat.uHeight != 960)){
                configdat.uHeight = 400;
    }
    /*
     * FPS追加 20101018
     */
    configdat.nDrawFPS = LoadCfgInt("DrawFPS", 25);
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

    /*
     * UI
     */
	SetCfgSection("UI");
	if (!LoadCfgString("StatusFont", StatusFont, MAXPATHLEN)) {
	    strcpy(StatusFont, STAT_FONT);
	}

}


    /*
     *  設定データ 削除
     */
static void DeleteCfg(char *key)
{
    ASSERT(key);
    INI_clearKey(pszSection, key);
}
    /*
     *  設定データ セーブ(文字列)
     */
static void SaveCfgString(char *key, char *string)
{
    ASSERT(key);
    ASSERT(string);
    INI_setString(pszSection, key, string);
}
    /*
     *  設定データ セーブ(４バイトint)
     */
static void SaveCfgInt(char *key, int dat)
{
    ASSERT(key);
    INI_setInt(pszSection, key, dat);
}
    /*
     *  設定データ セーブ(BOOL)
     */
static void SaveCfgBool(char *key, BOOL dat)
{
    ASSERT(key);
    INI_setBool(pszSection, key, dat);
}
    /*
     *  設定データ セーブ
     */
void SaveCfg(void)
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
    }
    SaveCfgInt("Version", configdat.fm7_ver);
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
#endif			/**/
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
    SaveCfgInt("DrawWidth", configdat.uWidth);
    SaveCfgInt("DrawHeight", configdat.uHeight);
    SaveCfgInt("DrawFPS", configdat.nDrawFPS); // 20101018
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
    /*
     * UI (XM7/SDL Only)
     */
    SetCfgSection("UI");
    SaveCfgString("StatusFont", StatusFont);

    SaveCfgFile();
}

extern void 	ResizeGL(int w, int h);

    /*
     *  設定データ適用
     * ※VMのロックは行っていないので注意
     */
void ApplyCfg(void)
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
    nStereoOut = configdat.nStereoOut % 4;
    bForceStereo = configdat.bForceStereo;
    bTapeMon = configdat.bTapeMon;
    tape_monitor = configdat.bTapeMon;


#ifdef FDDSND
    fdc_sound = configdat.bFddSound;
    tape_sound = configdat.bFddSound;
#endif
    /* 音量設定 */
    uChSeparation = configdat.uChSeparation;
    nFMVolume = configdat.nFMVolume;
    nPSGVolume = configdat.nPSGVolume;
    nBeepVolume = configdat.nBeepVolume;
    nCMTVolume = configdat.nCMTVolume;
#ifdef FDDSND
    nWaveVolume = configdat.nWaveVolume;
#endif				/*  */
    uChSeparation = configdat.uChSeparation;

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
    nDrawFPS = configdat.nDrawFPS;
    display_notify();
	ResizeGL(nDrawWidth, nDrawHeight);


/*
 * Optionセクション
 */
    opn_enable = configdat.bOPNEnable;
    whg_enable = configdat.bWHGEnable;
    thg_enable = configdat.bTHGEnable;
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
void SetMachineVersion(void)
{
    configdat.fm7_ver = fm7_ver;
}



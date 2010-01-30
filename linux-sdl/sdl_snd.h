/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN サウンド ] 
 */  
    
#ifdef _XWIN

#ifndef _xw_snd_h_
#define _xw_snd_h_
    
/*
 *  定数定義 
 */ 
#define SNDBUF	20	      /* WAV最大同時発音数 */
#define XM7_PCM_MAX_VOLUME 0x3fff

#define CHSEPARATION_DEFAULT    7       /* チャンネルセパレーションデフォルト値 */
#define FMVOLUME_DEFAULT                0       /* FM音源ボリュームデフォルト値 */
#define PSGVOLUME_DEFAULT               -2      /* PSGボリュームデフォルト値 */
#define BEEPVOLUME_DEFAULT              -24     /* BEEP音ボリュームデフォルト値 */
#define CMTVOLUME_DEFAULT               -24     /* CMT音モニタボリュームデフォルト値 */
#define WAVEVOLUME_DEFAULT              -6      /* 各種効果音ボリュームデフォルト値 */

    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
/*
 *  主要エントリ 
 */ 
extern void        InitSnd(void);
                
/*
 * 初期化 
 */            
extern void        CleanSnd(void);
                   
/*
 * クリーンアップ 
 */            
extern BOOL        SelectSnd(void);
               
/*
 * セレクト 
 */            
extern void        ApplySnd(void);
                   
/*
 * 適用 
 */            
extern void        PlaySnd(void);
          
extern void         SetSoundVolume(void);

extern void  SetSoundVolume2(UINT uSp, int nFM, int nPSG,
                             int nBeep, int nCMT, int nWav);

/*
 * 演奏開始 
 */            
void        StopSnd(void);
                   
/*
 * 演奏停止 
 */            
void        ProcessSnd(BOOL bZero);
                   
/*
 * バッファ充填定時処理 
 */            
int         GetLevelSnd(int ch);
                   
/*
 * サウンドレベル取得 
 */            
void        OpenCaptureSnd(char *fname);
                   
/*
 * WAVキャプチャ開始 
 */            
void        CloseCaptureSnd(void);
                   
/*
 * WAVキャプチャ終了 
 */            
#ifdef FDDSND
void        InitFDDSnd(void);
                   
/*
 * 初期化 
 */            
void        CleanFDDSnd(void);
                   
/*
 * クリーンアップ 
 */            
#endif				/*  */
                   
/*
 *  主要ワーク 
 */            
extern UINT     nSampleRate;
             
/*
 * サンプルレート(Hz、0で無し) 
 */            
extern UINT     nSoundBuffer;
                   
/*
 * サウンドバッファ(ダブル、ms) 
 */            
extern BOOL     bFMHQmode;
                   
/*
 * FM高品質合成モード 
 */            
extern UINT     nBeepFreq;
                   
/*
 * BEEP周波数(Hz) 
 */            
extern int      hWavCapture;
                   
/*
 * WAVキャプチャファイルハンドル 
 */            
extern BOOL     bWavCapture;
                   
/*
 * WAVキャプチャ開始後 
 */            
extern UINT     nStereoOut;
                   
/*
 * 出力モード 
 */            
extern BOOL     bForceStereo;
                   
/*
 * 強制ステレオ出力 
 */            
extern BOOL     bTapeMon;
                   
/*
 * テープ音モニタ 
 */            
extern UINT     uClipCount;
                  
/*
 * クリッピングカウンタ 
 */            
#define SND_MAX_BANK 8
                  
#define SND_FIFO_SIZE (96000 * sizeof(WORD) * 2 * 1000)/1000
typedef enum { XM7_SND_FMBOARD = 0,	/* 富士通標準FM音源カード  */ 
               XM7_SND_FM_WHG = 1, /* WHG拡張FM音源 */ 
               XM7_SND_FM_THG = 2, /* THG拡張FM音源 */ 
               XM7_SND_MAIN = 3, XM7_SND_BEEP = 4, /* BEEP */ 
               XM7_SND_PSG = 5, /* PSG */ 
               XM7_SND_TAPE = 6, /* テープ音声 */ 
               /*
                * ここからWAVデータです 
                */ 
               XM7_SND_WAV_RELAY_ON = 7, /* リレーON */ 
               XM7_SND_WAV_RELAY_OFF = 8, /* リレーOFF */ 
               XM7_SND_WAV_FDD = 9, /* FDDシーク */ 
               XM7_SND_END = 10 
} SndChannels;
               
/*
 * 音楽演奏用データパッケージ 
 */            
                   
#ifdef FDDSND
extern int             nFMVolume;                                          /* FM音源ボリューム */
extern int             nPSGVolume;                                         /* PSGボリューム */
extern int             nBeepVolume;                                        /* BEEP音ボリューム */
extern int             nCMTVolume;                                         /* CMT音モニタボリューム */
extern int             nWaveVolume;                                        /* 各種効果音ボリューム */
extern int             iTotalVolume;	/* 全体ボリューム */
extern UINT            uChSeparation;

                   
/*
 * シーク音量 
 */            
                  
#endif				/*  */
#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_snd_h_ */
#endif	/* _XWIN */

/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN サウンド ]
 */

#ifdef _XWIN

#ifndef _xw_snd_h_
#define _xw_snd_h_

/*
 *	定数定義
 */
#define SNDBUF		20					/* WAV最大同時発音数 */


#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
void FASTCALL InitSnd(void);
										/* 初期化 */
void FASTCALL CleanSnd(void);
										/* クリーンアップ */
BOOL FASTCALL SelectSnd(void);
										/* セレクト */
void FASTCALL ApplySnd(void);
										/* 適用 */
void FASTCALL PlaySnd(void);
										/* 演奏開始 */
void FASTCALL StopSnd(void);
										/* 演奏停止 */
void FASTCALL ProcessSnd(BOOL bZero);
										/* バッファ充填定時処理 */
int FASTCALL GetLevelSnd(int ch);
										/* サウンドレベル取得 */
void FASTCALL OpenCaptureSnd(char *fname);
										/* WAVキャプチャ開始 */
void FASTCALL CloseCaptureSnd(void);
										/* WAVキャプチャ終了 */
#ifdef FDDSND
void FASTCALL InitFDDSnd(void);
										/* 初期化 */
void FASTCALL CleanFDDSnd(void);
										/* クリーンアップ */
#endif

/*
 *	主要ワーク
 */
extern UINT nSampleRate;
										/* サンプルレート(Hz、0で無し) */
extern UINT nSoundBuffer;
										/* サウンドバッファ(ダブル、ms) */
extern BOOL bFMHQmode;
										/* FM高品質合成モード */
extern UINT nBeepFreq;
										/* BEEP周波数(Hz) */
extern int hWavCapture;
										/* WAVキャプチャファイルハンドル */
extern BOOL bWavCapture;
										/* WAVキャプチャ開始後 */
extern UINT nStereoOut;
										/* 出力モード */
extern BOOL bForceStereo;
										/* 強制ステレオ出力 */
extern BOOL bTapeMon;
										/* テープ音モニタ */
extern UINT uClipCount;
									      
/* クリッピングカウンタ */
#define SND_MAX_BANK 8

typedef struct {
BYTE *buffer;  /* サウンドバッファ */
BYTE *uBufBegin[SND_MAX_BANK];
  int iCounter;
  int iWrote;
  BOOL bEndFlag;
DWORD uBufSize;
DWORD dwPlayC;
BYTE TotalVolume;
SDL_sem * playing;
BOOL isEnabled;
BOOL isEmpty;
}sndCallbackData;

  /* PCMデータ用FIFO */
typedef struct {
  BYTE *data;
  int bufSize;
  int readPtr;
  int writePtr;
  int dataSize;
  //int lastReadSize;
  BOOL bOverFlow;
  BOOL bUnderFlow;
  SDL_sem *fifo_sem;
} sndFifo;

#define SND_FIFO_SIZE (96000 * sizeof(WORD) * 2 * 1000)/1000

  typedef enum {
    XM7_SND_FMBOARD = 0, /* 富士通標準FM音源カード */
    XM7_SND_FM_WHG = 1, /* WHG拡張FM音源 */
    XM7_SND_FM_THG = 2, /* THG拡張FM音源 */
    XM7_SND_MAIN = 3,
    XM7_SND_BEEP = 4, /* BEEP */
    XM7_SND_PSG = 5, /* PSG */
    XM7_SND_TAPE = 6, /* テープ音声 */
    /* ここからWAVデータです */
    XM7_SND_WAV_RELAY_ON = 7, /* リレーON */
    XM7_SND_WAV_RELAY_OFF = 8, /* リレーOFF */
    XM7_SND_WAV_FDD = 9, /* FDDシーク */
    XM7_SND_END = 10
  } SndChannels;
    



/* 音楽演奏用データパッケージ */ 

#ifdef FDDSND
extern UINT uSeekVolume;
										/* シーク音量 */


#endif
#ifdef __cplusplus
}
#endif

#endif	/* _xw_snd_h_ */
#endif	/* _XWIN */

/*
 *  FM-7 EMULATOR "XM7"
 * Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp) 
 * Copyright (C) 2001-2003 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta 
 * [ SDL サウンド ]
 * 20100115 Initial
 * 20100117 SDL_Mixerを使うように変更した 
 */
#ifdef _XWIN

#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else				/* */
#include <linux/soundcard.h>
#endif				/* */
#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "opn.h"
// #include "whg.h"
// #include "thg.h"
#include "tapelp.h"
#include "cisc.h"
#include "opna.h"
#include "psg.h"
#include "opn.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "sdl_snd.h"

    /*
     *  グローバル ワーク 
     */
UINT                    nSampleRate;    /* サンプリングレート */
UINT                    nSoundBuffer;   /* サウンドバッファサイズ */
UINT                    nStereoOut;     /* 出力モード */
BOOL                    bFMHQmode;      /* FM高品質合成モード */
BOOL                    bForceStereo;   /* 強制ステレオ出力 */
UINT                    nBeepFreq;      /* BEEP周波数 */
BOOL                    bTapeMon;       /* テープ音モニタ */
int                     hWavCapture;    /* WAVキャプチャハンドル */
BOOL                    bWavCapture;    /* WAVキャプチャ開始 */
UINT                    uClipCount;     /* クリッピングカウンタ */

int                     nFMVolume;      /* FM音源ボリューム */
int                     nPSGVolume;     /* PSGボリューム */
int                     nBeepVolume;    /* BEEP音ボリューム */
int                     nCMTVolume;     /* CMT音モニタボリューム */
int                     nWaveVolume;    /* 各種効果音ボリューム */
int                     iTotalVolume;   /* 全体ボリューム */
UINT                    uChSeparation;
UINT                    uStereoOut;     /* 出力モード */

    /*
     *  スタティック ワーク
     */

    /*
     *   SDL -マルチスレッド化するために… 
     */
static SDL_sem          *musicSem;      /* マルチスレッドで同期するためのセマフォ(出力) */
static SDL_sem          *applySem;      /* マルチスレッドで同期するためのセマフォ(Apply期間中) */

static SDL_Thread       *playThread;
static int              uProcessCount;
static SDL_TimerID      uTid;
static UINT             localTick;      /* VM実行時間 */
static DWORD            dwExecLocal;
static BYTE             *lpdsb;         /* DSP用サウンド作成バッファ */
static DWORD            dwPlayC;        /* DSP用サウンド作成バッファ内の再生位置 */
static BOOL             bNowBank;
static DWORD            *lpsbuf;

    /*
     * 音声合成用のバッファ 
     */
//static BYTE             *sndSrcBuf[SOUND_FDDHEADDOWN + 1];/* バッファは別々にとる */
//static int              sndLen[SOUND_FDDHEADDOWN +1];
static Mix_Chunk        *sndDstBuf[XM7_SND_END + 1];    /* バッファは別々にとる */
static BYTE             *opnBuf[3];
static BYTE             *psgBuf;
static UINT             uBufSize;       /* サウンドバッファサイズ */
static UINT             uRate;          /* 合成レート */
static UINT             uTick;          /* 半バッファサイズの長さ */
static BOOL             bMode;          /* FM高品質合成モード */
static UINT             uStereo;        /* 出力モード */
static UINT             uSample;        /* サンプルカウンタ */
static UINT             uBeep;          /* BEEP波形カウンタ */
static int              nFMVol;
static int              nPSGVol;
static int              nBeepVol;
static int              nCMTVol;
static int              nWavVol;
static UINT             uChanSep;
static FM::OPN          *pOPN[3];       /* OPNデバイス */
static int              nScale[3];      /* OPNプリスケーラ */
static BYTE             uCh3Mode[3];    /* OPN Ch.3モード */
static BOOL             bInitFlag;      /* 初期化フラグ */
static WORD             *pWavCapture;   /* キャプチャバッファ(64KB) */
static UINT             nWavCapture;    /* キャプチャバッファ実データ */
static DWORD            dwWavCapture;   /* キャプチャファイルサイズ */
static WORD             uChannels;      /* 出力チャンネル数 */
static BOOL             bBeepFlag;      /* BEEP出力 */
static BOOL             bPartMute[3][6];        /* パートミュートフラグ */
static int              nBeepLevel;     /* BEEP音出力レベル */
static int              nCMTLevel;      /* CMT音モニタ出力レベル */
#ifdef FDDSND
static int              nWaveLevel;     /* 各種効果音出力レベル */
#endif

static BOOL             bTapeFlag;      /* 現在のテープ出力状態 */
static BOOL             bTapeFlag2;     /* 前回のテープ出力状態 */
static BYTE             uTapeDelta;     /* テープ波形補間カウンタ */
/* ステレオ出力時の左右バランステーブル */
static int              l_vol[3][4] = {
	{	16,	23,	 9,	16	},
	{	16,	 9,	23,	 9	},
	{	16,	16,	16,	23	},
};
static int r_vol[3][4] = {
	{	16,	 9,	23,	16	},
	{	16,	23,	 9,	23	},
	{	16,	16,	16,	 9	},
};


#ifdef FDDSND
    /*
     *  スタティック ワーク (WAV再生) 
     */
static struct _WAVDATA {
    short          *p;  /* 波形データポインタ */
    DWORD       size;   /* データサイズ(サンプル数) */

    DWORD       freq;   /* サンプリング周波数 */
} Wav[SOUND_FDDHEADDOWN + 1];

static struct _WAVPLAY {
    BOOL        bPlay;          /* WAV再生フラグ */
    DWORD       dwWaveNo;       /* WAVでーたなんばー */
    DWORD       dwCount1;       /* WAVでーたかうんた(整数部) */
    DWORD       dwCount2;       /* WAVでーたかうんた(小数部) */
    DWORD       dwCount3;       /* WAV再生かうんた */
} WavP[SNDBUF];
/*
 * Wav 演奏フラグ 
 */

static char     *WavName[] = {
 /* WAVファイル名 */
    "RELAY_ON.WAV",
    "RELAY_OFF.WAV",
    "FDDSEEK.WAV",
    NULL,
    NULL
#if 0
    "HEADUP.WAV",
    "HEADDOWN.WAV"
#endif  /* */
};


#endif  /* */

/*
 *  WAVファイル作成用構造体 
 */
typedef struct {
        WORD    wFormatTag;
        WORD    nChannels;
        DWORD   nSamplesPerSec;
        DWORD   nAvgBytesPerSec;
        WORD    nBlockAlign;
        WORD    wBitsPerSample;
        WORD    cbSize;
} WAVEFORMATEX;

static  WORD    WAVE_FORMAT_PCM = 0x0001;

/*
 *      * アセンブラ関数のためのプロトタイプ宣言→x86依存一度外す 
 */
#ifdef __cplusplus
extern
    "C" {

#endif
void    (*CopySoundBuffer) (DWORD * src, WORD * dst, int count);
    // extern void CopySndBufMMX(DWORD *src, WORD *dst, int count);
    // extern void CopySndBuf(DWORD *src, WORD *dst, int count);
#ifdef __cplusplus
}
#endif				/* */
static void
WaveSnd();

/*
 *  サウンドバッファへのコピー
 * XM7内のサウンドレンダリングは32bitで格納されてるので
 * 上位)16bitを切り捨てる 場合に依っては高速化が必要。 
 */
static void
CopySoundBufferGeneric(DWORD * from, WORD * to, int size)
{
        int         i,j;
        int32       *p = (int32 *) from;
        int16       *t = (int16 *) to;
        int32       tmp1;

        if (p == NULL) {
                return; 
        }
        if (t == NULL) {
                return;
        }
        i = (size / 4) * 4;
        for (j = 0; j < i; j += 4) {
                tmp1 = p[j];
                t[j] = (int16) (tmp1 & 0x0000ffff);
                tmp1 = p[j + 1];
                t[j + 1] = (int16) (tmp1 & 0x0000ffff);
                tmp1 = p[j + 2];
                t[j + 2] = (int16) (tmp1 & 0x0000ffff);
                tmp1 = p[j + 3];
                t[j + 3] = (int16) (tmp1 & 0x0000ffff);
        }
        i = size - i;
        for (j = 0; j < i; j++) {
                tmp1 = p[j];
                t[j] = (int16) (tmp1 & 0x0000ffff);
        }
}


/*
 *  実時間を測定(timeGetTime互換関数) 
 */
static
    DWORD
timeGetTime(void)
{
    return SDL_GetTicks();
}


/*
 *  初期化 
 */
void
InitSnd(void)
{
    int i;
/*
 * ワークエリア初期化 
 */
    nSampleRate = 44100;
    nSoundBuffer = 100;
    bFMHQmode = FALSE;
    nBeepFreq = 1200;
    nStereoOut = 0;
    bForceStereo = FALSE;
    bTapeMon = TRUE;
    uChSeparation = 9;
    uChanSep = uChSeparation;
    nFMVolume = 0;
    nFMVol = nFMVolume;
    nPSGVolume = -2;
    nPSGVol = nPSGVolume;
    nBeepVolume = -24;
    nBeepVol = nBeepVolume;
    nCMTVolume = -24;
    nCMTVol = nCMTVolume;
    nWaveVolume = 0;
    nWavVol = nWaveVolume;

    iTotalVolume = SDL_MIX_MAXVOLUME - 1;
    hWavCapture = -1;
    bWavCapture = FALSE;
    pWavCapture = NULL;
    nWavCapture = 0;
    dwWavCapture = 0;
    lpdsb = NULL;
    dwPlayC = 0;
    lpsbuf = NULL;
    bNowBank = 0;
    dwExecLocal = dwExecTotal;
    uBufSize = 0;
    uRate = 0;
    uSample = 0;
    uBeep = 0;
    bMode = FALSE;
    uStereo = 0;
    uChannels = 1;
    uClipCount = 0;
    bBeepFlag = FALSE;
    bTapeFlag = FALSE;
    bTapeFlag2 = FALSE;
    uTapeDelta = 0;
    pOPN[0] = NULL;
    pOPN[1] = NULL;
    pOPN[2] = NULL;
    nScale[0] = 0;
    nScale[1] = 0;
    nScale[2] = 0;
    memset(uCh3Mode, 0xff, sizeof(uCh3Mode));
    bInitFlag = FALSE;

    musicSem = NULL;

#ifdef FDDSND

    for (i = 0; i <= SOUND_FDDHEADDOWN; i++) {

    }

#endif				/* */

    /*
     * SDL用変数 
     */
    for (i = 0; i < XM7_SND_END; i++) {
	sndDstBuf[i] = NULL;
    }
    applySem = SDL_CreateSemaphore(0);
    CopySoundBuffer = CopySoundBufferGeneric;
    InitFDDSnd();
    SDL_InitSubSystem(SDL_INIT_AUDIO);
}

    /*
     *  クリーンアップ 
     */
void
CleanSnd(void)
{
    int
	i;
    int
	freq;
    Uint16
	format;
    int
	channels;
    int
	ret;
    /*
     * サウンド停止 
     */
    StopSnd();

    // do { /* StopSNDでCloseしてはならない 20100126 */
    // Mix_CloseAudio();
    // ret = Mix_QuerySpec(&freq, &format, &channels);
    // } while(ret != 0);

    for (i = XM7_SND_FMBOARD; i < XM7_SND_END + 1; i++) {
	if (sndDstBuf[i]) {
	    free(sndDstBuf[i]);	/* CloseしたらFreeChunk不要 ? */
	    sndDstBuf[i] = NULL;
	}
    }
    // SDL_LockAudio();
    // SDL_CloseAudio();
    /*
     * WAVバッファ解放
     */
    /*
     * OPNを解放 
     */
    for (i = 0; i < 3; i++) {
	if (pOPN[i]) {
	    delete
		pOPN[i];
	    pOPN[i] = NULL;
	}
    }
    for (i = 0; i < 3; i++) {
	if (opnBuf[i] != NULL) {
	    free(opnBuf[i]);
	}
	opnBuf[i] = NULL;
    }
    if (psgBuf != NULL)
	free(psgBuf);
    psgBuf = NULL;
//    for (i = 0; i <= SOUND_FDDHEADDOWN; i++) {
//	if (sndSrcBuf[i]) {
//	    free(sndSrcBuf[i]);
//	    sndSrcBuf[i] = NULL;
//	}
//    }

    /*
     * サウンド作成バッファを解放 
     */
    if (lpdsb) {
	free(lpdsb);
	lpdsb = NULL;
    }
    /*
     * サウンド作成バッファを解放 
     */
    if (lpsbuf) {
	free(lpsbuf);
	lpsbuf = NULL;
    }
    /*
     * Wavバッファを解放
     */

    /*
     * uRateをクリア 
     */
    uRate = 0;
    Mix_CloseAudio();

#if 1				/* WAVキャプチャは後で作る */
    /*
     * キャプチャ関連 
     */
    if (hWavCapture >= 0) {
	CloseCaptureSnd();
    }
    if (pWavCapture) {
	free(pWavCapture);
	pWavCapture = NULL;
    }
    hWavCapture = -1;
    bWavCapture = FALSE;
#endif				/* */
}


#ifdef FDDSND


    /*
     *  WAVEファイル読み込み
     * (16ビットモノラルデータ専用) 
     */
static BOOL
LoadWav(char *fname, struct _WAVDATA *wav)
{
    WAVEFORMATEX
	wfex;
    BYTE
	buf[16];
    DWORD
	filSize;
    DWORD
	hdrSize;
    DWORD
	datSize;
    int
                    fileh;
    ASSERT(fname);
    ASSERT(wav);

    /*
     * ファイルオープン 
     */
    fileh = file_open(fname, OPEN_R);
    if (fileh < 0) {
	return FALSE;
    }

    /*
     * RIFFヘッダチェック 
     */
    file_read(fileh, buf, 4);
    file_read(fileh, (BYTE *) & filSize, 4);
    buf[4] = '\0';
    if (strcmp((char *) buf, "RIFF")) {
	file_close(fileh);
	return FALSE;
    }
    filSize += 8;

    /*
     * WAVEヘッダチェック 
     */
    file_read(fileh, buf, 8);
    file_read(fileh, (BYTE *) & hdrSize, 4);
    buf[8] = '\0';
    if (strcmp((char *) buf, "WAVEfmt ")) {
	file_close(fileh);
	return FALSE;
    }
    hdrSize += (12 + 8);

    /*
     * WAVEFORMATEXチェック 
     */
    file_read(fileh, (BYTE *) & wfex, sizeof(wfex));
    if ((wfex.wFormatTag != WAVE_FORMAT_PCM) ||
	(wfex.nChannels != 1) || (wfex.wBitsPerSample != 16)) {

	/*
	 * 16ビットモノラル・リニアPCM以外は不可 
	 */
	file_close(fileh);
	return FALSE;
    }

    /*
     * dataチャンク検索 
     */
    while (hdrSize < filSize) {

	/*
	 * チャンクヘッダ読み込み 
	 */
	file_seek(fileh, hdrSize);
	file_read(fileh, buf, 4);
	file_read(fileh, (BYTE *) & datSize, 4);
	buf[4] = '\0';

	/*
	 * 次のチャンクヘッダオフセットを計算 
	 */
	hdrSize += (datSize + 8);
	if (strcmp((char *) buf, "data") == 0) {

	    /*
	     * dataチャンク読み込み 
	     */
	    wav->size = datSize / 2;
	    wav->freq = wfex.nSamplesPerSec;
	    wav->p = (short *) malloc(datSize);
	    if (wav->p == NULL) {
		file_close(fileh);
		return FALSE;
	    }
	    if (!file_read(fileh, (BYTE *) wav->p, wav->size)) {
		file_close(fileh);
		free(wav->p);
		wav->p = NULL;
		return FALSE;
	    }
	    file_close(fileh);
	    return TRUE;
	}
    }

    /*
     * dataチャンク発見できず 
     */
    file_close(fileh);
    return FALSE;
}




    /*
     *  FDDサウンド 初期化 
     */
void
InitFDDSnd(void)
{
        int i, ret, size;
    int fileh;
    struct stat fileStat;
    for (i = 0; i < 3; i++) {
            /*
             * ワーク初期化 
             */
            if(WavName[i]) {
                    LoadWav(WavName[i], &Wav[i]);
            }
            
            /*
             * とりあえずWAV読み込む。
             * OPENしたままになるので注意。 
             */
            //if (SDL_LoadWAV(WavName[i], WavSpec[i], &WavBuf[i], &WavLen[i]) == NULL) {

                    //fileh = file_open(WavName[i], OPEN_R);
                    //if(fileh < 0) {
                    //printf("Warning: Unable to load WAV#%d (%s).\n", i, WavName[i]);
                    //}
                    //ret = fstat(fileh, &fileStat);
                    //size = (int)fileStat.st_size;
            //if(size >= 0x10000) {
            //        printf("Warning: WavSize too large!\n", i, WavName[i]);
            //        size = 0x0ffff;
            //}    
            //WavBuf[i] = (BYTE *)malloc(size + 1);
            //file_read(fileh, WavBuf[i], size);
            //file_close(fileh);
            //printf("Loaded WAV #%d (%s). ADDR=%08x \n", i,
            //       WavName[i], WavBuf[i]);
            //WavSpec[i] = Mix_QuickLoad_WAV(WavBuf[i]); /* 最初の設定 */
            //}   
/*
 * WAVファイル読み込み 
 */
    }
}


    /*
     *  FDDサウンド クリーンアップ 
     */
void
CleanFDDSnd(void)
{
    int
                    i;
    for (i = 0; i <= SOUND_FDDHEADDOWN; i++) {

	/*
	 * WAVファイル読み込み 
	 */
	if (sndDstBuf[i + XM7_SND_WAV_RELAY_ON] != NULL) {
	    free(sndDstBuf[i + XM7_SND_WAV_RELAY_ON]);
	    sndDstBuf[i + XM7_SND_WAV_RELAY_ON] = NULL;
	}
        
	/*
	 * ワーク初期化 
	 */
        if(Wav[i].p != NULL) {
                free(Wav[i].p);
                Wav[i].p = NULL;
        }
    }
}


#endif				/* */

    /*
     *  レジスタ設定 
     */
static void
SetReg(FM::OPN * pOPN, BYTE * reg)
{
    int
                    i;

    /*
     * PSG 
     */
    for (i = 0; i < 16; i++) {
	pOPN->SetReg((BYTE) i, reg[i]);
    }

    /*
     * FM音源キーオフ 
     */
    for (i = 0; i < 3; i++) {
	pOPN->SetReg(0x28, (BYTE) i);
    }

    /*
     * FM音源レジスタ 
     */
    for (i = 0x30; i < 0xb4; i++) {
	pOPN->SetReg((BYTE) i, reg[i]);
    }

    /*
     * FM音源動作モード 
     */
    pOPN->SetReg(0x27, reg[0x27] & 0xc0);
}


    /*
     * 演奏用コールバック　
     */

    /*
     *  セレクト 
     */
BOOL
SelectSnd(void)
{
    int                 fmt;
    int                 channel;
    int                 freq;
    int                 arg;
    int                 i;
    int                 len;
    DWORD               bytes;
    SDL_AudioCVT        cvt;
    SDL_AudioSpec       in, out;

    /*
     * 起動フラグ立てる 
     */
    bInitFlag = TRUE;
    uTid = 0;

    /*
     * パラメータを設定 
     */
    uRate = nSampleRate;
    uTick = nSoundBuffer;
    bMode = bFMHQmode;
    uStereo = nStereoOut;
    nFMVol = nFMVolume;
    nPSGVol = nPSGVolume;
    nCMTVol = nCMTVolume;
    nBeepVol = nBeepVolume;
    nWavVol = nWaveVolume;
    uChanSep = uChSeparation;
    dwExecLocal = dwExecTotal;
    bNowBank = 0;
    uTapeDelta = 0;
    if ((uStereo > 0) || bForceStereo) {
	uChannels = 2;
    } else {
	uChannels = 1;
    }

#ifdef FDDSND
#endif				/* */

    /*
     * rate==0なら、何もしない 
     */
    if (uRate == 0) {
	return TRUE;
    }

    /*
     * SDL用変数領域の設定 
     */

    /*
     * SDL用サウンドバッファを作成 
     */
    bytes = (uRate * sizeof(WORD) * uChannels * uTick) / 1000;
    bytes += (DWORD) 7;
    bytes &= (DWORD) 0xfffffff8;	/* 8バイト境界 */
    uBufSize = bytes;
    if (musicSem == NULL) {
	musicSem = SDL_CreateSemaphore(0);
    }
    lpdsb = (BYTE *) malloc(uBufSize);
    if (lpdsb == NULL) {
	return FALSE;
    }
    memset(lpdsb, 0, uBufSize);

    /*
     * WAVEバッファ 
     */

    /*
     * opnバッファ 
     */
    for (i = 0; i < 3; i++) {
	opnBuf[i] = (BYTE *) malloc(uBufSize);	/* OPN合成は32bit */
	if (opnBuf[i] == NULL) {
	    printf("Err: Can't alloc opn[%d]. \n", i);
	    return FALSE;
	}
	memset(opnBuf[i], 0, uBufSize);
    }
    psgBuf = (BYTE *) malloc(uBufSize);	/* PSG合成は32bit */
    if (psgBuf == NULL) {
	printf("Err: Can't alloc psg. \n");
	return FALSE;
    }
    memset(psgBuf, 0, uBufSize);

    /*
     * サウンドバッファを作成(DSP用バッファの半分の時間で、DWORD) 
     */
    lpsbuf = (DWORD *) malloc(uBufSize);
    if (lpsbuf == NULL) {
	return FALSE;
    }
    memset(lpsbuf, 0, uBufSize);

    /*
     * サンプルカウンタ、サウンド時間をクリア 
     */
    uSample = 0;
    dwSoundTotal = 0;
    uClipCount = 0;

    /*
     * OPNデバイス(標準)を作成 
     */
    pOPN[0] = new FM::OPN;
    pOPN[0]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
    pOPN[0]->Reset();
    pOPN[0]->SetReg(0x27, 0);

    /*
     * OPNデバイス(WHG)を作成 
     */
    pOPN[1] = new FM::OPN;
    pOPN[1]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
    pOPN[1]->Reset();
    pOPN[1]->SetReg(0x27, 0);

    /*
     * OPNデバイス(THG)を作成 
     */
    pOPN[2] = new FM::OPN;
    pOPN[2]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
    pOPN[2]->Reset();
    pOPN[2]->SetReg(0x27, 0);

    SDL_SemPost(applySem);

    /*
     * 再セレクトに備え、レジスタ設定 
     */
    nScale[0] = 0;
    nScale[1] = 0;
    nScale[2] = 0;
    opn_notify(0x27, 0);
    whg_notify(0x27, 0);
    thg_notify(0x27, 0);
    SetReg(pOPN[0], opn_reg[OPN_STD]);
    SetReg(pOPN[1], opn_reg[OPN_WHG]);
    SetReg(pOPN[2], opn_reg[OPN_THG]);

    /*
     * キャプチャ関連 
     */
    if (!pWavCapture) {
	pWavCapture = (WORD *) malloc(sizeof(WORD) * 0x8000);
    }
    ASSERT(hWavCapture == -1);
    ASSERT(!bWavCapture);

    /*
     * SDL用に仕様変更 
     */

    // if(Mix_OpenAudio(uRate, AUDIO_S16SYS, uChannels, uBufSize) ==
    // -1) {
    if (Mix_OpenAudio
	(uRate, AUDIO_S16SYS, uChannels,
	 uBufSize / (2 * sizeof(int16) * uChannels)) == -1) {
	printf("Warning: Audio can't initialize!\n");	/* 後でダイアログにする 
							 */
	return FALSE;
    }
    Mix_AllocateChannels(XM7_SND_MAIN + 1);
    
    /*
     * テンプレ作成 
     */
    for (i = XM7_SND_FMBOARD; i < XM7_SND_END + 1; i++) {
	sndDstBuf[i] = (Mix_Chunk *) malloc(sizeof(Mix_Chunk));
	if (sndDstBuf[i] == NULL) {
	    printf("Err: Audio buffer can't allocate!\n");
	    return FALSE;
	}
	memset(sndDstBuf[i], 0, sizeof(Mix_Chunk));
    }

    /*
     * 各バッファとChunkの紐づけ 
     */
    for (i = 0; i < 2; i++) {
	sndDstBuf[i]->allocated = TRUE;
	sndDstBuf[i]->abuf = lpdsb;
	sndDstBuf[i]->alen = uBufSize;
	sndDstBuf[i]->volume = MIX_MAX_VOLUME;
    }

#ifdef FDDSND
    /*
     * 予めOpenしてあったWAVデータの読み込み 
     */

#endif				/* */

    /*
     * サウンドスタート 
     */
    PlaySnd();
    /* ボリューム設定 */
    SetSoundVolume();

    return TRUE;
   
}

/*
 *  適用 
 */
void
ApplySnd(void)
{

    /*
     * 起動処理時は、リターン 
     */
    if (!bInitFlag) {
	return;
    }

    /*
     * パラメータ一致チェック 
     */
    if ((uRate == nSampleRate) && (uTick == nSoundBuffer) &&
        (bMode == bFMHQmode) && (uStereo == nStereoOut) &&
        (nFMVol == nFMVolume) && (nPSGVol == nPSGVolume) &&
        (nBeepVol == nBeepVolume) && (nCMTVol == nCMTVolume) &&
        (nWavVol == nWaveVolume) && (uChanSep == uChSeparation)) {
	return;
    }
    printf("Apply!\n");
 /* 音声プロパティとOPNが衝突しないようにするためのセマフォ初期化 */
    SDL_SemWait(applySem);
    /*
     * 既に準備ができているなら、解放 
     */
    if (uRate != 0) {
	CleanSnd();
    }

    /*
     * 再セレクト 
     */
    SelectSnd();
    //SDL_SemPost(applySem);

}

/*
 * ボリューム設定: XM7/Win32 v3.4L30より
 */
void SetSoundVolume(void)
{
	int i;

	/* FM音源/PSGボリューム設定 */
	for (i=0; i<3; i++) {
		//ASSERT(pOPN[i]);

		if (pOPN[i]) {
			pOPN[i]->SetVolumeFM(nFMVolume * 2);
			pOPN[i]->psg.SetVolume(nPSGVolume * 2);
		}
	}

	/* BEEP音/CMT音/各種効果音ボリューム設定 */
	nBeepLevel = (int)(32767.0 * pow(10.0, nBeepVolume / 20.0));
	nCMTLevel = (int)(32767.0 * pow(10.0, nCMTVolume / 20.0));
#ifdef FDDSND
	nWaveLevel = (int)(32767.0 * pow(10.0, nWaveVolume / 20.0));
#endif

	/* チャンネルセパレーション設定 */
	l_vol[0][1] = l_vol[1][2] = l_vol[2][3] =
	r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = 16 + uChSeparation;
	r_vol[0][1] = r_vol[1][2] = r_vol[2][3] =
	l_vol[1][1] = l_vol[0][2] = l_vol[1][3] = 16 - uChSeparation;
}

/*
 *	ボリューム設定2(設定ダイアログ用)
 */
void  SetSoundVolume2(UINT uSp, int nFM, int nPSG,
                      int nBeep, int nCMT, int nWav)
{
        int i;

        uChSeparation = uSp;
        uChanSep = uChSeparation;
        nFMVolume = nFM;
        nFMVol = nFMVolume;
        nPSGVolume = nPSG;
        nPSGVol = nPSGVolume;
        nBeepVolume = nBeep;
        nBeepVol = nBeepVolume;
        nCMTVolume = nCMT;
        nCMTVol = nCMTVolume;
        nWaveVolume = nWav;
        nWavVol = nWaveVolume;

        /* 即時Apply出来るようにAssertではなくReturnする */
        if((pOPN[1] == NULL) || (pOPN[2] == NULL) || (pOPN[3] == NULL)) {
                        return;
        }       

        /* FM音源/PSGボリューム設定 */
	for (i=0; i<3; i++) {
		//ASSERT(pOPN[i]);
                
		if (pOPN[i]) {
			pOPN[i]->SetVolumeFM(nFM * 2);
			pOPN[i]->psg.SetVolume(nPSG * 2);
		}
	}

	/* BEEP音/CMT音/各種効果音ボリューム設定 */
	nBeepLevel = (int)(32767.0 * pow(10.0, nBeep / 20.0));
	nCMTLevel = (int)(32767.0 * pow(10.0, nCMT / 20.0));
#ifdef FDDSND
	nWaveLevel = (int)(32767.0 * pow(10.0, nWav / 20.0));
#endif

	/* チャンネルセパレーション設定 */
	l_vol[0][1] = l_vol[1][2] = l_vol[2][3] =
	r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = 16 + uSp;
	r_vol[0][1] = r_vol[1][2] = r_vol[2][3] =
	l_vol[1][1] = l_vol[0][2] = l_vol[1][3] = 16 - uSp;
}

/*
 *  演奏開始 
 */
void
PlaySnd()
{
    int
                    i;

    /*
     * サンプルカウンタ、サウンド時間をクリア 
     */
    uSample = 0;
    dwSoundTotal = 0;
    uClipCount = 0;
    dwPlayC = 0;
    uProcessCount = 0;
    dwExecLocal = dwExecTotal;
    iTotalVolume = SDL_MIX_MAXVOLUME - 1;	/* ここで音量設定する必要があるか? 
						 */
    memset(lpdsb, 0x00, uBufSize);
    SDL_SemPost(musicSem);

    /*
     * 変換できたので読み込む 
     */
}
/*
 *  演奏停止 
 */
void
StopSnd()
{

    // if(musicSem) {
    // SDL_SemWait(musicSem);
    // }
}
    /*
     *  BEEP合成 
     */
static void
BeepSnd(int32 * sbuf, int samples)
{
    int
                    sf;
    int
                    i;
    int32          *
	buf = (int32 *) sbuf;
    Mix_Chunk
	chunk;

    /*
     * BEEP音出力チェック 
     */
    if (!bBeepFlag) {
	return;
    }

    /*
     * サンプル書き込み 
     */
    for (i = 0; i < samples; i++) {

	/*
	 * 矩形波を作成 
	 */
	sf = (int) (uBeep * nBeepFreq * 2);
	sf /= (int) uRate;

	/*
	 * 偶・奇に応じてサンプル書き込み 
	 */
	if (uChannels == 1) {
	    if (sf & 1) {
		*buf++ += nBeepLevel;
	    }

	    else {
		*buf++ += -nBeepLevel;
	    }
	}

	else {
	    if (sf & 1) {
		*buf++ += nBeepLevel;
		*buf++ += nBeepLevel;
	    }

	    else {
		*buf++ += -nBeepLevel;
		*buf++ += -nBeepLevel;
	    }
	}

	/*
	 * カウンタアップ 
	 */
	uBeep++;
	if (uBeep >= uRate) {
	    uBeep = 0;
	}
    }
}


    /*
     *  テープ合成 (V3.1) SDL_Mixer対応:16bit仕様に 
     */
static void
TapeSnd(int32 * sbuf, int samples)
{
    DWORD
	dat;
    int
                    i;
    int
                    tmp;
    Mix_Chunk
	chunk;
    int32          *
	buf = (int32 *) sbuf;

    /*
     * テープ出力チェック 
     */
    if (!tape_motor || !bTapeMon) {
	return;
    }

    /*
     * 波形分割数を求める 
     */
    if ((uRate == 48000) || (uRate == 96000)) {
	tmp = (uRate * 5) / 48000;
    }

    else {
	tmp = (uRate * 4) / 44100;
    }

    /*
     * 出力状態が変化した場合、波形補間を開始する 
     */
    if (bTapeFlag != bTapeFlag2) {
	if (!uTapeDelta) {
	    uTapeDelta = 1;
	}

	else {
	    uTapeDelta = (BYTE) (tmp - uTapeDelta + 1);
	}
    }

    /*
     * サンプル書き込み 
     */
    for (i = 0; i < samples; i++) {
	if (uTapeDelta) {

	    /*
	     * 波形補間あり 
	     */
	    dat = (0x1000 / tmp) * uTapeDelta;
	    if (bTapeFlag) {
		dat = dat - nCMTLevel;
	    }

	    else {
		dat = nCMTLevel - dat;
	    }
	    uTapeDelta++;
	    if (uTapeDelta > tmp) {
		uTapeDelta = 0;
	    }
	}

	else {

	    /*
	     * 波形補間なし 
	     */
	    if (bTapeFlag) {
		dat = nCMTLevel;
	    }

	    else {
		dat = -nCMTLevel;
	    }
	}

	/*
	 * BIG ENDIANの場合にこれでいいのか確認 
	 */
	*buf++ += (Uint16) dat;	/* 音量小さすぎないか */
	if (uChannels == 2) {
	    *buf++ += (Uint16) dat;
	}
    }

    /*
     * 現在のテープ出力状態を保存 
     */
    bTapeFlag2 = bTapeFlag;
}


    /*
     *  WAVデータ合成 (FDD/CMT) 
     */
#ifdef FDDSND
    /*
     *  WAVデータは自動でMixされる(by SDL_Mixer)
     * WAVデータ合成（演奏) 
     */
static void
WaveSnd(int32 * buf, int samples)
{
	int i;
	int j;
	int dat;

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		for (j=0; j<SNDBUF; j++) {
			if (WavP[j].bPlay) {
				dat = Wav[WavP[j].dwWaveNo].p[WavP[j].dwCount1];
				dat = (short)(((int)dat * nWaveLevel) >>16);
				*buf += (int)dat;
				if (uChannels == 2) {
					*(buf+1) += (int)dat;
				}

				/* カウントアップ */
				WavP[j].dwCount2 += (Wav[WavP[j].dwWaveNo].freq << 16) / uRate;
				if (WavP[j].dwCount2 > 0x10000) {
					WavP[j].dwCount1 += (WavP[j].dwCount2 >> 16);
					WavP[j].dwCount2 &= 0xFFFF;
					if (WavP[j].dwCount1 >= Wav[WavP[j].dwWaveNo].size) {
						WavP[j].bPlay = FALSE;
					}
				}
				WavP[j].dwCount3 ++;
			}
		}
		buf ++;
		if (uChannels == 2) {
			buf ++;
		}
	}

}
#endif				/* */

/*
 *	波形合成
 */
static void FASTCALL MixingSound(DWORD *q, int samples, BOOL bZero)
{
        memset(q, 0, sizeof(DWORD) * samples * uChannels);
        if (!bZero) {
                if (uChannels == 1) {
/* モノラル */
#ifdef ROMEO
                        if ((pOPN[OPN_STD]) && !bUseRomeo) {
#else
                                if (pOPN[OPN_STD]) {
#endif
                                        pOPN[OPN_STD]->Mix((int32*)q, samples);
                                }
                                if (whg_use) {
#ifdef ROMEO
                                        if (bUseRomeo) {
                                                pOPN[OPN_WHG]->psg.Mix((int32*)q, samples);
                                        }
                                        else {
                                                pOPN[OPN_WHG]->Mix((int32*)q, samples);
                                        }
#else
                                        pOPN[OPN_WHG]->Mix((int32*)q, samples);
#endif
                                }
                                if (thg_use) {
                                        pOPN[OPN_THG]->Mix((int32*)q, samples);
                                }
                                else if (fm7_ver == 1) {
                                        pOPN[OPN_THG]->psg.Mix((int32*)q, samples);
		}
                        }
                        else {
                                /* ステレオ */
                                if (!whg_use && !thg_use) {
                                        /* WHG/THGを使用していない(強制モノラル) */
#ifdef ROMEO
                                        if (!bUseRomeo) {
                                                pOPN[OPN_STD]->Mix2((int32*)q, samples, 16, 16);
                                        }
#else
                                        pOPN[OPN_STD]->Mix2((int32*)q, samples, 16, 16);
#endif
                                        if (fm7_ver == 1) {
                                                pOPN[OPN_THG]->psg.Mix2((int32*)q, samples, 16, 16);
                                        }
                                }
                                else {
                                        /* WHGまたはTHGを使用中 */
#ifdef ROMEO
                                        if (!bUseRomeo) {
                                                pOPN[OPN_STD]->Mix2((int32*)q, samples,
                                                                    l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
                                        }
#else
                                        pOPN[OPN_STD]->Mix2((int32*)q, samples,
                                                            l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
#endif
                                        if (whg_use) {
#ifdef ROMEO
                                                if (bUseRomeo) {
                                                        pOPN[OPN_WHG]->psg.Mix2((int32*)q, samples,
                                                                                l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
                                                }
                                                else {
                                                        pOPN[OPN_WHG]->Mix2((int32*)q, samples,
                                                                            l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
                                                }
#else
                                                pOPN[OPN_WHG]->Mix2((int32*)q, samples,
                                                                    l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
#endif
                                        }
                                        if (thg_use) {
                                                pOPN[OPN_THG]->Mix2((int32*)q, samples,
                                                                    l_vol[OPN_THG][uStereo], r_vol[OPN_THG][uStereo]);
                                        }
                                        else if (fm7_ver == 1) {
                                                pOPN[OPN_THG]->psg.Mix2((int32*)q, samples, 16, 16);
                                        }
                                }
                        }

                        /* テープ */
                        TapeSnd((int32*)q, samples);

                        /* ビープ */
                        BeepSnd((int32*)q, samples);

#ifdef FDDSND
                        /* WAVサウンド */
                        WaveSnd((int32*)q, samples);
#endif
                }
        }


/*
 *  サウンド作成バッファへ追加 
 */
static void
AddSnd(BOOL bFill, BOOL bZero)
{
    int	samples;
    int	i;
    int	retval;
    DWORD        *q;
    WORD         *p;
    /*
     * OPNデバイスが作成されていなければ、何もしない 
     */
    if (!pOPN[2] ) {
	return ;
    }
    /*
     * bFillの場合のサンプル数(モノラル2byte/sample・ステレオ4byte/sample) 
     */
    samples = (uBufSize >> 2) / uChannels;
    samples -= uSample;

    /*
     * !bFillなら、時間から計測 
     */
    if (!bFill) {

	/*
	 * 時間経過から求めた理論サンプル数 
	 */
	/*
	 * 計算結果がオーバーフローする問題に対策
	 * 2002/11/25 
	 */
	i = (uRate / 25);
	i *= dwSoundTotal;
	i /= 40000;

	/*
	 * uSampleと比較、一致していれば何もしない 
	 */
	if (i <= (int) uSample) {
	    return ;
	}

	/*
	 * uSampleとの差が今回生成するサンプル数 
	 */
	i -= (int) (uSample);

	/*
	 * samplesよりも小さければ合格 
	 */
	if (i <= samples) {
	    samples = i;
	}
    }
    if (musicSem)
	SDL_SemWait(musicSem);

    /*
     * バッファを求める 
     */
    q = &lpsbuf[uSample * uChannels];

    /*
     * ミキシング 
     */
    MixingSound(&lpsbuf[uSample * uChannels], samples, bZero);
    /*
     * 更新 
     */
    if (bFill) {
	if (sndDstBuf[XM7_SND_MAIN] != NULL) {
	    uSample += samples;
	    p = (WORD *) lpdsb;
	    q = (DWORD *) lpsbuf;
	    i = (int) (uSample * uChannels);
	    if (uSample > 0) {
		CopySoundBufferGeneric(q, p, i);
		sndDstBuf[XM7_SND_MAIN]->allocated = 1;
		sndDstBuf[XM7_SND_MAIN]->abuf = (Uint8 *) lpdsb;
		sndDstBuf[XM7_SND_MAIN]->alen =
		    (Uint32) (uSample * uChannels * 2);
		sndDstBuf[XM7_SND_MAIN]->volume = iTotalVolume;
		Mix_PlayChannel(0, sndDstBuf[XM7_SND_MAIN], 0);
	    }
	}
	dwSoundTotal = 0;
	uSample = 0;
    } else {
	uSample += samples;
    }
    if (musicSem)
	SDL_SemPost(musicSem);
    return;
}



/*
 *  WAVキャプチャ処理 
 */
static void
WavCapture(void)
{
    UINT
	nSize;
    DWORD          *
	p;
    WORD           *
	q;
    int
	j;

    /*
     * WAVキャプチャ中でなければ、リターン 
     */
    if (hWavCapture < 0) {
	return;
    }
    ASSERT(pWavCapture);

    /*
     * ポインタ、サイズを仮決め(nSizeはWORD変換後のBYTE値) 
     */
    p = lpsbuf;
    nSize = uBufSize / 2;

    /*
     * bWavCaptureがFALSEなら 
     */
    if (!bWavCapture) {
	/*
	 * 頭出しチェック 
	 */
	while (nSize > 0) {
	    if (uChannels == 1) {
		if (*p != 0) {
		    break;
		} else {
		    nSize -= 2;
		    p++;
		}
	    } else {
		if ((p[0] != 0) || (p[1] != 0)) {
		    break;
		} else {
		    nSize -= 4;
		    p += 2;
		}
	    }
	}

	/*
	 * 判定 
	 */
	if (nSize == 0) {
	    return;
	}
    }

    /*
     * nWavCaptureを考慮 
     */
    if ((nWavCapture + nSize) >= 0x8000) {

	/*
	 * 32KBいっぱいまでコピー 
	 */
	j = (0x8000 - nWavCapture) >> 1;
	q = &pWavCapture[nWavCapture >> 1];
	CopySoundBuffer(p, q, j);
	p += j;

	/*
	 * 残りサイズを更新 
	 */
	nSize -= (0x8000 - nWavCapture);

	/*
	 * 書き込み 
	 */
	file_write(hWavCapture, (BYTE *) pWavCapture, 0x8000);
	dwWavCapture += 0x8000;
	nWavCapture = 0;
    }

    /*
     * 余りをコピー 
     */
    j = nSize >> 1;
    q = &pWavCapture[nWavCapture >> 1];
    CopySoundBuffer(p, q, j);
    nWavCapture += nSize;
    /*
     * 正式な録音状態 
     */
    bWavCapture = TRUE;
}


/*
 *  定期処理 / 1msごとに呼び出される。 
 */
void
ProcessSnd(BOOL bZero)
{
        BOOL    bWrite;
        WORD    *ptr1;
        DWORD   size1;
        DWORD   *p;
        DWORD   dwOffset;
        int     i,len;
        int     wsize = 0;
        int     wsamples;

/*
 * 初期化されていなければ、何もしない 
 */
        if ((!lpdsb) || (!musicSem)) {
                return;
        }

        // printf("Wrote: %d %d %d\n", wsize, dwPlayC, SDL_GetTicks());

/*
 * 書き込み位置とバンクから、必要性を判断 
 */
        bWrite = FALSE;
        if (bNowBank) {
                if (dwPlayC >= (uTick / 2)) {
                        bWrite = TRUE;
                }
        } else {
                if (dwPlayC < (uTick / 2)) {
                        bWrite = TRUE;
                }
        }
        if (dwPlayC >= uTick) {
                dwPlayC = 0;
        }
        dwPlayC++;

/*
 * サウンドデータの生成 
 */

 /*
  * 書き込む必要がなければ、リターン 
  */
        if (!bWrite) {

/*
 * テープ 
 */
                if (tape_motor && bTapeMon) {
                        bWrite = TRUE;
                }

/*
 * BEEP 
 */
                if (beep_flag && speaker_flag) {
                        bWrite = TRUE;
                }

/*
 * どちらかがONなら、バッファ充填 
 */
                if (bWrite) {
                        AddSnd(FALSE, bZero);
                }
                return;
        }
        
/*
 * ここから演奏開始 
 */

        AddSnd(TRUE, bZero);

/*
 * 書き込みバンク(仮想) 
 */
        bNowBank = (!bNowBank);
}


/*
 *  レベル取得 
 */
int
GetLevelSnd(int ch)
{
        FM::OPN *p;
        int     i;
        double  s;
        double  t;
        int     *buf;

        ASSERT((ch >= 0) && (ch < 18));

/*
 * OPN,WHGの区別 
 */
        if (ch < 6) {
                p = pOPN[0];
        }
        if ((ch >= 6) && (ch < 12)) {
                p = pOPN[1];
                ch -= 6;
/*
 * WHGの場合、実際に使われていなければ0 
 */
                if (!whg_enable || !whg_use) {
                        return 0;
                }
        }
        if ((ch >= 12) && (ch < 18)) {
                p = pOPN[2];
                ch -= 12;

/*
 * THGの場合、実際に使われていなければ0 
 */
                if ((!thg_enable || !thg_use) && (fm7_ver != 1)) {
                        return 0;
                }
        }

/*
 * 存在チェック 
 */
        if (!p) {
                return 0;
        }

/*
 * FM,PSGの区別 
 */
        if (ch < 3) {
                /*
                 * FM:512サンプルの2乗和を計算 
                 */
                buf = p->rbuf[ch];
                s = 0;
                for (i = 0; i < 512; i++) {
                        t = (double) *buf++;
                        t *= t;
                        s += t;
                }
                s /= 512;

                /*
                 * ゼロチェック 
                 */
                if (s == 0) {
                        return 0;
                }

                /*
                 * log10を取る 
                 */
                s = log10(s);

                /*
                 * FM音源補正 
                 */
                s *= 40.0;
        } else {

                /*
                 * PSG:512サンプルの2乗和を計算 
                 */
                buf = p->psg.rbuf[ch - 3];
                s = 0;
                for (i = 0; i < 512; i++) {
                        t = (double) *buf++;
                        t*= t;
                        s += t;
                }
                s /= 512;

                /*
                 * ゼロチェック 
                 */
                if (s == 0) {
                        return 0;
                }

                /*
                 * log10を取る 
                 */
                s = log10(s);

                /*
                 * PSG音源補正 
                 */
                s *= 60.0;
        }
        return (int) s;
}       

/*
 *  WAVキャプチャ開始 
 */
void
OpenCaptureSnd(char *fname)
{
        WAVEFORMATEX        wfex;
        DWORD               dwSize;
        int                 fileh;

        ASSERT(fname);
        ASSERT(hWavCapture < 0);
        ASSERT(!bWavCapture);

/*
 * 合成中でなければ、リターン 
 */
        if (!pOPN[OPN_STD] || !pOPN[OPN_WHG] || !pOPN[OPN_THG]) {
                return;
        }

/*
 * バッファが無ければ、リターン 
 */
        if (!pWavCapture) {
                return;
        }

/*
 * uBufSize / 2が0x8000以下でないとエラー 
 */
        if ((uBufSize / 2) > 0x8000) {
                return;
        }

/*
 * ファイルオープン(書き込みモード) 
 */
        fileh = file_open(fname, OPEN_W);
        if (fileh < 0) {
                return;
        }

/*
 * RIFFヘッダ書き込み 
 */
        if (!file_write(fileh, (BYTE *) "RIFFxxxxWAVEfmt ", 16)) {
                file_close(fileh);
                return;
        }

/*
 * WAVEFORMATEX書き込み 
 */
        dwSize = sizeof(wfex);
        if (!file_write(fileh, (BYTE *) & dwSize, sizeof(dwSize))) {
                file_close(fileh);
                return;
        }
        memset(&wfex, 0, sizeof(wfex));
        wfex.cbSize = sizeof(wfex);
        wfex.wFormatTag = WAVE_FORMAT_PCM;
        wfex.nChannels = uChannels;
        wfex.nSamplesPerSec = uRate;
        wfex.nBlockAlign = (WORD) (2 * uChannels);
        wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
        wfex.wBitsPerSample = 16;
        if (!file_write(fileh, (BYTE *) & wfex, sizeof(wfex))) {
                file_close(fileh);
                return;
        }

/*
 * dataサブヘッダ書き込み 
 */
        if (!file_write(fileh, (BYTE *) "dataxxxx", 8)) {
                file_close(fileh);
                return;
        }

/*
 * ok 
 */
        nWavCapture = 0;
        dwWavCapture = 0;
        bWavCapture = FALSE;
        hWavCapture = fileh;
}

/*
 *  WAVキャプチャ終了 
 */
void
CloseCaptureSnd(void)
{
        DWORD   dwLength;
        ASSERT(hWavCapture >= 0);

/*
 * バッファに残った分を書き込み 
 */

        file_write(hWavCapture, (BYTE *) pWavCapture, nWavCapture);
        dwWavCapture += nWavCapture;
        nWavCapture = 0;

/*
 * ファイルレングスを書き込む 
 */
        file_seek(hWavCapture, 4);
        dwLength = dwWavCapture + sizeof(WAVEFORMATEX) + 20;
        file_write(hWavCapture, (BYTE *) & dwLength, sizeof(dwLength));

/*
 * data部レングスを書き込む 
 */
        file_seek(hWavCapture, sizeof(WAVEFORMATEX) + 24);
        file_write(hWavCapture, (BYTE *) & dwWavCapture, sizeof(dwWavCapture));

/*
 * ファイルクローズ 
 */
        file_close(hWavCapture);

/*
 * ワークエリアクリア 
 */
        hWavCapture = -1;
        bWavCapture = FALSE;
}


    /*
     *  OPN出力 
     */
extern
    "C" {
void
opn_notify(BYTE reg, BYTE dat) {
/*
 * ApplySnd()期間中にアクセスがあると落ちる
 * 事に対する対策 20100201
 */

        if(applySem == NULL) {
                return;
        }
        SDL_SemTryWait(applySem);
/*
 * OPNがなければ、何もしない 
 */
        if (!pOPN[OPN_STD]) {
                SDL_SemPost(applySem);
                return;
        }
/*
 * プリスケーラを調整 
 */ 
        if (opn_scale[OPN_STD] != nScale[OPN_STD]) {
                nScale[OPN_STD] = opn_scale[OPN_STD];
                switch (opn_scale[OPN_STD]) {
                case 2:
                        pOPN[OPN_STD]->SetReg(0x2f, 0);
                        break;
                case 3:
                        pOPN[OPN_STD]->SetReg(0x2e, 0);
                        break;
                case 6:
                        pOPN[OPN_STD]->SetReg(0x2d, 0);
                        break;
                }
        }

/*
 * Ch3動作モードチェック 
 */
        if (reg == 0x27) {
                if (uCh3Mode[OPN_STD] == dat) {
                        SDL_SemPost(applySem);
                        return;
                }
                uCh3Mode[OPN_STD] = dat;
        }

/*
 * 0xffレジスタはチェック 
 */
        if (reg == 0xff) {
                if ((opn_reg[OPN_STD][0x27] & 0xc0) != 0x80) {
                        SDL_SemPost(applySem);
                        return;
                }
        }

 /*
 * サウンド合成 
 */
	AddSnd(FALSE, FALSE);

/*
 * 出力 
 */
        pOPN[OPN_STD]->SetReg((uint8) reg, (uint8) dat);
       SDL_SemPost(applySem);
    }
}
/*
 *  WHG出力 
 */
extern
        "C" {
void
whg_notify(BYTE reg, BYTE dat)
{

/*
 * ApplySnd()中にアクセスがあると落ちることへの対策
 * 20100201
 */
        if(applySem == NULL) {
                return;
        }
        SDL_SemTryWait(applySem);
/*
 * WHGがなければ、何もしない 
 */

        if (!pOPN[OPN_WHG]) {
                SDL_SemPost(applySem);
                return;
        }


/*
 * プリスケーラを調整 
 */ 
        if (opn_scale[OPN_WHG] != nScale[OPN_WHG]) {
                nScale[OPN_WHG] = opn_scale[OPN_WHG];
                switch (opn_scale[OPN_WHG]) {
                case 2:
                        pOPN[OPN_WHG]->SetReg(0x2f, 0);
                        break;
                case 3:
                        pOPN[OPN_WHG]->SetReg(0x2e, 0);
                        break;
                case 6:
                        pOPN[OPN_WHG]->SetReg(0x2d, 0);
                        break;
                }
        }

/*
 * Ch3動作モードチェック 
 */
        if (reg == 0x27) {
                if (uCh3Mode[OPN_WHG] == dat) {
                        SDL_SemPost(applySem);
                        return;
                }
                uCh3Mode[OPN_WHG] = dat;
        }

/*
 * 0xffレジスタはチェック 
 */
        if (reg == 0xff) {
                if ((opn_reg[OPN_WHG][0x27] & 0xc0) != 0x80) {
                        SDL_SemPost(applySem);
                        return;
                }
        }

/*
 * サウンド合成 
 */
        AddSnd(FALSE, FALSE);

/*
 * 出力 
 */
        pOPN[OPN_WHG]->SetReg((uint8) reg, (uint8) dat);
        SDL_SemPost(applySem);
    }
}


/*
 *  THG出力 
 */
extern
    "C" {
void
thg_notify(BYTE reg, BYTE dat)
{
/*
 * ApplySnd()期間中にアクセスがあると落ちる
 * 事に対する対策 20100201
 */
        if(applySem == NULL) {
                return;
        }
        SDL_SemTryWait(applySem);

/*
 * THGがなければ、何もしない 
 */
        if (!pOPN[OPN_THG]) {
                SDL_SemPost(applySem);
                return;
        }

/*
 * プリスケーラを調整 
 */ 
        if (opn_scale[OPN_THG] != nScale[OPN_THG]) {
                nScale[OPN_THG] = opn_scale[OPN_THG];
                switch (opn_scale[OPN_THG]) {
                case 2:
                        pOPN[OPN_THG]->SetReg(0x2f, 0);
                        break;
                case 3:
                        pOPN[OPN_THG]->SetReg(0x2e, 0);
                        break;
                case 6:
                        pOPN[OPN_THG]->SetReg(0x2d, 0);
                        break;
                }
        }

/*
 * Ch3動作モードチェック 
 */
        if (reg == 0x27) {
                if (uCh3Mode[OPN_THG] == dat) {
                        SDL_SemPost(applySem);
                        return;
                }
                uCh3Mode[OPN_THG] = dat;
        }
/*
 * 0xffレジスタはチェック 
 */
        if (reg == 0xff) {
                if ((opn_reg[OPN_THG][0x27] & 0xc0) != 0x80) {
                        SDL_SemPost(applySem);
                        return;
                }
        }

/*
 * サウンド合成 
 */
        AddSnd(FALSE, FALSE);

/*
 * 出力 
 */
        pOPN[OPN_THG]->SetReg((uint8) reg, (uint8) dat);
        SDL_SemPost(applySem);
    }
}

/*
 *  BEEP出力 
 */
extern
    "C" {
void
beep_notify(void)
{

/*
 * 出力状態が変化していなければリターン 
 */
        if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
                return;
        }

/*
 * サウンド合成 
 */ 
        AddSnd(FALSE, FALSE);

/*
 * フラグ保持 
 */
        if (beep_flag && speaker_flag) {
                bBeepFlag = TRUE;
        } else {
                bBeepFlag = FALSE;
        }
}
}

/*
 *  テープ出力 
 */
extern
    "C" {
void
tape_notify(BOOL flag)
{
/*
 * 出力状態が変化したかチェック 
 */
        if (bTapeFlag == flag) {
                return;
        }

/*
 * サウンド合成 
 */ 
        if (bTapeMon) {
                AddSnd(FALSE, FALSE);
        }

/*
 * フラグ保持 
 */
        bTapeFlag = flag;
}
}


/*
 *  WAV出力 
 */
#ifdef FDDSND
extern    "C" {
void 
wav_notify(BYTE no)
{
        int        i;
        int        j;
        DWORD      k;

/*
 * サウンド合成 
 */
        AddSnd(FALSE, FALSE);
        if (no == SOUND_STOP) {
                /* 停止 */
                for (i=0; i<SNDBUF; i++) {
                        WavP[i].bPlay = FALSE;
                }
        }
        else {
                if (Wav[no].freq) {
                        j = 0;
                        k = 0;
                        for (i=0; i<SNDBUF; i++) {
                                /* 再生停止中のチャンネルを検索 */
                                if (!WavP[i].bPlay) {
                                        j = i;
                                        break;
                                }
                                else {
                                        /* 一番最初に再生が開始されたチャンネルを検索 */
                                        if (k < WavP[i].dwCount3) {
                                                k = WavP[i].dwCount3;
                                                j = i;
                                        }
                                }
                        }

                        /* データセット */
                        WavP[j].dwWaveNo        = no;
                        WavP[j].dwCount1        = 0;
                        WavP[j].dwCount2        = 0;
                        WavP[j].dwCount3        = 0;
                        WavP[j].bPlay           = TRUE;
                }
        }
}
}

#endif				/* */
#endif				/* _XWIN */

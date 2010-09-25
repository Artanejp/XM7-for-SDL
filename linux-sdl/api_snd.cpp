/*
 * snd_api.cpp
 *
 *  Created on: 2010/09/26
 *      Author: whatisthis
 */

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

#include "SndDrvBeep.h"


#define WAV_SLOT 5
#define SND_BUF 2
/*
 *      * アセンブラ関数のためのプロトタイプ宣言→x86依存一度外す
 */
#ifdef __cplusplus
extern "C" {

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


static int16 *Wav[WAV_SLOT];
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

static SndDrvBeep *DrvBeep;
static SndDrvBeep *DrvWav[WAV_SLOT];

/*
 * 予めLoadしておいたWAVを演奏できるようにする
 */


/*
 * ファイルの読み込みは別オブジェクトにする
 */
static Uint8 *LoadWav(char *filename, int slot)
{
	int fileh;
	int filesize;
	if(slot >= WAV_SLOT) return NULL;
	Uint8 *rbuf = (Uint8 *)Wav[slot];

	if(Wav[slot] != NULL) return NULL; /* 既にWAVが読み込まれてる */
	fileh = file_open(filename, OPEN_R);
	if(fileh < 0) return NULL;
	filesize = file_size(fileh);

	Wav[slot] = malloc(filesize + 1);
	if(Wav[slot] == NULL) {
		file_close(fileh);
		return NULL; /* 領域確保に失敗 */
	}

	file_read(fileh, rbuf, filesize);
	file_close(fileh);
	return Wav[slot];
}

/*
 * 読み込んだWAVを消去する
 */
static void WavDelete(int slot)
{
	if(slot >= WAV_SLOT) return;
	if(Wav[slot] == NULL) return;
	free(Wav[slot]);
	Wav[slot] = NULL;
}

/*
 *  初期化
 */
void
InitSnd(void)
{
	int i,j;
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


	iTotalVolume = SDL_MIX_MAXVOLUME - 1;
	uBufSize = 0;
	uRate = 0;
	uSample = 0;
	uBeep = 0;
	bMode = FALSE;
	uStereo = 0;

	uClipCount = 0;
	bInitFlag = FALSE;
	//    InitFDDSnd();
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	/*
	 * WAVよむ
	 */
	for(i = 0; i < WAV_SLOT; i++){
		Wav[i] = (Uint16 *)LoadWav(WavName[i], i);
	}

	/*
	 * オブジェクト作成
	 */
	DrvBeep = new SndDrvBeep[SND_BUF];
	for(i = 0; i<WAV_SLOT; i++) {
		DrvWav[i] = new SndDrvWav[SND_BUF];
	}
}

/*
 *  クリーンアップ
 */
void
CleanSnd(void)
{
	int i,j;
	/*
	 * サウンド停止
	 */
	StopSnd();
	/*
	 * スレッド停止
	 */

	/*
	 * スレッド資源解放待ち
	 */
	/*
	 * OPNを解放
	 */
	/*
	 * サウンド作成バッファを解放
	 */
	/*
	 * Wavバッファを解放
	 */
	for(i = 0; i< WAV_SLOT; i++){
		WavDelete(i);
	}
	if(DrvBeep) 	delete [] DrvBeep;
	for(i = 0; i<WAV_SLOT; i++) {
		if(DrvWav[i])  	delete [] DrvWav[i];
	}

	/*
	 * uRateをクリア
	 */
	uRate = 0;
	snd_desc.uSample = 0;
	snd_desc.bank   = 0;


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
	Mix_CloseAudio();

}

static void AddSnd()
{
	int sp;
	int samples;

    samples = DrvBeep[0].GetSamples();
    samples -= uSample;
	sp = (uRate / 25);
	sp *= dwSoundTotal;
	sp /= 40000;

	/*
	 * uSampleと比較、一致していれば何もしない
	 */
	if (sp <= (int) uSample) {
	    return ;
	}
	/*
	 * uSampleとの差が今回生成するサンプル数
	 */
	sp -= (int) (uSample);

	/*
	 * samplesよりも小さければ合格
	 */
	if (sp <= samples) {
	    samples = sp;
	}

	uSample += samples;


}

void
wav_notify(BYTE no)
{
        int    i;
        int    j;



/*
 * サウンド合成
 */
//        AddSnd(FALSE, FALSE);
        if (no == SOUND_STOP) {
                /* 停止 */
                for (i=0; i<WAV_SLOT; i++) {
                	for(j = 0; j<SND_BUF ; j++){
                		DrvWav[i][j].Enable(FALSE);
                	}
                }

        }
        else {
        	// レンダリング
        	for(j = 0; j<SND_BUF; j++) {
        		if(DrvWav[no][j].isPlay() == FALSE) {
        			DrvWav[no][j].Render();
        		}
        }
}
}


}

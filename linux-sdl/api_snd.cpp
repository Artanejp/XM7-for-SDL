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

static void memcpy_add16(Uint8 *out,Uint8 in,int  size)
{
	int16 *p = (int16 *)out;
	int16 *q = (int16 *)in;
	int i,j;
	for(j = 0; j < (size - 8); j+=8){
			*p++ += *q++;
			*p++ += *q++;
			*p++ += *q++;
			*p++ += *q++;
	}
	for(i = j - 8; i < size; i++){
			*p++ += *q++;
	}

}

static int WavCaptureSub(Uint8 *out)
{
	int bytes;
	int w,tmp;
	Mix_Chunk *p;
	int16 *q = (int16 *)out;
	int len;
	int slots = 0;
	int i,j;


	if(!bWavCapture) return 0;
	if(out == NULL) return 0;
	if(DrvBeep[slot] != NULL) {
		p = DrvBeep[slot].GetChunk();
		len = p->alen / sizeof(int16);
		memcpy_add16(out, p->abuf, len);
		slots++;
	}
	if(DrvWav[0][slot] != NULL) {
		p = DrvWav[0][slot].GetChunk();
		len = p->alen / sizeof(int16);
		memcpy_add16(out, p->abuf, len);
		slots++;
	}
	if(DrvWav[1][slot] != NULL) {
		p = DrvWav[1][slot].GetChunk();
		len = p->alen / sizeof(int16);
		memcpy_add16(out, p->abuf, len);
		slots++;
	}
	if(DrvWav[2][slot] != NULL) {
		p = DrvWav[2][slot].GetChunk();
		len = p->alen / sizeof(int16);
		memcpy_add16(out, p->abuf, len);
		slots++;
	}
	if(slots>0) {
		for(j = 0; j < (size - 8); j+=8){
			*p += *p / slots;
			p++;
			*p += *p / slots;
			p++;
			*p += *p / slots;
			p++;
			*p += *p / slots;
			p++;
		}
		for(i = j - 8; i < size; i++){
			*p += *p / slots;
			p++;
		}

	}
	return len;
}

static int
RenderThreadSub(int start,int size,int slot)
{
	int bufsize;
	int wsize;
	int uChannels;
	int uStereo;
	int w, tmp;

	uStereo = uStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
	bufsize = ((nSampleRate*  nSoundBuffer) / 2 ) / 1000;
	if((start + size) > bufsize) { /* 大きすぎるのでサイズ切り詰める */
		wsize = (start + size) - bufsize;
	} else {
		wsize = size;
	}
	if(slot > SND_BUF) return 0;
	/* レンダリング */
	/* BEEP */
	if(DrvBeep[slot] != NULL) {
		w = DrvBeep[slot].Render(start, wsize, FALSE);
	}
	if(DrvWav[0][slot] != NULL) {
		tmp = DrvWav[0][slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}
	if(DrvWav[1][slot] != NULL) {
		tmp = DrvWav[1][slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}
	if(DrvWav[2][slot] != NULL) {
		tmp = DrvWav[2][slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}

#if 0
	if(bTapeMon) {
		if(DrvCmt[slot] == NULL) break;
		tmp = DrvCMT[slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}

	if(DrvOpn[slot] != NULL) {
		tmp =DrvOpn[slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}

	if(DrvPsg[slot] != NULL) {
		tmp = DrvPsg[slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}


	if(whg_use) {
		if(DrvWhg[slot] == NULL) break;
		tmp = DrvWhg[slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}
	if(thg_use) {
		if(DrvThg[slot] == NULL) break;
		tmp = DrvThg[slot].Render(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}
#endif
	return wsize;
}

static int
RenderThreadBZero(int start,int size,int slot)
{
	int bufsize;
	int wsize;
	int uChannels;
	int uStereo;
	int w, tmp;

	uStereo = uStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
	bufsize = ((nSampleRate*  nSoundBuffer) / 2 ) / 1000;
	if((start + size) > bufsize) { /* 大きすぎるのでサイズ切り詰める */
		wsize = (start + size) - bufsize;
	} else {
		wsize = size;
	}
	if(slot > SND_BUF) return 0;
	/* レンダリング */
	/* BEEP */
	if(DrvBeep[slot] != NULL) {
		w = DrvBeep[slot].BZero(start, wsize);
	}
	if(DrvWav[0][slot] != NULL) {
		tmp = DrvWav[0][slot].BZero(start, wsize);
		if(tmp < w) w = tmp;
	}
	if(DrvWav[1][slot] != NULL) {
		tmp = DrvWav[1][slot].BZero(start, wsize);
		if(tmp < w) w = tmp;
	}
	if(DrvWav[2][slot] != NULL) {
		tmp = DrvWav[2][slot].BZero(start, wsize);
		if(tmp < w) w = tmp;
	}

#if 0
	if(bTapeMon) {
		if(DrvCmt[slot] == NULL) break;
		tmp = DrvCMT[slot].BZero(start, wsize);
		if(tmp < w) w = tmp;
	}

	if(DrvOpn[slot] != NULL) {
		tmp =DrvOpn[slot].BZero(start, wsize);
		if(tmp < w) w = tmp;
	}

	if(DrvPsg[slot] != NULL) {
		tmp = DrvPsg[slot].Render(BZero, wsize);
		if(tmp < w) w = tmp;
	}


	if(whg_use) {
		if(DrvWhg[slot] == NULL) break;
		tmp = DrvWhg[slot].BZero(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}
	if(thg_use) {
		if(DrvThg[slot] == NULL) break;
		tmp = DrvThg[slot].BZero(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}
#endif
	return wsize;
}

static void RenderPlay(int slot, BOOL play)
{

	if(play) {
		if(DrvBeep[slot] != NULL) {
			Mix_PlayChannel(0 + slot * 10, DrvBeep[slot].GetChunk(), 0);
		}
		if(DrvWav[0][slot] != NULL) {
			Mix_PlayChannel(1 + slot * 10, DrvWav[0][slot].GetChunk(), 0);
		}
		if(DrvWav[1][slot] != NULL) {
			Mix_PlayChannel(2 + slot * 10, DrvWav[1][slot].GetChunk(), 0);
		}
		if(DrvWav[2][slot] != NULL) {
			Mix_PlayChannel(3 + slot * 10, DrvWav[2][slot].GetChunk(), 0);
		}
#if 0
		if(DrvPsg[slot] != NULL) {
			if(bTapeMon) {
				Mix_PlayChannel(0 + slot * 10, DrvPsg[slot].GetChunk(), 0);
			}
		}
		if(DrvOpn[slot] != NULL) {
			Mix_PlayChannel(0 + slot * 10, DrvOpn[slot].GetChunk(), 0);
		}
		if(DrvWhg[slot] != NULL) {
			if(whg_use) {
				Mix_PlayChannel(0 + slot * 10, DrvWhg[slot].GetChunk(), 0);
			}
		}
		if(DrvThg[slot] != NULL) {
			if(thg_use) {
				Mix_PlayChannel(0 + slot * 10, DrvThg[slot].GetChunk(), 0);
			}
		}
#endif
	}

}

/*
 * bfill
 */
static void RenderThread(void)
{
	int i,j;
	int samples;
	int uSample;
	int uChannels;
	BOOL bFill;
	int uPtr = 0;
	int uSample = 0;
	int wsamples;
	int cmd;
	int wbank,wbankOld;
	int totalSamples;

	wbank = 0;
	wbankOld = 0;

	while(1){
		/*
		 * メッセージキュー取り込み、判断
		 */
		// Wait(MessageQ);
		// Get(MessageQ);
		switch(cmd) {
		case SND_RENDER:
		case SND_BZERO: /* ARG: bFill, uSample */

		/*
		 * レンダリング: bFill = TRUEで音声出力
		 */
			if ((uStereo > 0) || bForceStereo) {
				uChannels = 2;
			} else {
				uChannels = 1;
			}
			samples = totalSamples;
			samples -= uSample;
			/*
			 * 時間経過から求めた理論サンプル数
			 */
			/*
			 * 計算結果がオーバーフローする問題に対策
			 * 2002/11/25
			 */
			if(!bFill) {
				i = (uRate / 25);
				i *= dwSoundTotal;
				i /= 40000;
				/*
				 * uSampleと比較、一致していれば何もしない
				 */
				if (i <= (int) uSample) {
					continue ;
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
			if(cmd != SND_BZERO){
				wsamples = RenderThreadSub(uSample, samples, wbank);
			} else {
				wsamples = RenderThreadBZero(uSample, samples, wbank);
			}
			if(bFill) {
				wbankOld = wbank;
				wbank = (wbank==0)?1:0;
//				WavCaptureSub(Uint8 *out); /* ここでWAV書き込みやる */
				RenderPlay(wbankOld, TRUE);
				uSample = 0;
			} else {
				uSample += sample;
			}
			break;
		case SND_SETUP:
			/*
			 * SETUP : ARG=SAMPLES;
			 */
			break;
		case SND_SHUTDOWN:
			/*
			 * WAV書き込みバッファのクリーンアップ
			 */
			/*
			 * 演奏のクリーンアップ
			 */
			/*
			 * スレッド終了(その他諸々の資源解放後)
			 */
			// return;
			break;
		defalt:
			break;
		}
	}
}

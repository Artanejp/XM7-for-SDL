/*
 * snd_api2.cpp
 *
 *  Created on: 2010/09/26-> 2011/05/14
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */

#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else				/* */
#  ifdef _WINDOWS
#  else
#    include <linux/soundcard.h>
#  endif
#endif				/* */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#ifndef _WINDOWS
#include <sys/ioctl.h>
#endif

#include <sys/time.h>
#include <errno.h>
#include <SDL/SDL.h>
//#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "opn.h"
#include "tapelp.h"
#include "xm7_sdl.h"
#include "sdl_sch.h"
#include "api_snd.h"

#include "api_wavwriter.h"

#include "SndDrvIF.h"
#include "SndDrvBeep.h"
#include "SndDrvWav.h"
#include "SndDrvOpn.h"
#include "SndDrvCMT.h"
//#include "util_ringbuffer.h"




/*
 *  グローバル ワーク
 */
extern "C"
{
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
BOOL                    bSoundDebug = FALSE;

extern int AddSoundBuffer(Sint16 *dst, Sint32 *opnsrc, Sint16 *beepsrc, Sint16 *cmtsrc, Sint16 *wavsrc, int samples); // バッファ転送関数

}


/*
 * 内部変数
 */

static int              nFMVol;
static int              nPSGVol;
static int              nBeepVol;
static int              nCMTVol;
static int              nWavVol;
static UINT             uChanSep;
static UINT             uStereo;     /* 出力モード */
static BOOL             bBeepFlag;      /* BEEP出力 */
static BOOL		bHQMode; /* HQモード */

static BOOL             bStreamWrap;

static Sint16 *pCaptureBuf;
static Sint16 *pSoundBuf;

static Sint16 *pOpnSndBuf;
static Sint16 *pBeepSndBuf;
static Sint16 *pCMTSndBuf;
static Sint32 *pOpnSndBuf32;
static Sint32 *pBeepSndBuf32;
static Sint32 *pCMTSndBuf32;


/*
 * OPN内部変数
 */
static int           nScale[3];      /* OPNプリスケーラ */

static BOOL bSndEnable;
static DWORD dwSndCount;
static DWORD uTick;   // バッファサイズ(時間)
static DWORD uRate;   // サンプリングレート
static DWORD uBufSize; // バッファサイズ(バイト数)
static BOOL bWavCaptureOld;

static BOOL             bTapeFlag;      /* 現在のテープ出力状態 */
static BOOL				bWavFlag; /* WAV演奏許可フラグ */
static BOOL             bSndExit;
static SDL_sem 			*applySem;
static int         nSndBank;
static int         nPlayBank;
static int         nSndPos;
static int         nSndBeforePos;
static int         nSndWritePos;
static int         nSndDevWritePos;
static int         nSndDataLen;
static int         nSamples;
static SDL_AudioSpec sAudioSpec;
static int nLastTime;
/*
 * サウンドレンダリングドライバ
 */
static SndDrvIF *DrvBeep;
static SndDrvWav *DrvWav;
static SndDrvIF *DrvPSG;
static SndDrvIF *DrvOPN;
static SndDrvIF *DrvCMT;


static const char     *WavName[] = {
		/* WAVファイル名 */
		"RELAY_ON.WAV",
		"RELAYOFF.WAV",
		"FDDSEEK.WAV",
#if 0
		"HEADUP.WAV",
		"HEADDOWN.WAV"
#endif  /* */
};


/*
 *  初期化
 */
void InitSnd(void)
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
	bBeepFlag = FALSE;      /* BEEP出力 */
	bTapeFlag = TRUE;
	bWavCapture = FALSE;
	bWavCaptureOld = FALSE;
        hWavCapture = 0;
        bSndExit = FALSE;
	bHQMode = FALSE;
	iTotalVolume = SDL_MIX_MAXVOLUME - 1;

	bSndEnable = FALSE;
	bWavFlag = FALSE;
	DrvBeep = NULL;
	DrvOPN = NULL;
	DrvPSG = NULL;
	DrvWav = NULL;
	DrvCMT = NULL;
	applySem = SDL_CreateSemaphore(1);
	uTick = 0;   // バッファサイズ(時間)
	uRate = 0;   // サンプリングレート
	uBufSize = 0; // バッファサイズ(バイト数)

	/*
	 * ボリューム初期化
	 */
	nFMVol = 0;
	nPSGVol = 0;
	nBeepVol = 0;
	nCMTVol = 0;
	nWavVol = 0;
	uChanSep = 0;
	iTotalVolume = 127;
	bBeepFlag = FALSE;      /* BEEP出力 */

	/*
	 * バッファ初期化
	 */

        pOpnSndBuf = NULL;
	pBeepSndBuf = NULL;
	pCMTSndBuf = NULL;
        pOpnSndBuf32 = NULL;
	pBeepSndBuf32 = NULL;
	pCMTSndBuf32 = NULL;
      
	pCaptureBuf = NULL;
	pSoundBuf = NULL;
        nSndPos = 0;
        nSndBeforePos = 0;
        dwSoundTotal = 0;
        nLastTime = 0;
        nSndWritePos = 0;
        nSndDevWritePos = 0;
        nSndDataLen = 0;
        nSamples = 0;
        bSoundDebug = FALSE;
}

/*
 *  クリーンアップ
 */
void CleanFDDSnd(void)
{

}

void CleanSnd(void)
{
   int i;
	/*
	 * もしもWAV取り込んでいたら、強制終了
	 */
	if(bWavCapture) {
		CloseCaptureSnd();
	}
//    Mix_CloseAudio();
   bSndExit = TRUE;
   SDL_CloseAudio();
   if(applySem) {
        SDL_SemWait(applySem);
    }
    bWavCapture = FALSE;

	   if(pOpnSndBuf != NULL) free(pOpnSndBuf);
	   pOpnSndBuf = NULL;
	   if(pBeepSndBuf != NULL) free(pBeepSndBuf);
	   pBeepSndBuf = NULL;
	   if(pCMTSndBuf != NULL) free(pCMTSndBuf);
	   pCMTSndBuf = NULL;
	   if(pOpnSndBuf32 != NULL) free(pOpnSndBuf32);
	   pOpnSndBuf32 = NULL;
	   if(pBeepSndBuf32 != NULL) free(pBeepSndBuf32);
	   pBeepSndBuf32 = NULL;
	   if(pCMTSndBuf32 != NULL) free(pCMTSndBuf32);
	   pCMTSndBuf32 = NULL;
	if(applySem) {
		SDL_DestroySemaphore(applySem);
		applySem = NULL;
	}
	if(pCaptureBuf != NULL) {
	   free(pCaptureBuf);
	   pCaptureBuf = NULL;
	}

	/*
	 * ドライバの抹消(念の為)
	 */


    if(DrvOPN) {
		delete DrvOPN;
		DrvOPN = NULL;
	}
	if(DrvCMT) {
		delete DrvCMT;
		DrvCMT = NULL;
	}
	if(DrvBeep) {
		delete DrvBeep;
		DrvBeep = NULL;
	}
	if(DrvPSG) {
		delete DrvPSG;
		DrvPSG = NULL;
	}
	if(DrvWav) {
		delete[] DrvWav;
		DrvWav = NULL;
	}
        nSndPos = 0;
        nSndBeforePos = 0;
        nLastTime = 0;
        nSndWritePos = 0;
        nSndDevWritePos = 0;
        nSndDataLen = 0;
        nSamples = 0;
        bStreamWrap = FALSE;
        bSoundDebug = FALSE;
}

/*
 * バッファ関連の消去(Apply向け)
 */
static void CloseSnd(void)
{
   int i;
	if(bSndEnable) {
	   bSndExit = TRUE;
	   SDL_PauseAudio(1);
	   SDL_CloseAudio();
		/*
		 * もしもWAV取り込んでいたら、強制終了
		 */
		if(bWavCapture) {
			CloseCaptureSnd();
		}
	   bWavCapture = FALSE;
	   bSndExit = FALSE;
	   if(pOpnSndBuf != NULL) free(pOpnSndBuf);
	   pOpnSndBuf = NULL;
	   if(pBeepSndBuf != NULL) free(pBeepSndBuf);
	   pBeepSndBuf = NULL;
	   if(pCMTSndBuf != NULL) free(pCMTSndBuf);
	   pCMTSndBuf = NULL;
	   if(pOpnSndBuf32 != NULL) free(pOpnSndBuf32);
	   pOpnSndBuf32 = NULL;
	   if(pBeepSndBuf32 != NULL) free(pBeepSndBuf32);
	   pBeepSndBuf32 = NULL;
	   if(pCMTSndBuf32 != NULL) free(pCMTSndBuf32);
	   pCMTSndBuf32 = NULL;
	   if(pCaptureBuf != NULL) free(pCaptureBuf);
	   pCaptureBuf = NULL;
	   if(pSoundBuf != NULL) free(pSoundBuf);
	   pSoundBuf = NULL;
	   bSndEnable = FALSE;
	}
        dwSoundTotal = 0;
        nLastTime = dwSoundTotal;
        nSndPos = 0;
        nSndBeforePos = 0;
        nSndWritePos = 0;
        nSndDevWritePos = 0;
        nSndDataLen = 0;
        nSamples = 0;
        dwSndCount = 0;
        nSndBank = 0;
        nPlayBank = 0;
        uRate = 0;
        uBufSize = 0;
        uTick = 0;
        bStreamWrap = FALSE;

//        SDL_SemPost(applySem);
}



static void AudioCallbackSDL(void *udata, Uint8 *stream, int len)
{
   int pos;
   int blen = len;
   int len2 = 0;
   int channels = 2;
   int spos;
   struct timespec req, remain;
   Uint8 *p;
   Uint8 *s;

   req.tv_sec = 0;
   req.tv_nsec = 2000 * 1000; //  0.1ms
   
   if(len <= 0) return;
   spos = 0;
   memset(stream, 0x00, len);
   do {
       if(uBufSize <= nSndWritePos) { // Wrap
	   nSndWritePos = 0;
	}
        len2 = uBufSize - nSndWritePos;
        if(bSndExit) {
	   return;
	}
        if(len2 >= nSndDataLen) len2 = nSndDataLen;  // Okay
        if((spos + len2) >= len) {
	   len2 = len - spos;
	}
        if(applySem) SDL_SemWait(applySem);
        if((len2 > 0) && (nSndDataLen > 0)){
	   p = (Uint8 *)pSoundBuf;
	   p = &p[nSndWritePos];
	   s = &stream[spos];
	   SDL_MixAudio(s, p, len2, iTotalVolume);
	   if(bSoundDebug) printf("SND:Time=%d,Callback,nSndWritePos=%d,spos=%d,len=%d,len2=%d\n", SDL_GetTicks(), nSndWritePos, spos, len, len2);
	   nSndDataLen -= len2;
	   if(nSndDataLen <= 0) nSndDataLen = 0;
	   if(applySem) SDL_SemPost(applySem);
	} else {
	   len2 = 0;
	   if(applySem) SDL_SemPost(applySem);
	   if(spos >= len) return;
	   SDL_Delay(5); // 5ms
//	   while(nSndDataLen <= 0) {
//	   nanosleep(&req, &remain); // Wait 500uS
	   if(bSndExit) return;
//	   }
	}
        nSndWritePos += len2;
        spos += len2;
   } while(spos < len);
   
}

BOOL SelectSnd(void)
{

    int members;
    int freq;
    Uint16 format;
    int channels = 2;
    int i;

	/*
	 * パラメータを設定
	 */
   
	bHQMode = bFMHQmode;
	nFMVol = nFMVolume;
	nPSGVol = nPSGVolume;
	nCMTVol = nCMTVolume;
	nBeepVol = nBeepVolume;
	nWavVol = nWaveVolume;
	uChanSep = uChSeparation;
	uStereo = nStereoOut %4;
        if(uStereo <= 0) uStereo = 1;
	/*
	 * もしもWAV取り込んでいたら、強制終了
	 */
	if(bWavCapture) {
		CloseCaptureSnd();
	}
	bWavCapture = FALSE;
        bSndExit = FALSE;

/*
 * バッファの初期化
 */
     {
	SDL_AudioSpec desired;
	desired.freq = nSampleRate;
	desired.format = AUDIO_S16SYS;
	desired.channels = channels;
	desired.samples = (nSoundBuffer * nSampleRate) * 1 / 4000; // Add overrun buffer
//	desired.samples = (nSoundBuffer * nSampleRate) / 2000; // Add overrun buffer
//	desired.samples = 1024;
	desired.callback = AudioCallbackSDL;
	SDL_OpenAudio(&desired, &sAudioSpec);
	nSampleRate = sAudioSpec.freq;
	uRate = sAudioSpec.freq;
	nSampleRate = uRate;
//	uTick = ((sAudioSpec.samples * 1000) / uRate) * 2;
	uTick = nSoundBuffer;
     }

	dwSndCount = 0;
	uBufSize = (uRate * uTick) / 1000  * channels * sizeof(Sint16);
#ifndef _WINDOWS
        if(posix_memalign((void **)&pCaptureBuf, 32, uBufSize + 31) < 0) return -1;
        if(posix_memalign((void **)&pSoundBuf, 32, uBufSize + 31) < 0) {
	   free(pCaptureBuf);
	   return -1;
	}
#else
	{
	  int size = (((uBufSize * 2) + 31) / 32) * 32;
	   pCaptureBuf = malloc(size);
	   if(pCaptureBuf == NULL) return -1;
	   pSoundBuf = malloc(size);
	   if(pSoundBuf == NULL) {
              free(pCaptureBuf);
	      return -1;
	   }
	}

#endif

        bSndEnable = TRUE;
	members = uBufSize;
        if(posix_memalign((void **)&pOpnSndBuf32, 32, members * 2) < 0) pOpnSndBuf32 = NULL;
        if(posix_memalign((void **)&pOpnSndBuf, 32, members) < 0) pOpnSndBuf = NULL;
        if(posix_memalign((void **)&pBeepSndBuf, 32, members) < 0) pBeepSndBuf = NULL;
        if(posix_memalign((void **)&pCMTSndBuf, 32, members) < 0) pCMTSndBuf = NULL;

	/*
	 * レンダリングドライバの設定
	 */
    if(DrvOPN == NULL) {
        DrvOPN = new SndDrvOpn ;
        if(DrvOPN) {
            DrvOPN->SetRate(uRate);
            DrvOPN->Setup(uTick);
        }
    }
    if(DrvBeep == NULL) {
	   DrvBeep = new SndDrvBeep ;
	   if(DrvBeep) {
            DrvBeep->SetRate(uRate);
            DrvBeep->Setup(uTick);
	   }
	}
    if(DrvWav == NULL) {
	   DrvWav = new SndDrvWav[WAV_CHANNELS] ;
	}
    if(DrvCMT == NULL) {
	   DrvCMT= new SndDrvCMT ;
	   if(DrvCMT) {
            DrvCMT->SetRate(uRate);
            DrvCMT->Setup(uTick);
        }
	}
	if(DrvOPN) {
	   int i;
		DrvOPN->SetRate(uRate);
		DrvOPN->Enable(TRUE);
		DrvOPN->SetRenderVolume(OPN_STD, nFMVolume, nPSGVolume);
		DrvOPN->SetRenderVolume(OPN_WHG, nFMVolume, nPSGVolume);
		DrvOPN->SetRenderVolume(OPN_THG, nFMVolume, nPSGVolume);
	        DrvOPN->SetReg(OPN_STD, 0x27, 0);
	        DrvOPN->SetReg(OPN_WHG, 0x27, 0);
	        DrvOPN->SetReg(OPN_THG, 0x27, 0);
	        for(i = 0; i < 3; i++) nScale[i] = 0;
	        opn_notify(0x27, 0);
	        whg_notify(0x27, 0);
	        thg_notify(0x27, 0);
	        for(i = 0; i < 3 ; i++) DrvOPN->SetReg(i, opn_reg[i]);
	}
	if(DrvBeep) {
		DrvBeep->SetRate(uRate);
		DrvBeep->Enable(FALSE);
	    DrvBeep->SetRenderVolume(nBeepVolume);
	}
	if(DrvCMT) {
		DrvCMT->SetRate(uRate);
		DrvCMT->Enable(TRUE);
	    DrvCMT->SetRenderVolume(nCMTVolume);
	}
	if(DrvWav) {
	   char WavPath[MAXPATHLEN+1];
	   int i;
	   for(i = 0; i < 3; i++) {
		strcpy(WavPath, RSSDIR);
	        strcat(WavPath, WavName[i]);
		DrvWav[i].Setup(WavPath);
		DrvWav[i].Enable(1);
	   }
	   
	}
        dwSoundTotal = 0;
        nLastTime = dwSoundTotal;
        nSndPos = 0;
        nSndBeforePos = 0;
        nSndWritePos = 0;
        nSndDevWritePos = 0;
        nSndDataLen = 0;
        nSamples = 0;
        dwSndCount = 0;
        nSndBank = 0;
        nPlayBank = 0;
        bStreamWrap = FALSE;
//        bSoundDebug = TRUE;
        if(bSoundDebug) printf("SND:Time=%d,SelectSnd,uTick=%d,uRate=%d,uBufSize=%d,SDL_Buffer=%d\n", SDL_GetTicks(), uTick, uRate, uBufSize, sAudioSpec.samples);
        SDL_PauseAudio(0);
	return TRUE;
}

/*
 *  適用
 */
void ApplySnd(void)
{
	/*
	 * パラメータ一致チェック
	 */
	if ((uRate == nSampleRate) && (uTick == nSoundBuffer) &&
			(bHQMode == bFMHQmode) ) {
		return;
	}
	SDL_SemWait(applySem);
	/*
	 * 既に準備ができているなら、解放
	 */
	if (uRate != 0) {
	   CloseSnd();
	} else {
	   SDL_SemPost(applySem);
	   return;
	}


	/*
	 * 再セレクト
	 */
	SelectSnd();
        SDL_SemPost(applySem);

	// BEEPについて、SelectSnd()し直しても音声継続するようにする
	bBeepFlag = !bBeepFlag;
	beep_notify();
	tape_notify(!bTapeFlag);
//    opn_notify(0xff, 0);
//    thg_notify(0xff, 0);
//    whg_notify(0xff, 0);

}

static struct WavDesc *WavDescCapture; // 取り込みバッファ
/*
 *  WAVキャプチャ開始
 */
void OpenCaptureSnd(char *fname)
{
   	if(bWavCapture) {
	   CloseCaptureSnd();
	   bWavCapture=FALSE;
	}
	WavDescCapture = StartWavWrite(fname, uRate);
	if(WavDescCapture == NULL) {
	   printf("Error opening WAV\n");
	   return;
	}
        if(bSoundDebug) printf("SND:Time=%d,StartedSoundCapture,file=%s\n", SDL_GetTicks(), fname);
	bWavCapture = TRUE;    /* WAVキャプチャ開始 */

}

void CloseCaptureSnd(void)
{
//   if(WavSem == NULL) return;
//   SDL_SemWait(WavSem);
   if(WavDescCapture != NULL) {
      printf("DBG:End Wav Write\n");
      bWavCapture = FALSE;
      EndWriteWavData(WavDescCapture);
      WavDescCapture = NULL;
   }

//   SDL_SemPost(WavSem);
}


/*
 * ボリューム設定: XM7/Win32 v3.4L30より
 */
void SetSoundVolume(void)
{
	int i;

	SDL_SemWait(applySem);
	/* FM音源/PSGボリューム設定 */
	if(DrvOPN) {
			DrvOPN->SetRenderVolume(OPN_STD, nFMVol, nPSGVol);
			DrvOPN->SetRenderVolume(OPN_WHG, nFMVol, nPSGVol);
			DrvOPN->SetRenderVolume(OPN_THG, nFMVol, nPSGVol);
	}

	/* BEEP音/CMT音/各種効果音ボリューム設定 */
	if(DrvBeep) {
			DrvBeep->SetRenderVolume(nBeepVol);
	}
	if(DrvCMT) {
			DrvCMT->SetRenderVolume(nCMTVol);
	}
	if(DrvWav) {
		for(i = 0; i < 3; i++) {
				DrvWav[i].SetRenderVolume(nWavVol);
			}
	}
	if(DrvOPN) DrvOPN->SetLRVolume();
	SDL_SemPost(applySem);
}

/*
 *	ボリューム設定2(設定ダイアログ用)
 */
void  SetSoundVolume2(UINT uSp, int nFM, int nPSG,
		int nBeep, int nCMT, int nWav)
{

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
	if(DrvOPN == NULL) {
		return;
	}
	SetSoundVolume();
}

void SetTotalVolume(int vol)
{
   if(vol < 0) vol = 0;
   if(vol > 128) vol = 127;
   iTotalVolume = vol;
}

/*
 *  演奏開始
 */
void PlaySnd()
{
   return;
}
/*
 *  演奏停止
 */
void StopSnd(void)
{

}

static BOOL SndWavWrite(struct WavDesc *h, Sint16 *src, int len, int channels);

static void AddSnd(int pos, int samples, bool bZero)
{
   int samples2;
   int rpos;
   int channels = 2;
   
   rpos = pos + nSndBank * (uBufSize / (2 * sizeof(Sint16) * channels));

   while(samples > 0) {
      if((nSndDevWritePos + samples) >= (uBufSize / sizeof(Sint16))) { // Wrap 
	 samples2 = (uBufSize / sizeof(Sint16)) - nSndDevWritePos;
      } else {
	 samples2 = samples;
      }
      memset(&pSoundBuf[nSndDevWritePos], 0x00, samples2 * sizeof(Sint16));
      
      if(!bZero) AddSoundBuffer(&pSoundBuf[nSndDevWritePos], &pOpnSndBuf32[rpos], &pBeepSndBuf[rpos], &pCMTSndBuf[rpos], NULL, samples2);
      if(bWavCapture) SndWavWrite(WavDescCapture, &pSoundBuf[nSndDevWritePos], samples2 / 2, 2);
      
      if(bSoundDebug) printf("SND:Time=%d,AddSnd,bank=%d,rpos=%d,nSndDevWritePos=%d,samples2=%d\n", SDL_GetTicks(), nSndBank, rpos, nSndDevWritePos,samples2);
      
      nSndDevWritePos += samples2;
      //	      SDL_LockAudio();
      nSndDataLen = nSndDataLen + samples2 * sizeof(Sint16);
      if(nSndDevWritePos >= (uBufSize / sizeof(Sint16))) {
	 //		   nSndDevWritePos -= (uBufSize / sizeof(Sint16));
	 nSndDevWritePos = 0; // Wrap
      }
      if(nSndDataLen >= uBufSize) {
	 nSndDataLen = uBufSize; // Overflow
	 //		   SDL_UnlockAudio();
	 break;
      }
      //	      SDL_UnlockAudio();
      rpos += samples2;
      samples -= samples2;
   }
}


/*
 * Rendering 1:
 * Normal Type
 */
static int RenderSub(Sint32 *p32, Sint16 *p, SndDrvIF *drv, int samples, BOOL bZero)
{
   int j;
//   if(p == NULL) return 0;
//   if(p32 == NULL) return 0;
   if(drv == NULL) return 0;
   if(samples <= 0) return 0;

   j = drv->Render(p32, p, nSndPos , samples,  FALSE, bZero);
   return j;
}

// RenderCommon: Render snd devices:
static DWORD RenderCommon(DWORD ttime, int samples, BOOL bZero)
{
   DWORD max = 0;
   DWORD n;
   int channels = 2;
   int wpos;
   
   wpos =  0 + nSndBank * (uBufSize / (2 * sizeof(Sint16) * channels));
   if(samples <= 0) goto _end;
   max = RenderSub(&pOpnSndBuf32[wpos],  &pOpnSndBuf[wpos], DrvOPN, samples, bZero);
   n = RenderSub(&pBeepSndBuf32[wpos], &pBeepSndBuf[wpos], DrvBeep, samples, bZero);
   if(n > max) max = n;
   n = RenderSub(&pCMTSndBuf32[wpos],  &pCMTSndBuf[wpos], DrvCMT, samples, bZero);
   if(n > max) max = n;
   nSamples += max;
   AddSnd(nSndPos * 2, max * 2, bZero);
   nSndPos += max;
   if(nSndPos >= (uBufSize / (2 * channels * sizeof(Sint16)))) {
      nSndPos -= (uBufSize / (2 * channels * sizeof(Sint16)));
//      nSndPos = 0;
   }
//   if(nSamples >= uBufSize / (2 * channels * sizeof(Sint16))) nSamples = uBufSize / (2 * channels * sizeof(Sint16));
_end:
   nLastTime = ttime;
   return max;
}

// Render2: Render snd devices: Addsnd(TRUE, bZero).
static DWORD Render2(DWORD ttime, BOOL bZero)
{
   DWORD max = 0;
   int samples;
   int channels = 2;
   int i;

//   samples = (dwSndCount * uRate) / 1000;
   samples = ((uRate * uTick) / 1000 ) / 2;
//   samples = uBufSize / (2 * channels * sizeof(Sint16));
   samples -= nSamples;
   if(samples <= 0) return 0;
   if(applySem) SDL_SemWait(applySem);
   max = RenderCommon(ttime, samples, bZero);
   
//   if(DrvOPN  != NULL) DrvOPN->ResetRenderCounter();
//   if(DrvBeep != NULL) DrvBeep->ResetRenderCounter();
//   if(DrvCMT  != NULL) DrvCMT->ResetRenderCounter();
//   printf("Render2: Bank = %d , Before = %d,  Pos = %d, samples = %d : Total = %d\n",nSndBank, nSndBeforePos, nSndPos, samples, nSamples);
   if(bSoundDebug)printf("SND:Time=%d,Render2,ttime=%d,samples=%d,bank=%d\n", SDL_GetTicks(), ttime, samples, nSndBank);
   if(applySem) SDL_SemPost(applySem);
   return max;
}


// Render1: Render snd devices: Addsnd(FALSE, bZero).
static DWORD Render1(DWORD ttime, BOOL bZero)
{
   DWORD max = 0;
   DWORD n;
   int samples;
   int i;
   int channels = 2;
   
   samples = ((uRate * uTick) / 1000 ) / 2;
//   samples = uBufSize / (2 * channels * sizeof(Sint16));
   samples -= nSamples;
   if(samples <= 0) return 0;
   /* !bFillなら、時間から計測 */
   if(1){
	/* 時間経過から求めた理論サンプル数 */
	/* 計算結果がオーバーフローする問題に対策 2002/11/25 */
	i = (uRate / 25);
	i *= ttime;
	i /= 40000;
	
	/* uSampleと比較、一致していれば何もしない */
	if (i <= (int)nSndPos) 
	{
	   return 0;
	}
	/* uSampleとの差が今回生成するサンプル数 */
	i -= (int)(nSndPos);
	
	/* samplesよりも小さければ合格 */
	if (i <= samples) 
	{
	   samples = i;
	}
   }
   if(samples <= 0) return 0;
   if(applySem) SDL_SemWait(applySem);
   max = RenderCommon(ttime, samples, bZero);
   if(bSoundDebug)printf("SND:Time=%d,Render1,ttime=%d,samples=%d,bank=%d\n", SDL_GetTicks(), ttime, samples, nSndBank);
   if(applySem) SDL_SemPost(applySem);
   return max;
}





/*
 * XXX Notify系関数…VM上の仮想デバイスに変化があったとき呼び出される
 */

static void OpnNotifySub(BYTE reg, BYTE dat, SndDrvIF *sdrv, int opnch)
{
   DWORD ttime = dwSoundTotal;
   int samples;
    BYTE r;

    if(sdrv == NULL) return;
    if((opnch != OPN_STD) && (opnch != OPN_WHG) && (opnch != OPN_THG)) return;
	if (opn_scale[opnch] != nScale[opnch]) {
		nScale[opnch] = opn_scale[opnch];
		switch (opn_scale[opnch]) {
		case 2:
		   sdrv->SetReg(opnch, 0x2f, 0);
		   break;
		case 3:
		   sdrv->SetReg(opnch, 0x2e, 0);
		   break;
		case 6:
		   sdrv->SetReg(opnch, 0x2d, 0);
		   break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (sdrv->GetCh3Mode(opnch) == dat) {
			return;
		}
		sdrv->SetCh3Mode(opnch, dat);
	}

	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = sdrv->GetReg(opnch, 0x27);
		if ((r & 0xc0) != 0x80) {
			return;
		}
	}

	/*
	 * サウンド合成
	 */
        Render1(ttime, FALSE);

	/*
	 * 出力
	 */
    sdrv->SetReg(opnch, (uint8) reg, (uint8) dat);
}


#ifdef __cplusplus
extern "C" {
#endif



void opn_notify(BYTE reg, BYTE dat)
{
    OpnNotifySub(reg, dat, DrvOPN, OPN_STD);
}

void thg_notify(BYTE reg, BYTE dat)
{
    OpnNotifySub(reg, dat, DrvOPN, OPN_THG);
}

void whg_notify(BYTE reg, BYTE dat)
{
    OpnNotifySub(reg, dat, DrvOPN, OPN_WHG);
}


void wav_notify(BYTE no)
{

   if(DrvWav == NULL) return;
   switch(no) 
     {
      case SOUND_CMTMOTORON:
//	DrvWav[0].Play(CH_WAV_RELAY_ON, 0);
	break;
      case SOUND_CMTMOTOROFF:
//	DrvWav[1].Play(CH_WAV_RELAY_OFF, 0);
	break;
      case SOUND_FDDSEEK:
//	DrvWav[2].Play(CH_WAV_FDDSEEK, 0);
	break;
      default:
	break;
     }
   

}

void beep_notify(void)
{
	DWORD ttime = dwSoundTotal;
	int samples;

	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}
        Render1(ttime, FALSE);
	if(DrvBeep) {
//		DrvBeep->ResetCounter(!bBeepFlag);
		bBeepFlag = beep_flag & speaker_flag;
		DrvBeep->Enable(bBeepFlag);
	} else {
		bBeepFlag = beep_flag & speaker_flag;
	}
}

void tape_notify(BOOL flag)
{
	int samples;
	DWORD ttime = dwSoundTotal;

	if (bTapeFlag == flag) {
		return;
	}
	if(!DrvCMT) return;
	DrvCMT->SetState((BOOL)bTapeFlag);
	DrvCMT->Enable(bTapeMon);
        Render1(ttime, FALSE);


	bTapeFlag = flag;
}

#ifdef __cplusplus
}
#endif


/*
 * 前回レンダリングからttime迄のレンダリングサンプル数を計算する
 */
int SndCalcSamples(struct SndBufType *p, DWORD ttime)
{
	uint64_t time2;
	uint64_t last;
	uint64_t diff;
	int samples;

//	if(p == NULL) return 0;
	time2 =(uint64_t) ttime;
//	last = (uint64_t) p->nLastTime;
	last = (uint64_t) nLastTime;
   
	if(time2 < last) {
		diff = (time2 + 0x100000000) - last;
	} else {
		diff = time2 - last;
	}
	samples = (int)((diff * uRate) / 1000000);
//        if(samples > p->nSize) samples = p->nSize;
	if(samples <= 0) samples = 0;
        //p->nLastTime = ttime;
	return samples;
}



/*
 * WAV書き込み
 */
static BOOL SndWavWrite(struct WavDesc *h, Sint16 *src, int len, int channels)
{
   Sint16 *wavbuf;
   int len2 = len * sizeof(Sint16) * channels;

   wavbuf = pCaptureBuf;
   if(h == NULL) return FALSE;
   if(len2 <= 0) return FALSE;
   if(!bWavCapture){
	CloseCaptureSnd();
        return FALSE;
   }

   if((wavbuf != NULL) && (src != NULL)){ 
     memset(wavbuf, 0x00, len2);
     memcpy(wavbuf, src, len2);
     WriteWavDataSint16(h, wavbuf, len * channels);
//      free(wavbuf);
      return TRUE;
   }
   return FALSE;
}

/*
 * Rendering 2:
 * Fill Type
 */



/*
 * 1msごとにスケジューラから呼び出されるhook
 */

void ProcessSnd(BOOL bZero)
{
	DWORD ttime = dwSoundTotal;
	int samples;
        int samples2;
        int bsamples = 0;
	int channels = 2;
        int rpos;
        
        int playBank;
	BOOL bWrite = FALSE;

        dwSndCount++;
        if(dwSndCount >= (uTick / 2)) {
	      bWrite = TRUE;
	}
   
   
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
		       // OPNについては不要か？必要か？
		       if(applySem) {
			  Render1(ttime, FALSE);
		       }
		   }
	   return;
    } else {
		// フラッシュする
        if(applySem) {
	   Render2(ttime, bZero);
	   SDL_SemWait(applySem);
	   //AddSnd(0, nSamples * channels, bZero);
	   if(!bWavCapture && bWavCaptureOld) {
	   	CloseCaptureSnd();
	      if(bSoundDebug) printf("SND:Time=%d,ClosedSoundCapture\n", SDL_GetTicks());
	   }
	    if(nSamples <= 0) {
	       nSamples = 0;
	       dwSndCount = 0;
	       dwSoundTotal = 0;
	       nSndDevWritePos = 0;
	       SDL_SemPost(applySem);
	       return;
	    }
	   nSndBeforePos = nSndPos;
	   nSndPos = 0;
	   nSamples = 0;
	   nSndBank = (nSndBank + 1) & 1;
	   
	   bWavCaptureOld = bWavCapture;
	   dwSndCount = 0;
	   dwSoundTotal = 0;
  	   SDL_SemPost(applySem);
        }
	return;
    }
   
}

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

#ifndef _WINDOWS
#include <sys/ioctl.h>
#endif

#include <sys/time.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
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
static BOOL		bMode; /* HQモード */


static struct SndBufType *pOpnBuf;
static struct SndBufType *pBeepBuf;
static struct SndBufType *pCMTBuf;
static Sint16 *pCaptureBuf;
static Sint16 *pSoundBuf;

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
static BOOL             bbZero;
static SDL_sem 			*applySem;
//static int         nSndBank;
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
    bbZero   = FALSE;
	bMode = FALSE;

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
	 * バッファ(概要)初期化
	 */
	pOpnBuf = InitBufferDesc();
	pBeepBuf = InitBufferDesc();
	pCMTBuf = InitBufferDesc();
	pCaptureBuf = NULL;
	pSoundBuf = NULL;
}

/*
 *  クリーンアップ
 */
void CleanFDDSnd(void)
{

}

void CleanSnd(void)
{
	/*
	 * もしもWAV取り込んでいたら、強制終了
	 */
	if(bWavCapture) {
		CloseCaptureSnd();
	}
    Mix_CloseAudio();
    if(applySem) {
        SDL_SemWait(applySem);
    }
    bWavCapture = FALSE;
    DetachBufferDesc(pOpnBuf);
	pOpnBuf = NULL;

	DetachBufferDesc(pBeepBuf);
	pBeepBuf = NULL;

	DetachBufferDesc(pCMTBuf);
	pCMTBuf = NULL;

	if(applySem) {
		SDL_DestroySemaphore(applySem);
		applySem = NULL;
	}

    bSndExit = TRUE;
//	DetachBufferDesc(pCaptureBuf);
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


}

/*
 * バッファ関連の消去(Apply向け)
 */
static void CloseSnd(void)
{
	if(bSndEnable) {
		Mix_CloseAudio();
//		SDL_CloseAudio();
		/*
		 * もしもWAV取り込んでいたら、強制終了
		 */
		if(bWavCapture) {
			CloseCaptureSnd();
		}
	   bWavCapture = FALSE;
	   bSndExit = FALSE;
		DetachBuffer(pBeepBuf);
		DetachBuffer(pCMTBuf);
		DetachBuffer(pOpnBuf);
	        if(pCaptureBuf != NULL) {
		   free(pCaptureBuf);
		   pCaptureBuf = NULL;
		}
	        if(pSoundBuf != NULL) {
		   free(pSoundBuf);
		   pSoundBuf = NULL;
		}

		bSndEnable = FALSE;
	}
}

BOOL SelectSnd(void)
{

	int members;
    int freq;
    Uint16 format;
    int channels;

	/*
	 * パラメータを設定
	 */
	uRate = nSampleRate;
	uTick = nSoundBuffer;
	bMode = bFMHQmode;
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
        bbZero   = FALSE;

/*
 * バッファの初期化
 */
	dwSndCount = 0;
	uBufSize = (nSampleRate * nSoundBuffer * 2 * sizeof(Sint16)) / 1000;
#ifndef _WINDOWS
        if(posix_memalign((void **)&pCaptureBuf, 16, uBufSize * 2) < 0) return -1;
        if(posix_memalign((void **)&pSoundBuf, 16, uBufSize * 2) < 0) {
	   free(pCaptureBuf);
	   return -1;
	}
#else
	{
	  int size = (((uBufSize * 2) +15) / 16) * 16;
	   pCaptureBuf = malloc(size);
	   if(pCaptureBuf == NULL) return -1;
	   pSoundBuf = malloc(size);
	   if(pSoundBuf == NULL) {
              free(pCaptureBuf);
	      return -1;
	   }
	}

#endif

    //Mix_QuerySpec(&freq, &format, &channels);
    if (Mix_OpenAudio(nSampleRate, AUDIO_S16SYS, 2, (nSoundBuffer * nSampleRate) / 4000)< 0) {
//    if (Mix_OpenAudio(nSampleRate, AUDIO_S16SYS, 2, uBufSize / 8 ) < 0) {
	   printf("Warning: Audio can't initialize!\n");
	   return -1;
	}
	Mix_AllocateChannels(CH_CHANNELS);
	Mix_Volume(-1,iTotalVolume);


    bSndEnable = TRUE;
	uTick = nSoundBuffer;
	members = (nSampleRate * nSoundBuffer) / 1000 * 4;
	SetupBuffer(pBeepBuf, members, TRUE, FALSE);
	SetupBuffer(pCMTBuf, members, TRUE, FALSE);
	SetupBuffer(pOpnBuf, members , TRUE, TRUE);
//        nSndBank =

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
			(bMode == bFMHQmode) ) {
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
	}
	WavDescCapture = StartWavWrite(fname, nSampleRate);
	if(WavDescCapture == NULL) printf("Error opening WAV\n");
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

/*
 * Rendering 1:
 * Normal Type
 */
static DWORD Render1(DWORD ttime, BOOL bZero)
{
   DWORD max;
   DWORD n;
   int samples;

   samples  = SndCalcSamples(pOpnBuf, ttime);

   max = RenderSub(pOpnBuf, DrvOPN, ttime, samples, bZero);
   n = RenderSub(pBeepBuf, DrvBeep, ttime, samples, bZero);
   if(n > max) max = n;
   n = RenderSub(pCMTBuf, DrvCMT, ttime, samples, bZero);
   if(n > max) max = n;
   return n;
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
	DrvWav[0].Play(CH_WAV_RELAY_ON, 0);
	break;
      case SOUND_CMTMOTOROFF:
	DrvWav[1].Play(CH_WAV_RELAY_OFF, 0);
	break;
      case SOUND_FDDSEEK:
	DrvWav[2].Play(CH_WAV_FDDSEEK, 0);
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
#if 1
        Render1(ttime, FALSE);
#endif
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

	if(p == NULL) return 0;
	time2 =(uint64_t) ttime;
	last = (uint64_t) p->nLastTime;

	if(time2 < last) {
		diff = last - time2;
	} else {
		diff = time2 - last;
	}
	samples = (int)((diff * uRate) / 1000000);
        if(samples > p->nSize) samples = p->nSize;
	//if(samples <= 0) samples = 0;
        //p->nLastTime = ttime;
	return samples;
}


/*
 * ChunkをSetする
 */
static void SetChunkSub(Mix_Chunk *p, Sint16 *buf, Uint32 len, int volume)
{
	int channels = 2;

	p->abuf = (Uint8 *)buf;
	p->alen = len * sizeof(Sint16) * channels;
//	p->alen = len * channels;
	p->allocated = 1;
	p->volume = (Uint8)volume;
//	printf("Play: buf=%08x len=%d\n", p->abuf, p->alen);
}


static int SetChunk(struct SndBufType *p, int ch)
{
    int i = p->nChunkNo;
    int j;
    int channels = 2;
    int samples;

    if(p->nWritePTR > p->nReadPTR) {
        samples = p->nReadPTR - p->nWritePTR;
    } else{
        samples = p->nReadPTR - p->nWritePTR;
    }
   if(samples == 0) return 0;
   if(samples < 0) samples += p->nSize;
   j = p->nSize - p->nReadPTR;
   if(j > samples) {
        // 分割不要
//	    printf("SND:DBG:CHUNK:%d #%d Size=%d\n", ch, i, samples);
        SetChunkSub(p->mChunk[i], &p->pBuf[p->nReadPTR * channels], samples, iTotalVolume);
        Mix_PlayChannel(ch , p->mChunk[i], 0);
        p->nReadPTR += samples;
       i++;
       if(i >= p->nChunks) i = 0;
       p->nChunkNo = i;
       if(p->nReadPTR >= p->nSize) {
	  p->nReadPTR = 0;
       }
    } else {
//	    printf("SND:DBG:CHUNK:%d #%d Size=%d\n", ch, i, j);
       SetChunkSub(p->mChunk[i], &p->pBuf[p->nReadPTR * channels], j, iTotalVolume);
       Mix_PlayChannel(ch , p->mChunk[i], 0);
       p->nReadPTR += j;
       samples -= j;
       i++;
       if(i >= p->nChunks) i = 0;
       p->nChunkNo = i;
       if(p->nReadPTR >= p->nSize) {
	  p->nReadPTR = 0;
       }
       if(samples > 0) {
	  SetChunkSub(p->mChunk[i], &p->pBuf[p->nReadPTR * channels], samples, iTotalVolume);
//       SetChunkSub(p->mChunk[i], p->pBuf, samples, iTotalVolume);

	  Mix_PlayChannel(ch , p->mChunk[i], 0);
	  p->nReadPTR += samples;
	  i++;
	  if(i >= p->nChunks) i = 0;
	  p->nChunkNo = i;
	  if(p->nReadPTR >= p->nSize) {
	     p->nReadPTR = 0;
	  }

       }
       
    }
    return i;
}


/*
 * WAV書き込み
 */
static BOOL SndWavWrite(struct WavDesc *h, int channels)
{
   Sint16 *wavbuf;
   int x, y, z;

   wavbuf = pCaptureBuf;
   if(h == NULL) return FALSE;
   if(!bWavCapture){
	CloseCaptureSnd();
        return FALSE;
   }

   if(wavbuf != NULL) {
      memset(wavbuf, 0x00, uBufSize *  2);
      x = CopyChunk(pOpnBuf, wavbuf, 0);
      y = CopyChunk(pCMTBuf, wavbuf, 0);
      z = CopyChunk(pBeepBuf, wavbuf, 0);
      if(x < y) x = y;
      if(x < z) x = z;
//      WriteWavDataSint16(h, wavbuf, x * channels);
      WriteWavDataSint16(h, wavbuf, x);
//      free(wavbuf);
      return TRUE;
   }
   return FALSE;
}

/*
 * Rendering 2:
 * Fill Type
 */

static BOOL SndFlush(DWORD ttime, DWORD ms, BOOL bZero)
{
   int chunksize;
   BOOL r;

   chunksize = (ms * uRate * 2) / 1000;
   //if(DrvOPN) chunksize -= DrvOPN->GetRenderCounter();
   if(chunksize <= 0) return TRUE;
   r = SndFlushSub(pOpnBuf, DrvOPN, ttime, bZero, chunksize);
   r &= SndFlushSub(pBeepBuf, DrvBeep, ttime, bZero, chunksize);
   r &= SndFlushSub(pCMTBuf, DrvCMT, ttime, bZero, chunksize);
   return r;
}


/*
 * 1msごとにスケジューラから呼び出されるhook
 */

void ProcessSnd(BOOL bZero)
{
	DWORD ttime = dwSoundTotal;
	int samples;
	int chunksize;
	int channels = 2;
	BOOL bWrite = FALSE;
	dwSndCount++;
	bbZero = bZero;
	if(dwSndCount >= (uTick / 2)) {
//	if(dwSndCount > (uTick / 2)) {
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
			  SDL_SemWait(applySem);
			  Render1(ttime, FALSE);
			  SDL_SemPost(applySem);
		       }
		   }
		   return;
    } else {

		// フラッシュする
        if(applySem) {
            SDL_SemWait(applySem);
//	    Render1(ttime, bZero);
	    SndFlush(ttime, dwSndCount, bZero);
		/*
		 * 演奏本体
		 * 20110524 マルチスレッドにすると却って音飛びが悪くなるのでこれでいく。
		 *          こちらの方がWAV取り込みに悪影響がでない（？？）
		 */
        if(bWavCapture){
            SndWavWrite(WavDescCapture, channels);
        } //else {
	   //if(bWavCaptureOld) {
	//	CloseCaptureSnd();
	//   }
	//}

        bWavCaptureOld = bWavCapture;
        SetChunk(pOpnBuf ,  CH_SND_OPN);
        SetChunk(pBeepBuf , CH_SND_BEEP);
        SetChunk(pCMTBuf , CH_SND_CMT);
        if(DrvOPN != NULL) DrvOPN->ResetRenderCounter();
        if(DrvBeep != NULL) DrvBeep->ResetRenderCounter();
        if(DrvCMT != NULL) DrvCMT->ResetRenderCounter();
        SDL_SemPost(applySem);
        }
//		SDL_UnlockAudio();
	dwSndCount = 0;
//        nSndBank = (nSndBank +1) & 1;

	}
}

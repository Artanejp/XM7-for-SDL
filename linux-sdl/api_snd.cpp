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
#include "tapelp.h"
#include "cisc.h"
#include "opna.h"
#include "psg.h"
#include "opn.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "sdl_snd.h"

#include "SndDrvBeep.h"
#include "SndDrvWav.h"
#include "SndDrvOpn.h"
#include "SndDrvCMT.h"
#include "util_ringbuffer.h"

#define WAV_SLOT 3
#define SND_BUF 4


enum {
	CH_SND_BEEP = 0,
	CH_SND_CMT = 1,
	CH_SND_OPN = 2,
	CH_WAV_RELAY_ON = 3,
	CH_WAV_RELAY_OFF,
	CH_WAV_FDDSEEK,
	CH_WAV_RESERVE1,
	CH_WAV_RESERVE2,
	CH_CHANNELS
};
enum {
	GROUP_SND_BEEP = 0,
	GROUP_SND_CMT ,
	GROUP_SND_OPN ,
	GROUP_SND_SFX
};

/*
 *      * アセンブラ関数のためのプロトタイプ宣言→x86依存一度外す
 */

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
 * 内部変数
 */

// ここからレガシーな変数
static int bInitFlag;
static UINT             uBufSize;       /* サウンドバッファサイズ */
static UINT             uRate;          /* 合成レート */
static UINT             uTick;          /* 半バッファサイズの長さ */
static UINT             uStereo;        /* 出力モード */
static UINT             uSample;        /* サンプルカウンタ */
static int              nFMVol;
static int              nPSGVol;
static int              nBeepVol;
static int              nCMTVol;
static int              nWavVol;
static UINT             uChanSep;

static int              nScale[3];      /* OPNプリスケーラ */
static WORD             *pWavCapture;   /* キャプチャバッファ(64KB) */
static UINT             nWavCapture;    /* キャプチャバッファ実データ */
static DWORD            dwWavCapture;   /* キャプチャファイルサイズ */
static WORD             uChannels;      /* 出力チャンネル数 */
static BOOL             bBeepFlag;      /* BEEP出力 */

static BOOL             bTapeFlag;      /* 現在のテープ出力状態 */
static int bNowBank;
static DWORD dwPlayC;
static BOOL				bWavFlag; /* WAV演奏許可フラグ */
//ここまで
static BOOL bMode;
static DWORD uProcessCount;
static SDL_sem *applySem;
static BOOL bPlayEnable;
static BOOL bSndDataEnable[4]; // 4ch


static char     *WavName[] = {
		/* WAVファイル名 */
		"RELAY_ON.WAV",
		"RELAY_OFF.WAV",
		"FDDSEEK.WAV",
#if 0
		"HEADUP.WAV",
		"HEADDOWN.WAV"
#endif  /* */
};



/*
 * サウンドレンダリングドライバ
 */
static SndDrvBeep *DrvBeep;
static SndDrvWav *DrvWav;
static SndDrvOpn *DrvPSG;
static SndDrvOpn *DrvOPN;
static SndDrvCMT *DrvCMT;

static SDL_Thread *snd_thread;
static struct RingBufferDesc *CmdRing;
static SDL_cond *SndCond;
static SDL_mutex *SndMutex;

static void RenderPlay(int samples, int slot, BOOL play);
static int RenderThreadBZero(int start,int size,int slot);
static int RenderThreadSub(int start, int size, int slot);

/*
 * 予めLoadしておいたWAVを演奏できるようにする
 */


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
	bBeepFlag = FALSE;      /* BEEP出力 */
	bTapeFlag = TRUE;

	iTotalVolume = SDL_MIX_MAXVOLUME - 1;
	snd_thread = NULL;
	SndMutex = NULL;
	SndCond = NULL;
	bMode = FALSE;
	bNowBank = 0;
	dwPlayC = 0;

	bWavFlag = FALSE;
	DrvBeep = NULL;
	DrvOPN = NULL;
	DrvPSG = NULL;
	DrvWav = NULL;
	DrvCMT = NULL;
	applySem = NULL;
	bPlayEnable = FALSE;

	snd_thread = NULL;
	CmdRing = NULL;

	uClipCount = 0;
	//	bInitFlag = FALSE;
	//    InitFDDSnd();
	//SDL_InitSubSystem(SDL_INIT_AUDIO);
	/*
	 * WAVよむ
	 */
	for(i = 0; i < 4 ; i++) {
		bSndDataEnable[i] = FALSE; // 4ch
	}

}

/*
 *  クリーンアップ
 */
void
CleanFDDSnd(void)
{

}

void
CleanSnd(void)
{
	/*
	 * サウンド停止
	 */
	StopSnd();
	/*
	 * スレッド停止
	 */
	SDL_SemWait(applySem);
	SDL_DestroySemaphore(applySem);
	applySem = NULL;
	/*
	 * スレッド資源解放待ち
	 */
	/*
	 * OPNを解放
	 */
	/*
	 * サウンド作成バッファを解放
	 */
#if 0
	if(snd_thread != NULL) {
		SDL_WaitThread(snd_thread, &i);
		snd_thread = NULL;
	}

//	Mix_CloseAudio();
	if(SndCond != NULL) {
		SDL_DestroyCond(SndCond);
		SndCond = NULL;
	}
	if(SndMutex != NULL) {
		SDL_DestroyMutex(SndMutex);
		SndMutex = NULL;
	}
#endif
#if 1				/* WAVキャプチャは後で作る */
	/*
	 * キャプチャ関連
	 */
	if (hWavCapture >= 0) {
		CloseCaptureSnd();
	}
	//	if (pWavCapture) {
	//		free(pWavCapture);
	//		pWavCapture = NULL;
	//	}
	hWavCapture = -1;
	bWavCapture = FALSE;
#endif				/* */

	if(DrvBeep) 	{
		delete DrvBeep;
		DrvBeep = NULL;
	}

	if(DrvWav) {
			delete [] DrvWav;
			DrvWav = NULL;
	}
	if(DrvOPN)		{
		delete DrvOPN;
		DrvOPN = NULL;
	}
	if(DrvCMT) 		{
		delete DrvCMT;
		DrvCMT = NULL;
	}

	bWavFlag = FALSE;
	bPlayEnable = FALSE;
	Mix_CloseAudio();

//	DeleteCommandBuffer();
	/*
	 * uRateをクリア
	 */
	uRate = 0;
}

//static int RenderThread(void *arg);

BOOL SelectSnd(void)
{
	int                 i,j;
	char prefix[MAXPATHLEN];
	int bytes;

	/*
	 * 起動フラグ立てる
	 */
	bInitFlag = TRUE;

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

//	if ((uStereo > 0) || bForceStereo) {
//		uChannels = 2;
//	} else {
//		uChannels = 1;
//	}
	uChannels = 2;
	dwPlayC = 0;
	bNowBank = 0;

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
	 * サウンドバッファを作成(DSP用バッファの半分の時間で、DWORD)
	 */
    bytes = (uRate * sizeof(WORD) * uChannels * uTick) / 1000;
    bytes += (DWORD) 7;
    bytes &= (DWORD) 0xfffffff8;        /* 8バイト境界 */
    uBufSize = bytes;

	/*
	 * サンプルカウンタ、サウンド時間をクリア
	 */
	uSample = 0;
	dwSoundTotal = 0;
	uClipCount = 0;

	for(i = 0; i < 4 ; i++) {
		bSndDataEnable[i] = FALSE; // 4ch
	}
	if(applySem == NULL) {
		applySem = SDL_CreateSemaphore(1);
		SDL_SemPost(applySem);
	}
	if (Mix_OpenAudio(uRate, AUDIO_S16SYS, 2, uBufSize / 16 ) < 0) {
//		if (Mix_OpenAudio(uRate, AUDIO_S16SYS, 2, 512 ) < 0) {
	   printf("Warning: Audio can't initialize!\n");
		return FALSE;
	}
	Mix_AllocateChannels(CH_CHANNELS);
	/*
	 *
	 */
	Mix_GroupChannels(CH_WAV_RELAY_ON, CH_WAV_RESERVE2, GROUP_SND_SFX);
	Mix_Volume(-1,iTotalVolume);


	DrvBeep= new SndDrvBeep;
	if(DrvBeep) {
			DrvBeep->Setup(uTick);
	}



	/*
	 * OPNデバイス(標準)を作成
	 */
	DrvOPN= new SndDrvOpn ;
	if(DrvOPN) {
			DrvOPN->Setup(uTick);
			DrvOPN->Enable(TRUE);
	}
	/*
	 * CMT
	 */
	DrvCMT = new SndDrvCMT;
	if(DrvCMT) {
		DrvCMT->Setup(uTick);
		DrvCMT->Enable(TRUE);
		DrvCMT->SetState(FALSE);
	}
	/*
	 * 再セレクトに備え、レジスタ設定
	 */
	nScale[0] = 0;
	nScale[1] = 0;
	nScale[2] = 0;
	opn_notify(0x27, 0);
	whg_notify(0x27, 0);
	thg_notify(0x27, 0);
	if(DrvOPN) {
		DrvOPN->SetReg(OPN_STD, opn_reg[OPN_STD]);
		DrvOPN->SetReg(OPN_WHG, opn_reg[OPN_WHG]);
		DrvOPN->SetReg(OPN_THG, opn_reg[OPN_THG]);
	}

	/*
	 * キャプチャ関連
	 */
	if (!pWavCapture) {
		pWavCapture = (WORD *) malloc(sizeof(WORD) * 0x8000);
	}
	ASSERT(hWavCapture == -1);
	ASSERT(!bWavCapture);

	/*
	 * SDL用に仕様変更…出来ればInitSnd()でやったら後はOpen/Closeしたくないんぼだけれどー。
	 */

	bPlayEnable = TRUE;
    DrvWav = new SndDrvWav[3];
    if(!DrvWav) return FALSE;
	for(i = 0; i < WAV_SLOT; i++) {
		   strcpy(prefix, ModuleDir);
	       strcat(prefix, WavName[i]);
//	       WavChunk[i] = Mix_LoadWAV(prefix);
	       DrvWav[i].Setup(prefix);
	}
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
	/* 音声プロパティとOPNが衝突しないようにするためのセマフォ初期化 */
	    SDL_SemWait(applySem);
	/*
	 * 既に準備ができているなら、解放
	 */
	if (uRate != 0) {
		Mix_HaltChannel(-1);
		CleanSnd();
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
}


void        PlaySnd(void);


/*
 *  WAVキャプチャ開始
 */
void
OpenCaptureSnd(char *fname)
{

}

void
CloseCaptureSnd(void)
{

}

/*
 * ボリューム設定: XM7/Win32 v3.4L30より
 */
void SetSoundVolume(void)
{
	int i;
	int j;

	SDL_SemWait(applySem);
	/* FM音源/PSGボリューム設定 */
	if(DrvOPN) {
			DrvOPN->SetRenderVolume(OPN_STD, nFMVolume, nPSGVolume);
			DrvOPN->SetRenderVolume(OPN_WHG, nFMVolume, nPSGVolume);
			DrvOPN->SetRenderVolume(OPN_THG, nFMVolume, nPSGVolume);
	}

	/* BEEP音/CMT音/各種効果音ボリューム設定 */
	if(DrvBeep) {
			DrvBeep->SetRenderVolume(nBeepVolume);
	}
	if(DrvCMT) {
			DrvCMT->SetRenderVolume(nCMTVolume);
	}
	if(DrvWav) {
		for(i = 0; i < 3; i++) {
				DrvWav[i].SetRenderVolume(nWaveVolume);
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

/*
 *  演奏開始
 */
void
PlaySnd()
{
   return;
	/*
	 * サンプルカウンタ、サウンド時間をクリア
	 */
	uSample = 0;
	dwSoundTotal = 0;
	uClipCount = 0;
	dwPlayC = 0;
	uProcessCount = 0;

	/*
	 * 変換できたので読み込む
	 */
}
/*
 *  演奏停止
 */
void
StopSnd(void)
{
}

static void AddSnd(BOOL bfill, BOOL bZero)
{
	int samples;
	int i;
	int wbank;

	/*
	 * レンダリング: bFill = TRUEで音声出力
	 */
	samples = (uBufSize >>2) / uChannels;
//	samples = ((uTick  * uRate) / 2) / 1000;  
	samples -= uSample;
	/*
	 * 時間経過から求めた理論サンプル数
	 */
	/*
	 * 計算結果がオーバーフローする問題に対策
	 * 2002/11/25
	 */
	if(!bfill) {
		i = (uRate / 25);
		i *= dwSoundTotal;
		i /= 40000;
		/*
		 * uSampleと比較、一致していれば何もしない
		 */
		if (i <= (int) uSample) {
			return;
		}
		/*
		 * uSampleとの差が今回生成するサンプル数
		 */
		i -= (int)uSample;
		/*
		 * samplesよりも小さければ合格
		 */
		if (i <= samples) {
			samples = i;
		}
	}

//	wbank = bNowBank?0:1;
	wbank = bNowBank;
      //  if(samples>0) {
	   if(!bZero) {
	      RenderThreadSub(uSample,samples,wbank);
	   } else {
	      RenderThreadBZero(uSample,samples,wbank);
	   }
	//}

	if(bfill) {
	  //      if(samples > 0) {
		   uSample += samples;
	//	}
	   	RenderPlay(uSample, wbank, bPlayEnable);
	   	bNowBank++;
	   	if(bNowBank >= 4) bNowBank = 0;
	uSample = 0;
	dwSoundTotal = 0;
	} else {
    //  if(samples > 0) {
		uSample += samples;
	//}
	}
}

#ifdef __cplusplus
extern "C" {
#endif

void opn_notify(BYTE reg, BYTE dat)
{
	BYTE r;

	/*
	 * OPNがなければ、何もしない
	 */
	if (!DrvOPN) {
		return;
	}
	/*
	 * プリスケーラを調整
	 */
	if (opn_scale[OPN_STD] != nScale[OPN_STD]) {
		nScale[OPN_STD] = opn_scale[OPN_STD];
		switch (opn_scale[OPN_STD]) {
		case 2:
			DrvOPN->SetReg(OPN_STD, 0x2f, 0);
			break;
		case 3:
            DrvOPN->SetReg(OPN_STD, 0x2e, 0);
			break;
		case 6:
            DrvOPN->SetReg(OPN_STD, 0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (DrvOPN->GetCh3Mode(OPN_STD) == dat) {
			return;
		}
		DrvOPN->SetCh3Mode(OPN_STD, dat);
	}

	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = DrvOPN->GetReg(OPN_STD, 0x27);
		if ((r & 0xc0) != 0x80) {
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
	    DrvOPN->SetReg(OPN_STD, (uint8) reg, (uint8) dat);
}



void
thg_notify(BYTE reg, BYTE dat)
{
	BYTE r;
	/*
	 * THGがなければ、何もしない
	 */
	if (!DrvOPN) {
		return;
	}

	/*
	 * プリスケーラを調整
	 */
	if (opn_scale[OPN_THG] != nScale[OPN_THG]) {
		nScale[OPN_THG] = opn_scale[OPN_THG];
		switch (opn_scale[OPN_THG]) {
		case 2:
			DrvOPN->SetReg(OPN_THG, 0x2f, 0);
			break;
		case 3:
            DrvOPN->SetReg(OPN_THG, 0x2e, 0);
			break;
		case 6:
            DrvOPN->SetReg(OPN_THG, 0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (DrvOPN->GetCh3Mode(OPN_THG) == dat) {
			return;
		}
		DrvOPN->SetCh3Mode(OPN_THG, dat);
	}

	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = DrvOPN->GetReg(OPN_THG, 0x27);

		 if ((r & 0xc0) != 0x80) {
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
	    DrvOPN->SetReg(OPN_THG, (uint8) reg, (uint8) dat);
}

void
whg_notify(BYTE reg, BYTE dat)
{
	BYTE r;
	/*
	 * THGがなければ、何もしない
	 */
	if (!DrvOPN) {
		return;
	}

	/*
	 * プリスケーラを調整
	 */
	if (opn_scale[OPN_WHG] != nScale[OPN_WHG]) {
		nScale[OPN_WHG] = opn_scale[OPN_WHG];
		switch (opn_scale[OPN_THG]) {
		case 2:
			DrvOPN->SetReg(OPN_WHG, 0x2f, 0);
			break;
		case 3:
			DrvOPN->SetReg(OPN_WHG, 0x2e, 0);
			break;
		case 6:
			DrvOPN->SetReg(OPN_WHG, 0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (DrvOPN->GetCh3Mode(OPN_WHG) == dat) {
			return;
		}
		DrvOPN->SetCh3Mode(OPN_WHG, dat);
	}
	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = DrvOPN->GetReg(OPN_WHG, 0x27);

		 if ((r & 0xc0) != 0x80) {
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
		DrvOPN->SetReg(OPN_WHG, reg, dat);
}


void wav_notify(BYTE no)
{
   int ch;

   if(applySem == NULL) return;
   SDL_SemWait(applySem);

	if(no == SOUND_STOP){
		Mix_HaltGroup(GROUP_SND_SFX);
	} else {
		if(DrvWav != NULL) {
		  ch = CH_WAV_RELAY_ON + no;
			DrvWav[no].Play(ch, 0);
		}
	}
	SDL_SemPost(applySem);
}

void beep_notify(void)
{

	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}
    AddSnd(FALSE, FALSE);

	if (beep_flag && speaker_flag) {
		bBeepFlag = TRUE;
	} else {
		bBeepFlag = FALSE;
	}
	if(DrvBeep)  DrvBeep->Enable(bBeepFlag);
}

void tape_notify(BOOL flag)
{

	if (bTapeFlag == flag) {
		return;
	}
	if(!DrvCMT) return;
	DrvCMT->SetState((BOOL)bTapeFlag);
	if (bTapeMon) {
		DrvCMT->Enable(TRUE);
		AddSnd(FALSE, FALSE);
	} else {
		DrvCMT->Enable(FALSE);
		AddSnd(FALSE, FALSE);
	}
	bTapeFlag = flag;
}

#ifdef __cplusplus
}
#endif

/*
 * 演奏停止
 */
void        ProcessSnd(BOOL bZero)
{
	BOOL    bWrite;
	/*
	 * 初期化されていなければ、何もしない
	 */
	if (!DrvOPN) {
		return;
	}
	SDL_SemWait(applySem);



	/*
	 * 書き込み位置とバンクから、必要性を判断
	 */
	bWrite = FALSE;
#if 0
   if (bNowBank) {
		if (dwPlayC >= (uTick / 2) ) {
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
 #else
       if (dwPlayC >= (uTick / 2 )) {
		bWrite = TRUE;
//	        dwPlayC = 0;
		dwPlayC++;
	}  else {
	        dwPlayC++;
	}
   
	if (dwPlayC >= uTick) {
		dwPlayC = 0;
	}

#endif   

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
	       SDL_SemPost(applySem);
		   return;
	  }

	  /*
	   * ここから演奏開始
	   */
   AddSnd(TRUE, bZero);

	  /*
	   * 書き込みバンク(仮想)
	   */
//	  bNowBank = (!bNowBank);
    dwPlayC = 0;
	SDL_SemPost(applySem);

}

static int WavCaptureSub(Uint8 *out, int slot)
{
	int w,tmp;
	Mix_Chunk *p;
	Sint16 *q = (Sint16 *)out;
	int len;
	int slots = 0;
	int i,j;
	int size;


	if(!bWavCapture) return 0;
	if(out == NULL) return 0;
#if 0
	if(DrvBeep != NULL) {
		p = DrvBeep->GetChunk(slot);
		len = p->alen / sizeof(Sint16);
		memcpy_add16(out, p->abuf, len);
		slots++;
	}
#endif
	if(slots>0) {
//		p = DrvBeep->GetChunk(slot);
//		if(p) size = p->alen / sizeof(Sint16);
		for(j = 0; j < (size - 8); j+=8){
			*q =  *q / slots;
			q++;
			*q =  *q / slots;
			q++;
			*q = *q / slots;
			q++;
			*q = *q / slots;
			q++;
		}
		for(i = j - 8; i < size; i++){
			*q = *q / slots;
			q++;
		}

	}
	return len;
}

static int RenderThreadSub(int start, int size, int slot)
{
	int w;
	DWORD *bank = NULL;


//	if(slot > SND_BUF) return 0;
	/* レンダリング */
	/* BEEP */
	if(DrvBeep != NULL){
		w = DrvBeep->Render(start, size, slot, TRUE);
	}
#if 1
	if(DrvCMT != NULL) {
		w = DrvCMT->Render(start, size, slot, TRUE);
	}

	if(DrvOPN != NULL) {
		w =DrvOPN->Render(start, size, slot, TRUE);
	}
#endif
	return size;
}

static int RenderThreadBZero(int start,int size,int slot)
{
	int w;
	DWORD *bank = NULL;

	//if(slot > SND_BUF) return 0;
	/* レンダリング */
	/* BEEP */
	if(DrvBeep != NULL) {
		w = DrvBeep->BZero(start, size, slot, TRUE);
	}
#if 1
	if(DrvCMT != NULL) {
		w =DrvCMT->BZero(start, size, slot, TRUE);
	}

	if(DrvOPN != NULL) {
		w =DrvOPN->BZero(start, size, slot, TRUE);
	}

#endif
	return size;
}

static void RenderPlay(int samples, int slot, BOOL play)
{
	/*
	 * レンダリング: bFill = TRUEで音声出力
	 */
	DWORD *bank = NULL;
	int slot2 = slot - 1;
	BOOL playing;
	int i;

	if(slot2 < 0) slot2 = 3;
	bSndDataEnable[slot] = TRUE;

	if(play) {
		if(applySem == NULL) return;
		SDL_SemWait(applySem);
		Mix_Volume(-1,iTotalVolume);
		if(DrvBeep != NULL) {
			DrvBeep->Play(CH_SND_BEEP , slot, samples);
		}

		if(DrvCMT != NULL) {
			DrvCMT->Play(CH_SND_CMT , slot, samples);
		}

#if 1
		if(DrvOPN != NULL) {
			DrvOPN->Play(CH_SND_OPN , slot, samples);
		}
#endif
		SDL_SemPost(applySem);
	}
}


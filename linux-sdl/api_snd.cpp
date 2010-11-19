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
//#include <gtk/gtk.h>
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
#define SND_BUF 2


enum {
	CH_SND_BEEP = 0,
	CH_SND_CMT = 2,
	CH_SND_OPN = 4,
	CH_SND_WHG = 6,
	CH_SND_THG = 8,
	CH_WAV_RELAY_ON = 10,
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
	GROUP_SND_WHG ,
	GROUP_SND_THG ,
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
static UINT             uBeep;          /* BEEP波形カウンタ */
static int              nFMVol;
static int              nPSGVol;
static int              nBeepVol;
static int              nCMTVol;
static int              nWavVol;
static UINT             uChanSep;

static int              nScale[3];      /* OPNプリスケーラ */
static BYTE             uCh3Mode[3];    /* OPN Ch.3モード */
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
static int bNowBank;
static DWORD dwPlayC;
static BOOL				bWavFlag; /* WAV演奏許可フラグ */
//ここまで
static Mix_Chunk *WavChunk[WAV_SLOT];
static BOOL bMode;
static DWORD uProcessCount;
static SDL_sem *applySem;
static BOOL bPlayEnable;


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
static SndDrvOpn *DrvWHG;
static SndDrvOpn *DrvTHG;
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

	iTotalVolume = SDL_MIX_MAXVOLUME;
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
	DrvWHG = NULL;
	DrvTHG = NULL;
	DrvWav = NULL;
	DrvCMT = NULL;
	applySem = NULL;
	bPlayEnable = FALSE;

	snd_thread = NULL;
	CmdRing = NULL;

	uClipCount = 0;
	//	bInitFlag = FALSE;
	//    InitFDDSnd();
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	/*
	 * WAVよむ
	 */

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
	if(DrvWHG)		{
		delete DrvWHG;
		DrvWHG = NULL;
	}
	if(DrvTHG)		{
		delete DrvTHG;
		DrvTHG = NULL;
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

	if ((uStereo > 0) || bForceStereo) {
		uChannels = 2;
	} else {
		uChannels = 1;
	}
	dwPlayC = 0;
	bNowBank = 0;

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

	if(applySem == NULL) {
		applySem = SDL_CreateSemaphore(1);
		SDL_SemPost(applySem);
	}
	if (Mix_OpenAudio
			(uRate, AUDIO_S16SYS, uChannels, uBufSize / (2 * sizeof(Sint16) * uChannels)) == -1) {
	   printf("Warning: Audio can't initialize!\n");
		return FALSE;
	}
	Mix_AllocateChannels(CH_CHANNELS - 1);
	/*
	 *
	 */
	Mix_GroupChannels(CH_SND_BEEP, CH_SND_BEEP + 1, GROUP_SND_BEEP);
	Mix_GroupChannels(CH_SND_CMT, CH_SND_CMT + 1, GROUP_SND_CMT);
	Mix_GroupChannels(CH_SND_OPN, CH_SND_OPN + 1, GROUP_SND_OPN);
	Mix_GroupChannels(CH_SND_WHG, CH_SND_WHG + 1, GROUP_SND_WHG);
	Mix_GroupChannels(CH_SND_THG, CH_SND_THG + 1, GROUP_SND_THG);
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
			DrvOPN->SetOpNo(OPN_STD);
			DrvOPN->Setup(uTick);
			DrvOPN->Enable(TRUE);
	}
	/*
	 * OPNデバイス(WHG)を作成
	 */
	DrvWHG= new SndDrvOpn;
	if(DrvWHG) {
			DrvWHG->SetOpNo(OPN_WHG);
			DrvWHG->Setup(uTick);
			DrvWHG->Enable(TRUE);
	}
	/*
	 * OPNデバイス(THG)を作成
	 */
	DrvTHG= new SndDrvOpn;
	if(DrvTHG) {
			DrvTHG->SetOpNo(OPN_THG);
			DrvTHG->Setup(uTick);
			DrvTHG->Enable(TRUE);
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
		DrvOPN->SetReg(opn_reg[OPN_STD]);
	}
	if(DrvWHG) {
		DrvWHG->SetReg(opn_reg[OPN_WHG]);
	}
	if(DrvTHG) {
		DrvTHG->SetReg(opn_reg[OPN_THG]);
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
			DrvOPN->SetRenderVolume(nFMVolume, nPSGVolume);
	}
	if(DrvWHG) {
			DrvWHG->SetRenderVolume(nFMVolume, nPSGVolume);
	}
	if(DrvTHG) {
			DrvTHG->SetRenderVolume(nFMVolume, nPSGVolume);
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
	if(DrvTHG) DrvTHG->SetLRVolume();
	if(DrvWHG) DrvWHG->SetLRVolume();
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
	if((DrvOPN == NULL) || (DrvWHG == NULL) || (DrvTHG == NULL)) {
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

	/*
	 * サンプルカウンタ、サウンド時間をクリア
	 */
	uSample = 0;
	dwSoundTotal = 0;
	uClipCount = 0;
	dwPlayC = 0;
	uProcessCount = 0;
	iTotalVolume = SDL_MIX_MAXVOLUME - 1;	/* ここで音量設定する必要があるか?
	 */

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
	wbank = bNowBank?0:1;
	if(!bZero) {
		RenderThreadSub(uSample,samples,wbank);
	} else {
		RenderThreadBZero(uSample,samples,wbank);
	}

	if(bfill) {
		uSample += samples;
		RenderPlay(uSample, wbank, bPlayEnable);
		dwSoundTotal = 0;
		uSample = 0;
	} else {
		uSample += samples;
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
			DrvOPN->SetReg(0x2f, 0);
			break;
		case 3:
            DrvOPN->SetReg(0x2e, 0);
			break;
		case 6:
            DrvOPN->SetReg(0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (DrvOPN->GetCh3Mode() == dat) {
			return;
		}
		DrvOPN->SetCh3Mode(dat);
	}

	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = DrvOPN->GetReg(0x27);
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
	if(DrvOPN) {
	    DrvOPN->SetReg((uint8) reg, (uint8) dat);
	}
}



void
thg_notify(BYTE reg, BYTE dat)
{
	BYTE r;
	/*
	 * THGがなければ、何もしない
	 */
	if (!DrvTHG) {
		return;
	}

	/*
	 * プリスケーラを調整
	 */
	if (opn_scale[OPN_THG] != nScale[OPN_THG]) {
		nScale[OPN_THG] = opn_scale[OPN_THG];
		switch (opn_scale[OPN_THG]) {
		case 2:
			DrvTHG->SetReg(0x2f, 0);
			break;
		case 3:
            DrvTHG->SetReg(0x2e, 0);
			break;
		case 6:
            DrvTHG->SetReg(0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (DrvTHG->GetCh3Mode() == dat) {
			return;
		}
		DrvTHG->SetCh3Mode(dat);
	}

	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = DrvTHG->GetReg(0x27);

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
	if(DrvTHG) {
	    DrvTHG->SetReg((uint8) reg, (uint8) dat);
	}
}

void
whg_notify(BYTE reg, BYTE dat)
{
	BYTE r;
	/*
	 * THGがなければ、何もしない
	 */
	if (!DrvWHG) {
		return;
	}

	/*
	 * プリスケーラを調整
	 */
	if (opn_scale[OPN_WHG] != nScale[OPN_WHG]) {
		nScale[OPN_WHG] = opn_scale[OPN_WHG];
		switch (opn_scale[OPN_THG]) {
		case 2:
			DrvWHG->SetReg(0x2f, 0);
			break;
		case 3:
			DrvWHG->SetReg(0x2e, 0);
			break;
		case 6:
			DrvWHG->SetReg(0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (DrvWHG->GetCh3Mode() == dat) {
			return;
		}
		DrvWHG->SetCh3Mode(dat);
	}
	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = DrvWHG->GetReg(0x27);

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
	if(DrvWHG) {
		DrvWHG->SetReg(reg, dat);
	}
}


void
wav_notify(BYTE no)
{
	int    i;
	int    j;
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

void
beep_notify(void)
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

void
tape_notify(BOOL flag)
{

	if (bTapeFlag == flag) {
		return;
	}
	DrvCMT->SetState((BOOL)bTapeFlag);
	if(!DrvCMT) return;
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
	  bNowBank = (!bNowBank);
	SDL_SemPost(applySem);

}

/*
 *  レベル取得
 */
int
GetLevelSnd(int ch)
{
	ASSERT((ch >= 0) && (ch < 18));

	/*
	 * OPN,WHGの区別
	 */
	if (ch < 6) {
		if(DrvOPN) {
			return DrvOPN->GetLevelSnd(ch);
		} else {
			return 0;
		}
	} else if ((ch >= 6) && (ch < 12)) {
		if (!whg_enable || !whg_use) {
			return 0;
		}

		if(DrvWHG) {
			return DrvWHG->GetLevelSnd(ch-6);
		} else {
			return 0;
		}
		/*
		 * WHGの場合、実際に使われていなければ0
		 */
	} else   if ((ch >= 12) && (ch < 18)) {
		/*
		 * THGの場合、実際に使われていなければ0
		 */
		if ((!thg_enable || !thg_use) && (fm7_ver != 1)) {
			return 0;
		}
		if(DrvTHG) {
			return DrvTHG->GetLevelSnd(ch-6);
		} else {
			return 0;
		}
	}
}


/*
 * サウンド合成
 */


static void memcpy_add16(Uint8 *out,Uint8 *in,int  size)
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

static int
RenderThreadSub(int start, int size, int slot)
{
	int bufsize;
	int w;
	DWORD *bank = NULL;


	bufsize = (uTick * uRate ) / 1000;
	if(slot > SND_BUF) return 0;
	/* レンダリング */
	/* BEEP */
	if(DrvBeep != NULL){
		w = DrvBeep->Render(start, size, slot, FALSE);
	}
#if 1
	if(DrvCMT != NULL) {
		w = DrvCMT->Render(start, size, slot, FALSE);
	}

	if(DrvOPN != NULL) {
		w =DrvOPN->Render(start, size, slot, TRUE);
	}


	if(whg_use) {
		if(DrvWHG != NULL) {
			w = DrvWHG->Render(start, size, slot, TRUE);
		}
	}
	if(thg_use) {
		if(DrvTHG != NULL) {
			w = DrvTHG->Render(start, size, slot, TRUE);
		}
	}

#endif
	return size;
}

static int
RenderThreadBZero(int start,int size,int slot)
{
	int w;
	DWORD *bank = NULL;

	if(slot > SND_BUF) return 0;
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

	if(whg_use) {
		if(DrvWHG != NULL) {
			w = DrvWHG->BZero(start, size, slot, TRUE);
		}
	}
	if(thg_use) {
		if(DrvTHG != NULL) {
			w = DrvTHG->BZero(start, size, slot, TRUE);
		}
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

	if(play) {
		if(applySem == NULL) return;
		SDL_SemWait(applySem);

		if(DrvBeep != NULL) {
			if(bPlayEnable) DrvBeep->Play(CH_SND_BEEP + slot, slot);
		}

		if(DrvCMT != NULL) {
			if(bPlayEnable) DrvCMT->Play(CH_SND_CMT + slot, slot);
		}

#if 1

		if(DrvOPN != NULL) {
			if(bPlayEnable) DrvOPN->Play(CH_SND_OPN + slot , slot, samples);
		}

		if(DrvWHG != NULL) {
			if(whg_use) {
				if(bPlayEnable) DrvWHG->Play(CH_SND_WHG + slot, slot, samples);
			}
		}
		if(DrvTHG != NULL) {
			if(thg_use) {
				if(bPlayEnable) DrvTHG->Play(CH_SND_THG + slot , slot, samples);
			}
		}
#endif
		SDL_SemPost(applySem);
	}
}


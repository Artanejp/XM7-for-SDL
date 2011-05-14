/*
 *
 */

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
 * 定数など
 */
#define CHUNKS 2
#define WAV_CHANNELS 5

enum {
	CH_SND_BEEP = 0,
	CH_SND_CMT = 4,
	CH_SND_OPN = 8,
	CH_WAV_RELAY_ON = 12,
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

struct SndBufType {
	Sint32 *pBuf32;
	Sint16 *pBuf;
	Uint32 nSize;
	Uint32 nReadPTR;
	Uint32 nWritePTR;
	int nHeadChunk;
	int nLastChunk;
	DWORD nLastTime;
	int nChunks;
	int nChunkNo;
	Mix_Chunk **mChunk; /* Chunkの配列へのポインタ */
} ;


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
static BOOL					bMode; /* HQモード */
static struct SndBufType *pOpnBuf;
static struct SndBufType *pBeepBuf;
static struct SndBufType *pCMTBuf;
static struct SndBufType *pCaptureBuf;
static struct SndBufType *pSndBuf;

/*
 * OPN内部変数
 */
static int           nScale[3];      /* OPNプリスケーラ */

static BOOL bSndEnable;
static DWORD dwSndCount;
static DWORD uTick;   // バッファサイズ(時間)
static DWORD uRate;   // サンプリングレート
static DWORD uBufSize; // バッファサイズ(バイト数)

static BOOL             bTapeFlag;      /* 現在のテープ出力状態 */
static BOOL				bWavFlag; /* WAV演奏許可フラグ */
static SDL_sem 			*applySem;

/*
 * サウンドレンダリングドライバ
 */
static SndDrvBeep *DrvBeep;
static SndDrvWav *DrvWav;
static SndDrvOpn *DrvPSG;
static SndDrvOpn *DrvOPN;
static SndDrvCMT *DrvCMT;
static struct RingBufferDesc *CmdRing;


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
 * サウンドバッファの概要を初期化する
 */
static struct SndBufType *InitBufferDesc(void)
{
	struct SndBufType *p;
	void *cp;
	int i;

	p = (struct SndBufType *)malloc(sizeof(struct SndBufType));
	if(p){
		memset(p, 0x00, sizeof(struct SndBufType));
		cp = malloc(sizeof(Mix_Chunk *) * CHUNKS);
		if(cp) {
			memset(cp, 0x00, sizeof(Mix_Chunk *) * CHUNKS);
		}
		p->mChunk = (Mix_Chunk **)cp;
		for(i = 0; i < CHUNKS ; i++) {
			p->mChunk[i] = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
			if(p->mChunk[i]) {
				memset((void *)p->mChunk[i], 0x00, sizeof(Mix_Chunk));
			}
		}
		p->nChunks = CHUNKS;
		p->nChunkNo = 0;
		p->nHeadChunk = 0;
		p->nLastChunk = 0;
	}
	return p;
}

static void DetachBuffer(struct SndBufType *p);

/*
 * サウンドバッファの概要を消す
 */
static void DetachBufferDesc(struct SndBufType *p)
{
	int i;

	if(p){
		DetachBuffer(p);
		for(i = 0; i < p->nChunks ; i++) {
			if(p->mChunk[i]) {
				free(p->mChunk[i]);
				p->mChunk[i] = NULL;
			}
		}
		p->nChunks = 0;
		free(p);
		p = NULL;
	}
	return;
}


/*
 * 実バッファのALLOCATE
 */
static void SetupBuffer(struct SndBufType *p, int members, BOOL flag16, BOOL flag32)
{
    int size;
    int channels = 2;

	if(p == NULL) return;
	if(flag16) {
		size = members * sizeof(Sint16) * channels;
		p->pBuf = (Sint16 *)malloc(size);
		if(p->pBuf) {
			memset(p->pBuf, 0x00, size);
			p->nSize = members;
		}
	}
	if(flag32) {
		size = members * sizeof(Sint32) * channels;
		p->pBuf32 = (Sint32 *)malloc(size);
		if(p->pBuf32) {
			memset(p->pBuf32, 0x00, size);
			p->nSize = members;
		}
	}
	p->nReadPTR = 0;
	p->nWritePTR = 0;

}

static void DetachBuffer(struct SndBufType *p)
{
	if(p == NULL) return;
	if(p->pBuf) {
		free(p->pBuf);
		p->pBuf = NULL;
	}
	if(p->pBuf32) {
		free(p->pBuf32);
		p->pBuf32 = NULL;
	}
}

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
	bMode = FALSE;

	iTotalVolume = SDL_MIX_MAXVOLUME - 1;

	bSndEnable = FALSE;
	bWavFlag = FALSE;
	DrvBeep = NULL;
	DrvOPN = NULL;
	DrvPSG = NULL;
	DrvWav = NULL;
	DrvCMT = NULL;
	applySem = NULL;

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
	pSndBuf = InitBufferDesc();
	pCaptureBuf = InitBufferDesc();
}

/*
 *  クリーンアップ
 */
void CleanFDDSnd(void)
{

}


void CleanSnd(void)
{
	DetachBufferDesc(pOpnBuf);
	pOpnBuf = NULL;

	DetachBufferDesc(pBeepBuf);
	pBeepBuf = NULL;

	DetachBufferDesc(pCMTBuf);
	pCMTBuf = NULL;

	DetachBufferDesc(pCaptureBuf);
	pCaptureBuf = NULL;
//	DetachBufferDesc(pSndBuf);

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
		DetachBuffer(pBeepBuf);
		DetachBuffer(pCMTBuf);
		DetachBuffer(pOpnBuf);
		DetachBuffer(pCaptureBuf);
		//	DetachBuffer(pSndBuf);
		bSndEnable = FALSE;
	}
	/*
	 * ドライバの抹消
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

BOOL SelectSnd(void)
{

	int members;
	int wavlength;

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

/*
 * バッファの初期化
 */
	dwSndCount = 0;
	uBufSize = (nSampleRate * nSoundBuffer * 2 * sizeof(Sint16)) / 1000;
    if (Mix_OpenAudio(uRate, AUDIO_S16SYS, 2, uBufSize / 8 ) < 0) {
	   printf("Warning: Audio can't initialize!\n");
	   return -1;
	}
	Mix_AllocateChannels(CH_CHANNELS);
	Mix_GroupChannels(CH_WAV_RELAY_ON, CH_WAV_RESERVE2, GROUP_SND_SFX);
//	Mix_Volume(-1,iTotalVolume);
	Mix_Volume(-1,127);


    bSndEnable = TRUE;
	uTick = nSoundBuffer;
	members = (nSampleRate * nSoundBuffer) / 1000;
	SetupBuffer(pBeepBuf, members, TRUE, FALSE);
	SetupBuffer(pCMTBuf, members, TRUE, FALSE);
	SetupBuffer(pOpnBuf, members, TRUE, TRUE);

	wavlength = (nSampleRate * 2000) / 1000 ; // キャプチャバッファは二秒
	SetupBuffer(pCaptureBuf, members, TRUE, FALSE);

	/*
	 * レンダリングドライバの設定
	 */
	DrvOPN = new SndDrvOpn ;
	DrvBeep = new SndDrvBeep ;
	DrvWav = new SndDrvWav[WAV_CHANNELS] ;
	DrvCMT= new SndDrvCMT ;


	if(DrvOPN) {
		DrvOPN->SetRate(uRate);
		DrvOPN->Setup(uTick);
		DrvOPN->Enable(TRUE);
	}
	if(DrvOPN) {
		DrvBeep->SetRate(uRate);
		DrvBeep->Setup(uTick);
		DrvBeep->Enable(FALSE);
	}

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
	   CloseSnd();
	}

	/*
	 * 再セレクト
	 */
	SelectSnd();
    //    SDL_SemPost(applySem);
	// BEEPについて、SelectSnd()し直しても音声継続するようにする
	bBeepFlag = !bBeepFlag;
	beep_notify();
	tape_notify(!bTapeFlag);
}

/*
 *  WAVキャプチャ開始
 */
void OpenCaptureSnd(char *fname)
{

}

void CloseCaptureSnd(void)
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

void SetTotalVolume(int vol)
{

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
 * サウンドレンダリング本体
 * レンダラは上書きしていく(!)
 */

static DWORD RenderOpnSub(DWORD time, int samples, BOOL bZero)
{
	struct SndBufType *p = pOpnBuf;
	int j;

	if(samples <= 0) return 0;
	j = samples;

	if((j + p->nWritePTR) > p->nSize){
		// バッファオーバフローの時は分割する
		int k;

		k = p->nSize - p->nWritePTR;
		if(k > 0) {
			DrvOPN->Render32(p->pBuf32, p->nWritePTR, k,  TRUE, bZero);
			DrvOPN->Copy32(p->pBuf32, p->pBuf, p->nWritePTR, k);
			j = j - k;
		}
		p->nWritePTR = 0;
		if(j > 0) {
			DrvOPN->Render32(p->pBuf32, 0, j, TRUE, bZero);
			DrvOPN->Copy32(p->pBuf32, p->pBuf,0,  j);
			p->nWritePTR = j;
		}
	} else {
		if(j > 0) {
			DrvOPN->Render32(p->pBuf32, p->nWritePTR, j,  TRUE, bZero);
			DrvOPN->Copy32(p->pBuf32, p->pBuf, p->nWritePTR, j);
			p->nWritePTR += j;
			if(p->nWritePTR > p->nSize) p->nWritePTR -= p->nSize;
		}
	}
	p->nLastTime = time;
	return j;
}

/*
 * バッファを一定時間の所まで埋める
 * TRUE : 埋めた
 * FALSE : 埋める必要がなかった
 */
static BOOL FlushOpnSub(DWORD time,  BOOL bZero)
{
	struct SndBufType *p;
	int i;
	int chunksize = ((uTick * uRate) / 1000) / CHUNKS;

	p = pOpnBuf;

	if(chunksize<0) return FALSE;
	i = chunksize - (p->nWritePTR % chunksize);
	/*
	 * オーバーフロー対策込
	 */
	RenderOpnSub(time,  i, bZero);
	return TRUE;
}


static DWORD RenderBeepSub(DWORD time, int samples, BOOL bZero)
{
	struct SndBufType *p = pBeepBuf;
	int j;

	if(samples <= 0) return 0;
	j = samples;

	if((j + p->nWritePTR) > p->nSize){
		// バッファオーバフローの時は分割する
		int k;

		k = p->nSize - p->nWritePTR;
		if(k > 0) {
			DrvBeep->Render32(p->pBuf, p->nWritePTR, k,  TRUE, bZero);
			j = j - k;
		}
		p->nWritePTR = 0;
		if(j > 0) {
			DrvBeep->Render32(p->pBuf, 0, j, TRUE, bZero);
			p->nWritePTR = j;
		}
	} else {
		if(j > 0) {
			DrvBeep->Render32(p->pBuf, p->nWritePTR, j,  TRUE, bZero);
			p->nWritePTR += j;
			if(p->nWritePTR > p->nSize) p->nWritePTR -= p->nSize;
		}
	}
	p->nLastTime = time;
	return j;
}

/*
 * バッファを一定時間の所まで埋める
 * TRUE : 埋めた
 * FALSE : 埋める必要がなかった
 */
static BOOL FlushBeepSub(DWORD time,  BOOL bZero)
{
	struct SndBufType *p;
	int i;
	int chunksize = ((uTick * uRate) / 1000) / CHUNKS;

	p = pOpnBuf;

	if(chunksize<0) return FALSE;
	i = chunksize - (p->nWritePTR % chunksize);
	/*
	 * オーバーフロー対策込
	 */
	RenderBeepSub(time,  i, bZero);
	return TRUE;
}


/*
 * ChunkをSetする
 */
static void SetChunkSub(Mix_Chunk *p, Sint16 *buf, Uint32 len, int volume)
{
	int channels = 2;

	p->abuf = (Uint8 *)buf;
	p->alen = len * sizeof(Sint16) * channels;
	p->allocated = 1;
	p->volume = (Uint8)volume;
}

/*
 * XXX Notify系関数…VM上の仮想デバイスに変化があったとき呼び出される
 */
#ifdef __cplusplus
extern "C" {
#endif

static int CalcSamples(DWORD time)
{
	uint64_t time2;
	uint64_t last;
	uint64_t diff;
	int samples;

	time2 =(uint64_t) time;
	last = (uint64_t) pOpnBuf->nLastTime;

	if(time2 < last) {
		diff = 0x100000000 + time2 - last;
	} else {
		diff = time2 - last;
	}
	samples = (int)((diff * uRate) / 1000000);

	if(samples <= 0) samples = 0;
	return samples;
}

void opn_notify(BYTE reg, BYTE dat)
{
	DWORD time = dwSoundTotal;
	int samples = CalcSamples(time);
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
//	 AddSnd(FALSE, FALSE);
	 RenderOpnSub(time, samples, FALSE);

	/*
	 * 出力
	 */
    DrvOPN->SetReg(OPN_STD, (uint8) reg, (uint8) dat);

}

void thg_notify(BYTE reg, BYTE dat)
{
	DWORD time = dwSoundTotal;
	int samples = CalcSamples(time);
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
//    AddSnd(FALSE, FALSE);
	 RenderOpnSub(time, samples, FALSE);
	/*
	 * 出力
	 */
	    DrvOPN->SetReg(OPN_THG, (uint8) reg, (uint8) dat);

}

void whg_notify(BYTE reg, BYTE dat)
{
	DWORD time = dwSoundTotal;
	int samples = CalcSamples(time);
	BYTE r;
	/*
	 * WHGがなければ、何もしない
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
//	AddSnd(FALSE, FALSE);
	 RenderOpnSub(time, samples, FALSE);

	 /*
	 * 出力
	 */
		DrvOPN->SetReg(OPN_WHG, reg, dat);


}


void wav_notify(BYTE no)
{

}

void beep_notify(void)
{
	DWORD time = dwSoundTotal;
	int samples = CalcSamples(time);
	BYTE r;

	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}
//        AddSnd(FALSE, FALSE);
   	 RenderBeepSub(time, samples, FALSE);

	if (beep_flag && speaker_flag) {
		bBeepFlag = TRUE;
	} else {
		bBeepFlag = FALSE;
	}
	if(DrvBeep)  DrvBeep->Enable(bBeepFlag);

}

void tape_notify(BOOL flag)
{

}

#ifdef __cplusplus
}
#endif

static int SetChunk(struct SndBufType *p, int samples, int ch)
{
	int i = p->nChunkNo;
	int j;
	int k;
	int channels = 2;

	if(p->nReadPTR > p->nWritePTR) {
		j = p->nSize - p->nReadPTR;
		if((p->nReadPTR + samples) > p->nSize) {
			k = samples -j;
		} else {
			k = p->nWritePTR;
		}
	} else {
		j = p->nWritePTR - p->nReadPTR;
		k = 0;
	}

#if 0
	SetChunkSub(p->mChunk[i], &p->pBuf[p->nReadPTR], j, 127);
	Mix_PlayChannel(ch , p->mChunk[i], 0);
	p->nReadPTR += j;
	if(p->nReadPTR >= p->nSize) p->nReadPTR = 0;
	i++;
	if(i >= p->nChunks) i = 0;
	if(k > 0) {
		SetChunkSub(p->mChunk[i], &p->pBuf[p->nReadPTR], k, 127);
		Mix_PlayChannel(ch , p->mChunk[i], 0);
		p->nReadPTR += k;
		i++;
		if(i >= p->nChunks) i = 0;
	}
	j += k;
#else
	SetChunkSub(p->mChunk[i], &p->pBuf[p->nReadPTR], samples, 127);
	Mix_PlayChannel(ch , p->mChunk[i], 0);
	p->nReadPTR += samples;
	if(p->nReadPTR > p->nSize) p->nReadPTR -= p->nSize;
	i++;
	if(i >= p->nChunks) i = 0;
#endif
	p->nChunkNo = i;
	return i;
}

/*
 * 1msごとにスケジューラから呼び出されるhook
 */

void ProcessSnd(BOOL bZero)
{
	DWORD time = dwSoundTotal;
	int samples = CalcSamples(time);
	int chunksize;
	BOOL bWrite = FALSE;

	chunksize = ((uTick * uRate) / 1000) / CHUNKS;

	dwSndCount++;
	if(dwSndCount >= (uTick / CHUNKS)) {
		bWrite = TRUE;
		dwSndCount = 0;
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
			   RenderOpnSub(time, samples, bZero);
			   RenderBeepSub(time, samples, bZero);
		   }
		   return;
	  }

	if(bWrite) {
		// フラッシュする
		FlushOpnSub(time, bZero);
		FlushBeepSub(time, bZero);

		/*
		 * 演奏本体
		 */
		SetChunk(pOpnBuf , chunksize, CH_SND_OPN);
		SetChunk(pBeepBuf , chunksize, CH_SND_BEEP);
		dwSndCount = 0;
		dwSoundTotal = 0;
	}
}


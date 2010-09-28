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
#include "SndDrvWav.h"
#include "SndDrvOpn.h"
#include "util_ringbuffer.h"

#define WAV_SLOT 3
#define SND_BUF 2

enum {
	SND_SETUP,
	SND_SHUTDOWN,
	SND_BZERO,
	SND_RENDER,
	SND_BEEP_SETFREQ,
	SND_WAV_PLAY,
	SND_PSG_SETREG,
	SND_PSG_READREG,
	SND_OPN_SETREG,
	SND_OPN_READREG,
	SND_WHG_SETREG,
	SND_WHG_READREG,
	SND_THG_SETREG,
	SND_THG_READREG
};

enum {
	CH_WAV_RELAY_ON = 11,
			CH_WAV_RELAY_OFF,
			CH_WAV_FDDSEEK
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


struct SNDPushPacket {
	int cmd;
	int arg1;
	int arg2;
	void *arg3;
	void *arg4;
};

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
static BOOL             bTapeFlag2;     /* 前回のテープ出力状態 */
static BYTE             uTapeDelta;     /* テープ波形補間カウンタ */
static int bNowBank;
static DWORD dwPlayC;
//ここまで
static Sint16 *WavCache[WAV_SLOT];
static Mix_Chunk *WavChunk[WAV_SLOT];
static BOOL bMode;
static DWORD uProcessCount;


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



/*
 * サウンドレンダリングドライバ
 */
static SndDrvBeep *DrvBeep;
static SndDrvWav *DrvWav[WAV_SLOT];
static SndDrvOpn *DrvPSG;
static SndDrvOpn *DrvOPN;
static SndDrvOpn *DrvWHG;
static SndDrvOpn *DrvTHG;

static SDL_Thread *snd_thread;
static struct RingBufferDesc *CmdRing;
static SDL_cond *SndCond;
static SDL_mutex *SndMutex;

static void InitCommandBuffer(void)
{
	CmdRing = CreateRingBuffer(sizeof(SNDPushPacket), 255); /* キューは255段 */
}

static void DeleteCommandBuffer(void)
{
	DeleteRingBuffer(CmdRing); /* キューは255段 */
	CmdRing = NULL;
}


static int PushCommand(void *arg)
{
//	struct SNDPushPacket *p;
	int r = 0;

	if(arg != NULL) {
//		p = (struct SNDPushPacket *)malloc(sizeof(struct SNDPushPacket));
//		memcpy(p, arg, sizeof(struct SNDPushPacket));
		r = WriteRingBuffer(CmdRing, arg);
		if(SndCond != NULL) {
			SDL_CondSignal(SndCond);
		}
	}
	return r;
}

static int PullCommand(void *ret)
{
	struct SNDPushPacket p;
	int r;

	r = ReadRingBuffer(CmdRing, ret);
//	if(r>0) {
//		memcpy(ret, p, sizeof(struct SNDPushPacket));
//	}
	return r;
}



/*
 * 予めLoadしておいたWAVを演奏できるようにする
 */


/*
 * ファイルの読み込みは別オブジェクトにする
 */
static Uint8 *LoadWav(char *filename, int slot)
{
	int fileh;
	DWORD filesize;
	if(slot >= WAV_SLOT) return NULL;
	Uint8 *rbuf;

	if(WavCache[slot] != NULL) return NULL; /* 既にWAVが読み込まれてる */
	fileh = file_open(filename, OPEN_R);
	if(fileh < 0) return NULL;
	filesize = file_getsize(fileh);

	WavCache[slot] = (Sint16 *)malloc(filesize + 1);
	if(WavCache[slot] == NULL) {
		file_close(fileh);
		return NULL; /* 領域確保に失敗 */
	}
	rbuf = (Uint8 *)WavCache[slot];
	file_read(fileh, rbuf, filesize);
	file_close(fileh);
	return (Uint8 *)WavCache[slot];
}

/*
 * 読み込んだWAVを消去する
 */
static void WavDelete(int slot)
{
	if(slot >= WAV_SLOT) return;
	if(WavCache[slot] == NULL) return;
	free(WavCache[slot]);
	WavCache[slot] = NULL;
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
	bBeepFlag = TRUE;      /* BEEP出力 */


	iTotalVolume = SDL_MIX_MAXVOLUME - 1;
	snd_thread = NULL;
	SndMutex = NULL;
	SndCond = NULL;
	bMode = FALSE;
	bNowBank = 0;
	dwPlayC = 0;

	uClipCount = 0;
	//	bInitFlag = FALSE;
	//    InitFDDSnd();
	SDL_InitSubSystem(SDL_INIT_AUDIO);

	/*
	 * WAVよむ
	 */
	for(i = 0; i< WAV_SLOT; i++) {
		WavChunk[i] = NULL;
	}
#if 0
	for(i = 0; i < WAV_SLOT; i++){
	   char prefix[MAXPATHLEN];
	   strcpy(prefix, ModuleDir);
       strcat(prefix, WavName[i]);
       LoadWav(prefix, i);
	}
#endif

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
	int i;
	struct SNDPushPacket cmd;
	/*
	 * サウンド停止
	 */
	StopSnd();
	/*
	 * スレッド停止
	 */
	cmd.cmd = SND_SHUTDOWN;
	PushCommand(&cmd);
	/*
	 * スレッド資源解放待ち
	 */
	/*
	 * OPNを解放
	 */
	/*
	 * サウンド作成バッファを解放
	 */

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

	/*
	 * Wavバッファを解放
	 */
	for(i = 0; i< WAV_SLOT; i++){
		if(WavChunk[i]) Mix_FreeChunk(WavChunk[i]);
//		WavDelete(i);
	}

	if(DrvBeep) 	{
		delete DrvBeep;
		DrvBeep = NULL;
	}
	for(i = 0; i< WAV_SLOT ; i++) {
		if(DrvWav[i]){
			delete [] DrvWav[i];
		}
	}
	if(DrvPSG)		{
		delete [] DrvPSG;
		DrvPSG = NULL;
	}
	if(DrvOPN)		{
		delete [] DrvOPN;
		DrvOPN = NULL;
	}
	if(DrvWHG)		{
		delete [] DrvWHG;
		DrvWHG = NULL;
	}
	if(DrvTHG)		{
		delete [] DrvTHG;
		DrvTHG = NULL;
	}

	for(i = 0 ; i<WAV_SLOT; i++) {
		WavDelete(i);
	}
	DeleteCommandBuffer();
	/*
	 * uRateをクリア
	 */
	uRate = 0;



}

static int RenderThread(void *arg);

BOOL SelectSnd(void)
{
	int                 i,j;
	char prefix[MAXPATHLEN];

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
	uTapeDelta = 0;
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
	uBufSize = (uRate * uTick * uChannels * sizeof(Sint16)) / (1000 *2);
	/*
	 * サンプルカウンタ、サウンド時間をクリア
	 */
	uSample = 0;
	dwSoundTotal = 0;
	uClipCount = 0;
#if 0
		for(i = 0; i < 3; i++) {
			DrvWav[i] = new SndDrvWav[2];
			if(DrvWav[i]) {
				for(j = 0; j < 2; j++) {
					DrvWav[i][j].NewBuffer();
//					if(WavCache[i]) {
//						DrvWav[i][j].Setup((void *)WavCache[i]);
//					}
				}
			}
		}
#endif

	DrvBeep= new SndDrvBeep;
	if(DrvBeep) {
			DrvBeep->Setup(NULL);
	}

//	DrvPSG= new SndDrvOpn[SND_BUF] ;
//	if(DrvPSG) {
//		for(i = 0; i < SND_BUF ; i++) {
//			DrvPSG[i].SetOpNo(OPN_STD);
//			DrvPSG[i].NewBuffer();
//		}
//	}

	/*
	 * OPNデバイス(標準)を作成
	 */
	DrvOPN= new SndDrvOpn[SND_BUF] ;
	if(DrvOPN) {
		for(i = 0; i < SND_BUF ; i++) {
			DrvOPN[i].SetOpNo(OPN_STD);
			DrvOPN[i].NewBuffer();
		}
	}
	/*
	 * OPNデバイス(WHG)を作成
	 */
	DrvWHG= new SndDrvOpn[SND_BUF];
	if(DrvWHG) {
		for(i = 0; i < SND_BUF ; i++) {
			DrvWHG[i].SetOpNo(OPN_WHG);
			DrvWHG[i].NewBuffer();
		}
	}
	/*
	 * OPNデバイス(THG)を作成
	 */
	DrvTHG= new SndDrvOpn[SND_BUF];
	if(DrvTHG) {
		for(i = 0; i < SND_BUF ; i++) {
			DrvTHG[i].SetOpNo(OPN_THG);
			DrvTHG[i].NewBuffer();
		}
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
		DrvOPN[0].SetReg(opn_reg[OPN_STD]);
		DrvOPN[1].SetReg(opn_reg[OPN_STD]);
	}
	if(DrvWHG) {
		DrvWHG[0].SetReg(opn_reg[OPN_STD]);
		DrvWHG[1].SetReg(opn_reg[OPN_WHG]);
	}
	if(DrvTHG) {
		DrvWHG[0].SetReg(opn_reg[OPN_STD]);
		DrvTHG[1].SetReg(opn_reg[OPN_THG]);
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
	Mix_AllocateChannels(20);
	for(i = 0; i < WAV_SLOT; i++) {
		   strcpy(prefix, ModuleDir);
	       strcat(prefix, WavName[i]);
	       WavChunk[i] = Mix_LoadWAV(prefix);
	}

	/*
	 * テンプレ作成
	 */
	if(SndCond == NULL) {
		SndCond = SDL_CreateCond();
	}
	if(SndMutex == NULL) {
		SndMutex = SDL_CreateMutex();
	}
	InitCommandBuffer();
	if(snd_thread == NULL) {
		snd_thread = SDL_CreateThread(RenderThread, NULL);
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
	/* 音声プロパティとOPNが衝突しないようにするためのセマフォ初期化 */
	//    SDL_SemWait(applySem);
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
	//    SDL_SemPost(applySem);

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

	/* FM音源/PSGボリューム設定 */
	if(DrvOPN) {
		for(i = 0; i< SND_BUF; i++) {
			DrvOPN->SetRenderVolume(nFMVolume, nPSGVolume);
		}
	}
	if(DrvWHG) {
		for(i = 0; i< SND_BUF; i++) {
			DrvWHG[i].SetRenderVolume(nFMVolume, nPSGVolume);
		}
	}
	if(DrvTHG) {
		for(i = 0; i< SND_BUF; i++) {
			DrvTHG[i].SetRenderVolume(nFMVolume, nPSGVolume);
		}
	}

	/* BEEP音/CMT音/各種効果音ボリューム設定 */
	if(DrvBeep) {
			DrvBeep->SetRenderVolume(nBeepVolume);
	}

	//	nBeepLevel = (int)(32767.0 * pow(10.0, nBeepVolume / 20.0));
	//	nCMTLevel = (int)(32767.0 * pow(10.0, nCMTVolume / 20.0));
#ifdef FDDSND
//	for(i = 0; i < WAV_SLOT; i++) {
//		if(DrvWav[i]) {
//			for(j = 0 ; j < SND_BUF; j++) {
//				DrvWav[i][j].SetRenderVolume(nWaveVolume);
//			}
//		}
//	}
	//	nWaveLevel = (int)(32767.0 * pow(10.0, nWaveVolume / 20.0));
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
StopSnd()
{

	do {
		SDL_Delay(1);
	} while(Mix_Playing(-1));
	//        Mix_CloseAudio();


}
#ifdef __cplusplus
extern "C" {
#endif

void opn_notify(BYTE reg, BYTE dat)
{
	struct SNDPushPacket packet;
	BYTE r;
	BOOL flag;

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
			packet.cmd = SND_OPN_SETREG;
			packet.arg1 = 0x2f;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//pOPN->SetReg(0x2f, 0);
			break;
		case 3:
			packet.cmd = SND_OPN_SETREG;
			packet.arg1 = 0x2e;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//	                        pOPN->SetReg(0x2e, 0);
			break;
		case 6:
			packet.cmd = SND_OPN_SETREG;
			packet.arg1 = 0x2d;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//	                        pOPN->SetReg(0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_STD] == dat) {
			return;
		}
		uCh3Mode[OPN_STD] = dat;
	}

	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
//		flag = FALSE;
//		packet.cmd = SND_OPN_READREG;
//		packet.arg1 = 0x27;
//		packet.arg2 = 0x00;
//		packet.arg3 = (void *)&r;
//		packet.arg4 = (void *)&flag;
//		PushCommand(&packet);
		/*
		 * 返事待ち
		 */
//		 do {
//		 } while(!flag);
		r = opn_reg[OPN_STD][0x27];
		if ((r & 0xc0) != 0x80) {
			return;
		}
	}

	/*
	 * サウンド合成
	 */
	packet.cmd = SND_RENDER;
	packet.arg1 = FALSE;
	packet.arg2 = bNowBank;
	PushCommand(&packet);

	// AddSnd(FALSE, FALSE);

	/*
	 * 出力
	 */
	if(DrvOPN) {
		packet.cmd = SND_OPN_SETREG;
		packet.arg1 = (int) reg;
		packet.arg2 = (int) dat;
		PushCommand(&packet);
		//	                pOPN->SetReg((uint8) reg, (uint8) dat);
	}
}



void
thg_notify(BYTE reg, BYTE dat)
{
	struct SNDPushPacket packet;
	BYTE r;
	BOOL flag;

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
			packet.cmd = SND_THG_SETREG;
			packet.arg1 = 0x2f;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//                        pOPN[OPN_THG]->SetReg(0x2f, 0);
			break;
		case 3:
			packet.cmd = SND_THG_SETREG;
			packet.arg1 = 0x2e;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//                        pOPN[OPN_THG]->SetReg(0x2e, 0);
			break;
		case 6:
			packet.cmd = SND_THG_SETREG;
			packet.arg1 = 0x2d;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//                        pOPN[OPN_THG]->SetReg(0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_THG] == dat) {
			return;
		}
		uCh3Mode[OPN_THG] = dat;
	}
	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
//		flag = FALSE;
//		packet.cmd = SND_THG_READREG;
//		packet.arg1 = 0x27;
//		packet.arg2 = 0x00;
//		packet.arg3 = (void *)&r;
//		packet.arg4 = (void *)&flag;
//		PushCommand(&packet);
		/*
		 * 返事待ち
		 */
//		 do {
//		 } while(!flag);
//		r = DrvOPN->ReadReg(0x27);
		r = opn_reg[OPN_THG][0x27];
		 if ((r & 0xc0) != 0x80) {
			 return;
		 }
	}


	/*
	 * サウンド合成
	 */
	//        AddSnd(FALSE, FALSE);
	packet.cmd = SND_RENDER;
	packet.arg1 = FALSE;
	packet.arg2 = bNowBank;
	PushCommand(&packet);

	/*
	 * 出力
	 */
	if(DrvTHG) {
		packet.cmd = SND_THG_SETREG;
		packet.arg1 = (int)reg;
		packet.arg2 = (int)dat;
		PushCommand(&packet);
	}
}

void
whg_notify(BYTE reg, BYTE dat)
{
	struct SNDPushPacket packet;
	BYTE r;
	BOOL flag;

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
			packet.cmd = SND_WHG_SETREG;
			packet.arg1 = 0x2f;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//                        pOPN[OPN_THG]->SetReg(0x2f, 0);
			break;
		case 3:
			packet.cmd = SND_WHG_SETREG;
			packet.arg1 = 0x2e;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//                        pOPN[OPN_THG]->SetReg(0x2e, 0);
			break;
		case 6:
			packet.cmd = SND_WHG_SETREG;
			packet.arg1 = 0x2d;
			packet.arg2 = 0x00;
			PushCommand(&packet);
			//                        pOPN[OPN_THG]->SetReg(0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (uCh3Mode[OPN_WHG] == dat) {
			return;
		}
		uCh3Mode[OPN_WHG] = dat;
	}
	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
//		flag = FALSE;
//		packet.cmd = SND_WHG_READREG;
//		packet.arg1 = 0x27;
//		packet.arg2 = 0x00;
//		packet.arg3 = (void *)&r;
//		packet.arg4 = (void *)&flag;
//		PushCommand(&packet);
		/*
		 * 返事待ち
		 */
//		 do {
//		 } while(!flag);
//		r = DrvWHG->ReadReg(0x27);
		r = opn_reg[OPN_WHG][0x27];

		 if ((r & 0xc0) != 0x80) {
			 return;
		 }
	}


	/*
	 * サウンド合成
	 */
	//        AddSnd(FALSE, FALSE);
	packet.cmd = SND_RENDER;
	packet.arg1 = FALSE;
	packet.arg2 = bNowBank;
	PushCommand(&packet);

	/*
	 * 出力
	 */
	if(DrvWHG) {
		packet.cmd = SND_WHG_SETREG;
		packet.arg1 = (int)reg;
		packet.arg2 = (int)dat;
		PushCommand(&packet);
	}
}


void
wav_notify(BYTE no)
{
	int    i;
	int    j;
	struct SNDPushPacket packet;

	packet.cmd = SND_WAV_PLAY;
	packet.arg1 = (int) no;
	PushCommand(&packet);
}

void
beep_notify(void)
{
	struct SNDPushPacket packet;

	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}
	packet.cmd = SND_RENDER;
	packet.arg1 = (int)FALSE;
	packet.arg2 = bNowBank;
	PushCommand(&packet);
	//                AddSnd(FALSE, FALSE);
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
	struct SNDPushPacket packet;

	if (bTapeFlag == flag) {
		return;
	}

	if (bTapeMon) {
		packet.cmd = SND_RENDER;
		packet.arg1 = (int)FALSE;
		packet.arg2 = bNowBank;
		PushCommand(&packet);

		//            AddSnd(FALSE, FALSE);
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
	struct SNDPushPacket cmd;
	/*
	 * 初期化されていなければ、何もしない
	 */
	if (!DrvBeep) {
		return;
	}



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
			   if(bZero) {
				   cmd.cmd = SND_BZERO;
			   } else {
				   cmd.cmd = SND_RENDER;
			   }
			   cmd.arg1 = (int)FALSE;
			   cmd.arg2 = bNowBank;
			   PushCommand(&cmd);

			   //	                        AddSnd(FALSE, bZero);
		   }
		   return;
	  }

	  /*
	   * ここから演奏開始
	   */
	  if(bZero) {
		  cmd.cmd = SND_BZERO;
	  } else {
		  cmd.cmd = SND_RENDER;
	  }
	  cmd.arg1 = (int)TRUE;
	  cmd.arg2 = bNowBank;
	  PushCommand(&cmd);
	  //	        AddSnd(TRUE, bZero);

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
	if(DrvBeep != NULL) {
		p = DrvBeep->GetChunk(slot);
		len = p->alen / sizeof(Sint16);
		memcpy_add16(out, p->abuf, len);
		slots++;
	}
	//	if(DrvWav[0] != NULL) {
	//		p = DrvWav[0][slot].GetChunk();
	//		len = p->alen / sizeof(Sint16);
	//		memcpy_add16(out, p->abuf, len);
	//		slots++;
	//	}
	//	if(DrvWav[1] != NULL) {
	//		p = DrvWav[1][slot].GetChunk();
	//		len = p->alen / sizeof(Sint16);
	//		memcpy_add16(out, p->abuf, len);
	//		slots++;
	//	}
	//	if(DrvWav[2] != NULL) {
	//		p = DrvWav[2][slot].GetChunk();
	//		len = p->alen / sizeof(Sint16);
	//		memcpy_add16(out, p->abuf, len);
	//		slots++;
	//	}
	if(slots>0) {
		p = DrvBeep->GetChunk(slot);
		if(p) size = p->alen / sizeof(Sint16);
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
	int wsize;
//	int uChannels;
	int uStereo;
	int w;
	int tmp;

//	uStereo = uStereoOut %4;
//	if ((uStereo > 0) || bForceStereo) {
//		uChannels = 2;
//	} else {
//		uChannels = 1;
//	}
	bufsize = ((nSampleRate*  nSoundBuffer ) / 2 ) / 1000;
	if((start + size) > bufsize) { /* 大きすぎるのでサイズ切り詰める */
		wsize = (start + size) - bufsize;
	} else {
		wsize = size;
	}
	if(slot > SND_BUF) return 0;
	/* レンダリング */
	/* BEEP */
	if(DrvBeep != NULL) {
		w = DrvBeep->Render(start, wsize, slot, TRUE);
	}
#if 0
		if(DrvWav[0] != NULL) {
			tmp = DrvWav[0][slot].Render(start, wsize, FALSE);
			if(tmp < w) w = tmp;
		}
		if(DrvWav[1] != NULL) {
			tmp = DrvWav[1][slot].Render(start, wsize, FALSE);
			if(tmp < w) w = tmp;
		}
		if(DrvWav[2] != NULL) {
			tmp = DrvWav[2][slot].Render(start, wsize, FALSE);
			if(tmp < w) w = tmp;
		}
#endif
#if 1
//	if(bTapeMon) {
//		if(DrvCmt[slot] == NULL) break;
//		tmp = DrvCMT[slot].Render(start, wsize, FALSE);
//		if(tmp < w) w = tmp;
//	}

	if(DrvOPN != NULL) {
		tmp =DrvOPN[slot].Render(start, wsize, TRUE);
		if(tmp < w) w = tmp;
	}

//	if(DrvPsg != NULL) {
//		tmp = DrvPsg[slot].Render(start, wsize, FALSE);
//		if(tmp < w) w = tmp;
//	}


	if(whg_use) {
		if(DrvWHG != NULL) {
			tmp = DrvWHG[slot].Render(start, wsize, TRUE);
			if(tmp < w) w = tmp;
		}
	}
	if(thg_use) {
		if(DrvTHG != NULL) {
			tmp = DrvTHG[slot].Render(start, wsize, TRUE);
			if(tmp < w) w = tmp;
		}
	}
#endif
	return w;
}

static int
RenderThreadBZero(int start,int size,int slot)
{
	int bufsize;
	int wsize;
//	int uChannels;
//	int uStereo;
	int w;
	int tmp;

//	uStereo = uStereoOut %4;
//	if ((uStereo > 0) || bForceStereo) {
//		uChannels = 2;
//	} else {
//		uChannels = 1;
//	}
	bufsize = ((nSampleRate*  nSoundBuffer) / 2 ) / 1000;
	if((start + size) > bufsize) { /* 大きすぎるのでサイズ切り詰める */
		wsize = (start + size) - bufsize;
	} else {
		wsize = size;
	}
	if(slot > SND_BUF) return 0;
	/* レンダリング */
	/* BEEP */
	if(DrvBeep != NULL) {
		w = DrvBeep->BZero(start, wsize, slot, TRUE);
	}
#if 0
		if(DrvWav[0] != NULL) {
			tmp = DrvWav[0][slot].BZero(start, wsize, TRUE);
			if(tmp < w) w = tmp;
		}
		if(DrvWav[1] != NULL) {
			tmp = DrvWav[1][slot].BZero(start, wsize, TRUE);
			if(tmp < w) w = tmp;
		}
		if(DrvWav[2] != NULL) {
			tmp = DrvWav[2][slot].BZero(start, wsize, TRUE);
			if(tmp < w) w = tmp;
		}
#endif
#if 1
//	if(bTapeMon) {
//		if(DrvCmt[slot] == NULL) break;
//		tmp = DrvCMT[slot].BZero(start, wsize);
//		if(tmp < w) w = tmp;
//	}

	if(DrvOPN != NULL) {
		tmp =DrvOPN[slot].BZero(start, wsize, FALSE);
		if(tmp < w) w = tmp;
	}


	if(whg_use) {
		if(DrvWHG != NULL) {
			tmp = DrvWHG[slot].BZero(start, wsize, FALSE);
			if(tmp < w) w = tmp;
		}
	}
	if(thg_use) {
		if(DrvTHG != NULL) {
			tmp = DrvTHG[slot].BZero(start, wsize, FALSE);
			if(tmp < w) w = tmp;
		}
	}
#endif
	return w;
}

static void RenderPlay(int samples, int slot, BOOL play)
{
	int len;
	Mix_Chunk *c;
//	DWORD uStereo;
//	int uChannels;

	/*
	 * レンダリング: bFill = TRUEで音声出力
	 */
//	uStereo = nStereoOut % 4;
//	if ((uStereo > 0) || bForceStereo) {
//		uChannels = 2;
//	} else {
//		uChannels = 1;
//	}


	len = samples * uChannels * sizeof(Sint16);
	if(play) {
		if(DrvBeep != NULL) {
			c = DrvBeep->GetChunk(slot);
//			if(c) c->alen = len;
			Mix_Volume(0 + slot * 10, 128);
			Mix_PlayChannel(0 + slot * 10, c, 0);
		}
#if 0
		if(DrvWav[0] != NULL) {
			Mix_Volume(5 + slot * 10, 128);
			Mix_PlayChannel(5 + slot * 10, DrvWav[0][slot].GetChunk(), 0);
		}
		if(DrvWav[1] != NULL) {
			Mix_Volume(6 + slot * 10, 128);
			Mix_PlayChannel(6 + slot * 10, DrvWav[1][slot].GetChunk(), 0);
		}
		if(DrvWav[1] != NULL) {
			Mix_Volume(7 + slot * 10, 128);
			Mix_PlayChannel(7 + slot * 10, DrvWav[2][slot].GetChunk(), 0);
		}
#endif

#if 1
//		if(DrvPsg != NULL) {
//			if(bTapeMon) {
//				Mix_PlayChannel(0 + slot * 10, DrvPsg[slot].GetChunk(), 0);
//			}
//		}
		if(DrvOPN != NULL) {
			Mix_Volume(2 + slot * 10, 128);
			Mix_PlayChannel(2 + slot * 10, DrvOPN[slot].GetChunk(), 0);
		}
		if(DrvWHG != NULL) {
			if(whg_use) {
				Mix_Volume(3 + slot * 10, 128);
				Mix_PlayChannel(3 + slot * 10, DrvWHG[slot].GetChunk(), 0);
			}
		}
		if(DrvTHG != NULL) {
			if(thg_use) {
				Mix_Volume(4 + slot * 10, 128);
				Mix_PlayChannel(4 + slot * 10, DrvTHG[slot].GetChunk(), 0);
			}
		}
#endif
	}
//	SDL_Delay(100);
}

/*
 * bfill
 */
static int RenderThread(void *arg)
{
	int i,j;
	int samples;
//	int uChannels;
	BOOL bFill;
	int uSample = 0;
	int wsamples;
	int cmd;
	int wbank,wbankOld;
	int totalSamples;
	BYTE r;
	UINT uStereo;
	struct SNDPushPacket cmdarg;



	wbank = 0;
	wbankOld = 0;
	while(SndCond == NULL) {
		SDL_Delay(1);
	}
	while(SndMutex == NULL) {
		SDL_Delay(1);
	}

	while(1){
		/*
		 * メッセージキュー取り込み、判断
		 */
		// Wait(MessageQ);
		// Get(MessageQ);
		if(SndCond == NULL) {
			SDL_Delay(1);
			continue;
		}
		if(SndMutex == NULL) {
			SDL_Delay(1);
			continue;
		}
		SDL_mutexP(SndMutex);
		SDL_CondWait(SndCond, SndMutex);
		PullCommand(&cmdarg);
//		while(PullCommand(&cmdarg) == 0) {
//			SDL_Delay(1);
//		}
		cmd = cmdarg.cmd;
		switch(cmd) {
		case SND_BZERO: /* ARG: bFill */
		case SND_RENDER:
			bFill = (BOOL)cmdarg.arg1;
			wbank = cmdarg.arg2;
#if 1
			/*
			 * レンダリング: bFill = TRUEで音声出力
			 */
//			uStereo = nStereoOut % 4;
//			if ((uStereo > 0) || bForceStereo) {
//				uChannels = 2;
//			} else {
//				uChannels = 1;
//			}
			samples = (uRate *  uTick) / 1000;
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
			wbankOld = wbank;
//			wbankOld = (wbank==0)?1:0;

			if(bFill) {
				//				WavCaptureSub(Uint8 *out); /* ここでWAV書き込みやる */
				RenderPlay(uSample + samples, wbankOld, TRUE);
				dwSoundTotal = 0;
				uSample = 0;
			} else {
				uSample += samples;
			}
#endif

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
			do {
				SDL_Delay(1);
			} while(Mix_Playing(-1) != 0);
//			Mix_CloseAudio();
#if 0
			delete [] DrvWav;
			DrvWav[i] = NULL;
			/*
			 * 演奏のクリーンアップ
			 */
			delete [] DrvBeep;
			DrvBeep = NULL;
			delete [] DrvPSG;
			DrvPSG = NULL;
			delete [] DrvOPN;
			DrvOPN = NULL;
			delete [] DrvWHG;
			DrvWHG = NULL;
			delete [] DrvTHG;
			DrvTHG = NULL;
#endif
			/*
			 * スレッド終了(その他諸々の資源解放後)
			 */
			return 0;
			break;
		case SND_PSG_SETREG:
			DrvPSG[0].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			DrvPSG[1].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			break;
		case SND_OPN_SETREG:
			DrvOPN[0].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			DrvOPN[1].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			break;
		case SND_WHG_SETREG:
			DrvWHG[0].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			DrvWHG[1].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			break;
		case SND_THG_SETREG:
			DrvTHG[0].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			DrvTHG[1].SetReg((Uint8) cmdarg.arg1, (Uint8)cmdarg.arg2);
			break;


		case SND_PSG_READREG:
		case SND_OPN_READREG:
			//
			r = opn_reg[OPN_STD][cmdarg.arg1];
			*((BYTE *)cmdarg.arg3) = r;
			*((BYTE *)cmdarg.arg4) = TRUE;
			break;
		case SND_THG_READREG:
			//
			r = opn_reg[OPN_THG][cmdarg.arg1];
			*((BYTE *)cmdarg.arg3) = r;
			*((BYTE *)cmdarg.arg4) = TRUE;
			break;
		case SND_WHG_READREG:
			//
			r = opn_reg[OPN_WHG][cmdarg.arg1];
			*((BYTE *)cmdarg.arg3) = r;
			*((BYTE *)cmdarg.arg4) = TRUE;
			break;


		case SND_BEEP_SETFREQ:
			break;
		case SND_WAV_PLAY:
			if(cmdarg.arg1 == 0) {
				WavChunk[0]->volume = 127;
				Mix_PlayChannel(CH_WAV_RELAY_ON, WavChunk[0], 0);
			} else if(cmdarg.arg1 == 1) {
				WavChunk[1]->volume = 127;
				Mix_PlayChannel(CH_WAV_RELAY_OFF, WavChunk[1], 0);
			} else if(cmdarg.arg1 == 2) {
				WavChunk[2]->volume = 127;
				Mix_PlayChannel(CH_WAV_FDDSEEK, WavChunk[2], 0);
			} else if(cmdarg.arg1 == SOUND_STOP) {
				Mix_HaltChannel(CH_WAV_RELAY_ON);
				Mix_HaltChannel(CH_WAV_RELAY_OFF);
				Mix_HaltChannel(CH_WAV_FDDSEEK);
			}
			break;
		default:
			SDL_Delay(1);
			break;
		}
//		SDL_mutexP(SndMutex);

	}
}



/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010     K.Ohta
 *
 *	[ SDL サウンド ]
 *   20100115 Initial
 *   20100117 SDL_Mixerを使うように変更した
 */
#ifdef _XWIN

#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else
#include <linux/soundcard.h>
#endif

#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "opn.h"
//#include "whg.h"
//#include "thg.h"
#include "tapelp.h"
#include "cisc.h"
#include "opna.h"
#include "psg.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "sdl_snd.h"


/*
 *	グローバル ワーク
 */
UINT nSampleRate;						/* サンプリングレート */
UINT nSoundBuffer;						/* サウンドバッファサイズ */
UINT nStereoOut;						/* 出力モード */
BOOL bFMHQmode;							/* FM高品質合成モード */
BOOL bForceStereo;						/* 強制ステレオ出力 */
UINT nBeepFreq;							/* BEEP周波数 */
BOOL bTapeMon;							/* テープ音モニタ */
int hWavCapture;						/* WAVキャプチャハンドル */
BOOL bWavCapture;						/* WAVキャプチャ開始 */
UINT uClipCount;						/* クリッピングカウンタ */
#ifdef FDDSND
UINT uSeekVolume;						/* シーク音量 */
#endif
int iTotalVolume;                                              /* 全体ボリューム */

/*
 *	スタティック ワーク
 */

/*
 * SDL -マルチスレッド化するために…
 */
static SDL_sem *musicSem; /* マルチスレッドで同期するためのセマフォ */
static SDL_AudioSpec *audiospec;
static SDL_AudioSpec *audio_obtained;
static SDL_Thread *playThread;
static int uProcessCount;
static SDL_TimerID uTid;
static UINT localTick;   /* VM実行時間 */
static DWORD dwExecLocal;



static BOOL bFirstSndProcess;                                  /* 最初のSndProcess */ 
static BOOL bDataReady;
static BYTE *lpdsb;						/* DSP用サウンド作成バッファ */
static DWORD dwPlayC;					/* DSP用サウンド作成バッファ内の再生位置 */
static BOOL bNowBank;
static DWORD *lpsbuf;


/* 音声合成用のバッファ */
static BYTE *sndSrcBuf[11]; /* バッファは別々にとる */
static Mix_Chunk *sndDstBuf[11]; /* バッファは別々にとる */
static BYTE *opnBuf[3];
static BYTE *psgBuf;

static UINT uBufSize;					/* サウンドバッファサイズ */
static UINT uRate;						/* 合成レート */
static UINT uTick;						/* 半バッファサイズの長さ */
static BOOL bMode;						/* FM高品質合成モード */
static UINT nStereo;					/* 出力モード */
static UINT uSample;					/* サンプルカウンタ */
static UINT uBeep;						/* BEEP波形カウンタ */
static FM::OPN *pOPN[3];				/* OPNデバイス */
static int nScale[3];					/* OPNプリスケーラ */
static BYTE uCh3Mode[3];				/* OPN Ch.3モード */
static BOOL bInitFlag;					/* 初期化フラグ */
static WORD *pWavCapture;				/* キャプチャバッファ(64KB) */
static UINT nWavCapture;				/* キャプチャバッファ実データ */
static DWORD dwWavCapture;				/* キャプチャファイルサイズ */
static WORD uChannels;					/* 出力チャンネル数 */
static BOOL bBeepFlag;					/* BEEP出力 */

static BOOL bTapeFlag;					/* 現在のテープ出力状態 */
static BOOL bTapeFlag2;					/* 前回のテープ出力状態 */
static BYTE uTapeDelta;					/* テープ波形補間カウンタ */

#ifdef FDDSND
/*
 *	スタティック ワーク (WAV再生)
 */
static struct _WAVDATA {
	short *p;							/* 波形データポインタ */
	DWORD size;							/* データサイズ(サンプル数) */
	DWORD freq;							/* サンプリング周波数 */
} Wav[3];

//static struct _WAVPLAY {
//	BOOL	bPlay;						/* WAV再生フラグ */
//	DWORD	dwWaveNo;					/* WAVでーたなんばー */
//	DWORD	dwCount1;					/* WAVでーたかうんた(整数部) */
//	DWORD	dwCount2;					/* WAVでーたかうんた(小数部) */
//	DWORD	dwCount3;					/* WAV再生かうんた */
//} WavP[SNDBUF];
/* Wav 演奏フラグ */
static BOOL bWavPlay[XM7_SND_END - XM7_SND_WAV_RELAY_ON]; 
static BYTE *WavBuf[3]; 
static SDL_AudioSpec *WavSpec[3];
static Uint32 WavLen[3];

static char *WavName[] = {				/* WAVファイル名 */
	"RELAY_ON.WAV",
	"RELAY_OFF.WAV",
	"FDDSEEK.WAV",
#if 0
	"HEADUP.WAV",
	"HEADDOWN.WAV"
#endif
};
#endif

/*
 * WAVファイル作成用構造体
 */
typedef struct { 
    WORD  wFormatTag; 
    WORD  nChannels; 
    DWORD nSamplesPerSec; 
    DWORD nAvgBytesPerSec; 
    WORD  nBlockAlign; 
    WORD  wBitsPerSample; 
    WORD  cbSize; 
} WAVEFORMATEX; 

static WORD WAVE_FORMAT_PCM=0x0001;

/*
 *	アセンブラ関数のためのプロトタイプ宣言→x86依存一度外す
 */
#ifdef __cplusplus
extern "C" {
#endif
void (*CopySoundBuffer)(DWORD *src, WORD *dst, int count);
//extern void CopySndBufMMX(DWORD *src, WORD *dst, int count);
  //extern void CopySndBuf(DWORD *src, WORD *dst, int count);
#ifdef __cplusplus
}
#endif

//static void WaveSnd();
static void FASTCALL WaveSnd(int32 *buf, int samples);
/*
 * サウンドバッファへのコピー
 * XM7内のサウンドレンダリングは32bitで格納されてるので
 * 上位)16bitを切り捨てる
 * 場合に依っては高速化が必要。
 */

//extern void CopySndBufMMX(DWORD *from, WORD *to, int size);

static void CopySoundBufferGeneric(DWORD *from, WORD *to, int size)
{
  int i,j;
  int32 *p = (int32 *)from;
  int16 *t = (int16 *)to;
  int32 tmp1;

  if(p == NULL) return;
  if(t == NULL) return;
  i = (size / 4) *4;
    for(j=0 ; j<i ;j+=4){

      tmp1 = p[j];
      if(tmp1 > XM7_PCM_MAX_VOLUME) tmp1 = XM7_PCM_MAX_VOLUME;
      if(tmp1 < -XM7_PCM_MAX_VOLUME) tmp1 = -XM7_PCM_MAX_VOLUME;
      t[j] = (int16)(tmp1 & 0x0000ffff);

      tmp1 = p[j+1];
      if(tmp1 > XM7_PCM_MAX_VOLUME) tmp1 = XM7_PCM_MAX_VOLUME;
      if(tmp1 < -XM7_PCM_MAX_VOLUME) tmp1 = -XM7_PCM_MAX_VOLUME;
      t[j+1] = (int16)(tmp1 & 0x0000ffff);

      tmp1 = p[j+2];
      if(tmp1 > XM7_PCM_MAX_VOLUME) tmp1 = XM7_PCM_MAX_VOLUME;
      if(tmp1 < -XM7_PCM_MAX_VOLUME) tmp1 = -XM7_PCM_MAX_VOLUME;
      t[j+2] = (int16)(tmp1 & 0x0000ffff);

      tmp1 = p[j+3];
      if(tmp1 > XM7_PCM_MAX_VOLUME) tmp1 = XM7_PCM_MAX_VOLUME;
      if(tmp1 < -XM7_PCM_MAX_VOLUME) tmp1 = -XM7_PCM_MAX_VOLUME;
      t[j+3] = (int16)(tmp1 & 0x0000ffff);

    }
    j = size - i;
    for(j=0; j<i ; j++) {
      tmp1 = p[j];
      if(tmp1 > XM7_PCM_MAX_VOLUME) tmp1 = XM7_PCM_MAX_VOLUME;
      if(tmp1 < -XM7_PCM_MAX_VOLUME) tmp1 = -XM7_PCM_MAX_VOLUME;
      t[j] = (int16)(tmp1 & 0x0000ffff);
    }

}



/*
 * 実時間を測定(timeGetTime互換関数)
 */
static DWORD  timeGetTime(void) {
  struct timeval t;
  
  return SDL_GetTicks();

	
}



/*
 *	初期化
 */
void FASTCALL InitSnd(void)
{
  int i;

         /* ワークエリア初期化 */
	nSampleRate = 44100;
	nSoundBuffer = 100;
	bFMHQmode = FALSE;
	nBeepFreq = 1200;
	nStereoOut = 0;
	bForceStereo = FALSE;
	bTapeMon = TRUE;
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
	nStereo = 0;
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
	bFirstSndProcess = TRUE;
        musicSem = NULL;
#ifdef FDDSND
	uSeekVolume = 128;
        for(i=0; i< (XM7_SND_END - XM7_SND_WAV_RELAY_ON + 1) ; i++) {
          bWavPlay[i] = FALSE;
        }
#endif

	/* SDL用変数 */

        for(i = 0; i < XM7_SND_END ; i++) {
          sndSrcBuf[i] = NULL;
          sndDstBuf[i] = NULL;
        }
        //CopySoundBuffer = CopySndBufMMX;
       CopySoundBuffer = CopySoundBufferGeneric;
#ifdef FDDSND   
        InitFDDSnd();
#endif
        SDL_InitSubSystem(SDL_INIT_AUDIO);
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanSnd(void)
{
	int i;

	//SDL_KillThread(playThread);
	/* サウンド停止 */
	StopSnd();
        for(i = XM7_SND_FMBOARD ; i < XM7_SND_END + 1 ; i++ ) {
          if(sndDstBuf[i]) {
            free(sndDstBuf[i]); /* CloseしたらFreeChunk不要 ? */
            sndDstBuf[i] = NULL;
          }
        }

	//SDL_LockAudio();
	//SDL_CloseAudio();

	/* OPNを解放 */
	for (i=0; i< 3; i++) {
          if (pOPN[i]) {
			delete pOPN[i];
			pOPN[i] = NULL;
          }
	}


        for(i = 0; i < 3 ; i++) {
          if(opnBuf[i] != NULL) {
            free(opnBuf[i]);
          }
          opnBuf[i] = NULL;
        }
        if(psgBuf != NULL) free(psgBuf);
        psgBuf = NULL;

        for(i = 0 ; i <XM7_SND_END ; i++){
          if(sndSrcBuf[i]) {
            free(sndSrcBuf[i]);
            sndSrcBuf[i] = NULL;
          }
        }
	/* サウンド作成バッファを解放 */
	if (lpdsb) {
		free(lpdsb);
		lpdsb = NULL;
	}

	/* サウンド作成バッファを解放 */
	if (lpsbuf) {
		free(lpsbuf);
		lpsbuf = NULL;
	}
	/* uRateをクリア */
	uRate = 0;
        Mix_CloseAudio();
#if 1 /* WAVキャプチャは後で作る */
	/* キャプチャ関連 */
	if (hWavCapture >= 0) {
		CloseCaptureSnd();
	}
	if (pWavCapture) {
		free(pWavCapture);
		pWavCapture = NULL;
	}
	hWavCapture = -1;
	bWavCapture = FALSE;
#endif
}        

#ifdef FDDSND

#if 0
/*
 *	WAVEファイル読み込み (16ビットモノラルデータ専用)
 */
static BOOL FASTCALL LoadWav(char *fname, struct _WAVDATA *wav)
{
	WAVEFORMATEX wfex;
	BYTE buf[16];
	DWORD filSize;
	DWORD hdrSize;
	DWORD datSize;
	int fileh;

	ASSERT(fname);
	ASSERT(wav);

	/* ファイルオープン */
	fileh = file_open(fname, OPEN_R);
	if (fileh < 0) {
		return FALSE;
	}

	/* RIFFヘッダチェック */
	file_read(fileh, buf, 4);
	file_read(fileh, (BYTE *)&filSize, 4);
	buf[4] = '\0';
	if (strcmp((char *)buf, "RIFF")) {
		file_close(fileh);
		return FALSE;
	}
	filSize += 8;

	/* WAVEヘッダチェック */
	file_read(fileh, buf, 8);
	file_read(fileh, (BYTE *)&hdrSize, 4);
	buf[8] = '\0';
	if (strcmp((char *)buf, "WAVEfmt ")) {
		file_close(fileh);
		return FALSE;
	}
	hdrSize += (12 + 8);

	/* WAVEFORMATEXチェック */
	file_read(fileh, (BYTE *)&wfex, sizeof(wfex));
	if ((wfex.wFormatTag != WAVE_FORMAT_PCM) ||
		(wfex.nChannels != 1) || (wfex.wBitsPerSample != 16)) {
		/* 16ビットモノラル・リニアPCM以外は不可 */
		file_close(fileh);
		return FALSE;
	}

	/* dataチャンク検索 */
	while (hdrSize < filSize) {
		/* チャンクヘッダ読み込み */
		file_seek(fileh, hdrSize);
		file_read(fileh, buf, 4);
		file_read(fileh, (BYTE *)&datSize, 4);
		buf[4] = '\0';

		/* 次のチャンクヘッダオフセットを計算 */
		hdrSize += (datSize + 8);

		if (strcmp((char *)buf, "data") == 0) {
			/* dataチャンク読み込み */
			wav->size = datSize / 2;
			wav->freq = wfex.nSamplesPerSec;
			wav->p = (short *)malloc(datSize);
			if (wav->p == NULL) {
				file_close(fileh);
				return FALSE;
			}
			if (!file_read(fileh, (BYTE *)wav->p, wav->size)) {
				file_close(fileh);
				free(wav->p);
				wav->p = NULL;
				return FALSE;
			}
			file_close(fileh);
			return TRUE;
		}
	}

	/* dataチャンク発見できず */
	file_close(fileh);
	return FALSE;
}
#endif

/*
 *	FDDサウンド 初期化
 */
void FASTCALL InitFDDSnd(void)
{
  int i;


  for (i=0; i< (XM7_SND_WAV_FDD - XM7_SND_WAV_RELAY_ON + 1) ;i++) {
    /* ワーク初期化 */
    WavBuf[i] = NULL;
    WavSpec[i] = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
    if(WavSpec[i] == NULL) {
      printf("Warning: Unable to allocate WAV#%d (%s).\n", i, WavName[i]);
      continue;
    }
    memset(WavSpec[i], 0, sizeof(SDL_AudioSpec));
    /* とりあえずWAV読み込む。 OPENしたままになるので注意。 */
    if(SDL_LoadWAV(WavName[i], WavSpec[i], &WavBuf[i],&WavLen[i]) == NULL) {
      printf("Warning: Unable to load WAV#%d (%s).\n", i, WavName[i]);
      free(WavSpec[i]);
      WavSpec[i]=NULL;
    }
    printf("Loaded WAV #%d (%s). ADDR=%08x samples = %d\n", i, WavName[i] , WavBuf[i], WavSpec[i]->samples);
  } 
      /* WAVファイル読み込み */
}

/*
 *	FDDサウンド クリーンアップ
 */
void FASTCALL CleanFDDSnd(void)
{
  int i;
  for (i=0; i< (XM7_SND_WAV_FDD - XM7_SND_WAV_RELAY_ON +1) ;i++) {
    /* WAVファイル読み込み */
    if(sndDstBuf[i + XM7_SND_WAV_RELAY_ON] != NULL) {
      free(sndDstBuf[i + XM7_SND_WAV_RELAY_ON]);
      sndDstBuf[i + XM7_SND_WAV_RELAY_ON] = NULL;
    }
    /* ワーク初期化 */
    if(WavBuf[i] != NULL) {
      SDL_FreeWAV(WavBuf[i]);
      WavBuf[i] = NULL;
    }
    if(WavSpec[i] != NULL) {
      free(WavSpec[i]);
      WavSpec[i] = NULL;
    }
  }


}
#endif

/*
 *	レジスタ設定
 */
static void FASTCALL SetReg(FM::OPN *pOPN, BYTE *reg)
{
	int i;

	/* PSG */
	for (i=0; i<16; i++) {
		pOPN->SetReg((BYTE)i, reg[i]);
	}

	/* FM音源キーオフ */
	for (i=0; i<3; i++) {
		pOPN->SetReg(0x28, (BYTE)i);
	}

	/* FM音源レジスタ */
	for (i=0x30; i<0xb4; i++) {
		pOPN->SetReg((BYTE)i, reg[i]);
	}

	/* FM音源動作モード */
	pOPN->SetReg(0x27, reg[0x27] & 0xc0);
}

/* 演奏用コールバック　*/

  

/*
 *	セレクト
 */
BOOL FASTCALL SelectSnd(void)
{
	int fmt;
	int channel;
	int freq;
	int arg;
	int i;
	DWORD bytes;
        SDL_AudioCVT cvt;
        

	/* 起動フラグ立てる */
	bInitFlag = TRUE;
	uTid = 0;

	/* パラメータを設定 */
	uRate = nSampleRate;
	uTick = nSoundBuffer;
	bMode = bFMHQmode;
	nStereo = nStereoOut;
	dwExecLocal = dwExecTotal;
	bNowBank = 0;
	uTapeDelta = 0;
	if ((nStereo > 0) || bForceStereo) {
		uChannels = 2;
	}
	else {
		uChannels = 1;
	}
#ifdef FDDSND
        for(i=0; i< XM7_SND_END - XM7_SND_WAV_RELAY_ON ; i++) {
          bWavPlay[i] = FALSE;
        }
#endif
	
	/* rate==0なら、何もしない */
	if (uRate == 0) {
		return TRUE;
	}
	/* SDL用変数領域の設定 */

	/* SDL用サウンドバッファを作成 */
	bytes = (uRate * 2 * uChannels * uTick ) / 1000;
	bytes += (DWORD)7;
	bytes &= (DWORD)0xfffffff8;	/* 8バイト境界 */
	uBufSize = bytes;



        if(musicSem == NULL) {
                 musicSem = SDL_CreateSemaphore(0);
        } 

	lpdsb = (BYTE *)malloc(uBufSize);
	if (lpdsb == NULL) {
		return FALSE;
	}
	memset(lpdsb, 0, uBufSize);
        for(i = XM7_SND_FMBOARD ; i <= XM7_SND_TAPE ; i++ )
          {
            sndSrcBuf[i] = (BYTE *)malloc(uBufSize);
            if(sndSrcBuf[i] == NULL) {
              printf("Err: Can't alloc sound buffer %d \n",i);
              return FALSE;
            }
            memset(sndSrcBuf[i], 0, uBufSize);
          }
        /* WAVEバッファ */
        for(i = XM7_SND_WAV_RELAY_ON; i <= XM7_SND_END ; i++ ) {
          sndSrcBuf[i] = NULL;
        }

        /* opnバッファ */
        for(i = 0 ; i < 3 ; i++){
          opnBuf[i] = (BYTE *)malloc(uBufSize); /* OPN合成は32bit */
          if(opnBuf[i] == NULL) {
            printf("Err: Can't alloc opn[%d]. \n",i);
            return FALSE;
          }
          memset(opnBuf[i], 0, uBufSize);
        }

        psgBuf = (BYTE *)malloc(uBufSize); /* PSG合成は32bit */
        if(psgBuf == NULL) {
            printf("Err: Can't alloc psg. \n");
            return FALSE;
        }          
        memset(psgBuf, 0, uBufSize);
	/* サウンドバッファを作成(DSP用バッファの半分の時間で、DWORD) */
	lpsbuf = (DWORD *)malloc(uBufSize);
	if (lpsbuf == NULL) {
          return FALSE;
	}

	memset(lpsbuf, 0, uBufSize);

	/* サンプルカウンタ、サウンド時間をクリア */
	uSample = 0;
	dwSoundTotal = 0;
	uClipCount = 0;

	/* OPNデバイス(標準)を作成 */
	pOPN[0] = new FM::OPN;
	pOPN[0]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
	pOPN[0]->Reset();
	pOPN[0]->SetReg(0x27, 0);

	/* OPNデバイス(WHG)を作成 */
	pOPN[1] = new FM::OPN;
	pOPN[1]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
	pOPN[1]->Reset();
	pOPN[1]->SetReg(0x27, 0);

	/* OPNデバイス(THG)を作成 */
	pOPN[2] = new FM::OPN;
	pOPN[2]->Init(OPN_CLOCK * 100, uRate, bMode, NULL);
	pOPN[2]->Reset();
	pOPN[2]->SetReg(0x27, 0);

	/* 再セレクトに備え、レジスタ設定 */
	nScale[0] = 0;
	nScale[1] = 0;
	nScale[2] = 0;
	opn_notify(0x27, 0);
	whg_notify(0x27, 0);
	thg_notify(0x27, 0);
	SetReg(pOPN[0], opn_reg[OPN_STD]);
	SetReg(pOPN[1], opn_reg[OPN_WHG]);
	SetReg(pOPN[2], opn_reg[OPN_THG]);

	/* キャプチャ関連 */
	if (!pWavCapture) {
		pWavCapture = (WORD *)malloc(sizeof(WORD) * 0x8000);
	}
	ASSERT(hWavCapture == -1);
	ASSERT(!bWavCapture);
	/* SDL用に仕様変更 */
        //Mix_Init(0);
        if(Mix_OpenAudio(uRate, AUDIO_S16SYS, uChannels, uBufSize) == -1) {
          printf("Warning: Audio can't initialize!\n"); /* 後でダイアログにする */
          return FALSE;
        }
        Mix_AllocateChannels(XM7_SND_MAIN + 1);
        /* テンプレ作成 */
        for(i = XM7_SND_FMBOARD ; i < XM7_SND_END + 1 ; i++ ) {
          sndDstBuf[i] = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
          if(sndDstBuf[i] == NULL) {
            printf("Err: Audio buffer can't allocate!\n");
            return FALSE;
          }
          memset(sndDstBuf[i], 0 , sizeof(Mix_Chunk));
        }
        /* 各バッファとChunkの紐づけ */
        /* OPN */
        for(i = XM7_SND_FMBOARD; i < (XM7_SND_FM_THG - XM7_SND_FMBOARD + 1) ; i++) {
          sndDstBuf[i]->allocated = TRUE;
          sndDstBuf[i]->abuf = (Uint8 *)opnBuf[i];
          sndDstBuf[i]->alen = uBufSize * 2;
          sndDstBuf[i]->volume = MIX_MAX_VOLUME;
        }
        /* PSG */
        i = XM7_SND_PSG;
        sndDstBuf[i]->allocated = TRUE;
        sndDstBuf[i]->abuf = (Uint8 *)psgBuf;
        sndDstBuf[i]->alen = uBufSize * 2;
        sndDstBuf[i]->volume = MIX_MAX_VOLUME;
        /* BEEP */
        i = XM7_SND_BEEP;
        sndDstBuf[i]->allocated = TRUE;
        sndDstBuf[i]->abuf = sndSrcBuf[i]; 
        sndDstBuf[i]->alen = uBufSize;
        sndDstBuf[i]->volume = MIX_MAX_VOLUME;
        /* Tape */
        i = XM7_SND_TAPE;
        sndDstBuf[i]->allocated = TRUE;
        sndDstBuf[i]->abuf = sndSrcBuf[i]; 
        sndDstBuf[i]->alen = uBufSize;
        sndDstBuf[i]->volume = MIX_MAX_VOLUME;
        /* Main */
        i = XM7_SND_MAIN;
        sndDstBuf[i]->allocated = TRUE;
        sndDstBuf[i]->abuf = lpdsb; 
        sndDstBuf[i]->alen = uBufSize;
        sndDstBuf[i]->volume = MIX_MAX_VOLUME;

#ifdef FDDSND
        /* 予めOpenしてあったWAVデータの読み込み */
        for(i = 0; i < (XM7_SND_WAV_FDD - XM7_SND_WAV_RELAY_ON + 1 ) ; i++ ){
          if((WavBuf[i]  != NULL) && (WavSpec[i] != NULL)) {
            /* Wavデータのレート変換 */
            if(SDL_BuildAudioCVT(&cvt, WavSpec[i]->format, WavSpec[i]->channels, WavSpec[i]->freq, AUDIO_S16SYS, uChannels, uRate) == 0) {
              printf("Warn: Unable to use WAV #%d (%s)\n", i, WavName[i]);
              continue;
            }
            printf("BuildCVT #%d (%s) ...sample = %d ch = %d freq = %d \n", i, WavName[i], WavSpec[i]->samples, WavSpec[i]->channels, WavSpec[i]->freq);
            sndSrcBuf[XM7_SND_WAV_RELAY_ON + i] = (BYTE *)malloc(96000 * uChannels * 2 * sizeof(WORD)); /* WAVバッファを最大レンジで取る */

            if(sndSrcBuf[XM7_SND_WAV_RELAY_ON + i] == NULL) {
              printf("Warn: Unable to use WAV #%d (%s)\n", i, WavName[i]);
              continue;
            }
            printf("CVT #%d (%s)...", i, WavName[i]);
            memset(sndSrcBuf[XM7_SND_WAV_RELAY_ON + i], 0, WavSpec[i]->samples * uChannels * sizeof(WORD));
            memcpy(sndSrcBuf[XM7_SND_WAV_RELAY_ON + i], WavBuf[i], WavLen[i]);
            cvt.buf = sndSrcBuf[XM7_SND_WAV_RELAY_ON + i];
            cvt.len = WavLen[i];
            if(SDL_ConvertAudio(&cvt)<0) {
              printf("Warn: Unable to use WAV #%d (%s)\n", i, WavName[i]);
              free(sndSrcBuf[XM7_SND_WAV_RELAY_ON + i]);
              //sndSrcBuf[XM7_SND_WAV_RELAY_ON + i] = NULL;
              continue;
            }
            printf("...Done.\n");
            if(sndDstBuf[XM7_SND_WAV_RELAY_ON + i] != NULL) {
              sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->allocated = 1;
              sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->abuf = sndSrcBuf[XM7_SND_WAV_RELAY_ON + i];
              sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->alen = WavSpec[i]->samples * uChannels * sizeof(WORD);
              sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->volume = MIX_MAX_VOLUME - 1;
            } else {
              
              if(sndDstBuf[XM7_SND_WAV_RELAY_ON + i] != NULL) {
                sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->allocated = 0;
                sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->abuf = NULL;
                sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->alen = 0;
                sndDstBuf[XM7_SND_WAV_RELAY_ON + i]->volume = MIX_MAX_VOLUME - 1;
              }
            }
          }
            
        }
#endif

	/* サウンドスタート */
	PlaySnd();

	return TRUE;
}

/*
 *	適用
 */
void FASTCALL ApplySnd(void)
{
	/* 起動処理時は、リターン */
	if (!bInitFlag) {
		return;
	}

	/* パラメータ一致チェック */
	if ((uRate == nSampleRate) && (uTick == nSoundBuffer) &&
		(bMode == bFMHQmode) && (nStereo == nStereoOut)) {
		return;
	}

	printf("Apply!\n");

	/* 既に準備ができているなら、解放 */
	if (uRate != 0) {
		CleanSnd();

	}

	/* 再セレクト */
	SelectSnd();
}


/*
 *	演奏開始
 */
void FASTCALL PlaySnd()
{
  int i;
	/* サンプルカウンタ、サウンド時間をクリア */
	uSample = 0;
	dwSoundTotal = 0;
	uClipCount = 0;
	dwPlayC = 0;
	uProcessCount = 0;
	dwExecLocal = dwExecTotal;
	bDataReady = FALSE;
	iTotalVolume = SDL_MIX_MAXVOLUME - 1; /*ここで音量設定する必要があるか?*/
	memset(lpdsb, 0x00, uBufSize);
	SDL_SemPost(musicSem);
            /* 変換できたので読み込む */
}

/*
 *	演奏停止
 */
void FASTCALL StopSnd()
{
  int freq;
  Uint16 format;
  int channels;
  int ret;
  //if(musicSem) {
  //  SDL_SemWait(musicSem);
  //}
  do {
    Mix_CloseAudio();
    ret = Mix_QuerySpec(&freq, &format, &channels);
  } while(ret != 0);
   if(musicSem) SDL_SemPost(musicSem);        

}

/*
 *	BEEP合成
 */
static void BeepSnd(int32 *sbuf, int samples)
{
	int sf;
	int i;
        int32 *buf = (int32 *)sbuf;
        Mix_Chunk chunk;
 
	/* BEEP音出力チェック */
	if (!bBeepFlag) {
		return;
	}

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		/* 矩形波を作成 */
		sf = (int)(uBeep * nBeepFreq * 2);
		sf /= (int)uRate;

		/* 偶・奇に応じてサンプル書き込み */
		if (uChannels == 1) {
			if (sf & 1) {
				*buf++ += 0x0800;
			}
			else {
				*buf++ += -0x0800;
			}
		}
		else {
			if (sf & 1) {
				*buf++ += 0x0800;
				*buf++ += 0x0800;
			}
			else {
				*buf++ += -0x0800;
				*buf++ += -0x0800;
			}
		}

		/* カウンタアップ */
		uBeep++;
		if (uBeep >= uRate) {
			uBeep = 0;
		}
	}
                                  

}

/*
 *	テープ合成 (V3.1)
 *               SDL_Mixer対応:16bit仕様に
 */
 static void TapeSnd(int32 *sbuf, int samples)
{
  DWORD dat;
  int i;
  int tmp;
  Mix_Chunk chunk;
  int32 *buf = (int32 *)sbuf;
        

	/* テープ出力チェック */
	if (!tape_motor || !bTapeMon) {
		return;
	}

	/* 波形分割数を求める */
	if ((uRate == 48000) || (uRate == 96000)) {
		tmp = (uRate * 5) / 48000;
	}
	else {
		tmp = (uRate * 4) / 44100;
	}

	/* 出力状態が変化した場合、波形補間を開始する */
	if (bTapeFlag != bTapeFlag2) {
		if (!uTapeDelta) {
			uTapeDelta = 1;
		}
		else {
			uTapeDelta = (BYTE)(tmp - uTapeDelta + 1);
		}
	}

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		if (uTapeDelta) {
			/* 波形補間あり */
			dat = (0x1000 / tmp) * uTapeDelta;
			if (bTapeFlag) {
				dat = dat - 0x0801;
			}
			else {
				dat = 0x0800 - dat;
			}

			uTapeDelta ++;
			if (uTapeDelta > tmp) {
				uTapeDelta = 0;
			}
		}
		else {
			/* 波形補間なし */
			if (bTapeFlag) {
				dat = 0x07ff;
			}
			else {
				dat = -0x0800;
			}
		}

                /* BIG ENDIANの場合にこれでいいのか確認 */
		*buf++ += (Uint16 )dat; /* 音量小さすぎないか */
		if (uChannels == 2) {
                                         *buf++ += (Uint16)dat;
		}
	}
	/* 現在のテープ出力状態を保存 */
	bTapeFlag2 = bTapeFlag;

}

/*
 *	WAVデータ合成 (FDD/CMT)
 */
#ifdef FDDSND
#if 0
static void FASTCALL WaveSnd(int32 *buf, int samples)
{
	int i;
	int j;
	short dat;

	/* サンプル書き込み */
	for (i=0; i<samples; i++) {
		for (j=0; j<SNDBUF; j++) {
			if (WavP[j].bPlay) {
				dat = Wav[WavP[j].dwWaveNo].p[WavP[j].dwCount1];
				dat = (short)(((int)dat * uSeekVolume) >> 8);
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

#endif

static void FASTCALL WaveSnd(int32 *buf, int samples)
{
	int i;
	int j;
	short dat;

	/* サンプル書き込み */
       for (i=0; i<samples; i++) {
          for (j=0; j<(XM7_SND_WAV_FDD -XM7_SND_WAV_RELAY_ON + 1); j++) {
            if (bWavPlay[j]) {
                         dat = *((int16*)sndSrcBuf[XM7_SND_WAV_RELAY_ON] + j);
                          dat = (int16)(((int)dat * uSeekVolume) >> 8);
                          *buf++ += (int32)dat;
                          if (uChannels == 2) {
                            *buf++ += (int32)dat;
                          }
            }
          }
        }
}

#endif /* FDD */
#if 1
/*
 *	サウンド作成バッファへ追加
 */
static int FASTCALL AddSnd(BOOL bFill, BOOL bZero)
{
        int samples;
	int i;
	int retval;
	DWORD *q;
        WORD *p;

	/* OPNデバイスが作成されていなければ、何もしない */
	if (!pOPN[2]) {
		return 0;
	}


	/* bFillの場合のサンプル数(モノラル2byte/sample・ステレオ4byte/sample) */
	samples = (uBufSize >> 2) / uChannels;
	samples -= uSample;

	/* !bFillなら、時間から計測 */
	if (!bFill) {
		/* 時間経過から求めた理論サンプル数 */
		/* 計算結果がオーバーフローする問題に対策 2002/11/25 */
		i = (uRate / 25);
		i *= dwSoundTotal;
		i /= 40000;

		/* uSampleと比較、一致していれば何もしない */
		if (i <= (int)uSample) {
			return 0;
		}

		/* uSampleとの差が今回生成するサンプル数 */
		i -= (int)(uSample);

		/* samplesよりも小さければ合格 */
		if (i <= samples) {
			samples = i;
		}
	}

        if(musicSem) SDL_SemWait(musicSem);        
	/* バッファを求める */
	q = &lpsbuf[uSample * uChannels];
        
	/* ミキシング */

	memset(q, 0, sizeof(DWORD) * samples * uChannels);
	retval = samples;
	//printf("Gen: %d %d\n", samples, SDL_GetTicks());
	if (!bZero) {
		if (uChannels == 1) {
			/* モノラル */
			if (pOPN[0]) {
				pOPN[0]->Mix((int32*)q, samples);
			}
			if (whg_use) {
				pOPN[1]->Mix((int32*)q, samples);
			}
			if (thg_use) {
				pOPN[2]->Mix((int32*)q, samples);
			}
			else if (fm7_ver == 1) {
				pOPN[2]->psg.Mix((int32*)q, samples);
			}
		}
		else {
			/* ステレオ */
			if (!whg_use && !thg_use) {
				pOPN[0]->Mix2((int32*)q, samples, 16, 16);
				if (fm7_ver == 1) {
					pOPN[2]->psg.Mix2((int32*)q, samples, 16, 16);
				}
			}
			else {
				/* WHGまたはTHGを使用中 */
				switch (nStereo) {
					/* モノラル(強制ステレオ) */
					case 0:
						pOPN[0]->Mix2((int32*)q, samples, 16, 16);
						if (whg_use) {
							pOPN[1]->Mix2((int32*)q, samples, 16, 16);
						}
						if (thg_use) {
							pOPN[2]->Mix2((int32*)q, samples, 16, 16);
						}
						else if (fm7_ver == 1) {
                                                  pOPN[2]->psg.Mix2((int32*)q, samples, 16, 16);
						}
						break;

					/* ステレオ */
					case 1:
						pOPN[0]->Mix2((int32*)q, samples, 23, 9);
						if (whg_use) {
							pOPN[1]->Mix2((int32*)q, samples, 9, 23);
						}
						if (thg_use) {
							pOPN[2]->Mix2((int32*)q, samples, 16, 16);
						}
						else if (fm7_ver == 1) {
							pOPN[2]->psg.Mix2((int32*)q, samples, 16, 16);
						}
						break;

					/* ステレオ(反転) */
					case 2:
						pOPN[0]->Mix2((int32*)q, samples, 9, 23);
						if (whg_use) {
							pOPN[1]->Mix2((int32*)q, samples, 23, 9);
						}
						if (thg_use) {
							pOPN[2]->Mix2((int32*)q, samples, 16, 16);
						}
						else if (fm7_ver == 1) {
							pOPN[2]->psg.Mix2((int32*)q, samples, 16, 16);
						}
						break;

					/* ステレオ(THG) */
					case 3:
						pOPN[0]->Mix2((int32*)q, samples, 16, 16);
						if (whg_use) {
							pOPN[1]->Mix2((int32*)q, samples, 9, 23);
						}
						if (thg_use) {
							pOPN[2]->Mix2((int32*)q, samples, 23, 9);
						}
						else if (fm7_ver == 1) {
							pOPN[2]->psg.Mix2((int32*)q, samples, 16, 16);
						}
				}
			}
		}

		/* テープ */
                  q = &lpsbuf[uSample * uChannels];
                  TapeSnd((int32 *)q, samples);
		/* ビープ */
                  q = &lpsbuf[uSample * uChannels];
                  BeepSnd((int32 *)q, samples);

#ifdef FDDSND
		/* WAVサウンド */
                  q = &lpsbuf[uSample * uChannels];
                  WaveSnd((int32 *)q, samples);
#endif

	}

	/* 更新 */
	if (bFill) {
          if(sndDstBuf[XM7_SND_MAIN] != NULL) {
            uSample += samples;
            p = (WORD *)lpdsb;
            q = (DWORD *)lpsbuf;
            i = (int)(uSample * uChannels);
            if(uSample > 0) {
              CopySoundBufferGeneric(q, p, i);
              sndDstBuf[XM7_SND_MAIN]->allocated = 1;
              //sndDstBuf[XM7_SND_MAIN]->abuf = (Uint8 *)&lpdsb[dwPlayC];
              sndDstBuf[XM7_SND_MAIN]->abuf = (Uint8 *)lpdsb;
              sndDstBuf[XM7_SND_MAIN]->alen = (Uint32)(uSample * uChannels * 2);
              sndDstBuf[XM7_SND_MAIN]->volume = iTotalVolume;
              //printf("Play: %d %d %d\n", wsize, dwPlayC, SDL_GetTicks());
              Mix_PlayChannel(0,sndDstBuf[XM7_SND_MAIN], 0);
            }
          }

          dwSoundTotal = 0;
          uSample = 0;

	}
	else {
		uSample += samples;
	}
          if(musicSem) SDL_SemPost(musicSem);        
	return retval; /* 生成サンプル数を返す様にした 20100115 */
}
#endif

/*
 *	WAVキャプチャ処理
 */
static void FASTCALL WavCapture(void)
{
	UINT nSize;
	DWORD *p;
	WORD *q;
	int j;

	/* WAVキャプチャ中でなければ、リターン */
	if (hWavCapture < 0) {
		return;
	}
	ASSERT(pWavCapture);

	/* ポインタ、サイズを仮決め(nSizeはWORD変換後のBYTE値) */
	p = lpsbuf;
	nSize = uBufSize / 2;

	/* bWavCaptureがFALSEなら */
	if (!bWavCapture) {
		/* 頭出しチェック */
		while (nSize > 0) {
			if (uChannels == 1) {
				if (*p != 0) {
					break;
				}
				else {
					nSize -= 2;
					p++;
				}
			}
			else {
				if ((p[0] != 0) || (p[1] != 0)) {
					break;
				}
				else {
					nSize -= 4;
					p += 2;
				}
			}
		}
		/* 判定 */
		if (nSize == 0) {
			return;
		}
	}

	/* nWavCaptureを考慮 */
	if ((nWavCapture + nSize) >= 0x8000) {
		/* 32KBいっぱいまでコピー */
		j = (0x8000 - nWavCapture) >> 1;
		q = &pWavCapture[nWavCapture >> 1];
		CopySoundBuffer(p, q, j);
		p += j;

		/* 残りサイズを更新 */
		nSize -= (0x8000 - nWavCapture);

		/* 書き込み */
		file_write(hWavCapture, (BYTE*)pWavCapture, 0x8000);
		dwWavCapture += 0x8000;
		nWavCapture = 0;
	}

	/* 余りをコピー */
	j = nSize >> 1;
	q = &pWavCapture[nWavCapture >> 1];
	CopySoundBuffer(p, q, j);
	nWavCapture += nSize;

	/* 正式な録音状態 */
	bWavCapture = TRUE;
}


    



/*
 *	定期処理 / 1msごとに呼び出される。
 */
void FASTCALL ProcessSnd(BOOL bZero)
{
  
  BOOL bWrite;
  WORD *ptr1;
  DWORD size1;
  DWORD *p;
  DWORD dwOffset;
  Uint32 lTime;
  int i,len;
  int wsize = 0;
  int samples;
  DWORD bufTick;
  int wsamples;
  Mix_Chunk sndChunk, tapeChunk, beepChunk;
     
  //if(dwExecLocal >= dwExecTotal) dwExecLocal = dwExecTotal;
  
  /* 初期化されていなければ、何もしない */
  if ( (!lpdsb) || (!musicSem)) {
    return;
  }
	
  bDataReady = FALSE;
  //printf("Wrote: %d %d %d\n", wsize, dwPlayC, SDL_GetTicks());

        
  dwPlayC ++;
  /* 書き込み位置とバンクから、必要性を判断 */
  bWrite = FALSE;
  if(bNowBank) {
    if(dwPlayC >= (uTick / 2) ){
      bWrite = TRUE;
    }
  } else {
    if(dwPlayC < (uTick /2 ) ) {
      bWrite = TRUE;
    }
  }
  if(dwPlayC >= uTick){
    dwPlayC = 0;
  }
//  dwPlayC ++;
  /* サウンドデータの生成 */
          
          /* 書き込む必要がなければ、リターン */
          if (!bWrite) {
            /* テープ */
            if (tape_motor && bTapeMon) {
              bWrite = TRUE;
            }
            /* BEEP */
            if (beep_flag && speaker_flag) {
              bWrite = TRUE;
            }
            
            /* どちらかがONなら、バッファ充填 */
            if (bWrite) {
              wsamples = AddSnd(FALSE, bZero);
              bDataReady = TRUE;
            }
            return;
          }
          /* ここから演奏開始 */
          
          AddSnd(TRUE, bZero);
          bDataReady = TRUE;
          /* 書き込みバンク(仮想) */
          bNowBank = (!bNowBank);
}

/*
 *	レベル取得
 */
int FASTCALL GetLevelSnd(int ch)
{
	FM::OPN *p;
	int i;
	double s;
	double t;
	int *buf;

	ASSERT((ch >= 0) && (ch < 18));

	/* OPN,WHGの区別 */
	if (ch < 6) {
		p = pOPN[0];
	}
	if ((ch >= 6) && (ch < 12)) {
		p = pOPN[1];
		ch -= 6;

		/* WHGの場合、実際に使われていなければ0 */
		if (!whg_enable || !whg_use) {
			return 0;
		}
	}
	if ((ch >= 12) && (ch < 18)) {
		p = pOPN[2];
		ch -= 12;

		/* THGの場合、実際に使われていなければ0 */
		if ((!thg_enable || !thg_use) && (fm7_ver != 1)) {
			return 0;
		}
	}

	/* 存在チェック */
	if (!p) {
		return 0;
	}

	/* FM,PSGの区別 */
	if (ch < 3) {
		/* FM:512サンプルの2乗和を計算 */
		buf = p->rbuf[ch];

		s = 0;
		for (i=0; i<512; i++) {
			t = (double)*buf++;
			t *= t;
			s += t;
		}
		s /= 512;

		/* ゼロチェック */
		if (s == 0) {
			return 0;
		}

		/* log10を取る */
		s = log10(s);

		/* FM音源補正 */
		s *= 40.0;
	}
	else {
		/* PSG:512サンプルの2乗和を計算 */
		buf = p->psg.rbuf[ch - 3];

		s = 0;
		for (i=0; i<512; i++) {
			t = (double)*buf++;
			t *= t;
			s += t;
		}
		s /= 512;

		/* ゼロチェック */
		if (s == 0) {
			return 0;
		}

		/* log10を取る */
		s = log10(s);

		/* PSG音源補正 */
		s *= 60.0;
	}

	return (int)s;
}

/*
 *	WAVキャプチャ開始
 */
void FASTCALL OpenCaptureSnd(char *fname)
{
	WAVEFORMATEX wfex;
	DWORD dwSize;
	int fileh;

	ASSERT(fname);
	ASSERT(hWavCapture < 0);
	ASSERT(!bWavCapture);

	/* 合成中でなければ、リターン */
	if (!pOPN[2]) {
		return;
	}

	/* バッファが無ければ、リターン */
	if (!pWavCapture) {
		return;
	}

	/* uBufSize / 2が0x8000以下でないとエラー */
	if ((uBufSize / 2) > 0x8000) {
		return;
	}

	/* ファイルオープン(書き込みモード) */
	fileh = file_open(fname, OPEN_W);
	if (fileh < 0) {
		return;
	}

	/* RIFFヘッダ書き込み */
	if (!file_write(fileh, (BYTE*)"RIFFxxxxWAVEfmt ", 16)) {
		file_close(fileh);
		return;
	}

	/* WAVEFORMATEX書き込み */
	dwSize = sizeof(wfex);
	if (!file_write(fileh, (BYTE*)&dwSize, sizeof(dwSize))) {
		file_close(fileh);
		return;
	}
	memset(&wfex, 0, sizeof(wfex));
	wfex.cbSize = sizeof(wfex);
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nChannels = uChannels;
	wfex.nSamplesPerSec = uRate;
	wfex.nBlockAlign = (WORD)(2 * uChannels);
	wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;
	wfex.wBitsPerSample = 16;
	if (!file_write(fileh, (BYTE *)&wfex, sizeof(wfex))) {
		file_close(fileh);
		return;
	}

	/* dataサブヘッダ書き込み */
	if (!file_write(fileh, (BYTE *)"dataxxxx", 8)) {
		file_close(fileh);
		return;
	}

	/* ok */
	nWavCapture = 0;
	dwWavCapture = 0;
	bWavCapture = FALSE;
	hWavCapture = fileh;
}

/*
 *	WAVキャプチャ終了
 */
void FASTCALL CloseCaptureSnd(void)
{
	DWORD dwLength;

	ASSERT(hWavCapture >= 0);

	/* バッファに残った分を書き込み */
	file_write(hWavCapture, (BYTE*)pWavCapture, nWavCapture);
	dwWavCapture += nWavCapture;
	nWavCapture = 0;

	/* ファイルレングスを書き込む */
	file_seek(hWavCapture, 4);
	dwLength = dwWavCapture + sizeof(WAVEFORMATEX) + 20;
	file_write(hWavCapture, (BYTE *)&dwLength, sizeof(dwLength));

	/* data部レングスを書き込む */
	file_seek(hWavCapture, sizeof(WAVEFORMATEX) + 24);
	file_write(hWavCapture, (BYTE *)&dwWavCapture, sizeof(dwWavCapture));

	/* ファイルクローズ */
	file_close(hWavCapture);

	/* ワークエリアクリア */
	hWavCapture = -1;
	bWavCapture = FALSE;
}

/*
 *	OPN出力
 */
extern "C" {
void FASTCALL opn_notify(BYTE reg, BYTE dat)
{
	/* OPNがなければ、何もしない */
	if (!pOPN[0]) {
		return;
	}

	/* プリスケーラを調整 */
	if (opn_scale[OPN_STD] != nScale[0]) {
		nScale[0] = opn_scale[OPN_STD];
		switch (opn_scale[OPN_STD]) {
			case 2:
				pOPN[0]->SetReg(0x2f, 0);
				break;
			case 3:
				pOPN[0]->SetReg(0x2e, 0);
				break;
			case 6:
				pOPN[0]->SetReg(0x2d, 0);
				break;
		}
	}

	/* Ch3動作モードチェック */
	if (reg == 0x27) {
		if (uCh3Mode[0] == dat) {
			return;
		}
		uCh3Mode[0] = dat;
	}

	/* 0xffレジスタはチェック */
	if (reg == 0xff) {
		if ((opn_reg[OPN_STD][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* 出力 */
	pOPN[OPN_STD]->SetReg((uint8)reg, (uint8)dat);

}
}

/*
 *	WHG出力
 */
extern "C" {
void FASTCALL whg_notify(BYTE reg, BYTE dat)
{
	/* WHGがなければ、何もしない */
	if (!pOPN[OPN_WHG]) {
		return;
	}

	/* プリスケーラを調整 */
	if (opn_scale[OPN_WHG] != nScale[1]) {
		nScale[1] = opn_scale[OPN_WHG];
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

	/* Ch3動作モードチェック */
	if (reg == 0x27) {
		if (uCh3Mode[1] == dat) {
			return;
		}
		uCh3Mode[1] = dat;
	}

	/* 0xffレジスタはチェック */
	if (reg == 0xff) {
		if ((opn_reg[OPN_WHG][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* 出力 */
	pOPN[OPN_WHG]->SetReg((uint8)reg, (uint8)dat);

}
}

/*
 *	THG出力
 */
extern "C" {
void FASTCALL thg_notify(BYTE reg, BYTE dat)
{
	/* THGがなければ、何もしない */
	if (!pOPN[2]) {
		return;
	}

	/* プリスケーラを調整 */
	if (opn_scale[OPN_THG] != nScale[2]) {
		nScale[2] = opn_scale[OPN_THG];

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

	/* Ch3動作モードチェック */
	if (reg == 0x27) {
		if (uCh3Mode[2] == dat) {
			return;
		}
		uCh3Mode[2] = dat;
	}

	/* 0xffレジスタはチェック */
	if (reg == 0xff) {
		if ((opn_reg[OPN_THG][0x27] & 0xc0) != 0x80) {
			return;
		}
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* 出力 */
	pOPN[OPN_THG]->SetReg((uint8)reg, (uint8)dat);

}
}
/*
 *	BEEP出力
 */
extern "C" {
void FASTCALL beep_notify(void)
{
	/* 出力状態が変化していなければリターン */
	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);

	/* フラグ保持 */
	if (beep_flag && speaker_flag) {
		bBeepFlag = TRUE;
	}
	else {
		bBeepFlag = FALSE;
	}
}
}

/*
 *	テープ出力
 */
extern "C" {
void FASTCALL tape_notify(BOOL flag)
{
	/* 出力状態が変化したかチェック */
	if (bTapeFlag == flag) {
		return;
	}

	/* サウンド合成 */
	if (bTapeMon) {
	  AddSnd(FALSE, FALSE);
	}

	/* フラグ保持 */
	bTapeFlag = flag;
}
}

/*
 *	WAV出力
 */
#ifdef FDDSND
extern "C" {
void FASTCALL wav_notify(BYTE no)
{
	int i;
	int j;
	DWORD k;

	/* サウンド合成 */
	AddSnd(FALSE, FALSE);
        
      if (no == SOUND_STOP) {
		/* 停止 */
          for (i=0; i< 3; i++) {
                  //WavP[i].bPlay = FALSE;
            bWavPlay[i] = FALSE;
            //Mix_HaltChannel(XM7_SND_WAV_RELAY_ON + i);
            //Mix_Pause(XM7_SND_WAV_RELAY_ON + i);
          }
      }
      else {
        if(no < 3) {
          if(bWavPlay[no]) {
            /* 演奏 */
            if(sndDstBuf[XM7_SND_WAV_RELAY_ON + no]) {
              /* 重くなるのでここでは鳴らさない */
              bWavPlay[no] = TRUE;
            } else {
              /* 演奏止める */
              bWavPlay[no] = FALSE;
            }
          }
        }
      }
}
}
#endif
#endif	/* _XWIN */

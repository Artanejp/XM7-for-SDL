/*
 * api_snd2.cpp
 *
 *  Created on: 2010/12/26
 *      Author: whatisthis
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
static WORD             uChannels;      /* 出力チャンネル数 */
static BOOL             bBeepFlag;      /* BEEP出力 */
static BOOL             bPartMute[3][6];        /* パートミュートフラグ */
static int              nBeepLevel;     /* BEEP音出力レベル */
static int              nCMTLevel;      /* CMT音モニタ出力レベル */
#ifdef FDDSND
static int              nWaveLevel;     /* 各種効果音出力レベル */
#endif
static BOOL             bTapeFlag;      /* 現在のテープ出力状態 */
static BOOL				bWavFlag; /* WAV演奏許可フラグ */


/*
 *
 */
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
 * Sound routine New Make
 */
enum {
	SND_OPN = 0,
	SND_WHG,
	SND_THG,
	SND_PSG,
	SND_BEEP,
	SND_CMT
};
static DWORD oldTime[SND_CMT+1]; // 前回の時間

/*
 * Sound Drivers
 */
static SndDrvBeep *DrvBeep;
static SndDrvWav *DrvWav;
static SndDrvOpn *DrvPSG;
static SndDrvOpn *DrvOPN[3];
static SndDrvCMT *DrvCMT;

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
	int ch;

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
	 * OPNデバイス(標準 / WHG / THG)を作成
	 */
	for(ch = 0; ch <= SND_THG; ch ++) {
		DrvOPN[ch]= new SndDrvOpn ;
		if(DrvOPN[ch]) {
				DrvOPN[ch]->SetOpNo(OPN_STD);
				DrvOPN[ch]->Setup(uTick);
				DrvOPN[ch]->Enable(TRUE);
		}
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
	if(DrvOPN[SND_OPN]) {
		DrvOPN[SND_OPN]->SetReg(opn_reg[OPN_STD]);
	}
	if(DrvOPN[SND_WHG]) {
		DrvOPN[SND_WHG]->SetReg(opn_reg[OPN_WHG]);
	}
	if(DrvOPN[SND_THG]) {
		DrvOPN[SND_THG]->SetReg(opn_reg[OPN_THG]);
	}

	/*
	 * キャプチャ関連
	 */
	if (!pWavCapture) {
		pWavCapture = (WORD *) malloc(sizeof(WORD) * 0x8000);
	}
//	ASSERT(hWavCapture == -1);
//	ASSERT(!bWavCapture);

	/*
	 * SDL用に仕様変更…出来ればInitSnd()でやったら後はOpen/Closeしたくないんだけれどー。
	 */

	bPlayEnable = TRUE;
    DrvWav = new SndDrvWav[3];
    if(!DrvWav) return FALSE;
	for(i = 0; i < WAV_SLOT; i++) {
		   strcpy(prefix, ModuleDir);
	       strcat(prefix, WavName[i]);
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
 *  演奏開始
 */
void PlaySnd()
{
	/*
	 * サンプルカウンタ、サウンド時間をクリア
	 */
	uSample = 0;
	dwSoundTotal = 0;
	uClipCount = 0;
	dwPlayC = 0;
	uProcessCount = 0;
//	iTotalVolume = SDL_MIX_MAXVOLUME - 1;	/* ここで音量設定する必要があるか? */
	/*
	 * 変換できたので読み込む
	 */
}
/*
 *  演奏停止
 */
void StopSnd(void)
{
}



/*
 *  OpnSet
 */
static void AddOpnSnd(int ch, BOOL bFill, BOOL bZero, DWORD time)
{
	DWORD diff;
	int count;
	int samples;

	if(time == uOldOpnTime[ch]) return; // 時間経ってないので終わる
	if(time > uOldTOpnime[ch]) {
		diff = time - uOldOpnTime[ch];
	} else {
		diff = time + 0xffffffff - uOldOpnTime[ch] + 1;
	}

	if(!bFill) { // 途中までレンダリング
		samples = (uRate / 25) * (int)diff;
		if(samples < 40000) {
			return;
		}
		samples = samples / 40000;
	} else { // バッファ最大までレンダリング
		samples = ((nSoundBuffer / 2) * uRate) / 1000 -  nOpnSamples[ch];
	}
	if(bZreo) {
		DrvOpn[ch]->BZero(nOpnSamples[ch], samples, nRenderBank, TRUE);
	} else {
		DrvOpn[ch]->Render(nOpnSamples[ch], samples, nRenderBank, FALSE);
	}
	nOpnSamples[ch] += samples;
	uOldOpnTime[ch] = time;
}

static void AddBeepSnd(BOOL bFill, BOOL bZero, DWORD time)
{
	DWORD diff;
	int count;
	int samples;

	if(time == uOldBeepTime) return; // 時間経ってないので終わる
	if(time > uOldBeepTime) {
		diff = time - uOldBeepTime;
	} else {
		diff = time + 0xffffffff - uOldBeepTime + 1;
	}

	if(!bFill) { // 途中までレンダリング
		samples = (uRate / 25) * (int)diff;
		if(samples < 40000) {
			return;
		}
		samples = samples / 40000;
	} else { // バッファ最大までレンダリング
		samples = ((nSoundBuffer / 2) * uRate) / 1000 -  nBeepSamples;
	}
	if(bZreo) {
		DrvBeep->BZero(nOpnSamples[ch], samples, nRenderBank, TRUE);
	} else {
		DrvBeep->Render(nOpnSamples[ch], samples, nRenderBank, FALSE);
	}
	nBeepSamples += samples;
}

static void opn_notify_internal(int ch, BYTE reg, BYTE dat)
{
	BYTE r;
	if((ch < 0) || (ch > SND_THG)) return;

	/*
	 * OPNがなければ、何もしない
	 */
	if (DrvOPN[ch] == NULL) {
		return;
	}
	/*
	 * プリスケーラを調整
	 */
	if (opn_scale[ch] != nScale[ch]) {
		nScale[ch] = opn_scale[ch];
		switch (opn_scale[ch]) {
		case 2:
			DrvOPN[ch]->SetReg(0x2f, 0);
			break;
		case 3:
            DrvOPN[ch]->SetReg(0x2e, 0);
			break;
		case 6:
            DrvOPN[ch]->SetReg(0x2d, 0);
			break;
		}
	}

	/*
	 * Ch3動作モードチェック
	 */
	if (reg == 0x27) {
		if (DrvOPN[ch]->GetCh3Mode() == dat) {
			return;
		}
		DrvOPN[ch]->SetCh3Mode(dat);
	}

	/*
	 * 0xffレジスタはチェック
	 */
	if (reg == 0xff) {
		/*
		 * スレッド間の逆方向チェックやるか？
		 */
		r = DrvOPN[ch]->GetReg(0x27);
		if ((r & 0xc0) != 0x80) {
			return;
		}
	}

	/*
	 * レジスタ変更直前までサウンド合成
	 */
	 AddOpnSnd(ch, FALSE, FALSE, dwSoundTotal);

	/*
	 * 出力
	 */
    DrvOPN[ch]->SetReg((uint8) reg, (uint8) dat);
}

#ifdef __cplusplus
extern "C" {
#endif

void opn_notify(BYTE reg, BYTE dat)
{
	opn_notify_internal(SND_OPN, reg, dat);
}



void thg_notify(BYTE reg, BYTE dat)
{
	opn_notify_internal(SND_THG, reg, dat);
}

void whg_notify(BYTE reg, BYTE dat)
{
	opn_notify_internal(SND_WHG, reg, dat);
}

/*
 * SFX出力
 */
void wav_notify(BYTE no)
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

void beep_notify(void)
{

	if (!((beep_flag & speaker_flag) ^ bBeepFlag)) {
		return;
	}
	 AddBeepSnd(FALSE, FALSE, dwSoundTotal);

	if (beep_flag && speaker_flag) {
		bBeepFlag = TRUE;
	} else {
		bBeepFlag = FALSE;
	}
	if(DrvBeep)  DrvBeep->Enable(bBeepFlag);
}

/*
 * CMT Notify:
 *
 */
void tape_notify(BOOL flag)
{

	if (bTapeFlag == flag) {
		return;
	}
	DrvCMT->SetState((BOOL)bTapeFlag);
	if(!DrvCMT) return;
	DrvCMT->Enable(bTapeMON);
	AddCmtSnd(FALSE, FALSE, dwSoundTotal);
	bTapeFlag = flag;
}

#ifdef __cplusplus
}
#endif


/*
 * サウンドエミュレーション、本体（と言うかなんというか）
 */
void        ProcessSnd(BOOL bZero)
{

}

/*
 *  レベル取得
 */
int GetLevelSnd(int ch)
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


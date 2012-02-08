/*
 * SndDrvCMT.cpp
 *
 *  Created on: 2010/10/05
 *      Author: whatisthis
 */

#include "SndDrvCMT.h"
#include "api_snd.h"

namespace {
static	BOOL bTapeFlag2;
static	BOOL bTapeFlag;
static	int uTapeDelta;
}
#define VolLimit 32767

SndDrvCMT::SndDrvCMT() {
	// TODO Auto-generated constructor stub
	uTapeDelta = 0;
	bTapeFlag = FALSE;
	bTapeFlag2 = FALSE;
	enable = FALSE;

	uStereo = nStereoOut %4;
	channels = 2;
	ms = nSoundBuffer;
    srate = nSampleRate;
	counter = 0;
	nLevel = 32767;
	RenderSem = SDL_CreateSemaphore(1);
    SDL_SemPost(RenderSem);
}

SndDrvCMT::~SndDrvCMT() {
	// TODO Auto-generated destructor stub
	bTapeFlag2 = FALSE;
	bTapeFlag = FALSE;
	enable = FALSE;
	if(RenderSem != NULL) {
		SDL_SemWait(RenderSem);
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
}


void SndDrvCMT::SetChannels(int c)
{
	channels = c;
}

void SndDrvCMT::SetRate(int rate)
{
	srate = rate;
}


void SndDrvCMT::Enable(BOOL flag)
{
	enable = flag;
}


void SndDrvCMT::SetState(BOOL state)
{
	bTapeFlag = state;
}


void SndDrvCMT::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}

void SndDrvCMT::Setup(int tick)
{
	UINT uChannels;

	uStereo = nStereoOut %4;
	uChannels = 2;
	channels = uChannels;
	ms = tick;

	enable = FALSE;
	counter = 0;
	return;
}

/*
 * Beepのみのダミー関数
 */
void SndDrvCMT::SetFreq(int f)
{

}

void SndDrvCMT::ResetCounter(BOOL flag)
{
    if(flag) {
        counter = 0;
    }
}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::Setup(int tick, int opno)
{
  Setup(tick);
}

/*
 * OPNのみのダミー関数
 */
BYTE SndDrvCMT::GetCh3Mode(int opn)
{
    return 0;
}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::SetCh3Mode(int opn, Uint8 dat)
{

}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::SetRenderVolume(void)
{
	SetRenderVolume(0);
}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::SetRenderVolume(int ch, int level)
{
    SetRenderVolume(level);
}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::SetRenderVolume(int ch, int fm, int psg)
{
	SetRenderVolume(0);
}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::SetLRVolume(void)
{

}
/*
 * OPNのみのダミー関数
 */
int *SndDrvCMT::GetLVolume(int num)
{
    return NULL;
}

/*
 * OPNのみのダミー関数
 */
int *SndDrvCMT::GetRVolume(int num)
{
    return NULL;
}
/*
 * OPNのみのダミー関数
 */
BYTE SndDrvCMT::GetReg(int opn, BYTE)
{
    return 0;
}
/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::SetReg(int opn, BYTE reg, BYTE dat)
{

}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::SetReg(int opn, BYTE *reg)
{

}

/*
 * OPNのみのダミー関数
 */
int SndDrvCMT::Render(Sint32 *pBuf32, Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero)
{
    return Render(pBuf, start, sSamples, clear, bZero);
}

/*
 * OPNのみのダミー関数
 */
int SndDrvCMT::Render32(Sint32 *pBuf32, int start, int sSamples, BOOL clear,BOOL bZero)
{
    // 32bitバッファがないから、そのまま帰る
    return 0;
}

/*
 * OPNのみのダミー関数
 */
void SndDrvCMT::Copy32(Sint32 *src, Sint16 *dst, int ofset, int samples)
{
    return;
}


int SndDrvCMT::Render(Sint16 *pBuf, int start, int uSamples,  BOOL clear, BOOL bZero)
{
	sint32		dat;
	int      i;
	int    tmp;
	int 	sSamples = uSamples;

	int 	ss2;
	int 	level;
	Sint16 *wbuf;


	if(pBuf == NULL) return 0;
	ss2 = sSamples;
	if(RenderSem == NULL) return 0;
	SDL_SemWait(RenderSem);

	wbuf = pBuf;
	wbuf = &wbuf[start * channels];
    level = nLevel;
//    if((start <= 0) && (clear != TRUE)) {
//    	memset(pBuf, 0x00, s * channels * sizeof(Sint16));
//    }
	/*
	 * テープ出力チェック
	 */
	if (!tape_motor || !enable) {
    	memset(wbuf, 0x00, sizeof(Sint16) * ss2 * channels);
    	SDL_SemPost(RenderSem);
		return ss2;
	}
	/*
	 * 波形分割数を求める
	 */
	if ((srate == 48000) || (srate == 96000)) {
		tmp = (srate * 5 * 2) / 48000;
	}

	else {
		tmp = (srate * 4 * 2) / 44100;
	}
	/*
	 * 出力状態が変化した場合、波形補間を開始する
	 */
	if (bTapeFlag != bTapeFlag2) {
		if (uTapeDelta == 0) {
			uTapeDelta = 1;
		} else {
			uTapeDelta = (BYTE) (tmp - uTapeDelta + 1);
		}
	}
	/*
	 * サンプル書き込み
	 */
	if(clear) memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));

	if(bZero) {
		memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
		SDL_SemPost(RenderSem);
	   RenderCounter += ss2;
		return ss2;
	}
	/*
	 * bZero=FALSE : 合成する
	 */
	for (i = 0; i < ss2; i++) {

		if (uTapeDelta > 0) {
			/*
			 * 波形補間あり
			 */
			dat = (level / tmp) * uTapeDelta;
			if (bTapeFlag) {
				dat = dat - level;
			} else {
				dat = level - dat;
			}
			uTapeDelta++;
			if (uTapeDelta > tmp) {
				uTapeDelta = 0;
			}
		} else {
			/*
			 * 波形補間なし
			 */
			if (bTapeFlag) {
				dat = level;
			} else {
				dat = -level;
			}
		}
		/*
		 * BIG ENDIANの場合にこれでいいのか確認
		 */
		/*
		 * リミッタ追加 20101005
		 */
		if(dat>VolLimit) {
			dat = VolLimit;
		} else if(dat < -VolLimit){
			dat = -VolLimit;
		}
		*wbuf++ = (Uint16) dat;	/* 音量小さすぎないか */
		if (channels == 2) {
			*wbuf++ = (Uint16) dat;
		}
	}
	SDL_SemPost(RenderSem);
	/*
	 * 現在のテープ出力状態を保存
	 */
	bTapeFlag2 = bTapeFlag;
	RenderCounter += ss2;
	return ss2;
}

int SndDrvCMT::GetRenderCounter(void)
{
    return RenderCounter;
}

void SndDrvCMT::ResetRenderCounter(void)
{
    RenderCounter = 0;
}

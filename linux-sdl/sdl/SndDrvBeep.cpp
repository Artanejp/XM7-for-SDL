/*
 * SndDrvBeep.cpp
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "xm7.h"
#include "SndDrvBeep.h"
#include "api_snd.h"

SndDrvBeep::SndDrvBeep() {
	// TODO Auto-generated constructor stub
	enable = FALSE;
	counter = 0;
	freq = 1200;

	uStereo = nStereoOut %4;
	channels = 2;
	ms = nSoundBuffer;
    srate = nSampleRate;
	nLevel = 32767;
	RenderSem = SDL_CreateSemaphore(1);
	SDL_SemPost(RenderSem);
}

SndDrvBeep::~SndDrvBeep() {
	// TODO Auto-generated destructor stub
	enable = FALSE;
	if(RenderSem != NULL) {
		SDL_SemWait(RenderSem);
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
	counter = 0;
}



void SndDrvBeep::SetChannels(int c)
{
	channels = c;
}

void SndDrvBeep::SetRate(int rate)
{
	srate = rate;
}


void SndDrvBeep::Enable(BOOL flag)
{
	enable = flag;
}


void SndDrvBeep::SetState(BOOL state)
{

}


void SndDrvBeep::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}

void SndDrvBeep::Setup(int tick)
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
void SndDrvBeep::SetFreq(int f)
{

}


/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::Setup(int tick, int opno)
{
  Setup(tick);
}

/*
 * OPNのみのダミー関数
 */
BYTE SndDrvBeep::GetCh3Mode(int opn)
{
    return 0;
}

/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::SetCh3Mode(int opn, Uint8 dat)
{

}

/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::SetRenderVolume(void)
{
	SetRenderVolume(0);
}

/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::SetRenderVolume(int ch, int level)
{
    SetRenderVolume(level);
}

/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::SetRenderVolume(int ch, int fm, int psg)
{
	SetRenderVolume(0);
}

/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::SetLRVolume(void)
{

}
/*
 * OPNのみのダミー関数
 */
int *SndDrvBeep::GetLVolume(int num)
{
    return NULL;
}

/*
 * OPNのみのダミー関数
 */
int *SndDrvBeep::GetRVolume(int num)
{
    return NULL;
}
/*
 * OPNのみのダミー関数
 */
BYTE SndDrvBeep::GetReg(int opn, BYTE)
{
    return 0;
}
/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::SetReg(int opn, BYTE reg, BYTE dat)
{

}

/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::SetReg(int opn, BYTE *reg)
{

}

/*
 * OPNのみのダミー関数
 */
int SndDrvBeep::Render(Sint32 *pBuf32, Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero)
{
    return Render(pBuf, start, sSamples, clear, bZero);
}

/*
 * OPNのみのダミー関数
 */
int SndDrvBeep::Render32(Sint32 *pBuf32, int start, int sSamples, BOOL clear,BOOL bZero)
{
    // 32bitバッファがないから、そのまま帰る
    return 0;
}

/*
 * OPNのみのダミー関数
 */
void SndDrvBeep::Copy32(Sint32 *src, Sint16 *dst, int ofset, int samples)
{
    return;
}



void SndDrvBeep::ResetCounter(BOOL flag)
{
	if(flag) {
		counter = 0;
	}

}


extern DWORD dwSoundTotal;

int SndDrvBeep::Render(Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero)
{
	int i;
	int ss2;

	Sint16          *wbuf;
	int sf;
	Sint16 level;

	if(pBuf == NULL) return 0;
	channels = 2;
	ss2 = sSamples;
	if(RenderSem == NULL) return 0;

	SDL_SemWait(RenderSem);

	wbuf = pBuf;
	wbuf = &wbuf[start * channels];
	if(counter >= srate) counter = 0;

    level = nLevel >>2;
//    if((start <= 0) && (clear != TRUE)) {
//        	memset(pBuf, 0x00, s * channels * sizeof(Sint16));
//     }
//	printf("Beep Called: @%08d bufsize=%d start=%d rend.size=%d enable=%d bzero=%d clear=%d\n", dwSoundTotal,  s, start, ss2, enable, bZero, clear);

	if(clear)  memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16)); // 全消去
	if(!enable)  {
		memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16)); // Not Enable →サンプル数分の領域をクリア
		SDL_SemPost(RenderSem);
		return ss2;
	}
	if(enable) {
		if(bZero) {
			memset(wbuf, 0, sizeof(Sint16) * ss2 * channels);
			SDL_SemPost(RenderSem);
            RenderCounter += ss2;
			return ss2;
		}

		/*
		 * ここにレンダリング関数ハンドリング
		 */

		/*
		 * サンプル書き込み
		 */
		for (i = 0; i < ss2; i++) {
			/*
			 * 矩形波を作成
			 */
			sf = (int) (counter * freq * 2);
			sf /= (int) srate;
			/*
			 * 偶・奇に応じてサンプル書き込み
			 */
			if (channels == 1) {
				if (sf & 1) {
					wbuf[i] = level;
				}
				else {
					wbuf[i] = -level;
				}
			} else {
				if (sf & 1) {
					wbuf[i * 2] = level;
					wbuf[i * 2 + 1] = level;
				} else {
					wbuf[i * 2] = -level;
					wbuf[i * 2 + 1] = -level;
				}
			}
			/*
			 * カウンタアップ
			 */
			counter+=1;
			if (counter >= srate) {
				counter = 0;
			}
		}
	}
	SDL_SemPost(RenderSem);
	RenderCounter += ss2;
	return ss2;
}

int SndDrvBeep::GetRenderCounter(void)
{
    return RenderCounter;
}

void SndDrvBeep::ResetRenderCounter(void)
{
    RenderCounter = 0;
}

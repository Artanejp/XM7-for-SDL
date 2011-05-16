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


SndDrvBeep::SndDrvBeep() {
	// TODO Auto-generated constructor stub
	enable = FALSE;
	counter = 0;
	freq = 1200;
}

SndDrvBeep::~SndDrvBeep() {
	// TODO Auto-generated destructor stub
	counter = 0;
	enable = FALSE;
}

//Uint8  *SndDrvBeep::Setup(int tick) -> SndDrvTmpl::Setup(int)

void SndDrvBeep::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}

void SndDrvBeep::ResetCounter(BOOL flag)
{
	if(flag) {
		counter = 0;
	}

}

void SndDrvBeep::SetFreq(int f)
{
	freq = f;
}

extern DWORD dwSoundTotal;

int SndDrvBeep::Render(Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero)
{
	int i;
	int s;
	int ss2;

	Sint16          *wbuf;
	int sf;
	Sint16 level;

	s = (ms * srate)/1000;

	if(pBuf == NULL) return 0;
	channels = 2;
#if 0
	if(start > s) return 0; /* 開始点にデータなし */
	if((sSamples + start) >s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
#else
	ss2 = sSamples;
#endif
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
	return ss2;
}


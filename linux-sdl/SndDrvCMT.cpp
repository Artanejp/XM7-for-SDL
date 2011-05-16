/*
 * SndDrvCMT.cpp
 *
 *  Created on: 2010/10/05
 *      Author: whatisthis
 */

#include "SndDrvCMT.h"

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
}

SndDrvCMT::~SndDrvCMT() {
	// TODO Auto-generated destructor stub
	bTapeFlag2 = FALSE;
	bTapeFlag = FALSE;
	enable = FALSE;
}

void SndDrvCMT::SetState(BOOL state)
{
	bTapeFlag = state;
}


void SndDrvCMT::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}

int SndDrvCMT::Render(Sint16 *pBuf, int start, int uSamples,  BOOL clear, BOOL bZero)
{
	int		dat;
	int      i;
	int    tmp;
	int 	sSamples = uSamples;
	int 	s;
	int 	ss;
	int 	ss2;
	int 	level;
	Sint16 *wbuf;


	if(pBuf == NULL) return 0;
	s = (ms * srate) / 1000;
#if 0
	if(sSamples > s) sSamples = s;
	ss = sSamples + start;
	if(ss > s) {
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
    level = nLevel;
    if((start <= 0) && (clear != TRUE)) {
    	memset(pBuf, 0x00, s * channels * sizeof(Sint16));
    }
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
		if (!uTapeDelta) {
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
		return ss2;
	}
	/*
	 * bZero=FALSE : 合成する
	 */
	for (i = 0; i < ss2; i++) {

		if (uTapeDelta) {
			/*
			 * 波形補間あり
			 */
			dat = (0x1000 / tmp) * uTapeDelta;
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
	return ss2;
}

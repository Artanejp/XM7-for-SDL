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


void SndDrvCMT::SetVolume(Uint8 level)
{
	int i;
	volume = level;
	if(volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
	for(i = 0; i<bufSlot; i++) {
		chunk[i].volume = volume;
	}
}


/*
 * BZERO : 指定領域を0x00で埋める
 */
int SndDrvCMT::BZero(int start, int uSamples, int slot, BOOL clear)
{
	int sSamples = uSamples;
	int s ;
	int ss,ss2;
	Sint16          *wbuf;

	if((slot > bufSlot) || (slot < 0)) return 0;
	s = (ms * srate)/1000;
    if(start <= 0) bufSize[slot] = 0;
	if(buf[slot] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;
    if((start <= 0) && (clear != TRUE)) {
    	memset(buf[slot], 0x00, (ms * srate * channels *sizeof(Sint16)) / 1000 - sizeof(Sint16));
    }

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	if(RenderSem == NULL) return 0;
	if(!enable) return 0;
	SDL_SemWait(RenderSem);
	wbuf = (Sint16 *)buf[slot];
	wbuf = &wbuf[start * channels];
	memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	bufSize[slot] += ss2 * channels * sizeof(Sint16);
	SDL_SemPost(RenderSem);
	return ss2;
}


int SndDrvCMT::Render(int start, int uSamples, int slot, BOOL clear)
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


	if(slot > bufSlot) return 0;
	if(buf[slot] == NULL) return -1;
//	if(!enable) return 0;
	s = (ms * srate) / 1000;
	if(sSamples > s) sSamples = s;
	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	wbuf = (Sint16 *)buf[slot];
	wbuf = &wbuf[start * channels];
    level = nLevel;
    if((start <= 0) && (clear != TRUE)) {
    	memset(buf[slot], 0x00, sizeof(buf[slot]));
    }
    if(start <= 0) bufSize[slot] = 0;
	/*
	 * テープ出力チェック
	 */
	if (!tape_motor || !enable) {
    	memset(wbuf, 0x00, sizeof(Sint16) * ss2 * channels);
		return 0;
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
	SDL_SemWait(RenderSem);
	if(clear) memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));

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
//	bufSize[slot] = (start + ss2) * channels * sizeof(Sint16);
	bufSize[slot] += ss2 * channels * sizeof(Sint16);
//	samples = sSamples;
	return 0;
}

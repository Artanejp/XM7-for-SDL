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
}

SndDrvBeep::~SndDrvBeep() {
	// TODO Auto-generated destructor stub
	enable = FALSE;
}

//Uint8  *SndDrvBeep::Setup(int tick) -> SndDrvTmpl::Setup(int)

void SndDrvBeep::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}


void SndDrvBeep::SetVolume(Uint8 level)
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
int SndDrvBeep::BZero(int start, int uSamples, int slot, BOOL clear)
{
	int sSamples = uSamples;
	int s ;
	int ss,ss2;
	Sint16          *wbuf;

	if((slot > bufSlot) || (slot < 0)) return 0;
	s = (ms * srate)/1000;

	if(buf[slot] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;

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
	//	if(!enable) return 0;

	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = (ss2 + start) * channels * sizeof(Sint16);
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = 128; /* 一応最大 */
	SDL_SemPost(RenderSem);
	return ss2;
}
/*
 * レンダリング
 */
int SndDrvBeep::Render(int start, int uSamples, int slot, BOOL clear)
{
	int i;
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	Sint16          *wbuf;
	int sf;
   Sint16 level;


	if((slot > bufSlot)  || (slot <0)) return 0;
//	s = bufSize / (channels * sizeof(Sint16));
	s = (ms * srate)/1000;

	if(buf[slot] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if((uSamples + start) >s) {
		ss2 = s - start;
	} else {
		ss2 = uSamples;
	}
	if(RenderSem == NULL) return 0;
	SDL_SemWait(RenderSem);
	wbuf = (Sint16 *)buf[slot];
	wbuf = &wbuf[start * channels];
        level = nLevel >>2;
	//if(clear)  memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
        memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	if(enable) {

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
			sf = (int) (counter * nBeepFreq * 2);
			sf /= (int) srate;

			/*
			 * 偶・奇に応じてサンプル書き込み
			 */
			if (channels == 1) {
				if (sf & 1) {
					*wbuf++ = level;
				}
				else {
					*wbuf++ = -level;
				}
			}

			else {
				if (sf & 1) {
					*wbuf++ = level;
					*wbuf++ = level;
				}

				else {
					*wbuf++ = -level;
					*wbuf++ = -level;
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
		bufSize = (start + ss2) * channels * sizeof(Sint16);
		chunk[slot].abuf = buf[slot];
		chunk[slot].alen = (ss2 + start) * channels * sizeof(Sint16);
		chunk[slot].allocated = 1; /* アロケートされてる */
		chunk[slot].volume = volume; /* 一応最大 */

	}
	bufSize = (start + ss2) * channels * sizeof(Sint16);
	SDL_SemPost(RenderSem);
	return ss2;
}


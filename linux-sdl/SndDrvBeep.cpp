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


Uint8 *buf;
int bufSize;
int samples;
UINT channels;
UINT srate;
UINT ms;
int nLevel;
Mix_Chunk chunk;
BOOL enable;
SDL_sem *RenderSem;
UINT counter;




SndDrvBeep::SndDrvBeep() {
	// TODO Auto-generated constructor stub
	bufSize = 0;
	buf = NULL;
	ms = 0;
	srate = nSampleRate;
	channels = 1;
	bufSize = 0;
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
	nLevel = 0;
}

SndDrvBeep::~SndDrvBeep() {
	// TODO Auto-generated destructor stub
	DeleteBuffer();
}


Uint8 *SndDrvBeep::NewBuffer(void)
{
	int uStereo,uChannels;

	if(buf != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
    channels = uChannels;

	bufSize = (ms * srate * channels * sizeof(Sint16)) / 1000;
	buf = (Uint8 *)malloc(bufSize);
	if(buf == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf, 0x00, bufSize); /* 初期化 */
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 1; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
	return buf;
}

void SndDrvBeep::DeleteBuffer(void)
{

	if(buf != NULL) free(buf);
	buf = NULL;
	srate = 0;
	ms = 0;
	bufSize = 0;
	channels = 1;
	chunk.abuf = buf;
	chunk.alen = 0;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}


void SndDrvBeep::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}
/*
 * レンダリング
 */
int SndDrvBeep::Render(int start, int uSamples, BOOL clear)
{
	int i;
	int sSamples = uSamples;
	int s = chunk.alen / (sizeof(Sint16) * channels);
	int ss,ss2;
	Sint16          *wbuf = (Sint16 *) buf;
	int sf;


	if(buf == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(!enable) return 0;
	if(sSamples > s) sSamples = s;

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	if(clear)  memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	/*
	 * ここにレンダリング関数ハンドリング
	 */

	/*
	 * サンプル書き込み
	 */
	for (i = 0; i < samples; i++) {

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
				*wbuf++ = nLevel;
			}

			else {
				*wbuf++ = -nLevel;
			}
		}

		else {
			if (sf & 1) {
				*wbuf++ = nLevel;
				*wbuf++ = nLevel;
			}

			else {
				*wbuf++ = -nLevel;
				*wbuf++ = -nLevel;
			}
		}

		/*
		 * カウンタアップ
		 */
		counter++;
		if (counter >= srate) {
			counter = 0;
		}
	}
	return ss2;
}

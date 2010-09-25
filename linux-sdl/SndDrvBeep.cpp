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
int ms;
int channels;
int playCh;
int srate;
int howlong; /* 実際の演奏秒数 */
Mix_Chunk chunk;
BOOL enable;
int counter;



SndDrvBeep::SndDrvBeep() {
	// TODO Auto-generated constructor stub
	bufSize = 0;
	buf = NULL;
	ms = 0;
	srate = nSampleRate;
	channels = 1;
	playCh = -1;
	bufSize = 0;
	howlong = 0;
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}

SndDrvBeep::~SndDrvBeep() {
	// TODO Auto-generated destructor stub
	DeleteBuffer();
}


Uint8 *SndDrvBeep::NewBuffer(void)
{
	if(buf != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	bufSize = (ms * srate * ch * sizeof(int16)) / 1000;
	buf = (Uint8 *)malloc(bufSize);
	if(buf == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf, 0x00, bufSize); /* 初期化 */
	howlong = ms;
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 1; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}

void SndDrvBeep::DeleteBuffer(void)
{
	do{
	} while(Mix_Playing(playCh));

	if(buf != NULL) free(buf);
	buf = NULL;
	playCh = -1;
	srate = 0;
	ms = 0;
	bufSize = 0;
	howlong = 0;
	channels = 1;
	chunk.abuf = buf;
	chunk.alen = 0;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}


/*
 * レンダリング
 */
void SndDrvBeep::Render(int msec, BOOL clear)
{
	int s = msec;
	int i;
	int size;
	int samples;
	int16          *wbuf = (int16 *) buf;


	if(buf == NULL) return;
	if(!enable) return;
	if(s>ms) s = ms;
	do {
		SDL_Delay(1); /* Waitいるか? */
	} while(playCh < -1);
	size = (uRate * s * channels * sizeof(int16)) / 1000;
	samples = size / sizeof(int16);

	if(clear) memset(wbuf, 0x00, size);
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
		sf = (int) (count * nBeepFreq * 2);
		sf /= (int) srate;

		/*
		 * 偶・奇に応じてサンプル書き込み
		 */
		if (uChannels == 1) {
			if (sf & 1) {
				*wbuf++ = nBeepLevel;
			}

			else {
				*wbuf++ = -nBeepLevel;
			}
		}

		else {
			if (sf & 1) {
				*wbuf++ = nBeepLevel;
				*wbuf++ = nBeepLevel;
			}

			else {
				*wbuf++ = -nBeepLevel;
				*wbuf++ = -nBeepLevel;
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

	howlong = s;
}

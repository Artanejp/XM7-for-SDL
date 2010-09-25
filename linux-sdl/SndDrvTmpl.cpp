/*
 * SndDrvTmpl.cpp
 *
 *  Created on: 2010/09/25
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "xm7.h"

#include "SndDrvTmpl.h"


Uint8 *buf;
int bufSize;
int samples;
int channels;
int playCh;
int srate;
int howlong; /* 実際の演奏秒数 */
Mix_Chunk chunk;
BOOL enable;



SndDrvTmpl::SndDrvTmpl() {
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
}

SndDrvTmpl::~SndDrvTmpl() {
	// TODO Auto-generated destructor stub
	DeleteBuffer();
}

Uint8 *SndDrvTmpl::NewBuffer(void)
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
}

void SndDrvTmpl::DeleteBuffer(void)
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

}

Uint8  *SndDrvTmpl::Setup(int ch)
{

	   if((ch == playCh) && (nSampleRate == srate)
			   && (nSoundBuffer == ms)) return buf;
	   if(buf == NULL) {
		   /*
		    * バッファが取られてない == 初期状態
		    */
		   playCh = ch;
		   ms = nSoundBuffer;
		   srate = nSampleRate;
		   buf = NewBuffer();
	   } else {
		   /*
		    * バッファが取られてる == 初期状態ではない
		    */
		   DeleteBuffer(); /* 演奏終了後バッファを潰す */
		   playCh = ch;
		   ms = nSoundBuffer;
		   srate = nSampleRate;
		   buf = NewBuffer();
	   }
	   return buf;
}


void SndDrvTmpl::SetCh(int ch)
{
	playCh = ch;
}



void SndDrvTmpl::SetVolume(int vol)
{
	if(playCh < 0) return;
	Mix_Volume(playCh, vol);
}

void SndDrvTmpl::Enable(BOOL flag)
{
	enable = flag;
}


void SndDrvTmpl::Play(void)
{
	if(playCh < 0) return;
	chunk.abuf = buf;
	chunk.alen = (srate * howlong * channels * sizeof(int16)) / 1000;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	Mix_PlayChannel(playCh, &chunk, 0);
}

/*
 * レンダリング
 */
void SndDrvTmpl::Render(int msec, BOOL clear)
{
	int s = msec;
	int size;
	if(buf == NULL) return;
	if(!enable) return;
	if(s>ms) s = ms;
	do {
		SDL_Delay(1); /* Waitいるか? */
	} while(playCh < -1);
	size = (uRate * s * channels * sizeof(int16)) / 1000;
	if(clear) memset(buf, 0x00, size);
	/*
	 * ここにレンダリング関数ハンドリング
	 */
	/*
	 * ここではヌルレンダラ
	 */

	howlong = s;
}

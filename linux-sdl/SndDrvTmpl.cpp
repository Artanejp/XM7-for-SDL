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
int srate;
int ms;
Mix_Chunk chunk;
BOOL enable;
SDL_sem *RenderSem;



SndDrvTmpl::SndDrvTmpl() {
	// TODO Auto-generated constructor stub
	bufSize = 0;
	buf = NULL;
	srate = nSampleRate;
	ms = 0;
	channels = 1;
	playCh = -1;
	bufSize = 0;
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	RenderSem = NULL;
}

SndDrvTmpl::~SndDrvTmpl() {
	// TODO Auto-generated destructor stub
	DeleteBuffer();
}

Uint8 *SndDrvTmpl::NewBuffer(void)
{
	int uStereo;
	if(buf != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	uStereo = uStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	channels = 2;
    } else {
    	channels = 1;
    }

	bufSize = (ms * srate * channels * sizeof(Int16)) / 1000;
	buf = (Uint8 *)malloc(bufSize);
	if(buf == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf, 0x00, bufSize); /* 初期化 */
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 1; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	if(RenderSem == NULL) {
		RenderSem = SDL_CreateSemaphore(1);
	}
}

void SndDrvTmpl::DeleteBuffer(void)
{
	if(RenderSem != NULL) {
		SDL_SemWait(RenderSem);
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
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

}

Uint8  *SndDrvTmpl::Setup(void *p)
{
	int uStereo,uChannels;

	uStereo = uStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }

	   if((nSampleRate == srate) && (channels == uChannels)
			   && (nSoundBuffer == ms)) return buf;
	   channels = uChannels;
	   if(buf == NULL) {
		   /*
		    * バッファが取られてない == 初期状態
		    */
		   ms = nSoundBuffer;
		   srate = nSampleRate;
		   buf = NewBuffer();
	   } else {
		   /*
		    * バッファが取られてる == 初期状態ではない
		    */
		   DeleteBuffer(); /* 演奏終了後バッファを潰す */
		   ms = nSoundBuffer;
		   srate = nSampleRate;
		   buf = NewBuffer();
	   }
	   return buf;
}

Mix_Chunk *SndDrvTmpl::GetChunk(void)
{
	chunk.abuf = buf;
	chunk.alen = sSample * channels * sizeof(int16);
	chunk.allocated = 1;
	chunk.volume = 128;
	return &chunk;
}


void SndDrvTmpl::Enable(BOOL flag)
{
	enable = flag;
}

/*
 * レンダリング
 */
void SndDrvTmpl::Render(int uSample, BOOL clear)
{
	int sSample = uSample;
	if(buf == NULL) return;
	if(!enable) return;
	if(samples < sSample) sSample = samples;
	if(clear)  memset(buf, 0x00, size);
	SDL_SemWait(RenderSem);
	/*
	 * ここにレンダリング関数ハンドリング
	 */
	/*
	 * ここではヌルレンダラ
	 */
	SDL_SemPost(RenderSem);
	samples = sSample;
}

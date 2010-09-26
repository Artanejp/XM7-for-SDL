/*
 * SndDrvWav.cpp
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_rwops.h>
#include "xm7.h"

#include "SndDrvWav.h"

Uint8 *buf;
int bufSize;
int samples;
int channels;
int srate;
Mix_Chunk chunk;
Mix_Chunk *chunkP;
Uint8 volume;
BOOL enable;
int counter;
SDL_Sem *RenderSem;


SndDrvWav::SndDrvWav() {
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
	volume = 128;
}

SndDrvWav::~SndDrvWav() {
	// TODO Auto-generated destructor stub
	DeleteBuffer();
}

Uint8 *SndDrvWav::NewBuffer(void)
{
	int uStereo;
	if(buf != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	uStereo = uStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	channels = 2;
    } else {
    	channels = 1;
    }

	bufSize = (ms * srate * channels * sizeof(int16)) / 1000;
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

void SndDrvWav::DeleteBuffer(void)
{
	if(RenderSem != NULL) {
		SDL_SemWait(RenderSem);
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
	if(buf != NULL) {
		free(buf);
		buf = NULL;
	}
	if(chunkP != NULL) {
		Mix_FreeChunk(chunkP);
		chunkP = NULL;
	}
	srate = 0;
	ms = 0;
	bufSize = 0;
	channels = 1;
	chunk.abuf = buf;
	chunk.alen = 0;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
}

/*
 * RAM上のWAVデータを演奏可能な形で読み込む
 */
Uint8 *SndDrvWav::Setup(void *p)
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
    if(chunkP == NULL) {
    	/*
    	 * バッファが取られてない == 初期状態
    	 */
    	ms = nSoundBuffer;
    	srate = nSampleRate;
    } else {
    	/*
    	 * バッファが取られてる == 初期状態ではない
    	 */
    	DeleteBuffer();
    	ms = nSoundBuffer;
    	srate = nSampleRate;
    }

	chunkP = Mix_QuickLoadWav((Uint8 *)p);
	if(chunkP == NULL) return NULL;
	ms = chunkP->alen / (srate * channels * sizeof(int16));
	enable = FALSE;
	counter = 0;
	return chunkP->abuf;
}


Mix_Chunk  *SndDrvWav::GetChunk(void)
{
	return &chunk;
}

void SndDrvWav::SetVolume(int vol)
{
	volume = (Uint8) vol;
}

Uint8 SndDrvWav::GetVolume(void)
{
	return volume;
}


/*
 * レンダリング
 */
int SndDrvWav::Render(int start, int uSamples, BOOL clear)
{
	int sSamples = uSamples;
	int s = chunkP->alen / (sizeof(int16) * channels);
	int ss,ss2;
	int16 *p = (int16 *)buf;
	int16 *q = (int16 *)chunkP->abuf;
	int sSample = uSample;


	if(buf == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(!enable) return 0;
	if(sSamples > s) sSamples = s;

	ss = sSample + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSample;
	}
	if(ss2 <= 0) return 0;

	p = &p[start];
	q = &q[start];
	if(clear)  memset(p, 0x00, ss2 * channels * sizeof(int16));
	SDL_SemWait(RenderSem);

	for(i = 0;i < ss2; i++){
		if(channels == 1){
			*p++ = *q++;
		} else if(channels == 2) {
			*p++ = *q;
			*p++ = *q++;
		}
	}
	chunk.abuf = buf;
	chunk.alen = (sSamples + start) * channels * sizeof(uint16);
	chunk.allocated = 1; /* アロケートされてる */
	chunk.volume = volume; /* 一応最大 */
	SDL_SemPost(RenderSem);
	return ss2;
}

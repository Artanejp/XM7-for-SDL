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


SndDrvWav::SndDrvWav() {
	// TODO Auto-generated constructor stub
	int i;

	bufSlot = 1; /* WAVバッファは一つ */
	lastslot = 0;

	ms = 0;
	srate = nSampleRate;
	channels = 1;
	bufSize = 0;
	buf= NULL;
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = MIX_MAX_VOLUME; /* 一応最大 */
	chunkP = NULL;

	enable = FALSE;
	counter = 0;
	nLevel = (int)32767.0;
	RenderSem = SDL_CreateSemaphore(1);
	SDL_SemPost(RenderSem);
}

SndDrvWav::~SndDrvWav() {
	// TODO Auto-generated destructor stub
	DeleteBuffer();
	SDL_DestroySemaphore(RenderSem);
	RenderSem = NULL;
}

Uint8 *SndDrvWav::NewBuffer(void)
{
	NewBuffer(0);
	return buf;
}

Uint8 *SndDrvWav::NewBuffer(int slot)
{
	int uStereo;

	if(buf != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	channels = 2;
    } else {
    	channels = 1;
    }

    bufSize = (chunkP->alen > ((ms * srate * channels) / 1000))?chunkP->alen:(ms * srate * channels) / 1000;
	buf = (Uint8 *)malloc(bufSize * 2);
	if(buf == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf, 0x00, bufSize); /* 初期化 */
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 1; /* アロケートされてる */
	chunk.volume = MIX_MAX_VOLUME; /* 一応最大 */
	if(RenderSem == NULL) {
		RenderSem = SDL_CreateSemaphore(1);
	}
	return buf;
}

void SndDrvWav::DeleteBuffer(void)
{
	DeleteBuffer(0);
}

void SndDrvWav::DeleteBuffer(int slot)
{

	if(RenderSem == NULL) return;
		SDL_SemWait(RenderSem);
		if(buf != NULL) {
			free(buf);
			buf = NULL;
		}
		if(chunkP) Mix_FreeChunk(chunkP);
		chunkP = NULL;
		SDL_SemPost(RenderSem);
}


void SndDrvWav::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
	if(chunkP == NULL) return;
	Render(0,chunkP->alen / (channels * sizeof(Sint16)), 0, TRUE);

}

Uint8 *SndDrvWav::Setup(char *p)
{
	return Setup(p, 0);
}

/*
 * RAM上のWAVデータを演奏可能な形で読み込む
 */
Uint8 *SndDrvWav::Setup(char *p, int wslot)
{
	int uStereo;
	UINT uChannels;

	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
//    if((nSampleRate == srate) && (channels == uChannels)
//    		&& (nSoundBuffer == ms)) return buf[0];
    channels = uChannels;
    if(p == NULL) {
    	if(chunkP) Mix_FreeChunk(chunkP);
    	chunkP = NULL;
  //  	enable = FALSE;
    	return NULL;
    }
	chunkP = Mix_LoadWAV(p);
	if(chunkP == NULL) {
		DeleteBuffer();
		return NULL;
	}

    if(buf == NULL) {
    	/*
    	 * バッファが取られてない == 初期状態
    	 */
    	ms = nSoundBuffer;
    	srate = nSampleRate;
    	NewBuffer();
    } else {
    	/*
    	 * バッファが取られてる == 初期状態ではない
    	 */
    	DeleteBuffer();
    	ms = nSoundBuffer;
    	srate = nSampleRate;
    	NewBuffer();
    }
//	chunkP[wslot] = Mix_QuickLoad_WAV((Uint8 *)p);
	SetRenderVolume(nWaveVolume);
	return buf;
}


Mix_Chunk *SndDrvWav::GetChunk(void)
{
	return GetChunk(0);
}

Mix_Chunk *SndDrvWav::GetChunk(int slot)
{
	return &chunk;
}


void SndDrvWav::Enable(BOOL flag)
{
	enable = flag;
}



/*
 * BZERO : 指定領域を0x00で埋める
 */
int SndDrvWav::BZero(int start, int uSamples, int slot, BOOL clear)
{
#if 1
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	Sint16 *wbuf;


	if(chunkP == NULL) return 0;
	s = chunkP->alen / (channels * sizeof(Sint16));

	if(buf == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;
    wbuf = (Sint16 *) buf;
    wbuf = &wbuf[start];

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	return ss2;
#else
	return uSamples;
#endif
}

/*
 * レンダリング
 */
int SndDrvWav::Render(int start, int uSamples, int slot, BOOL clear)
{
	int i;
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	Sint16 *p;
	Sint16 *q;
	Sint32 tmp;

	if(chunkP == NULL) return 0;
	s = chunkP->alen / (channels * sizeof(Sint16));

	if(buf) {
		if(RenderSem == NULL) return 0;
		if(chunkP == NULL) return 0;
		p = (Sint16 *)chunkP->abuf;
		q = (Sint16 *)buf;
		SDL_SemWait(RenderSem);
		for(i = 0; i< s; i++) {
			tmp = (nLevel * *p++);
			*q++ = (Sint16)(tmp >>14); // 怨霊^h^h音量が小さすぎるので補正 20101001 K.O
//			*q++ = *p++;
		}
		SDL_SemPost(RenderSem);
	}
	chunk.abuf = buf;
	chunk.alen = chunkP->alen;
	chunk.allocated = 1; /* アロケートされてる */
	chunk.volume = MIX_MAX_VOLUME; /* 一応最大 */
	return s;
}

void SndDrvWav::Play(int ch, int vol, int slot)
{
	if(slot >= bufSlot) return;
	if(chunk.abuf == NULL) return;
	if(chunk.alen <= 0) return;
	//if(!enable) return;
	if(RenderSem == NULL) return;
	SDL_SemWait(RenderSem);
	Mix_Volume(ch, vol);
	Mix_PlayChannel(ch, &chunk, 0);
	SDL_SemPost(RenderSem);
}

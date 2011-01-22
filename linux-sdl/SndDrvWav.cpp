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

	chunkP = NULL;
	enable = FALSE;
	bufSlot = 1;
	counter = 0;
	nLevel = (int)32767.0;
	volume = MIX_MAX_VOLUME;
}

SndDrvWav::~SndDrvWav() {
	// TODO Auto-generated destructor stub
}


Uint8 *SndDrvWav::NewBuffer(int slot)
{
	int uStereo;

	if(slot != 0) return NULL; /* slot0以外は使わない */
	if(buf[0] != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	channels = 2;
    } else {
    	channels = 1;
    }
    if(RenderSem == NULL) return NULL;
    SDL_SemWait(RenderSem);
//    bufSize = (chunkP->alen > ((ms * srate * channels) / 1000))?chunkP->alen:(ms * srate * channels) / 1000;
	bufSize[0] = chunkP->alen;
	buf[0] = (Uint8 *)malloc(bufSize[0]);
	if(buf[0] == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf[0], 0x00, bufSize[0]); /* 初期化 */
	chunk[0].abuf = buf[0];
	chunk[0].alen = bufSize[0];
	chunk[0].allocated = 1; /* アロケートされてる */
	chunk[0].volume = MIX_MAX_VOLUME; /* 一応最大 */
	SDL_SemPost(RenderSem);
	return buf[0];
}

void SndDrvWav::DeleteBuffer(int slot)
{

	if(RenderSem == NULL) return;
		SDL_SemWait(RenderSem);
		if(buf[slot] != NULL) {
			free(buf[slot]);
			buf[slot] = NULL;
		}
		if(chunkP) Mix_FreeChunk(chunkP);
		chunkP = NULL;
		SDL_SemPost(RenderSem);
}


void SndDrvWav::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
	if(chunkP == NULL) return;
	Render(0, (ms * srate)/1000 , 0, TRUE);
}

Uint8 *SndDrvWav::Setup(void)
{
	return Setup(NULL);
}

/*
 * RAM上のWAVデータを演奏可能な形で読み込む
 */
Uint8 *SndDrvWav::Setup(char *p)
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
    	return NULL;
    }
	chunkP = Mix_LoadWAV(p);
	if(chunkP == NULL) {
		DeleteBuffer(0);
		return NULL;
	}

    if(buf[0] == NULL) {
    	/*
    	 * バッファが取られてない == 初期状態
    	 */
    	ms = nSoundBuffer;
    	srate = nSampleRate;
    	NewBuffer(0);
    } else {
    	/*
    	 * バッファが取られてる == 初期状態ではない
    	 */
    	DeleteBuffer(0);
    	ms = nSoundBuffer;
    	srate = nSampleRate;
    	NewBuffer(0);
    }
//	chunkP[wslot] = Mix_QuickLoad_WAV((Uint8 *)p);
	SetRenderVolume(nWaveVolume);
	return buf[0];
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

	if(buf[0] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;
    wbuf = (Sint16 *) buf[0];
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
	int s;
	Sint16 *p;
	Sint16 *q;
	Sint32 tmp;
	if(chunkP == NULL) return 0;
	s = chunkP->alen / (channels * sizeof(Sint16));

	if(buf[0]) {
		if(RenderSem == NULL) return 0;
		p = (Sint16 *)chunkP->abuf;
		q = (Sint16 *)buf[0];
		SDL_SemWait(RenderSem);
		for(i = 0; i< s; i++) {
			tmp = (nLevel * *p++);
			*q++ = (Sint16)(tmp >>15); // 怨霊^h^h音量が小さすぎるので補正 20101001 K.O
		}
		SDL_SemPost(RenderSem);
	}
	chunk[0].abuf = buf[0];
	chunk[0].alen = bufSize[0];
	chunk[0].allocated = 1; /* アロケートされてる */
	chunk[0].volume = volume; /* 一応最大 */
	return s;
}


void SndDrvWav::Play(int ch,  int slot)
{
	if(slot >= bufSlot) return;
	if(chunk[slot].abuf == NULL) return;
	if(chunk[slot].alen <= 0) return;
//		if(!enable) return;
	if(RenderSem == NULL) return;
	SDL_SemWait(RenderSem);
	if(chunk[slot].abuf) Mix_PlayChannel(ch, &chunk[slot], 0);
	SDL_SemPost(RenderSem);
}


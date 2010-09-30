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

#include <vector>
#include "SndDrvWav.h"

namespace {
std::vector<Uint8 *> buf;
std::vector<Mix_Chunk>chunk;
std::vector<Mix_Chunk *> chunkP;
}

SndDrvWav::SndDrvWav() {
	// TODO Auto-generated constructor stub
	int i;

	bufSlot = WAV_SLOT;
	buf.reserve(bufSlot);
	chunk.reserve(bufSlot);
	chunkP.reserve(3);
	lastslot = 0;

	ms = 0;
	srate = nSampleRate;
	channels = 1;
	bufSize = 0;
	for(i = 0 ; i< bufSlot; i++) {
		buf[i] = NULL;
		chunk[i].abuf = buf[i];
		chunk[i].alen = bufSize;
		chunk[i].allocated = 0; /* アロケートされてる */
		chunk[i].volume = 128; /* 一応最大 */
	}
	enable = FALSE;
	counter = 0;
	nLevel = (int)32767.0;
	RenderSem = SDL_CreateSemaphore(1);
}

SndDrvWav::~SndDrvWav() {
	// TODO Auto-generated destructor stub
	int i;
	for(i = 0; i < bufSlot; i++) {
		DeleteBuffer(i);
	}
	if(RenderSem) SDL_DestroySemaphore(RenderSem);
}

Uint8 *SndDrvWav::NewBuffer(void)
{
	int i;
	for(i=0; i<bufSlot ; i++) {
		NewBuffer(i);
	}
	return buf[0];
}

Uint8 *SndDrvWav::NewBuffer(int slot)
{
	int uStereo;
	if(buf[slot] != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	channels = 2;
    } else {
    	channels = 1;
    }

	bufSize = (ms * srate * channels * sizeof(Sint16)) / 1000;
	buf[slot] = (Uint8 *)malloc(bufSize);
	if(buf[slot] == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf[slot], 0x00, bufSize); /* 初期化 */
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = bufSize;
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = 128; /* 一応最大 */
	if(RenderSem == NULL) {
		RenderSem = SDL_CreateSemaphore(1);
	}
	return buf[slot];
}

void SndDrvWav::DeleteBuffer(void)
{
	int i;
	for(i = 0; i<bufSlot ; i++) {
		DeleteBuffer(i);
	}
}

void SndDrvWav::DeleteBuffer(int slot)
{

	if(RenderSem == NULL) return;
	if(slot > bufSlot) return;
		SDL_SemWait(RenderSem);
		if(buf[slot] != NULL) {
			free(buf[slot]);
			buf[slot] = NULL;
		}
		SDL_SemPost(RenderSem);
}

void SndDrvWav::SetRenderVolume(int level)
{
	SetRenderVolume(level, 0);
}

void SndDrvWav::SetRenderVolume(int level, int slot)
{
	int i;
	int s;
	Sint32 tmp;
	Sint16 *p;
	Sint16 *q;

	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
	if(chunkP[slot] == NULL) return;
	Render(0,chunkP[slot]->alen / (channels * sizeof(Sint16)), slot, TRUE);

}

Uint8 *SndDrvWav::Setup(void *p)
{
	return Setup(p, 0);
}

/*
 * RAM上のWAVデータを演奏可能な形で読み込む
 */
Uint8 *SndDrvWav::Setup(void *p, int wslot)
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
    if(chunkP[wslot] == NULL) {
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
    if(p == NULL) {
    	chunkP[wslot] = NULL;
  //  	enable = FALSE;
    	counter = 0;
    	return NULL;
    }
//	chunkP[wslot] = Mix_QuickLoad_WAV((Uint8 *)p);
	chunkP[wslot] = Mix_LoadWAV((char *)p);
	if(chunkP[wslot] == NULL) return NULL;
	SetRenderVolume(nWaveVolume, wslot);
	return buf[wslot];
}


Mix_Chunk *SndDrvWav::GetChunk(void)
{
	return GetChunk(0);
}

Mix_Chunk *SndDrvWav::GetChunk(int slot)
{
	if(slot > bufSlot) return NULL;
	return &chunk[slot];
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

	if(slot > bufSlot) return 0;

	if(chunkP[slot] == NULL) return 0;
	s = chunkP[slot]->alen / (channels * sizeof(Sint16));

	if(buf[slot] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;
    wbuf = (Sint16 *) buf[slot];
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

	if(chunkP[slot] == NULL) return 0;
	s = chunkP[slot]->alen / (channels * sizeof(Sint16));

	if(buf[slot]) {
		if(RenderSem == NULL) return 0;
		if(chunkP[slot] == NULL) return 0;
		p = (Sint16 *)chunkP[slot]->abuf;
		q = (Sint16 *)buf[slot];
		SDL_SemWait(RenderSem);
		for(i = 0; i< s; i++) {
			tmp = (nLevel * *p++);
			*q++ = (Sint16)(tmp >>14); // 怨霊^h^h音量が小さすぎるので補正 20101001 K.O
//			*q++ = *p++;
		}
		SDL_SemPost(RenderSem);
	}
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = chunkP[slot]->alen;
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = 128; /* 一応最大 */
	return s;
}


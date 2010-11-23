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


SndDrvTmpl::SndDrvTmpl() {
	// TODO Auto-generated constructor stub
	int i;
	uStereo = nStereoOut %4;
	if ((uStereo > 0) || bForceStereo) {
		channels = 2;
	} else {
		channels = 1;
	}
	ms = nSoundBuffer;
	srate = nSampleRate;
	bufSlot = DEFAULT_SLOT;
	srate = nSampleRate;
	bufSize = (ms * srate * channels *sizeof(Sint16)) / 1000;
	for(i = 0; i<bufSlot; i++) {
		buf[i] = NULL;
		chunk[i].abuf = NULL;
		chunk[i].alen = 0;
		chunk[i].allocated = 0;
		chunk[i].volume = 0;
	}
	enable = FALSE;
	counter = 0;
	nLevel = 32767;
	volume = MIX_MAX_VOLUME;
	RenderSem = SDL_CreateSemaphore(1);
}

SndDrvTmpl::~SndDrvTmpl() {
	// TODO Auto-generated destructor stub
	int i;

	enable = FALSE;
	for(i = 0; i < bufSlot; i++) {
		DeleteBuffer(i);
	}
	if(RenderSem != NULL) {
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
}

// TODO  bufSlot を少なくした時の対策を後で考える


Uint8 *SndDrvTmpl::NewBuffer(void)
{
	int i;
	for(i = 0; i< bufSlot; i++) {
		NewBuffer(i);
	}
	return buf[0];
}

Uint8 *SndDrvTmpl::NewBuffer(int slot)
{

	if(slot > bufSlot) return NULL;
	if(buf[slot] != NULL) {
		return NULL; /* バッファがあるよ？Deleteしましょう */
	}

	buf[slot] = (Uint8 *)malloc(bufSize);
	if(buf[slot] == NULL) return NULL; /* バッファ取得に失敗 */

	memset(buf[slot], 0x00, bufSize); /* 初期化 */
	return buf[slot];
}

void SndDrvTmpl::DeleteBuffer(void)
{
	int i;
	for(i = 0; i <bufSlot ; i++)
	{
		DeleteBuffer(i);
	}
}

void SndDrvTmpl::DeleteBuffer(int slot)
{
	if(slot > bufSlot) return;
	if(buf[slot] != NULL) free(buf[slot]);
	buf[slot] = NULL;

	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = 0;
	chunk[slot].allocated = 0; /* アロケートされてる */
	chunk[slot].volume = volume; /* 一応最大 */

}

void SndDrvTmpl::SetLRVolume(void)
{
}

void SndDrvTmpl::SetVolume(Uint8 level)
{
	volume = level;
	if(volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
}


void SndDrvTmpl::SetRate(int rate)
{
	srate = rate;
}

Uint8  *SndDrvTmpl::Setup(int tick)
{
	UINT uChannels;
	int i;
	uStereo = nStereoOut %4;
	if ((uStereo > 0) || bForceStereo) {
		uChannels = 2;
	} else {
		uChannels = 1;
	}
	//	if((nSampleRate == srate) && (channels == uChannels)
	//			&& (tick == ms)) return buf[0];
	channels = uChannels;
	ms = tick;
	srate = nSampleRate;
	bufSize = (ms * srate * channels *sizeof(Sint16)) / 1000;

	for(i = 0; i < bufSlot; i++) {
		if(buf[i] == NULL) {
			/*
			 * バッファが取られてない == 初期状態
			 */
			buf[i] = NewBuffer(i);
		} else {
			/*
			 * バッファが取られてる == 初期状態ではない
			 */
			DeleteBuffer(i); /* 演奏終了後バッファを潰す */
			buf[i] = NewBuffer(i);
		}
		chunk[i].abuf = buf[i];
		chunk[i].alen = bufSize;
		chunk[i].allocated = 1; /* アロケートされてる */
		chunk[i].volume = 128; /* 一応最大 */
	}
	enable = FALSE;
	counter = 0;
	return buf[0];
}

Mix_Chunk *SndDrvTmpl::GetChunk(void)
{
	return GetChunk(0);
}


Mix_Chunk *SndDrvTmpl::GetChunk(int slot)
{
	if(slot > bufSlot) return NULL;
	return &chunk[slot];
}

int SndDrvTmpl::GetBufSlots(void)
{
	return bufSlot;
}

void SndDrvTmpl::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}


void SndDrvTmpl::Enable(BOOL flag)
{
	enable = flag;
}


/*
 * BZERO : 指定領域を0x00で埋める
 */
int SndDrvTmpl::BZero(int start, int uSamples, int slot, BOOL clear)
{
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	Sint16          *wbuf;

	if(slot > bufSlot) return 0;
	if(buf[slot] == NULL) return 0;
	if(!enable) return 0;
	if(RenderSem == NULL) return 0;
	s = chunk[slot].alen / (sizeof(Sint16) * channels);
	if(start > s) return 0; /* 開始点にデータなし */


	wbuf = (Sint16 *)buf[slot];
	if(sSamples > s) sSamples = s;

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;

	SDL_SemWait(RenderSem);
	memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	SDL_SemPost(RenderSem);

	bufSize = ss2 * channels * sizeof(Sint16);
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = bufSize;
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = volume; /* 一応最大 */
	enable = TRUE;
	counter = 0;
	return ss2;
}

/*
 * レンダリング
 */
int SndDrvTmpl::Render(int start, int uSamples, int slot, BOOL clear)
{
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	Sint16          *wbuf;

	if(slot > bufSlot) return 0;


	if(buf[slot] == NULL) return -1;
	if(!enable) return 0;

	s = chunk[slot].alen / (sizeof(Sint16) * channels);
	wbuf = (Sint16 *)buf[slot];

	if(sSamples > s) sSamples = s;
	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;

	SDL_SemWait(RenderSem);
	//	if(clear)  memset(buf, 0x00, size);

	/*
	 * ここにレンダリング関数ハンドリング
	 */
	/*
	 * ここではヌルレンダラ
	 */
	SDL_SemPost(RenderSem);
	bufSize = ss2 * channels * sizeof(Sint16);
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = bufSize;
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = volume; /* 一応最大 */
	enable = TRUE;
	counter = 0;
	samples = sSamples;
	return 0;
}

void SndDrvTmpl::Play(int ch,  int slot)
{
	if(slot >= bufSlot) return;
	if(chunk[slot].abuf == NULL) return;
	if(chunk[slot].alen <= 0) return;
	//		if(!enable) return;
	if(RenderSem == NULL) return;
	SDL_SemWait(RenderSem);
	if(chunk[slot].abuf) Mix_PlayChannel(ch, &chunk[slot], 0);
	chunk[slot].alen = 0;
	SDL_SemPost(RenderSem);
}

void SndDrvTmpl::Play(int ch,  int slot, int samples)
{
	if(slot >= bufSlot) return;
	if(chunk[slot].abuf == NULL) return;
	chunk[slot].alen = (Uint32)(sizeof(Sint16) * samples * channels);
	//		if(!enable) return;
	if(RenderSem == NULL) return;
	SDL_SemWait(RenderSem);
	if(chunk[slot].abuf) Mix_PlayChannel(ch, &chunk[slot], 0);
	chunk[slot].alen = 0;
	SDL_SemPost(RenderSem);
}

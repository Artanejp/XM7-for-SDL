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

namespace {
std::vector<Uint8 *> buf;
std::vector<Mix_Chunk>chunk;
//int bufSize;
//int samples;
//UINT channels;
//UINT srate;
//UINT ms;
//UINT uStereo;
//int bufSlot;
//int nLevel; /* レンダリングの音量 */
//Uint8 volume; /* 出力する音量 */
//BOOL enable;
//int counter;
//SDL_sem *RenderSem;
}

SndDrvTmpl::SndDrvTmpl() {
	// TODO Auto-generated constructor stub


	int i;

	bufSlot = DEFAULT_SLOT;
	buf.reserve(bufSlot);
	chunk.reserve(bufSlot);

	volume = MIX_MAX_VOLUME;
	bufSize = 0;
	srate = nSampleRate;
	ms = 0;
	channels = 1;
	bufSize = 0;
	enable = FALSE;
	RenderSem = SDL_CreateSemaphore(1);
	nLevel = 0;
	counter = 0;

for(i = 0; i<DEFAULT_SLOT ; i++) {
	buf[i] = NULL;
	chunk[i].abuf = buf[i];
	chunk[i].alen = bufSize;
	chunk[i].allocated = 0; /* アロケートされてる */
	chunk[i].volume = volume; /* 一応最大 */
	}
}

SndDrvTmpl::~SndDrvTmpl() {
	// TODO Auto-generated destructor stub
	if(RenderSem != NULL) {
		SDL_SemWait(RenderSem);
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
	DeleteBuffer();
}

// TODO  bufSlot を少なくした時の対策を後で考える


Uint8 *SndDrvTmpl::NewBuffer(void)
{
	int i;
	Uint8 *p;
	for(i = 0; i <bufSlot ; i++)
	{
		p=NewBuffer(i);
	}
	return buf[0];

}

Uint8 *SndDrvTmpl::NewBuffer(int slot)
{
	int uChannels;

	if(slot > bufSlot) return NULL;

	if(buf.size()<(unsigned int)bufSlot) {
    	buf.resize(bufSlot, NULL);
    } else 	if(buf[slot] != NULL) {
    	return NULL; /* バッファがあるよ？Deleteしましょう */
    }

	if(chunk.size()<(unsigned int)bufSlot) {
		Mix_Chunk cc;
    	chunk.resize(bufSlot, cc);
    }

	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
    channels = uChannels;
	bufSize = (ms * srate * channels * sizeof(Sint16)) / 1000;
	buf[slot] = (Uint8 *)malloc(bufSize);
	if(buf[slot] == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf[slot], 0x00, bufSize); /* 初期化 */
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = bufSize;
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = 128; /* 一応最大 */
	enable = TRUE;
	counter = 0;
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

	srate = 0;
	ms = 0;
	bufSize = 0;
	channels = 1;
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = 0;
	chunk[slot].allocated = 0; /* アロケートされてる */
	chunk[slot].volume = volume; /* 一応最大 */
	enable = TRUE;
	counter = 0;
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
    ms = tick;

   if((nSampleRate == srate) && (channels == uChannels)
			   && (nSoundBuffer == ms)) return buf[0];
   channels = uChannels;
   for(i = 0; i < bufSlot; i++) {
	   if(buf[i] == NULL) {
	   /*
	    * バッファが取られてない == 初期状態
	    */
		   ms = nSoundBuffer;
		   srate = nSampleRate;
		   buf[i] = NewBuffer(i);
	   } else {
		   /*
		    * バッファが取られてる == 初期状態ではない
		    */
		   DeleteBuffer(i); /* 演奏終了後バッファを潰す */
		   ms = nSoundBuffer;
		   srate = nSampleRate;
		   buf[i] = NewBuffer(i);
	   }
   }
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


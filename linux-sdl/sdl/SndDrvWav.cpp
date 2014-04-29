/*
 * SndDrvWav.cpp
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
//#include <SDL/SDL_mixer.h>
#include <SDL/SDL_rwops.h>
#include "xm7.h"

#include "SndDrvWav.h"
#include "api_snd.h"

SndDrvWav::SndDrvWav() {
	// TODO Auto-generated constructor stub

//	chunkP = NULL;
	buf = NULL;
	enable = FALSE;
	bufSlot = 1;
	counter = 0;
	nLevel = (int)32767.0;
	volume = SDL_MIX_MAXVOLUME;
	RenderSem = SDL_CreateSemaphore(1);
	SDL_SemPost(RenderSem);
}

SndDrvWav::~SndDrvWav() {
   DeleteBuffer(0);
	// TODO Auto-generated destructor stub
}


Uint8 *SndDrvWav::NewBuffer(int slot)
{
   int uStereo;
   
   if(slot != 0) return NULL; /* slot0以外は使わない */
   if(buf != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
   uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	channels = 2;
    } else {
    	channels = 1;
    }
    if(RenderSem == NULL) return NULL;
    SDL_SemWait(RenderSem);
//    bufSize = (chunkP->alen > ((ms * srate * channels) / 1000))?chunkP->alen:(ms * srate * channels) / 1000;
//   bufSize = chunkP->alen;
   buf = (Uint8 *)malloc(bufSize);
   if(buf == NULL) return NULL; /* バッファ取得に失敗 */
   memset(buf, 0x00, bufSize); /* 初期化 */
//   chunk.abuf = buf;
//   chunk.alen = bufSize;
//   chunk.allocated = 1; /* アロケートされてる */
//   chunk.volume = SDL_MIX_MAXVOLUME; /* 一応最大 */
   SDL_SemPost(RenderSem);
   return buf;
}

void SndDrvWav::DeleteBuffer(int slot)
{

	if(RenderSem == NULL) return;
		SDL_SemWait(RenderSem);
		if(buf != NULL) {
			free(buf);
			buf = NULL;
		}
//		if(chunkP) Mix_FreeChunk(chunkP);
//		chunkP = NULL;
		SDL_SemPost(RenderSem);
}


void SndDrvWav::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
//	if(chunkP == NULL) return;
//        chunkP->volume = (Uint8)(SDL_MIX_MAXVOLUME * nLevel /  32767.0);
//	Render(0, (ms * srate)/1000 , 0, TRUE);
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
   channels = uChannels;
//   if((nSampleRate == srate) && (channels == uChannels)
//    		&& (nSoundBuffer == ms)) return chunkP->abuf;

   if(p == NULL) {
//      if(chunkP) Mix_FreeChunk(chunkP);
//      chunkP = NULL;
      return NULL;
   }
//   chunkP = Mix_LoadWAV(p);
//   printf("WAV Load %s CHUNK=%08x\n", p, chunkP);
//   if(chunkP == NULL) {
//      DeleteBuffer(0);
//      return NULL;
//   }

//   if(buf == NULL) {
//      /*
//       * バッファが取られてない == 初期状態
//       */
//      ms = nSoundBuffer;
//      srate = nSampleRate;
//      NewBuffer(0);
//    } else {
//    	/*
//    	 * バッファが取られてる == 初期状態ではない
//   	 */
//       DeleteBuffer(0);
//       ms = nSoundBuffer;
//       srate = nSampleRate;
//       NewBuffer(0);
//    }
   ms = nSoundBuffer;
   srate = nSampleRate;

   SetRenderVolume(nWaveVolume);
//   return chunkP->abuf;
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
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	Sint16 *wbuf;


//	if(chunkP == NULL) return 0;
	if(channels <= 0) return 0;
//	s = chunkP->alen / (channels * sizeof(Sint16));

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

//	if(chunkP == NULL) return 0;
	if(channels <= 0) return 0;
//	s = chunkP->alen / (channels * sizeof(Sint16));

	if(buf) {
		if(RenderSem == NULL) return 0;
//		p = (Sint16 *)chunkP->abuf;
		q = (Sint16 *)buf;
		SDL_SemWait(RenderSem);
		for(i = 0; i< s; i++) {
			tmp = (nLevel * *p++);
			*q++ = (Sint16)(tmp >>13); // 怨霊^h^h音量が小さすぎるので補正 20101001 K.O
		}
		SDL_SemPost(RenderSem);
	}
	return s;
}

void SndDrvWav::Play(int ch,  int slot, int samples)
{
	if(slot >= bufSlot) return;
//	if(buf == NULL) return;
//	if(chunkP == NULL) return;
//	chunk.abuf = chunkP->abuf;
//	chunk.alen = chunkP->alen;
//	chunk.allocated = chunkP->allocated; /* アロケートされてる */
//	chunk.volume = volume;
	if(!enable) return;
	if(RenderSem == NULL) return;
	SDL_SemWait(RenderSem);
//	if(chunkP->abuf) Mix_PlayChannel(ch, chunkP, 0);
	SDL_SemPost(RenderSem);
}

void SndDrvWav::Play(int ch,  int slot)
{
//	if(channels <= 0) return;
//	if(chunkP == NULL) return;
//	Play(ch, slot, chunkP->alen / (channels * sizeof(Sint16)));
}

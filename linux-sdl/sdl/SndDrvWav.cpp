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
	ppos = 0;
	plen = 0;
	nLevel = (int)32767.0;
	volume = SDL_MIX_MAXVOLUME;
	RenderSem = SDL_CreateSemaphore(1);
        // Load Wav
	cvt.len = 0;
	cvt.len_cvt = 0;
        cvt.buf = NULL;
        buf = NULL;
        RawBuf = NULL;
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
    SDL_SemPost(RenderSem);
   return (Uint8 *)buf;
}

void SndDrvWav::DeleteBuffer(int slot)
{

   if(RenderSem == NULL) return;
   SDL_SemWait(RenderSem);
   if(RawBuf != NULL) {
      SDL_FreeWAV(RawBuf);
      RawBuf = NULL;
   }
   if(buf != NULL) {
      free(buf);
      buf = NULL;
   }
   SDL_SemPost(RenderSem);
}


void SndDrvWav::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
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
   if(RawBuf != NULL) return RawBuf;
   
   if(SDL_LoadWAV(p, &RawSpec, &RawBuf, &bufSize) == NULL) {
      return NULL;
   }
   
   SetRate(nSampleRate);
   SetRenderVolume(nWaveVolume);
}

void SndDrvWav::SetRate(int rate)
{
   ms = nSoundBuffer;
   if(rate < 0) return;
   srate = rate;

   if(cvt.buf != NULL) {
      free(cvt.buf);
      cvt.buf = NULL;
      cvt.len = 0;
      cvt.len_cvt = 0;
   }
   
   if(SDL_BuildAudioCVT(&cvt, RawSpec.format, RawSpec.channels, RawSpec.freq,
			AUDIO_S16SYS, channels, srate) < 0) {
        cvt.buf = NULL;
        cvt.len = 0;
        return;
   }
   if(RawBuf != NULL) {
      cvt.buf = malloc(bufSize * cvt.len_mult);
      cvt.len = bufSize;
      memcpy(cvt.buf, RawBuf, bufSize);
      SDL_ConvertAudio(&cvt);
      ppos = 0;
      plen = cvt.len_cvt / (sizeof(Sint16) * channels);
   }
   
}

	
   
void SndDrvWav::Enable(BOOL flag)
{
   enable = flag;
   SDL_SemWait(RenderSem);
   ppos = 0;
   SDL_SemPost(RenderSem);
}

BOOL SndDrvWav::IsPlaying(void)
{
   return playing;
}

BOOL SndDrvWav::IsEnabled(void)
{
   return enable;
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
   

   if(channels <= 0) return 0;
   if(buf == NULL) return 0;
   if(start > s) return 0; /* 開始点にデータなし */
   if(sSamples > s) sSamples = s;
   wbuf = buf;
   wbuf = &wbuf[start];

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
   return ss2;
}

/*
 * レンダリング
 */
int SndDrvWav::Render(Sint16 *pBuf, int start, int sSamples, BOOL clear, BOOL bZero)
{
	int i;
	int s;
	Sint16 *p;
	Sint16 *q;
	Sint32 tmp;
        int ss2;
   
        if(sSamples <= 0) return 0;
        ss2 = sSamples;
        s = plen - ppos;
        
        if(s <= 0) {
	   SDL_SemWait(RenderSem);
	   playing = FALSE;
	   memset(pBuf, 0x00, ss2 * channels * sizeof(Sint16));
	   SDL_SemPost(RenderSem);
	   return ss2;
	}
        if(s >= ss2) {
	   s = ss2;
	}
	if(channels <= 0) return 0;
	if(pBuf != NULL) {
		if(RenderSem == NULL) return 0;
		q = (Sint16 *)pBuf;
	        q = &q[start * channels];
	        p = (Sint16 *)cvt.buf;
	        if(p == NULL) return 0;
	        p = &p[ppos * channels];
		SDL_SemWait(RenderSem);
	        if(clear) memset(q, 0x00, ss2 * channels * sizeof(Sint16));
	        if(s > 0) playing = TRUE;
	        if((bZero) || (!enable)) {
		   memset(q, 0x00, ss2 * channels * sizeof(Sint16));
		   playing = FALSE;
		   SDL_SemPost(RenderSem);
		   return ss2;
		}
	   
		for(i = 0; i < (s * channels); i++) {
			tmp = (nLevel * *p++);
			*q++ = (Sint16)(tmp >>13); // 怨霊^h^h音量が小さすぎるので補正 20101001 K.O
		}
		ppos += s;
		if(ss2 > s) {
		  memset(q, 0x00, (ss2 - s) * channels * sizeof(Sint16));
		}
	        if(ppos >= plen) {
		   ppos = 0;
		   playing = FALSE;
		   enable = FALSE;
		}
		SDL_SemPost(RenderSem);
	}
	return ss2;
}

int SndDrvWav::Render(Sint32 *pBuf32, Sint16 *pBuf, int start, int sSamples, BOOL clear, BOOL bZero)
{
   return Render(pBuf, start, sSamples, clear, bZero);
}

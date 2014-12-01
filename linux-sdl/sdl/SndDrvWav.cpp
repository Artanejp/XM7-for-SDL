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
#include "agar_logger.h"

SndDrvWav::SndDrvWav() {
	// TODO Auto-generated constructor stub

//	chunkP = NULL;
	enable = FALSE;
	bufSlot = 1;
	ppos = 0;
	plen = 0;
	nLevel = (int)32767.0;
	volume = SDL_MIX_MAXVOLUME;
	RenderSem = SDL_CreateSemaphore(1);
        // Load Wav
	wavlen = 0;
        wavsrc = NULL;
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
   uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	channels = 2;
    } else {
    	channels = 1;
    }
    if(RenderSem == NULL) return NULL;
   return (Uint8 *)NULL;
}

void SndDrvWav::DeleteBuffer(int slot)
{

   if(RenderSem == NULL) return;
   SDL_DestroySemaphore(RenderSem);
   RenderSem = NULL;
}


void SndDrvWav::SetRenderVolume(int level)
{
   SDL_SemWait(RenderSem);
   nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
   SDL_SemPost(RenderSem);
}


/*
 * RAM上のWAVデータを演奏可能な形で読み込む
 */
void SndDrvWav::Setup(void)
{
   int uStereo;
   UINT uChannels;

   SDL_SemWait(RenderSem);
   uStereo = nStereoOut %4;
   if ((uStereo > 0) || bForceStereo) {
      uChannels = 2;
   } else {
      uChannels = 1;
   }
   channels = uChannels;
   SDL_SemPost(RenderSem);
   SetRenderVolume(nWaveVolume);
}


void SndDrvWav::SetSrc(Uint8 *p, int len)
{
   SDL_SemWait(RenderSem);
   wavsrc = p;
   wavlen = len;
   plen = len / (sizeof(Sint16) * channels);
   ppos = 0;
   SDL_SemPost(RenderSem);
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
int SndDrvWav::BZero(Sint16 *buf, int start, int uSamples, int slot, BOOL clear)
{
   int sSamples = uSamples;
   int s;
   int ss,ss2;
   Sint16 *wbuf;
   

   if(channels <= 0) return 0;
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
	if(channels <= 0) return 0;
        if(sSamples <= 0) return 0;
        if(RenderSem == NULL) return 0;

        ss2 = sSamples;
        s = plen - ppos;
        
        if((s <= 0) && (enable)){
	   SDL_SemWait(RenderSem);
	   playing = FALSE;
	   enable = FALSE;
	   ppos = 0;
	   //if(enable) memset(pBuf, 0x00, ss2 * channels * sizeof(Sint16));
	   SDL_SemPost(RenderSem);
	   return ss2;
	}
        if(s >= ss2) {
	   s = ss2;
	}
	if(pBuf != NULL) {
		q = (Sint16 *)pBuf;
	        q = &q[start * channels];
		SDL_SemWait(RenderSem);
	        if(clear) memset(q, 0x00, ss2 * channels * sizeof(Sint16));
	        p = (Sint16 *)wavsrc;
	        if(p == NULL) {
		   SDL_SemPost(RenderSem);
		   return ss2;
		}
	   
	        p = &p[ppos * channels];
	        if(s > 0) playing = TRUE;
	        if((bZero) && (enable)) {
		   //memset(q, 0x00, ss2 * channels * sizeof(Sint16));
		   ppos += s;
		   playing = FALSE;
		   if(ppos >= plen) {
		      enable = FALSE;
		      ppos = 0;
		   }
		   
		   SDL_SemPost(RenderSem);
		   return ss2;
		}
	        if(!enable) {
		   ppos += s;
		   playing = TRUE;
		   SDL_SemPost(RenderSem);
		   return ss2;
		}
	   
		for(i = 0; i < (s * channels); i++) {
		        tmp = (Sint32)*q;
			tmp += (nLevel * *p++);
			*q++ = (Sint16)(tmp >>13); // 怨霊^h^h音量が小さすぎるので補正 20101001 K.O
		}
		ppos += s;
		if(ss2 > s) {
		  //memset(q, 0x00, (ss2 - s) * channels * sizeof(Sint16));
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

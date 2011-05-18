/*
 * AudioSDL.cpp
 * リアルタイム性を重視したSDLオーディオ関数
 *
 *  Created on: 2011/05/18
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include "AudioSDL.h"


/*
 *  Audio Call back for SDL; for example
 */

void AudioSDL::SampleCallback(void* userdata, Uint8* stream, int len)
{

	int len2 = len / sizeof(Sint16);
	int len3;
	Sint16 *p;


	if(IntBuf == NULL) return;
	if(BufSem == NULL) return;
    if((BufRptr < 0) || (BufRptr >= BufSize)) return;
    if(BufLeft <= 0) {
    	BufLeft = 0;
    }
    if(BufLeft > BufSize){
    	// Buffer Under Flow
    	// Nullサウンド出すべき？
    	return;
    }

	SDL_SemWait(BufSem);
	if(len2 > BufLeft) len2 = BufLeft;
	p = &(IntBuf[BufRptr]);
	len3 = (BufSize - BufRptr);
	if(len3 > len2) len3 = len2;
	SDL_MixAudio(stream,(Uint8 *)p, len3 * sizeof(Sint16), volume);
	len2 -= len3;
	BufLeft +=  len3;
	BufRptr += len3;
	if(BufRptr >= BufSize) {
		BufRptr = 0;
	}

	if((BufLeft <= BufSize) && (len2 > 0)) {
		p = &(IntBuf[BufRptr]);
		SDL_MixAudio(stream, (Uint8 *)p, len2 * sizeof(Sint16), volume);
		BufRptr += len2;
		BufLeft += len2;
	}
	SDL_SemPost(BufSem);
}

AudioSDL::AudioSDL() {
	// TODO Auto-generated constructor stub
	SampleRate = 0;
	Channels = 0;
	Samples = 0;
	callbackfunc = NULL;

	RealAudioSpec = NULL;
	volume = 0;

	Sounds16 = NULL;
	Members16 = 0;
	Sounds32 = NULL;
	Members32 = 0;

	BufSize = 0;
	BufLeft = 0;
	BufRptr = 0;
	BufWptr = 0;
	IntBuf = NULL;
	BufSem = SDL_CreateSemaphore(1);
	if(BufSem == NULL) {
		return;
	}
//	RegCallback((void (*)(void *, Uint8 *, int))&AudioSDL::SampleCallback);


}

AudioSDL::~AudioSDL() {
	if(IntBuf) {
		free(IntBuf);
	}
	if(BufSem){
		SDL_DestroySemaphore(BufSem);
	}

	SDL_CloseAudio();
	if(RealAudioSpec){
		free(RealAudioSpec);
		RealAudioSpec = NULL;
	}
}

void AudioSDL::SetVolume(WORD vol)
{
	volume = vol;
}

WORD AudioSDL::GetVolume(void)
{
	return volume ;
}


void AudioSDL::RegCallback(void (* RealCallBack)(void *, Uint8 *, int))
{
	callbackfunc = RealCallBack;
}


void AudioSDL::PutSound(Sint16 *src, int len)
{
   int len2;
   int len3 = len;
   int ofset = 0;
   Sint16 *p;
   Sint16 *q;

   if(len <= 0) return;
   if(src == NULL) return;
   if(BufSem == NULL) return;
   if(IntBuf == NULL) return;
   if((BufWptr < 0) || (BufWptr >= BufSize)) return;
   if(BufLeft <= 0) return;

   SDL_SemWait(BufSem);
   len2 = BufSize - BufRptr;
   if(len2 < len3) {
	   p = &(IntBuf[BufWptr]);
	   memcpy((void *)p, (void *)src, len2 * sizeof(Sint16));
	   len3 -= len2;
	   ofset += len2;
	   BufWptr = 0;
	   BufLeft -= len2;
	   if(BufLeft <= 0) {
		   SDL_SemPost(BufSem);
		   return;
	   }
   }
   p = &(IntBuf[BufWptr]);
   q = &(src[ofset]);
   memcpy((void *)p, (void *)q, len3 * sizeof(Sint16));
   BufWptr += len3;
   BufLeft -= len3;
   SDL_SemPost(BufSem);
   return;
}

void AudioSDL::RegSound16(Sint16 **p, int members)
{
	Sounds16 = p;
	Members16 = members;
}

void AudioSDL::RegSound32(Sint32 **p, int members)
{
	Sounds32 = p;
	Members32 = members;
}

Sint16 *AudioSDL::MixSounds(Sint16 *dst, int len, BOOL clear)
{
	int m16;
	int m32;

	if(dst == NULL) return NULL;
	if((Sounds16 == NULL) && (Sounds32 == NULL)) return NULL;
	if(clear) {
		memset(dst, 0x00, len * sizeof(Sint16));
	}
	if(Members16 >= 1) {
		memcpy(dst, Sounds16[0], len * sizeof(Sint16));
		m16 = Members16 - 1;
		if(m16 > 0) {
			int i,j;
			Sint16 tmp;
			Sint32 tmp32;
			Sint16 *p;

			for(i = 0; i < len; i++) {
				tmp32 = 0;
				for(j = 0; j < m16 ; j++) { // キャッシュIN/OUTを勘案するとこちらが有利？
					p = Sounds16[j];
					tmp = p[i];
					tmp32 += (Sint32)tmp;
				}
				if(tmp32 > 32767) {
					tmp = 32767;
				} else 	if(tmp32 < -32767) {
					tmp = -32767;
				} else {
					tmp = (Sint16) tmp;
				}
				dst[i] = tmp;
			}
		}
	}

	m32 = Members32;
	if((Members16 <= 0) &&(Members32 > 0)) {
		memset(dst, 0x00, len * sizeof(Sint16));
	}
	if(Members32 > 0) {
		int i,j;
		Sint16 tmp;
		Sint32 tmp32;
		Sint32 *p;
		for(i = 0; i < len ; i++){
			tmp = dst[i];
			tmp32 = (Sint32)tmp;  // 符号付き拡張
			for(j = 0; j < m32 ; j++) {
				p = Sounds32[j];
				tmp32 += p[i];
			}
			if(tmp32 > 32767) {
				tmp = 32767;
			} else 	if(tmp32 < -32767) {
				tmp = -32767;
			} else {
				tmp = (Sint16) tmp;
			}
			dst[i] = tmp;
		}
	}
	return dst;
}

void AudioSDL::Lock()
{
	SDL_LockAudio();
}

void AudioSDL::Unlock()
{
	SDL_UnlockAudio();
}

void AudioSDL::Kick(BOOL sw)
{
	if(sw) {
		SDL_PauseAudio(0);
	} else {
		SDL_PauseAudio(1);
	}
}

int AudioSDL::Open(int rate, int ch, int spl)
{
	int chunksize;
	SDL_AudioSpec ReqAudioSpec;
	int r;

	if(BufSem == NULL) return -1;
	SDL_SemWait(BufSem);
	if(IntBuf) {
		free(IntBuf);
	}

	Channels = ch;
	Samples = spl;
	SampleRate = rate;

	RealAudioSpec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
	if(RealAudioSpec == NULL) {
		SDL_SemPost(BufSem);
		return -1;
	}
	chunksize = Samples * Channels * sizeof(Sint16);
	IntBuf = (Sint16 *)malloc(chunksize);
	if(IntBuf == NULL) {
		free(RealAudioSpec);
		RealAudioSpec = NULL;
		SDL_SemPost(BufSem);
		return -1;
	}
        
	ReqAudioSpec.freq = SampleRate;
	ReqAudioSpec.format = AUDIO_S16SYS;
	ReqAudioSpec.channels = Channels;
	ReqAudioSpec.samples = Samples;
	ReqAudioSpec.callback = callbackfunc;
   	ReqAudioSpec.userdata = NULL;
	r = SDL_OpenAudio(&ReqAudioSpec, RealAudioSpec);
	if ( r < 0) {
		free(RealAudioSpec);
		RealAudioSpec = NULL;
		free(IntBuf);
		IntBuf = NULL;
		SDL_SemPost(BufSem);
		return -1;
	}
	BufSize = Samples * Channels;
	BufRptr = 0;
	BufWptr = 0;
	BufLeft = BufSize;

	SampleRate = RealAudioSpec->freq;
	Channels = RealAudioSpec->channels;
	Samples = RealAudioSpec->samples;
	Format = RealAudioSpec->format;
	callbackfunc = RealAudioSpec->callback;
	SDL_SemPost(BufSem);
	return chunksize;
}


void AudioSDL::Close()
{
	SDL_CloseAudio();
	if(RealAudioSpec){
		free(RealAudioSpec);
		RealAudioSpec = NULL;
	}

	if(IntBuf) {
		// Destroy Ring BUffer
		free(IntBuf);
		IntBuf = NULL;
	}
	BufRptr = 0;
	BufWptr = 0;
	BufLeft = 0;
	BufSize = 0;
}


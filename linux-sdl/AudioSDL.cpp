/*
 * AudioSDL.cpp
 * リアルタイム性を重視したSDLオーディオ関数
 *
 *  Created on: 2011/05/18
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include "util_ringbuffer.h"
#include "AudioSDL.h"

AudioSDL::AudioSDL() {
	// TODO Auto-generated constructor stub
	SampleRate = 0;
	Channels = 0;
	Samples = 0;

	ring = NULL;
	ReqAudioSpec = NULL;
	RealAudioSpec = NULL;

	Sounds16 = NULL;
	Members16 = 0;
	Sounds32 = NULL;
	Members32 = 0;

	callbackfunc = SampleCallback;

}

AudioSDL::~AudioSDL() {
	if(ring) {
		DeleteRingBuffer(ring);
	}

	SDL_CloseAudio();
	if(RealAudioSpec){
		free(RealAudioSpec);
		RealAudioSpec = NULL;
	}
}

/*
 *  Audio Call back for SDL; for example
 */
void AudioSDL::SampleCallback(void* userdata, Uint8* stream, int len)
{
	struct RingBufferDesc *queue = ring;
	int result = 0;
	int len2 = len;
	Uint8 *p = stream;

	if(queue == NULL) return;
	if(p == NULL) return;

	while(queue->chunkSize < len2) {
		result = ReadRingBuffer(queue, (void *)p);
		if(result <= 0) return; // Queue Empty
		p += result;
		len2 -= result;
	}
	if(len2 > 0){
		ReadRingBufferLimited(queue, (void *)p, len2);
	}
}

void AudioSDL::RegCallback(void (* RealCallBack)(void *, Uint8 *, int))
{
	callbackfunc = RealCallBack;
}

void AudioSDL::PutSound(Sint16 *src, int len)
{
   int len2;
   int result;
   Uint8 *p;
   struct RingBufferDesc *dst = ring;

   if(len <= 0) return;
   if(src == NULL) return;
   if(dst == NULL) return;

   len2 = len * sizeof(Sint16);
   p = (Uint8 *)src;

   while(len2 > dst->chunkSize) {
	   result = WriteRingBuffer(dst, (void *)p);
	   if(result <= 0) return;
	   p += result;
	   len2 -= result;
   }
   if(len2 > 0) {
	   result = WriteRingBufferLimited(dst, (void *)p, len2);
	   if(result > 0){
		   p += result;
		   len2 -= result;
	   }
   }
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
		memcpy(dst, Sound16[0], len * sizeof(Sint16));
		m16 = Members16 - 1;
		if(m16 > 0) {
			int i,j;
			Sint16 tmp;
			Sint32 tmp32;
			Sint16 *p;

			for(i = 0; i < len; i++) {
				tmp32 = 0;
				for(j = 0; j < m16 ; j++) { // キャッシュIN/OUTを勘案するとこちらが有利？
					p = Sound16[j];
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
		Sint16 *p;
		for(i = 0; i < len ; i++){
			tmp = dst[i];
			tmp32 = (Sint32)tmp;  // 符号付き拡張
			for(j = 0; j < m32 ; j++) {
				p = Sound32[j];
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

int AudioSDL::Open(int rate, int ch, int spl, int chunks)
{
	int chunksize;
	SDL_AudioSpec *ReqAudioSpec;
	int r;

	if(ring) {
		// Destroy Ring BUffer
		DeleteRingBuffer(ring);
	}


	Channels = ch;
	Samples = spl;
	SampleRate = rate;

	ReqAudioSpec = malloc(sizeof(SDL_AudioSpec));
	if(ReqAudioSpec == NULL) return -1;

	RealAudioSpec = malloc(sizeof(SDL_AudioSpec));
	if(RealAudioSpec == NULL) {
		free(ReqAudioSpec);
		return -1;
	}

	ReqAudioSpec->freq = SampleRate;
	ReqAudioSpec->format = AUDIO_S16SYS;
	ReqAudioSpec->channels = Channels;
	ReqAudioSpec->samples = Samples;
	ReqAudioSpec->callback = callbackfunc;
	r = SDL_OpenAudio(ReqAudioSpec, RealAudiospec);
	if ( r < 0) {
		free(ReqAudioSpec);
		free(RealAudioSpec);
		RealAudioSpec = NULL;
		return -1;
	}
	free(ReqAudioSpec);

	SampleRate = RealAudioSpec->freq;
	Channels = RealAudioSpec->channels;
	Samples = RealAudioSpec->samples;
	Format = RealAudioSpec->format;
	callbackfunc = RealAudioSpec->callback;
	Chunks = chunks;

	chunksize = Samples * Channels * sizeof(Sint16);
	ring = CreateRingBuffer(chunksize, chunks);
	if(ring == NULL) {
		Close();
		return 0;
	}

	return chunksize;
}


void AudioSDL::Close()
{
	SDL_CloseAudio();
	if(RealAudioSpec){
		free(RealAudioSpec);
		RealAudioSpec = NULL;
	}

	if(ring) {
		// Destroy Ring BUffer
		DeleteRingBuffer(ring);
		ring = NULL;
	}
}


/*
 * AudioSDL.h
 *
 *  Created on: 2011/05/18
 *      Author: whatisthis
 */

#ifndef AUDIOSDL_H_
#define AUDIOSDL_H_

#include "util_ringbuffer.h"
#include <SDL/SDL.h>

class AudioSDL {
public:
	AudioSDL();
	virtual ~AudioSDL();
	void SampleCallback(void* userdata, Uint8* stream, int len);
	void RegCallback(void (* RealCallBack)(void *, Uint8 *, int));

	int Open(int rate, int ch, int spl, int chunks);
	void Close(void);
	void Unlock();
	void Lock(void)
	void Kick(BOOL sw);
	void PutSound(Sint16 *src, int len);

	Sint16 *MixSounds(Sint16 *dst, int len, BOOL clear);

protected:
	void (*callbackfunc)(void *, Uint8 *, int);
	Sint16 **Sounds16;
	int Members16;
	Sint32 **Sounds32;
	int Members32;
	struct RingBufferDesc *ring;
	SDL_AudioSpec *RealAudioSpec;

	int SampleRate;
	int Channels;
	int Samples;
	int Format;
	int Chunks;
};

#endif /* AUDIOSDL_H_ */

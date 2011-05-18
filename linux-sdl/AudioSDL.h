/*
 * AudioSDL.h
 *
 *  Created on: 2011/05/18
 *      Author: whatisthis
 */

#ifndef AUDIOSDL_H_
#define AUDIOSDL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <SDL/SDL.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int     BOOL;

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef TRUE
#define TRUE (!FALSE)
#endif /* TRUE */


class AudioSDL {
public:
	AudioSDL();
	virtual ~AudioSDL();
	void RegCallback(void (* RealCallBack)(void *, Uint8 *, int));

	int Open(int rate, int ch, int spl);
	void Close(void);
	void Unlock();
	void Lock(void);
	void Kick(BOOL sw);
	void PutSound(Sint16 *src, int len);

	Sint16 *MixSounds(Sint16 *dst, int len, BOOL clear);
	void SampleCallback(void* userdata, Uint8* stream, int len);
	void RegSound16(Sint16 **p, int members);
	void RegSound32(Sint32 **p, int members);

	void SetVolume(WORD vol);
	WORD GetVolume(void);


	Sint16 *IntBuf;
	int BufSize;
	int BufRptr;
	int BufWptr;
	int BufLeft;
	SDL_semaphore *BufSem;

protected:
	void (* callbackfunc)(void *, Uint8 *, int);
	Uint8 volume;

	Sint16 **Sounds16;
	int Members16;
	Sint32 **Sounds32;
	int Members32;
	SDL_AudioSpec *RealAudioSpec;

	int SampleRate;
	int Channels;
	int Samples;
	int Format;
};

#endif /* AUDIOSDL_H_ */

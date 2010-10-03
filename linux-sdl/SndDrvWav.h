/*
 * SndDrvWav.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVWAV_H_
#define SNDDRVWAV_H_
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "sdl.h"
#include "sdl_snd.h"

#include <vector>
#include "SndDrvTmpl.h"

#define WAV_SLOT 3

class SndDrvWav: public SndDrvTmpl {
public:
	SndDrvWav();
	virtual ~SndDrvWav();
	Uint8 *NewBuffer(void);
	Uint8 *NewBuffer(int slot);
	void DeleteBuffer(void);
	void DeleteBuffer(int slot);

	void SetRenderVolume(int level);

	Uint8 *Setup(char *p);
	Uint8 *Setup(char *p, int wslot);
	Mix_Chunk  *GetChunk(void);
	Mix_Chunk  *GetChunk(int slot);
	int Render(int start, int uSamples, int slot, BOOL clear);
	int BZero(int start, int uSamples, int slot, BOOL clear);
	void Enable(BOOL flag);
	void Play(int ch, int vol, int slot);
	void Play(int ch, int vol, Mix_Chunk *c);

private:
	void SetRenderVolume(int level, int slot);
	Uint8 *buf;
	Uint8 *sbuf;
	Mix_Chunk chunk;
	Mix_Chunk *chunkP;
	int bufSize;
	int bufSlot;
	int samples;
	int lastslot;
	UINT channels;
	UINT srate;
	UINT ms;
	UINT counter;
	int nLevel;
	BOOL enable;
	SDL_sem *RenderSem;
	Uint8 volume;
};

#endif /* SNDDRVWAV_H_ */

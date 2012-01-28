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
//#include "sdl_snd.h"

#include "SndDrvIF.h"

#define WAV_SLOT 3

class SndDrvWav {
public:
	SndDrvWav();
	virtual ~SndDrvWav();
	Uint8 *NewBuffer(int slot);
	void DeleteBuffer(int slot);
	void Enable(BOOL flag);
	void SetRenderVolume(int level);
	Uint8 *Setup(void);
	Uint8 *Setup(char *p);
	int Render(int start, int uSamples, int slot, BOOL clear);
	int BZero(int start, int uSamples, int slot, BOOL clear);
	void Play(int ch,  int slot, int samples);
	void Play(int ch, int slot);
private:
	void SetRenderVolume(int level, int slot);
	Mix_Chunk *chunkP;
	Mix_Chunk chunk;
	int bufSize;
	Uint8 *buf;
	int samples;
	UINT channels;
	UINT srate;
	UINT ms;
	UINT uStereo;
	int bufSlot;
	int nLevel; /* レンダリングの音量 */
	Uint8 volume; /* 出力する音量 */
	BOOL enable;
	UINT counter;
	SDL_sem *RenderSem;
};

#endif /* SNDDRVWAV_H_ */

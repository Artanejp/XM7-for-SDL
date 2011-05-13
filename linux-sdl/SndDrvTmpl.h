/*
 * SndDrvTmpl.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVTMPL_H_
#define SNDDRVTMPL_H_

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "sdl.h"
#include "sdl_snd.h"
#include <vector>

#define DEFAULT_SLOT 8


class SndDrvTmpl {
public:
	SndDrvTmpl();
	virtual ~SndDrvTmpl();

	Uint8 *NewBuffer(int slot);
	Uint8 *NewBuffer(void);
	void DeleteBuffer(void);
	void DeleteBuffer(int slot);

	Uint8  *Setup(int tick);
	Mix_Chunk *GetChunk(void);
	Mix_Chunk *GetChunk(int slot);
	int GetBufSlots(void);
	void Enable(BOOL flag);
	void SetRate(int rate);
	void SetVolume(Uint8 level);
	void SetLRVolume(void);
	int Render(int start, int uSamples, int slot, BOOL clear);
	int BZero(int start, int uSamples, int slot, BOOL clear);
	void SetRenderVolume(int level);
	void Play(int ch,  int slot);
	void Play(int ch,  int slot, int samples);
        Uint8 *GetBuf(int slot);
        int GetBufSize(int slot);
protected:
	Uint8 *buf[DEFAULT_SLOT];
	Mix_Chunk chunk[DEFAULT_SLOT];
	int bufSize[DEFAULT_SLOT];
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

#endif /* SNDDRVTMPL_H_ */

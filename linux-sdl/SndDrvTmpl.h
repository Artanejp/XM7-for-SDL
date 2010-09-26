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

class SndDrvTmpl {
public:
	SndDrvTmpl();
	virtual ~SndDrvTmpl();
	Uint8 *NewBuffer(void);
	void DeleteBuffer(void);
	Uint8  *Setup(void *p);
	int Render(int start, int uSamples, BOOL clear);
	void Enable(BOOL flag);
	void SetRenderVolume(int level);
	Mix_Chunk *GetChunk(void);
private:
	Uint8 *buf;
	int bufSize;
	int samples;
	UINT channels;
	UINT srate;
	UINT counter;
	UINT ms;
	int nLevel;
	Mix_Chunk chunk;
	BOOL enable;
	SDL_sem *RenderSem;

};

#endif /* SNDDRVTMPL_H_ */

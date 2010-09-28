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

#include "SndDrvTmpl.h"

class SndDrvWav: public SndDrvTmpl {
public:
	SndDrvWav();
	virtual ~SndDrvWav();
	Uint8 *NewBuffer(void);
	void DeleteBuffer(void);
	void SetRenderVolume(int level);
	Uint8 *Setup(void *p);
	Mix_Chunk  *GetChunk(void);
	int Render(int start, int uSamples, BOOL clear);
	int BZero(int start, int uSamples, BOOL clear);
	void Enable(BOOL flag);
private:
	Uint8 *buf;
	int bufSize;
	int samples;
	UINT channels;
	UINT srate;
	UINT ms;
	UINT counter;
	int nLevel;
	Mix_Chunk chunk;
	Mix_Chunk *chunkP;
	BOOL enable;
	SDL_sem *RenderSem;

};

#endif /* SNDDRVWAV_H_ */

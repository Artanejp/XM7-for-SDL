/*
 * SndDrvBeep.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVBEEP_H_
#define SNDDRVBEEP_H_

#include "SndDrvTmpl.h"

class SndDrvBeep: public SndDrvTmpl {
public:
	SndDrvBeep();
	virtual ~SndDrvBeep();
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
	UINT ms;
	UINT counter;
	int nLevel;
	Mix_Chunk chunk;
	BOOL enable;
	SDL_sem *RenderSem;

};

#endif /* SNDDRVBEEP_H_ */

/*
 * SndDrvBeep.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVBEEP_H_
#define SNDDRVBEEP_H_

#include "SndDrvTmpl.h"

#define BEEP_SLOT 2


class SndDrvBeep: public SndDrvTmpl {
public:
	SndDrvBeep();
	virtual ~SndDrvBeep();
	Uint8 *NewBuffer(int slot);
	void DeleteBuffer(void);
	void DeleteBuffer(int slot);
	Uint8  *Setup(void *p);
	int BZero(int start, int uSamples, int slot, BOOL clear);
	int Render(int start, int uSamples, int slot, BOOL clear);
	void Enable(BOOL flag);
	void SetRenderVolume(int level);
	Mix_Chunk *GetChunk(void);
	Mix_Chunk *GetChunk(int slot);
private:
	Uint8 *buf[BEEP_SLOT];
	int bufSize;
	int samples;
	UINT channels;
	UINT srate;
	UINT ms;
	UINT counter;
	int nLevel;
	Mix_Chunk chunk[BEEP_SLOT];
	BOOL enable;
	SDL_sem *RenderSem;

};

#endif /* SNDDRVBEEP_H_ */

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

	Uint8 *NewBuffer(void);
	Uint8 *NewBuffer(int slot);
	void DeleteBuffer(void);
	void DeleteBuffer(int slot);

	void SetVolume(Uint8 vol);
	void SetLRVolume(void);
	void SetRate(int rate);

	Uint8  *Setup(void *p);
	Uint8  *Setup(int tick);
	int BZero(int start, int uSamples, int slot, BOOL clear);
	int Render(int start, int uSamples, int slot, BOOL clear);
	void Enable(BOOL flag);
	void SetRenderVolume(int level);
	Mix_Chunk *GetChunk(void);
	Mix_Chunk *GetChunk(int slot);
private:
	std::vector<Uint8 *> buf;
	std::vector<Mix_Chunk>chunk;

	int bufSize;
	int bufSlot;
	int samples;
	UINT channels;
	UINT srate;
	int uStereo;
	Uint8 volume;

	int lastslot;
	UINT ms;
	UINT counter;
	int nLevel;
	BOOL enable;
	SDL_sem *RenderSem;

};

#endif /* SNDDRVBEEP_H_ */

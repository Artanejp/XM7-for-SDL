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
#include "xm7.h"
#include "sdl_snd.h"

class SndDrvTmpl {
public:
	SndDrvTmpl();
	virtual ~SndDrvTmpl();
	Uint8 *NewBuffer(void);
	void DeleteBuffer(void);
	Uint8  *Setup(int ch);
	void SetCh(int ch);
	void SetVolume(int vol);
	void Play(void);
	void Render(int msec, BOOL clear);
	void Enable(BOOL flag);
private:
	Uint8 *buf;
	int bufSize;
	int ms;
	int channels;
	int playCh;
	int srate;
	int howlong; /* 実際の演奏秒数 */
	Mix_Chunk chunk;
	BOOL enable;
};

#endif /* SNDDRVTMPL_H_ */

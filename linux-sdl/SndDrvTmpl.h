/*
 * SndDrvTmpl.h
 *
 *  Created on: 2010/09/25
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *  Changelog:
 *     05/16/2011 レンダラのみに特化、バッファ操作を排除
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
	void Setup(int tick);
	void Enable(BOOL flag);
	void SetChannels(int c);
	void SetRate(int rate);
	void SetRenderVolume(int level);
    int Render(Sint16 *pBuf, int start, int samples,  BOOL clear, BOOL bZero);
protected:
	UINT channels;
	UINT srate;
	UINT ms;
	UINT uStereo;
	int nLevel; /* レンダリングの音量 */
	BOOL enable;
	UINT counter;
	SDL_sem *RenderSem;
};

#endif /* SNDDRVTMPL_H_ */

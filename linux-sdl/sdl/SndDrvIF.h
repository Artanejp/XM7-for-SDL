/*
 * SndDrvIF.h
 *
 *  Created on: 2010/09/25
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *  Changelog:
 *     06/01/2011 インタフェース記述
 */

#ifndef SNDDRVIF_H_
#define SNDDRVIF_H_

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "xm7_sdl.h"
//#include "api_snd.h"
#include <vector>

#define DEFAULT_SLOT 8


class SndDrvIF {
public:
	virtual void Setup(int tick) = 0;
	virtual void Enable(BOOL flag)  = 0;
	virtual void SetChannels(int c) = 0;
	virtual void SetRate(int rate)  = 0;
	virtual void SetRenderVolume(int level) = 0;
    virtual int Render(Sint16 *pBuf, int start, int samples,  BOOL clear, BOOL bZero) = 0;

	virtual void SetState(BOOL state)= 0; // CMTのみ？

	virtual void SetFreq(int f)= 0; // Beepのみ?
	virtual void ResetCounter(BOOL flag) = 0; // Beepのみ？

	virtual void Setup(int tick, int opno) = 0; // OPNのみ？
	virtual BYTE GetCh3Mode(int opn) = 0; // OPNのみ？
	virtual void SetCh3Mode(int opn, Uint8 dat) = 0; // OPNのみ?

	virtual void SetRenderVolume(void) = 0; // OPNのみ？
	virtual void SetRenderVolume(int ch, int level) = 0; // OPNのみ？
	virtual void SetRenderVolume(int ch, int fm, int psg) = 0; // OPNのみ？

	virtual void SetLRVolume(void) = 0; // OPNのみ？
	virtual int *GetLVolume(int num) = 0; // OPNのみ？
	virtual int *GetRVolume(int num) = 0; // OPNのみ？

	virtual BYTE GetReg(int opn, BYTE) = 0; // OPNのみ？
	virtual void SetReg(int opn, BYTE reg, BYTE dat) = 0; // OPNのみ？
	virtual void SetReg(int opn, BYTE *reg) = 0; // OPNのみ？

	virtual int Render(Sint32 *pBuf32, Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero) = 0; // OPNのみ？
	virtual int Render32(Sint32 *pBuf32, int start, int sSamples, BOOL clear,BOOL bZero) = 0; // OPNのみ？
	virtual void Copy32(Sint32 *src, Sint16 *dst, int ofset, int samples) = 0; // OPNのみ？
	virtual int GetRenderCounter(void) = 0;
	virtual void ResetRenderCounter(void) = 0;
protected:
    int RenderCounter;
};

#endif /* SNDDRVIF_H_ */


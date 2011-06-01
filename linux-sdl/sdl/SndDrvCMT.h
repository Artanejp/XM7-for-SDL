/*
 * SndDrvCMT.h
 *
 *  Created on: 2010/10/05
 *      Author: whatisthis
 */

#ifndef SNDDRVCMT_H_
#define SNDDRVCMT_H_

#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "tapelp.h"
#include "sdl.h"
#include "sdl_sch.h"

#include "SndDrvIF.h"

class SndDrvCMT: public SndDrvIF {
public:
	SndDrvCMT();
	virtual ~SndDrvCMT();

	void SetRenderVolume(int level);
	void SetVolume(Uint8 level);
	void SetState(BOOL state);

	void Setup(int tick);
	void Enable(BOOL flag);
	void SetChannels(int c);
	void SetRate(int rate);
	int Render(Sint16 *pBuf, int start, int uSamples,  BOOL clear, BOOL bZero);

    // 以下、インタフェース互換のためのダミー関数
	void SetFreq(int f);
	void ResetCounter(BOOL flag);
	void Setup(int tick, int opno);
	BYTE GetCh3Mode(int opn);
	void SetCh3Mode(int opn, Uint8 dat);
	void SetRenderVolume(void);
	void SetRenderVolume(int ch, int level);
	void SetRenderVolume(int ch, int fm, int psg);
	void SetLRVolume(void);
	int *GetLVolume(int num);
	int *GetRVolume(int num);
	BYTE GetReg(int opn, BYTE);
	void SetReg(int opn, BYTE reg, BYTE dat);
	void SetReg(int opn, BYTE *reg);
	int Render(Sint32 *pBuf32, Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero);
	int Render32(Sint32 *pBuf32, int start, int sSamples, BOOL clear,BOOL bZero);
	void Copy32(Sint32 *src, Sint16 *dst, int ofset, int samples);

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

#endif /* SNDDRVCMT_H_ */

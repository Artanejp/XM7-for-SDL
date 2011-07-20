/*
 * SndDrvBeep.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVBEEP_H_
#define SNDDRVBEEP_H_

#include "SndDrvIF.h"

class SndDrvBeep: public SndDrvIF {
public:
	SndDrvBeep();
	virtual ~SndDrvBeep();


	void SetRenderVolume(int level);
	int Render(Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero);
	void ResetCounter(BOOL flag);
	void SetFreq(int f);

	void Setup(int tick);
	void Enable(BOOL flag);
	void SetChannels(int c);
	void SetRate(int rate);

    // 以下、インタフェース互換のためのダミー関数
	void SetState(BOOL state);
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
    int GetRenderCounter(void);
	void ResetRenderCounter(void);
protected:
	UINT channels;
	UINT srate;
	UINT ms;
	UINT uStereo;
	int nLevel; /* レンダリングの音量 */
	BOOL enable;
	UINT counter;
	SDL_sem *RenderSem;
private:
	int freq;
};

#endif /* SNDDRVBEEP_H_ */

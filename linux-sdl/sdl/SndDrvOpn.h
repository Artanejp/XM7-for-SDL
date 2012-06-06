/*
 * SndDrvOpn.h
 *
 *  Created on: 2010/09/28
 *      Author: whatisthis
 */

#ifndef SNDDRVOPN_H_
#define SNDDRVOPN_H_

//#include <SDL/SDL.h>
//#include <SDL/SDL_mixer.h>
#include "xm7.h"
#include "../fmgen/misc.h"
#include "opna.h"

#include "fmgen.h"
#include "fmgeninl.h"
#include "opn.h"
#include "psg.h"
#include "xm7_sdl.h"
//#include "sdl_sch.h"
//#include "sdl_snd.h"


#include <vector>
#include "SndDrvIF.h"

#define OPN_SLOT 8

class SndDrvOpn: public SndDrvIF {
public:
	SndDrvOpn();
	virtual ~SndDrvOpn();

	void Setup(int tick);
	void Setup(int tick, int opno);
	void Enable(BOOL flag);
	void SetChannels(int c);
	void SetRate(int rate);
	void SetRenderVolume(int level);
	void SetState(BOOL state); // CMTのみ？
	void SetFreq(int f); // Beepのみ?
	void ResetCounter(BOOL flag); // Beepのみ？


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
	int Render(Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero);
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
	SDL_sem *RenderSem;

private:
	void CopySoundBufferGeneric(DWORD * from, WORD * to, int size);
	void InitOpn(void);
	void DeleteOpn(void);
        void SetRate(int ch, unsigned int clk, int rate, BOOL hq);
	UINT counter;
	int nScale; /* プリスケーラ */
	UINT uChanSep;
	BYTE uCh3Mode[3];
	FM::OPN *pOPN;
};

#endif /* SNDDRVOPN_H_ */

/*
 * SndDrvOpn.h
 *
 *  Created on: 2010/09/28
 *      Author: whatisthis
 */

#ifndef SNDDRVOPN_H_
#define SNDDRVOPN_H_

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "xm7.h"
#include "cisc.h"
#include "opna.h"

#include "fmgen.h"
#include "fmgeninl.h"
#include "opn.h"
#include "psg.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "sdl_snd.h"


#include <vector>
#include "SndDrvTmpl.h"

#define OPN_SLOT 2

class SndDrvOpn: public SndDrvTmpl {
public:
	SndDrvOpn();
	virtual ~SndDrvOpn();
	Uint8 *NewBuffer(void);
	Uint8 *NewBuffer(int slot);
	void DeleteBuffer(void);
	void DeleteBuffer(int slot);
	Uint8 *Setup(int tick);
	Uint8 *Setup(int tick, int opno);
	BYTE GetCh3Mode(int opn);
	void SetCh3Mode(int opn, Uint8 dat);
	int GetBufSlots(void);

	void SetRenderVolume(void);
	void SetRenderVolume(int ch, int level);
	void SetRenderVolume(int ch, int fm, int psg);

	void SetLRVolume(void);
	int *GetLVolume(int num);
	int *GetRVolume(int num);

	BYTE GetReg(int opn, BYTE);
	void SetReg(int opn, BYTE reg, BYTE dat);
	void SetReg(int opn, BYTE *reg);
	int Render(int start, int uSamples,int slot,  BOOL clear);
	int BZero(int start, int uSamples,int slot,  BOOL clear);
	void Play(int ch, int slot, int samples);
private:
	DWORD *GetBuf32(int slot);
	void CopySoundBufferGeneric(DWORD * from, WORD * to, int size);
	void InitOpn(void);
	void DeleteOpn(void);
	Uint32 *buf32[DEFAULT_SLOT];
	UINT counter;
	int nScale; /* プリスケーラ */
	UINT uChanSep;
	BYTE uCh3Mode[3];
	FM::OPN *pOPN;
	int bufSlot;
};

#endif /* SNDDRVOPN_H_ */

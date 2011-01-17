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
#include "fmgen/misc.h"
#include "fmgen/opna.h"

#include "fmgen/fmgen.h"
#include "fmgen/fmgeninl.h"
#include "opn.h"
#include "fmgen/psg.h"
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
	BYTE GetCh3Mode(void);
	void SetCh3Mode(Uint8 dat);
	int GetBufSlots(void);

	void SetRenderVolume(void);
	void SetRenderVolume(int level);
	void SetRenderVolume(int fm, int psg);

	void SetLRVolume(void);
	int *GetLVolume(int num);
	int *GetRVolume(int num);

	BYTE GetReg(BYTE);
	void SetReg(BYTE reg, BYTE dat);
	void SetReg(BYTE *reg);
	void SetOpNo(int num);

	int Render(int start, int uSamples,int slot,  BOOL clear);
	int BZero(int start, int uSamples,int slot,  BOOL clear);
	int GetLevelSnd(int ch);
	void Play(int ch, int slot, int samples);
private:
	void CopySoundBufferGeneric(DWORD * from, WORD * to, int size);
	void InitOpn(void);
	void DeleteOpn(void);
	int opn_number;
	UINT counter;
	int nScale; /* プリスケーラ */
	UINT uChanSep;
	BYTE uCh3Mode;
	FM::OPN *pOPN;
	int bufSlot;
};

#endif /* SNDDRVOPN_H_ */

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
	BYTE GetCh3Mode(void);
	void SetCh3Mode(Uint8 dat);
	int GetBufSlots(void);

	void SetRenderVolume(void);
	void SetRenderVolume(int level);
	void SetRenderVolume(int fm, int psg);

	void SetLRVolume(void);
	BYTE GetReg(BYTE);
	void SetReg(BYTE reg, BYTE dat);
	void SetReg(BYTE *reg);
	void SetOpNo(int num);

	void SetVolume(Uint8 level);
	Mix_Chunk *GetChunk(void);
	Mix_Chunk *GetChunk(int slot);
	int Render(int start, int uSamples,int slot,  BOOL clear);
	int BZero(int start, int uSamples,int slot,  BOOL clear);
	int GetLevelSnd(int ch);
private:
	void CopySoundBufferGeneric(DWORD * from, WORD * to, int size);
	void InitOpn(void);
	void DeleteOpn(void);
	Uint8 *buf[OPN_SLOT];
	Uint32 *buf32[OPN_SLOT];
	Mix_Chunk chunk[OPN_SLOT];

	int bufSize;
	int samples;
	UINT channels;
	UINT srate;
	UINT ms;
	UINT uStereo;
	int nLevel; /* レンダリングの音量 */
	Uint8 volume; /* 出力する音量 */
	BOOL enable;
	SDL_sem *RenderSem;
	int opn_number;
	UINT counter;
	int nScale; /* プリスケーラ */
	UINT uChanSep;
	BYTE uCh3Mode;
	FM::OPN *pOPN;
	int bufSlot;
};

#endif /* SNDDRVOPN_H_ */

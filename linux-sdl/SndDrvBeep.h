/*
 * SndDrvBeep.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVBEEP_H_
#define SNDDRVBEEP_H_

#include "SndDrvTmpl.h"

class SndDrvBeep: public SndDrvTmpl {
public:
	SndDrvBeep();
	virtual ~SndDrvBeep();

	void SetVolume(Uint8 vol);
	int BZero(int start, int uSamples, int slot, BOOL clear);
	int Render(int start, int uSamples, int slot, BOOL clear);
	void SetRenderVolume(int level);
	int Render32(Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero);
private:

};

#endif /* SNDDRVBEEP_H_ */

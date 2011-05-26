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
	void SetRenderVolume(int level);
	int Render(Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero);
	void ResetCounter(BOOL flag);
	void SetFreq(int f);

private:
	int freq;
};

#endif /* SNDDRVBEEP_H_ */

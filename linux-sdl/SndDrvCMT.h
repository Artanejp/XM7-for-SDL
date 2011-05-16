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

#include "SndDrvTmpl.h"

class SndDrvCMT: public SndDrvTmpl {
public:
	SndDrvCMT();
	virtual ~SndDrvCMT();
	void SetRenderVolume(int level);
	void SetVolume(Uint8 level);
	void SetState(BOOL state);
	int Render(Sint16 *pBuf, int start, int uSamples,  BOOL clear, BOOL bZero);
};

#endif /* SNDDRVCMT_H_ */

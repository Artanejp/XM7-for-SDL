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
	void Render(int msec, BOOL clear);
	Uint8 *NewBuffer(void);
	void DeleteBuffer(void);
private:
	Uint8 *buf;
	int bufSize;
	int ms;
	int channels;
	int playCh;
	int srate;
	int howlong; /* 実際の演奏秒数 */
	Mix_Chunk chunk;
	BOOL enable;
	int counter;
};

#endif /* SNDDRVBEEP_H_ */

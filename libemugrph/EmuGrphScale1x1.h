/*
 * EmuGrphScale1x1.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE1X1_H_
#define EMUGRPHSCALE1X1_H_

#include "EmuGrphScale1x2.h"

class EmuGrphScale1x1: public EmuGrphScale1x2 {
public:
	EmuGrphScale1x1();
	virtual ~EmuGrphScale1x1();
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
};

#endif /* EMUGRPHSCALE1X1_H_ */

/*
 * EmuGrphScale2x2.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE2X2_H_
#define EMUGRPHSCALE2X2_H_

#include "EmuGrphScale2x4.h"

class EmuGrphScale2x2: public EmuGrphScale2x4 {
public:
	EmuGrphScale2x2();
	virtual ~EmuGrphScale2x2();
	void PutVram(SDL_Surface* p, int x, int y, int w, int h, Uint32 mpage);
};

#endif /* EMUGRPHSCALE2X2_H_ */

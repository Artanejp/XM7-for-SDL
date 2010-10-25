/*
 * EmuGrphScale2x4i.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE2X4I_H_
#define EMUGRPHSCALE2X4I_H_

#include "EmuGrphScale2x4.h"

class EmuGrphScale2x4i: public EmuGrphScale2x4 {
public:
	EmuGrphScale2x4i();
	virtual ~EmuGrphScale2x4i();
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
};

#endif /* EMUGRPHSCALE2X4I_H_ */

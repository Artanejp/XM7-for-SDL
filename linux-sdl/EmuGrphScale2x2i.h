/*
 * EmuGrphScale2x2i.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE2X2I_H_
#define EMUGRPHSCALE2X2I_H_

#include "EmuGrphScale2x4i.h"

class EmuGrphScale2x2i: public EmuGrphScale2x4i {
public:
	EmuGrphScale2x2i();
	virtual ~EmuGrphScale2x2i();
	void PutVram(SDL_Surface* p, int x, int y, int w, int h, Uint32 mpage);
};

#endif /* EMUGRPHSCALE2X2I_H_ */

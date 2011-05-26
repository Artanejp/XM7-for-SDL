/*
 * EmuGrphScale1x2i.h
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE1X2I_H_
#define EMUGRPHSCALE1X2I_H_

#include "EmuGrphScale1x2.h"

class EmuGrphScale1x2i: public EmuGrphScale1x2 {
public:
	EmuGrphScale1x2i();
	virtual ~EmuGrphScale1x2i();
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
};

#endif /* EMUGRPHSCALE1X2I_H_ */

/*
 * EmuGrphScale2x4.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE2X4_H_
#define EMUGRPHSCALE2X4_H_

#include "EmuGrphScale1x2.h"

class EmuGrphScale2x4: public EmuGrphScale1x2 {
public:
	EmuGrphScale2x4();
	virtual ~EmuGrphScale2x4();
	void PutWord2x(Uint32 *disp,Uint32 pixels, Uint32 *cbuf);
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
};

#endif /* EMUGRPHSCALE2X4_H_ */

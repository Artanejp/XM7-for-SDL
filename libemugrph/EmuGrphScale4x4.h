/*
 * EmuGrphScale4x4.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE4X4_H_
#define EMUGRPHSCALE4X4_H_

#include "EmuGrphScale2x4.h"

class EmuGrphScale4x4: public EmuGrphScale2x4 {
public:
	EmuGrphScale4x4();
	virtual ~EmuGrphScale4x4();
	void PutWord4x(Uint32 *disp,Uint32 pixels, Uint32 *cbuf);
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
};

#endif /* EMUGRPHSCALE4X4_H_ */

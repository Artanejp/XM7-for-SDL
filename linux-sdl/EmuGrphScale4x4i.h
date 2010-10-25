/*
 * EmuGrphScale4x4i.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE4X4I_H_
#define EMUGRPHSCALE4X4I_H_

#include "EmuGrphScale4x4.h"

class EmuGrphScale4x4i: public EmuGrphScale4x4 {
public:
	EmuGrphScale4x4i();
	virtual ~EmuGrphScale4x4i();
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);

};

#endif /* EMUGRPHSCALE4X4I_H_ */

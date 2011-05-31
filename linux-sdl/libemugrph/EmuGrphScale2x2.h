/*
 * EmuGrphScale2x2.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE2X2_H_
#define EMUGRPHSCALE2X2_H_

#include "EmuGrphScaleTmpl.h"

class EmuGrphScale2x2: public EmuGrphScaleTmpl {
public:
	EmuGrphScale2x2();
	virtual ~EmuGrphScale2x2();
	void SetVramReader(void f(Uint32, Uint32 *, Uint32), int w, int h);
	void SetConvWord(void f(SDL_Surface *, Uint32 *, Uint32 *));
	void SetPutWord(void f(Uint32 *, Uint32, Uint32 *));
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
protected:
	void (*getvram)(Uint32, Uint32 *, Uint32);
	void (*putword)(Uint32 *, Uint32 , Uint32 *);
	void (*convword)(SDL_Surface *, Uint32 *, Uint32 *);
	int vramwidth;
	int vramheight;
};

#endif /* EMUGRPHSCALE2X2_H_ */

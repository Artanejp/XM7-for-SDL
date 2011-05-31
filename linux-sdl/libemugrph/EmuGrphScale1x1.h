/*
 * EmuGrphScale1x1.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE1X1_H_
#define EMUGRPHSCALE1X1_H_

#include "EmuGrphScaleTmpl.h"

class EmuGrphScale1x1: public EmuGrphScaleTmpl {
public:
	EmuGrphScale1x1();
	virtual ~EmuGrphScale1x1();
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void SetConvWord(void f(SDL_Surface *, Uint32 *, Uint32 *));
	void SetPutWord(void f(Uint32 *, Uint32, Uint32 *));
    void SetVramReader(void p(Uint32, Uint32 *, Uint32), int w, int h);
protected:
	void (*getvram)(Uint32, Uint32 *, Uint32);
	void (*putword)(Uint32 *, Uint32 , Uint32 *);
	void (*convword)(SDL_Surface *, Uint32 *, Uint32 *);
	int vramwidth;
	int vramheight;

};

#endif /* EMUGRPHSCALE1X1_H_ */

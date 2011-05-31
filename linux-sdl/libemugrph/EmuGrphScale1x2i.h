/*
 * EmuGrphScale1x2i.h
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#ifndef EMUGRPHSCALE1X2I_H_
#define EMUGRPHSCALE1X2I_H_

#include "EmuGrphScaleTmpl.h"

class EmuGrphScale1x2i: public EmuGrphScaleTmpl {
public:
	EmuGrphScale1x2i();
	virtual ~EmuGrphScale1x2i();
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void SetVramReader(void f(Uint32, Uint32 *, Uint32), int w, int h);
	void SetConvWord(void f(SDL_Surface *, Uint32 *, Uint32 *));
	void SetPutWord(void f(Uint32 *, Uint32, Uint32 *));
protected:
	void (*getvram)(Uint32, Uint32 *, Uint32);
	void (*putword)(Uint32 *, Uint32 , Uint32 *);
	void (*convword)(SDL_Surface *, Uint32 *, Uint32 *);
	int vramwidth;
	int vramheight;

};

#endif /* EMUGRPHSCALE1X2I_H_ */

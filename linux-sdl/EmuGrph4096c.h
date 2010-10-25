/*
 * EmuGrph4096c.h
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#ifndef EMUGRPH4096C_H_
#define EMUGRPH4096C_H_

#include "EmuGrphLib.h"

class EmuGrph4096c: public EmuGrphLib {
public:
	EmuGrph4096c();
	virtual ~EmuGrph4096c();
	void CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Surface *disp);
	void SetPaletteTable(Uint32 *p);
	void PutVram(BOOL interlace);
	void InitPalette(void);
	void GetVram(Uint32 addr, Uint32 *cbuf, Uint32 mpage);
	void PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *c);
	void PutWordx2(Uint32 *disp, Uint32 pixsize, Uint32 *c);
protected:
	void GetVram(Uint32 addr, Uint32 *cbuf);
};

#endif /* EMUGRPH4096C_H_ */

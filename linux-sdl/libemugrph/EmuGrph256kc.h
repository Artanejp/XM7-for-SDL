/*
 * EmuGrph256kc.h
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#ifndef EMUGRPH256KC_H_
#define EMUGRPH256KC_H_

#include "EmuGrphLib.h"
#include "EmuGrph4096c.h"

class EmuGrph256kc: public EmuGrph4096c {
public:
	EmuGrph256kc();
	virtual ~EmuGrph256kc();
	void CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Surface *disp);
	void InitPalette(void);
	void GetVram(Uint32 addr, Uint32 *cbuf2, Uint32 mpage);
	void PutVram(BOOL interlace);
	void SetVram(Uint8 *p, Uint32 w, Uint32 h);
	void SetVram(Uint8 *pr, Uint8 *pg, Uint8 *pb, Uint32 w, Uint32 h);
protected:
	Uint8 *vram_p;
};

#endif /* EMUGRPH256KC_H_ */

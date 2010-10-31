/*
 * EmuGrphLib.h
 *
 *  Created on: 2010/10/25
 *      Author: whatisthis
 */

#ifndef EMUGRPHLIB_H_
#define EMUGRPHLIB_H_

#include <SDL/SDL.h>

typedef int BOOL;

class EmuGrphLib {
public:
	EmuGrphLib();
	virtual ~EmuGrphLib();
	void CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Surface *disp);
	void CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	void SetVram(Uint8 *p, Uint32 w, Uint32 h);
	void SetVram(Uint8 *pr, Uint8 *pg, Uint8 *pb, Uint32 w, Uint32 h);
	void SetPaletteTable(Uint32 *p);
	void PutVram(BOOL interlace);
	void InitPalette(void);
	void ConvWord(SDL_Surface *p, Uint32 *dst, Uint32 *src);
	void GetVram(Uint32 addr, Uint32 *cbuf, Uint32 mpage);
	void PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *c);
protected:
	void GetVram(Uint32 addr, Uint32 *cbuf);
	Uint32 vram_w;
	Uint32 vram_h;
	Uint8 *vram_pr;
	Uint8 *vram_pg;
	Uint8 *vram_pb;
	Uint32 *palette;

};

#endif /* EMUGRPHLIB_H_ */

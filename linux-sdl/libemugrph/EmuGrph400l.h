/*
 * EmuGrph400l.h
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#ifndef EMUGRPH400L_H_
#define EMUGRPH400L_H_

#include "EmuGrphLib.h"

class EmuGrph400l: public EmuGrphLib {
public:
	EmuGrph400l();
	virtual ~EmuGrph400l();
	void GetVram(Uint32 addr, Uint32 *cbuf, Uint32 mpage);
	void PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *c);
	void PutVram(BOOL interlace);
protected:
	void GetVram(Uint32 addr, Uint32 *cbuf);
};

#endif /* EMUGRPH400L_H_ */

/*
 * EmuAgarGL.h
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 */

#ifndef EMUAGARGL_H_
#define EMUAGARGL_H_

#include "EmuGLUtils.h"
#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>

#ifndef Uint
#define Uint unsigned int
#endif

class EmuAgarGL: public EmuGLUtils {
public:
	EmuAgarGL();
	virtual ~EmuAgarGL();
	void InitUI(char *name);
	void InitUI(char *name, Uint Flags);
	void InitGL(int w, int h);
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void SetViewPort(int x, int y, int w, int h);
	void SetViewPort(void);
	void Flip(void);
    void SetScanLine(BOOL flag);
    void CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void SetPaletteTable(Uint32 *p);

protected:
    AG_Surface *video;
    AG_PixelFormat format;
    Uint32 *palette;
    BOOL   InitVideo;
private:
};

#endif /* EMUAGARGL_H_ */

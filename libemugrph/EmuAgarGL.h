/*
 * EmuAgarGL.h
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 */

#ifndef EMUAGARGL_H_
#define EMUAGARGL_H_

#include "EmuGLUtils.h"
#include <SDL.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>


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
	void SetDrawArea(AG_Window *p, int x, int y, int w, int h);
	void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void PutVram(AG_Surface *p, int x, int y, int w, int h, Uint32 mpage);
	void Flip(void);
    void CalcPalette(Uint32 src, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void SetPaletteTable(Uint32 *p);
protected:
    AG_Surface *video;
    Uint32 *palette;
    BOOL   InitVideo;
    Uint textureid;
    AG_Box *drawarea;
    AG_PixelFormat format;
private:
    BOOL  UseTexture;
};

#endif /* EMUAGARGL_H_ */

/*
* Scaler I/F Template
*  2011/05/31 K.Ohta <whatisthis.sowhat@gmail.com>
*/
#ifndef __EMU_GRPH_SCALE_TMPL_H
#define __EMU_GRPH_SCALE_TMPL_H

#include <SDL/SDL.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef BOOL
typedef int BOOL;
#endif

class EmuGrphScaleTmpl {
public:
//	virtual ~EmuGrphScaleTmpl() = 0;
	virtual void SetVramReader(void f(Uint32, Uint32 *, Uint32), int w, int h) = 0;
	virtual void SetConvWord(void f(SDL_Surface *, Uint32 *, Uint32 *)) = 0;
	virtual void SetPutWord(void f(Uint32 *, Uint32, Uint32 *)) = 0;
	virtual void PutVram(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage) = 0;
protected:
//	virtual void (*getvram)(Uint32, Uint32 *, Uint32) = 0;
//	virtual void (*putword)(Uint32 *, Uint32 , Uint32 *) = 0;
//	virtual void (*convword)(SDL_Surface *, Uint32 *, Uint32 *) = 0;
//	virtual int vramwidth = 0;
//	virtual int vramheight = 0;

};

#endif /* __EMU_GRPH_SCALE_TMPL_H */

/*
 * api_scaler.h
 *
 *  Created on: 2010/11/03
 *      Author: whatisthis
 */

#ifndef API_SCALER_H_
#define API_SCALER_H_

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#ifdef USE_OPENGL
#include <SDL/SDL_opengl.h>
#endif
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"
#include "sdl.h"
#include "api_draw.h"
#include "xm7.h"

#ifdef __cplusplus
#include "EmuGrphLib.h"
#include "EmuGrph400l.h"
#include "EmuGrph4096c.h"
#include "EmuGrph256kc.h"

#include "EmuGrphScale1x1.h"
#include "EmuGrphScale1x2.h"
#include "EmuGrphScale1x2i.h"
#include "EmuGrphScale2x2.h"
#include "EmuGrphScale2x2i.h"
#include "EmuGrphScale2x4.h"
#include "EmuGrphScale2x4i.h"
#include "EmuGrphScale4x4.h"
#include "EmuGrphScale4x4i.h"
#include "EmuGLUtils.h"

extern EmuGrphLib *vramhdr;
extern EmuGrph400l *vramhdr_400l;
extern EmuGrph4096c *vramhdr_4096;
extern EmuGrph256kc *vramhdr_256k;

extern EmuGrphScale1x1 *scaler1x1;
extern EmuGrphScale1x2 *scaler1x2;
extern EmuGrphScale1x2i *scaler1x2i;
extern EmuGrphScale2x2 *scaler2x2;
extern EmuGrphScale2x2i *scaler2x2i;
extern EmuGrphScale2x4 *scaler2x4;
extern EmuGrphScale2x4i *scaler2x4i;
extern EmuGrphScale4x4 *scaler4x4;
extern EmuGrphScale4x4i *scaler4x4i;

extern EmuGLUtils *scalerGL;
#endif


#ifdef __cplusplus
extern "C"
{
#endif
/*
 * 初期化関連
 */
extern void init_scaler(void);
extern void initsub_scaler(void);
extern void detachsub_scaler(void);

/*
 * VRAMAPI
 */
extern void SetVramReader_200l(void);
extern void SetVramReader_400l(void);
extern void SetVramReader_4096(void);
extern void SetVramReader_256k(void);

/*
 * 描画関連
 */
extern void SetupGL(int w, int h);
extern void InitGL(int w, int h);
extern void Flip(void);

/*
 * スケーラ
 */
extern void Scaler_1x2(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern void Scaler_1x2i(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern void Scaler_2x4(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern void Scaler_2x4i(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern void Scaler_GL(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern void Scaler_1x1(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip);
extern void Scaler_2x2(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip);
extern void Scaler_2x2i(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip);
extern void Scaler_4x4(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip);
extern void Scaler_4x4i(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip);

#ifdef __cplusplus
}
#endif

#endif /* API_SCALER_H_ */

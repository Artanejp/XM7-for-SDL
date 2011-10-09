/*
 * api_scaler.h
 *
 *  Created on: 2010/11/03
 *      Author: whatisthis
 */

#ifndef API_SCALER_H_
#define API_SCALER_H_

#include <SDL.h>
#include <SDL_syswm.h>

#ifndef USE_AGAR
#ifdef USE_OPENGL
#include <SDL_opengl.h>
#endif
#endif

#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif
#include "api_draw.h"
#include "xm7.h"

#ifdef __cplusplus
#include "EmuGrphLib.h"
#include "EmuGrph400l.h"
#include "EmuGrph4096c.h"
#include "EmuGrph256kc.h"

#include "EmuGrphScaleTmpl.h"
#include "EmuGrphScale1x1.h"
#include "EmuGrphScale1x2.h"
#include "EmuGrphScale1x2i.h"
#include "EmuGrphScale2x2.h"
#include "EmuGrphScale2x2i.h"
#include "EmuGrphScale2x4.h"
#include "EmuGrphScale2x4i.h"
#include "EmuGrphScale4x4.h"
#include "EmuGrphScale4x4i.h"

#ifdef USE_AGAR
//#include "EmuAgarGL.h"
#else
#include "EmuGLUtils.h"
#endif

extern EmuGrphLib *vramhdr;
extern EmuGrph400l *vramhdr_400l;
extern EmuGrph4096c *vramhdr_4096;
extern EmuGrph256kc *vramhdr_256k;

extern EmuGrphScaleTmpl *pSwScaler;

#ifdef USE_AGAR
//extern EmuAgarGL *pScalerGL;
#else
extern EmuGLUtils *pScalerGL;
#endif
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
extern  void Scaler_GL(SDL_Surface *p, int x, int y, int w, int h, Uint32 mpage);
extern  void SwScaler(SDL_Surface *p, int x, int y, int w, int h, Uint32 multip);
extern  void SelectScaler(int w, int h);

#ifdef __cplusplus
}
#endif

#endif /* API_SCALER_H_ */

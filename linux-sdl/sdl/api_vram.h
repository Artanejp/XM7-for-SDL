/*
 * XM7: Vram Emulation Routines
 *  (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */
#ifndef API_VRAM_H_INCLUDED
#define API_VRAM_H_INCLUDED

#include <SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include "api_draw.h"
//#include "api_scaler.h"

#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"

#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_xm7.h"

/*
 * api_vram8.cpp
*/
extern void SetVram_200l(Uint8 *p);
extern void getvram_400l(Uint32 addr,Uint32 *p, Uint32 mpage);
extern void getvram_200l(Uint32 addr,Uint32 *p, Uint32 mpage);

extern void CalcPalette_8colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern void CreateVirtualVram8(Uint32 *p, int x, int y, int w, int h, int mode);
extern void CreateVirtualVram8_1Pcs(Uint32 *p, int x, int y, int pitch, int mpage);
extern void CreateVirtualVram8_1Pcs_Nopal(Uint32 *p, int x, int y, int pitch, int mpage);

/*
 * api_vram4096.cpp
 */
extern void CalcPalette_4096Colors(Uint32 index, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern void CreateVirtualVram4096(Uint32 *p, int x, int y, int w, int h, int mode, Uint32 mpage);
extern void CreateVirtualVram4096_1Pcs(Uint32 *p, int x, int y, int pitch, int mpage);
extern void CreateVirtualVram4096_1Pcs_Nopal(Uint32 *p, int x, int y, int pitch, int mode);

/*
 * api_vram256k.cpp
 */
extern void CreateVirtualVram256k(Uint32 *p, int x, int y, int w, int h, int mode, Uint32 mpage);
extern void CreateVirtualVram256k_1Pcs(Uint32 *p, int x, int y, int pitch, int mpage);

#endif // API_VRAM_H_INCLUDED

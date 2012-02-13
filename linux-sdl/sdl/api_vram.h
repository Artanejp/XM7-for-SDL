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
#include "../fmgen/types.h"

/*
 * api_vram8.cpp
*/
#ifdef __cplusplus
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
#endif // C++ ONLY

#ifdef __cplusplus
extern "C" {
#endif
//typedef uint16_t v4si __attribute__ ((__vector_size__(16), aligned(16)));
//typedef uint16_t v8si __attribute__ ((__vector_size__(32), aligned(16)));


//typedef union {
//     v4si v;
//     uint32_t i[4];
//     uint16_t s[8];
//     uint8_t  b[16];
//} v4hi;


//typedef union {
//     v8si v;
//     uint32_t i[8];
//     uint16_t s[16];
//     uint8_t  b[32];
//} v8hi;

extern void getvram_4096_vec(Uint32 addr, v4hi *cbuf);
static inline void putword2_vec(Uint32 *disp, v4hi cbuf)
{
   disp[0] = rgbAnalogGDI[cbuf.s[0]];
   disp[1] = rgbAnalogGDI[cbuf.s[1]];
   disp[2] = rgbAnalogGDI[cbuf.s[2]];
   disp[3] = rgbAnalogGDI[cbuf.s[3]];
   disp[4] = rgbAnalogGDI[cbuf.s[4]];
   disp[5] = rgbAnalogGDI[cbuf.s[5]];
   disp[6] = rgbAnalogGDI[cbuf.s[6]];
   disp[7] = rgbAnalogGDI[cbuf.s[7]];
}

extern __volatile__ void initvramtbl_8_vec(void);
extern __volatile__ void initvramtbl_4096_vec(void);

extern void getvram_8_vec(Uint32 addr, v4hi *cbuf);
static inline void  putword8_vec(Uint32 *disp, v4hi *p)
{
    disp[0] = rgbTTLGDI[p->s[0]];
    disp[1] = rgbTTLGDI[p->s[1]];
    disp[2] = rgbTTLGDI[p->s[2]];
    disp[3] = rgbTTLGDI[p->s[3]];
    disp[4] = rgbTTLGDI[p->s[4]];
    disp[5] = rgbTTLGDI[p->s[5]];
    disp[6] = rgbTTLGDI[p->s[6]];
    disp[7] = rgbTTLGDI[p->s[7]];
}


enum {
   PLAINB0 = 0,
   PLAINB1,
   PLAINB2,
   PLAINB3,
   PLAINR0,
   PLAINR1,
   PLAINR2,
   PLAINR3,
   PLAING0,
   PLAING1,
   PLAING2,
   PLAING3
};

enum {
   PLAINB = 0,
   PLAINR,
   PLAING,
   PLAINW
};


#ifdef __cplusplus
}
#endif


#endif // API_VRAM_H_INCLUDED

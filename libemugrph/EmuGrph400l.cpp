/*
 * EmuGrph400l.cpp
 *
 *  Created on: 2010/10/26
 *      Author: whatisthis
 */

#include "EmuGrph400l.h"

EmuGrph400l::EmuGrph400l() {
	// TODO Auto-generated constructor stub

}

EmuGrph400l::~EmuGrph400l() {
	// TODO Auto-generated destructor stub
}



void EmuGrph400l::GetVram(Uint32 addr, Uint32 *cbuf, Uint32 mpage)
{
	GetVram(addr, cbuf);
}


void EmuGrph400l::GetVram(Uint32 addr, Uint32 *cbuf)
{
   Uint8    cb,
            cr,
            cg;

        cb = vram_pb[addr];
        cr = vram_pr[addr];
        cg = vram_pg[addr];
#if 1
        cbuf[0] =   palette[(cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2)];
        cbuf[1] =   palette[((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1)];
        cbuf[2] =   palette[((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04)];
        cbuf[3] =   palette[((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +((cg & 0x08) >> 1)];
        cbuf[4] =   palette[((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) + ((cg & 0x10) >> 2)];
        cbuf[5] =   palette[((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) + ((cg & 0x20) >> 3)];
        cbuf[6] =   palette[((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) + ((cg & 0x40) >> 4)];
        cbuf[7] =   palette[((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) + ((cg & 0x80) >> 5)];
#endif
}

static inline void
__SETDOT(Uint8 * addr, Uint32 c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
#endif
}


void EmuGrph400l::PutWord(Uint32 *disp, Uint32 pixsize, Uint32 *c)
{
	Uint8 *d = (Uint8 *)disp;
	__SETDOT(d, c[7]);
	d += pixsize;
	__SETDOT(d, c[6]);
	d += pixsize;
	__SETDOT(d, c[5]);
	d += pixsize;
	__SETDOT(d, c[4]);
	d += pixsize;
	__SETDOT(d, c[3]);
	d += pixsize;
	__SETDOT(d, c[2]);
	d += pixsize;
	__SETDOT(d, c[1]);
	d += pixsize;
	__SETDOT(d, c[0]);
}

/*
 * 以下、ダミー
 */
void EmuGrph400l::PutVram(BOOL interlace)
{

}

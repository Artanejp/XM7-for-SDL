/*
 * sdl_draw_320.h
 *
 *  Created on: 2010/09/13
 *      Author: K.Ohta
 */
/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *      Copyright (C) 2001-2003 Ryu Takegami
 *      Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *      [ SDL 表示 / 共通ルーチンヘッダ]
 *               20100125 sdl_draw.cを分割
 *               20100130 sdl_draw_*.cからインラインマクロなど分離
 *				 20100913 sdl_drawcommon.h から320ドット描画関連分離
 */
#ifndef SDL_DRAW_320_H_
#define SDL_DRAW_320_H_

#ifndef SDL_DRAWCOMMON_H
#include <SDL.h>
#include "xm7.h"
#include "sdl_draw.h"
#include "sdl.h"

/*
 * 共通ルーチンのextern設定
 */
extern BOOL     BitBlt(int nDestLeft, int nDestTop, int nWidth,
		       int nHeight, int nSrcLeft, int nSrcTop);
extern void     RenderFullScan(void);
//extern void     RenderSetOddLine(void);
extern void     SetDrawFlag(BOOL flag);
extern void     AllClear(void);
extern SDL_Surface *realDrawArea;	/* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する)  */
#endif

/*
 * 320x200,非フルスキャン、原寸
 * (DDRAW)
 */
static inline void
__SETDOT_DDRAW_320i(Uint8 * addr, int bpp, int pitch, DWORD c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
#endif
}
/*
 * 320x200,非フルスキャン、原寸
 * (DDRAW) 16bpp
 */
static inline void
__SETDOT_DDRAW_320i_16(Uint8 * addr, int bpp, int pitch, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 0) & 0xff;	/* R */
    addr[0] = (c >> 8) & 0xff;	/* G */
    addr += bpp;
    addr[1] = (c >> 0) & 0xff;	/* R */
    addr[0] = (c >> 8) & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
#endif
}


/*
 * 320x200,非フルスキャン、原寸
 * (DDRAW)
 */
static inline void
__SETDOT_DDRAW_320p(Uint8 * addr, int bpp, int pitch, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += pitch;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr -= bpp;
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */


#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += pitch;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr -= bpp;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
#endif
}

/*
 * 320x200,非フルスキャン、原寸
 * (DDRAW) 16/15bpp
 */
static inline void
__SETDOT_DDRAW_320p_16(Uint8 * addr, int bpp, int pitch, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = c  & 0xff;	/* G */
    addr += bpp;
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = c  & 0xff;	/* G */
    addr += pitch;

    addr[1] = (c >> 8)& 0xff;		/* B */
    addr[0] = c  & 0xff;	/* R */
    addr -= bpp;
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = c  & 0xff;	/* G */


#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += pitch;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr -= bpp;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
#endif
}


/*
 * 320x200,フルスキャン、倍角
 * (DDRAW)
 * 16/15bpp
 */

static inline void
__SETDOT_DDRAW_320p_DUP_16(Uint8 * addr, int bpp, int pitch, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    /*
     * 上段
     */
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += (pitch - bpp * 3);

    /*
     * 下段
     */
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    // addr += (pitch - bpp * 3);
#else
    /*
     * 上段
     */
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += (pitch - bpp * 3);

    /*
     * 下段
     */
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    // addr += (pitch - bpp * 3);


#endif
}

/*
 * 320x200,フルスキャン、倍角
 * (DDRAW)
 */

static inline void
__SETDOT_DDRAW_320p_DUP(Uint8 * addr, int bpp, int pitch, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    /*
     * 上段
     */
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += (pitch - bpp * 3);

    /*
     * 下段
     */
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    // addr += (pitch - bpp * 3);
#else
    /*
     * 上段
     */
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += (pitch - bpp * 3);

    /*
     * 下段
     */
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += bpp;

    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    // addr += (pitch - bpp * 3);


#endif
}

/*
 * 320x200,非フルスキャン、倍角
 * (DDRAW)
 */

static inline void
__SETBYTE_DDRAW_320i(Uint8 * addr, int bpp, int pitch, DWORD *c)
{
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[0]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[1]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[2]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[3]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[4]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[5]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[6]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i(addr, bpp, pitch, c[7]);
        // addr += bpp * 2;
}

/*
 * 320x200,非フルスキャン、倍角
 * (DDRAW)
 * 16bpp
 */

static inline void
__SETBYTE_DDRAW_320i_16(Uint8 * addr, int bpp, int pitch, DWORD *c)
{
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[0]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[1]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[2]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[3]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[4]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[5]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[6]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320i_16(addr, bpp, pitch, c[7]);
        // addr += bpp * 2;
}

/*
 * 一バイト単位で描画
 * 320x200,非フルスキャン、原寸
 * (DDRAW)
 */

static inline void
__SETBYTE_DDRAW_320p(Uint8 * addr, int bpp, int pitch, DWORD *c)
{
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[0]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[1]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[2]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[3]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[4]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[5]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[6]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p(addr, bpp, pitch, c[7]);
        // addr += bpp * 2;
}

/*
 * 一バイト単位で描画
 * 320x200,非フルスキャン、原寸
 * (DDRAW)
 * 16bpp
 */

static inline void
__SETBYTE_DDRAW_320p_16(Uint8 * addr, int bpp, int pitch, DWORD *c)
{
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[0]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[1]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[2]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[3]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[4]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[5]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[6]);
        addr += bpp * 2;
        __SETDOT_DDRAW_320p_16(addr, bpp, pitch, c[7]);
        // addr += bpp * 2;
}

/*
 * 一バイト単位で描画
 * 320x200,非フルスキャン、倍角
 * (DDRAW)
 */

static inline void
__SETBYTE_DDRAW_1280_320i(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[0]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[1]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[2]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[3]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[4]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[5]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[6]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[7]);
        // addr += bpp * 4;
}

/*
 * 一バイト単位で描画
 * 320x200,非フルスキャン、倍角
 * (DDRAW)
 * 16bpp
 */

static inline void
__SETBYTE_DDRAW_1280_320i_16(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[0]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[1]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[2]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[3]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[4]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[5]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[6]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[7]);
        // addr += bpp * 4;
}

/*
 * 一バイト単位で描画
 * 320x200,フルスキャン、倍角
 * (DDRAW)
 */

static inline void
__SETBYTE_DDRAW_1280_320p(Uint8 * addr, int bpp, int pitch, DWORD *c)
{
        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[0]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[0]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[1]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[1]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[2]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[2]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[3]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[3]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[4]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[4]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[5]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[5]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[6]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[6]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP(addr, bpp, pitch, c[7]);
        __SETDOT_DDRAW_320p_DUP(addr + pitch, bpp, pitch, c[7]);
        // addr += bpp * 4;

}


/*
 * 一バイト単位で描画
 * 320x200,フルスキャン、倍角
 * (DDRAW)
 * 16bpp
 */

static inline void
__SETBYTE_DDRAW_1280_320p_16(Uint8 * addr, int bpp, int pitch, DWORD *c)
{
        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[0]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[0]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[1]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[1]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[2]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[2]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[3]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[3]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[4]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[4]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[5]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[5]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[6]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[6]);
        addr += bpp * 4;

        __SETDOT_DDRAW_320p_DUP_16(addr, bpp, pitch, c[7]);
        __SETDOT_DDRAW_320p_DUP_16(addr + pitch, bpp, pitch, c[7]);
        // addr += bpp * 4;

}


#endif /* SDL_DRAW_320_H_ */

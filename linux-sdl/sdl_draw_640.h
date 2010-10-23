/*
 * sdl_draw_640.h
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
 *				 20100913 sdl_drawcommon.h から640ドット描画関連分離
 */



#ifndef SDL_DRAW_640_H_
#define SDL_DRAW_640_H_

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
 * 640x200ドット画面の描画(非FullScan)
 * 直接SDLの画面を叩くときに使う
 */
static inline void
__SETDOT_DDRAW_640i(Uint8 * addr, DWORD c)
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



/*
 * 640x200ドット画面の描画(非FullScan)
 * 直接SDLの画面を叩くときに使う
 * 8bpp
 */
static inline void
__SETDOT_DDRAW_640i_8(Uint8 * addr, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[0] = c & 0xff;		/* B */
#else
    addr[0] = c & 0xff;		/* B */
#endif

}

/*
 * 640x200ドット画面の描画(非FullScan)
 * 直接SDLの画面を叩くときに使う
 * 15/16bpp
 */
static inline void
__SETDOT_DDRAW_640i_15(Uint8 * addr, int pitch, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;
#else
    addr[1] = c & 0xff;		/* B */
    addr[0] = (c >> 8) & 0xff;
#endif

}


/*
 * 640x200モード、プログレッシブ点打ち。(DIRECT DRAW)
 * ビット配列は[BRG]である
 */
static inline void
__SETDOT_DDRAW_640p(Uint8 * addr, int pitch, DWORD c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += pitch;
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */

#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */
    addr += pitch;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[2] = (c >> 16) & 0xff;	/* G */

#endif

}

/*
 * 640x200モード、プログレッシブ点打ち。(DIRECT DRAW)
 * ビット配列は[BRG]である 16/15bpp
 */
static inline void
__SETDOT_DDRAW_640p_15(Uint8 * addr, int pitch, DWORD c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[1] = c & 0xff;		/* B */
    addr[0] = (c >> 8) & 0xff;	/* R */
    addr += pitch;
    addr[1] = c & 0xff;		/* B */
    addr[0] = (c >> 8) & 0xff;	/* R */
#else
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr += pitch;
    addr[0] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
#endif
}

/*
 * 640x200モード、プログレッシブ点打ち。(DIRECT DRAW)
 * ビット配列は[BRG]である
 */
static inline void
__SETDOT_DDRAW_640p_8(Uint8 * addr, int pitch, DWORD c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[0] = c  & 0xff;	/* G */
    addr += pitch;
    addr[0] = c  & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr += pitch;
    addr[0] = c & 0xff;		/* B */
#endif
}


/*
 * 640x200モード、横二倍拡大プログレッシブ点打ち。(DIRECT DRAW)
 * ビット配列は[BRG]である
 */
static inline void
__SETDOT_DDRAW_640p_DBL(Uint8 * addr, int bpp, int pitch, DWORD c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += bpp;
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr += (pitch - bpp);
    addr[2] = c & 0xff;		/* B */
    addr[1] = (c >> 8) & 0xff;	/* R */
    addr[0] = (c >> 16) & 0xff;	/* G */
    addr +=  bpp;
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
    addr += (pitch - bpp);
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
 * 640x200モード、横二倍拡大プログレッシブ点打ち。(DIRECT DRAW)
 * ビット配列は[BRG]である
 * 8bpp
 */
static inline void
__SETDOT_DDRAW_640p_DBL_8(Uint8 * addr, int bpp, int pitch, DWORD c)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr[0] = c & 0xff;		/* B */
    addr += bpp;
    addr[0] = c & 0xff;	/* G */
    addr += (pitch - bpp);
    addr[0] = c & 0xff;	/* G */
    addr += bpp;
    addr[0] = c & 0xff;	/* G */
#else
    addr[0] = c & 0xff;		/* B */
    addr += bpp;
    addr[0] = c & 0xff;		/* B */
    addr += (pitch - bpp);
    addr[0] = c & 0xff;		/* B */
    addr += bpp;
    addr[0] = c & 0xff;		/* B */
#endif
}

/*
 * 640x200ドット画面のバイト単位描画(FullScan)
 * 直接SDLの画面を叩くときに使う 8bpp
 */

static inline void
__SETBYTE_DDRAW_640p_8(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
     __SETDOT_DDRAW_640p_8(addr, pitch, c[7]);
    addr += bpp;
    __SETDOT_DDRAW_640p_8(addr, pitch, c[6]);
    addr += bpp;
    __SETDOT_DDRAW_640p_8(addr, pitch, c[5]);
    addr += bpp;
    __SETDOT_DDRAW_640p_8(addr, pitch, c[4]);
    addr += bpp;
    __SETDOT_DDRAW_640p_8(addr, pitch, c[3]);
    addr += bpp;
    __SETDOT_DDRAW_640p_8(addr, pitch, c[2]);
    addr += bpp;
    __SETDOT_DDRAW_640p_8(addr, pitch, c[1]);
    addr += bpp;
    __SETDOT_DDRAW_640p_8(addr, pitch, c[0]);
    // addr += bpp;
}

static inline void
__SETBYTE_DDRAW_640p(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
     __SETDOT_DDRAW_640p(addr, pitch, c[7]);
    addr += bpp;
    __SETDOT_DDRAW_640p(addr, pitch, c[6]);
    addr += bpp;
    __SETDOT_DDRAW_640p(addr, pitch, c[5]);
    addr += bpp;
    __SETDOT_DDRAW_640p(addr, pitch, c[4]);
    addr += bpp;
    __SETDOT_DDRAW_640p(addr, pitch, c[3]);
    addr += bpp;
    __SETDOT_DDRAW_640p(addr, pitch, c[2]);
    addr += bpp;
    __SETDOT_DDRAW_640p(addr, pitch, c[1]);
    addr += bpp;
    __SETDOT_DDRAW_640p(addr, pitch, c[0]);
    // addr += bpp;
}




/*
 * 640x200ドット画面のバイト単位描画(非FullScan)
 * 直接SDLの画面を叩くときに使う
 */

static inline void
__SETBYTE_DDRAW_640i(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
    __SETDOT_DDRAW_640i(addr, c[7]);
    addr += bpp;
    __SETDOT_DDRAW_640i(addr, c[6]);
    addr += bpp;
    __SETDOT_DDRAW_640i(addr, c[5]);
    addr += bpp;
    __SETDOT_DDRAW_640i(addr, c[4]);
    addr += bpp;
    __SETDOT_DDRAW_640i(addr, c[3]);
    addr += bpp;
    __SETDOT_DDRAW_640i(addr, c[2]);
    addr += bpp;
    __SETDOT_DDRAW_640i(addr, c[1]);
    addr += bpp;
    __SETDOT_DDRAW_640i(addr, c[0]);
}

static inline void
__SETBYTE_DDRAW_640i_8(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
    __SETDOT_DDRAW_640i_8(addr, c[7]);
    addr += bpp;
    __SETDOT_DDRAW_640i_8(addr, c[6]);
    addr += bpp;
    __SETDOT_DDRAW_640i_8(addr, c[5]);
    addr += bpp;
    __SETDOT_DDRAW_640i_8(addr, c[4]);
    addr += bpp;
    __SETDOT_DDRAW_640i_8(addr, c[3]);
    addr += bpp;
    __SETDOT_DDRAW_640i_8(addr, c[2]);
    addr += bpp;
    __SETDOT_DDRAW_640i_8(addr, c[1]);
    addr += bpp;
    __SETDOT_DDRAW_640i_8(addr, c[0]);
}

/*
 * 640x200ドット画面のバイト単位描画(倍角FullScan)
 * 直接SDLの画面を叩くときに使う
 */

static inline void
__SETBYTE_DDRAW_1280_640p(Uint8 * addr, int bpp, int pitch, DWORD * c)
{

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[7]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[7]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[6]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[6]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[5]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[5]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[4]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[4]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[3]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[3]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[2]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[2]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[1]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[1]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[0]);
    __SETDOT_DDRAW_640p_DBL(addr + pitch * 2, bpp, pitch, c[0]);
    // addr += bpp * 2;


}

/*
 * 640x200ドット画面のバイト単位描画(倍角FullScan)
 * 直接SDLの画面を叩くときに使う
 * 8bpp
 */

static inline void
__SETBYTE_DDRAW_1280_640p_8(Uint8 * addr, int bpp, int pitch, DWORD * c)
{

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[7]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[7]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[6]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[6]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[5]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[5]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[4]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[4]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[3]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[3]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[2]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[2]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[1]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[1]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[0]);
    __SETDOT_DDRAW_640p_DBL_8(addr + pitch * 2, bpp, pitch, c[0]);
    // addr += bpp * 2;


}

/*
 * 640x200ドット画面のバイト単位描画(倍角、非FullScan)
 * 直接SDLの画面を叩くときに使う
 */

static inline void
__SETBYTE_DDRAW_1280_640i(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[7]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[6]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[5]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[4]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[3]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[2]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[1]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL(addr, bpp, pitch, c[0]);
    addr += bpp * 2;

}


static inline void
__SETBYTE_DDRAW_1280_640i_8(Uint8 * addr, int bpp, int pitch, DWORD * c)
{
    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[7]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[6]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[5]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[4]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[3]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[2]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[1]);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p_DBL_8(addr, bpp, pitch, c[0]);
    addr += bpp * 2;
}


#endif /* SDL_DRAW_640_H_ */

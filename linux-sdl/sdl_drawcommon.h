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
 */

#include <SDL/SDL.h>
#include "xm7.h"
#include "sdl_draw.h"
#include "sdl.h"

/*
 *      グローバル ワーク(のextern設定)
 */
extern DWORD    rgbTTLGDI[16];	/* デジタルパレット */
extern DWORD    rgbAnalogGDI[4096];	/* アナログパレット */
// guchar pBitsGDI[400*640*3]; /* ビットデータ */
extern BYTE     GDIDrawFlag[4000];	/* 8x8 再描画領域フラグ */
extern BOOL     bFullScan;	/* フルスキャン(Window) */
extern BOOL     bDirectDraw;	/* 直接書き込みフラグ */
extern SDL_Surface *realDrawArea;	/* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する) 
					 */
extern WORD     nDrawTop;	/* 描画範囲上 */
extern WORD     nDrawBottom;	/* 描画範囲下 */
extern WORD     nDrawLeft;	/* 描画範囲左 */
extern WORD     nDrawRight;	/* 描画範囲右 */
extern BOOL     bPaletFlag;	/* パレット変更フラグ */
extern BOOL     bClearFlag;	/* クリアフラグ */


/*
 * 共通ルーチンのextern設定
 */
extern BOOL     BitBlt(int nDestLeft, int nDestTop, int nWidth,
		       int nHeight, int nSrcLeft, int nSrcTop);
extern void     RenderFullScan(void);
extern void     RenderSetOddLine(void);
extern void     SetDrawFlag(BOOL flag);
extern void     AllClear(void);


static inline void
__SETDOT_640i(Uint8 * addr, DWORD c)
{

    DWORD          *addr32 = (DWORD *) addr;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    *addr32 = (c >> 8) | 0xff000000;
#else
    *addr32 = (c << 8) | 0x000000ff;

#endif

}

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
 * 640x200モード、プログレッシブ点打ち。
 * ビット配列は[ABRG]である
 * 20100124 32bit前提になった為、Uint32 (DWORD)で一気に書き込むようにする。
 */
static inline void
__SETDOT_640p(Uint8 * addr, int pitch, DWORD c)
{

    DWORD          *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;

    addr += pitch;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
#else
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;

    addr += pitch;
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;
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
    addr += pitch;
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
 * 640x200ドット画面のバイト単位描画(FullScan)
 * 直接SDLの画面を叩くときに使う
 */

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
    // addr += bpp;
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

/*
 * 640x400モード、一バイト一ライン一気に書き込む(8色)
 */
static inline void
 __SETBYTE_1LINE_DDRAW_640_640(Uint8 *addr,int bpp, int pitch, DWORD *c)
{
        Uint8 *a = addr;
        __SETDOT_DDRAW_640i(addr, c[7]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[6]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[5]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[4]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[3]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[2]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[1]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[0]);
        //a += bpp;
}

/*
 * 1280x800モード、一バイト一ライン一気に書き込む(8色)
 */
static inline void __SETBYTE_1LINE_DDRAW_640_1280(Uint8 *addr,int bpp, int pitch, DWORD *c)
{
        Uint8 *a = addr;
        __SETDOT_DDRAW_640i(addr, c[7]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[7]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[6]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[6]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[5]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[5]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[4]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[4]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[3]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[3]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[2]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[2]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[1]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[1]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[0]);
        a += bpp;
        __SETDOT_DDRAW_640i(addr, c[0]);
        //a += bpp;
}

/*
 * 320x200モードでのドット打ち(横二ドット一気に打つ)
 */
static inline void
__SETDOT_320i(WORD x, WORD y, DWORD c)
{
    Uint8          *addr =
	    (Uint8 *) realDrawArea->pixels + y * realDrawArea->pitch +
	    x * 2 * realDrawArea->format->BytesPerPixel;
    DWORD          *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;

    addr += realDrawArea->format->BytesPerPixel;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
#else
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;
    addr += realDrawArea->format->BytesPerPixel;
    /*
     * 横拡大 
     */
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;

#endif
}

/*
 * 320x200,非フルスキャン、原寸 
 * (DDRAW)
 */
static inline void
__SETDOT_DDRAW_320i(Uint8 * addr, int bpp, int pitch, DWORD c)
{
    DWORD          *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;

    addr += bpp;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
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
 * 320x200,フルスキャン、原寸 
 * 
 */
static inline void
__SETDOT_320p(WORD x, WORD y, DWORD c)
{
    Uint8          *addr =
	(Uint8 *) realDrawArea->pixels + y * realDrawArea->pitch +
	x * 2 * realDrawArea->format->BytesPerPixel;
    DWORD          *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;

    addr += realDrawArea->format->BytesPerPixel;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
    addr += realDrawArea->pitch;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
    addr -= realDrawArea->format->BytesPerPixel;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
#else
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;
    /*
     * 横拡大 
     */
    // addr += displayArea->format->BytesPerPixel;
    addr += realDrawArea->format->BytesPerPixel;
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;
    /*
     * 縦拡大 
     */
    addr += realDrawArea->pitch;
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;

    addr -= realDrawArea->format->BytesPerPixel;
    addr32 = (DWORD *) addr;
    *addr32 = (c << 8) | 0x000000ff;

#endif
}

/*
 * 320x200,非フルスキャン、原寸 
 * (DDRAW)
 */
static inline void
__SETDOT_DDRAW_320p(Uint8 * addr, int bpp, int pitch, DWORD c)
{
    DWORD          *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;

    addr += bpp;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
    addr += pitch;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
    addr -= bpp;
    addr32 = (DWORD *) addr;
    *addr32 = (c >> 8) | 0xff000000;
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
 * 320x200,フルスキャン、倍角
 * (DDRAW)
 */

static inline void
__SETDOT_DDRAW_320p_DUP(Uint8 * addr, int bpp, int pitch, DWORD c)
{
    DWORD          *addr32;

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
 * 8色モード,200ラインのVRAMデータを取り込んでピクセルデータに変換する
 */
static inline void
__GETVRAM_3bpp(BYTE * vram, int x, int y, DWORD * c)
{
        BYTE    cb,
                cr,
                cg;
        int     offset;

/*
 * オフセット設定 
     */
        offset = 80 * y + x;
#if XM7_VER >= 3
        cb = vram[offset + 0x00000];
        cr = vram[offset + 0x08000];
        cg = vram[offset + 0x10000];
#else
        cb = vram[offset + 0x00000];
        cr = vram[offset + 0x04000];
        cg = vram[offset + 0x08000];
#endif				/* XM7_VER */

        c[0] =   rgbTTLGDI[(cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2)];
        c[1] =   rgbTTLGDI[((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1)];
        c[2] =   rgbTTLGDI[((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04)];
        c[3] =   rgbTTLGDI[((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +((cg & 0x08) >> 1)];
        c[4] =   rgbTTLGDI[((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) + ((cg & 0x10) >> 2)];
        c[5] =   rgbTTLGDI[((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) + ((cg & 0x20) >> 3)];
        c[6] =   rgbTTLGDI[((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) + ((cg & 0x40) >> 4)];
        c[7] =   rgbTTLGDI[((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) + ((cg & 0x80) >> 5)];
}

/*
 * 4096色モード,200ラインのVRAMデータを取り込んでピクセルデータに変換する
 */
static inline void
__GETVRAM_12bpp(BYTE * vramptr, int x, int y, DWORD * c)
{
    BYTE            b[4],
                    r[4],
                    g[4];
    int             offset;
    DWORD           dat[8];

    /*
     * オフセット設定 
     */
    offset = 40 * y + x;
    /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う 
     */

    g[3] = vramptr[offset + 0x10000];
    g[2] = vramptr[offset + 0x12000];
    g[1] = vramptr[offset + 0x14000];
    g[0] = vramptr[offset + 0x16000];

    r[3] = vramptr[offset + 0x08000];
    r[2] = vramptr[offset + 0x0a000];
    r[1] = vramptr[offset + 0x0c000];
    r[0] = vramptr[offset + 0x0e000];

    b[3] = vramptr[offset + 0x00000];
    b[2] = vramptr[offset + 0x02000];
    b[1] = vramptr[offset + 0x04000];
    b[0] = vramptr[offset + 0x06000];


    /*
     * bit7 
     */
    dat[7] =
	((b[0] & 0x01)) + ((b[1] & 0x01) << 1) + ((b[2] & 0x01) << 2) +
	((b[3] & 0x01) << 3)
	+ ((r[0] & 0x01) << 4) + ((r[1] & 0x01) << 5) +
	((r[2] & 0x01) << 6) + ((r[3] & 0x01) << 7)
	+ ((g[0] & 0x01) << 8) + ((g[1] & 0x01) << 9) +
	((g[2] & 0x01) << 10) + ((g[3] & 0x01) << 11);
    c[7] = rgbAnalogGDI[dat[7]];

    /*
     * bit6 
     */
    dat[6] =
	((b[0] & 0x02) >> 1) + ((b[1] & 0x02)) + ((b[2] & 0x02) << 1) +
	((b[3] & 0x02) << 2)
	+ ((r[0] & 0x02) << 3) + ((r[1] & 0x02) << 4) +
	((r[2] & 0x02) << 5) + ((r[3] & 0x02) << 6)
	+ ((g[0] & 0x02) << 7) + ((g[1] & 0x02) << 8) +
	((g[2] & 0x02) << 9) + ((g[3] & 0x02) << 10);
    c[6] = rgbAnalogGDI[dat[6]];

    /*
     * bit5 
     */
    dat[5] =
	((b[0] & 0x04) >> 2) + ((b[1] & 0x04) >> 1) + ((b[2] & 0x04)) +
	((b[3] & 0x04) << 1)
	+ ((r[0] & 0x04) << 2) + ((r[1] & 0x04) << 3) +
	((r[2] & 0x04) << 4) + ((r[3] & 0x04) << 5)
	+ ((g[0] & 0x04) << 6) + ((g[1] & 0x04) << 7) +
	((g[2] & 0x04) << 8) + ((g[3] & 0x04) << 9);
    c[5] = rgbAnalogGDI[dat[5]];

    /*
     * bit4 
     */
    dat[4] =
	((b[0] & 0x08) >> 3) + ((b[1] & 0x08) >> 2) +
	((b[2] & 0x08) >> 1) + ((b[3] & 0x08))
	+ ((r[0] & 0x08) << 1) + ((r[1] & 0x08) << 2) +
	((r[2] & 0x08) << 3) + ((r[3] & 0x08) << 4)
	+ ((g[0] & 0x08) << 5) + ((g[1] & 0x08) << 6) +
	((g[2] & 0x08) << 7) + ((g[3] & 0x08) << 8);
    c[4] = rgbAnalogGDI[dat[4]];

    /*
     * bit3 
     */
    dat[3] =
	((b[0] & 0x10) >> 4) + ((b[1] & 0x10) >> 3) +
	((b[2] & 0x10) >> 2) + ((b[3] & 0x10) >> 1)
	+ ((r[0] & 0x10)) + ((r[1] & 0x10) << 1) + ((r[2] & 0x10) << 2) +
	((r[3] & 0x10) << 3)
	+ ((g[0] & 0x10) << 4) + ((g[1] & 0x10) << 5) +
	((g[2] & 0x10) << 6) + ((g[3] & 0x10) << 7);
    c[3] = rgbAnalogGDI[dat[3]];

    /*
     * bit2 
     */
    dat[2] =
	((b[0] & 0x20) >> 5) + ((b[1] & 0x20) >> 4) +
	((b[2] & 0x20) >> 3) + ((b[3] & 0x20) >> 2)
	+ ((r[0] & 0x20) >> 1) + ((r[1] & 0x20)) + ((r[2] & 0x20) << 1) +
	((r[3] & 0x20) << 2)
	+ ((g[0] & 0x20) << 3) + ((g[1] & 0x20) << 4) +
	((g[2] & 0x20) << 5) + ((g[3] & 0x20) << 6);
    c[2] = rgbAnalogGDI[dat[2]];

    /*
     * bit1 
     */
    dat[1] =
	((b[0] & 0x40) >> 6) + ((b[1] & 0x40) >> 5) +
	((b[2] & 0x40) >> 4) + ((b[3] & 0x40) >> 3)
	+ ((r[0] & 0x40) >> 2) + ((r[1] & 0x40) >> 1) + ((r[2] & 0x40)) +
	((r[3] & 0x40) << 1)
	+ ((g[0] & 0x40) << 2) + ((g[1] & 0x40) << 3) +
	((g[2] & 0x40) << 4) + ((g[3] & 0x40) << 5);
    c[1] = rgbAnalogGDI[dat[1]];

    /*
     * bit0 
     */
    dat[0] =
	((b[0] & 0x80) >> 7) + ((b[1] & 0x80) >> 6) +
	((b[2] & 0x80) >> 5) + ((b[3] & 0x80) >> 4)
	+ ((r[0] & 0x80) >> 3) + ((r[1] & 0x80) >> 2) +
	((r[2] & 0x80) >> 1) + ((r[3] & 0x80))
	+ ((g[0] & 0x80) << 1) + ((g[1] & 0x80) << 2) +
	((g[2] & 0x80) << 3) + ((g[3] & 0x80) << 4);
    c[0] = rgbAnalogGDI[dat[0]];

}

/*
 * 26万色モード,200ラインのVRAMデータを取り込んでピクセルデータに変換する
 */
static inline void
__GETVRAM_18bpp(BYTE * vramptr, int x, int y, DWORD * c)
{
    BYTE            b[6],
                    r[6],
                    g[6];
    int             offset;

    /*
     * オフセット設定 
     */
    offset = 40 * y + x;
    /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う 
     */

    g[5] = vramptr[offset + 0x10000];
    g[4] = vramptr[offset + 0x12000];
    g[3] = vramptr[offset + 0x14000];
    g[2] = vramptr[offset + 0x16000];
    g[1] = vramptr[offset + 0x28000];
    g[0] = vramptr[offset + 0x2a000];

    r[5] = vramptr[offset + 0x08000];
    r[4] = vramptr[offset + 0x0a000];
    r[3] = vramptr[offset + 0x0c000];
    r[2] = vramptr[offset + 0x0e000];
    r[1] = vramptr[offset + 0x20000];
    r[0] = vramptr[offset + 0x22000];

    b[5] = vramptr[offset + 0x00000];
    b[4] = vramptr[offset + 0x02000];
    b[3] = vramptr[offset + 0x04000];
    b[2] = vramptr[offset + 0x06000];
    b[1] = vramptr[offset + 0x18000];
    b[0] = vramptr[offset + 0x1a000];


    /*
     * bit7 
     */
    c[7] =
	    ((b[0] & 0x01)) + ((b[1] & 0x01) << 1) + ((b[2] & 0x01) << 2) +
	    ((b[3] & 0x01) << 3) + ((b[4] & 0x01) << 4) + ((b[5] & 0x01) << 5) +	
	    ((r[0] & 0x01) << 6) + ((r[1] & 0x01) << 7) + ((r[2] & 0x01) << 8) +
	    ((r[3] & 0x01) << 9) + ((r[4] & 0x01) << 10) + ((r[5] & 0x01) << 11) +
	    ((g[0] & 0x01) << 12) + ((g[1] & 0x01) << 13) + ((g[2] & 0x01) << 14) +
	    ((g[3] & 0x01) << 15) + ((g[4] & 0x01) << 16) + ((g[5] & 0x01) << 17);
    /*
     * bit6 
     */
    c[6] =
	    ((b[0] & 0x02) >> 1) + ((b[1] & 0x02)) + ((b[2] & 0x02) << 1) +
	    ((b[3] & 0x02) << 2) + ((b[4] & 0x02) << 3) + ((b[5] & 0x02) << 4) +
	    ((r[0] & 0x02) << 5) + ((r[1] & 0x02) << 6) + ((r[2] & 0x02) << 7) +
	    ((r[3] & 0x02) << 8) + ((r[4] & 0x02) << 9) + ((r[5] & 0x02) << 10) +
	    ((g[0] & 0x02) << 11) + ((g[1] & 0x02) << 12) + ((g[2] & 0x02) << 13) +
	    ((g[3] & 0x02) << 14) + ((g[4] & 0x02) << 15) + ((g[5] & 0x02) << 16);
    /*
     * bit5 
     */
    c[5] =
	    ((b[0] & 0x04) >> 2) + ((b[1] & 0x04) >> 1) + ((b[2] & 0x04)) +
	    ((b[3] & 0x04) << 1) + ((b[4] & 0x04) << 2) + ((b[5] & 0x04) << 3) +
	    ((r[0] & 0x04) << 4) + ((r[1] & 0x04) << 5) + ((r[2] & 0x04) << 6) + 
	    ((r[3] & 0x04) << 7) + ((r[4] & 0x04) << 8) + ((r[5] & 0x04) << 9) +
	    ((g[0] & 0x04) << 10) + ((g[1] & 0x04) << 11) + ((g[2] & 0x04) << 12) + 
	    ((g[3] & 0x04) << 13) + ((g[4] & 0x04) << 14) + ((g[5] & 0x04) << 15);

    /*
     * bit4 
     */
    c[4] =
	    ((b[0] & 0x08) >> 3) + ((b[1] & 0x08) >> 2) + ((b[2] & 0x08) >> 1) + 
	    ((b[3] & 0x08)) + ((b[4] & 0x08) << 1) + ((b[5] & 0x08) << 2) +
	    ((r[0] & 0x08) << 3) + ((r[1] & 0x08) << 4) + ((r[2] & 0x08) << 5) +
	    ((r[3] & 0x08) << 6) + ((r[4] & 0x08) << 7) + ((r[5] & 0x08) << 8) +
	    ((g[0] & 0x08) << 9) + ((g[1] & 0x08) << 10) + ((g[2] & 0x08) << 11) + 
	    ((g[3] & 0x08) << 12) + ((g[4] & 0x08) << 13) + ((g[5] & 0x08) << 14);

    /*
     * bit3 
     */
    c[3] =
	    ((b[0] & 0x10) >> 4) + ((b[1] & 0x10) >> 3) +((b[2] & 0x10) >> 2) + 
	    ((b[3] & 0x10) >> 1) + ((b[4] & 0x10)) + ((b[5] & 0x10) << 1) +
	    ((r[0] & 0x10) << 2) + ((r[1] & 0x10) << 3) + ((r[2] & 0x10) << 4) +
	    ((r[3] & 0x10) << 5) + ((r[4] & 0x10) << 6) + ((r[5] & 0x10) << 7) +
	    ((g[0] & 0x10) << 8) + ((g[1] & 0x10) << 9) + ((g[2] & 0x10) << 10) + 
	    ((g[3] & 0x10) << 11) + ((g[4] & 0x10) << 12) + ((g[5] & 0x10) << 13);

    /*
     * bit2 
     */
    c[2] =
	    ((b[0] & 0x20) >> 5) + ((b[1] & 0x20) >> 4) + ((b[2] & 0x20) >> 3) +
	    ((b[3] & 0x20) >> 2) + ((b[4] & 0x20) >> 1) + ((b[5] & 0x20)) +
	    ((r[0] & 0x20) << 1) + ((r[1] & 0x20) << 2) + ((r[2] & 0x20) << 3) +
	    ((r[3] & 0x20) << 4) + ((r[4] & 0x20) << 5) + ((r[5] & 0x20) << 6) + 
	    ((g[0] & 0x20) << 7) + ((g[1] & 0x20) << 8) + ((g[2] & 0x20) << 9) + 
	    ((g[3] & 0x20) << 10) + ((g[4] & 0x20) << 11) + ((g[5] & 0x20) << 12);
    

    /*
     * bit1 
     */
    c[1] =
	    ((b[0] & 0x40) >> 6) + ((b[1] & 0x40) >> 5) + ((b[2] & 0x40) >> 4) +
	    ((b[3] & 0x40) >> 3) + ((b[4] & 0x40) >> 2) + ((b[5] & 0x40) >> 1) +
	    ((r[0] & 0x40))      + ((r[1] & 0x40) << 1) + ((r[2] & 0x40) << 2) +
	    ((r[3] & 0x40) << 3) + ((r[4] & 0x40) << 4) + ((r[5] & 0x40) << 5) +
	    ((g[0] & 0x40) << 6) + ((g[1] & 0x40) << 7) + ((g[2] & 0x40) << 8) + 
	    ((g[3] & 0x40) << 9) + ((g[4] & 0x40) << 10)+ ((g[5] & 0x40) << 11);
  

    /*
     * bit0 
     */
    c[0] =
	    ((b[0] & 0x80) >> 7) + ((b[1] & 0x80) >> 6) + ((b[2] & 0x80) >> 5) + 
	    ((b[3] & 0x80) >> 4) + ((b[4] & 0x80) >> 3) + ((b[5] & 0x80) >> 2) +
	    ((r[0] & 0x80) >> 1) + ((r[1] & 0x80)) + ((r[2] & 0x80) << 1) + 
	    ((r[3] & 0x80) << 2) + ((r[4] & 0x80) << 3) + ((r[5] & 0x80) << 4) +
	    ((g[0] & 0x80) << 5) + ((g[1] & 0x80) << 6) + ((g[2] & 0x80) << 7) + 
	    ((g[3] & 0x80) << 8) + ((g[4] & 0x80) << 9) + ((g[5] & 0x80) << 10);
    
}



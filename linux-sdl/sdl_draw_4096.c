/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *	[ SDL 表示 / 4096色パート]
 *               20100125 sdl_draw.cを分割
 */

#ifdef _XWIN

#include <SDL/SDL.h>
#include "xm7.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"
#include "sdl.h"
#include "sdl_draw.h"

/*
 *	グローバル ワーク(のextern設定)
 */
extern DWORD rgbTTLGDI[16];						/* デジタルパレット */
extern DWORD rgbAnalogGDI[4096];					/* アナログパレット */
//guchar pBitsGDI[400*640*3];					/* ビットデータ */
extern BYTE GDIDrawFlag[4000];						/* 8x8 再描画領域フラグ */
extern BOOL bFullScan;								/* フルスキャン(Window) */
extern BOOL bDirectDraw; /* 直接書き込みフラグ */
extern SDL_Surface *realDrawArea; /* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する) */
extern WORD nDrawTop;						/* 描画範囲上 */
extern WORD nDrawBottom;					/* 描画範囲下 */
extern WORD nDrawLeft;						/* 描画範囲左 */
extern WORD nDrawRight;						/* 描画範囲右 */
extern BOOL bPaletFlag;						/* パレット変更フラグ */
extern BOOL bClearFlag;						/* クリアフラグ */



/*
 * 共通ルーチンのextern設定
 */
extern BOOL BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight, int nSrcLeft, int nSrcTop);
extern void RenderFullScan(void);
extern void RenderSetOddLine(void);
extern void SetDrawFlag(BOOL flag);
extern void AllClear(void);

/*
 * 320x200モードでのドット打ち(横二ドット一気に打つ)
 */
static inline void __SETDOT_320i(WORD x, WORD y, DWORD c)
{
        Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * realDrawArea->pitch + x * 2 * realDrawArea->format->BytesPerPixel;
        DWORD *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;

                    addr += realDrawArea->format->BytesPerPixel;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
#else
                    addr32 = (DWORD *)addr;
                    *addr32 = (c << 8) | 0x000000ff;
                    addr += realDrawArea->format->BytesPerPixel;
	   /* 横拡大 */
                    addr32 = (DWORD *)addr;
                    *addr32 = (c << 8) | 0x000000ff;

#endif	   
}
/*
 * 320x200 (DDRAW)
 */
static inline void __SETDOT_DDRAW_320i(Uint8 *addr, int bpp, int pitch,  DWORD c)
{
        DWORD *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;

                    addr += bpp;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
#else
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
                    addr += bpp;
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
#endif	   
}
static inline void __SETDOT_320p(WORD x, WORD y, DWORD c)
{
        Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * realDrawArea->pitch + x * 2 * realDrawArea->format->BytesPerPixel;
        DWORD *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN 
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;

                    addr += realDrawArea->format->BytesPerPixel;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
                    addr += realDrawArea->pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
                    addr -= realDrawArea->format->BytesPerPixel;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
#else
                    addr32 = (DWORD *)addr;
                    *addr32 = (c << 8) | 0x000000ff;
	   /* 横拡大 */
                    //addr += displayArea->format->BytesPerPixel;
                    addr += realDrawArea->format->BytesPerPixel;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c << 8) | 0x000000ff;
                    /* 縦拡大 */
                    addr += realDrawArea->pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c << 8) | 0x000000ff;

                    addr -= realDrawArea->format->BytesPerPixel;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c << 8) | 0x000000ff;
           
#endif	   
}

static inline void __SETDOT_DDRAW_320p(Uint8 *addr, int bpp, int pitch, DWORD c)
{
        DWORD *addr32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN 
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;

                    addr += bpp;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
                    addr += pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
                    addr -= bpp;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
#else
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
                    addr += bpp;
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
                    addr += pitch;

                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
                    addr -= bpp;
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
#endif	   
}

static void __SETBYTE_DDRAW_320i(Uint8 *addr, int bpp, int pitch, DWORD *c)
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
    //addr += bpp * 2;
}


static void __SETBYTE_DDRAW_320p(Uint8 *addr, int bpp, int pitch, DWORD *c)
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
    //addr += bpp * 2;
}

static void __SETBYTE_DDRAW_1280_320i(Uint8 *addr, int bpp, int pitch, DWORD *c)
{
    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[0]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[0]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[1]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[1]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[2]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[2]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[3]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[3]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[4]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[4]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[5]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[5]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[6]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[6]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320i(addr, bpp, pitch, c[7]);
    __SETDOT_DDRAW_320i(addr + bpp * 2, bpp, pitch, c[7]);
    //addr += bpp * 2;
}

static void __SETBYTE_DDRAW_1280_320p(Uint8 *addr, int bpp, int pitch, DWORD *c)
{
    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[0]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[0]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[0]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[0]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[1]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[1]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[1]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[1]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[2]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[2]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[2]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[2]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[3]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[3]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[3]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[3]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[4]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[4]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[4]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[4]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[5]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[5]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[5]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[5]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[6]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[6]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[6]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[6]);
    addr += bpp * 4;

    __SETDOT_DDRAW_320p(addr, bpp, pitch, c[7]);
    __SETDOT_DDRAW_320p(addr + bpp * 2, bpp, pitch, c[7]);
    __SETDOT_DDRAW_320p(addr + pitch , bpp, pitch, c[7]);
    __SETDOT_DDRAW_320p(addr + bpp * 2 + pitch, bpp, pitch, c[7]);
    //addr += bpp * 4;

}


/*
 * 4096色モード,200ラインのVRAMデータを取り込んでピクセルデータに変換する
 */
static void __GETVRAM_12bpp(BYTE *vramptr, int x, int y, DWORD *c)
{
  BYTE b[4], r[4], g[4];
  int offset;
  DWORD dat[8];
 
  /* オフセット設定 */
  offset = 40 * y + x;
                        /* R,G,Bについて8bit単位で描画する。
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


                          /* bit7 */
                          dat[7] = ((b[0] & 0x01)) + ((b[1] & 0x01)<<1) + ((b[2] & 0x01)<<2) + ((b[3] & 0x01)<<3) 
                              + ((r[0] & 0x01)<<4) + ((r[1] & 0x01)<<5) + ((r[2] & 0x01)<<6) + ((r[3] & 0x01)<<7)
                              + ((g[0] & 0x01)<<8) + ((g[1] & 0x01)<<9) + ((g[2] & 0x01)<<10) + ((g[3] & 0x01)<<11);
                          c[7] = rgbAnalogGDI[dat[7]];
                          
                          /* bit6 */
                          dat[6] = ((b[0] & 0x02)>>1) + ((b[1] & 0x02)) + ((b[2] & 0x02)<<1) + ((b[3] & 0x02)<<2) 
                              + ((r[0] & 0x02)<<3) + ((r[1] & 0x02)<<4) + ((r[2] & 0x02)<<5) + ((r[3] & 0x02)<<6)
                              + ((g[0] & 0x02)<<7) + ((g[1] & 0x02)<<8) + ((g[2] & 0x02)<<9) + ((g[3] & 0x02)<<10);
                          c[6] = rgbAnalogGDI[dat[6]];

                          /* bit5 */
                          dat[5] = ((b[0] & 0x04)>>2) + ((b[1] & 0x04)>>1) + ((b[2] & 0x04)) + ((b[3] & 0x04)<<1) 
                              + ((r[0] & 0x04)<<2) + ((r[1] & 0x04)<<3) + ((r[2] & 0x04)<<4) + ((r[3] & 0x04)<<5)
                              + ((g[0] & 0x04)<<6) + ((g[1] & 0x04)<<7) + ((g[2] & 0x04)<<8) + ((g[3] & 0x04)<<9);
                          c[5] = rgbAnalogGDI[dat[5]];

                          /* bit4 */
                          dat[4] = ((b[0] & 0x08)>>3) + ((b[1] & 0x08)>>2) + ((b[2] & 0x08)>>1) + ((b[3] & 0x08)) 
                              + ((r[0] & 0x08)<<1) + ((r[1] & 0x08)<<2) + ((r[2] & 0x08)<<3) + ((r[3] & 0x08)<<4)
                              + ((g[0] & 0x08)<<5) + ((g[1] & 0x08)<<6) + ((g[2] & 0x08)<<7) + ((g[3] & 0x08)<<8);
                          c[4] = rgbAnalogGDI[dat[4]];

                          /* bit3 */
                          dat[3] = ((b[0] & 0x10)>>4) + ((b[1] & 0x10)>>3) + ((b[2] & 0x10)>>2) + ((b[3] & 0x10)>>1  ) 
                              + ((r[0] & 0x10)) + ((r[1] & 0x10)<<1) + ((r[2] & 0x10)<<2) + ((r[3] & 0x10)<<3)
                              + ((g[0] & 0x10)<<4) + ((g[1] & 0x10)<<5) + ((g[2] & 0x10)<<6) + ((g[3] & 0x10)<<7);
                          c[3] = rgbAnalogGDI[dat[3]];

                          /* bit2 */
                          dat[2] = ((b[0] & 0x20)>>5) + ((b[1] & 0x20)>>4) + ((b[2] & 0x20)>>3) + ((b[3] & 0x20)>>2) 
                              + ((r[0] & 0x20)>>1) + ((r[1] & 0x20)) + ((r[2] & 0x20)<<1) + ((r[3] & 0x20)<<2)
                              + ((g[0] & 0x20)<<3) + ((g[1] & 0x20)<<4) + ((g[2] & 0x20)<<5) + ((g[3] & 0x20)<<6);
                          c[2] = rgbAnalogGDI[dat[2]];

                          /* bit1 */
                          dat[1] = ((b[0] & 0x40)>>6) + ((b[1] & 0x40)>>5) + ((b[2] & 0x40)>>4) + ((b[3] & 0x40)>>3) 
                              + ((r[0] & 0x40)>>2) + ((r[1] & 0x40)>>1) + ((r[2] & 0x40)) + ((r[3] & 0x40)<<1)
                              + ((g[0] & 0x40)<<2) + ((g[1] & 0x40)<<3) + ((g[2] & 0x40)<<4) + ((g[3] & 0x40)<<5);
                          c[1] = rgbAnalogGDI[dat[1]];

                          /* bit0 */
                          dat[0] = ((b[0] & 0x80)>>7) + ((b[1] & 0x80)>>6) + ((b[2] & 0x80)>>5) + ((b[3] & 0x80)>>4) 
                              + ((r[0] & 0x80)>>3) + ((r[1] & 0x80)>>2) + ((r[2] & 0x80)>>1) + ((r[3] & 0x80))
                              + ((g[0] & 0x80)<<1) + ((g[1] & 0x80)<<2) + ((g[2] & 0x80)<<3) + ((g[3] & 0x80)<<4);
                          c[0] = rgbAnalogGDI[dat[0]];

}


/*
 *	320x200、アナログモード
 *	パレット設定
 */
void Palet320()
{
	int i, j;
	DWORD color;
	DWORD r, g, b;
	int amask;

	/* アナログマスクを作成 */
	amask = 0;
	if (!(multi_page & 0x10)) {
		amask |= 0x000f;
	}
	if (!(multi_page & 0x20)) {
		amask |= 0x00f0;
	}
	if (!(multi_page & 0x40)) {
		amask |= 0x0f00;
	}

	for (i=0; i<4096; i++) {
		/* 最下位から5bitづつB,G,R */
		color = 0;
		if (crt_flag) {
			j = i & amask;
			r = apalet_r[j];
			g = apalet_g[j];
			b = apalet_b[j];
		}
		else {
			r = 0;
			g = 0;
			b = 0;
		}

		/* R */
		r <<= 4;
		if (r > 0) {
			r |= 0x0f;
		}
		color |= r;
		color <<= 8;

		/* G */
		g <<= 4;
		if (g > 0) {
			g |= 0x0f;
		}
		color |= g;
		color <<= 8;

		/* B */
		b <<= 4;
		if (b > 0) {
			b |= 0x0f;
		}
		color |= b;


		/* セット */
		rgbAnalogGDI[i] = color;
	}
}




/*
 *	320x200、アナログモード
 *	ウィンドウ外描画用サブ
 */
static void Draw320Sub(int top, int bottom)
{
       int x, y;
       DWORD c[8];
       Uint8 *addr;

        SDL_LockSurface(realDrawArea);
        /* yループ */
       for (y=top; y<bottom; y++) {
         /* xループ */
          for (x=nDrawLeft>>4; x<nDrawRight>>4; x++) {
            __GETVRAM_12bpp(vram_dptr, x, y, c);
            
            switch(nDrawWidth){
            case 1280:
            addr = (Uint8 *)realDrawArea->pixels + 
              (y<<2) * realDrawArea->pitch + 
              (x<<5) *  realDrawArea->format->BytesPerPixel;
            if(bFullScan) { 
              __SETBYTE_DDRAW_1280_320p(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
            } else {
              __SETBYTE_DDRAW_1280_320i(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
            }
            break;
            case 640:
            default:
            addr = (Uint8 *)realDrawArea->pixels + 
              (y<<1) * realDrawArea->pitch + 
              (x<<4) * realDrawArea->format->BytesPerPixel;
            if(bFullScan) { 
              __SETBYTE_DDRAW_320p(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
            } else {
              __SETBYTE_DDRAW_320i(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
            }
            break;
            }
          }
       }
        SDL_UnlockSurface(realDrawArea);
}

#if XM7_VER >= 3
/*
 *	320x200、アナログモード
 *	ウィンドウ内描画用サブ
 */
static void Draw320WSub(int top, int bottom, int left, int right)
{
  int x, y;
  DWORD c[8];
  BYTE bit;
  BYTE b[4], r[4], g[4];
  Uint8 *addr;
  
  SDL_LockSurface(realDrawArea);
  /* yループ */
  for (y=top; y<bottom; y++) {
    
          /* xループ */
    for (x=left; x<right; x++) {
      __GETVRAM_12bpp(vram_bdptr, x, y, c);
      switch(nDrawWidth){
      case 1280:
        addr = (Uint8 *)realDrawArea->pixels + 
          (y<<2) * realDrawArea->pitch + 
          (x<<5) * realDrawArea->format->BytesPerPixel;
        if(bFullScan) { 
          __SETBYTE_DDRAW_1280_320p(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
        } else {
          __SETBYTE_DDRAW_1280_320i(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
        }
        break;
      case 640:
      default:
        addr = (Uint8 *)realDrawArea->pixels + 
          (y<<1) * realDrawArea->pitch + 
          (x<<4) * realDrawArea->format->BytesPerPixel;
        if(bFullScan) { 
          __SETBYTE_DDRAW_320p(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
        } else {
          __SETBYTE_DDRAW_320i(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
        }
        break;
      }
    }
  }
  SDL_UnlockSurface(realDrawArea);
}

/*
 *	320x200、アナログモード
 *	描画
 */
void Draw320(void)
{
#if XM7_VER >= 3
	WORD wdtop, wdbtm;
#endif

	/* パレット設定 */
	if (bPaletFlag) {
		Palet320();
	}

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if (nDrawTop >= nDrawBottom) {
		return;
	}

#if XM7_VER >= 3
	/* ウィンドウオープン時 */
	if (window_open) {
		/* ウィンドウ外 上側の描画 */
		if ((nDrawTop >> 1) < window_dy1) {
			Draw320Sub(nDrawTop >> 1, window_dy1);
		}

		/* ウィンドウ内の描画 */
		if ((nDrawTop >> 1) > window_dy1) {
			wdtop = (WORD)(nDrawTop >> 1);
		}
		else {
			wdtop = window_dy1;
		}

		if ((nDrawBottom >> 1) < window_dy2) {
			wdbtm = (WORD)(nDrawBottom >> 1);
		}
		else {
			wdbtm = window_dy2;
		}

		if (wdbtm > wdtop) {
			Draw320WSub(wdtop, wdbtm, window_dx1, window_dx2);
		}

		/* ウィンドウ外 下側の描画 */
		if ((nDrawBottom >> 1) > window_dy2) {
			Draw320Sub(window_dy2, nDrawBottom >> 1);
		}
	}
	else {
		Draw320Sub(nDrawTop >> 1, nDrawBottom >> 1);
	}
#else
	Draw320Sub(nDrawTop >> 1, nDrawBottom >> 1);
#endif

	if (!bFullScan) {
		RenderSetOddLine();
	}

	BitBlt(nDrawLeft, nDrawTop,
			(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
			nDrawLeft, nDrawTop);

	/* 次回に備え、ワークリセット */
	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	bPaletFlag = FALSE;
	SetDrawFlag(FALSE);
}

#endif /* XM7_VER >=3 */

#endif /* _XWIN */

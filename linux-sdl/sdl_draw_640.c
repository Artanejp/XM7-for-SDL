/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *	[ SDL 表示 / 640ドットパート]
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
static BOOL bDoubleDraw;


/*
 * 共通ルーチンのextern設定
 */
extern BOOL BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight, int nSrcLeft, int nSrcTop);
extern void RenderFullScan(void);
extern void RenderSetOddLine(void);
extern void SetDrawFlag(BOOL flag);
extern void AllClear(void);


static inline void __SETDOT_640i(Uint8 *addr, DWORD c)
{

  DWORD *addr32 = (DWORD *)addr;
  
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    *addr32 = ( c >> 8) | 0xff000000; 
#else
                    *addr32 = ( c << 8) | 0x000000ff; 

#endif	   
   
} 
/*
 * 直接SDLの画面を叩くときに使う
 */
static inline void __SETDOT_DDRAW_640i(Uint8 *addr, DWORD c)
{

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    addr[2] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[0] = (c >>16) & 0xff; /* G */
#else
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */

#endif	   
   
} 



/*
 * 640x200モード、プログレッシブ点打ち。
 * ビット配列は[ABRG]である
 * 20100124 32bit前提になった為、Uint32 (DWORD)で一気に書き込むようにする。
 */
static inline void __SETDOT_640p(Uint8 *addr, int pitch, DWORD c)
{

  DWORD *addr32;
  
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;

                    addr += pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
#else
                    addr32 = (DWORD *)addr;
                    *addr32 = (c <<8) | 0x000000ff;

                    addr += pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c <<8) | 0x000000ff;
#endif	   
   
} 

/*
 * 640x200モード、プログレッシブ点打ち。(DIRECT DRAW)
 * ビット配列は[BRG]である
 */
static inline void __SETDOT_DDRAW_640p(Uint8 *addr, int pitch, DWORD c)
{

  
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    addr[2] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[0] = (c >>16) & 0xff; /* G */
                    addr += pitch;
                    addr[2] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[0] = (c >>16) & 0xff; /* G */

#else
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
                    addr += pitch;
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */

#endif	   
   
} 

static inline void __SETBYTE_DDRAW_640p(Uint8 *addr, int bpp, int pitch, DWORD *c)
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
    //addr += bpp;
}

static inline void __SETBYTE_DDRAW_640i(Uint8 *addr, int bpp, int pitch, DWORD *c)
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
    //addr += bpp;
}

static void __SETBYTE_DDRAW_1280p(Uint8 *addr, int bpp, int pitch, DWORD *c)
{
  int i;
  DWORD c0, c1, c2, c3;
  for(i = 7; i >=0 ; i-=4) {
    c0 = c[i];
    c1 = c[i-1];
    c2 = c[i-2];
    c3 = c[i-3];
    __SETDOT_DDRAW_640p(addr, pitch, c0);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c0);
    __SETDOT_DDRAW_640p(addr + pitch * 2, pitch, c0);
    __SETDOT_DDRAW_640p(addr + bpp + pitch * 2, pitch, c0);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p(addr, pitch, c1);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c1);
    __SETDOT_DDRAW_640p(addr + pitch * 2, pitch, c1);
    __SETDOT_DDRAW_640p(addr + bpp + pitch * 2, pitch, c1);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p(addr, pitch, c2);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c2);
    __SETDOT_DDRAW_640p(addr + pitch * 2, pitch, c2);
    __SETDOT_DDRAW_640p(addr + bpp + pitch * 2, pitch, c2);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p(addr, pitch, c3);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c3);
    __SETDOT_DDRAW_640p(addr + pitch * 2, pitch, c3);
    __SETDOT_DDRAW_640p(addr + bpp + pitch * 2, pitch, c3);
    addr += bpp * 2;

  }
}

static void __SETBYTE_DDRAW_1280i(Uint8 *addr, int bpp, int pitch, DWORD *c)
{
  int i;
  DWORD c0, c1, c2, c3;
  for(i = 7; i >=0 ; i-=4) {
    c0 = c[i];
    c1 = c[i-1];
    c2 = c[i-2];
    c3 = c[i-3];
    __SETDOT_DDRAW_640p(addr, pitch, c0);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c0);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p(addr, pitch, c1);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c1);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p(addr, pitch, c2);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c2);
    addr += bpp * 2;

    __SETDOT_DDRAW_640p(addr, pitch, c3);
    __SETDOT_DDRAW_640p(addr + bpp, pitch, c3);
    addr += bpp * 2;

  }

}




/*
 * 8色モード,200ラインのVRAMデータを取り込んでピクセルデータに変換する
 */
static inline void __GETVRAM_3bpp(BYTE *vram, int x, int y, DWORD *c)
{
  BYTE cb, cr, cg;
  int offset;
 
  /* オフセット設定 */
  offset = 80 * y + x;
#if XM7_VER >= 3
  cb = vram[offset + 0x00000];
  cr = vram[offset + 0x08000];
  cg = vram[offset + 0x10000];
#else
  cb = vram[offset + 0x00000];
  cr = vram[offset + 0x04000];
  cg = vram[offset + 0x08000];
#endif /* XM7_VER */

  c[0] = rgbTTLGDI[(cb & 0x01) + ((cr & 0x01) <<1) + ((cg & 0x01) <<2)];
  c[1] = rgbTTLGDI[((cb & 0x02) >>1)  + (cr & 0x02)  + ((cg & 0x02) <<1)];  
  c[2] = rgbTTLGDI[((cb & 0x04) >>2)  + ((cr & 0x04) >>1) + (cg & 0x04)];  
  c[3] = rgbTTLGDI[((cb & 0x08) >>3)  + ((cr & 0x08) >>2)  + ((cg & 0x08) >>1)];  
  c[4] = rgbTTLGDI[((cb & 0x10) >>4)  + ((cr & 0x10) >>3)  + ((cg & 0x10) >>2)];  
  c[5] = rgbTTLGDI[((cb & 0x20) >>5)  + ((cr & 0x20) >>4) + ((cg & 0x20) >>3)];  
  c[6] = rgbTTLGDI[((cb & 0x40) >>6)  + ((cr & 0x40) >>5) + ((cg & 0x40) >>4)];  
  c[7] = rgbTTLGDI[((cb & 0x80) >>7)  + ((cr & 0x80) >>6) + ((cg & 0x80) >>5)];
}

/*
 *	640x200、デジタルモード
 *	ウィンドウ外描画サブ
 */

static void Draw640Sub2_DDRAW640p(int top, int bottom) {
  int x, y;
  int i;
  BYTE bit;
  Uint8 *addr;
  DWORD c[8];
  
  /* yループ */
  for (y=top; y<bottom; y++) {
    /* xループ */
    for (x=nDrawLeft>>3; x<nDrawRight>>3; x++) {
      addr =(Uint8 *)realDrawArea->pixels + 
        (y<<1) * realDrawArea->pitch + 
        (x <<3) * realDrawArea->format->BytesPerPixel;    
      __GETVRAM_3bpp(vram_dptr, x, y, c);
      __SETBYTE_DDRAW_640p(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
    }
    
  }
}

static void Draw640Sub2_DDRAW640i(int top, int bottom) {
  int x, y;
  int i;
  BYTE bit;
  Uint8 *addr;

  DWORD c[8];
  
  /* yループ */
  for (y=top; y<bottom; y++) {
    /* xループ */
    for (x=nDrawLeft>>3; x<nDrawRight>>3; x++) {
      addr =(Uint8 *)realDrawArea->pixels + 
        (y<<1) * realDrawArea->pitch + 
        (x<<3) * realDrawArea->format->BytesPerPixel;    
      __GETVRAM_3bpp(vram_dptr, x, y, c);
      __SETBYTE_DDRAW_640i(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
    }
    
  }
}


static void Draw640Sub2_DDRAW1280p(int top, int bottom) {
  int x, y;
  int i;
  int offset;
  BYTE bit;
  Uint8 *addr;
  DWORD c[8];
  
  /* yループ */
  for (y=top; y<bottom; y++) {
    /* xループ */
    for (x=nDrawLeft>>3; x<nDrawRight>>3; x++) {
      addr =(Uint8 *)realDrawArea->pixels + 
        (y<<2) * realDrawArea->pitch + 
        (x<<4) * realDrawArea->format->BytesPerPixel;    
  
      __GETVRAM_3bpp(vram_dptr, x, y, c);
      __SETBYTE_DDRAW_1280p(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
    }
    
  }
}

static void Draw640Sub2_DDRAW1280i(int top, int bottom) {
  int x, y;
  int i;
  BYTE bit;
  Uint8 *addr;
  DWORD c[8];
  /* yループ */
  for (y=top; y<bottom; y++) {
    /* xループ */
    for (x=nDrawLeft>>3; x<nDrawRight>>3; x++) {
      addr =(Uint8 *)realDrawArea->pixels + 
        (y<<2) * realDrawArea->pitch + 
        (x<<4) * realDrawArea->format->BytesPerPixel;    
      __GETVRAM_3bpp(vram_dptr, x, y, c);
      __SETBYTE_DDRAW_1280i(addr, realDrawArea->format->BytesPerPixel, realDrawArea->pitch, c);
    }
    
  }
}

static void Draw640Sub(int top, int bottom) {
  int x, y;
  int i;
  int offset;
  BYTE bit;
  Uint8 *addr;
  DWORD c[8];
  
  SDL_LockSurface(realDrawArea);
        switch(nDrawWidth) {
        case 1280:
          if(bFullScan) {
            Draw640Sub2_DDRAW1280p(top, bottom);
          } else {
            Draw640Sub2_DDRAW1280i(top, bottom);
          }
          break;
        default:
          if(bFullScan) {
            Draw640Sub2_DDRAW640p(top, bottom);
          } else {
            Draw640Sub2_DDRAW640i(top, bottom);
          }
          break;
        }

  SDL_UnlockSurface(realDrawArea);
}


static void Draw640WSub2_DDRAW640p(int top, int bottom, int left, int right)
{
  Uint8 *pos,*posbase;
  int x, y;
  int pitch, bpp;
  DWORD c[8];
  

  pitch = realDrawArea->pitch;
  bpp = realDrawArea->format->BytesPerPixel;
  posbase = (Uint8 *)realDrawArea->pixels + 
    top * pitch * 2 +
    left * bpp;

      for (y=top; y<bottom; y++) {
          pos = posbase;
		/* xループ */
          for (x = left; x < right ; x++) {
            __GETVRAM_3bpp(vram_bdptr, x, y, c);
                  __SETBYTE_DDRAW_640p(pos, bpp, pitch, c);
                  pos += bpp * 8;
                }
          posbase += (pitch * 2);
        }
}

static void Draw640WSub2_DDRAW640i(int top, int bottom, int left, int right)
{
  Uint8 *pos,*posbase;
  int x, y;
  int pitch, bpp;
  DWORD c[8];
  
  pitch = realDrawArea->pitch;
  bpp = realDrawArea->format->BytesPerPixel;
  posbase = (Uint8 *)realDrawArea->pixels + 
    top * pitch * 2 +
    left * bpp;

      for (y=top; y<bottom; y++) {
          pos = posbase;
		/* xループ */
          for (x = left; x < right; x++) {
            __GETVRAM_3bpp(vram_bdptr, x, y, c);
                  __SETBYTE_DDRAW_640i(pos, bpp, pitch, c);
                  pos += bpp * 8;
                }
          posbase += pitch * 2;
        }
}

static void Draw640WSub2_DDRAW1280p(int top, int bottom, int left, int right)
{
  Uint8 *pos,*posbase;
  int x, y;
  int pitch, bpp;
  DWORD c[8];


  pitch = realDrawArea->pitch;
  bpp = realDrawArea->format->BytesPerPixel;
  posbase = (Uint8 *)realDrawArea->pixels + 
    top * pitch * 4 +
    left * bpp * 2;

      for (y=top; y<bottom; y++) {
          pos = posbase;
		/* xループ */
          for (x = left; x < right; x++) {
            __GETVRAM_3bpp(vram_bdptr, x, y, c);
                  __SETBYTE_DDRAW_1280p(pos, bpp, pitch, c);
                  pos += bpp * 16;
                }
          posbase += pitch * 4;
        }
}

static void Draw640WSub2_DDRAW1280i(int top, int bottom, int left, int right)
{
  Uint8 *pos,*posbase;
  int x, y;
  int pitch, bpp;
  DWORD c[8];


  pitch = realDrawArea->pitch;
  bpp = realDrawArea->format->BytesPerPixel;
  posbase = (Uint8 *)realDrawArea->pixels + 
    top * pitch * 4 +
    left * bpp * 2;

      for (y=top; y<bottom; y++) {
          pos = posbase;
		/* xループ */
          for (x = left; x< right; x++) {
            __GETVRAM_3bpp(vram_bdptr, x, y, c);
                  __SETBYTE_DDRAW_1280i(pos, bpp, pitch, c);
                  pos += bpp * 16;
                }
          posbase += pitch * 4;
        }
}

#if XM7_VER >= 3



/*
 *	640x200、デジタルモード
 *	ウィンドウ内描画サブ
 */
static void Draw640WSub(int top, int bottom, int left, int right) {


        SDL_LockSurface(realDrawArea);

        switch(nDrawWidth) {
        case 1280:
          if(bFullScan) {
            Draw640WSub2_DDRAW1280p(top, bottom, left, right);
          } else {
            Draw640WSub2_DDRAW1280i(top, bottom, left, right);
          }
          break;
        default:
          if(bFullScan) {
            Draw640WSub2_DDRAW640p(top, bottom, left, right);
          } else {
            Draw640WSub2_DDRAW640i(top, bottom, left, right);
          }
          break;
        }
        SDL_UnlockSurface(realDrawArea);
}
#endif
/*
 *	640x200/400、デジタルモード
 *	パレット設定
 */
void Palet640(void)
{
	int i;
	int vpage;

   
	/* パレットテーブル */
	static DWORD rgbTable[] = {
		0x00000000,
		0x000000ff,
		0x00ff0000,
		0x00ff00ff,
		0x0000ff00,
		0x0000ffff,
		0x00ffff00,
		0x00ffffff
	};

	/* マルチページより、表示プレーン情報を得る */
	vpage = (~(multi_page >> 4)) & 0x07;

	/* 640x200/400、デジタルパレット */
	for (i=0; i<8; i++) {
		if (crt_flag) {
			/* CRT ON */
			rgbTTLGDI[i] = rgbTable[ttl_palet[i & vpage] & 0x07];
		}
		else {
			/* CRT OFF */
			rgbTTLGDI[i] = rgbTable[0];
		}
	}

	/* 奇数ライン用 */
	rgbTTLGDI[8] = rgbTable[0];
	rgbTTLGDI[9] = rgbTable[4];
}
/*
 *	640x200、デジタルモード
 *	描画
 */
void Draw640(void)
{
#if XM7_VER >= 3
	WORD wdtop, wdbtm;
#endif
        realDrawArea = SDL_GetVideoSurface();
	/* パレット設定 */
	if (bPaletFlag) {
		Palet640();
		nDrawTop = 0;
		nDrawBottom = 400;
		nDrawLeft = 0;
		nDrawRight = 640;
		SetDrawFlag(TRUE);
	}

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if ((nDrawTop < nDrawBottom) && (nDrawLeft < nDrawRight)) {
#if XM7_VER >= 3
		/* ウィンドウオープン時 */
		if (window_open) {
			/* ウィンドウ外 上側の描画 */
			if ((nDrawTop >> 1) < window_dy1) {
				Draw640Sub(nDrawTop >> 1, window_dy1);
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
				Draw640WSub(wdtop, wdbtm, window_dx1, window_dx2);
			}

			/* ウィンドウ外 下側の描画 */
			if ((nDrawBottom >> 1) > window_dy2) {
				Draw640Sub(window_dy2, nDrawBottom >> 1);
			}
		}
		else {
			Draw640Sub(nDrawTop >> 1, nDrawBottom >> 1);
		}
#else
		Draw640Sub(nDrawTop >> 1, nDrawBottom >> 1);
#endif
		if(!bFullScan){
			RenderSetOddLine();
		}
	}

	BitBlt(nDrawLeft, nDrawTop,
			(nDrawRight - nDrawLeft), (nDrawBottom - nDrawTop),
			nDrawLeft, nDrawTop);

	nDrawTop = 400;
	nDrawBottom = 0;
	nDrawLeft = 640;
	nDrawRight = 0;
	bPaletFlag = FALSE;
	SetDrawFlag(FALSE);
}


#endif /* _XWIN */

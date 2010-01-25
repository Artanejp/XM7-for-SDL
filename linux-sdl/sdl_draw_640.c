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


/*
 * 共通ルーチンのextern設定
 */
extern BOOL BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight, int nSrcLeft, int nSrcTop);
extern void RenderFullScan(void);
extern void RenderSetOddLine(void);
extern void SetDrawFlag(BOOL flag);
extern void AllClear(void);

static inline void __SETDOT_640i(WORD x, WORD y, DWORD c)
{

  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * realDrawArea->pitch + x * realDrawArea->format->BytesPerPixel;
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
static inline void __SETDOT_DDRAW_640i(WORD x, WORD y, DWORD c)
{

  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * realDrawArea->pitch + x * realDrawArea->format->BytesPerPixel;
  DWORD *addr32 = (DWORD *)addr;
  
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
static inline void __SETDOT_640p(WORD x, WORD y, DWORD c)
{

  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * realDrawArea->pitch + x * realDrawArea->format->BytesPerPixel;
  DWORD *addr32;
  
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;

                    addr += realDrawArea->pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
#else
                    addr32 = (DWORD *)addr;
                    *addr32 = (c <<8) | 0x000000ff;

                    addr += realDrawArea->pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c <<8) | 0x000000ff;
#endif	   
   
} 

/*
 * 640x200モード、プログレッシブ点打ち。(DIRECT DRAW)
 * ビット配列は[BRG]である
 */
static inline void __SETDOT_DDRAW_640p(WORD x, WORD y, DWORD c)
{

  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * realDrawArea->pitch + x * realDrawArea->format->BytesPerPixel;
  
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;

                    addr += realDrawArea->pitch;
                    addr32 = (DWORD *)addr;
                    *addr32 = (c >> 8) | 0xff000000;
#else
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */
                    addr += realDrawArea->pitch;
                    addr[0] = c & 0xff;  /* B */
                    addr[1] = ( c>>8 ) & 0xff; /* R */
                    addr[2] = (c >>16) & 0xff; /* G */

#endif	   
   
} 


/*
 *	640x200、デジタルモード
 *	ウィンドウ外描画サブ
 */
static void Draw640Sub(int top, int bottom) {
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE *vramptr;
	BYTE col[2];
        BYTE c7,c6,c5,c4,c3,c2,c1,c0;
        BYTE cb,cr,cg;
        int addr;


        SDL_LockSurface(realDrawArea);
	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=nDrawLeft>>3; x<nDrawRight>>3; x++) {
                  //if(GDIDrawFlag[(y >>3)  * 80 + x] == 0) continue; /* 再描画フラグを真面目に見てみることにした */
			bit = 0x80;
			vramptr = vram_dptr;

			/* オフセット設定 */
			offset = 80 * y + x;

#if XM7_VER >= 3
		       cb = vramptr[offset + 0x00000];
		       cr = vramptr[offset + 0x08000];
		       cg = vramptr[offset + 0x10000];
#else
  		       cb = vramptr[offset + 0x00000];
		       cr = vramptr[offset + 0x04000];
		       cg = vramptr[offset + 0x08000];
#endif /* XM7_VER */
//		       addr = ((y * 1280) + x<<3) *3;
		   
		       c0 = (cb & 0x01) + ((cr & 0x01) <<1) + ((cg & 0x01) <<2);
		       c1 = ((cb & 0x02) >>1)  + (cr & 0x02)  + ((cg & 0x02) <<1);  
		       c2 = ((cb & 0x04) >>2)  + ((cr & 0x04) >>1) + (cg & 0x04);  
		       c3 = ((cb & 0x08) >>3)  + ((cr & 0x08) >>2)  + ((cg & 0x08) >>1);  
		       c4 = ((cb & 0x10) >>4)  + ((cr & 0x10) >>3)  + ((cg & 0x10) >>2);  
		       c5 = ((cb & 0x20) >>5)  + ((cr & 0x20) >>4) + ((cg & 0x20) >>3);  
		       c6 = ((cb & 0x40) >>6)  + ((cr & 0x40) >>5) + ((cg & 0x40) >>4);  
		       c7 = ((cb & 0x80) >>7)  + ((cr & 0x80) >>6) + ((cg & 0x80) >>5);
                       if(bDirectDraw) {
                         if(bFullScan) {
		       __SETDOT_DDRAW_640p((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_DDRAW_640p((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_DDRAW_640p((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_DDRAW_640p((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_DDRAW_640p((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_DDRAW_640p((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_DDRAW_640p((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_DDRAW_640p((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       } else {
		       __SETDOT_DDRAW_640i((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_DDRAW_640i((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_DDRAW_640i((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_DDRAW_640i((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_DDRAW_640i((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_DDRAW_640i((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_DDRAW_640i((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_DDRAW_640i((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       } 
                       } else {
                       if(bFullScan) {
		       __SETDOT_640p((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_640p((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_640p((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_640p((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_640p((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_640p((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_640p((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_640p((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       } else {
		       __SETDOT_640i((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_640i((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_640i((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_640i((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_640i((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_640i((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_640i((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_640i((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       }
                       }

		}
	   
	}


        SDL_UnlockSurface(realDrawArea);
}
#if XM7_VER >= 3
/*
 *	640x200、デジタルモード
 *	ウィンドウ内描画サブ
 */
static void Draw640WSub(int top, int bottom, int left, int right) {
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE *vramptr;
//	BYTE col[2];
//	DWORD col[8];
        BYTE c7,c6,c5,c4,c3,c2,c1,c0;
        BYTE cb,cr,cg;
        int addr;


        SDL_LockSurface(realDrawArea);
	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=left; x<right; x++) {
                  //if(GDIDrawFlag[(y >>3)  * 80 + x] == 0) continue; /* 再描画フラグを真面目に見てみることにした */
			bit = 0x80;
			vramptr = vram_bdptr;
                        /* R,G,Bについて8bit単位で描画する。
                         * 高速化…キャッシュヒット率の向上を考慮して、
                         * インライン展開と細かいループの廃止を同時に行う
                         */ 

			/* オフセット設定 */
			offset = 80 * y + x;
		       cb = vramptr[offset + 0x00000];
		       cr = vramptr[offset + 0x08000];
		       cg = vramptr[offset + 0x10000];
		   
		       c0 = (cb & 0x01) + ((cr & 0x01) <<1) + ((cg & 0x01) <<2);
		       c1 = ((cb & 0x02) >>1)  + (cr & 0x02)  + ((cg & 0x02) <<1);  
		       c2 = ((cb & 0x04) >>2)  + ((cr & 0x04) >>1) + (cg & 0x04);  
		       c3 = ((cb & 0x08) >>3)  + ((cr & 0x08) >>2)  + ((cg & 0x08) >>1);  
		       c4 = ((cb & 0x10) >>4)  + ((cr & 0x10) >>3)  + ((cg & 0x10) >>2);  
		       c5 = ((cb & 0x20) >>5)  + ((cr & 0x20) >>4) + ((cg & 0x20) >>3);  
		       c6 = ((cb & 0x40) >>6)  + ((cr & 0x40) >>5) + ((cg & 0x40) >>4);  
		       c7 = ((cb & 0x80) >>7)  + ((cr & 0x80) >>6) + ((cg & 0x80) >>5);
                       if(bDirectDraw) {
                         if(bFullScan) {
		       __SETDOT_DDRAW_640p((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_DDRAW_640p((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_DDRAW_640p((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_DDRAW_640p((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_DDRAW_640p((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_DDRAW_640p((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_DDRAW_640p((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_DDRAW_640p((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       } else {
		       __SETDOT_DDRAW_640i((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_DDRAW_640i((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_DDRAW_640i((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_DDRAW_640i((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_DDRAW_640i((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_DDRAW_640i((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_DDRAW_640i((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_DDRAW_640i((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       } 
                       } else {
                       if(bFullScan) {
		       __SETDOT_640p((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_640p((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_640p((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_640p((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_640p((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_640p((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_640p((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_640p((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       } else {
		       __SETDOT_640i((x<<3)+0  ,y<<1 ,rgbTTLGDI[c7]);
		       __SETDOT_640i((x<<3)+1  ,y<<1 ,rgbTTLGDI[c6]);
  		       __SETDOT_640i((x<<3)+2  ,y<<1 ,rgbTTLGDI[c5]);
   		       __SETDOT_640i((x<<3)+3  ,y<<1 ,rgbTTLGDI[c4]);
   		       __SETDOT_640i((x<<3)+4  ,y<<1 ,rgbTTLGDI[c3]);
   		       __SETDOT_640i((x<<3)+5  ,y<<1 ,rgbTTLGDI[c2]);
   		       __SETDOT_640i((x<<3)+6  ,y<<1 ,rgbTTLGDI[c1]);
   		       __SETDOT_640i((x<<3)+7  ,y<<1 ,rgbTTLGDI[c0]);
                       }
                       }

                }
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

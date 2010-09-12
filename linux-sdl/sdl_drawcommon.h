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

#ifndef SDL_DRAWCOMMON_H
#define SDL_DRAWCOMMON_H

#include <SDL.h>
#include "xm7.h"
#include "sdl_draw.h"
#include "sdl.h"



/*
 *      グローバル ワーク(のextern設定)
 */
extern DWORD    rgbTTLGDI[16];	/* デジタルパレット */
extern DWORD    rgbAnalogGDI[4096];	/* アナログパレット */
//extern BOOL     bDirectDraw;	/* 直接書き込みフラグ */
//extern SDL_Surface *realDrawArea;	/* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する)  */
//extern WORD     nDrawTop;	/* 描画範囲上 */
//extern WORD     nDrawBottom;	/* 描画範囲下 */
//extern WORD     nDrawLeft;	/* 描画範囲左 */
//extern WORD     nDrawRight;	/* 描画範囲右 */
//extern BOOL     bPaletFlag;	/* パレット変更フラグ */
//extern BOOL     bClearFlag;	/* クリアフラグ */


/*
 * 共通ルーチンのextern設定
 */
extern BOOL     BitBlt(int nDestLeft, int nDestTop, int nWidth,
		       int nHeight, int nSrcLeft, int nSrcTop);
extern void     RenderFullScan(void);
//extern void     RenderSetOddLine(void);
extern void     SetDrawFlag(BOOL flag);
extern void     AllClear(void);


#include "sdl_draw_640.h"
#include "sdl_draw_320.h"
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
 * 8色モード,200ラインのVRAMデータを取り込んでピクセルデータに変換する
 * ハードウェアパレット対応
 */
static inline void
__GETVRAM_3bpp_HW(BYTE * vram, int x, int y, DWORD * c)
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

        c[0] =   (cb & 0x01) + ((cr & 0x01) << 1) + ((cg & 0x01) << 2);
        c[1] =   ((cb & 0x02) >> 1) + (cr & 0x02) + ((cg & 0x02) << 1);
        c[2] =   ((cb & 0x04) >> 2) + ((cr & 0x04) >> 1) + (cg & 0x04);
        c[3] =   ((cb & 0x08) >> 3) + ((cr & 0x08) >> 2) +((cg & 0x08) >> 1);
        c[4] =   ((cb & 0x10) >> 4) + ((cr & 0x10) >> 3) + ((cg & 0x10) >> 2);
        c[5] =   ((cb & 0x20) >> 5) + ((cr & 0x20) >> 4) + ((cg & 0x20) >> 3);
        c[6] =   ((cb & 0x40) >> 6) + ((cr & 0x40) >> 5) + ((cg & 0x40) >> 4);
        c[7] =   ((cb & 0x80) >> 7) + ((cr & 0x80) >> 6) + ((cg & 0x80) >> 5);
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
    int i;

    /*
     * オフセット設定 
     */
    offset = 40 * y + x;
    /*
     * R,G,Bについて8bit単位で描画する。
     * 高速化…キャッシュヒット率の向上を考慮して、
     * インライン展開と細かいループの廃止を同時に行う 
     */

    c[0] = 0x00000000;
    c[1] = 0x00000000;
    c[2] = 0x00000000;
    c[3] = 0x00000000;
    c[4] = 0x00000000;
    c[5] = 0x00000000;
    c[6] = 0x00000000;
    c[7] = 0x00000000;

    if(!(multi_page & 0x40)){
    	g[5] = vramptr[offset + 0x10000];
    	g[4] = vramptr[offset + 0x12000];
    	g[3] = vramptr[offset + 0x14000];
    	g[2] = vramptr[offset + 0x16000];
    	g[1] = vramptr[offset + 0x28000];
    	g[0] = vramptr[offset + 0x2a000];
    }
    if(!(multi_page & 0x20)){
    	r[5] = vramptr[offset + 0x08000];
    	r[4] = vramptr[offset + 0x0a000];
    	r[3] = vramptr[offset + 0x0c000];
    	r[2] = vramptr[offset + 0x0e000];
    	r[1] = vramptr[offset + 0x20000];
    	r[0] = vramptr[offset + 0x22000];
    }
    if(!(multi_page & 0x10)){
    	b[5] = vramptr[offset + 0x00000];
    	b[4] = vramptr[offset + 0x02000];
    	b[3] = vramptr[offset + 0x04000];
    	b[2] = vramptr[offset + 0x06000];
    	b[1] = vramptr[offset + 0x18000];
    	b[0] = vramptr[offset + 0x1a000];
    }

 for(i = 0; i<8;i++){
	 c[i] = 0x00000000;
   if(!(multi_page & 0x20)){
	   c[i] = c[i] | (r[0] & 0x80)>>5 | (r[1] & 0x80)>>4 | (r[2] & 0x80)>>3 | (r[3] & 0x80)>>2 | (r[4] & 0x80)>>1 | (r[5] & 0x80);
	    r[0]<<=1;
	    r[1]<<=1;
	    r[2]<<=1;
	    r[3]<<=1;
	    r[4]<<=1;
	    r[5]<<=1;
	    if((c[i] & 0x0000f8)!=0) c[i] |= 0x000003;
   }
   c[i] <<=8;
   if(!(multi_page & 0x40)){
	   c[i] = c[i] | (g[0] & 0x80)>>5 | (g[1] & 0x80)>>4 | (g[2] & 0x80)>>3 | (g[3] & 0x80)>>2 | (g[4] & 0x80)>>1 | (g[5] & 0x80);
	    g[0]<<=1;
	    g[1]<<=1;
	    g[2]<<=1;
	    g[3]<<=1;
	    g[4]<<=1;
	    g[5]<<=1;
	    if((c[i] & 0x0000f8)!=0) c[i] |= 0x000003;
   }
   c[i] <<=8;
   if(!(multi_page & 0x10)){
	   c[i] = c[i] | (b[0] & 0x80)>>5 | (b[1] & 0x80)>>4 | (b[2] & 0x80)>>3 | (b[3] & 0x80)>>2 | (b[4] & 0x80)>>1 | (b[5] & 0x80);
	    b[0]<<=1;
	    b[1]<<=1;
	    b[2]<<=1;
	    b[3]<<=1;
	    b[4]<<=1;
	    b[5]<<=1;
	    if((c[i] & 0x0000f8)!=0) c[i] |= 0x000003;
   }

 }
}

#endif /* #ifndef SDL_DRAWCOMMON_H */


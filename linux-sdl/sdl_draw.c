/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *               Copyright (C) 2010      K.Ohta
 *
 *	[ XWIN 表示 ]
 */

#ifdef _XWIN

#include <gtk/gtk.h>
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
 *	グローバル ワーク
 */
DWORD rgbTTLGDI[16];						/* デジタルパレット */
DWORD rgbAnalogGDI[4096];					/* アナログパレット */
//guchar pBitsGDI[400*640*3];					/* ビットデータ */
BYTE GDIDrawFlag[4000];						/* 8x8 再描画領域フラグ */
BOOL bFullScan;								/* フルスキャン(Window) */

/*
 *	スタティック ワーク
 */
#if XM7_VER >= 3
static BYTE bMode;							/* 画面モード */
#else
static BOOL bAnalog;						/* アナログモードフラグ */
#endif
static BYTE bNowBPP;						/* 現在のビット深度 */
static WORD nDrawTop;						/* 描画範囲上 */
static WORD nDrawBottom;					/* 描画範囲下 */
static WORD nDrawLeft;						/* 描画範囲左 */
static WORD nDrawRight;						/* 描画範囲右 */
static BOOL bPaletFlag;						/* パレット変更フラグ */
static BOOL bClearFlag;						/* クリアフラグ */
#if XM7_VER >= 3
static BOOL bWindowOpen;					/* ハードウェアウィンドウ状態 */
static WORD nWindowDx1;						/* ウィンドウ左上X座標 */
static WORD nWindowDy1;						/* ウィンドウ左上Y座標 */
static WORD nWindowDx2;						/* ウィンドウ右下X座標 */
static WORD nWindowDy2;						/* ウィンドウ右下Y座標 */
#endif

/*
 *	プロトタイプ宣言
 */
static void FASTCALL SetDrawFlag(BOOL flag);

/*
 * SETDOT（そのまま）
 */
static void FASTCALL SETDOT(WORD x, WORD y, DWORD c)
{
  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
   Uint8 *addr = (Uint8 *)drawArea->pixels + y * drawArea->pitch + x * drawArea->format->BytesPerPixel;
        SDL_LockSurface(drawArea);
        *(DWORD *)addr = c;
        SDL_UnlockSurface(drawArea);
   
} 

/*
 * SETDOT（inline）
 * 24bpp前提,SurfaceLockしません!!
 */
static inline void __SETDOT(WORD x, WORD y, DWORD c)
{

  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)drawArea->pixels + y * drawArea->pitch + x * drawArea->format->BytesPerPixel;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN   
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
#else
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;

#endif	   
   
} 

static inline void __SETDOT_640i(WORD x, WORD y, DWORD c)
{

  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)drawArea->pixels + y * drawArea->pitch + x * drawArea->format->BytesPerPixel;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN   
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
#else
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;

#endif	   
   
} 
static inline void __SETDOT_640p(WORD x, WORD y, DWORD c)
{

  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)drawArea->pixels + y * drawArea->pitch + x * drawArea->format->BytesPerPixel;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN   
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
                    addr += drawArea->pitch;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;

#else
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
                    addr += drawArea->pitch;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;


#endif	   
   
} 

/*
 * SETDOT（inline） 拡大モード
 * 24bpp前提,SurfaceLockしません!!
 */
static inline void __SETDOT_DOUBLE(WORD x, WORD y, DWORD c)
{
  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * 2 * displayArea->pitch + x * 2 * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)drawArea->pixels + y * 2 * drawArea->pitch + x * 2 * drawArea->format->BytesPerPixel;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN   
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
           //addr += displayArea->format->BytesPerPixel;
           addr += drawArea->format->BytesPerPixel;
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
	   //addr += displayArea->pitch;
           addr += drawArea->pitch;
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
           //addr -= displayArea->format->BytesPerPixel;
           addr -= drawArea->format->BytesPerPixel;
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
#else
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
	   /* 横拡大 */
           //addr += displayArea->format->BytesPerPixel;
           addr += drawArea->format->BytesPerPixel;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
	   /* 縦拡大 */
	   //addr += displayArea->pitch;
           addr += drawArea->pitch;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
           //addr -= displayArea->format->BytesPerPixel;
           addr -= drawArea->format->BytesPerPixel;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
#endif	   
} 

/*
 * 320x200モードでのドット打ち(横二ドット一気に打つ)
 */
static inline void __SETDOT_320i(WORD x, WORD y, DWORD c)
{
        Uint8 *addr = (Uint8 *)drawArea->pixels + y * drawArea->pitch + x * 2 * drawArea->format->BytesPerPixel;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN   
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
           //addr += displayArea->format->BytesPerPixel;
           addr += drawArea->format->BytesPerPixel;
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
#else
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
	   /* 横拡大 */
           //addr += displayArea->format->BytesPerPixel;
           addr += drawArea->format->BytesPerPixel;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
#endif	   
}

static inline void __SETDOT_320p(WORD x, WORD y, DWORD c)
{
        Uint8 *addr = (Uint8 *)drawArea->pixels + y * drawArea->pitch + x * 2 * drawArea->format->BytesPerPixel;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN   
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
           //addr += displayArea->format->BytesPerPixel;
           addr += drawArea->format->BytesPerPixel;
	   addr[2] = c & 0xff;
	   addr[1] = (c >>8 ) & 0xff;
	   addr[0] = (c >>16) & 0xff;
#else
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
	   /* 横拡大 */
           //addr += displayArea->format->BytesPerPixel;
           addr += drawArea->format->BytesPerPixel;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
           /* 縦拡大 */
           addr += drawArea->pitch;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;

           addr -= drawArea->format->BytesPerPixel;
	   addr[0] = c & 0xff;
	   addr[1] = ( c>>8 ) & 0xff;
	   addr[2] = (c >>16) & 0xff;
           
#endif	   
}



/*
 * BITBLT
 */
static BOOL BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight, int nSrcLeft, int nSrcTop)
{
   SDL_Rect srcrect,dstrect;
   srcrect.x = nSrcLeft;
   srcrect.y = nSrcTop;
   srcrect.w = (Uint16)nWidth;
   srcrect.h = (Uint16)nHeight;
   
   dstrect.x = nDestLeft;
   dstrect.y = nDestTop;
   dstrect.w = (Uint16)nWidth;
   dstrect.h = (Uint16)nHeight;

   /* SurfaceLock */
   /* データ転送 */
   SDL_UpdateRect(drawArea, 0, 0, drawArea->w, drawArea->h);
   SDL_BlitSurface(drawArea, &srcrect ,displayArea ,&dstrect );
   SDL_UpdateRect(displayArea, 0, 0, displayArea->w, displayArea->h);
   //printf("BitBlt %d %d\n",drawArea->w, drawArea->h);

}

/*
 *	初期化
 */
void FASTCALL InitDraw(void)
{
	/* ワークエリア初期化 */
#if XM7_VER >= 3
	bMode = SCR_200LINE;
#else
	bAnalog = FALSE;
#endif
	bNowBPP = 24;
	memset(rgbTTLGDI, 0, sizeof(rgbTTLGDI));
	memset(rgbAnalogGDI, 0, sizeof(rgbAnalogGDI));

	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = FALSE;
	SetDrawFlag(FALSE);

#if XM7_VER >= 3
	bWindowOpen = FALSE;
	nWindowDx1 = 640;
	nWindowDy1 = 400;
	nWindowDx2 = 0;
	nWindowDy2 = 0;
#endif
        
}

/*
 *	クリーンアップ
 */
void FASTCALL CleanDraw(void)
{
}

/*
 *	全ての再描画フラグを設定
 */
static void FASTCALL SetDrawFlag(BOOL flag)
{
	memset(GDIDrawFlag, (BYTE)flag, sizeof(GDIDrawFlag));
}

/*
 *  640x200、デジタルモード
 *	セレクト
 */
static BOOL FASTCALL Select640(void)
{
	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

#if XM7_VER >= 3
	/* デジタル/200ラインモード */
	bMode = SCR_200LINE;
#else
	/* デジタルモード */
	bAnalog = FALSE;
#endif

	return TRUE;
}

#if XM7_VER >= 3
/*
 *  640x400、デジタルモード
 *	セレクト
 */
static BOOL FASTCALL Select400l(void)
{
	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* デジタル/400ラインモード */
	bMode = SCR_400LINE;

	return TRUE;
}
#endif

/*
 *  320x200、アナログモード
 *	セレクト
 */
static BOOL FASTCALL Select320(void)
{
	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

#if XM7_VER >= 3
	/* アナログ/200ラインモード */
	bMode = SCR_4096;
#else
	/* アナログモード */
	bAnalog = TRUE;
#endif

	return TRUE;
}

#if XM7_VER >= 3
/*
 *  320x200、26万色モード
 *	セレクト
 */
static BOOL FASTCALL Select256k()
{
	/* 全領域無効 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	SetDrawFlag(TRUE);

	/* アナログ(26万色)/200ラインモード */
	bMode = SCR_262144;

	return TRUE;
}
#endif

/*
 *	セレクトチェック
 */
static BOOL FASTCALL SelectCheck(void)
{

#if XM7_VER >= 3
	/* 限りない手抜き(ォ */
	if (bMode == screen_mode) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#else
	/* 320x200 */
	if (mode320) {
		if (bAnalog) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	/* 640x200 */
	if (!bAnalog) {
		return TRUE;
	}
	else {
		return FALSE;
	}
#endif
}

/*
 *  セレクト
 */
BOOL FASTCALL SelectDraw(void)
{
        SDL_Rect rect;
	/* 一致しているかチェック */
	if (SelectCheck()) {
		return TRUE;
	}

        rect.h = displayArea->h;
        rect.w = displayArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(displayArea);
        SDL_FillRect(displayArea, &rect, 0x00000000); 
        SDL_UnlockSurface(displayArea);

        rect.h = drawArea->h;
        rect.w = drawArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(drawArea);
        SDL_FillRect(drawArea, &rect, 0x00000000); 
        SDL_UnlockSurface(drawArea);

   
	/* セレクト */
#if XM7_VER >= 3
	switch (screen_mode) {
		case SCR_400LINE	:	return Select400l();
		case SCR_262144		:	return Select256k();
		case SCR_4096		:	return Select320();
		default				:	return Select640();
	}
#else
	if (mode320) {
		return Select320();
	}
	return Select640();
#endif
}

/*-[ 描画 ]-----------------------------------------------------------------*/

/*
 *	オールクリア
 */
static void FASTCALL AllClear(void)
{
        SDL_Rect rect;
        rect.h = displayArea->h;
        rect.w = displayArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(displayArea);
        SDL_FillRect(displayArea, &rect, 0x00000000); 
        SDL_UnlockSurface(displayArea);

        rect.h = drawArea->h;
        rect.w = drawArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(drawArea);
        SDL_FillRect(drawArea, &rect, 0x00000000); 
        SDL_UnlockSurface(drawArea);


	/* 全領域をレンダリング対象とする */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);

	bClearFlag = FALSE;
}

/*
 *	フルスキャン
 */
static void FASTCALL RenderFullScan(void)
{
	BYTE *p;
	BYTE *q;
	WORD u;
        Uint32 pitch;

        // SDL_LockSurface(displayArea);
	/* ポインタ初期化 */
        //p = (BYTE *)displayArea->pixels + nDrawTop * displayArea->pitch;
        //q = p + displayArea->pitch;
        //pitch = displayArea->pitch;
        SDL_LockSurface(drawArea);
	/* ポインタ初期化 */
        p = (BYTE *)drawArea->pixels + nDrawTop * drawArea->pitch;
        pitch = drawArea->pitch;
        q = p + pitch;


	/* ループ */
	for (u=nDrawTop; u<nDrawBottom; u += (WORD)2) {
		memcpy(q, p, pitch);
		p += pitch *2;
		q += pitch *2;
	}
        //SDL_UnlockSurface(displayArea);
        //SDL_UpdateRect(displayArea, 0, 0, displayArea->w, displayArea->h);
        SDL_UnlockSurface(drawArea);
        SDL_UpdateRect(drawArea, 0, 0, drawArea->w, drawArea->h);

}


/*
 *	奇数ライン設定
 */
static void FASTCALL RenderSetOddLine(void)
{
	BYTE *p;
	WORD u;
        Uint32 pitch;

	/* ポインタ初期化 */

        p = drawArea->pixels + (nDrawTop + 1) * drawArea->pitch;
        pitch = drawArea->pitch;
        SDL_LockSurface(drawArea);

	/* ループ */
	for (u=nDrawTop; u<nDrawBottom; u += (WORD)2) {
	   	memset(p, 0x00, pitch );
		p += pitch * 2;
	}
        SDL_UnlockSurface(drawArea);
        SDL_UpdateRect(drawArea, 0, 0, drawArea->w, drawArea->h);

}

/*
 *	640x200/400、デジタルモード
 *	パレット設定
 */
static void FASTCALL Palet640(void)
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
 *	ウィンドウ外描画サブ
 */
static void FASTCALL Draw640Sub(int top, int bottom) {
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE *vramptr;
	BYTE col[2];
        BYTE c7,c6,c5,c4,c3,c2,c1,c0;
        BYTE cb,cr,cg;
        int addr;

	//SDL_LockSurface(displayArea);
        SDL_LockSurface(drawArea);
	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=nDrawLeft>>3; x<nDrawRight>>3; x++) {
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

        //SDL_UnlockSurface(displayArea);
        SDL_UnlockSurface(drawArea);
}

#if XM7_VER >= 3
/*
 *	640x200、デジタルモード
 *	ウィンドウ内描画サブ
 */
static void FASTCALL Draw640WSub(int top, int bottom, int left, int right) {
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

	//SDL_LockSurface(displayArea);
        SDL_LockSurface(drawArea);
	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=left; x<right; x++) {
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
        //SDL_UnlockSurface(displayArea);
	   SDL_UnlockSurface(drawArea);
}
#endif

/*
 *	640x200、デジタルモード
 *	描画
 */
static void FASTCALL Draw640(void)
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

		if (bFullScan) {
                  //RenderFullScan();
		}
		else {
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

#if XM7_VER >= 3
/*
 *	640x400、デジタルモード
 *	ウィンドウ外描画サブ
 */
static void FASTCALL Draw400lSub(int top, int bottom) {
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE *vramptr;
	BYTE col[2];
	DWORD r,g,b;
        BYTE c0,c1,c2,c3,c4,c5,c6,c7;
        BYTE cb,cr,cg;

        //SDL_LockSurface(displayArea);
        SDL_LockSurface(drawArea);
	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=nDrawLeft>>3; x<nDrawRight>>3; x++) {
			bit = 0x80;
			vramptr = vram_dptr;

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
		       __SETDOT((x<<3)+0  ,y ,rgbTTLGDI[c7]);
		       __SETDOT((x<<3)+1  ,y ,rgbTTLGDI[c6]);
  		       __SETDOT((x<<3)+2  ,y ,rgbTTLGDI[c5]);
   		       __SETDOT((x<<3)+3  ,y ,rgbTTLGDI[c4]);
   		       __SETDOT((x<<3)+4  ,y ,rgbTTLGDI[c3]);
   		       __SETDOT((x<<3)+5  ,y ,rgbTTLGDI[c2]);
   		       __SETDOT((x<<3)+6  ,y ,rgbTTLGDI[c1]);
   		       __SETDOT((x<<3)+7  ,y ,rgbTTLGDI[c0]);

		}
	}
        //SDL_UnlockSurface(displayArea);
        SDL_UnlockSurface(drawArea);

}

/*
 *	640x400、デジタルモード
 *	ウィンドウ内描画サブ
 */
static void FASTCALL Draw400lWSub(int top, int bottom, int left, int right) {
	int x, y;
	int i;
	int offset;
	BYTE bit;
	BYTE *vramptr;
	BYTE col[2];
	DWORD r,g,b;
        BYTE c0,c1,c2,c3,c4,c5,c6,c7;
        BYTE cb,cr,cg;
        //        SDL_LockSurface(displayArea);
        SDL_LockSurface(drawArea);
	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=left; x<right; x++) {
			bit = 0x80;
			vramptr = vram_bdptr;

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
		       __SETDOT((x<<3)+0  ,y ,rgbTTLGDI[c7]);
		       __SETDOT((x<<3)+1  ,y ,rgbTTLGDI[c6]);
  		       __SETDOT((x<<3)+2  ,y ,rgbTTLGDI[c5]);
   		       __SETDOT((x<<3)+3  ,y ,rgbTTLGDI[c4]);
   		       __SETDOT((x<<3)+4  ,y ,rgbTTLGDI[c3]);
   		       __SETDOT((x<<3)+5  ,y ,rgbTTLGDI[c2]);
   		       __SETDOT((x<<3)+6  ,y ,rgbTTLGDI[c1]);
   		       __SETDOT((x<<3)+7  ,y ,rgbTTLGDI[c0]);

		}
	}
        //        SDL_UnlockSurface(displayArea);
        SDL_UnlockSurface(drawArea);

}

/*
 *	640x400、デジタルモード
 *	描画
 */
static void FASTCALL Draw400l(void)
{
	WORD wdtop, wdbtm;

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
		/* ウィンドウオープン時 */
		if (window_open) {
			/* ウィンドウ外 上側の描画 */
			if (nDrawTop < window_dy1) {
				Draw400lSub(nDrawTop, window_dy1);
			}

			/* ウィンドウ内の描画 */
			if (nDrawTop > window_dy1) {
				wdtop = nDrawTop;
			}
			else {
				wdtop = window_dy1;
			}

			if (nDrawBottom < window_dy2) {
				wdbtm = nDrawBottom;
			}
			else {
				wdbtm = window_dy2;
			}

			if (wdbtm > wdtop) {
				Draw400lWSub(wdtop, wdbtm, window_dx1, window_dx2);
			}

			/* ウィンドウ外 下側の描画 */
			if (nDrawBottom > window_dy2) {
				Draw400lSub(window_dy2, nDrawBottom);
			}
		}
		else {
			Draw400lSub(nDrawTop, nDrawBottom);
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
#endif

/*
 *	320x200、アナログモード
 *	パレット設定
 */
static void FASTCALL Palet320()
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
static void FASTCALL Draw320Sub(int top, int bottom)
{
	int x, y;
	int offset;
	int i;
	BYTE bit;
	BYTE *vramptr;
        BYTE b[8], r[8], g[8];
        Uint32 dat[8];
        //        SDL_LockSurface(displayArea);
        SDL_LockSurface(drawArea);

	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=nDrawLeft>>4; x<nDrawRight>>4; x++) {
			bit = 0x80;
			vramptr = vram_dptr;

			/* オフセット設定 */
			offset = 40 * y + x;


#if 0 /* XM7V.2以前ではVRAM配列が異なる */
				/* G評価 */
				if (vramptr[offset + 0x08000] & bit) {
					dat |= 0x800;
				}
				if (vramptr[offset + 0x0a000] & bit) {
					dat |= 0x400;
				}
				if (vramptr[offset + 0x14000] & bit) {
					dat |= 0x200;
				}
				if (vramptr[offset + 0x16000] & bit) {
					dat |= 0x100;
				}

				/* R評価 */
				if (vramptr[offset + 0x04000] & bit) {
					dat |= 0x080;
				}
				if (vramptr[offset + 0x06000] & bit) {
					dat |= 0x040;
				}
				if (vramptr[offset + 0x10000] & bit) {
					dat |= 0x020;
				}
				if (vramptr[offset + 0x12000] & bit) {
					dat |= 0x010;
				}

				/* B評価 */
				if (vramptr[offset + 0x00000] & bit) {
					dat |= 0x008;
				}
				if (vramptr[offset + 0x02000] & bit) {
					dat |= 0x004;
				}
				if (vramptr[offset + 0x0c000] & bit) {
					dat |= 0x002;
				}
				if (vramptr[offset + 0x0e000] & bit) {
					dat |= 0x001;
				}


                                                                    /* アナログパレットよりデータ取得 */
				__SETDOT_320((x<<3)+i  ,y<<1 ,rgbAnalogGDI[dat]);
				/* 次のビットへ */
#endif


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
                          
                          /* bit6 */
                          dat[6] = ((b[0] & 0x02)>>1) + ((b[1] & 0x02)) + ((b[2] & 0x02)<<1) + ((b[3] & 0x02)<<2) 
                              + ((r[0] & 0x02)<<3) + ((r[1] & 0x02)<<4) + ((r[2] & 0x02)<<5) + ((r[3] & 0x02)<<6)
                              + ((g[0] & 0x02)<<7) + ((g[1] & 0x02)<<8) + ((g[2] & 0x02)<<9) + ((g[3] & 0x02)<<10);
                          

                          /* bit5 */
                          dat[5] = ((b[0] & 0x04)>>2) + ((b[1] & 0x04)>>1) + ((b[2] & 0x04)) + ((b[3] & 0x04)<<1) 
                              + ((r[0] & 0x04)<<2) + ((r[1] & 0x04)<<3) + ((r[2] & 0x04)<<4) + ((r[3] & 0x04)<<5)
                              + ((g[0] & 0x04)<<6) + ((g[1] & 0x04)<<7) + ((g[2] & 0x04)<<8) + ((g[3] & 0x04)<<9);

                          /* bit4 */
                          dat[4] = ((b[0] & 0x08)>>3) + ((b[1] & 0x08)>>2) + ((b[2] & 0x08)>>1) + ((b[3] & 0x08)) 
                              + ((r[0] & 0x08)<<1) + ((r[1] & 0x08)<<2) + ((r[2] & 0x08)<<3) + ((r[3] & 0x08)<<4)
                              + ((g[0] & 0x08)<<5) + ((g[1] & 0x08)<<6) + ((g[2] & 0x08)<<7) + ((g[3] & 0x08)<<8);
                          
                          /* bit3 */
                          dat[3] = ((b[0] & 0x10)>>4) + ((b[1] & 0x10)>>3) + ((b[2] & 0x10)>>2) + ((b[3] & 0x10)>>1  ) 
                              + ((r[0] & 0x10)) + ((r[1] & 0x10)<<1) + ((r[2] & 0x10)<<2) + ((r[3] & 0x10)<<3)
                              + ((g[0] & 0x10)<<4) + ((g[1] & 0x10)<<5) + ((g[2] & 0x10)<<6) + ((g[3] & 0x10)<<7);

                          /* bit2 */
                          dat[2] = ((b[0] & 0x20)>>5) + ((b[1] & 0x20)>>4) + ((b[2] & 0x20)>>3) + ((b[3] & 0x20)>>2) 
                              + ((r[0] & 0x20)>>1) + ((r[1] & 0x20)) + ((r[2] & 0x20)<<1) + ((r[3] & 0x20)<<2)
                              + ((g[0] & 0x20)<<3) + ((g[1] & 0x20)<<4) + ((g[2] & 0x20)<<5) + ((g[3] & 0x20)<<6);

                          /* bit1 */
                          dat[1] = ((b[0] & 0x40)>>6) + ((b[1] & 0x40)>>5) + ((b[2] & 0x40)>>4) + ((b[3] & 0x40)>>3) 
                              + ((r[0] & 0x40)>>2) + ((r[1] & 0x40)>>1) + ((r[2] & 0x40)) + ((r[3] & 0x40)<<1)
                              + ((g[0] & 0x40)<<2) + ((g[1] & 0x40)<<3) + ((g[2] & 0x40)<<4) + ((g[3] & 0x40)<<5);

                          /* bit0 */
                          dat[0] = ((b[0] & 0x80)>>7) + ((b[1] & 0x80)>>6) + ((b[2] & 0x80)>>5) + ((b[3] & 0x80)>>4) 
                              + ((r[0] & 0x80)>>3) + ((r[1] & 0x80)>>2) + ((r[2] & 0x80)>>1) + ((r[3] & 0x80))
                              + ((g[0] & 0x80)<<1) + ((g[1] & 0x80)<<2) + ((g[2] & 0x80)<<3) + ((g[3] & 0x80)<<4);
                          
                          if(bFullScan) { 
                            __SETDOT_320p((x<<3)+7  ,y<<1 ,rgbAnalogGDI[dat[7]]);
                            __SETDOT_320p((x<<3)+6  ,y<<1 ,rgbAnalogGDI[dat[6]]);
                            __SETDOT_320p((x<<3)+5  ,y<<1 ,rgbAnalogGDI[dat[5]]);
                            __SETDOT_320p((x<<3)+4  ,y<<1 ,rgbAnalogGDI[dat[4]]);
                            __SETDOT_320p((x<<3)+3  ,y<<1 ,rgbAnalogGDI[dat[3]]);
                            __SETDOT_320p((x<<3)+2  ,y<<1 ,rgbAnalogGDI[dat[2]]);
                            __SETDOT_320p((x<<3)+1  ,y<<1 ,rgbAnalogGDI[dat[1]]);
                            __SETDOT_320p((x<<3)+0  ,y<<1 ,rgbAnalogGDI[dat[0]]);
                          } else {
                            __SETDOT_320i((x<<3)+7  ,y<<1 ,rgbAnalogGDI[dat[7]]);
                            __SETDOT_320i((x<<3)+6  ,y<<1 ,rgbAnalogGDI[dat[6]]);
                            __SETDOT_320i((x<<3)+5  ,y<<1 ,rgbAnalogGDI[dat[5]]);
                            __SETDOT_320i((x<<3)+4  ,y<<1 ,rgbAnalogGDI[dat[4]]);
                            __SETDOT_320i((x<<3)+3  ,y<<1 ,rgbAnalogGDI[dat[3]]);
                            __SETDOT_320i((x<<3)+2  ,y<<1 ,rgbAnalogGDI[dat[2]]);
                            __SETDOT_320i((x<<3)+1  ,y<<1 ,rgbAnalogGDI[dat[1]]);
                            __SETDOT_320i((x<<3)+0  ,y<<1 ,rgbAnalogGDI[dat[0]]);
                          }




                }
                }
        //SDL_UnlockSurface(displayArea);
        SDL_UnlockSurface(drawArea);

}

#if XM7_VER >= 3
/*
 *	320x200、アナログモード
 *	ウィンドウ内描画用サブ
 */
static void FASTCALL Draw320WSub(int top, int bottom, int left, int right)
{
	int x, y;
	int offset;
	int i;
	Uint32 dat[8];
	BYTE bit;
	BYTE *vramptr;
                 BYTE b[4], r[4], g[4];

                 //SDL_LockSurface(displayArea);
                 SDL_LockSurface(drawArea);
	/* yループ */
	for (y=top; y<bottom; y++) {

		/* xループ */
		for (x=left; x<right; x++) {
			bit = 0x80;
			vramptr = vram_bdptr;

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

                          /* bit6 */
                          dat[6] = ((b[0] & 0x02)>>1) + ((b[1] & 0x02)) + ((b[2] & 0x02)<<1) + ((b[3] & 0x02)<<2) 
                              + ((r[0] & 0x02)<<3) + ((r[1] & 0x02)<<4) + ((r[2] & 0x02)<<5) + ((r[3] & 0x02)<<6)
                              + ((g[0] & 0x02)<<7) + ((g[1] & 0x02)<<8) + ((g[2] & 0x02)<<9) + ((g[3] & 0x02)<<10);

                          /* bit5 */
                          dat[5] = ((b[0] & 0x04)>>2) + ((b[1] & 0x04)>>1) + ((b[2] & 0x04)) + ((b[3] & 0x04)<<1) 
                              + ((r[0] & 0x04)<<2) + ((r[1] & 0x04)<<3) + ((r[2] & 0x04)<<4) + ((r[3] & 0x04)<<5)
                              + ((g[0] & 0x04)<<6) + ((g[1] & 0x04)<<7) + ((g[2] & 0x04)<<8) + ((g[3] & 0x04)<<9);

                          /* bit4 */
                          dat[4] = ((b[0] & 0x08)>>3) + ((b[1] & 0x08)>>2) + ((b[2] & 0x08)>>1) + ((b[3] & 0x08)) 
                              + ((r[0] & 0x08)<<1) + ((r[1] & 0x08)<<2) + ((r[2] & 0x08)<<3) + ((r[3] & 0x08)<<4)
                              + ((g[0] & 0x08)<<5) + ((g[1] & 0x08)<<6) + ((g[2] & 0x08)<<7) + ((g[3] & 0x08)<<8);

                          /* bit3 */
                          dat[3] = ((b[0] & 0x10)>>4) + ((b[1] & 0x10)>>3) + ((b[2] & 0x10)>>2) + ((b[3] & 0x10)>>1  ) 
                              + ((r[0] & 0x10)) + ((r[1] & 0x10)<<1) + ((r[2] & 0x10)<<2) + ((r[3] & 0x10)<<3)
                              + ((g[0] & 0x10)<<4) + ((g[1] & 0x10)<<5) + ((g[2] & 0x10)<<6) + ((g[3] & 0x10)<<7);

                          /* bit2 */
                          dat[2] = ((b[0] & 0x20)>>5) + ((b[1] & 0x20)>>4) + ((b[2] & 0x20)>>3) + ((b[3] & 0x20)>>2) 
                              + ((r[0] & 0x20)>>1) + ((r[1] & 0x20)) + ((r[2] & 0x20)<<1) + ((r[3] & 0x20)<<2)
                              + ((g[0] & 0x20)<<3) + ((g[1] & 0x20)<<4) + ((g[2] & 0x20)<<5) + ((g[3] & 0x20)<<6);
                          
                          /* bit1 */
                          dat[1] = ((b[0] & 0x40)>>6) + ((b[1] & 0x40)>>5) + ((b[2] & 0x40)>>4) + ((b[3] & 0x40)>>3) 
                              + ((r[0] & 0x40)>>2) + ((r[1] & 0x40)>>1) + ((r[2] & 0x40)) + ((r[3] & 0x40)<<1)
                              + ((g[0] & 0x40)<<2) + ((g[1] & 0x40)<<3) + ((g[2] & 0x40)<<4) + ((g[3] & 0x40)<<5);
                          

                          /* bit0 */
                          dat[0] = ((b[0] & 0x80)>>7) + ((b[1] & 0x80)>>6) + ((b[2] & 0x80)>>5) + ((b[3] & 0x80)>>4) 
                              + ((r[0] & 0x80)>>3) + ((r[1] & 0x80)>>2) + ((r[2] & 0x80)>>1) + ((r[3] & 0x80))
                              + ((g[0] & 0x80)<<1) + ((g[1] & 0x80)<<2) + ((g[2] & 0x80)<<3) + ((g[3] & 0x80)<<4);

                          if(bFullScan) { 
                            __SETDOT_320p((x<<3)+7  ,y<<1 ,rgbAnalogGDI[dat[7]]);
                            __SETDOT_320p((x<<3)+6  ,y<<1 ,rgbAnalogGDI[dat[6]]);
                            __SETDOT_320p((x<<3)+5  ,y<<1 ,rgbAnalogGDI[dat[5]]);
                            __SETDOT_320p((x<<3)+4  ,y<<1 ,rgbAnalogGDI[dat[4]]);
                            __SETDOT_320p((x<<3)+3  ,y<<1 ,rgbAnalogGDI[dat[3]]);
                            __SETDOT_320p((x<<3)+2  ,y<<1 ,rgbAnalogGDI[dat[2]]);
                            __SETDOT_320p((x<<3)+1  ,y<<1 ,rgbAnalogGDI[dat[1]]);
                            __SETDOT_320p((x<<3)+0  ,y<<1 ,rgbAnalogGDI[dat[0]]);
                          } else {
                            __SETDOT_320i((x<<3)+7  ,y<<1 ,rgbAnalogGDI[dat[7]]);
                            __SETDOT_320i((x<<3)+6  ,y<<1 ,rgbAnalogGDI[dat[6]]);
                            __SETDOT_320i((x<<3)+5  ,y<<1 ,rgbAnalogGDI[dat[5]]);
                            __SETDOT_320i((x<<3)+4  ,y<<1 ,rgbAnalogGDI[dat[4]]);
                            __SETDOT_320i((x<<3)+3  ,y<<1 ,rgbAnalogGDI[dat[3]]);
                            __SETDOT_320i((x<<3)+2  ,y<<1 ,rgbAnalogGDI[dat[2]]);
                            __SETDOT_320i((x<<3)+1  ,y<<1 ,rgbAnalogGDI[dat[1]]);
                            __SETDOT_320i((x<<3)+0  ,y<<1 ,rgbAnalogGDI[dat[0]]);
                          }
		}
	}
        //SDL_UnlockSurface(displayArea);
        SDL_UnlockSurface(drawArea);
}
#endif

/*
 *	320x200、アナログモード
 *	描画
 */
static void FASTCALL Draw320(void)
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


#if XM7_VER >= 3
/*
 *	320x200、26万色モード
 *	描画
 */
static void FASTCALL Draw256k(void)
{
	int x, y;
	int offset;
	int i;
	BYTE bit;
	DWORD color;

	/* クリア処理 */
	if (bClearFlag) {
		AllClear();
	}

	/* レンダリング */
	if (nDrawTop >= nDrawBottom) {
		return;
	}
        //SDL_LockSurface(displayArea);
        SDL_LockSurface(drawArea);
	/* yループ */
	for (y=nDrawTop >> 1; y<nDrawBottom >> 1; y++) {

		/* xループ */
		for (x=nDrawLeft>>4; x<nDrawRight>>4; x++) {
			bit = 0x80;

			/* オフセット設定 */
			offset = 40 * y + x;

			/* ビットループ */
			for (i=0; i<8; i++) {
				color = 0;

				if (!(multi_page & 0x40)) {
					/* G評価 */
					if (vram_c[offset + 0x10000] & bit) {
						color |= 0x008300;
					}
					if (vram_c[offset + 0x12000] & bit) {
						color |= 0x004300;
					}
					if (vram_c[offset + 0x14000] & bit) {
						color |= 0x002300;
					}
					if (vram_c[offset + 0x16000] & bit) {
						color |= 0x001300;
					}
					if (vram_c[offset + 0x28000] & bit) {
						color |= 0x000b00;
					}
					if (vram_c[offset + 0x2a000] & bit) {
						color |= 0x000700;
					}
				}

				if (!(multi_page & 0x20)) {
					/* R評価 */
					if (vram_c[offset + 0x08000] & bit) {
						color |= 0x830000;
					}
					if (vram_c[offset + 0x0a000] & bit) {
						color |= 0x430000;
					}
					if (vram_c[offset + 0x0c000] & bit) {
						color |= 0x230000;
					}
					if (vram_c[offset + 0x0e000] & bit) {
						color |= 0x130000;
					}
					if (vram_c[offset + 0x20000] & bit) {
						color |= 0x0b0000;
					}
					if (vram_c[offset + 0x22000] & bit) {
						color |= 0x070000;
					}
				}

				if (!(multi_page & 0x10)) {
					/* B評価 */
					if (vram_c[offset + 0x00000] & bit) {
						color |= 0x000083;
					}
					if (vram_c[offset + 0x02000] & bit) {
						color |= 0x000043;
					}
					if (vram_c[offset + 0x04000] & bit) {
						color |= 0x000023;
					}
					if (vram_c[offset + 0x06000] & bit) {
						color |= 0x000013;
					}
					if (vram_c[offset + 0x18000] & bit) {
						color |= 0x00000b;
					}
					if (vram_c[offset + 0x1a000] & bit) {
						color |= 0x000007;
					}
				}

				/* CRTフラグ */
				if (!crt_flag) {
					color = 0;
				}
				__SETDOT_320p((x<<3)+i  ,y<<1 ,color);

				/* 次のビットへ */
				bit >>= 1;
			}
		}
	}
        //SDL_UnlockSurface(displayArea);
        SDL_UnlockSurface(drawArea);
	if (bFullScan) {
		RenderFullScan();
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
#endif

/*
 *	描画(通常)
 */
void FASTCALL OnDraw(void)
{
	/* 640-320 自動切り替え */
	SelectDraw();

#if XM7_VER >= 3
	/* いずれかを使って描画 */
	switch (bMode) {
		case SCR_400LINE	:	Draw400l();
								break;
		case SCR_262144		:	Draw256k();
								break;
		case SCR_4096		:	Draw320();
								break;
		case SCR_200LINE	:	Draw640();
								break;
	}
#else
	/* どちらかを使って描画 */
	if (bAnalog) {
		Draw320();
	}
	else {
		Draw640();
	}
#endif
}

/*
 * 描画(PAINT) *GTK依存だが、ダミー。
 */
gint FASTCALL OnPaint(GtkWidget *widget, GdkEventExpose *event)
{
        SDL_Rect srcrect,dstrect;
        srcrect.x = 0;
        srcrect.y = 0;
        srcrect.w = (Uint16)drawArea->w;
        srcrect.h = (Uint16)drawArea->h;
   
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = (Uint16)displayArea->w;
        dstrect.h = (Uint16)displayArea->h;
	

        SDL_BlitSurface(drawArea ,&srcrect ,displayArea ,&dstrect);

        SDL_UpdateRect(drawArea ,0 ,0 ,drawArea->w ,drawArea->h);
        SDL_UpdateRect(displayArea ,0 ,0 ,displayArea->w ,displayArea->h);
        printf("paint");
	return FALSE;
}

/*-[ VMとの接続 ]-----------------------------------------------------------*/

/*
 *	VRAMセット
 */
void FASTCALL vram_notify(WORD addr, BYTE dat)
{
	WORD x;
	WORD y;

	/* y座標算出 */
#if XM7_VER >= 3
	switch (bMode) {
		case SCR_400LINE	:	addr &= 0x7fff;
								x = (WORD)((addr % 80) << 3);
								y = (WORD)(addr / 80);
								break;
		case SCR_262144		:
		case SCR_4096		:	addr &= 0x1fff;
								x = (WORD)((addr % 40) << 4);
								y = (WORD)((addr / 40) << 1);
								break;
		case SCR_200LINE	:	addr &= 0x3fff;
								x = (WORD)((addr % 80) << 3);
								y = (WORD)((addr / 80) << 1);
								break;
	}
#else
	if (bAnalog) {
		addr &= 0x1fff;
		x = (WORD)((addr % 40) << 4);
		y = (WORD)((addr / 40) << 1);
	}
	else {
		addr &= 0x3fff;
		x = (WORD)((addr % 80) << 3);
		y = (WORD)((addr / 80) << 1);
	}
#endif

	/* オーバーチェック */
	if ((x >= 640) || (y >= 400)) {
		return;
	}

	/* 再描画フラグを設定 */
	GDIDrawFlag[(y >> 3) * 80 + (x >> 3)] = 1;

	/* 垂直方向更新 */
	if (nDrawTop > y) {
		nDrawTop = y;
	}
	if (nDrawBottom <= y) {
#if XM7_VER >= 3
		if (bMode == SCR_400LINE) {
			nDrawBottom = (WORD)(y + 1);
		}
		else {
			nDrawBottom = (WORD)(y + 2);
		}
#else
		nDrawBottom = (WORD)(y + 2);
#endif
	}

	/* 水平方向更新 */
	if (nDrawLeft > x) {
		nDrawLeft = x;
	}
	if (nDrawRight <= x) {
#if XM7_VER >= 3
		if (bMode & SCR_ANALOG) {
#else
		if (bAnalog) {
#endif
			nDrawRight = (WORD)(x + 16);
		}
		else {
			nDrawRight = (WORD)(x + 8);
		}
	}
}

/*
 *	TTLパレットセット
 */
void FASTCALL ttlpalet_notify(void)
{
	/* 不要なレンダリングを抑制するため、領域設定は描画時に行う */
	bPaletFlag = TRUE;
}

/*
 *	アナログパレットセット
 */
void FASTCALL apalet_notify(void)
{
	bPaletFlag = TRUE;
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	SetDrawFlag(TRUE);
}

/*
 *	再描画要求
 */
void FASTCALL display_notify(void)
{
	/* 再描画 */
	nDrawTop = 0;
	nDrawBottom = 400;
	nDrawLeft = 0;
	nDrawRight = 640;
	bPaletFlag = TRUE;
	bClearFlag = TRUE;
	SetDrawFlag(TRUE);
}

/*
 *      ディジタイズ要求通知
 */
void FASTCALL digitize_notify(void)
{
}

#if XM7_VER >= 3
/*
 *	ハードウェアウィンドウ通知
 */
void FASTCALL window_notify(void)
{
	WORD tmpLeft, tmpRight;
	WORD tmpTop, tmpBottom;
	WORD tmpDx1, tmpDx2;
	WORD tmpDy1, tmpDy2;
	BYTE *p;
	int i;

	/* 26万色モード時は何もしない */
	if (bMode == SCR_262144) {
		return;
	}

	/* 前もってクリッピングする */
	window_clip(bMode);

	/* ウィンドウサイズを補正 */
	tmpDx1 = window_dx1;
	tmpDy1 = window_dy1;
	tmpDx2 = window_dx2;
	tmpDy2 = window_dy2;
	if (bMode != SCR_400LINE) {
		tmpDy1 <<= 1;
		tmpDy2 <<= 1;
	}
	if (bMode == SCR_4096) {
		tmpDx1 <<= 1;
		tmpDx2 <<= 1;
	}

	if (bWindowOpen != window_open) {
		if (window_open) {
			/* ウィンドウを開いた場合 */
			tmpLeft = tmpDx1;
			tmpRight = tmpDx2;
			tmpTop = tmpDy1;
			tmpBottom = tmpDy2;
		}
		else {
			/* ウィンドウを閉じた場合 */
			tmpLeft = nWindowDx1;
			tmpRight = nWindowDx2;
			tmpTop = nWindowDy1;
			tmpBottom = nWindowDy2;
		}
	}
	else {
		if (window_open) {
			/* 更新領域サイズを現在のものに設定 */
			tmpTop = nDrawTop;
			tmpBottom = nDrawBottom;
			tmpLeft = nDrawLeft;
			tmpRight = nDrawRight;

			/* 座標変更チェック */
			if (!((nWindowDx1 == tmpDx1) &&
				  (nWindowDy1 == tmpDy1) &&
				  (nWindowDx2 == tmpDx2) &&
				  (nWindowDy2 == tmpDy2))) {
				/* 左上X */
				if (nWindowDx1 < tmpDx1) {
					tmpLeft = nWindowDx1;
				}
				else {
					tmpLeft = tmpDx1;
				}

				/* 右下X */
				if (nWindowDx2 > tmpDx2) {
					tmpRight = nWindowDx2;
				}
				else {
					tmpRight = tmpDx2;
				}

				/* 左上Y */
				if (nWindowDy1 < tmpDy1) {
					tmpTop = nWindowDy1;
				}
				else {
					tmpTop = tmpDy1;
				}

				/* 右下Y */
				if (nWindowDy2 > tmpDy2) {
					tmpBottom = nWindowDy2;
				}
				else {
					tmpBottom = tmpDy2;
				}
			}
		}
		else {
			/* ウィンドウが開いていないので何もしない */
			return;
		}
	}

	/* 処理前の再描画領域と比較して広ければ領域を更新 */
	if (tmpLeft < nDrawLeft) {
		nDrawLeft = tmpLeft;
	}
	if (tmpRight > nDrawRight) {
		nDrawRight = tmpRight;
	}
	if (tmpTop < nDrawTop) {
		nDrawTop = tmpTop;
	}
	if (tmpBottom > nDrawBottom) {
		nDrawBottom = tmpBottom;
	}

	/* 再描画フラグを更新 */
	if ((nDrawLeft < nDrawRight) && (nDrawTop < nDrawBottom)) {
		p = &GDIDrawFlag[(nDrawTop >> 3) * 80 + (nDrawLeft >> 3)];
		for (i = (nDrawTop >> 3); i < ((nDrawBottom + 7) >> 3) ; i++) {
			memset(p, 1, (nDrawRight - nDrawLeft) >> 3);
			p += 80;
		}
	}

	/* ウィンドウオープン状態を保存 */
	bWindowOpen = window_open;
	nWindowDx1 = tmpDx1;
	nWindowDy1 = tmpDy1;
	nWindowDx2 = tmpDx2;
	nWindowDy2 = tmpDy2;
}
#endif

 void OnFullScreen(void)
 {
   SDL_WM_ToggleFullScreen(displayArea);
   
 }

 void OnWindowedScreen(void)
 {
   SDL_WM_ToggleFullScreen(displayArea);

 }

#endif	/* _XWIN */

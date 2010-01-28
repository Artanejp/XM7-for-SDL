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
#if defined(USE_OPENGL)
#include <SDL/SDL_opengl.h>
#endif /* USE_OPENGL */

/*
 *	グローバル ワーク
 */
DWORD rgbTTLGDI[16];						/* デジタルパレット */
DWORD rgbAnalogGDI[4096];					/* アナログパレット */
//guchar pBitsGDI[400*640*3];					/* ビットデータ */
BYTE GDIDrawFlag[80*50]; /* 8x8ドットのメッシュを作る */						/* 8x8 再描画領域フラグ */
BOOL bFullScan;								/* フルスキャン(Window) */
BOOL bDirectDraw; /* 直接書き込みフラグ */
SDL_Surface *realDrawArea; /* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する) */
extern GtkWidget *gtkDrawArea;

/*
 *	スタティック ワーク
 */
#if XM7_VER >= 3

static BYTE bMode;							/* 画面モード */
#else
static BOOL bAnalog;						/* アナログモードフラグ */
#endif
static BYTE bNowBPP;						/* 現在のビット深度 */
WORD nDrawTop;						/* 描画範囲上 */
WORD nDrawBottom;					/* 描画範囲下 */
WORD nDrawLeft;						/* 描画範囲左 */
WORD nDrawRight;						/* 描画範囲右 */
WORD nDrawWidth;
WORD nDrawHeight;
BOOL bPaletFlag;						/* パレット変更フラグ */
BOOL bClearFlag;
						/* クリアフラグ */
static WORD nOldDrawWidth;
static WORD nOldDrawHeight;
#if XM7_VER >= 3
static BOOL bWindowOpen;					/* ハードウェアウィンドウ状態 */
static WORD nWindowDx1;						/* ウィンドウ左上X座標 */
static WORD nWindowDy1;						/* ウィンドウ左上Y座標 */
static WORD nWindowDx2;						/* ウィンドウ右下X座標 */
static WORD nWindowDy2;						/* ウィンドウ右下Y座標 */
#endif
#if defined(USE_OPENGL)
GLuint src_texture = 0;
#endif

/*
 *	プロトタイプ宣言
 */
extern void Draw640(void);
extern void Draw320(void);
extern void Palet320();
extern void Palet640(void);
extern void Draw400l(void);
extern void Draw256k(void);
void SetDrawFlag(BOOL flag);


/*
 * SETDOT（そのまま）
 */
static void SETDOT(WORD x, WORD y, DWORD c)
{
  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * displayArea->pitch + x * displayArea->format->BytesPerPixel;
   Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * realDrawArea->pitch + x * realDrawArea->format->BytesPerPixel;
        SDL_LockSurface(realDrawArea);
        *(DWORD *)addr = c;
        SDL_UnlockSurface(realDrawArea);
   
} 




/*
 * SETDOT（inline） 拡大モード
 * 32bpp前提,SurfaceLockしません!!
 */
static inline void __SETDOT_DOUBLE(WORD x, WORD y, DWORD c)
{
  //Uint8 *addr = (Uint8 *)displayArea->pixels + y * 2 * displayArea->pitch + x * 2 * displayArea->format->BytesPerPixel;
  Uint8 *addr = (Uint8 *)realDrawArea->pixels + y * 2 * realDrawArea->pitch + x * 2 * realDrawArea->format->BytesPerPixel;
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



#if defined(USE_OPENGL)
/*
 * BITBLT(OpenGL)...まだうごかない。SEGVする
 */
static BOOL OpenGL_BitBlt()
{

   GLuint texture[4];
   int w, h;
   GLfloat texAttr[4];
   SDL_Surface *textureArea;

   displayArea = SDL_GetVideoSurface();
#if 0
   textureArea = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                      w, h,
   w = 1024; /* 640x400... w,h は2^nであること。 */
   h = 512; 
                                      32,
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN /* OpenGL RGBA masks */
			0x000000FF, 
			0x0000FF00, 
			0x00FF0000, 
			0xFF000000
#else
			0xFF000000,
			0x00FF0000, 
			0x0000FF00, 
			0x000000FF
#endif
		       );
#endif 
   /*
    * OpenGLでは描画した画面をテクスチャとして扱う
    */
   texAttr[0] = 0.0f; /* X始点 */
   texAttr[1] = 0.0f; /* Y始点 */
   texAttr[2] = (GLfloat)640 / w; /* X大きさ(比率) */
   texAttr[3] = (GLfloat)400 / h; /* X大きさ(比率) */
   //SDL_GL_LoadTexture(displayArea, texAttr);
   glClear(GL_COLOR_BUFFER_BIT);

   glGenTextures(1, texture);
   glBindTexture(GL_TEXTURE_2D, texture[0]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGBA,
                w, h,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                realDrawArea->pixels);


   glBegin(GL_TRIANGLE_STRIP);
   glTexCoord2f(texAttr[0], texAttr[1]);
   glVertex2i(0, 0);
   glTexCoord2f(texAttr[2], texAttr[1]);
   glVertex2i(640, 0);
   glTexCoord2f(texAttr[0], texAttr[3]);
   glVertex2i(0, 400);
   glTexCoord2f(texAttr[2], texAttr[3]);
   glVertex2i(640 , 400);
   glEnd();


   SDL_GL_SwapBuffers();


}
#endif
static void ChangeResolution()
{
        /* ビデオモード再設定 */
        if((nDrawWidth != nOldDrawWidth) || (nDrawHeight != nOldDrawHeight)) {
          {
            char EnvMainWindow[64];
            SDL_Surface *tmpSurface;
            SDL_Rect srcrect,dstrect;

            /* まずは現在のサーフェイスを退避する */
            tmpSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, nOldDrawWidth, nOldDrawHeight, 24, 0, 0, 0, 0);
            displayArea = SDL_GetVideoSurface();
            srcrect.x = 0;
            srcrect.y = 0;
            srcrect.w = displayArea->w;
            srcrect.h = displayArea->h;

            dstrect.x = 0;
            dstrect.y = 0;
            dstrect.w = tmpSurface->w;
            dstrect.h = tmpSurface->h;

            if(srcrect.w >dstrect.w) srcrect.w = dstrect.w;
            if(srcrect.h >dstrect.h) srcrect.h = dstrect.h;
            SDL_BlitSurface(displayArea, &srcrect, tmpSurface, &dstrect);
            /* 表示部分のリサイズ */
            gtk_widget_set_usize(gtkDrawArea, nDrawWidth, nDrawHeight);
            sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(gtkDrawArea->window));
            SDL_putenv(EnvMainWindow);
            drawArea = SDL_SetVideoMode(nDrawWidth, nDrawHeight, 24, 
                             SDL_HWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | 0);
            printf("RESO CHG: %d x %d -> %d x %d\n", nOldDrawWidth ,nOldDrawHeight, nDrawWidth, nDrawHeight);
            /* 退避したエリアの復帰（原寸…) */
            dstrect.x = 0;
            dstrect.y = 0;
            dstrect.w = displayArea->w;
            dstrect.h = displayArea->h;

            srcrect.x = 0;
            srcrect.y = 0;
            srcrect.w = tmpSurface->w;
            srcrect.h = tmpSurface->h;

            if(srcrect.w >dstrect.w) srcrect.w = dstrect.w;
            if(srcrect.h >dstrect.h) srcrect.h = dstrect.h;
            SDL_BlitSurface(tmpSurface, &srcrect, displayArea, &dstrect);
            SDL_FreeSurface(tmpSurface);
            /* 以下に、全画面強制再描画処理を入れる */
          }
          nOldDrawHeight = nDrawHeight;
          nOldDrawWidth = nDrawWidth;
        }
}

/*
 * BITBLT
 */
BOOL BitBlt(int nDestLeft, int nDestTop, int nWidth, int nHeight, int nSrcLeft, int nSrcTop)
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
   displayArea = SDL_GetVideoSurface();
   if(!bDirectDraw) {
   SDL_UpdateRect(realDrawArea, 0, 0, realDrawArea->w, realDrawArea->h);
   }
#if defined(USE_OPENGL)
   //SDL_BlitSurface(drawArea, &srcrect ,displayArea ,&dstrect );
   OpenGL_BitBlt();
#else /* OpenGL */
   if(!bDirectDraw) {
   SDL_BlitSurface(realDrawArea, &srcrect ,displayArea ,&dstrect );
   }
   SDL_UpdateRect(displayArea, 0, 0, displayArea->w, displayArea->h);

   /* ここまでやっておいてから解像度を変更する */
   ChangeResolution();
   //printf("BitBlt %d %d\n",drawArea->w, drawArea->h);
#endif /* OpenGL */
}

#define XM7_DRAWMODE_SDL SDL_SWSURFACE
#define XM7_DRAW_WIDTH 640
#define XM7_DRAW_HEIGHT 400
#define XM7_DRAW_MAX_BPP 24 /* 32bitもやるか？遅いが */

#if SDL_BYTEORDER == SDL_BIG_ENDIAN /* BIGENDIAN */
#if XM7_DRAW_MAX_BPP == 32
#define XM7_DRAW_RMASK 0xff000000 /* R */
#define XM7_DRAW_GMASK 0x00ff0000 /* G */
#define XM7_DRAW_BMASK 0x0000ff00 /* B */
#define XM7_DRAW_AMASK 0x000000ff /* ALPHA */
#elif XM7_DRAW_MAX_BPP == 24 /* 24bit */
#define XM7_DRAW_RMASK 0x00ff0000
#define XM7_DRAW_GMASK 0x0000ff00
#define XM7_DRAW_BMASK 0x000000ff
#define XM7_DRAW_AMASK 0x00000000
#else
#define XM7_DRAW_RMASK 0x0000f000
#define XM7_DRAW_GMASK 0x00000f00
#define XM7_DRAW_BMASK 0x000000f0
#define XM7_DRAW_AMASK 0x0000000f
#endif
#else /* SDL_BYTEORDER */
#if XM7_DRAW_MAX_BPP ==32
#define XM7_DRAW_RMASK 0x000000ff
#define XM7_DRAW_GMASK 0x0000ff00
#define XM7_DRAW_BMASK 0x00ff0000
#define XM7_DRAW_AMASK 0xff000000
#elif XM7_DRAW_MAX_BPP == 24 /* 24bit */
#define XM7_DRAW_RMASK 0x00000000
#define XM7_DRAW_GMASK 0x00000000
#define XM7_DRAW_BMASK 0x00000000
#define XM7_DRAW_AMASK 0x00000000
#else /* not 32bit */
#define XM7_DRAW_RMASK 0x0000000f
#define XM7_DRAW_GMASK 0x000000f0
#define XM7_DRAW_BMASK 0x00000f00
#define XM7_DRAW_AMASK 0x0000f000
#endif
#endif


/*
 *	初期化
 */
void InitDraw(void)
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
	nOldDrawHeight = 400;
	nOldDrawWidth = 640;
	nDrawHeight = 400;
	nDrawWidth = 640;

	bPaletFlag = FALSE;
	SetDrawFlag(FALSE);

#if XM7_VER >= 3
	bWindowOpen = FALSE;
	nWindowDx1 = 640;
	nWindowDy1 = 400;
	nWindowDx2 = 0;
	nWindowDy2 = 0;
#endif
        bDirectDraw = TRUE;
        /* 直接書き込み */
        realDrawArea = SDL_GetVideoSurface();
        //realDrawArea = drawArea;
#ifdef USE_OPENGL
        //glEnable(GL_TEXTURE_2D);
#endif

        
}

/*
 *	クリーンアップ
 */
void CleanDraw(void)
{
}

/*
 *	全ての再描画フラグを設定
 */
void SetDrawFlag(BOOL flag)
{
	memset(GDIDrawFlag, (BYTE)flag, sizeof(GDIDrawFlag));
}

/*
 *  640x200、デジタルモード
 *	セレクト
 */
static BOOL Select640(void)
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
static BOOL Select400l(void)
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
static BOOL Select320(void)
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
static BOOL Select256k()
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
static BOOL SelectCheck(void)
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
BOOL SelectDraw(void)
{
        SDL_Rect rect;
	/* 一致しているかチェック */
	if (SelectCheck()) {
		return TRUE;
	}

        displayArea = SDL_GetVideoSurface();
        rect.h = displayArea->h;
        rect.w = displayArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(displayArea);
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN
        SDL_FillRect(displayArea, &rect, 0x000000ff);
#else
        SDL_FillRect(displayArea, &rect, 0xff000000);
#endif 
        SDL_UnlockSurface(displayArea);

        rect.h = realDrawArea->h;
        rect.w = realDrawArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(realDrawArea);
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN
        SDL_FillRect(realDrawArea, &rect, 0x000000ff); 
#else
        SDL_FillRect(realDrawArea, &rect, 0xff000000); 
#endif
        SDL_UnlockSurface(realDrawArea);

   
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
void AllClear(void)
{
        SDL_Rect rect;
        int i;
        for(i = 0; i< (80*50) ; i++) {
          GDIDrawFlag[i] = 0;
        }
        displayArea = SDL_GetVideoSurface();
        rect.h = displayArea->h;
        rect.w = displayArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(displayArea);
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN
        SDL_FillRect(displayArea, &rect, 0x00000000); 
#else
        SDL_FillRect(displayArea, &rect, 0x00000000); 
#endif
        SDL_UnlockSurface(displayArea);
        if(bDirectDraw) {
          realDrawArea = SDL_GetVideoSurface();
        } else {
          realDrawArea = drawArea;
        }
        rect.h = realDrawArea->h;
        rect.w = realDrawArea->w;
        rect.x = 0;
        rect.y = 0;
	/* すべてクリア */
        SDL_LockSurface(realDrawArea);
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN
        SDL_FillRect(realDrawArea, &rect, 0x00000000);
#else
        SDL_FillRect(realDrawArea, &rect, 0x00000000);
#endif
        SDL_UnlockSurface(realDrawArea);


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
void RenderFullScan(void)
{
	BYTE *p;
	BYTE *q;
	WORD u;
        Uint32 pitch;

        SDL_LockSurface(realDrawArea);
	/* ポインタ初期化 */
        p = (BYTE *)realDrawArea->pixels + nDrawTop * realDrawArea->pitch;
        pitch = realDrawArea->pitch;
        q = p + pitch;


	/* ループ */
	for (u=nDrawTop; u<nDrawBottom; u += (WORD)2) {
		memcpy(q, p, pitch);
		p += pitch *2;
		q += pitch *2;
	}
        SDL_UnlockSurface(realDrawArea);
        SDL_UpdateRect(realDrawArea, 0, 0, realDrawArea->w, realDrawArea->h);

}


/*
 *	奇数ライン設定
 */
void RenderSetOddLine(void)
{
	BYTE *p;
	WORD u;
        Uint32 pitch;

	/* ポインタ初期化 */

        p = realDrawArea->pixels + (nDrawTop + 1) * realDrawArea->pitch;
        pitch = realDrawArea->pitch;
        SDL_LockSurface(realDrawArea);

	/* ループ */
	for (u=nDrawTop; u<nDrawBottom; u += (WORD)2) {
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN
	   	memset(p, 0x00, pitch );
#else
	   	memset(p, 0x00, pitch );
#endif
		p += pitch * 2;
	}
        SDL_UnlockSurface(realDrawArea);
        SDL_UpdateRect(realDrawArea, 0, 0, realDrawArea->w, realDrawArea->h);

}










/*
 *	描画(通常)
 */
void OnDraw(void)
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
gint OnPaint(GtkWidget *widget, GdkEventExpose *event)
{
        SDL_Rect srcrect,dstrect;
        int i;
        
        displayArea = SDL_GetVideoSurface();
        for(i = 0; i< (80*50) ; i++) {
          GDIDrawFlag[i] = 1;
        }
        

        srcrect.x = 0;
        srcrect.y = 0;
        srcrect.w = (Uint16)realDrawArea->w;
        srcrect.h = (Uint16)realDrawArea->h;
   
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = (Uint16)displayArea->w;
        dstrect.h = (Uint16)displayArea->h;
	

        SDL_UpdateRect(realDrawArea ,0 ,0 ,realDrawArea->w ,realDrawArea->h);
        if(!bDirectDraw) {
        SDL_BlitSurface(realDrawArea ,&srcrect ,displayArea ,&dstrect);
        }
        SDL_UpdateRect(displayArea ,0 ,0 ,displayArea->w ,displayArea->h);
        //printf("paint");
         return FALSE;
}

/*-[ VMとの接続 ]-----------------------------------------------------------*/

/*
 *	VRAMセット
 */
void vram_notify(WORD addr, BYTE dat)
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
void ttlpalet_notify(void)
{
	/* 不要なレンダリングを抑制するため、領域設定は描画時に行う */
	bPaletFlag = TRUE;
}

/*
 *	アナログパレットセット
 */
void apalet_notify(void)
{
	bPaletFlag = TRUE;
	nDrawTop = 0;
        //	nDrawBottom = 400;
        	nDrawLeft = 0;
        //nDrawRight = 640;
	SetDrawFlag(TRUE);
}

/*
 * 似非フルスクリーン(GTK)。
 */
void fullscreen_notify_gtk(void)
{
        char EnvMainWindow[64];
        /* Test Code */
           gtk_widget_set_usize(gtkDrawArea, 1280, 800);

        sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(gtkDrawArea->window));
        SDL_putenv(EnvMainWindow);

        SDL_SetVideoMode(1280,800,24, 
	    		  SDL_HWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | 0);
}

/*
 * 似非フルスクリーン(GTK)。
 */
void standard_notify_gtk(void)
{
        char EnvMainWindow[64];
        /* Test Code */
           gtk_widget_set_usize(gtkDrawArea, 640, 400);

        sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(gtkDrawArea->window));
        SDL_putenv(EnvMainWindow);

        SDL_SetVideoMode(640,400,24, 
	    		  SDL_HWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | 0);
}

   /*
 * 似非フルスクリーン(GTK)。
 */
void half_notify_gtk(void)
{
        char EnvMainWindow[64];
        /* Test Code */
           gtk_widget_set_usize(gtkDrawArea, 320, 200);

        sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(gtkDrawArea->window));
        SDL_putenv(EnvMainWindow);

        SDL_SetVideoMode(320,200,24, 
	    		  SDL_HWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | 0);
}

   
/*
 *	再描画要求
 */
void display_notify(void)
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
void digitize_notify(void)
{
}

#if XM7_VER >= 3
/*
 *	ハードウェアウィンドウ通知
 */
void window_notify(void)
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
   displayArea = SDL_GetVideoSurface();
   SDL_WM_ToggleFullScreen(displayArea);
   
 }

 void OnWindowedScreen(void)
 {
   displayArea = SDL_GetVideoSurface();
   SDL_WM_ToggleFullScreen(displayArea);
 }

#endif	/* _XWIN */

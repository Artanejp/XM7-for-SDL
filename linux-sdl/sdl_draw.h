/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta
 *  [ SDL 表示 ] 
 */  
    

    
#ifndef _sdl_draw_h_
#define _sdl_draw_h_
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
    
	/*
	 *  主要エントリ 
	 */ 
    void            InitDraw(void);
                   
	/*
	 * 初期化 
	 */            
    void            CleanDraw(void);
                   
	/*
	 * クリーンアップ 
	 */            
                    BOOL SelectDraw(void);
                   
	/*
	 * セレクト 
	 */            
    void            OnDraw(void);
                   
	/*
	 * 描画 
	 */            
	
#ifdef USE_GTK	
        gint OnPaint(GtkWidget * widget, GdkEventExpose * event);
#else
        int  OnPaint(void);
#endif                   
	/*
	 * 再描画 
	 */            
        void    OnFullScreen(void);
        void    OnWindowedScreen(void);
                
	/*
	 *  主要ワーク 
	 */            
        extern WORD  nDrawWidth;
        extern WORD  nDrawHeight;
        extern DWORD    rgbTTLGDI[16];	/* デジタルパレット */
        extern DWORD    rgbAnalogGDI[4096];	/* アナログパレット */
        extern BYTE     GDIDrawFlag[4000];	/* 8x8 再描画領域フラグ */
        extern BOOL     bFullScan;	/* フルスキャン(Window) */
        extern BOOL     bDirectDraw;	/* 直接書き込みフラグ */
        extern SDL_Surface *realDrawArea;	/* 実際に書き込むSurface(DirectDrawやOpenGLを考慮する)  */
                   
        extern WORD     nDrawTop;	/* 描画範囲上 */
        extern WORD     nDrawBottom;	/* 描画範囲下 */
        extern WORD     nDrawLeft;	/* 描画範囲左 */
        extern WORD     nDrawRight;	/* 描画範囲右 */
        extern BOOL     bPaletFlag;	/* パレット変更フラグ */
        extern BOOL     bClearFlag;	/* クリアフラグ */
	/*
	 * フルスキャン(Window) 
	 */            
#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _sdl_draw_h_ */


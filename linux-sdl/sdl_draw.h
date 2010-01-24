/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN 表示 ]
 */

#ifdef _XWIN

#ifndef _xw_draw_h_
#define _xw_draw_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	主要エントリ
 */
void FASTCALL InitDraw(void);
										/* 初期化 */
void FASTCALL CleanDraw(void);
										/* クリーンアップ */
BOOL FASTCALL SelectDraw(void);
										/* セレクト */
void FASTCALL OnDraw(void);
										/* 描画 */
gint FASTCALL OnPaint(GtkWidget *widget, GdkEventExpose *event);
										/* 再描画 */
  void OnFullScreen(void);
  void OnWindowedScreen(void);



/*
 *	主要ワーク
 */
extern BOOL bFullScan;
										/* フルスキャン(Window) */
#ifdef __cplusplus
}
#endif

#endif	/* _xw_draw_h_ */
#endif	/* _XWIN */

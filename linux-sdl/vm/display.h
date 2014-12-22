/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2012 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2012 Ryu Takegami
 *
 *      [ ディスプレイ ]
 */

#ifndef _display_h_
#define _display_h_

/*
 *      画面モード定義
 */
#define	SCR_200LINE	0x00
#define	SCR_4096	0x01
#define	SCR_400LINE	0x02
#define	SCR_262144	0x03
#define	SCR_ANALOG	0x01
#define	SCR_AV40	0x02

#ifdef __cplusplus
extern "C"
{
#endif
	/*
	 *      主要エントリ
	 */
	BOOL FASTCALL display_init(void);
	/*
	 * 初期化
	 */
	void FASTCALL display_cleanup(void);
	/*
	 * クリーンアップ
	 */
	void FASTCALL display_reset(void);
	/*
	 * リセット
	 */
	BOOL FASTCALL display_readb(WORD addr, BYTE * dat);
	/*
	 * メモリ読み出し
	 */
	BOOL FASTCALL display_writeb(WORD addr, BYTE dat);
	/*
	 * メモリ書き込み
	 */
	BOOL FASTCALL display_save(SDL_RWops * fileh);
	/*
	 * セーブ
	 */
	BOOL FASTCALL display_load(SDL_RWops * fileh, int ver);
	/*
	 * ロード
	 */

	void FASTCALL display_setpointer(BOOL redraw);
	/*
	 * 関連ワークの再設定
	 */
#if XM7_VER >= 3
	void FASTCALL window_clip(int mode);
	/*
	 * ウインドウクリッピング
	 */
	BOOL FASTCALL fix_vram_address(void);
	/*
	 * 400ラインモード用VRAM配置補正
	 */
#endif

	/*
	 *      主要ワーク
	 */
	extern BOOL crt_flag;
	/*
	 * CRT表示フラグ
	 */
	extern BOOL vrama_flag;
	/*
	 * VRAMアクセスフラグ
	 */
	extern WORD vram_offset[2];
	/*
	 * VRAMオフセットレジスタ
	 */
	extern BOOL vram_offset_flag;
	/*
	 * 拡張VRAMオフセットフラグ
	 */
	extern BOOL vsync_flag;
	/*
	 * VSYNCフラグ
	 */
	extern BOOL draw_aftervsync;
	/*
	 * 画面描画通知タイミング
	 */
	extern int now_raster;
	/* 現在ラスタ位置 */

#if XM7_VER >= 2
	extern BOOL subnmi_flag;
	/*
	 * サブNMIイネーブルフラグ
	 */
	extern BOOL blank_flag;
	/*
	 * ブランキングフラグ
	 */
	extern BYTE vram_active;
	/*
	 * アクティブページ
	 */
	extern BYTE *vram_aptr;
	/*
	 * VRAMアクティブポインタ
	 */
	extern BYTE vram_display;
	/*
	 * 表示ページ
	 */
	extern BYTE *vram_dptr;
	/*
	 * VRAM表示ポインタ
	 */

#if XM7_VER >= 3
	/*
	 * FM77AV40
	 */
	extern BYTE screen_mode;
	/*
	 * 画面モード
	 */
	extern BYTE subram_vrambank;
	/*
	 * アクティブページ(400line/26万色)
	 */

	/*
	 * FM77AV40EX
	 */
	extern WORD window_x1, window_dx1;
	/*
	 * ハードウェアウィンドウ X1
	 */
	extern WORD window_y1, window_dy1;
	/*
	 * ハードウェアウィンドウ Y1
	 */
	extern WORD window_x2, window_dx2;
	/*
	 * ハードウェアウィンドウ X2
	 */
	extern WORD window_y2, window_dy2;
	/*
	 * ハードウェアウィンドウ Y2
	 */
	extern BOOL window_open;
	/*
	 * ウィンドウオープンフラグ
	 */
	extern BYTE block_active;
	/*
	 * アクティブブロック
	 */
	extern BYTE block_display;
	/*
	 * 表示ブロック
	 */
	extern BYTE *vram_ablk;
	/*
	 * アクティブブロックポインタ
	 */
	extern BYTE *vram_bdptr;
	/*
	 * 裏表示ブロックポインタ
	 */
	extern BYTE *vram_dblk;
	/*
	 * 表示ブロックポインタ
	 */
	extern BYTE *vram_bdblk;
	/*
	 * 裏表示ブロックポインタ2
	 */
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
	extern BOOL width40_flag;
	/*
	 * WIDTH40モードフラグ
	 */
	extern BOOL cursor_lsb;
	/*
	 * カーソルアドレスLSB
	 */
	extern BOOL enable_400line;
	/*
	 * 400ラインモードフラグ
	 */
	extern BOOL workram_select;
	/*
	 * ワークRAM選択フラグ
	 */
	extern BYTE crtc_register[0x20];
	/*
	 * CRTCレジスタ
	 */
	extern BYTE crtc_regnum;
	/*
	 * CRTCレジスタ番号レジスタ
	 */
	extern WORD text_start_addr;
	/*
	 * テキストスタートアドレス
	 */
	extern BOOL text_blink;
	/*
	 * テキストブリンク状態
	 */
	extern WORD cursor_addr;
	/*
	 * カーソルアドレス
	 */
	extern BOOL cursor_blink;
	/*
	 * カーソルブリンク状態
	 */
#endif
#if XM7_VER >= 3
	extern const BYTE truecolorbrightness[64];
	/*
	 * 24/32bit Color用輝度テーブル
	 */
#endif

#ifdef __cplusplus
}
#endif
#endif													/* _display_h_ */

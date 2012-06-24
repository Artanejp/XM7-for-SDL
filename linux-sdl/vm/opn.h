/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ OPN/WHG/THG(YM2203) ]
 */

#ifndef _opn_h_
#define _opn_h_

/*
 *	定数定義
 */
#define OPN_STD 0 /* 標準OPN */
#define OPN_WHG 1 /* WHG OPN */
#define OPN_THG 2 /* THG OPN */

#define OPN_INACTIVE    0x00	/* インアクティブコマンド */
#define OPN_READDAT     0x01	/* リードデータコマンド */
#define OPN_WRITEDAT    0x02	/* ライトデータコマンド */
#define OPN_ADDRESS     0x03	/* ラッチアドレスコマンド */
#define OPN_READSTAT    0x04	/* リードステータスコマンド */
#define OPN_JOYSTICK    0x09	/* ジョイスティックコマンド */
#define OPN_CLOCK       12288	/* OPN基準クロック(1.2288MHz) */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL opn_init(void);
										/* 初期化 */
void FASTCALL opn_cleanup(void);
										/* クリーンアップ */
void FASTCALL opn_reset(void);
										/* リセット */
BOOL FASTCALL opn_readb(WORD addr, BYTE *dat);
										/* OPN メモリ読み出し */
BOOL FASTCALL opn_writeb(WORD addr, BYTE dat);
										/* OPN メモリ書き込み */
BOOL FASTCALL opn_save(SDL_RWops *fileh);
										/* OPN セーブ */
BOOL FASTCALL opn_load(SDL_RWops *fileh, int ver);
										/* OPN ロード */

BOOL FASTCALL whg_init(void);
										/* WHG 初期化 */
void FASTCALL whg_cleanup(void);
										/* WHG クリーンアップ */
void FASTCALL whg_reset(void);
										/* WHG リセット */
BOOL FASTCALL whg_readb(WORD addr, BYTE *dat);
										/* WHG メモリ読み出し */
BOOL FASTCALL whg_writeb(WORD addr, BYTE dat);
										/* WHG メモリ書き込み */
BOOL FASTCALL whg_save(SDL_RWops *fileh);
										/* WHG セーブ */
BOOL FASTCALL whg_load(SDL_RWops *fileh, int ver);
										/* WHG ロード */

BOOL FASTCALL thg_init(void);
										/* THG 初期化 */
void FASTCALL thg_cleanup(void);
										/* THG クリーンアップ */
void FASTCALL thg_reset(void);
										/* THG リセット */
BOOL FASTCALL thg_readb(WORD addr, BYTE *dat);
										/* THG メモリ読み出し */
BOOL FASTCALL thg_writeb(WORD addr, BYTE dat);
										/* THG メモリ書き込み */
BOOL FASTCALL thg_save(SDL_RWops *fileh);
										/* THG セーブ */
BOOL FASTCALL thg_load(SDL_RWops *fileh, int ver);
										/* THG ロード */

/*
 *	主要ワーク
 */
extern BOOL opn_enable;
										/* OPN有効・無効フラグ(7 only) */
extern BOOL whg_enable;
										/* WHG有効・無効フラグ */
extern BOOL whg_use;
										/* WHG使用フラグ */
extern BOOL thg_enable;
										/* THG有効・無効フラグ */
extern BOOL thg_use;
										/* THG使用フラグ */

extern BYTE opn_reg[3][256];
										/* OPNレジスタ */
extern BOOL opn_key[3][4];
										/* OPNキーオンフラグ */
extern BOOL opn_timera[3];
										/* タイマーA動作フラグ */
extern BOOL opn_timerb[3];
										/* タイマーB動作フラグ */
extern DWORD opn_timera_tick[3];
										/* タイマーA間隔(us) */
extern DWORD opn_timerb_tick[3];
										/* タイマーB間隔(us) */
extern BYTE opn_scale[3];
										/* プリスケーラ */
#ifdef __cplusplus
}
#endif

#endif	/* _opn_h_ */

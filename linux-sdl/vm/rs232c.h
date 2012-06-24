/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ RS-232Cインタフェース ]
 */

#ifndef _rs232c_h_
#define _rs232c_h_

/*
 *	定数定義
 */

/* ステータスレジスタ */
#define	RSS_DSR			0x80
#define	RSS_SYNDET		0x40
#define	RSS_FRAMEERR	0x20
#define	RSS_OVERRUN		0x10
#define	RSS_PARITYERR	0x08
#define	RSS_TXEMPTY		0x04
#define	RSS_RXRDY		0x02
#define	RSS_TXRDY		0x01

/* モードコマンドレジスタ */
#define	RSM_STOPBITM	0xc0
#define	RSM_STOPBIT1	0x40
#define	RSM_STOPBIT15	0x80
#define	RSM_STOPBIT2	0xc0
#define	RSM_PARITYM		0x30
#define	RSM_PARITYEVEN	0x20
#define	RSM_PARITYEN	0x10
#define	RSM_CHARLENM	0x0c
#define	RSM_CHARLEN5	0x00
#define	RSM_CHARLEN6	0x04
#define	RSM_CHARLEN7	0x08
#define	RSM_CHARLEN8	0x0c
#define	RSM_BAUDDIVM	0x03
#define	RSM_BAUDDIV1	0x01
#define	RSM_BAUDDIV16	0x02
#define	RSM_BAUDDIV64	0x03

/* コマンドレジスタ */
#define	RSC_EH			0x80
#define	RSC_IR			0x40
#define	RSC_RTS			0x20
#define	RSC_ER			0x10
#define	RSC_SBRK		0x08
#define	RSC_RXE			0x04
#define	RSC_DTR			0x02
#define	RSC_TXEN		0x01

/* FM77AV40/20 クロック・ボーレート設定レジスタ */
#define	RSCB_CLKM		0xe0
#define	RSCB_BAUDM		0x1c
#define	RSCB_CD			0x04

/* FM77AV40/20 拡張DTR制御レジスタ */
#define	RSEX_DTR		0x04
#define	RSEX_RSENABLE	0x01


#ifdef RSC

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL rs232c_init(void);
										/* 初期化 */
void FASTCALL rs232c_cleanup(void);
										/* クリーンアップ */
void FASTCALL rs232c_reset(void);
										/* リセット */
void FASTCALL rs232c_txrdy(BOOL flag);
										/* TxRDY/TxEMPTY信号変化 */
void FASTCALL rs232c_txempty_request(void);
										/* TxEMPTYイベント登録 */
void FASTCALL rs232c_rxrdy(BOOL flag);
										/* RxRDY信号変化 */
void FASTCALL rs232c_rxrdy_request(void);
										/* RxRDYイベント登録 */
void FASTCALL rs232c_syndet(BOOL flag);
										/* SYNDET信号変化 */
void FASTCALL rs232c_calc_timing(void);
										/* 送受信タイミング計算 */
BOOL FASTCALL rs232c_readb(WORD addr, BYTE *dat);
										/* 1バイト読み出し */
BOOL FASTCALL rs232c_writeb(WORD addr, BYTE dat);
										/* 1バイト書き込み */
BOOL FASTCALL rs232c_save(SDL_RWops *fileh);
										/* セーブ */
BOOL FASTCALL rs232c_load(SDL_RWops *fileh, int ver);
										/* ロード */

/*
 *	主要ワーク
 */
extern BOOL	rs_use;
										/* RS-232C使用フラグ */
extern BOOL	rs_mask;
										/* RS-232C機能マスク */
extern BOOL	rs_enable;
										/* RS-232C有効 */
extern BOOL	rs_selectmc;
										/* モードコマンド選択状態 */
extern BYTE	rs_modecmd;
										/* モードコマンドレジスタ */
extern BYTE	rs_command;
										/* コマンドレジスタ */
extern BYTE	rs_baudrate;
										/* ボーレート設定レジスタ */
extern BYTE	rs_baudrate_v2;
										/* ボーレート設定(V1/V2用) */
extern BOOL rs_dtrmask;
										/* DTR信号マスクフラグ */
extern BOOL rs_cd;
										/* CD信号 */
extern BOOL rs_cts;
										/* CTS信号 */
#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER >= 3 */
#endif	/* _rs232c_h_ */

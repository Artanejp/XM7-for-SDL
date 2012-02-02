/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ RS-232Cインタフェース ]
 */

#ifdef RSC

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "event.h"
#include "device.h"
#include "mainetc.h"
#include "rs232c.h"

/*
 *	グローバル ワーク
 */
BOOL	rs_use;						/* RS-232C使用フラグ */
BOOL	rs_mask;					/* RS-232C機能マスク */
BOOL	rs_enable;					/* RS-232C有効化フラグ(AV40/20以降) */
BOOL	rs_selectmc;				/* モードコマンド選択状態 */
BYTE	rs_modecmd;					/* モードコマンドレジスタ */
BYTE	rs_command;					/* コマンドレジスタ */
BYTE	rs_status;					/* ステータスレジスタ */
BYTE	rs_baudrate;				/* ボーレート設定レジスタ */
BYTE	rs_baudrate_v2;				/* ボーレート設定(V1/V2用) */
BOOL	rs_dtrmask;					/* DTR信号マスクフラグ */
BOOL	rs_cd;						/* CD信号 */
BOOL	rs_cts;						/* CTS信号 */

/*
 *	スタティック ワーク
 */
static DWORD	sndrcv_timing;		/* データ送受信タイミング(us) */


/*
 *	RS-232C
 *	初期化
 */
BOOL FASTCALL rs232c_init(void)
{
	rs_use = FALSE;
	rs_mask = TRUE;
	rs_cd = FALSE;

	return TRUE;
}

/*
 *	RS-232C
 *	クリーンアップ
 */
void FASTCALL rs232c_cleanup(void)
{
}

/*
 *	RS-232C
 *	リセット
 */
void FASTCALL rs232c_reset(void)
{
	/* ワークエリア初期化 */
	rs_enable = FALSE;
	rs_selectmc = TRUE;
#if XM7_VER >= 3
	if (fm7_ver <= 2) {
		rs_baudrate = (rs_baudrate_v2 << 2);
	}
	else {
		rs_baudrate = 0x00;
	}
#else
	rs_baudrate = (rs_baudrate_v2 << 2);
#endif
	rs_modecmd = 0x0c;
	rs_command = 0x40;
	rs_dtrmask = FALSE;
	rs_cts = FALSE;
	rs_status = (RSS_TXRDY | RSS_TXEMPTY);

	/* リセット通知 */
	rs232c_reset_notify();

	/* レジスタ設定 */
	rs232c_setbaudrate(rs_baudrate);
	rs232c_writemodecmd(rs_modecmd);
	rs232c_writecommand(rs_command);
	rs232c_calc_timing();
}

/*
 *	RS-232C
 *	TxRDY信号変化
 */
void FASTCALL rs232c_txrdy(BOOL flag)
{
	/* ステータス更新 */
	if (flag) {
		rs_status |= RSS_TXRDY;
	}
	else {
		rs_status &= (BYTE)~(RSS_TXRDY | RSS_TXEMPTY);
	}

	/* 割り込み */
	if (!txrdy_irq_mask && !rs_cts && (rs_status & RSS_TXEMPTY)) {
		txrdy_irq_flag = TRUE;
	}
	else {
		txrdy_irq_flag = FALSE;
	}
	maincpu_irq();
}

/*
 *	RS-232C
 *	TxEMPTYイベント
 */
BOOL FASTCALL rs232c_txempty_event(void)
{
	/* TxEMPTY ON */
	rs_status |= RSS_TXEMPTY;

	/* TxRDY割り込み */
	rs232c_txrdy(TRUE);

	/* イベント削除 */
	schedule_delevent(EVENT_RS_TXTIMING);

	return TRUE;
}

/*
 *	RS-232C
 *	TxEMPTYイベント登録
 */
void FASTCALL rs232c_txempty_request(void)
{
	/* TxRDY ON */
	rs232c_txrdy(TRUE);

	/* イベント登録 */
	schedule_setevent(EVENT_RS_TXTIMING, sndrcv_timing, rs232c_txempty_event);
}

/*
 *	RS-232C
 *	RxRDY信号変化
 */
void FASTCALL rs232c_rxrdy(BOOL flag)
{
	/* ステータス更新 */
	if (flag) {
		rs_status |= RSS_RXRDY;
	}
	else {
		rs_status &= (BYTE)~RSS_RXRDY;
	}

	/* 割り込み */
	if (!rxrdy_irq_mask && (rs_status & RSS_RXRDY)) {
		rxrdy_irq_flag = TRUE;
	}
	else {
		rxrdy_irq_flag = FALSE;
	}
	maincpu_irq();
}

/*
 *	RS-232C
 *	RxRDYイベント
 */
BOOL FASTCALL rs232c_rxrdy_event(void)
{
	/* RxRDY ON/割り込み */
	rs232c_rxrdy(TRUE);

	/* イベント削除 */
	schedule_delevent(EVENT_RS_RXTIMING);

	return TRUE;
}

/*
 *	RS-232C
 *	RxRDYイベント登録
 */
void FASTCALL rs232c_rxrdy_request(void)
{
	/* イベント登録…だけ */
	schedule_setevent(EVENT_RS_RXTIMING, sndrcv_timing, rs232c_rxrdy_event);
}

/*
 *	RS-232C
 *	SYNDET信号変化
 */
void FASTCALL rs232c_syndet(BOOL flag)
{
	/* ステータス更新 */
	if (flag) {
		rs_status |= RSS_SYNDET;
	}
	else {
		rs_status &= (BYTE)~RSS_SYNDET;
	}

	/* 割り込み */
	if (!syndet_irq_mask && (rs_status & RSS_SYNDET)) {
		syndet_irq_flag = TRUE;
	}
	else {
		syndet_irq_flag = FALSE;
	}
	maincpu_irq();
}

/*
 *	RS-232C
 *	送受信タイミング計算
 */
void FASTCALL rs232c_calc_timing(void)
{
	DWORD baudrate;
	BYTE databits;

	/* ボーレートを計算 */
	baudrate = 300 << ((rs_baudrate & RSCB_BAUDM) >> 2);
	if ((rs_modecmd & RSM_BAUDDIVM) == RSM_BAUDDIV16) {
		/* FASTモード */
		baudrate *= 4;
	}

	/* キャラクタ長を取得 */
	switch (rs_modecmd & RSM_CHARLENM) {
		case RSM_CHARLEN5	:	databits = 5;
								break;
		case RSM_CHARLEN6	:	databits = 6;
								break;
		case RSM_CHARLEN7	:	databits = 7;
								break;
		case RSM_CHARLEN8	:	databits = 8;
								break;
	}

	/* スタート/ストップビット分を加算 */
	if ((rs_modecmd & RSM_STOPBITM) == RSM_STOPBIT1) {
		databits += (BYTE)2;
	}
	else {
		databits += (BYTE)3;
	}

	/* 1バイトあたりの送出時間をμs単位で求める */
	sndrcv_timing = (10000000 / (baudrate / databits));
	sndrcv_timing = (sndrcv_timing + 5) / 10;
}


/*
 *	RS-232C
 *	１バイト読み込み
 */
BOOL FASTCALL rs232c_readb(WORD addr, BYTE *dat)
{
	switch (addr) {
		case 0xfd06 :	/* USARTデータレジスタ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_mask) {
#else
			if (rs_use && !rs_mask) {
#endif
				if (rs_selectmc) {
					*dat = 0;
				}
				else {
					*dat = rs232c_receivedata();
				}
				return TRUE;
			}
			break;

		case 0xfd07 :	/* USARTステータスレジスタ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_mask) {
#else
			if (rs_use && !rs_mask) {
#endif
				*dat = rs232c_readstatus();

				/* SYNDET信号を落とす */
				if (rs_status & RSS_SYNDET) {
					rs232c_syndet(FALSE);
				}

				/* もしかしたら他の割り込みも落ちるかも */
				txrdy_irq_flag = FALSE;
				rxrdy_irq_flag = FALSE;
				maincpu_irq();

				return TRUE;
			}
			break;
	}

	return FALSE;
}

/*
 *	RS-232C
 *	１バイト書き込み
 */
BOOL FASTCALL rs232c_writeb(WORD addr, BYTE dat)
{
	BOOL flag;

	switch (addr) {
		case 0xfd06 :	/* USARTデータレジスタ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_selectmc &&
				!rs_mask) {
#else
			if (rs_use && !rs_selectmc && !rs_mask) {
#endif
				rs232c_senddata(dat);
				return TRUE;
			}
			break;

		case 0xfd07 :	/* USARTコマンドレジスタ */
#if XM7_VER >= 3
			if (rs_use && ((fm7_ver <= 2) || rs_enable) && !rs_mask) {
#else
			if (rs_use && !rs_mask) {
#endif
				if (rs_selectmc) {
					rs232c_writemodecmd(dat);
					rs_modecmd = dat;
					rs_selectmc = FALSE;

					/* 送受信タイミング計算 */
					rs232c_calc_timing();
				}
				else {
					rs232c_writecommand(dat);
					rs_command = dat;

					if (dat & RSC_IR) {
						/* 内部リセット */
						rs_selectmc = TRUE;
					}
				}
				return TRUE;
			}
			break;

#if XM7_VER >= 3
		case 0xfd0b :	/* FM77AV40/20 クロック・ボーレート設定レジスタ */
			if ((fm7_ver >= 3) && !rs_mask) {
				rs232c_setbaudrate(dat);
				rs_baudrate = dat;

				/* 送受信タイミング計算 */
				rs232c_calc_timing();
			}

			return TRUE;

		case 0xfd0c :	/* FM77AV40/20 拡張DTRレジスタ */
			if (fm7_ver >= 3) {
				/* bit0:RS-232C有効 */
				if (dat & RSEX_RSENABLE) {
					rs_enable = TRUE;
				}
				else {
					rs_enable = FALSE;
				}

				/* bit2:DTR出力禁止 */
				flag = rs_dtrmask;
				if (dat & RSEX_DTR) {
					rs_dtrmask = TRUE;
					flag = !flag;
				}
				else {
					rs_dtrmask = FALSE;
				}
				if (rs_enable && !rs_mask && flag && (rs_command & RSC_DTR)) {
					rs232c_writecommand(rs_command);
				}
				return TRUE;
			}
#endif

			return FALSE;
	}

	return FALSE;
}

/*
 *	RS-232C
 *	セーブ
 */
BOOL FASTCALL rs232c_save(int fileh)
{
	if (!file_bool_write(fileh, rs_mask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_selectmc)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_modecmd)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_command)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_status)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, rs_baudrate)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_dtrmask)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_cd)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, rs_cts)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	RS-232C
 *	ロード
 */
BOOL FASTCALL rs232c_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}
#if XM7_VER >= 3
	if (((ver >= 500) && (ver < 715)) || ((ver >= 800) && (ver < 915))) {
#elif XM7_VER >= 2
	if (ver < 715) {
#else
	if (ver < 305) {
#endif
		return TRUE;
	}

	if (!file_bool_read(fileh, &rs_mask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_enable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_selectmc)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_modecmd)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_command)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_status)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &rs_baudrate)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_dtrmask)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_cd)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &rs_cts)) {
		return FALSE;
	}

	/* イベント */
	schedule_handle(EVENT_RS_TXTIMING, rs232c_txempty_event);
	schedule_handle(EVENT_RS_RXTIMING, rs232c_rxrdy_event);

	/* ポート設定 */
	rs232c_setbaudrate(rs_baudrate);
	rs232c_writemodecmd(rs_modecmd);
	rs232c_writecommand(rs_command);
	rs232c_calc_timing();

	return TRUE;
}

#endif		/* RSC */

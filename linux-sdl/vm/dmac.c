/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2013 Ryu Takegami
 *
 *      [ DMAC(HD6844) AV40仕様対応版 ]
 */

#if XM7_VER >= 3

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "dmac.h"
#include "fdc.h"
#include "mmr.h"
#include "event.h"


/*
 *	グローバル ワーク
 */
WORD dma_adr[4];								/* アドレスレジスタ */
WORD dma_bcr[4];								/* 転送語数レジスタ */
BYTE dma_chcr[4];								/* チャネル制御レジスタ */
BYTE dma_pcr;										/* 優先制御レジスタ */
BYTE dma_icr;										/* 割り込み制御レジスタ */
BYTE dma_dcr;										/* データチェイン制御レジスタ */

BYTE dma_reg;										/* 現在選択されているレジスタ番号 */
BOOL dma_flag;									/* DMA転送実行中フラグ */
BOOL dma_burst_transfer;				/* DMAバースト転送実行中フラグ */


/*
 *	プロトタイプ宣言
 */
static BOOL FASTCALL dmac_start_ch0(void);	/* DMA転送開始イベント */


/*
 *	DMAC
 *	初期化
 */
BOOL FASTCALL
dmac_init(void)
{
	return TRUE;
}

/*
 *	DMAC
 *	クリーンアップ
 */
void FASTCALL
dmac_cleanup(void)
{
}

/*
 *	DMAC
 *	リセット
 */
void FASTCALL
dmac_reset(void)
{
	int i;

	/* DMAC内部レジスタ初期化 */
	for (i = 0; i < 4; i++) {
		dma_adr[i] = 0xFFFF;
		dma_bcr[i] = 0xFFFF;
		dma_chcr[i] = 0x00;
	}
	dma_pcr = 0x00;
	dma_icr = 0x00;
	dma_dcr = 0x00;

	/* ワーク初期化 */
	dma_reg = 0x00;
	dma_flag = FALSE;
	dma_burst_transfer = FALSE;
}


/*
 *	DMAC
 *	転送開始
 */
void FASTCALL
dmac_start(void)
{
	/* バージョンチェック */
	if (fm7_ver < 3) {
		return;
	}

	/* 既にDMA転送動作中であれば何もしない */
	if (dma_flag) {
		return;
	}

	/* 転送語長レジスタ(BCR)チェック */
	if (dma_bcr[0] == 0) {
		return;
	}

	/* DMA使用フラグ(TxRQ)チェック */
	if (!(dma_pcr & 0x01)) {
		return;
	}

	/* DMA転送開始 イベント設定 */
	schedule_setevent(EVENT_FDC_DMA, 50, dmac_start_ch0);
}

/*
 *	DMAC Ch.0(FDC)
 *	転送開始イベント
 */
static BOOL FASTCALL
dmac_start_ch0(void)
{
	/* イベントを削除 */
	schedule_delevent(EVENT_FDC_DMA);

	/* DMAスタート */
	dma_flag = TRUE;
	dma_chcr[0] = (BYTE) ((dma_chcr[0] & 0x0f) | 0x40);

	return TRUE;
}

/*
 *	DMAC Ch.0(FDC)
 *	DMA転送処理
 */
void FASTCALL
dmac_exec(void)
{
	BYTE dat;
	BYTE seg;

	/* TxRQチェック */
	if (!(dma_pcr & 0x01)) {
		return;
	}

	/* この時点でBCRが0なら転送/割り込み発生をせずに処理を中止 */
	if (dma_bcr[0] == 0) {
		dma_flag = FALSE;
		dma_chcr[0] = (BYTE) ((dma_chcr[0] & 0x0f) | 0x80);
		return;
	}

	/* バーストモードのチェックをおこなう */
	if ((dma_chcr[0] & 0x02) && !dma_burst_transfer) {
		dma_burst_transfer = TRUE;
	}

	/* FDCからのDRQを受け、転送する */
	fdc_readb(0xfd1f, &dat);
	if (!(dat & 0x80)) {
		if (dma_burst_transfer) {
			/* スケジューリングの関係上、メインCPUクロックを進める */
			maincpu.total += (WORD) 2;
		}
		return;
	}

	/* DMA転送を実行。MMRは常にセグメント0になる */
	seg = mmr_seg;
	mmr_seg = 0;

	if (dma_chcr[0] & 0x01) {
		/* 1:メモリ→FDC (Write) */
		dat = mainmem_readb(dma_adr[0]);
		fdc_writeb(0xfd1b, dat);
	}
	else {
		/* 0:FDC→メモリ (Read) */
		fdc_readb(0xfd1b, &dat);
		mainmem_writeb(dma_adr[0], dat);
	}

	/* MMRセグメント復帰 */
	mmr_seg = seg;

	/* アドレス更新 */
	if (dma_chcr[0] & 0x08) {
		/* アドレスDOWN */
		dma_adr[0]--;
	}
	else {
		/* アドレスUP */
		dma_adr[0]++;
	}

	/* メインCPUクロックの引き延ばしを行う */
	maincpu.total += (WORD) 3;

	/* 転送サイズ更新 */
	dma_bcr[0]--;

	/* 転送終了チェック */
	if (dma_bcr[0] == 0) {
#ifdef DMAC_AV40
		if ((dma_dcr & 0x07) == 0x01) {
			/* データチェイン */
			dma_adr[0] = dma_adr[3];
			dma_bcr[0] = dma_bcr[3];
			dma_bcr[3] = 0;
		}
		else {
			/* DMA終了 */
			dma_flag = FALSE;
			dma_burst_transfer = FALSE;
			dma_chcr[0] = (BYTE) ((dma_chcr[0] & 0x0f) | 0x80);
			dma_dcr |= 0x10;

			/* 割り込みがイネーブルなら、割り込みをかける */
			if (dma_icr & 0x01) {
				dma_icr |= 0x80;
				dma_irq_flag = TRUE;
				maincpu_irq();
			}
			return;
		}
#else
		/* DMA終了 */
		dma_flag = FALSE;
		dma_burst_transfer = FALSE;
		dma_chcr[0] = (BYTE) ((dma_chcr[0] & 0x0f) | 0x80);
		dma_dcr |= 0x10;

		/* 割り込みがイネーブルなら、割り込みをかける */
		if (dma_icr & 0x01) {
			dma_icr |= 0x80;
			dma_irq_flag = TRUE;
			maincpu_irq();
		}
		return;
#endif
	}
}

/*
 *	DMAC
 *	レジスタ読み込み
 */
static BYTE FASTCALL
dmac_readreg(WORD addr)
{
	BYTE tmp;

	switch (addr) {
#ifdef DMAC_AV40
			/* アドレスレジスタ(上位) */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			return (BYTE) ((dma_adr[addr >> 2] >> 8) & 0xff);

			/* アドレスレジスタ(下位) */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			return (BYTE) (dma_adr[addr >> 2] & 0xff);

			/* 転送語数レジスタ(上位) */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			return (BYTE) ((dma_bcr[addr >> 2] >> 8) & 0xff);

			/* 転送語数レジスタ(下位) */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			return (BYTE) (dma_bcr[addr >> 2] & 0xff);

			/* チャネル制御レジスタ */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			tmp = dma_chcr[addr - 0x10];
			dma_chcr[addr - 0x10] &= 0x7f;
			return tmp;
#else
			/* アドレスレジスタ(上位) */
		case 0x00:
			return (BYTE) ((dma_adr[0] >> 8) & 0xff);

			/* アドレスレジスタ(下位) */
		case 0x01:
			return (BYTE) (dma_adr[0] & 0xff);

			/* 転送語数レジスタ(上位) */
		case 0x02:
			return (BYTE) ((dma_bcr[0] >> 8) & 0xff);

			/* 転送語数レジスタ(下位) */
		case 0x03:
			return (BYTE) (dma_bcr[0] & 0xff);

			/* チャネル制御レジスタ */
		case 0x10:
			tmp = dma_chcr[0];
			dma_chcr[0] &= 0x7f;
			return tmp;
#endif

			/* 優先制御レジスタ */
		case 0x14:
			return dma_pcr;

			/* 割り込み制御レジスタ */
		case 0x15:
			tmp = (BYTE) (((dma_dcr >> 4) | 0x80) & dma_icr);
			dma_dcr &= 0x0f;

			/* IRQを解除 */
			dma_icr &= (BYTE) ~ 0x80;
			dma_irq_flag = FALSE;
			maincpu_irq();
			return tmp;

			/* データチェイン制御レジスタ */
		case 0x16:
			return (BYTE) (dma_dcr & 0x0f);
	}

	/* その他のレジスタはデコードされていない */
	return 0x00;
}

/*
 *	DMAC
 *	レジスタ書き込み
 */
static void FASTCALL
dmac_writereg(WORD addr, BYTE dat)
{
	switch (addr) {
			/* アドレスレジスタ(上位) */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			dma_adr[addr >> 2] = (WORD) ((dma_adr[addr >> 2] & 0xff) | (dat << 8));
			return;

			/* アドレスレジスタ(下位) */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			dma_adr[addr >> 2] = (WORD) ((dma_adr[addr >> 2] & 0xff00) | dat);
			return;

			/* 転送語数レジスタ(上位) */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			dma_bcr[addr >> 2] = (WORD) ((dma_bcr[addr >> 2] & 0xff) | (dat << 8));
			return;

			/* 転送語数レジスタ(下位) */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			dma_bcr[addr >> 2] = (WORD) ((dma_bcr[addr >> 2] & 0xff00) | dat);
			return;

			/* チャネル制御レジスタ */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			dma_chcr[addr - 0x10] =
				(BYTE) ((dma_chcr[addr - 0x10] & 0xc0) | (dat & 0x0f));
			return;

			/* 優先制御レジスタ */
		case 0x14:
			dma_pcr = (BYTE) (dat & 0x8f);
			return;

			/* 割り込み制御レジスタ */
		case 0x15:
			dma_icr = (BYTE) ((dma_icr & 0x80) | (dat & 0x0f));
			return;

			/* データチェイン制御レジスタ */
		case 0x16:
			dma_dcr = (BYTE) ((dma_dcr & 0xf0) | (dat & 0x0f));
			return;
	}
}

/*
 *	DMAC
 *	１バイト取得
 */
BOOL FASTCALL
dmac_readb(WORD addr, BYTE * dat)
{
	/* バージョンチェック */
	if (fm7_ver < 3) {
		return FALSE;
	}

	switch (addr) {
			/* DMACアドレス */
		case 0xfd98:
			*dat = dma_reg;
			return TRUE;

			/* DMACデータ */
		case 0xfd99:
			*dat = dmac_readreg(dma_reg);
			return TRUE;
	}

	return FALSE;
}

/*
 *	DMAC
 *	１バイト書き込み
 */
BOOL FASTCALL
dmac_writeb(WORD addr, BYTE dat)
{
	/* バージョンチェック */
	if (fm7_ver < 3) {
		return FALSE;
	}

	switch (addr) {
			/* DMACアドレス */
		case 0xfd98:
			dma_reg = (BYTE) (dat & 0x1f);
			return TRUE;

			/* DMACデータ */
		case 0xfd99:
			dmac_writereg(dma_reg, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *      DMAC
 *      セーブ
 */
BOOL FASTCALL
dmac_save(SDL_RWops * fileh)
{
#ifdef DMAC_AV40
	int i;
#endif

	if (!file_word_write(fileh, dma_adr[0])) {
		return FALSE;
	}
	if (!file_word_write(fileh, dma_bcr[0])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_chcr[0])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_pcr)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_icr)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, dma_reg)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, dma_flag)) {
		return FALSE;
	}

	/*
	 * Ver9拡張
	 */
	if (!file_byte_write(fileh, dma_dcr)) {
		return FALSE;
	}

	/*
	 * Ver912拡張
	 */
	if (!file_bool_write(fileh, dma_burst_transfer)) {
		return FALSE;
	}

	/*
	 * Ver9拡張・AV40仕様時
	 * (40EX仕様時との互換性がなくなるので注意)
	 */
#ifdef DMAC_AV40
	for (i = 1; i < 4; i++) {
		if (!file_word_write(fileh, dma_adr[i])) {
			return FALSE;
		}
		if (!file_word_write(fileh, dma_bcr[i])) {
			return FALSE;
		}
		if (!file_word_write(fileh, dma_chcr[i])) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}

/*
 *      DMAC
 *      ロード
 */
BOOL FASTCALL
dmac_load(SDL_RWops * fileh, int ver)
{
	int i;

	/*
	 * バージョンチェック
	 */
	if (ver < 800) {
		dmac_reset();
		return TRUE;
	}

	if (!file_word_read(fileh, &dma_adr[0])) {
		return FALSE;
	}
	if (!file_word_read(fileh, &dma_bcr[0])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_chcr[0])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_pcr)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_icr)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &dma_reg)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &dma_flag)) {
		return FALSE;
	}

	/*
	 * Ver9拡張
	 */
	if (ver >= 900) {
		if (!file_byte_read(fileh, &dma_dcr)) {
			return FALSE;
		}

		/*
		 * Ver912拡張
		 */
		if (ver >= 912) {
			if (!file_bool_read(fileh, &dma_burst_transfer)) {
				return FALSE;
			}
		}
#ifdef DMAC_AV40
		/*
		 * Ver9拡張・AV40仕様時
		 * (40EX仕様時との互換性がなくなるので注意)
		 */
		for (i = 1; i < 4; i++) {
			if (!file_word_read(fileh, &dma_adr[i])) {
				return FALSE;
			}
			if (!file_word_read(fileh, &dma_bcr[i])) {
				return FALSE;
			}
			if (!file_byte_read(fileh, &dma_chcr[i])) {
				return FALSE;
			}
		}
#endif
	}
	else {
		dma_dcr = 0x08;
	}

	/*
	 * Ch1〜3を初期化
	 */
#ifndef DMAC_AV40
	for (i = 1; i < 4; i++) {
		dma_adr[i] = 0xFFFF;
		dma_bcr[i] = 0xFFFF;
		dma_chcr[i] = 0x00;
	}
#endif

	return TRUE;
}

#endif /* XM7_VER >= 3 */

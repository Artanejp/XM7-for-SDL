/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2012 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2012 Ryu Takegami
 *	Copyright (C) 2010-2012 Toma
 *
 *	[ バブルメモリ コントローラ (32KB専用版) ]
 */

#ifndef _bubble_h_
#define _bubble_h_

#if XM7_VER == 1 && defined(BUBBLE)

/*
 *	定数定義
 */
#define BMC_UNITS_32		2			/* 32KBサポートユニット数 */

#define BMC_ST_BUSY			0x01		/* BUSY */
#define BMC_ST_ERROR		0x02		/* ERROR ANALYSIS */
#define BMC_ST_WRITEP		0x04		/* 書き込み保護 */
#define BMC_ST_NONE			0x08		/* ---- */
#define BMC_ST_NOTREADY		0x10		/* メディア未挿入 */
#define BMC_ST_RDA			0x20		/* CAN READ */
#define BMC_ST_TDRA			0x40		/* CAN WRITE */
#define BMC_ST_CME			0x80		/* COMMAND END */

#define BMC_ES_UNDEF		0x01		/* UNDEFINED COMMAND ERROR */
#define BMC_ES_NOMAKER		0x02		/* NO MARKER */
#define BMC_ES_MANYBAD		0x04		/* MANY BAD LOOP */
#define BMC_ES_TRANSFER		0x08		/* TRANSFER MISSING */
#define BMC_ES_CRCERR		0x10		/* CRCエラー */
#define BMC_ES_PAGEOVER		0x20		/* PAGE ADDRESS OVER ERROR */
#define BMC_ES_NONE			0x40		/* ---- */
#define BMC_ES_EJECT		0x80		/* EJECT ERROR */

#define BMC_TYPE_NOTREADY	0			/* ファイルなし */
#define BMC_TYPE_32			1			/* 32KBファイルをマウント */

#define BMC_ACCESS_READY	0			/* アクセスなし */
#define BMC_ACCESS_READ		1			/* 読み込み系アクセス */
#define BMC_ACCESS_WRITE	2			/* 書き込み系アクセス */

#define BMC_32				0			/* 32KB */

#define BMC_PSIZE_32		0x20		/* 32KBページサイズ */
#define BMC_MAXADDR_32		0x03ff		/* 32KB最終ページアドレス */

#ifdef __cplusplus
extern "C" {
#endif
/*
 *	主要エントリ
 */
BOOL FASTCALL bmc_init(void);
										/* 初期化 */
void FASTCALL bmc_cleanup(void);
										/* クリーンアップ */
void FASTCALL bmc_reset(void);
										/* リセット */
BOOL FASTCALL bmc_readb(WORD addr, BYTE *dat);
										/* メモリ読み出し */
BOOL FASTCALL bmc_writeb(WORD addr, BYTE dat);
										/* メモリ書き込み */
BOOL FASTCALL bmc_save(int fileh);
										/* セーブ */
BOOL FASTCALL bmc_load(int fileh, int ver);
										/* ロード */
int FASTCALL bmc_setfile(int unit, char *fname);
										/* セット */
BOOL FASTCALL bmc_setwritep(int unit, BOOL writep);
										/* ライトプロテクト指定 */

/*
 *	主要ワーク
 */
extern BYTE bmc_datareg;
										/* $FD10   データレジスタ */
extern BYTE bmc_command;
										/* $FD11   コマンドレジスタ */
extern BYTE bmc_status;
										/* $FD12   ステータスレジスタ */
extern BYTE bmc_errorreg;
										/* $FD13   エラーステータスレジスタ */
extern WORD bmc_pagereg;
										/* $FD14-5 ページレジスタ */
extern WORD bmc_countreg;
										/* $FD16-7 ページカウントレジスタ */

extern WORD bmc_totalcnt;
										/* トータルカウンタ */
extern WORD bmc_nowcnt;
										/* カレントカウンタ */
extern BYTE bmc_unit;
										/* ユニット */
extern BYTE bmc_ready[BMC_UNITS_32];
										/* レディ状態 */
extern BOOL bmc_teject[BMC_UNITS_32];
										/* 一時イジェクト */
extern BOOL bmc_writep[BMC_UNITS_32];
										/* 書き込み禁止状態 */

extern char bmc_fname[BMC_UNITS_32][256+1];
										/* ファイル名 */
extern BOOL bmc_fwritep[BMC_UNITS_32];
										/* 書き込み禁止状態(ファイル単位) */
extern BYTE bmc_access[BMC_UNITS_32];
										/* アクセスLED */
extern BOOL bmc_enable;
										/* 有効・無効フラグ */
extern BOOL bmc_use;
										/* 使用フラグ */

#ifdef __cplusplus
}
#endif

#endif	/* XM7_VER == 1 && defined(BUBBLE) */

#endif	/* _bubble_h_ */

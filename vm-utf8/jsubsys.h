/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2009 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2009 Ryu Takegami
 *
 *	[ 日本語通信カード ]
 */

#ifndef _jsubsys_h_
#define _jsubsys_h_

#if (XM7_VER == 1 && defined(JSUB))

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	エントリ
 */
BOOL FASTCALL jsubsys_init(void);
										/* 日本語サブシステム 初期化 */
void FASTCALL jsubsys_cleanup(void);
										/* 日本語サブシステム クリーンアップ */
void FASTCALL jsubsys_reset(void);
										/* 日本語サブシステム リセット */
void FASTCALL jsubcpu_execline(void);
										/* 日本語サブCPU １行実行 */
void FASTCALL jsubcpu_exec(void);
										/* 日本語サブCPU 実行 */
void FASTCALL jsubcpu_nmi(void);
										/* 日本語サブCPU NMI割り込み要求 */
void FASTCALL jsubcpu_firq(void);
										/* 日本語サブCPU FIRQ割り込み要求 */
void FASTCALL jsubcpu_irq(void);
										/* 日本語サブCPU IRQ割り込み要求 */
BYTE FASTCALL jsubmem_readb(WORD addr);
										/* 日本語サブメモリ 読み出し */
BYTE FASTCALL jsubmem_readbnio(WORD addr);
										/* 日本語サブメモリ 読み出し(I/O無) */
void FASTCALL jsubmem_writeb(WORD addr, BYTE dat);
										/* 日本語サブメモリ 書き込み */
BOOL FASTCALL jsubsys_readb(WORD addr, BYTE *dat);
										/* 日本語サブI/O 読み出し */
BOOL FASTCALL jsubsys_writeb(WORD addr, BYTE dat);
										/* 日本語サブI/O 書き込み */
BOOL FASTCALL jsub_readb(WORD addr, BYTE *dat);
										/* 日本語サブI/F 読み出し */
BOOL FASTCALL jsub_writeb(WORD addr, BYTE dat);
										/* 日本語サブI/F 書き込み */
BOOL FASTCALL jsubsys_save(int fileh);
										/* 日本語サブシステム セーブ */
BOOL FASTCALL jsubsys_load(int fileh, int ver);
										/* 日本語サブシステム ロード */
extern BYTE FASTCALL jsub_readrcb(void);
										/* RCBリード */
extern void FASTCALL jsub_writercb(BYTE);
										/* RCBライト */
extern void FASTCALL jsub_clear_address(void);
										/* RCBアドレスカウンタクリア */

/*
 *	ワークエリア
 */
extern BOOL jsub_haltflag;
										/* 日本語サブシステムHALTフラグ */
extern BOOL jsub_available;
										/* 日本語サブ使用可能フラグ */
extern BOOL jsub_enable;
										/* 日本語サブ使用フラグ */
#ifdef __cplusplus
}
#endif

#endif

#endif	/* _jsubsys_h_ */

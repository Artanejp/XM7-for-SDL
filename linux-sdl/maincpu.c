/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ メインCPU ]
 */

#include "xm7.h"
#include "subctrl.h"
#include "keyboard.h"
#include "mainetc.h"
#include "tapelp.h"
#include "device.h"

/*
 *      グローバル ワーク
 */
cpu6809_t       maincpu;

/*
 *      プロトタイプ宣言
 */
void            main_reset(void);
void            main_line(void);
void            main_exec(void);

/*
 *      メインCPU
 *      初期化
 */
BOOL            FASTCALL
maincpu_init(void)
{
    maincpu.readmem = mainmem_readb;
    maincpu.writemem = mainmem_writeb;

    return TRUE;
}

/*
 *      メインCPU
 *      クリーンアップ
 */
void            FASTCALL
maincpu_cleanup(void)
{
    return;
}

/*
 *      メインCPU
 *      リセット
 */
void            FASTCALL
maincpu_reset(void)
{
    main_reset();
}

/*
 *      メインCPU
 *      １行実行
 */
void            FASTCALL
maincpu_execline(void)
{
    DWORD           cnt;

#if XM7_VER >= 2
    cnt = 0x100;
#else
    if (lowspeed_mode) {
	cnt = 0x09e;
    } else {
	cnt = 0x100;
    }
#endif

    /*
     * 1行実行 
     */
    main_line();

    /*
     * テープカウンタ処理 
     */
    if (tape_motor) {
	tape_subcnt += (DWORD) (maincpu.cycle << 4);
	if (tape_subcnt >= cnt) {
	    tape_subcnt -= cnt;
	    tape_count++;
	    if (tape_count == 0) {
		tape_count = 0xffff;
	    }
	}
    }
}

/*
 *      メインCPU
 *      実行
 */
void            FASTCALL
maincpu_exec(void)
{
    DWORD           cnt;

#if XM7_VER >= 2
    cnt = 0x100;
#else
    if (lowspeed_mode) {
	cnt = 0x09e;
    } else {
	cnt = 0x100;
    }
#endif

    /*
     * 実行 
     */
    main_exec();

    /*
     * テープカウンタ処理 
     */
    if (tape_motor) {
	tape_subcnt += (DWORD) (maincpu.cycle << 4);
	if (tape_subcnt >= cnt) {
	    tape_subcnt -= cnt;
	    tape_count++;
	    if (tape_count == 0) {
		tape_count = 0xffff;
	    }
	}
    }
}

/*
 *      メインCPU
 *      NMI割り込み設定
 */
void            FASTCALL
maincpu_nmi(void)
{
    /*
     * FM-77の1MB FDD対応(IRQ)で使われているらしい 
     */
    if (maincpu.intr & INTR_SLOAD) {
	maincpu.intr |= INTR_NMI;
    }
}

/*
 *      メインCPU
 *      FIRQ割り込み設定
 */
void            FASTCALL
maincpu_firq(void)
{
    /*
     * BREAKキー及び、サブCPUからのアテンション割り込み 
     */
    if (break_flag || subattn_flag) {
	maincpu.intr |= INTR_FIRQ;
    } else {
	maincpu.intr &= ~INTR_FIRQ;
    }
}

/*
 *      メインCPU
 *      IRQ割り込み設定
 */
void            FASTCALL
maincpu_irq(void)
{
#if XM7_VER == 1
    /*
     * FM-8モード時のIRQ割り込み設定 
     */
    if (fm_subtype == FMSUB_FM8) {
	if (mfd_irq_flag || txrdy_irq_flag || rxrdy_irq_flag
	    || syndet_irq_flag) {
	    maincpu.intr |= INTR_IRQ;
	} else {
	    maincpu.intr &= ~INTR_IRQ;
	}
	return;
    }
#endif

    /*
     * IRQ割り込み設定 
     */
    if ((key_irq_flag && !(key_irq_mask)) ||
	timer_irq_flag ||
	lp_irq_flag ||
	mfd_irq_flag || txrdy_irq_flag || rxrdy_irq_flag || syndet_irq_flag
	||
#if XM7_VER >= 3
	dma_irq_flag ||
#endif
	opn_irq_flag || whg_irq_flag || thg_irq_flag) {
	maincpu.intr |= INTR_IRQ;
    } else {
	maincpu.intr &= ~INTR_IRQ;
    }
}

/*
 *      メインCPU
 *      セーブ
 */
BOOL            FASTCALL
maincpu_save(int fileh)
{
    /*
     * プラットフォームごとのパッキング差を回避するため、分割 
     */
    if (!file_byte_write(fileh, maincpu.cc)) {
	return FALSE;
    }

    if (!file_byte_write(fileh, maincpu.dp)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.acc.d)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.x)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.y)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.u)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.s)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.pc)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.intr)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.cycle)) {
	return FALSE;
    }

    if (!file_word_write(fileh, maincpu.total)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      メインCPU
 *      ロード
 */
BOOL            FASTCALL
maincpu_load(int fileh, int ver)
{
    /*
     * バージョンチェック 
     */
    if (ver < 200) {
	return FALSE;
    }

    /*
     * プラットフォームごとのパッキング差を回避するため、分割 
     */
    if (!file_byte_read(fileh, &maincpu.cc)) {
	return FALSE;
    }

    if (!file_byte_read(fileh, &maincpu.dp)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.acc.d)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.x)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.y)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.u)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.s)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.pc)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.intr)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.cycle)) {
	return FALSE;
    }

    if (!file_word_read(fileh, &maincpu.total)) {
	return FALSE;
    }

    return TRUE;
}

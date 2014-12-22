/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2013 Ryu Takegami
 *
 *      [ サブCPU ]
 */

#include "xm7.h"
#include "subctrl.h"
#include "mainetc.h"
#include "device.h"
#include "event.h"

/*
 *      グローバル ワーク
 */
cpu6809_t subcpu;
BOOL disasm_sub_flag;
int disasm_sub_count;
char disasm_sub_buf[DISASM_BUF_SIZE];

/*
 *      プロトタイプ宣言
 */
void sub_reset(void);
void sub_line(void);
void sub_exec(void);
BOOL subcpu_event(void);

/*
 *      サブCPU
 *      初期化
 */
BOOL FASTCALL
subcpu_init(void)
{
	subcpu.readmem = submem_readb;
	subcpu.writemem = submem_writeb;
	disasm_sub_flag = FALSE;
	disasm_sub_count = 0;
	//memset(disasm_sub_buf, 0, DISASM_BUF_SIZE);
	return TRUE;
}

/*
 *      サブCPU
 *      クリーンアップ
 */
void FASTCALL
subcpu_cleanup(void)
{
	return;
}

/*
 *      サブCPU
 *      リセット
 */
void FASTCALL
subcpu_reset(void)
{
	sub_reset();
}

/*
 *      サブCPU
 *      １行実行
 */
void FASTCALL
subcpu_execline(void)
{
	sub_line();
}

/*
 *      サブCPU
 *      実行
 */
void FASTCALL
subcpu_exec(void)
{
	sub_exec();
}

/*
 *      サブCPU
 *      NMI割り込み設定
 */
void FASTCALL
subcpu_nmi(void)
{
	if (subcpu.intr & INTR_SLOAD) {
		subcpu.intr |= INTR_NMI;
	}
}

/*
 *      サブCPU
 *      FIRQ割り込み設定
 */
void FASTCALL
subcpu_firq(void)
{
	if (!key_irq_mask) {
		/*
		 * メインCPUにつながっている場合は、割り込みなし
		 */
		subcpu.intr &= ~INTR_FIRQ;
	}
	else {
		/*
		 * キー割り込みの有無で分かれる
		 */
		if (key_irq_flag) {
			subcpu.intr |= INTR_FIRQ;
		}
		else {
			subcpu.intr &= ~INTR_FIRQ;
		}
	}
}

/*
 *      サブCPU
 *      IRQ割り込み設定
 */
void FASTCALL
subcpu_irq(void)
{
	/*
	 * キャンセルIRQ
	 */
	if (subcancel_flag) {
		subcpu.intr |= INTR_IRQ;
	}
	else {
		subcpu.intr &= ~INTR_IRQ;
	}
}

/*
 *      サブCPU
 *      セーブ
 */
BOOL FASTCALL
subcpu_save(SDL_RWops * fileh)
{
	/*
	 * プラットフォームごとのパッキング差を回避するため、分割
	 */
	if (!file_byte_write(fileh, subcpu.cc)) {
		return FALSE;
	}

	if (!file_byte_write(fileh, subcpu.dp)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.x)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.y)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.u)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.s)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.pc)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.intr)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.cycle)) {
		return FALSE;
	}

	if (!file_word_write(fileh, subcpu.total)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *      サブCPU
 *      ロード
 */
BOOL FASTCALL
subcpu_load(SDL_RWops * fileh, int ver)
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
	if (!file_byte_read(fileh, &subcpu.cc)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &subcpu.dp)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.acc.d)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.x)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.y)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.u)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.s)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.pc)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.intr)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.cycle)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &subcpu.total)) {
		return FALSE;
	}

	return TRUE;
}

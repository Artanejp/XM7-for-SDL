/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ マルチページ ]
 */

#include <string.h>
#include "xm7.h"
#include "multipag.h"
#include "display.h"
#include "ttlpalet.h"
#include "subctrl.h"
#include "apalet.h"
#include "device.h"
/* XM7/SDL依存 */
#include <sdl.h>
#include <sdl_draw.h>

/*
 *	グローバル ワーク
 */
BYTE multi_page;						/* マルチページ ワーク */

/*
 *	マルチページ
 *	初期化
 */
BOOL FASTCALL multipag_init(void)
{
	return TRUE;
}

/*
 *	マルチページ
 *	クリーンアップ
 */
void FASTCALL multipag_cleanup(void)
{
}

/*
 *	マルチページ
 *	リセット
 */
void FASTCALL multipag_reset(void)
{
	multi_page = 0;
}

/*
 *	マルチページ
 *	１バイト読み出し
 */
BOOL FASTCALL multipag_readb(WORD addr, BYTE *dat)
{
#if XM7_VER == 1
	/* FM-8モードでは無効 */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	if (addr != 0xfd37) {
		return FALSE;
	}

	/* 常にFFが読み出される */
	*dat = 0xff;
	return TRUE;
}

/*
 *	マルチページ
 *	１バイト書き込み
 */
BOOL FASTCALL multipag_writeb(WORD addr, BYTE dat)
{
#if XM7_VER == 1
	/* FM-8モードでは無効 */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	if (addr != 0xfd37) {
		return FALSE;
	}

	if ((BYTE)(multi_page & 0x70) == (BYTE)(dat & 0x70)) {
		/* 表示状態に変更がない場合はデータ記憶だけして帰る */
		multi_page = dat;
		return TRUE;
	}

	/* データ記憶 */
	multi_page = dat;

	/* パレット再設定 */
	if (crt_flag) {
#if XM7_VER >= 2
#if XM7_VER >= 3
		if (!(screen_mode & SCR_ANALOG)) {
#else
		if (!mode320) {
#endif
			ttlpalet_notify();
		}
		else {
			apalet_notify();
		}
#else
		ttlpalet_notify();
#endif
	}

	return TRUE;
}

/*
 *	マルチページ
 *	セーブ
 */
BOOL FASTCALL multipag_save(int fileh)
{
	return file_byte_write(fileh, multi_page);
}

/*
 *	マルチページ
 *	ロード
 */
BOOL FASTCALL multipag_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	return file_byte_read(fileh, &multi_page);
}

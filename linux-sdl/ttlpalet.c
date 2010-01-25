/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ TTLパレット(MB15021) ]
 */

#include <string.h>
#include "xm7.h"
#include "ttlpalet.h"
#include "device.h"
#include "display.h"
#include "subctrl.h"

/*
 *	グローバル ワーク
 */
BYTE ttl_palet[8];			/* TTLパレットデータ */

/*
 *	TTLパレット
 *	初期化
 */
BOOL FASTCALL ttlpalet_init(void)
{
	return TRUE;
}

/*
 *	TTLパレット
 *	クリーンアップ
 */
void FASTCALL ttlpalet_cleanup(void)
{
}

/*
 *	TTLパレット
 *	リセット
 */
void FASTCALL ttlpalet_reset(void)
{
	int i;
	
	/* すべての色を初期化 */
	for (i=0; i<8; i++) {
		ttl_palet[i] = (BYTE)(i | 0x08);
	}

	/* 通知 */
	ttlpalet_notify();
}

/*
 *	TTLパレット
 *	１バイト読み出し
 */
BOOL FASTCALL ttlpalet_readb(WORD addr, BYTE *dat)
{
#if XM7_VER == 1
	/* FM-8モードでは無効 */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	/* 範囲チェック、読み出し */
	if ((addr >= 0xfd38) && (addr <= 0xfd3f)) {
		ASSERT((WORD)(addr - 0xfd38) <= 7);

		/* 上位ニブルは0xF0が入る */
#if XM7_VER >= 3
		if (fm7_ver == 3) {
			/* FM77AV40EXでは下位3ビットのみ有効 */
			*dat = (BYTE)((ttl_palet[(WORD)(addr - 0xfd38)] & 0x07) | 0xf0);
		}
		else {
			/* FM-7/FM77AV(MB15021)では下位4ビットが有効 */
			*dat = (BYTE)(ttl_palet[(WORD)(addr - 0xfd38)] | 0xf0);
		}
#else
		/* FM-7/FM77AV(MB15021)では下位4ビットが有効 */
		*dat = (BYTE)(ttl_palet[(WORD)(addr - 0xfd38)] | 0xf0);
#endif

		return TRUE;
	}

	return FALSE;
}

/*
 *	TTLパレット
 *	１バイト書き込み
 */
BOOL FASTCALL ttlpalet_writeb(WORD addr, BYTE dat)
{
	int no;

#if XM7_VER == 1
	/* FM-8モードでは無効 */
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	/* 範囲チェック、書き込み */
	if ((addr >= 0xfd38) && (addr <= 0xfd3f)) {
		no = addr - 0xfd38;
		if (ttl_palet[no] != (dat & 0x0f)) {
			ttl_palet[no] = (BYTE)(dat & 0x0f);

			/* 通知 */
#if XM7_VER >= 2
#if XM7_VER >= 3
			if (!(screen_mode & SCR_ANALOG)) {
#else
			if (!mode320) {
#endif
				ttlpalet_notify();
			}
#else
			ttlpalet_notify();
#endif
		}

		return TRUE;
	}

	return FALSE;
}

/*
 *	TTLパレット
 *	セーブ
 */
BOOL FASTCALL ttlpalet_save(int fileh)
{
	return file_write(fileh, ttl_palet, 8);
}

/*
 *	TTLパレット
 *	ロード
 */
BOOL FASTCALL ttlpalet_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	return file_read(fileh, ttl_palet, 8);
}

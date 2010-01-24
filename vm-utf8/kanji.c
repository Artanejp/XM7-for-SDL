/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2009 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2009 Ryu Takegami
 *
 *	[ 漢字ROM (非漢字・第一水準・第二水準) ]
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "kanji.h"
#include "subctrl.h"
#include "device.h"
#if XM7_VER >= 3
#include "jcard.h"
#endif

/*
 *	グローバル ワーク
 */
WORD kanji_addr;						/* アドレスレジスタ(共通) */
BYTE *kanji_rom;						/* 第１水準ROM */
#if XM7_VER >= 3
BYTE *kanji_rom2;						/* 第２水準ROM */
#endif

/*
 *	漢字ROM
 *	初期化
 */
BOOL FASTCALL kanji_init(void)
{
	/* メモリ確保 */
#if XM7_VER >= 3
	kanji_rom = (BYTE *)malloc(0x40000);
#else
	kanji_rom = (BYTE *)malloc(0x20000);
#endif
	if (!kanji_rom) {
		return FALSE;
	}

	/* 第1水準 ファイル読み込み */
	if (!file_load(KANJI_ROM, kanji_rom, 0x20000)) {
#if XM7_VER >= 2
		return FALSE;
#else
		/* 豆腐作成 */
		memset(kanji_rom, 0xff, 0x20000);
#endif
	}

#if XM7_VER >= 3
	/* 第2水準 ファイル読み込み */
	kanji_rom2 = (BYTE *)(kanji_rom + 0x20000);
	if (!file_load(KANJI_ROM2, kanji_rom2, 0x20000)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	漢字ROM
 *	クリーンアップ
 */
void FASTCALL kanji_cleanup(void)
{
	ASSERT(kanji_rom);
	if (kanji_rom) {
		free(kanji_rom);
		kanji_rom = NULL;
#if XM7_VER >= 3
		kanji_rom2 = NULL;
#endif
	}
}

/*
 *	漢字ROM
 *	リセット
 */
void FASTCALL kanji_reset(void)
{
	kanji_addr = 0;
}

/*
 *	漢字ROM
 *	１バイト読み出し
 */
BOOL FASTCALL kanji_readb(WORD addr, BYTE *dat)
{
	int offset;

	switch (addr) {
		/* アドレス(第1・第2共通)上位・下位 */
		case 0xfd20:
		case 0xfd21:
#if XM7_VER >= 3
		case 0xfd2c:
		case 0xfd2d:
#endif
			*dat = 0xff;
			return TRUE;

		/* 第1水準データ */
		case 0xfd22:		/* 第1データLEFT */
		case 0xfd23:		/* 第1データRIGHT */
#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
#if XM7_VER >= 3
			if (subkanji_flag) {
#else
			if (subkanji_flag && (fm_subtype == FMSUB_FM77)) {
#endif
				*dat = 0xff;
			}
			else {
				offset = kanji_addr << 1;
				*dat = kanji_rom[offset + (addr & 1)];
			}
#else
			offset = kanji_addr << 1;
			*dat = kanji_rom[offset + (addr & 1)];
#endif
			return TRUE;

#if XM7_VER >= 3
		/* 第2水準データ */
		case 0xfd2e:		/* 第2データLEFT */
		case 0xfd2f:		/* 第2データRIGHT */
			if ((fm7_ver < 3) || subkanji_flag) {
				*dat = 0xff;
			}
			else {
				offset = kanji_addr << 1;
				*dat = kanji_rom2[offset + (addr & 1)];
			}
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	漢字ROM
 *	１バイト書き込み
 */
BOOL FASTCALL kanji_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
		/* アドレス(第1・第2共通)上位 */
#if XM7_VER >= 3
		case 0xfd2c:
			if (fm7_ver < 3) {
				return TRUE;
			}
#endif
		case 0xfd20:
			kanji_addr &= 0x00ff;
			kanji_addr |= (WORD)(dat << 8);
			return TRUE;

		/* アドレス(第1・第2共通)下位 */
#if XM7_VER >= 3
		case 0xfd2d:
			if (fm7_ver < 3) {
				return TRUE;
			}
#endif
		case 0xfd21:
			kanji_addr &= 0xff00;
			kanji_addr |= dat;
			return TRUE;

		/* データ */
		case 0xfd22:
		case 0xfd23:
#if XM7_VER >= 3
		case 0xfd2f:
#endif
			return TRUE;

#if XM7_VER >= 3
		/* 日本語カード バンクセレクト */
		case 0xfd2e:
			if (fm7_ver < 3) {
				return TRUE;
			}

			/* bit7:学習RAM */
			if (dat & 0x80) {
				dicram_en = TRUE;
			}
			else {
				dicram_en = FALSE;
			}

			/* bit6:辞書ROMイネーブル */
			if (dat & 0x40) {
				dicrom_en = TRUE;
			}
			else {
				dicrom_en = FALSE;
			}

			/* bit0-5:辞書ROMバンク */
			dicrom_bank = (BYTE)(dat & 0x3f);
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	漢字ROM
 *	セーブ
 */
BOOL FASTCALL kanji_save(int fileh)
{
	if (!file_word_write(fileh, kanji_addr)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	漢字ROM
 *	ロード
 */
BOOL FASTCALL kanji_load(int fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_word_read(fileh, &kanji_addr)) {
		return FALSE;
	}

	return TRUE;
}

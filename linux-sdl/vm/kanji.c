/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ 漢字ROM (非漢字・第一水準・第二水準) ]
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
 *      グローバル ワーク
 */
WORD            kanji_addr;	/* アドレスレジスタ(共通) */
BYTE           *kanji_rom;	/* 第１水準ROM */
#if XM7_VER >= 3
BYTE           *kanji_rom_jis78;		/* 第１水準ROM (JIS78準拠) */
BYTE           *kanji_rom2;	/* 第２水準ROM */
#endif

/*
*	スタティック ワーク
*/
#if XM7_VER == 1
static BOOL kanjirom_available;			/* 漢字ROM使用可能フラグ */
#endif

/*
 *	プロトタイプ宣言
 */
static void FASTCALL kanji_make_jis78(BYTE *rom);



/*
 *      漢字ROM
 *      初期化
 */
BOOL            FASTCALL
kanji_init(void)
{
    /*
     * メモリ確保 
     */
#if XM7_VER >= 3
   kanji_rom_jis78 = (BYTE *)malloc(0x6000);
   if (!kanji_rom_jis78) {
	return FALSE;
   }
    kanji_rom = (BYTE *) malloc(0x40000);
#else
    kanji_rom = (BYTE *) malloc(0x20000);
#endif
    if (!kanji_rom) {
	return FALSE;
    }

    /*
     * 第1水準 ファイル読み込み 
     */
    if (!file_load(KANJI_ROM, kanji_rom, 0x20000)) {
#if XM7_VER >= 2
	return FALSE;
#else
	kanjirom_available = FALSE;
	}
	else {
	kanjirom_available = TRUE;
#endif
    }
#if XM7_VER >= 3
    /*
     * 第2水準 ファイル読み込み 
     */
    kanji_rom2 = (BYTE *) (kanji_rom + 0x20000);
    if (!file_load(KANJI_ROM2, kanji_rom2, 0x20000)) {
	return FALSE;
    }
#endif
	/* JIS78準拠漢字ROMデータ生成 */
#if XM7_VER >= 3
	memcpy(kanji_rom_jis78, kanji_rom, 0x6000);
	kanji_make_jis78(kanji_rom_jis78);
#else
	kanji_make_jis78(kanji_rom);
#endif
    return TRUE;
}

/*
 *      漢字ROM
 *      クリーンアップ
 */
void            FASTCALL
kanji_cleanup(void)
{
    ASSERT(kanji_rom);
#if XM7_VER >= 3
	ASSERT(kanji_rom_jis78);

	if (kanji_rom_jis78) {
		free(kanji_rom_jis78);
		kanji_rom_jis78 = NULL;
	}
#endif
    if (kanji_rom) {
	free(kanji_rom);
	kanji_rom = NULL;
#if XM7_VER >= 3
	kanji_rom2 = NULL;
#endif
    }
}

/*
 *      漢字ROM
 *      リセット
 */
void            FASTCALL
kanji_reset(void)
{
    kanji_addr = 0;
}

/*
 *      漢字ROM
 *      １バイト読み出し
 */
BOOL            FASTCALL
kanji_readb(WORD addr, BYTE * dat)
{
    int             offset;

    switch (addr) {
	/*
	 * アドレス(第1・第2共通)上位・下位 
	 */
    case 0xfd20:
    case 0xfd21:
#if XM7_VER >= 3
    case 0xfd2c:
    case 0xfd2d:
#endif
	*dat = 0xff;
	return TRUE;

	/*
	 * 第1水準データ 
	 */
    case 0xfd22:		/* 第1データLEFT */
    case 0xfd23:		/* 第1データRIGHT */
#if XM7_VER == 1
			if (!kanjirom_available) {
				*dat = 0xff;
				return TRUE;
			}
#endif

#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
#if XM7_VER >= 3
	if (subkanji_flag) {
#else
	if (subkanji_flag && (fm_subtype == FMSUB_FM77)) {
#endif
	    *dat = 0xff;
	return TRUE;
			}
#endif

			offset = kanji_addr << 1;
			if ((fm7_ver <= 2) && (offset < 0x6000)) {
				/* FM-7/FM77AVモード時の$0000〜$5FFFはJIS78準拠 */
#if XM7_VER >= 3
				*dat = kanji_rom_jis78[offset + (addr & 1)];
#else
				*dat = kanji_rom[offset + (addr & 1)];
#endif
			}
			else if ((fm7_ver <= 1) && (offset < 0x8000)) {
				/* FM-7モード時の$6000〜$7FFFは未定義領域 */
				*dat = (BYTE)(addr & 1);
 
	} else {
  	/* 通常領域 */
	    *dat = kanji_rom[offset + (addr & 1)];
	}
	return TRUE;

#if XM7_VER >= 3
	/*
	 * 第2水準データ 
	 */
    case 0xfd2e:		/* 第2データLEFT */
    case 0xfd2f:		/* 第2データRIGHT */
	if ((fm7_ver < 3) || subkanji_flag) {
	    *dat = 0xff;
	} else {
	    offset = kanji_addr << 1;
	    *dat = kanji_rom2[offset + (addr & 1)];
	}
	return TRUE;
#endif
    }

    return FALSE;
}
   
   
/*
 *      漢字ROM
 *      １バイト書き込み
 */
BOOL            FASTCALL
kanji_writeb(WORD addr, BYTE dat)
{
    switch (addr) {
	/*
	 * アドレス(第1・第2共通)上位 
	 */
#if XM7_VER >= 3
    case 0xfd2c:
	if (fm7_ver < 3) {
	    return TRUE;
	}
#endif
    case 0xfd20:
	kanji_addr &= 0x00ff;
	kanji_addr |= (WORD) (dat << 8);
	return TRUE;

	/*
	 * アドレス(第1・第2共通)下位 
	 */
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

	/*
	 * データ 
	 */
    case 0xfd22:
    case 0xfd23:
#if XM7_VER >= 3
    case 0xfd2f:
#endif
	return TRUE;

#if XM7_VER >= 3
	/*
	 * 日本語カード バンクセレクト 
	 */
    case 0xfd2e:
	if (fm7_ver < 3) {
	    return TRUE;
	}

	/*
	 * bit7:学習RAM 
	 */
	if (dat & 0x80) {
	    dicram_en = TRUE;
	} else {
	    dicram_en = FALSE;
	}

	/*
	 * bit6:辞書ROMイネーブル 
	 */
	if (dat & 0x40) {
	    dicrom_en = TRUE;
	} else {
	    dicrom_en = FALSE;
	}

	/*
	 * bit0-5:辞書ROMバンク 
	 */
	dicrom_bank = (BYTE) (dat & 0x3f);
	return TRUE;
#endif
    }

    return FALSE;
}

/*
 *	漢字ROM
 *	JIS78準拠領域作成
 */
static void FASTCALL kanji_make_jis78(BYTE *rom)
{
   static const DWORD jis78_table[] = {
		0xffffffff, 0x00000001, 0xffff8001, 0xfc00ffff,
		0x00000001, 0x00000001, 0xfe000001, 0x00000001,
		0xffffffff, 0x80000000, 0xffffffff, 0xf8000001,
		0xfff00000, 0xff800000, 0xffffffff, 0xfffc0000,
		0xffffffff, 0x00000000, 0xffffffff, 0xf8000001,
		0x00000000, 0x00000000, 0xfe000001, 0x0001fffc,
	};
	DWORD addr;
	DWORD i;

	/* JIS78準拠漢字ROMでは未定義になっている部分を未定義とする */
	for (addr = 0; addr < 0x6000; addr += 32) {
		if (jis78_table[addr >> 10] & (1 << (addr >> 5))) {
			/* 未定義フォント作成 */
			for (i = 0; i < 32; i += 2) {
				rom[addr + i + 0] = 0x00;
				rom[addr + i + 1] = 0x01;
			}
		}
	}
}

  
   
/*
 *      漢字ROM
 *      セーブ
 */
BOOL FASTCALL kanji_save(int fileh)
{
    if (!file_word_write(fileh, kanji_addr)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      漢字ROM
 *      ロード
 */
BOOL            FASTCALL
kanji_load(int fileh, int ver)
{
    /*
     * バージョンチェック 
     */
    if (ver < 200) {
	return FALSE;
    }

    if (!file_word_read(fileh, &kanji_addr)) {
	return FALSE;
    }

    return TRUE;
}

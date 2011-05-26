/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ 日本語カード / 拡張サブシステムROM ]
 */

#if XM7_VER >= 3

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "mmr.h"
#include "device.h"
#include "kanji.h"
#include "jcard.h"

/*
 *      グローバル ワーク
 */
BYTE           *dicrom;		/* 辞書ROM $40000 */
BYTE           *dicram;		/* 学習RAM $2000 */
BYTE           *extram_b;	/* 拡張RAM $10000 */

BYTE            dicrom_bank;	/* 辞書ROMバンク番号 */
BOOL            extrom_sel;	/* 辞書ROM/拡張ROM選択フラグ */
BOOL            dicrom_en;	/* 辞書ROMアクティブ */
BOOL            dicram_en;	/* 学習RAMアクティブ */

BYTE           *extrom;		/* 拡張ROM $20000 */


/*
 *      日本語カード
 *      初期化
 */
BOOL            FASTCALL
jcard_init(void)
{
    /*
     * ワークエリア初期化 
     */
    extram_b = NULL;
    dicrom = NULL;
    dicram = NULL;
    extrom = NULL;

    /*
     * 日本語空間 拡張RAM 
     */
    extram_b = (BYTE *) malloc(0x10000);
    if (extram_b == NULL) {
	return FALSE;
    }

    /*
     * 辞書ROM 
     */
    dicrom = (BYTE *) malloc(0x40000);
    if (dicrom == NULL) {
	return FALSE;
    }
    if (!file_load(DICT_ROM, dicrom, 0x40000)) {
	return FALSE;
    }

    /*
     * 学習RAM読み込み 
     */
    dicram = (BYTE *) malloc(0x2000);
    if (dicram == NULL) {
	return FALSE;
    }
    if (!file_load(DICT_RAM, dicram, 0x2000)) {
	/*
	 * ファイルが存在しない。初期化 
	 */
	memset(dicram, 0xff, 0x2000);
    }

    /*
     * 拡張ROM 
     */
    extrom = (BYTE *) malloc(0x20000);
    if (extrom == NULL) {
	return FALSE;
    }
    memset(extrom, 0xff, 0x20000);
    if (!file_load(EXTSUB_ROM, extrom, 0xc000)) {
	return FALSE;
    } else {
	/*
	 * バンク56〜63にBASIC
	 * ROM、隠しブートROMの内容をコピー 
	 */
	memcpy(&extrom[0x18000], basic_rom, 0x7c00);
	if (available_mmrboot) {
	   memcpy(&extrom[0x1fe00], boot_mmr, 0x200);
	}
	else {
	   memcpy(&extrom[0x1fe00], &init_rom[0x1a00], 0x1e0);
	   memset(&extrom[0x1ffe0], 0, 32);
	   extrom[0x1fffe] = 0xfe;
	   extrom[0x1ffff] = 0x00;
	}
    }

    return TRUE;
}

/*
 *      日本語カード
 *      クリーンアップ
 */
void            FASTCALL
jcard_cleanup(void)
{
    ASSERT(extram_b);
    ASSERT(dicram);
    ASSERT(dicrom);
    ASSERT(extrom);

    /*
     * 初期化途中で失敗した場合を考慮 
     */
    if (extrom) {
	free(extrom);
    }
    if (dicram) {
	/*
	 * 学習RAMの内容をファイルに書き出す 
	 */
	file_save(DICT_RAM, dicram, 0x2000);
	free(dicram);
    }
    if (dicrom) {
	free(dicrom);
    }
    if (extram_b) {
	free(extram_b);
    }
}

/*
 *      日本語カード
 *      リセット
 */
void            FASTCALL
jcard_reset(void)
{
    dicrom_bank = 0;
    dicram_en = FALSE;
    dicrom_en = FALSE;
    extrom_sel = FALSE;
}

/*
 *      日本語カード
 *      １バイト読み込み
 */
BYTE            FASTCALL
jcard_readb(WORD addr)
{
    DWORD           dicrom_addr;

    /*
     * FM77AV40EXのみサポート 
     */
    if (fm7_ver < 3) {
	return 0xff;
    }

    /*
     * $28000-$29FFF : 学習RAM 
     */
    if ((addr >= 0x8000) && (addr < 0xa000)) {
	if (dicram_en) {
	    return dicram[addr - 0x8000];
	}
    }

    /*
     * $2E000-$2EFFF : 辞書ROM or 拡張ROM 
     */
    if ((addr >= 0xe000) && (addr < 0xf000)) {
	/*
	 * 辞書ROMが有効か 
	 */
	if (dicrom_en) {
	    addr &= (WORD) 0x0fff;
	    dicrom_addr = (dicrom_bank << 12);

	    /*
	     * 拡張ROMが有効か 
	     */
	    if (extrom_sel) {
		/* バンク0〜31 : 第1水準漢字(JIS78準拠) */
		if (dicrom_bank < 6) {
			return kanji_rom_jis78[addr | dicrom_addr];
		}
		if (dicrom_bank < 8) {
			return (BYTE)(addr & 1);
		}
	       
		if (dicrom_bank < 32) {
		    return kanji_rom[addr | dicrom_addr];
		}
		/*
		 * バンク32〜43 :
		 * 拡張サブシステムROM(extsub.rom) 
		 */
		/*
		 * バンク56〜63 : F-BASIC V3.0 ROM ($8000-$EFFF) 
		 */
		/*
		 * バンク63 : DOSモードBOOT ($FE00-$FFDF) 
		 */
		/*
		 * バンク63 : 割り込みベクタ領域 ($FFE0-$FFFF) 
		 */
		return extrom[addr | (dicrom_addr - 0x20000)];
	    } else {
		/*
		 * 辞書ROM 
		 */
		return dicrom[addr | dicrom_addr];
	    }
	}
    }

    /*
     * 拡張RAM 
     */
    return extram_b[addr];
}

/*
 *      日本語カード
 *      １バイト書き込み
 */
void            FASTCALL
jcard_writeb(WORD addr, BYTE dat)
{
    /*
     * FM77AV40EXのみサポート 
     */
    if (fm7_ver < 3) {
	return;
    }

    /*
     * $28000-$29FFF : 学習RAM 
     */
    if ((addr >= 0x8000) && (addr < 0xa000)) {
	if (dicram_en) {
	    dicram[addr - 0x8000] = dat;
	    return;
	}
    }

    /*
     * 拡張RAM
     * (辞書ROMの選択状態に関わらず書き込み可能) 
     */
    extram_b[addr] = dat;
}

/*
 *      日本語カード
 *      セーブ
 */
BOOL            FASTCALL
jcard_save(int fileh)
{
    if (!file_byte_write(fileh, dicrom_bank)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, extrom_sel)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, dicrom_en)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, dicram_en)) {
	return FALSE;
    }
    if (!file_write(fileh, extram_b, 0x10000)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      日本語カード
 *      ロード
 */
BOOL            FASTCALL
jcard_load(int fileh, int ver)
{
    /*
     * バージョンチェック 
     */
    if (ver < 800) {
	dicrom_bank = 0;
	extrom_sel = FALSE;
	dicram_en = FALSE;
	dicrom_en = FALSE;
	return TRUE;
    }

    if (!file_byte_read(fileh, &dicrom_bank)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &extrom_sel)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &dicrom_en)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &dicram_en)) {
	return FALSE;
    }
    if (!file_read(fileh, extram_b, 0x10000)) {
	return FALSE;
    }

    return TRUE;
}

#endif				/* XM7_VER >= 3 */

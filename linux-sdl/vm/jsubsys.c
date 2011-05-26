/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ 日本語通信カード ]
 */

#if (XM7_VER == 1 && defined(JSUB))

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "jsubsys.h"
#include "kanji.h"
#include "mainetc.h"
#include "device.h"
#include "event.h"

/*
 *      グローバル ワーク
 */
cpu6809_t       jsubcpu;
BYTE           *jsubrom;	/* 日本語サブモニタROM */
BYTE           *jdicrom;	/* 辞書ROM(日本語通信カード) */
BYTE           *jsub_sram;	/* ワークSRAM */
BOOL            jsub_available;	/* 日本語サブシステム使用可能フラグ 
				 */
BOOL            jsub_enable;	/* 日本語サブシステム使用フラグ 
				 */
BOOL            jsub_haltflag;	/* 日本語サブシステムHALTフラグ 
				 */
BYTE            jsub_dicbank;	/* 辞書ROMバンク */
BYTE            jsub_address;	/* RCBアドレスカウンタ */

/*
 *      プロトタイプ宣言
 */
void            jsub_reset(void);
void            jsub_line(void);
void            jsub_exec(void);
BOOL            jsubcpu_event(void);

/*
 *      日本語サブCPU
 *      初期化
 */
BOOL            FASTCALL
jsubsys_init(void)
{
    /*
     * 一度、全てクリア 
     */
    jsubrom = NULL;
    jdicrom = NULL;
    jsub_sram = NULL;
    jsub_available = TRUE;
    jsub_enable = TRUE;
    jsub_address = 0;

    /*
     * メモリ確保 
     */
    jsubrom = (BYTE *) malloc(0x4000);
    if (jsubrom == NULL) {
	return FALSE;
    }
    jdicrom = (BYTE *) malloc(0x40000);
    if (jdicrom == NULL) {
	return FALSE;
    }
    jsub_sram = (BYTE *) malloc(0x2000);
    if (jsub_sram == NULL) {
	return FALSE;
    }

    /*
     * ROMファイル読み込み 
     */
    if (!file_load(JSUBSYS_ROM, jsubrom, 0x4000)) {
	jsub_available = FALSE;
    }
    if (!file_load(JSUBDIC_ROM, jdicrom, 0x40000)) {
	jsub_available = FALSE;
    }

    /*
     * メモリアクセス関数設定 
     */
    jsubcpu.readmem = jsubmem_readb;
    jsubcpu.writemem = jsubmem_writeb;

    return TRUE;
}

/*
 *      日本語サブCPU
 *      クリーンアップ
 */
void            FASTCALL
jsubsys_cleanup(void)
{
    ASSERT(jsubrom);
    ASSERT(jdicrom);
    ASSERT(jsub_sram);

    if (jsubrom) {
	free(jsubrom);
    }
    if (jdicrom) {
	free(jdicrom);
    }
    if (jsub_sram) {
	free(jsub_sram);
    }
}

/*
 *      日本語サブCPU
 *      リセット
 */
void            FASTCALL
jsubsys_reset(void)
{
    jsub_address = 0;
    jsub_haltflag = FALSE;
    jsub_kanji_addr = 0;

    if (jsub_available) {
	jsub_reset();
    }
}

/*
 *      日本語サブCPU
 *      １行実行
 */
void            FASTCALL
jsubcpu_execline(void)
{
    if (jsub_available && jsub_enable) {
	jsub_line();
    }
}

/*
 *      日本語サブCPU
 *      実行
 */
void            FASTCALL
jsubcpu_exec(void)
{
    if (jsub_available && jsub_enable) {
	jsub_exec();
    }
}

/*
 *      日本語サブCPU
 *      NMI割り込み設定
 */
void            FASTCALL
jsubcpu_nmi(void)
{
}

/*
 *      日本語サブCPU
 *      FIRQ割り込み設定
 */
void            FASTCALL
jsubcpu_firq(void)
{
}

/*
 *      日本語サブCPU
 *      IRQ割り込み設定
 */
void            FASTCALL
jsubcpu_irq(void)
{
}

/*
 *      日本語サブCPUメモリ
 *      １バイト取得
 */
BYTE            FASTCALL
jsubmem_readb(WORD addr)
{
    /*
     * 日本語サブモニタROM 
     */
    if (addr >= 0xc000) {
	return jsubrom[addr - 0xc000];
    }

    /*
     * 辞書ROM 
     */
    if ((addr >= 0xa000) && (addr <= 0xafff)) {
	return jdicrom[(addr & 0xfff) | (jsub_dicbank << 12)];
    }

    /*
     * SRAM 
     */
    if ((addr >= 0x8000) && (addr <= 0x9ffe)) {
	return jsub_sram[addr - 0x8000];
    }

    /*
     * 辞書バンク・同期フラグレジスタ 
     */
    if (addr == 0x9fff) {
	return jsub_dicbank;
    }

    return 0xff;
}

/*
 *      日本語サブCPUメモリ
 *      １バイト取得(I/Oなし)
 */
BYTE            FASTCALL
jsubmem_readbnio(WORD addr)
{
    /*
     * 日本語サブモニタROM 
     */
    if (addr >= 0xc000) {
	return jsubrom[addr - 0xc000];
    }

    /*
     * 辞書ROM 
     */
    if ((addr >= 0xa000) && (addr <= 0xafff)) {
	return jdicrom[(addr & 0xfff) | (jsub_dicbank << 12)];
    }

    /*
     * SRAM 
     */
    if ((addr >= 0x8000) && (addr <= 0x9ffe)) {
	return jsub_sram[addr - 0x8000];
    }

    /*
     * 辞書バンク・同期フラグレジスタ 
     */
    if (addr == 0x9fff) {
	return (BYTE) jsub_dicbank;
    }

    ASSERT(FALSE);
    return 0xff;
}

/*
 *      日本語サブCPUメモリ
 *      １バイト書き込み
 */
void            FASTCALL
jsubmem_writeb(WORD addr, BYTE dat)
{
    /*
     * 辞書バンク・同期フラグレジスタ 
     */
    if (addr == 0x9fff) {
	if (dat & 0x80) {
	    jsub_haltflag = FALSE;
	} else {
	    jsub_haltflag = TRUE;
	}

	jsub_dicbank = (BYTE) (dat & 0x3f);
	return;
    }

    /*
     * SRAM 
     */
    if ((addr >= 0x8000) && (addr <= 0x9ffe)) {
	jsub_sram[addr - 0x8000] = dat;
	return;
    }

    return;
}

/*
 *      日本語サブシステム
 *      メイン側からのRCB１バイト読み込み
 */
BYTE            FASTCALL
jsub_readrcb(void)
{
    return jsub_sram[0x1f00 | (jsub_address++)];
}

/*
 *      日本語サブシステム
 *      メイン側からのRCB１バイト書き込み
 */
void            FASTCALL
jsub_writercb(BYTE dat)
{
    jsub_sram[0x1f00 | (jsub_address++)] = dat;
}

/*
 *      日本語サブシステム
 *      アドレスカウンタクリア
 */
void            FASTCALL
jsub_clear_address(void)
{
    jsub_address = 0;
}

/*
 *      日本語サブシステムインタフェース
 *      １バイト読み出し
 */
BOOL            FASTCALL
jsub_readb(WORD addr, BYTE * dat)
{
    int             offset;
   /* FM-8モード時・日本語通信カード無効時は無効 */
   if ((fm_subtype == FMSUB_FM8) || !jsub_enable) {
      return FALSE;
   }
    switch (addr) {
	/*
	 * 日本語サブシステム同期フラグ 
	 */
    case 0xfd28:
	*dat = 0xff;
	if (jsub_haltflag) {
	    *dat &= ~0x80;
	}
	return TRUE;

	/*
	 * RCBデータ読み込み 
	 */
    case 0xfd29:
	if (jsub_haltflag) {
	   *dat = jsub_readrcb();
	} else {
	    *dat = 0xff;
	}
	return TRUE;
	/* 第1水準データ */
	case 0xfd2a:		/* 第1データLEFT */
	case 0xfd2b:		/* 第1データRIGHT */
         if (fm_subtype == FMSUB_FM77) {
		offset = jsub_kanji_addr << 1;
		if ((offset >= 0x6000) && (offset < 0x8000)) {
		/* $6000〜$7FFFは未定義領域 */
			*dat = (BYTE)(addr & 1);
		}
		else {
		/* 通常領域 */
			*dat = kanji_rom[offset + (addr & 1)];
			}
			return TRUE;
		}
	return FALSE;
    }
    return FALSE;
}

/*
 *      日本語サブシステムインタフェース
 *      １バイト書き込み
 */
BOOL            FASTCALL
jsub_writeb(WORD addr, BYTE dat)
{
	/* FM-8モード時・日本語通信カード無効時は無効 */
	if ((fm_subtype == FMSUB_FM8) || !jsub_enable) {
		return FALSE;
	}

       switch (addr) {
	/* アドレス上位 */
	case 0xfd28:
	  jsub_kanji_addr &= 0x00ff;
	  jsub_kanji_addr |= (WORD)(dat << 8);
	  return TRUE;
	/* アドレス下位 */
	case 0xfd29:
	  jsub_kanji_addr &= 0xff00;
	  jsub_kanji_addr |= dat;
	  return TRUE;
	/*
	 * 日本語サブシステム同期フラグ 
	 */
	case 0xfd2a:
	  if (dat & 0x80) {
	     jsub_haltflag = FALSE;
	  }
	  else {
	     jsub_haltflag = TRUE;
	     jsub_clear_address();
	  }
	return TRUE;

	/*
	 * RCBデータ書き込み 
	 */
	case 0xfd2b:
	  if (jsub_haltflag) {
	     jsub_writercb(dat);
	  }
	  return TRUE;
       }
   return FALSE;
}

/*
 *      日本語サブシステム
 *      セーブ
 */
BOOL FASTCALL jsubsys_save(int fileh)
{
    /*
     * プラットフォームごとのパッキング差を回避するため、分割 
     */
    if (!file_byte_write(fileh, jsubcpu.cc)) {
	return FALSE;
    }

    if (!file_byte_write(fileh, jsubcpu.dp)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.acc.d)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.x)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.y)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.u)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.s)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.pc)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.intr)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.cycle)) {
	return FALSE;
    }

    if (!file_word_write(fileh, jsubcpu.total)) {
	return FALSE;
    }

    if (!file_bool_write(fileh, jsub_enable)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, jsub_haltflag)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, jsub_dicbank)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, jsub_address)) {
	return FALSE;
    }
    if (!file_write(fileh, jsub_sram, 0x2000)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      日本語サブシステム
 *      ロード
 */
BOOL            FASTCALL
jsubsys_load(int fileh, int ver)
{
    /*
     * バージョンチェック 
     */
    if (ver < 200) {
	return FALSE;
    }

    if (ver >= 301 && ver <= 499) {
	/*
	 * プラットフォームごとのパッキング差を回避するため、分割 
	 */
	if (!file_byte_read(fileh, &jsubcpu.cc)) {
	    return FALSE;
	}
	if (!file_word_write(fileh, jsub_kanji_addr)) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &jsubcpu.dp)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.acc.d)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.x)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.y)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.u)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.s)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.pc)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.intr)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.cycle)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &jsubcpu.total)) {
	    return FALSE;
	}

	if (!file_bool_read(fileh, &jsub_enable)) {
	    return FALSE;
	}
	if (!file_bool_read(fileh, &jsub_haltflag)) {
	    return FALSE;
	}
	if (!file_byte_read(fileh, &jsub_dicbank)) {
	    return FALSE;
	}
	if (!file_byte_read(fileh, &jsub_address)) {
	    return FALSE;
	}
	if (!file_read(fileh, jsub_sram, 0x2000)) {
	    return FALSE;
	}
	if (ver >= 305 && ver <= 499) {
	   if (!file_word_read(fileh, &jsub_kanji_addr)) {
	      return FALSE;
	   }
	}
    }

    /*
     * 日本語サブが使用不可の場合ディセーブル 
     */
    if (!jsub_available) {
	jsub_enable = FALSE;
    }

    return TRUE;
}

#endif

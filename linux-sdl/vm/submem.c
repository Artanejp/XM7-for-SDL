/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ サブCPUメモリ ]
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "keyboard.h"
#include "multipag.h"
#include "aluline.h"
#if XM7_VER == 1 && defined(L4CARD)
#include "l4font.h"
#endif
/*
 * XM7/SDL依存 
 */

/*
 *      グローバル ワーク
 */
BYTE           *vram_c;		/* VRAM(タイプC)
				 * $C000(裏バンク有り) */
BYTE           *subrom_c;	/* ROM (タイプC) $2800 */
BYTE           *sub_ram;	/* コンソールRAM $1680 */
BYTE           *sub_io;		/* サブCPU I/O $0100 */
#if XM7_VER >= 3
BYTE           *subramde;	/* RAM (TypeD/E) $2000 */
BYTE           *subramcg;	/* RAM (フォント)$4000 */
BYTE           *subramcn;	/* 裏コンソール $2000 */
#endif

#if XM7_VER >= 2
BYTE           *vram_b;		/* VRAM(タイプB)
				 * $C000(裏バンク有り) */
BYTE           *subrom_a;	/* ROM (タイプA) $2000 */
BYTE           *subrom_b;	/* ROM (タイプB) $2000 */
BYTE           *subromcg;	/* ROM (フォント)$2000 */

BYTE            subrom_bank;	/* サブシステムROMバンク */
BYTE            cgrom_bank;	/* CGROMバンク */
#if XM7_VER >= 3
BYTE            cgram_bank;	/* CGRAMバンク */
BYTE            consram_bank;	/* コンソールRAMバンク */
#endif				/* XM7_VER >= 3 */
#endif				/* XM7_VER >= 2 */

#if XM7_VER == 1 && defined(L4CARD)
BYTE           *subrom_8;	/* ROM (FM-8) $2800 */
BYTE           *tvram_c;	/* Text VRAM $1000 */
BYTE           *subrom_l4;	/* ROM (400ライン)$4800 */
BYTE           *subcg_l4;	/* CG (400ライン)$1000 */
BOOL            enable_400linecard;	/* 400ラインカードイネーブル 
					 */
BOOL            detect_400linecard;	/* 400ラインカード発見フラグ 
					 */
#endif


/*
 *      サブCPUメモリ
 *      初期化
 */
BOOL            FASTCALL
submem_init(void)
{
    /*
     * 一度、全てクリア 
     */
    vram_c = NULL;
    subrom_c = NULL;
    sub_ram = NULL;
    sub_io = NULL;
#if XM7_VER == 1
    subrom_8 = NULL;
#endif

#if XM7_VER >= 2
    vram_b = NULL;
    subrom_a = NULL;
    subrom_b = NULL;
    subromcg = NULL;
#if XM7_VER >= 3
    subramde = NULL;
    subramcg = NULL;
    subramcn = NULL;
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
    tvram_c = NULL;
    subrom_l4 = NULL;
    subcg_l4 = NULL;
#endif

    /*
     * メモリ確保(タイプC) 
     */
#if XM7_VER >= 3
    vram_c = (BYTE *) malloc(0x30000);
#elif XM7_VER >= 2
    vram_c = (BYTE *) malloc(0x18000);
#else
    vram_c = (BYTE *) malloc(0xc000);
#endif
    if (vram_c == NULL) {
	return FALSE;
    }
    subrom_c = (BYTE *) malloc(0x2800);
    if (subrom_c == NULL) {
	return FALSE;
    }
#if XM7_VER == 1
    subrom_8 = (BYTE *) malloc(0x2800);
    if (subrom_8 == NULL) {
	return FALSE;
    }
#endif

    /*
     * メモリ確保(タイプA,B) 
     */
#if XM7_VER >= 2
    vram_b = vram_c + 0xc000;
    subrom_a = (BYTE *) malloc(0x2000);
    if (subrom_a == NULL) {
	return FALSE;
    }
    subrom_b = (BYTE *) malloc(0x2000);
    if (subrom_b == NULL) {
	return FALSE;
    }
    subromcg = (BYTE *) malloc(0x2000);
    if (subromcg == NULL) {
	return FALSE;
    }
#if XM7_VER >= 3
    /*
     * メモリ確保(タイプD,E) 
     */
    subramcn = (BYTE *) malloc(0x2000);
    if (subramcn == NULL) {
	return FALSE;
    }
    subramcg = (BYTE *) malloc(0x4000);
    if (subramcg == NULL) {
	return FALSE;
    }
    subramde = (BYTE *) malloc(0x2000);
    if (subramde == NULL) {
	return FALSE;
    }
#endif
#endif

    /*
     * メモリ確保(共通) 
     */
    sub_ram = (BYTE *) malloc(0x1680);
    if (sub_ram == NULL) {
	return FALSE;
    }
    sub_io = (BYTE *) malloc(0x0100);
    if (sub_io == NULL) {
	return FALSE;
    }
#if XM7_VER == 1 && defined(L4CARD)
    /*
     * メモリ確保(L4) 
     */
    tvram_c = (BYTE *) malloc(0x1000);
    if (tvram_c == NULL) {
	return FALSE;
    }
    subrom_l4 = (BYTE *) malloc(0x4800);
    if (subrom_l4 == NULL) {
	return FALSE;
    }
    subcg_l4 = (BYTE *) malloc(0x1000);
    if (subcg_l4 == NULL) {
	return FALSE;
    }
#endif

    /*
     * ROMファイル読み込み 
     */
    if (!file_load(SUBSYSC_ROM, subrom_c, 0x2800)) {
#if XM7_VER == 1
	available_fm7roms = FALSE;
#else
	return FALSE;
#endif
    }
#if XM7_VER >= 2
    if (!file_load(SUBSYSA_ROM, subrom_a, 0x2000)) {
	return FALSE;
    }
    if (!file_load(SUBSYSB_ROM, subrom_b, 0x2000)) {
	return FALSE;
    }
    if (!file_load(SUBSYSCG_ROM, subromcg, 0x2000)) {
	return FALSE;
    }
#endif

#if XM7_VER == 1
    if (!file_load(SUBSYS8_ROM, subrom_8, 0x2800)) {
	available_fm8roms = FALSE;
    }
#ifdef L4CARD
    /*
     * 400ライン対応処理 
     */
    if (!file_load(SUBSYSL4_ROM, subrom_l4, 0x4800)) {
	enable_400linecard = FALSE;
	detect_400linecard = FALSE;
    } else {
	enable_400linecard = TRUE;
	detect_400linecard = TRUE;

	if (!file_load(ANKCG16_ROM, subcg_l4, 0x1000)) {
	    if (!file_load(EXTSUB_ROM, subcg_l4, 0x1000)) {
#if 1
		memcpy(subcg_l4, subcg_internal, 4096);
#else
		int             addr;
		for (addr = 0x7ff; addr >= 0; addr--) {
		    subcg_l4[addr * 2 + 0] = subrom_c[addr];
		    subcg_l4[addr * 2 + 1] = subrom_c[addr];
		}
#endif
	    }
	}
    }
#endif

    if (!available_fm8roms && !available_fm7roms) {
	return FALSE;
    }
#endif

    return TRUE;
}

/*
 *      サブCPUメモリ
 *      クリーンアップ
 */
void            FASTCALL
submem_cleanup(void)
{
    ASSERT(vram_c);
    ASSERT(subrom_c);
    ASSERT(sub_ram);
    ASSERT(sub_io);
#if XM7_VER >= 2
    ASSERT(subrom_a);
    ASSERT(subrom_b);
    ASSERT(subromcg);
#if XM7_VER >= 3
    ASSERT(subramcn);
    ASSERT(subramcg);
    ASSERT(subramde);
#endif
#endif
#if XM7_VER == 1
    ASSERT(subrom_8);
#ifdef L4CARD
    ASSERT(tvram_c);
    ASSERT(subrom_l4);
    ASSERT(subcg_l4);
#endif
#endif

    /*
     * 初期化途中で失敗した場合を考慮 
     */
    if (vram_c) {
	free(vram_c);
    }
    if (subrom_c) {
	free(subrom_c);
    }
#if XM7_VER >= 2
    if (subrom_a) {
	free(subrom_a);
    }
    if (subrom_b) {
	free(subrom_b);
    }
    if (subromcg) {
	free(subromcg);
    }
#if XM7_VER >= 3
    if (subramcn) {
	free(subramcn);
    }
    if (subramcg) {
	free(subramcg);
    }
    if (subramde) {
	free(subramde);
    }
#endif
#endif

    if (sub_ram) {
	free(sub_ram);
    }
    if (sub_io) {
	free(sub_io);
    }
#if XM7_VER == 1
    if (subrom_8) {
	free(subrom_8);
    }
#ifdef L4CARD
    if (tvram_c) {
	free(tvram_c);
    }
    if (subrom_l4) {
	free(subrom_l4);
    }
    if (subcg_l4) {
	free(subcg_l4);
    }
#endif
#endif
}

/*
 *      サブCPUメモリ
 *      リセット
 */
void            FASTCALL
submem_reset(void)
{
#if XM7_VER >= 2
    /*
     * VRAMアクティブページ 
     */
    vram_aptr = vram_c;
    vram_active = 0;

    /*
     * バンククリア 
     */
    subrom_bank = 0;
    cgrom_bank = 0;
#if XM7_VER >= 3
    cgram_bank = 0;
    consram_bank = 0;
#endif
#endif

    /*
     * I/O空間 クリア 
     */
    memset(sub_io, 0xff, 0x0100);
}

/*
 *      サブCPUメモリ
 *      １バイト取得
 */
volatile BYTE            FASTCALL
submem_readb(WORD addr)
{
    BYTE            dat;

#if XM7_VER == 1 && defined(L4CARD)
    /*
     * FM-77L4 400ラインモード時の処理 
     */
    if (enable_400line && enable_400linecard) {
	/*
	 * GVRAM/ワークRAM 
	 */
	if (addr <= 0x7fff) {
	    /*
	     * ワークRAM 
	     */
	    if (workram_select) {
		if (multi_page & 4) {
		    return 0xff;
		}
		return vram_c[0x8000 + ((addr + vram_offset[0]) & 0x3fff)];
	    }

	    /*
	     * GVRAM 
	     */
	    addr += vram_offset[0];
	    addr &= 0x7fff;
	    if (multi_page & (1 << (addr >> 14))) {
		return 0xff;
	    }
	    return vram_c[addr];
	}

	/*
	 * テキストVRAM 
	 */
	if ((addr >= 0x8000) && (addr <= 0x97ff)) {
	    return tvram_c[addr & 0xfff];
	}

	/*
	 * サブモニタROM(前半) 
	 */
	if ((addr >= 0x9800) && (addr <= 0xbfff)) {
	    return subrom_l4[addr - 0x9800];
	}

	/*
	 * サブモニタROM(後半) 
	 */
	if (addr >= 0xe000) {
	    return subrom_l4[addr - 0xb800];
	}
    }
#endif

    /*
     * VRAM 
     */
    if (addr < 0xc000) {
#if XM7_VER >= 3
	if (mode400l) {
	    /*
	     * 400ライン 
	     */
	    if (addr >= 0x8000) {
		return 0xff;
	    }
	    aluline_extrb(addr);

	    if (multi_page & (1 << subram_vrambank)) {
		return 0xff;
	    } else {
		return vram_aptr[addr];
	    }
	} else {
	    /*
	     * 200ライン 
	     */
	    aluline_extrb(addr);
	    if (multi_page & (1 << (addr >> 14))) {
		return 0xff;
	    } else {
		return vram_aptr[((addr & 0xc000) << 1) | (addr & 0x3fff)];
	    }
	}
#elif XM7_VER >= 2
	/*
	 * 200ライン 
	 */
	aluline_extrb(addr);
	if (multi_page & (1 << (addr >> 14))) {
	    return 0xff;
	} else {
	    return vram_aptr[addr];
	}
#else
	/*
	 * 200ライン 
	 */
	if (multi_page & (1 << (addr >> 14))) {
	    return 0xff;
	} else {
	    return vram_c[addr];
	}
#endif
    }
#if XM7_VER >= 3
    /*
     * コンソールRAM(RAMモード専用) 
     */
    if ((addr < 0xd000) && (subrom_bank == 4) && (consram_bank >= 1)) {
	return subramcn[(consram_bank - 1) * 0x1000 + (addr - 0xc000)];
    }
#endif

    /*
     * ワークRAM 
     */
    if (addr < 0xd380) {
	return sub_ram[addr - 0xc000];
    }

    /*
     * 共有RAM 
     */
    if (addr < 0xd400) {
	return shared_ram[(WORD) (addr - 0xd380)];
    }
#if XM7_VER >= 2
    /*
     * サブROM 
     */
    if (addr >= 0xe000) {
	switch (subrom_bank) {
	    /*
	     * タイプC 
	     */
	case 0:
	    return subrom_c[addr - 0xd800];
	    /*
	     * タイプA 
	     */
	case 1:
	    return subrom_a[addr - 0xe000];
	    /*
	     * タイプB 
	     */
	case 2:
	    return subrom_b[addr - 0xe000];
	    /*
	     * CGROM 
	     */
	case 3:
	    return subromcg[addr - 0xe000];
#if XM7_VER >= 3
	    /*
	     * タイプD/E(RAM) 
	     */
	case 4:
	    return subramde[addr - 0xe000];
#endif
	}
    }

    /*
     * CGRAM,CGROM 
     */
    if (addr >= 0xd800) {
#if XM7_VER >= 3
	if (subrom_bank == 4) {
	    /*
	     * サブRAM バンク 
	     */
	    return subramcg[cgram_bank * 0x0800 + (addr - 0xd800)];
	} else {
	    /*
	     * サブROM バンク 
	     */
	    return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
	}
#else
	/*
	 * サブROM バンク 
	 */
	return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
#endif
    }

    /*
     * ワークRAM 
     */
//    if (addr >= 0xd500) {
//	return sub_ram[(addr - 0xd500) + 0x1380];
	if (fm7_ver >= 2) {
		if (addr >= 0xd500){
			return sub_ram[(addr - 0xd500) + 0x1380];
		}
    }
#else
    if (addr >= 0xd800) {
	if (fm_subtype == FMSUB_FM8) {
	    /*
	     * FM-8サブモニタ 
	     */
	    return subrom_8[addr - 0xd800];
	} else {
	    /*
	     * タイプC 
	     */
	    return subrom_c[addr - 0xd800];
	}
    }
#endif

    /*
     *      サブI/O
     */
#if XM7_VER == 1
	/* $D410〜$D7FFは$D400〜$D40Fのミラー */
	addr &= 0xfc0f;
#else
	if (fm7_ver == 1) {
		/* $D410〜$D7FFは$D400〜$D40Fのミラー */
		addr &= 0xfc0f;
	}
	else if (fm7_ver == 2) {
		/* $D440〜$D4FFは$D400〜$D43Fのミラー */
		addr &= 0xff3f;
	}
#endif

    /*
     * ディスプレイ 
     */
    if (display_readb(addr, &dat)) {
	return dat;
    }
    /*
     * キーボード 
     */
    if (keyboard_readb(addr, &dat)) {
	return dat;
    }
    /*
     * 論理演算・直線補間 
     */
#if XM7_VER >= 2
    if (aluline_readb(addr, &dat)) {
	return dat;
    }
#endif

    return 0xff;
}

/*
 *      サブCPUメモリ
 *      １バイト取得(I/Oなし)
 */
//BYTE            FASTCALL
volatile BYTE
submem_readbnio(WORD addr)
{
#if XM7_VER == 1 && defined(L4CARD)
    /*
     * FM-77L4 400ラインモード時の処理 
     */
    if (enable_400line && enable_400linecard) {
	/*
	 * GVRAM/ワークRAM 
	 */
	if (addr <= 0x7fff) {
	    /*
	     * ワークRAM 
	     */
	    if (workram_select) {
		if (multi_page & 4) {
		    return 0xff;
		}
		return vram_c[0x8000 + ((addr + vram_offset[0]) & 0x3fff)];
	    }

	    /*
	     * GVRAM 
	     */
	    addr += vram_offset[0];
	    addr &= 0x7fff;
	    if (multi_page & (1 << (addr >> 14))) {
		return 0xff;
	    }
	    return vram_c[addr];
	}

	/*
	 * テキストVRAM 
	 */
	if ((addr >= 0x8000) && (addr <= 0x97ff)) {
	    return tvram_c[addr & 0xfff];
	}

	/*
	 * サブモニタROM(前半) 
	 */
	if ((addr >= 0x9800) && (addr <= 0xbfff)) {
	    return subrom_l4[addr - 0x9800];
	}

	/*
	 * サブモニタROM(後半) 
	 */
	if (addr >= 0xe000) {
	    return subrom_l4[addr - 0xb800];
	}
    }
#endif

    /*
     * VRAM 
     */
    if (addr < 0xc000) {
#if XM7_VER >= 3
	if (mode400l) {
	    /*
	     * 400ライン 
	     */
	    if (addr < 0x8000) {
		return vram_aptr[addr];
	    } else {
		return 0xff;
	    }
	} else {
	    /*
	     * 200ライン 
	     */
	    return vram_aptr[((addr & 0xc000) << 1) | (addr & 0x3fff)];
	}
#elif XM7_VER >= 2
	/*
	 * 200ライン 
	 */
	return vram_aptr[addr];
#else
	/*
	 * 200ライン 
	 */
	return vram_c[addr];
#endif
    }
#if XM7_VER >= 3
    /*
     * コンソールRAM(RAMモード専用) 
     */
    if ((addr < 0xd000) && (subrom_bank == 4) && (consram_bank >= 1)) {
	return subramcn[(consram_bank - 1) * 0x1000 + (addr - 0xc000)];
    }
#endif

    /*
     * ワークRAM 
     */
    if (addr < 0xd380) {
	return sub_ram[addr - 0xc000];
    }

    /*
     * 共有RAM 
     */
    if (addr < 0xd400) {
	return shared_ram[(WORD) (addr - 0xd380)];
    }

 //   /*
 //    * サブI/O 
 //    */
 //   if (addr < 0xd500) {
//	return sub_io[addr - 0xd400];
//    }
	/* ワークRAM */
#if XM7_VER >= 2
	if (fm7_ver >= 2) {
		if ((addr >= 0xd500) && (addr < 0xd800)) {
			return sub_ram[(addr - 0xd500) + 0x1380];
		}
 	}
#endif

    
//    /*
//     * ワークRAM 
//     */
//    if (addr < 0xd800) {
//	return sub_ram[(addr - 0xd500) + 0x1380];
	/* サブI/O */
	if (addr < 0xd800) {
#if XM7_VER == 1
		/* $D410〜$D7FFは$D400〜$D40Fのミラー */
		addr &= 0xfc0f;
#else
		if (fm7_ver == 1) {
			/* $D410〜$D7FFは$D400〜$D40Fのミラー */
			addr &= 0xfc0f;
		}
		else if (fm7_ver == 2) {
			/* $D440〜$D4FFは$D400〜$D43Fのミラー */
			addr &= 0xff3f;
		}
#endif
		return sub_io[addr - 0xd400];

    }
#if XM7_VER >= 2
    /*
     * サブROM 
     */
    if (addr >= 0xe000) {
	ASSERT(subrom_bank <= 4);
	switch (subrom_bank) {
	    /*
	     * タイプC 
	     */
	case 0:
	    return subrom_c[addr - 0xd800];
	    /*
	     * タイプA 
	     */
	case 1:
	    return subrom_a[addr - 0xe000];
	    /*
	     * タイプB 
	     */
	case 2:
	    return subrom_b[addr - 0xe000];
	    /*
	     * CGROM 
	     */
	case 3:
	    return subromcg[addr - 0xe000];
#if XM7_VER >= 3
	    /*
	     * タイプD/E(RAM) 
	     */
	case 4:
	    return subramde[addr - 0xe000];
#endif
	}
    }

    /*
     * CGRAM,CGROM 
     */
    if (addr >= 0xd800) {
#if XM7_VER >= 3
	if (subrom_bank == 4) {
	    /*
	     * サブRAM バンク 
	     */
	    return subramcg[cgram_bank * 0x0800 + (addr - 0xd800)];
	} else {
	    /*
	     * サブROM バンク 
	     */
	    return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
	}
#else
	/*
	 * サブROM バンク 
	 */
	return subromcg[cgrom_bank * 0x0800 + (addr - 0xd800)];
#endif
    }
#else
    if (addr >= 0xd800) {
	if (fm_subtype == FMSUB_FM8) {
	    /*
	     * FM-8サブモニタ 
	     */
	    return subrom_8[addr - 0xd800];
	} else {
	    /*
	     * タイプC 
	     */
	    return subrom_c[addr - 0xd800];
	}
    }
#endif

    /*
     * ここには来ない 
     */
    ASSERT(FALSE);
    return 0;
}

/*
 *      サブCPUメモリ
 *      １バイト書き込み
 */
void            FASTCALL
submem_writeb(WORD addr, BYTE dat)
{
#if XM7_VER == 1 && defined(L4CARD)
    /*
     * FM-77L4 400ラインモード時の処理 
     */
    if (enable_400line && enable_400linecard) {
	/*
	 * GVRAM/ワークRAM 
	 */
	if (addr <= 0x7fff) {
	    /*
	     * ワークRAM 
	     */
	    if (workram_select) {
		if (!(multi_page & 4)) {
		    vram_c[0x8000 + ((addr + vram_offset[0]) & 0x3fff)] =
			dat;
		}
		return;
	    }

	    /*
	     * GVRAM 
	     */
	    addr += vram_offset[0];
	    addr &= 0x7fff;
	    if (!(multi_page & (1 << (addr >> 14)))) {
		/*
		 * 同じデータを書き込むなら、Notifyしない 
		 */
		if (vram_c[addr] != dat) {
		    vram_c[addr] = dat;
		    vram_notify(addr, dat);
		}
	    }
	    return;
	}

	/*
	 * テキストVRAM 
	 */
	if ((addr >= 0x8000) && (addr <= 0x97ff)) {
	    tvram_c[addr & 0xfff] = dat;
	    tvram_notify((WORD) (addr & 0xfff), dat);
	    return;
	}

	/*
	 * サブモニタROM 
	 */
	if (((addr >= 0x9800) && (addr <= 0xbfff)) || (addr >= 0xe000)) {
	    return;
	}
    }
#endif

    /*
     * VRAM(タイプC) 
     */
    if (addr < 0xc000) {
#if XM7_VER >= 3
	if (mode400l) {
	    /*
	     * 400ライン 
	     */
	    if (addr >= 0x8000) {
		return;
	    }
	    if (alu_command & 0x80) {
		aluline_extrb(addr);
		return;
	    }
	    if (!(multi_page & (1 << subram_vrambank))) {
		if (vram_aptr[addr] != dat) {
		    /*
		     * 同じデータを書き込むなら、Notifyしない 
		     */
		    vram_aptr[addr] = dat;
		    vram_notify(addr, dat);
		}
	    }
	} else {
	    if (alu_command & 0x80) {
		aluline_extrb(addr);
		return;
	    }
	    if (!(multi_page & (1 << (addr >> 14)))) {
		/*
		 * 同じデータを書き込むなら、Notifyしない 
		 */
		if (vram_aptr[((addr & 0xc000) << 1) | (addr & 0x3fff)] !=
		    dat) {
		    vram_aptr[((addr & 0xc000) << 1) | (addr & 0x3fff)] =
			dat;
		    vram_notify(addr, dat);
		}
	    }
	}
#elif XM7_VER >= 2
	if (alu_command & 0x80) {
	    aluline_extrb(addr);
	    return;
	}
	if (!(multi_page & (1 << (addr >> 14)))) {
	    /*
	     * 同じデータを書き込むなら、Notifyしない 
	     */
	    if (vram_aptr[addr] != dat) {
		vram_aptr[addr] = dat;
		vram_notify(addr, dat);
	    }
	}
#else
	if (!(multi_page & (1 << (addr >> 14)))) {
	    /*
	     * 同じデータを書き込むなら、Notifyしない 
	     */
	    if (vram_c[addr] != dat) {
		vram_c[addr] = dat;
		vram_notify(addr, dat);
	    }
	}
#endif
	return;
    }
#if XM7_VER >= 3
    /*
     * コンソールRAM(RAMモード専用) 
     */
    if ((addr < 0xd000) && (subrom_bank == 4) && (consram_bank >= 1)) {
	subramcn[(consram_bank - 1) * 0x1000 + (addr - 0xc000)] = dat;
	return;
    }
#endif

    /*
     * ワークRAM 
     */
    if (addr < 0xd380) {
	sub_ram[addr - 0xc000] = dat;
	return;
    }

    /*
     * 共有RAM 
     */
    if (addr < 0xd400) {
	shared_ram[(WORD) (addr - 0xd380)] = dat;
	return;
    }

    /*
     * ワークRAM 
     */
#if XM7_VER >= 2
//    if ((addr >= 0xd500) && (addr < 0xd800)) {
//	sub_ram[(addr - 0xd500) + 0x1380] = dat;
//	return;
	if (fm7_ver >= 2) {
		if ((addr >= 0xd500) && (addr < 0xd800)) {
			sub_ram[(addr - 0xd500) + 0x1380] = dat;
			return;
		}
    }
   
#endif

    /*
     * CGROM,サブRAM →ROMモードでは書き込みできない 
     */
    if (addr >= 0xd800) {
#if XM7_VER >= 3
	/*
	 * サブRAMでないか、プロテクトされている 
	 */
	if ((subrom_bank != 4) || (subram_protect)) {
	    return;
	}

	/*
	 * CGRAM 
	 */
	if (addr < 0xe000) {
	    subramcg[cgram_bank * 0x0800 + (addr - 0xd800)] = dat;
	    return;
	}

	/*
	 * サブRAM 
	 */
	subramde[addr - 0xe000] = dat;
	return;
#else
	/*
	 * V1/V2では無条件で書き込み不可 
	 */
	return;
#endif
    }

    /*
     *      サブI/O
     */
#if XM7_VER == 1
	/* $D410〜$D7FFは$D400〜$D40Fのミラー */
	addr &= 0xfc0f;
#else
	if (fm7_ver == 1) {
		/* $D410〜$D7FFは$D400〜$D40Fのミラー */
		addr &= 0xfc0f;
	}
	else if (fm7_ver == 2) {
		/* $D440〜$D4FFは$D400〜$D43Fのミラー */
		addr &= 0xff3f;
	}
#endif
   
    sub_io[addr - 0xd400] = dat;

    /*
     * ディスプレイ 
     */
    if (display_writeb(addr, dat)) {
	return;
    }
    /*
     * キーボード 
     */
    if (keyboard_writeb(addr, dat)) {
	return;
    }
    /*
     * 論理演算・直線補間 
     */
#if XM7_VER >= 2
    if (aluline_writeb(addr, dat)) {
	return;
    }
#endif
}

/*
 *      サブCPUメモリ
 *      セーブ
 */
BOOL            FASTCALL
submem_save(int fileh)
{
#if XM7_VER >= 3
    /*
     * 今までと同じ順序で書き込む 
     */
    if (mode400l) {
	if (!file_write(fileh, vram_c, 0x18000)) {
	    return FALSE;
	}
    } else {
	if (!file_write(fileh, vram_c, 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x8000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x10000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x4000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0xc000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x14000], 0x4000)) {
	    return FALSE;
	}
    }
#else
    if (!file_write(fileh, vram_c, 0x6000)) {
	return FALSE;
    }
    if (!file_write(fileh, &vram_c[0x6000], 0x6000)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_write(fileh, &vram_c[0xc000], 0x6000)) {
	return FALSE;
    }
    if (!file_write(fileh, &vram_c[0x12000], 0x6000)) {
	return FALSE;
    }
#endif
#endif

    if (!file_write(fileh, sub_ram, 0x1680)) {
	return FALSE;
    }

    if (!file_write(fileh, sub_io, 0x100)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_byte_write(fileh, subrom_bank)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, cgrom_bank)) {
	return FALSE;
    }
#if XM7_VER >= 3
    /*
     * Ver8拡張 
     */
    if (mode400l) {
	if (!file_write(fileh, &vram_c[0x18000], 0x18000)) {
	    return FALSE;
	}
    } else {
	if (!file_write(fileh, &vram_c[0x18000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x20000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x28000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x1c000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x24000], 0x4000)) {
	    return FALSE;
	}
	if (!file_write(fileh, &vram_c[0x2c000], 0x4000)) {
	    return FALSE;
	}
    }

    if (!file_write(fileh, subramde, 0x2000)) {
	return FALSE;
    }
    if (!file_write(fileh, subramcg, 0x4000)) {
	return FALSE;
    }
    if (!file_write(fileh, subramcn, 0x2000)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, cgram_bank)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, consram_bank)) {
	return FALSE;
    }
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
    if (!file_write(fileh, tvram_c, 0x1000)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, detect_400linecard)) {
	return FALSE;
    }
#endif

    return TRUE;
}

/*
 *      サブCPUメモリ
 *      ロード
 */
BOOL            FASTCALL
submem_load(int fileh, int ver)
{
#if XM7_VER == 1 && defined(L4CARD)
    BOOL            tmp;
#endif

    /*
     * バージョンチェック 
     */
    if (ver < 200) {
	return FALSE;
    }
#if XM7_VER >= 3
    /*
     * 今までと同じ順序で読み出す 
     */
    if (!file_read(fileh, vram_c, 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x8000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x10000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x4000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0xc000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x14000], 0x4000)) {
	return FALSE;
    }
#else
    if (!file_read(fileh, vram_c, 0x6000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x6000], 0x6000)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_read(fileh, &vram_c[0xc000], 0x6000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x12000], 0x6000)) {
	return FALSE;
    }
#endif
#endif

    if (!file_read(fileh, sub_ram, 0x1680)) {
	return FALSE;
    }

    if (!file_read(fileh, sub_io, 0x100)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_byte_read(fileh, &subrom_bank)) {
	return FALSE;
    }
    if (!file_byte_read(fileh, &cgrom_bank)) {
	return FALSE;
    }
#if XM7_VER >= 3
    /*
     * Ver8拡張 
     */
    if (ver < 800) {
	consram_bank = 0;
	cgram_bank = 0;
	return TRUE;
    }

    /*
     * 今までと同じ順序で読み出す 
     */
    if (!file_read(fileh, &vram_c[0x18000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x20000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x28000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x1c000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x24000], 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, &vram_c[0x2c000], 0x4000)) {
	return FALSE;
    }

    if (!file_read(fileh, subramde, 0x2000)) {
	return FALSE;
    }
    if (!file_read(fileh, subramcg, 0x4000)) {
	return FALSE;
    }
    if (!file_read(fileh, subramcn, 0x2000)) {
	return FALSE;
    }
    if (!file_byte_read(fileh, &cgram_bank)) {
	return FALSE;
    }
    if (!file_byte_read(fileh, &consram_bank)) {
	return FALSE;
    }
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
    if (!file_read(fileh, tvram_c, 0x1000)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &tmp)) {
	return FALSE;
    }
    if (!detect_400linecard && tmp) {
	return FALSE;
    }
#endif

    return TRUE;
}

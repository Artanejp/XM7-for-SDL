/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2013 Ryu Takegami
 *
 *	[ メインCPUメモリ ]
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "subctrl.h"
#include "ttlpalet.h"
#include "fdc.h"
#include "mainetc.h"
#include "multipag.h"
#include "kanji.h"
#include "tapelp.h"
#include "opn.h"
#include "mmr.h"
#include "apalet.h"
#include "rs232c.h"
#ifdef MIDI
#include "midi.h"
#endif
#if XM7_VER >= 3
#include "jcard.h"
#include "dmac.h"
#endif
#if (XM7_VER == 1 && defined(JSUB))
#include "jsubsys.h"
#endif
#if (XM7_VER == 1 && defined(BUBBLE))
#include "bubble.h"
#endif

/*
 *	グローバル ワーク
 */
BYTE *mainram_a;						/* RAM (表RAM)        $8000 */
BYTE *mainram_b;						/* RAM (裏RAM+α)     $7C80 */
BYTE *basic_rom;						/* ROM (F-BASIC)      $7C00 */
BYTE *main_io;							/* メインCPU I/O       $100 */
BOOL basicrom_en;						/* BASIC ROMイネーブルフラグ */
#if XM7_VER == 1
BYTE *basic_rom8;						/* ROM (F-BASIC V1.0) $7C00 */
BYTE *boot_bas;							/* ブート(BASIC,FM-7)  $200 */
BYTE *boot_dos;							/* ブート(DOS,FM-7)    $200 */
BYTE *boot_bas8;						/* ブート(BASIC,FM-8)  $200 */
BYTE *boot_dos8;						/* ブート(DOS,FM-8)    $200 */
BYTE *boot_bbl8;						/* ブート(BUBBLE,FM-8) $200 */
BYTE *boot_mmr;							/* ブート(隠し)        $200 */
#if defined(BUBBLE)
BOOL bubble_available;					/* バブル使用可能フラグ */
#endif
#endif
BYTE *boot_ram;							/* ブートRAM           $200 */
BOOL bootram_rw;						/* ブートRAM 書き込みフラグ */

BYTE *extram_a;							/* 拡張RAM           $10000 */
#if XM7_VER >= 3
BYTE *extram_c;							/* AV40拡張RAM       $C0000 */
BYTE *boot_mmr;							/* ブート(隠し)        $200 */
#endif

#if XM7_VER >= 2
BYTE *init_rom;							/* イニシエータROM    $2000 */
BOOL initrom_en;						/* イニシエータROMイネーブルフラグ */
#endif


/*
 *	スタティック ワーク
 */
#if XM7_VER >= 2
static BYTE *patch_branewboottfr;		/* 新ブート転送用処理へのBRA命令 */
static BYTE *patch_jmpnewboot;			/* 新ブートへのJMP命令 */
#endif
static BYTE ioaccess_count;				/* I/O領域アクセスカウンタ */
static BOOL ioaccess_flag;				/* I/Oアクセスウェイト調整フラグ */


/*
 *	メインCPUメモリ
 *	初期化
 */
BOOL FASTCALL mainmem_init(void)
{
	int i;
	BYTE *p;

	/* 一度、全てクリア */
	mainram_a = NULL;
	mainram_b = NULL;
	basic_rom = NULL;
	main_io = NULL;
	extram_a = NULL;
#if XM7_VER == 1
	boot_bas = NULL;
	boot_dos = NULL;
	boot_mmr = NULL;
	boot_bas8 = NULL;
	boot_dos8 = NULL;
	boot_bbl8 = NULL;
#if defined(BUBBLE)
	bubble_available = TRUE;
#endif
#else
	patch_branewboottfr = NULL;
	patch_jmpnewboot = NULL;
#if XM7_VER >= 3
	boot_mmr = NULL;
	extram_c = NULL;
#endif
	init_rom = NULL;
#endif
	boot_ram = NULL;

	/* RAM */
	mainram_a = (BYTE *)malloc(0x8000);
	if (mainram_a == NULL) {
		return FALSE;
	}
#if XM7_VER == 1
	mainram_b = (BYTE *)malloc(0x7e00);
#else
	mainram_b = (BYTE *)malloc(0x7c80);
#endif
	if (mainram_b == NULL) {
		return FALSE;
	}

	/* BASIC ROM, I/O */
	basic_rom = (BYTE *)malloc(0x7c00);
	if (basic_rom == NULL) {
		return FALSE;
	}
#if XM7_VER == 1
	basic_rom8 = (BYTE *)malloc(0x7c00);
	if (basic_rom8 == NULL) {
		return FALSE;
	}
#endif
	main_io = (BYTE *)malloc(0x0100);
	if (main_io == NULL) {
		return FALSE;
	}

	/* 拡張RAM、イニシエータROM */
#if XM7_VER >= 2
	extram_a = (BYTE *)malloc(0x10000);
	if (extram_a == NULL) {
		return FALSE;
	}
#if XM7_VER >= 3
	extram_c = (BYTE *)malloc(0xc0000);
	if (extram_c == NULL) {
		return FALSE;
	}
#endif
	init_rom = (BYTE *)malloc(0x2000);
	if (init_rom == NULL) {
		return FALSE;
	}
#else
	/* V1 (400ラインセット，192KB) */
	extram_a = (BYTE *)malloc(0x30000);
	if (extram_a == NULL) {
		return FALSE;
	}
#endif

	/* ブートROM/RAM */
#if XM7_VER == 1
	boot_bas = (BYTE *)malloc(0x200);
	if (boot_bas == NULL) {
		return FALSE;
	}
	boot_dos = (BYTE *)malloc(0x200);
	if (boot_dos == NULL) {
		return FALSE;
	}
	boot_bas8 = (BYTE *)malloc(0x200);
	if (boot_bas8 == NULL) {
		return FALSE;
	}
	boot_dos8 = (BYTE *)malloc(0x200);
	if (boot_dos8 == NULL) {
		return FALSE;
	}
	boot_bbl8 = (BYTE *)malloc(0x200);
	if (boot_bbl8 == NULL) {
		return FALSE;
	}
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
	boot_mmr = (BYTE *)malloc(0x200);
	if (boot_mmr == NULL) {
		return FALSE;
	}
#endif
	boot_ram = (BYTE *)malloc(0x200);
	if (boot_ram == NULL) {
		return FALSE;
	}

	/* ROMファイル読み込み */
	if (!file_load(FBASIC_ROM, basic_rom, 0x7c00)) {
#if XM7_VER == 1
		available_fm7roms = FALSE;
#else
		return FALSE;
#endif
	}
#if XM7_VER == 1
	if (!file_load(FBASIC10_ROM, basic_rom8, 0x7c00)) {
		available_fm8roms = FALSE;
	}
#endif

#if XM7_VER >= 2
	if (!file_load(INITIATE_ROM, init_rom, 0x2000)) {
		return FALSE;
	}

	/* 旧ブート高速起動(AVシリーズ共通) */
	for (i=0; i<2; i++) {
		p = &init_rom[0x1800 + i * 0x200];
		if (p[0x14f] == 0x26) {
			p[0x14f] = 0x21;
		}
		if (p[0x153] == 0x26) {
			p[0x153] = 0x21;
		}
	}

	/* 新ブート高速起動・ドライブ対応変更抑制(AV20EX/AV40EX/AV40SX) */
	p = &init_rom[0x1c00];
	if ((p[0x8f] == 0x10) && (p[0x90] == 0x23)) {
		p[0x8f] = 0x12;
		p[0x90] = 0x16;
		if (p[0x166] == 0x27) {
			p[0x166] = 0x21;
		}
		if (p[0x1d6] == 0x26) {
			p[0x1d6] = 0x21;
		}
	}

	/* 新ブート高速起動・ドライブ対応変更抑制(AV40/AV20) */
	if ((p[0x74] == 0x10) && (p[0x75] == 0x23)) {
		p[0x74] = 0x12;
		p[0x75] = 0x16;
		if (p[0xf3] == 0x27) {
			p[0xf3] = 0x21;
		}
		if (p[0x196] == 0x26) {
			p[0x196] = 0x21;
		}
	}

	/* 新ブート関連処理のアドレスを検索・記憶 */
	for (i=0; i<0xb00; i++) {
		/* 新ブート転送処理へのBRA命令 */
		if (!patch_branewboottfr) {
			if ((init_rom[i + 0] == 0x20) && (init_rom[i + 1] == 0xd7)) {
				patch_branewboottfr = &init_rom[i];
			}
		}

		/* 新ブートへのJMP命令(イニシエータの末尾に存在する) */
		if (!patch_jmpnewboot) {
			if ((init_rom[i + 0] == 0x7e) && (init_rom[i + 1] == 0x50) &&
				(init_rom[i + 2] == 0x00)) {
				patch_jmpnewboot = &init_rom[i];
			}
		}

		/* 両方発見したらループを抜ける */
		if (patch_branewboottfr && patch_jmpnewboot) {
			break;
		}
	}
#if XM7_VER >= 3
	if (!file_load(BOOTMMR_ROM, boot_mmr, 0x200)) {
		available_mmrboot = FALSE;
	}
#endif
#else
	if (!file_load(BOOTBAS_ROM, boot_bas, 0x1e0)) {
		available_fm7roms = FALSE;
	}
	if (!file_load(BOOTDOS_ROM, boot_dos, 0x1e0)) {
		available_fm7roms = FALSE;
	}
	if (!file_load(BOOTMMR_ROM, boot_mmr, 0x1e0)) {
		available_mmrboot = FALSE;
	}
	if (!file_load(BOOTBAS8_ROM, boot_bas8, 0x1e0)) {
		available_fm8roms = FALSE;
	}
	if (!file_load(BOOTDOS8_ROM, boot_dos8, 0x1e0)) {
		available_fm8roms = FALSE;
	}
#if defined(BUBBLE)
        if (!file_load(BOOTBBL8_ROM, boot_bbl8, 0x1e0)) {
		bubble_available = FALSE;
	}
#endif
	/* 旧ブート高速起動(FM-7/NEW7(1st lot)/8) */
	for (i=0; i<6; i++) {
		switch (i) {
			case 0:		p = boot_bas;
						break;
			case 1:		p = boot_dos;
						break;
			case 2:		p = boot_mmr;
						break;
			case 3:		p = boot_bas8;
						break;
			case 4:		p = boot_dos8;
						break;
			case 5:		p = boot_bbl8;
						break;
			default:	ASSERT(FALSE);
						break;
		}

		if (p[0x14f] == 0x26) {
			p[0x14f] = 0x21;
		}
		if (p[0x153] == 0x26) {
			p[0x153] = 0x21;
		}
		if (p[0x161] == 0x26) {
			p[0x161] = 0x21;
		}
		if (p[0x155] == 0x26) {
			p[0x155] = 0x21;
		}
		if (p[0x16b] == 0x26) {
			p[0x16b] = 0x21;
		}
		if (p[0x165] == 0x26) {
			p[0x165] = 0x21;
		}
		if (p[0x155] == 0x26) {
			p[0x155] = 0x21;
		}
		p[0x1fe] = 0xfe;
		p[0x1ff] = 0x00;
	}
#endif

	/* ブートRAMクリア */
	/* (女神転生対策・リセット時に全領域を初期化しなくなったため) */
	memset(boot_ram, 0, 0x200);

#if XM7_VER == 1
	/* FM-7/FM-8どちらのROMも不完全な場合、初期化失敗とする */
	if (!available_fm8roms && !available_fm7roms) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	メインCPUメモリ
 *	クリーンアップ
 */
void FASTCALL mainmem_cleanup(void)
{
	ASSERT(mainram_a);
	ASSERT(mainram_b);
	ASSERT(basic_rom);
	ASSERT(main_io);
	ASSERT(extram_a);
#if XM7_VER >= 3
	ASSERT(extram_c);
#endif
#if XM7_VER >= 2
	ASSERT(init_rom);
#endif

#if XM7_VER == 1
	ASSERT(basic_rom8);
	ASSERT(boot_bas);
	ASSERT(boot_dos);
	ASSERT(boot_bas8);
	ASSERT(boot_dos8);
	ASSERT(boot_bbl8);
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
	ASSERT(boot_mmr);
#endif
	ASSERT(boot_ram);

	/* 初期化途中で失敗した場合を考慮 */
	if (mainram_a) {
		free(mainram_a);
	}
	if (mainram_b) {
		free(mainram_b);
	}
	if (basic_rom) {
		free(basic_rom);
	}
	if (main_io) {
		free(main_io);
	}

	if (extram_a) {
		free(extram_a);
	}
#if XM7_VER >= 3
	if (extram_c) {
		free(extram_c);
	}
#endif
#if XM7_VER >= 2
	if (init_rom) {
		free(init_rom);
	}
#endif

#if XM7_VER == 1
	if (basic_rom8) {
		free(basic_rom8);
	}
	if (boot_bas) {
		free(boot_bas);
	}
	if (boot_dos) {
		free(boot_dos);
	}
	if (boot_bas8) {
		free(boot_bas8);
	}
	if (boot_dos8) {
		free(boot_dos8);
	}
	if (boot_bbl8) {
		free(boot_bbl8);
	}
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
	if (boot_mmr) {
		free(boot_mmr);
	}
#endif
	if (boot_ram) {
		free(boot_ram);
	}
}

/*
 *	メインCPUメモリ
 *	リセット
 */
void FASTCALL mainmem_reset(void)
{
	/* I/O空間・I/Oアクセスカウンタ初期化 */
	memset(main_io, 0xff, 0x0100);
	ioaccess_count = 0;
	ioaccess_flag = FALSE;

	/* BASICモードであれば、F-BASIC ROMをイネーブル */
	if (boot_mode == BOOT_BASIC) {
		basicrom_en = TRUE;
	}
	else {
		basicrom_en = FALSE;
	}

#if XM7_VER >= 2
	if (fm7_ver >= 2) {
		/* AV/40EXモード : イニシエータON、ブートRAM書き込み可 */
		initrom_en = TRUE;
		bootram_rw = TRUE;
	}
	else {
		/* FM-7モード : イニシエータOFF、ブートRAM書き込み禁止 */
		initrom_en = FALSE;
		bootram_rw = FALSE;
	}

	/* ブートRAM セットアップ */
	/* 割り込みベクタ以外を一旦すべてクリア(女神転生対策) */
	memset(boot_ram, 0, 0x1f0);

	/* FM-7モード時にはブートRAMの内容を転送する */
	mainmem_transfer_boot();

	/* イニシエータROM ハードウェアバージョン */
	switch (fm7_ver) {
		case 1:
			/* FM-7 */
			break;
		case 2:
			/* FM77AV */
			memset(&init_rom[0x0b0e], 0xff, 6);

			/* イニシエータがFM77AV相当の動作となるようにパッチ */
			if (patch_branewboottfr && patch_jmpnewboot) {
				patch_branewboottfr[0x0000] = 0x21;
				patch_jmpnewboot[0x0001] = 0xfe;
				patch_jmpnewboot[0x0002] = 0x00;
			}
			break;
#if XM7_VER >= 3
		case 3:
			/* FM77AV40EX */
			init_rom[0xb0e] = '4';
			init_rom[0xb0f] = '0';
			init_rom[0xb10] = '1';
			init_rom[0xb11] = 'M';
			init_rom[0xb12] = 'a';
			init_rom[0xb13] = '.';

			/* イニシエータがFM77AV40EX本来の動作となるよう元に戻す */
			if (patch_branewboottfr && patch_jmpnewboot) {
				patch_branewboottfr[0x0000] = 0x20;
				patch_jmpnewboot[0x0001] = 0x50;
				patch_jmpnewboot[0x0002] = 0x00;
			}
			break;
#endif
	}
#else
	/* ブート領域書き込み禁止 */
	bootram_rw = FALSE;
#endif
}

/*
 *	FM-7モード用 ブートRAM転送
 */
#if XM7_VER >= 2
void FASTCALL mainmem_transfer_boot(void)
{
	if (fm7_ver == 1) {
		if (boot_mode == BOOT_BASIC) {
			memcpy(boot_ram, &init_rom[0x1800], 0x1e0);
		}
		else {
			memcpy(boot_ram, &init_rom[0x1a00], 0x1e0);
		}
		boot_ram[0x1fe] = 0xfe;
		boot_ram[0x1ff] = 0x00;
	}
}
#endif

/*
 *	I/Oアクセス時のメモリレディ処理
 */
static void FASTCALL mainmem_iowait(void)
{
	BYTE tmp;

//#if XM7_VER == 1
	/* 1.2MHzモードではI/Oウェイトがかからない */
	if (lowspeed_mode && (fm7_ver == 1)) {
		return;
	}
//#endif

	/* クロック引き延ばしに必要なI/Oアクセス回数を設定 */
#if XM7_VER >= 3
	if ((mmr_flag || twr_flag) && !mmr_fastmode && ioaccess_flag) {
#else
	if ((mmr_flag || twr_flag) && ioaccess_flag) {
#endif
		/* MMRオン(2回目)  3回のアクセスで1サイクル */
		tmp = 3;
	}
	else {
		/* MMRオフ/高速モード/MMRオン(1回目)  2回のアクセスで1サイクル */
		tmp = 2;
	}

	/* 規定回数以上のI/Oアクセスがあれば、クロックを引き延ばす */
	ioaccess_count ++;
	if (ioaccess_count >= tmp) {
		maincpu.total ++;
		ioaccess_count = 0;

		ioaccess_flag = !ioaccess_flag;
	}
}


/*
 *	メインCPUメモリ
 *	１バイト取得
 */
volatile BYTE FASTCALL mainmem_readb(WORD addr)
{
	BYTE dat;

	/* MMR, TWRチェック */
	if (mmr_flag || twr_flag) {
		/* MMR、TWRを通す */
		if (mmr_extrb(&addr, &dat)) {
			return dat;
		}
	}

	/* メインRAM(表) */
#if XM7_VER >= 2
	if (addr < 0x6000) {
		return mainram_a[addr];
	}
	if (addr < 0x8000) {
		if (initrom_en) {
			return init_rom[addr - 0x6000];
		}
		else {
			return mainram_a[addr];
		}
	}
#else
	if (addr < 0x8000) {
		return mainram_a[addr];
	}
#endif

	/* BASIC ROM or メインRAM(裏) */
	if (addr < 0xfc00) {
		if (basicrom_en) {
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return basic_rom8[addr - 0x8000];
			}
#endif
			return basic_rom[addr - 0x8000];
		}
		else {
			return mainram_b[addr - 0x8000];
		}
	}

	/* メインROMの直後 */
	if (addr < 0xfc80) {
		return mainram_b[addr - 0x8000];
	}

	/* 共有RAM */
	if (addr < 0xfd00) {
		if (subhalt_flag) {
			return shared_ram[(WORD)(addr - 0xfc80)];
		}
		else {
			return 0xff;
		}
	}

	/* ブートROM/RAM */
#if XM7_VER >= 2
	if ((addr >= 0xfffe) && (initrom_en)) {
		/* リセットベクタ */
		mainmem_iowait();
		return init_rom[addr - 0xe000];
	}
#endif
	if (addr >= 0xfe00) {
		if (addr <= 0xffdf) {
			mainmem_iowait();
		}
#if XM7_VER >= 2
		return boot_ram[addr - 0xfe00];
#else
		if (((addr >= 0xffe0) && (addr < 0xfffe)) ||
			((fm_subtype == FMSUB_FM77) && bootram_rw)) {
			/* FM-77 RAM */
			return boot_ram[addr - 0xfe00];
		}
		else {
			if (fm_subtype == FMSUB_FM8) {
				/* FM-8 BOOT ROM */
				if (boot_mode == BOOT_BASIC) {
					return boot_bas8[addr - 0xfe00];
				}
				else if (boot_mode == BOOT_DOS) {
 					return boot_dos8[addr - 0xfe00];
 				}
				else {
					return boot_bbl8[addr - 0xfe00];
				}
			}
			else {
				/* FM-7 BOOT ROM */
				if (boot_mode == BOOT_BASIC) {
					return boot_bas[addr - 0xfe00];
				}
				else if (boot_mode == BOOT_DOS) {
 					return boot_dos8[addr - 0xfe00];
 				}
				else {
					return boot_bbl8[addr - 0xfe00];
				}
			}
		}
#endif
	}

	/*
	 *	I/O空間
	 */
	mainmem_iowait();
	if (mainetc_readb(addr, &dat)) {
		return dat;
	}
	if (ttlpalet_readb(addr, &dat)) {
		return dat;
	}
	if (subctrl_readb(addr, &dat)) {
		return dat;
	}
	if (multipag_readb(addr, &dat)) {
		return dat;
	}
	if (fdc_readb(addr, &dat)) {
		return dat;
	}
	if (kanji_readb(addr, &dat)) {
		return dat;
	}
	if (tapelp_readb(addr, &dat)) {
		return dat;
	}
	if (opn_readb(addr, &dat)) {
		return dat;
	}
	if (whg_readb(addr, &dat)) {
		return dat;
	}
	if (thg_readb(addr, &dat)) {
		return dat;
	}
	if (mmr_readb(addr, &dat)) {
		return dat;
	}
#if XM7_VER >= 2
	if (apalet_readb(addr, &dat)) {
		return dat;
	}
#endif
#ifdef MIDI
	if (midi_readb(addr, &dat)) {
		return dat;
	}
#endif
#ifdef RSC
	if (rs232c_readb(addr, &dat)) {
		return dat;
	}
#endif
#if XM7_VER >= 3
	if (dmac_readb(addr, &dat)) {
		return dat;
	}
#endif
#if (XM7_VER == 1 && defined(JSUB))
	if (jsub_readb(addr, &dat)) {
		return dat;
	}
#endif
#if (XM7_VER == 1 && defined(BUBBLE))
	if (bmc_readb(addr, &dat)) {
		return dat;
	}
#endif

	return 0xff;
}

/*
 *	メインCPUメモリ
 *	１バイト取得(I/Oなし)
 */
volatile BYTE FASTCALL mainmem_readbnio(WORD addr)
{
	BYTE dat;

	/* MMR, TWRチェック */
	if (mmr_flag || twr_flag) {
		/* MMR、TWRを通す */
		if (mmr_extbnio(&addr, &dat)) {
			return dat;
		}
	}

	/* メインRAM(表) */
#if XM7_VER >= 2
	if (addr < 0x6000) {
		return mainram_a[addr];
	}
	if (addr < 0x8000) {
		if (initrom_en) {
			return init_rom[addr - 0x6000];
		}
		else {
			return mainram_a[addr];
		}
	}
#else
	if (addr < 0x8000) {
		return mainram_a[addr];
	}
#endif

	/* BASIC ROM or メインRAM(裏) */
	if (addr < 0xfc00) {
		if (basicrom_en) {
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return basic_rom8[addr - 0x8000];
			}
#endif
			return basic_rom[addr - 0x8000];
		}
		else {
			return mainram_b[addr - 0x8000];
		}
	}

	/* メインROMの直後 */
	if (addr < 0xfc80) {
		return mainram_b[addr - 0x8000];
	}

	/* 共有RAM */
	if (addr < 0xfd00) {
		if (subhalt_flag) {
			return shared_ram[(WORD)(addr - 0xfc80)];
		}
		else {
			return 0xff;
		}
	}

	/* ブートROM/RAM */
#if XM7_VER >= 2
	if ((addr >= 0xfffe) && (initrom_en)) {
		/* リセットベクタ */
		return init_rom[addr - 0xe000];
	}
#endif
	if (addr >= 0xfe00) {
#if XM7_VER >= 2
		return boot_ram[addr - 0xfe00];
#else
		if (((addr >= 0xffe0) && (addr < 0xfffe)) ||
			((fm_subtype == FMSUB_FM77) && bootram_rw)) {
			return boot_ram[addr - 0xfe00];
		}
		else {
			if (fm_subtype == FMSUB_FM8) {
				if (boot_mode == BOOT_BASIC) {
					return boot_bas8[addr - 0xfe00];
				}
				else {
					return boot_dos8[addr - 0xfe00];
				}
			}
			else {
				if (boot_mode == BOOT_BASIC) {
					return boot_bas[addr - 0xfe00];
				}
				else {
					return boot_dos[addr - 0xfe00];
				}
			}
		}
#endif
	}

	/* I/O空間 */
	ASSERT((addr >= 0xfd00) && (addr < 0xfe00));
	return main_io[addr - 0xfd00];
}

/*
 *	メインCPUメモリ
 *	１バイト書き込み
 */
volatile void FASTCALL mainmem_writeb(WORD addr, BYTE dat)
{
	/* MMR, TWRチェック */
	if (mmr_flag || twr_flag) {
		/* MMR、TWRを通す */
		if (mmr_extwb(&addr, dat)) {
			return;
		}
	}

	/* メインRAM(表) */
#if XM7_VER >= 2
	if (addr < 0x6000) {
		mainram_a[addr] = dat;
		return;
	}
	if (addr < 0x8000) {
		if (!initrom_en) {
			mainram_a[addr] = dat;
		}
		return;
	}
#else
	if (addr < 0x8000) {
		mainram_a[addr] = dat;
		return;
	}
#endif

	/* BASIC ROM or メインRAM(裏) */
	if (addr < 0xfc00) {
		if (basicrom_en) {
			/* ROM内RCBでBIOSを呼び出すケース */
			return;
		}
		else {
			mainram_b[addr - 0x8000] = dat;
			return;
		}
	}

	/* メインROMの直後 */
	if (addr < 0xfc80) {
		mainram_b[addr - 0x8000] = dat;
		return;
	}

	/* 共有RAM */
	if (addr < 0xfd00) {
		if (subhalt_flag) {
			shared_ram[(WORD)(addr - 0xfc80)] = dat;
			return;
		}
		else {
			/* BASE09対策 */
			return;
		}
	}

	/* ブートRAM */
	if (addr >= 0xfe00) {
#if XM7_VER >= 2
		if ((bootram_rw) && (fm7_ver >= 2)) {
#else
		if ((fm_subtype == FMSUB_FM77) && bootram_rw) {
#endif
			boot_ram[addr - 0xfe00] = dat;
			if ((addr <= 0xffdf) || (addr >= 0xfffe)) {
				mainmem_iowait();
			}
			return;
		}

		/* ブートワークRAM、ベクタ */
		if ((addr >= 0xffe0) && (addr < 0xfffe)) {
			boot_ram[addr - 0xfe00] = dat;
		}
		return;
	}

	/*
	 *	I/O空間
	 */
	ASSERT((addr >= 0xfd00) && (addr < 0xfe00));
	main_io[(WORD)(addr - 0xfd00)] = dat;
	mainmem_iowait();

	if (mainetc_writeb(addr, dat)) {
		return;
	}
	if (ttlpalet_writeb(addr, dat)) {
		return;
	}
	if (subctrl_writeb(addr, dat)) {
		return;
	}
	if (multipag_writeb(addr, dat)) {
		return;
	}
	if (fdc_writeb(addr, dat)) {
		return;
	}
	if (kanji_writeb(addr, dat)) {
		return;
	}
	if (tapelp_writeb(addr, dat)) {
		return;
	}
	if (opn_writeb(addr, dat)) {
		return;
	}
	if (whg_writeb(addr, dat)) {
		return;
	}
	if (thg_writeb(addr, dat)) {
		return;
	}
	if (mmr_writeb(addr, dat)) {
		return;
	}
#if XM7_VER >= 2
	if (apalet_writeb(addr, dat)) {
		return;
	}
#endif
#ifdef MIDI
	if (midi_writeb(addr, dat)) {
		return;
	}
#endif
#ifdef RSC
	if (rs232c_writeb(addr, dat)) {
		return;
	}
#endif
#if XM7_VER >= 3
	if (dmac_writeb(addr, dat)) {
		return;
	}
#endif
#if (XM7_VER == 1 && defined(JSUB))
	if (jsub_writeb(addr, dat)) {
		return;
	}
#endif
#if (XM7_VER == 1 && defined(BUBBLE))
	if (bmc_writeb(addr, dat)) {
		return;
	}
#endif

	return;
}

/*
 *	メインCPUメモリ
 *	セーブ
 */
BOOL FASTCALL mainmem_save(SDL_RWops *fileh)
{
	if (!file_write(fileh, mainram_a, 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_write(fileh, mainram_b, 0x7e00)) {
#else
	if (!file_write(fileh, mainram_b, 0x7c80)) {
#endif
		return FALSE;
	}

	if (!file_write(fileh, main_io, 0x100)) {
		return FALSE;
	}

	if (!file_write(fileh, extram_a, 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x8000], 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_write(fileh, &extram_a[0x10000], 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x18000], 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x20000], 0x8000)) {
		return FALSE;
	}
	if (!file_write(fileh, &extram_a[0x28000], 0x8000)) {
		return FALSE;
	}
#endif
	if (!file_write(fileh, boot_ram, 0x200)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, basicrom_en)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_bool_write(fileh, initrom_en)) {
		return FALSE;
	}
#endif
	if (!file_bool_write(fileh, bootram_rw)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8拡張 */
	if (!file_write(fileh, &init_rom[0x0b0e], 3)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *	メインCPUメモリ
 *	ロード
 */
BOOL FASTCALL mainmem_load(SDL_RWops *fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_read(fileh, mainram_a, 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	/* Ver302拡張 */
	if (ver <= 301) {
		if (!file_read(fileh, mainram_b, 0x7c80)) {
			return FALSE;
		}
		memset(&mainram_b[0x7c80], 0, 0x180);
	}
	else {
		if (!file_read(fileh, mainram_b, 0x7e00)) {
			return FALSE;
		}
	}
#else
	if (!file_read(fileh, mainram_b, 0x7c80)) {
		return FALSE;
	}
#endif

	if (!file_read(fileh, main_io, 0x100)) {
		return FALSE;
	}

	if (!file_read(fileh, extram_a, 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x8000], 0x8000)) {
		return FALSE;
	}
#if XM7_VER == 1
	if (!file_read(fileh, &extram_a[0x10000], 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x18000], 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x20000], 0x8000)) {
		return FALSE;
	}
	if (!file_read(fileh, &extram_a[0x28000], 0x8000)) {
		return FALSE;
	}
#endif
	if (!file_read(fileh, boot_ram, 0x200)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &basicrom_en)) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (!file_bool_read(fileh, &initrom_en)) {
		return FALSE;
	}
#endif
	if (!file_bool_read(fileh, &bootram_rw)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8拡張 */
	if (ver >= 800) {
		if (!file_read(fileh, &init_rom[0x0b0e], 3)) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}

/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ システム管理 ]
 */

#include <string.h>
#include <stdlib.h>
#include "xm7.h"
#include "display.h"
#include "ttlpalet.h"
#include "subctrl.h"
#include "keyboard.h"
#include "fdc.h"
#include "mainetc.h"
#include "multipag.h"
#include "kanji.h"
#include "tapelp.h"
#include "display.h"
#include "opn.h"
#include "mmr.h"
#include "aluline.h"
#include "apalet.h"
#include "rtc.h"
#include "mouse.h"
#include "jsubsys.h"
/*
 * XM7/SDL依存 
 */
#include <sdl.h>
#include <sdl_draw.h>

#ifdef MIDI
#include "midi.h"
#endif
#if XM7_VER >= 3
#include "jcard.h"
#include "dmac.h"
#endif
#include "device.h"
#include "event.h"

/*
 *      グローバル ワーク
 */
int             fm7_ver;	/* ハードウェアバージョン */
int             boot_mode;	/* 起動モード BASIC/DOS */
#if XM7_VER == 1
BYTE            fm_subtype;	/* ハードウェアサブバージョン 
				 */
BOOL            lowspeed_mode;	/* 動作クロックモード */
BOOL            available_fm8roms;	/* FM-8 ROM使用可能フラグ */
BOOL            available_fm7roms;	/* FM-7 ROM使用可能フラグ */
BOOL            available_mmrboot;	/* FM-77 MMRブートROM使用可能フラグ */
#endif

BOOL            hotreset_flag;	/* メインホットリセットフラグ 
				 */
BOOL            reset_flag;	/* システムリセットフラグ */
BYTE            fetch_op;	/* 直前にフェッチした命令 */

/*
 *      ステートファイルヘッダ
 */
#if XM7_VER >= 3
const char     *state_header = "XM7 VM STATE 912";
#elif XM7_VER >= 2
const char     *state_header = "XM7 VM STATE 711";
#else
const char     *state_header = "XM7 VM STATE 302";
#endif

/*
 *      システム
 *      初期化
 */
BOOL            FASTCALL
system_init(void)
{
    /*
     * モード設定 
     */
#if XM7_VER >= 3
    fm7_ver = 3;		/* FM77AV40EX相当に設定 */
#elif XM7_VER >= 2
    fm7_ver = 2;		/* FM77AV相当に設定 */
#else
    fm7_ver = 1;		/* FM-77+192KB RAM相当に設定 */
    fm_subtype = FMSUB_FM77;
    lowspeed_mode = FALSE;
    available_fm8roms = TRUE;
    available_fm7roms = TRUE;
    available_mmrboot = TRUE;
#endif
    boot_mode = BOOT_BASIC;	/* BASIC MODE */

    hotreset_flag = FALSE;
    reset_flag = FALSE;

    /*
     * スケジューラ、メモリバス 
     */
    if (!schedule_init()) {
	return FALSE;
    }
    if (!mmr_init()) {
	return FALSE;
    }

    /*
     * メモリ、CPU 
     */
    if (!mainmem_init()) {
	return FALSE;
    }
    if (!submem_init()) {
	return FALSE;
    }
    if (!maincpu_init()) {
	return FALSE;
    }
    if (!subcpu_init()) {
	return FALSE;
    }
#if (XM7_VER == 1) && defined(JSUB)
    if (!jsubsys_init()) {
	return FALSE;
    }
#endif

    /*
     * その他デバイス 
     */
    if (!display_init()) {
	return FALSE;
    }
    if (!ttlpalet_init()) {
	return FALSE;
    }
    if (!subctrl_init()) {
	return FALSE;
    }
    if (!keyboard_init()) {
	return FALSE;
    }
    if (!fdc_init()) {
	return FALSE;
    }
    if (!mainetc_init()) {
	return FALSE;
    }
    if (!multipag_init()) {
	return FALSE;
    }
    if (!kanji_init()) {
	return FALSE;
    }
    if (!tapelp_init()) {
	return FALSE;
    }
    if (!opn_init()) {
	return FALSE;
    }
    if (!whg_init()) {
	return FALSE;
    }
    if (!thg_init()) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!aluline_init()) {
	return FALSE;
    }
    if (!apalet_init()) {
	return FALSE;
    }
    if (!rtc_init()) {
	return FALSE;
    }
#endif
#ifdef MIDI
    if (!midi_init()) {
	return FALSE;
    }
#endif
#if XM7_VER >= 3
    if (!jcard_init()) {
	return FALSE;
    }
    if (!dmac_init()) {
	return FALSE;
    }
#endif
#ifdef MOUSE
    if (!mos_init()) {
	return FALSE;
    }
#endif

    return TRUE;
}

/*
 *      システム
 *      クリーンアップ
 */
void            FASTCALL
system_cleanup(void)
{
    /*
     * その他デバイス 
     */
#ifdef MOUSE
    mos_cleanup();
#endif
#if XM7_VER >= 3
    dmac_cleanup();
    jcard_cleanup();
#endif
#ifdef MIDI
    midi_cleanup();
#endif
#if XM7_VER >= 2
    rtc_cleanup();
    apalet_cleanup();
    aluline_cleanup();
#endif
    thg_cleanup();
    whg_cleanup();
    opn_cleanup();
    tapelp_cleanup();
    kanji_cleanup();
    multipag_cleanup();
    mainetc_cleanup();
    fdc_cleanup();
    keyboard_cleanup();
    subctrl_cleanup();
    ttlpalet_cleanup();
    display_cleanup();

    /*
     * メモリ、CPU 
     */
#if (XM7_VER == 1) && defined(JSUB)
    jsubsys_cleanup();
#endif
    subcpu_cleanup();
    maincpu_cleanup();
    submem_cleanup();
    mainmem_cleanup();

    /*
     * スケジューラ、メモリバス 
     */
    mmr_cleanup();
    schedule_cleanup();
}

/*
 *      システム
 *      リセット
 */
void            FASTCALL
system_reset(void)
{
    /*
     * スケジューラ、メモリバス 
     */
    schedule_reset();
    mmr_reset();

    /*
     * その他デバイス 
     */
    display_reset();
    ttlpalet_reset();
    subctrl_reset();
    keyboard_reset();
    fdc_reset();
    mainetc_reset();
    multipag_reset();
    kanji_reset();
    tapelp_reset();
    opn_reset();
    whg_reset();
    thg_reset();
#if XM7_VER >= 2
    aluline_reset();
    apalet_reset();
    rtc_reset();
#endif
#ifdef MIDI
    midi_reset();
#endif
#if XM7_VER >= 3
    jcard_reset();
    dmac_reset();
#endif
#ifdef MOUSE
    mos_reset();
#endif

    /*
     * メモリ、CPU 
     */
    mainmem_reset();
    submem_reset();
    maincpu_reset();
    subcpu_reset();
#if (XM7_VER == 1) && defined(JSUB)
    jsubsys_reset();
#endif

    /*
     * 画面再描画 
     */
    display_setpointer(FALSE);
    display_notify();

    /*
     * システムリセットフラグを設定 
     */
    reset_flag = TRUE;

    /*
     * フェッチ命令初期化 
     */
    fetch_op = 0;
}

/*
 *      システム
 *      ホットリセット
 */
void            FASTCALL
system_hotreset(void)
{
#if XM7_VER == 1
    BOOL            flag;
#endif

    /*
     * TWRが有効なら起動モードをDOSに切り替え 
     */
#if XM7_VER >= 2
    if (twr_flag) {
	boot_mode = BOOT_DOS;
    }
#else
    flag = twr_flag;
#endif

    /*
     * システムリセット 
     */
    system_reset();

    /*
     * BREAKキーを押下状態にする 
     */
    break_flag = TRUE;
    maincpu_firq();

    /*
     * ホットリセットフラグ有効 
     */
    hotreset_flag = TRUE;

#if XM7_VER == 1
    /*
     * リセット直前にTWRが有効だったら裏RAMを選択 
     */
    if (flag) {
	basicrom_en = FALSE;
    }
#endif
}

/*
 *      システム
 *      ファイルセーブ
 */
BOOL            FASTCALL
system_save(char *filename)
{
    int             fileh;
    BOOL            flag;
    ASSERT(filename);

    /*
     * ファイルオープン 
     */
    fileh = file_open(filename, OPEN_W);	/* BUG:
						 * U*ixではWのみのファイルは読み込めない 
						 */
    if (fileh == -1) {
	return FALSE;
    }

    /*
     * フラグ初期化 
     */
    flag = TRUE;

    /*
     * ヘッダをセーブ 
     */
    if (!file_write(fileh, (BYTE *) state_header, 16)) {
	flag = FALSE;
    }

    /*
     * システムワーク 
     */
    if (!file_word_write(fileh, (WORD) fm7_ver)) {
	return FALSE;
    }
    if (!file_word_write(fileh, (WORD) boot_mode)) {
	return FALSE;
    }
#if XM7_VER == 1
    if (!file_byte_write(fileh, fm_subtype)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lowspeed_mode)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, available_fm8roms)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, available_fm7roms)) {
	return FALSE;
    }
#endif

    /*
     * 順番に呼び出す 
     */
    if (!mainmem_save(fileh)) {
	flag = FALSE;
    }
    if (!submem_save(fileh)) {
	flag = FALSE;
    }
    if (!maincpu_save(fileh)) {
	flag = FALSE;
    }
    if (!subcpu_save(fileh)) {
	flag = FALSE;
    }
    if (!schedule_save(fileh)) {
	flag = FALSE;
    }
    if (!display_save(fileh)) {
	flag = FALSE;
    }
    if (!ttlpalet_save(fileh)) {
	flag = FALSE;
    }
    if (!subctrl_save(fileh)) {
	flag = FALSE;
    }
    if (!keyboard_save(fileh)) {
	flag = FALSE;
    }
    if (!fdc_save(fileh)) {
	flag = FALSE;
    }
    if (!mainetc_save(fileh)) {
	flag = FALSE;
    }
    if (!multipag_save(fileh)) {
	flag = FALSE;
    }
    if (!kanji_save(fileh)) {
	flag = FALSE;
    }
    if (!tapelp_save(fileh)) {
	flag = FALSE;
    }
    if (!opn_save(fileh)) {
	flag = FALSE;
    }
    if (!mmr_save(fileh)) {
	flag = FALSE;
    }
#if XM7_VER >= 2
    if (!aluline_save(fileh)) {
	flag = FALSE;
    }
    if (!rtc_save(fileh)) {
	flag = FALSE;
    }
    if (!apalet_save(fileh)) {
	flag = FALSE;
    }
#endif
    if (!whg_save(fileh)) {
	flag = FALSE;
    }
    if (!thg_save(fileh)) {
	flag = FALSE;
    }
#if XM7_VER >= 3
    if (!jcard_save(fileh)) {
	flag = FALSE;
    }
    if (!dmac_save(fileh)) {
	flag = FALSE;
    }
#endif
#ifdef MOUSE
    if (!mos_save(fileh)) {
	flag = FALSE;
    }
#endif
#ifdef MIDI
    if (!midi_save(fileh)) {
	flag = FALSE;
    }
#endif
#if (XM7_VER == 1) && defined(JSUB)
    if (!jsubsys_save(fileh)) {
	flag = FALSE;
    }
#endif
    file_close(fileh);

    return flag;
}

/*
 *      システム
 *      ファイルロード
 */
int             FASTCALL
system_load(char *filename)
{
    int             fileh;
    int             ver;
    char            header[16];
    BOOL            flag;
    BOOL            old_scheduler;
    WORD            tmp;
    int             filesize;
#if XM7_VER == 1
    BOOL            temp;
#endif

    ASSERT(filename);

    /*
     * ファイルオープン 
     */
    fileh = file_open(filename, OPEN_R);
    if (fileh == -1) {
	return STATELOAD_OPENERR;
    }

    /*
     * フラグ初期化 
     */
    flag = TRUE;
    old_scheduler = FALSE;

    /*
     * ヘッダをロード 
     */
    if (!file_read(fileh, (BYTE *) header, 16)) {
	flag = FALSE;
    } else {
	if (memcmp(header, "XM7 VM STATE ", 13) != 0) {
	    flag = FALSE;
	}
    }

    /*
     * ヘッダチェック 
     */
    if (!flag) {
	file_close(fileh);
	return STATELOAD_HEADERR;
    }

    /*
     * ファイルバージョン取得、バージョン2以上が対象 
     */
    if (header[13] != 0x20) {
	ver = (int) ((BYTE) (header[13] - 0x30) * 100) +
	    ((BYTE) (header[14] - 0x30) * 10) +
	    ((BYTE) (header[15] - 0x30));
    } else {
	ver = (int) (BYTE) (header[15]);
	ver -= 0x30;
	ver *= 100;
    }
#if XM7_VER >= 3
    /*
     * V3 : Ver5未満はロードできない 
     */
    if ((ver < 200) || ((ver >= 300) && (ver <= 499))) {
	file_close(fileh);
	return STATELOAD_VERERR;
    }
#elif XM7_VER >= 2
    /*
     * V2 : Ver8以上・Ver5未満はロードできない 
     */
    if ((ver >= 800) || (ver < 200) || ((ver >= 300) && (ver <= 499))) {
	file_close(fileh);
	return STATELOAD_VERERR;
    }
#else
    /*
     * V1 : Ver5以上・Ver2未満はロードできない 
     */
    if ((ver >= 500) || (ver < 200)) {
	file_close(fileh);
	return STATELOAD_VERERR;
    }
#endif
    if (ver > (int) ((BYTE) (state_header[13] - 0x30) * 100) +
	((BYTE) (state_header[14] - 0x30) * 10) +
	((BYTE) (state_header[15] - 0x30))) {
	file_close(fileh);
	return STATELOAD_VERERR;
    }
#if XM7_VER >= 2
    /*
     * V3.0L30/V2.5L20以前のステートファイルに対処 
     */
    filesize = file_getsize(fileh);
    if (ver <= 500) {
	old_scheduler = TRUE;
    } else {
#if XM7_VER >= 3
	if ((filesize == 454758) ||	/* XM7 V3.0 (増設RAM無効) */
	    (filesize == 1241190) ||	/* XM7 V3.0 (増設RAM有効) */
	    (filesize == 258030)) {	/* XM7 V2 */
	    old_scheduler = TRUE;
	}
#else
	if (filesize == 258030) {	/* XM7 V2 */
	    old_scheduler = TRUE;
	}
#endif
    }
#else
    old_scheduler = FALSE;
#endif

    /*
     * システムワーク 
     */
    if (!file_word_read(fileh, &tmp)) {
	return FALSE;
    }
    fm7_ver = (int) tmp;
    if (!file_word_read(fileh, &tmp)) {
	return FALSE;
    }
    boot_mode = (int) tmp;
#if XM7_VER == 1
    if (!file_byte_read(fileh, &fm_subtype)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lowspeed_mode)) {
	return FALSE;
    }

    /*
     * セーブ時に有効だったROMデータがロード時に有効でない場合エラー 
     */
    if (!file_bool_read(fileh, &temp)) {
	return FALSE;
    }
    if (!available_fm8roms && temp) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &temp)) {
	return FALSE;
    }
    if (!available_fm7roms && temp) {
	return FALSE;
    }
#endif

    /*
     * 順番に呼び出す 
     */
    if (!mainmem_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!submem_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!maincpu_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!subcpu_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!schedule_load(fileh, ver, old_scheduler)) {
	flag = FALSE;
    }

    if (!display_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!ttlpalet_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!subctrl_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!keyboard_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!fdc_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!mainetc_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!multipag_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!kanji_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!tapelp_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!opn_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!mmr_load(fileh, ver)) {
	flag = FALSE;
    }
#if XM7_VER >= 2
    if (!aluline_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!rtc_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!apalet_load(fileh, ver)) {
	flag = FALSE;
    }
#endif
    if (!whg_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!thg_load(fileh, ver)) {
	flag = FALSE;
    }
#if XM7_VER >= 3
    if (!jcard_load(fileh, ver)) {
	flag = FALSE;
    }

    if (!dmac_load(fileh, ver)) {
	flag = FALSE;
    }
#endif
#ifdef MOUSE
    if (!mos_load(fileh, ver)) {
	flag = FALSE;
    }
#endif
#ifdef MIDI
    if (!midi_load(fileh, ver)) {
	flag = FALSE;
    }
#endif
#if (XM7_VER == 1) && defined(JSUB)
    if (!jsubsys_load(fileh, ver)) {
	flag = FALSE;
    }
#endif
    file_close(fileh);


#if XM7_VER >= 3
    /*
     * 400ラインモードのみVRAM配置補正が必要 
     */
    if (mode400l) {
	if (!fix_vram_address()) {
	    flag = FALSE;
	}
    }
#endif

    /*
     * 画面再描画 
     */
    display_notify();

    /*
     * CPU速度比率設定 
     */
    speed_ratio = 10000;

    if (!flag) {
	return STATELOAD_ERROR;
    }

    return STATELOAD_SUCCESS;
}

/*
 *      ファイル読み込み(BYTE)
 */
BOOL            FASTCALL
file_byte_read(int fileh, BYTE * dat)
{
    return file_read(fileh, dat, 1);
}

/*
 *      ファイル読み込み(WORD)
 */
BOOL            FASTCALL
file_word_read(int fileh, WORD * dat)
{
    BYTE            tmp;

    if (!file_read(fileh, &tmp, 1)) {
	return FALSE;
    }
    *dat = tmp;

    if (!file_read(fileh, &tmp, 1)) {
	return FALSE;
    }
    *dat <<= 8;
    *dat |= tmp;

    return TRUE;
}

/*
 *      ファイル読み込み(DWORD)
 */
BOOL            FASTCALL
file_dword_read(int fileh, DWORD * dat)
{
    BYTE            tmp;

    if (!file_read(fileh, &tmp, 1)) {
	return FALSE;
    }
    *dat = tmp;

    if (!file_read(fileh, &tmp, 1)) {
	return FALSE;
    }
    *dat *= 256;
    *dat |= tmp;

    if (!file_read(fileh, &tmp, 1)) {
	return FALSE;
    }
    *dat *= 256;
    *dat |= tmp;

    if (!file_read(fileh, &tmp, 1)) {
	return FALSE;
    }
    *dat *= 256;
    *dat |= tmp;

    return TRUE;
}

/*
 *      ファイル読み込み(BOOL)
 */
BOOL            FASTCALL
file_bool_read(int fileh, BOOL * dat)
{
    BYTE            tmp;

    if (!file_read(fileh, &tmp, 1)) {
	return FALSE;
    }

    switch (tmp) {
    case 0:
	*dat = FALSE;
	return TRUE;
    case 0xff:
	*dat = TRUE;
	return TRUE;
    }

    return FALSE;
}

/*
 *      ファイル書き込み(BYTE)
 */
BOOL            FASTCALL
file_byte_write(int fileh, BYTE dat)
{
    return file_write(fileh, &dat, 1);
}

/*
 *      ファイル書き込み(WORD)
 */
BOOL            FASTCALL
file_word_write(int fileh, WORD dat)
{
    BYTE            tmp;

    tmp = (BYTE) (dat >> 8);
    if (!file_write(fileh, &tmp, 1)) {
	return FALSE;
    }

    tmp = (BYTE) (dat & 0xff);
    if (!file_write(fileh, &tmp, 1)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      ファイル書き込み(DWORD)
 */
BOOL            FASTCALL
file_dword_write(int fileh, DWORD dat)
{
    BYTE            tmp;

    tmp = (BYTE) (dat >> 24);
    if (!file_write(fileh, &tmp, 1)) {
	return FALSE;
    }

    tmp = (BYTE) (dat >> 16);
    if (!file_write(fileh, &tmp, 1)) {
	return FALSE;
    }

    tmp = (BYTE) (dat >> 8);
    if (!file_write(fileh, &tmp, 1)) {
	return FALSE;
    }

    tmp = (BYTE) (dat & 0xff);
    if (!file_write(fileh, &tmp, 1)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      ファイル書き込み(BOOL)
 */
BOOL            FASTCALL
file_bool_write(int fileh, BOOL dat)
{
    BYTE            tmp;

    if (dat) {
	tmp = 0xff;
    } else {
	tmp = 0;
    }

    if (!file_write(fileh, &tmp, 1)) {
	return FALSE;
    }

    return TRUE;
}

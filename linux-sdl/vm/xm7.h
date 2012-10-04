/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2011 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2011 Ryu Takegami
 *
 *      [ 共通定義 ]
 */

#ifndef _xm7_h_
#define _xm7_h_

#include <stdio.h>
#include "build_config.h"
#ifdef USE_AGAR
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include "xm7_types.h"
#endif
//#ifndef _WINDOWS
#include <SDL/SDL.h>
//#endif

#ifdef MEMWATCH
#include <memwatch.h>
#endif

/*
 *      バージョン
 */
#if XM7_VER >= 3
#define	VERSION		"V3.4"
#elif XM7_VER >= 2
#define	VERSION		"V2.9"
#else
#define	VERSION		"V1.1"
#endif
#if XM7_VER == 1
#define	LEVEL		"L41"
// #define BETAVER
#else
#define	LEVEL		"L51"
// #define BETAVER
#endif

#ifdef USE_OPENGL
#define          LOCALVER         "SDL/Agar/OpenGL 0.3.2pre1"
#else
#define          LOCALVER         "SDL/Agar 0.3.2pre1"
#endif
#define	DATE		"2012/10/05"

/* 未使用変数の処理 */
#ifndef UNUSED
#define UNUSED(x)              ((void)(x))
#endif


/*
 *      定数、型定義
 */

/*
 * 汎用定数
 */
#ifndef FALSE
#define FALSE			0
#define TRUE			(!FALSE)
#endif
#ifndef NULL
#define NULL			((void)0)
#endif

/*
 * 診断
 */
#ifndef ASSERT
#ifdef _DEBUG
#include <assert.h>
#define ASSERT(exp)		assert(exp)
#else
#define ASSERT(exp)		((void)0)
#endif
#endif

/*
 * 最適化 ->FASTCALLは64bitだとバグることがあるのですっぱり削除
 */
#if defined(FASTCALL)
#undef FASTCALL
#define FASTCALL
#else
#define FASTCALL
#endif
/*
 * CPUレジスタ定義
 */
#pragma pack(push, 1)
typedef struct
{
    BYTE            cc;
    BYTE            dp;
    union
    {
        struct
        {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            BYTE b __attribute__ ((aligned(1)));
            BYTE a __attribute__ ((aligned(1)));
#else
            BYTE a __attribute__ ((aligned(1)));
            BYTE b __attribute__ ((aligned(1)));
#endif
        } h;
        WORD            d;
    } acc;
    WORD            x;
    WORD            y;
    WORD            u;
    WORD            s;
    WORD            pc;
    WORD            intr;
    WORD            cycle;
    WORD            total;
    BYTE            (FASTCALL *readmem) (WORD);
    void            (FASTCALL *writemem) (WORD, BYTE);
    WORD            ea;
} cpu6809_t;
#pragma pack(pop)

/*
 * 割り込み定義
 */
#define INTR_NMI		0x0001	/* NMI割り込み */
#define INTR_FIRQ		0x0002	/* FIRQ割り込み */
#define INTR_IRQ		0x0004	/* IRQ割り込み */

#define INTR_SLOAD		0x0010	/* リセット後Sを設定 */
#define INTR_SYNC_IN	0x0020	/* SYNC実行中 */
#define INTR_SYNC_OUT	0x0040	/* SYNC終了可能 */
#define INTR_CWAI_IN	0x0080	/* CWAI実行中 */
#define INTR_CWAI_OUT	0x0100	/* CWAI終了可能 */
#define INTR_NMI_LC	0x1000			/* NMI割り込み信号3サイクル未満 */
#define INTR_FIRQ_LC	0x2000			/* FIRQ割り込み信号3サイクル未満 */
#define INTR_IRQ_LC	0x4000			/* IRQ割り込み信号3サイクル未満 */
#define INTR_HALT	0x8000			/* HALT命令実行中 */

/*
 * ブレークポイント定義
 */
#define BREAKP_NOTUSE	0	/* 未使用 */
#define BREAKP_ENABLED	1	/* 使用中 */
#define BREAKP_DISABLED	2	/* 禁止中 */
#define BREAKP_STOPPED	3	/* 停止中 */
#define BREAKP_MAXNUM	16	/* ブレークポイントの個数 */
#define BREAKP_MAXNUM_OLD	8	/* ブレークポイントの個数
* (〜V3.1L10) */
typedef struct
{
    int             flag;	/* 上のフラグ */
    int             cpu;	/* CPU種別 */
    WORD            addr;	/* ブレークポイントアドレス */
} breakp_t;

/*
 * イベント定義
 */
#define EVENT_NOTUSE	0	/* 未使用 */
#define EVENT_ENABLED	1	/* 使用中 */
#define EVENT_DISABLED	2	/* 禁止中 */
#define	EVENT_MAXNUM	24	/* イベントの個数 */
#define EVENT_MAXNUM_L31	18	/* V3.0L31でのイベントの個数
*/
#define	EVENT_MAXNUM_L30	16	/* V3.0L30でのイベントの個数
*/
typedef struct
{
    int             flag;	/* 上のフラグ */
    DWORD           current;	/* カレント時間カウンタ */
    DWORD           reload;	/* リロード時間カウンタ */
    BOOL(FASTCALL * callback) (void);	/* コールバック関数
							 */
} event_t;

/*
 * CPU実行サイクル数
 */
#define MAINCYCLES		1794	/* メインCPU MMR OFF */
#define MAINCYCLES_MMR	1565	/* メインCPU MMR ON */
#define MAINCYCLES_FMMR	2016	/* メインCPU 高速MMR */
#define SUBCYCLES		2000	/* サブCPU */
#define MAINCYCLES_LOW	1095	/* メインCPU MMR OFF(低速) */
#define SUBCYCLES_LOW	999	/* サブCPU(低速) */
#define JSUBCYCLES		1228	/* 日本語サブCPU */

/*
 * ハードウェアサブバージョン
 */
#define FMSUB_FM7		0x00	/* FM-7 */
#define FMSUB_FM77		0x01	/* FM-77 */
#define FMSUB_FM8		0x02	/* FM-8 */

/*
 * その他定数
 */
#define MAINCPU			0	/* メインCPU */
#define SUBCPU			1	/* サブCPU */
#define JSUBCPU			2	/* 日本語サブCPU */
#define CPU_VOID		255	/* 無効(どのCPUでもない) */
#if XM7_VER >= 2
#define MAXCPU			2	/* 最大CPU:2個(V2,V3) */
#else
#define MAXCPU			3	/* 最大CPU:3個(V1) */
#endif

#define BOOT_BASIC		0	/* BASICモード */
#define BOOT_DOS		1	/* DOSモード */
#define BOOT_BUBBLE		2	/* BUBBLEモード */


#define STATELOAD_ERROR		0	/* ステートロードエラー */
#define STATELOAD_SUCCESS	1	/* ステートロード成功 */
#define STATELOAD_OPENERR	-1	/* ファイルオープンエラー
*/
#define STATELOAD_HEADERR	-2	/* ヘッダエラー */
#define STATELOAD_VERERR	-3	/* バージョンエラー */

/*
 * ROMファイル名定義
 */

#define FBASIC_ROM		"FBASIC30.ROM" /* F-BASIC 3.0 */
#define FBASIC10_ROM	"FBASIC10.ROM"	/* F-BASIC V1.0 */
#define BOOTBAS_ROM		"BOOT_BAS.ROM" /* FM-7 BASIC BOOT */
#define BOOTDOS_ROM		"BOOT_DOS.ROM" /* FM-7 5inch DOS BOOT */
#define BOOTBBL_ROM             "BOOT_BBL.ROM"  /* FM-7 Bubble BOOT */
#define BOOT1MB_ROM		"BOOT_1MB.ROM"	/* FM-77 1MB BOOT */
#define BOOTMMR_ROM		"BOOT_MMR.ROM"	/* FM-77 MMR BOOT */
#define BOOTBAS8_ROM	"BOOTBAS8.ROM"	/* MICRO 8 BASIC BOOT */
#define BOOTDOS8_ROM	"BOOTDOS8.ROM"	/* MICRO 8 5inchDOS BOOT */
#define BOOTBBL8_ROM	"BOOTBBL8.ROM"	/* MICRO 8 Bubble BOOT */
#define BOOTSFD8_ROM	"BOOTSFD8.ROM"	/* MICRO 8 8inchDOS BOOT */
#define SUBSYSC_ROM		"SUBSYS_C.ROM"  /* FM-7 SUBSYSTEM */
#define SUBSYS8_ROM		"SUBSYS_8.ROM"	/* MICRO 8 SUBSYSTEM */
#define SUBSYSL4_ROM	"SUBSYSL4.ROM"	/* FM-77L4 400LINE SUBSYSTEM */
#define ANKCG16_ROM		"ANKCG16.ROM"	/* FM-77L4 400LINE ANK FONT */
#define KANJI_ROM              "KANJI1.ROM"    /* JIS83 FIRST STANDARD KANJI ROM */
#define KANJI_ROM_J78   "KANJI.ROM"             /* JIS78 FIRST STANDARD KANJI ROM */
#define JSUBSYS_ROM            "JSUBMON.ROM"   /* FM77-101 DICTIONARY ACCESS ROM */
#define JSUBDIC_ROM            "DICROM.ROM"    /* FM77-101/FM77-211 DICTIONARY ROM */
#define EXTSUB_ROM             "EXTSUB.ROM"    /* FM77AV40EX/SX EXTEND SUBSYSTEM */

#if XM7_VER >= 2
#define INITIATE_ROM   "INITIATE.ROM"  /* FM77AV INITIATER ROM */
#define SUBSYSA_ROM            "SUBSYS_A.ROM"  /* FM77AV Type-A SUBSYSTEM 3 */
#define SUBSYSB_ROM            "SUBSYS_B.ROM"  /* FM77AV Type-B SUBSYSTEM 3 */
#define SUBSYSCG_ROM   "SUBSYSCG.ROM"  /* FM77AV CGROM/SUBSYSTEM 1/2 */
#endif

#if XM7_VER >= 3
#define KANJI_ROM2             "KANJI2.ROM"    /* JIS83 SECOND STANDARD KANJI ROM */
#define DICT_ROM               "DICROM.ROM"    /* FM77-101/FM77-211 DICTIONARY ROM */
#define DICT_RAM                "USERDIC.DAT"   /* FM77-211 BATTERY BACKUP RAM */
#endif

/*
 *     Borland C++での鬼のようなWarning対策
 */
#ifdef __BORLANDC__
#undef MAKELONG
#define MAKELONG(l, h)                 (LONG)(((WORD)(l)) | ((DWORD)(h) << 16))
#undef MAKEWPARAM
#define MAKEWPARAM(l, h)               (WPARAM)(MAKELONG(l, h))
#undef MAKELPARAM
#define MAKELPARAM(l, h)               (LPARAM)(MAKELONG(l, h))
#endif


#ifdef __cplusplus
extern          "C" {
#endif
    /*
     *      主要エントリ
     */

    /*
     * システム(system.c)
     */
    BOOL FASTCALL   system_init(void);
    /*
     * システム 初期化
     */
    void FASTCALL   system_cleanup(void);
    /*
     * システム クリーンアップ
     */
    void FASTCALL   system_reset(void);
    /*
     * システム リセット
     */
    void FASTCALL   system_hotreset(void);
    /*
     * システム ホットリセット
     */
    BOOL FASTCALL   system_save(char *filename);
    /*
     * システム セーブ
     */
    int FASTCALL    system_load(char *filename);
    /*
     * システム ロード
     */

    /*
     * スケジューラ(schedule.c)
     */
    BOOL FASTCALL   schedule_init(void);
    /*
     * スケジューラ 初期化
     */
    void FASTCALL   schedule_cleanup(void);
    /*
     * スケジューラ クリーンアップ
     */
    void FASTCALL   schedule_reset(void);
    /*
     * スケジューラ リセット
     */
    DWORD FASTCALL  schedule_exec(DWORD microsec);
    /*
     * 実行
     */
    void FASTCALL   schedule_main_fullspeed(void);
    /*
     * メインだけ全力駆動(tape用)
     */
    void FASTCALL   schedule_fullspeed(void);
    /*
     * 全力駆動
     */
    void FASTCALL   schedule_trace(void);
    /*
     * トレース
     */
    BOOL FASTCALL   schedule_setbreak(int cpu, WORD addr);
    /*
     * ブレークポイント設定
     */
    BOOL FASTCALL   schedule_setbreak2(int num, int cpu, WORD addr);
    /*
     * ブレークポイント設定(位置指定)
     */
    BOOL FASTCALL   schedule_setevent(int id, DWORD microsec,
                                      BOOL(FASTCALL * func) (void));
    /*
     * イベント追加
     */
    BOOL FASTCALL   schedule_delevent(int id);
    /*
     * イベント削除
     */
    void FASTCALL schedule_handle(int id, BOOL(FASTCALL * func) (void));
    /*
     * イベントハンドラ設定
     */
    BOOL FASTCALL   schedule_save(SDL_RWops *fileh);
    /*
     * スケジューラ セーブ
     */
    BOOL FASTCALL   schedule_load(SDL_RWops *fileh, int ver, BOOL old);
    /*
     * スケジューラ ロード
     */

    /*
     * 逆アセンブラ(disasm.c)
     */
    int FASTCALL    disline(int cputype, WORD pc, char *buffer);
    /*
     * １行逆アセンブル
     */

    /*
     * メインCPUメモリ(mainmem.c)
     */
    BOOL FASTCALL   mainmem_init(void);
    /*
     * メインCPUメモリ 初期化
     */
    void FASTCALL   mainmem_cleanup(void);
    /*
     * メインCPUメモリ クリーンアップ
     */
    void FASTCALL mainmem_reset(void);
    /*
     * メインCPUメモリ リセット
     */
    void FASTCALL mainmem_transfer_boot(void);
    /*
     * メインCPUメモリ ブート転送
     */
    volatile BYTE FASTCALL mainmem_readb(WORD addr);
    /*
     * メインCPUメモリ 読み出し
     */
    volatile BYTE mainmem_readbnio(WORD addr);
    /*
     * メインCPUメモリ 読み出し(I/Oなし)
     */
    void FASTCALL mainmem_writeb(WORD addr, BYTE dat);
    /*
     * メインCPUメモリ 書き込み
     */
    BOOL FASTCALL mainmem_save(SDL_RWops *fileh);
    /*
     * メインCPUメモリ セーブ
     */
    BOOL FASTCALL   mainmem_load(SDL_RWops *fileh, int ver);
    /*
     * メインCPUメモリ ロード
     */

    /*
     * サブCPUメモリ(submem.c)
     */
    BOOL FASTCALL   submem_init(void);
    /*
     * サブCPUメモリ 初期化
     */
    void FASTCALL   submem_cleanup(void);
    /*
     * サブCPUメモリ クリーンアップ
     */
    void FASTCALL   submem_reset(void);
    /*
     * サブCPUメモリ リセット
     */
    volatile BYTE FASTCALL   submem_readb(WORD addr);
    /*
     * サブCPUメモリ 読み出し
     */
    volatile BYTE submem_readbnio(WORD addr);
    /*
     * サブCPUメモリ 読み出し(I/Oなし)
     */
    void FASTCALL   submem_writeb(WORD addr, BYTE dat);
    /*
     * サブCPUメモリ 書き込み
     */
    BOOL FASTCALL   submem_save(SDL_RWops *fileh);
    /*
     * サブCPUメモリ セーブ
     */
    BOOL FASTCALL   submem_load(SDL_RWops *fileh, int ver);
    /*
     * サブCPUメモリ ロード
     */

    /*
     * メインCPU(maincpu.c)
     */
    BOOL FASTCALL   maincpu_init(void);
    /*
     * メインCPU 初期化
     */
    void FASTCALL   maincpu_cleanup(void);
    /*
     * メインCPU クリーンアップ
     */
    void FASTCALL   maincpu_reset(void);
    /*
     * メインCPU リセット
     */
    void FASTCALL   maincpu_execline(void);
    /*
     * メインCPU １行実行
     */
    void FASTCALL   maincpu_exec(void);
    /*
     * メインCPU 実行
     */
    void FASTCALL   maincpu_nmi(void);
    /*
     * メインCPU NMI割り込み要求
     */
    void FASTCALL   maincpu_firq(void);
    /*
     * メインCPU FIRQ割り込み要求
     */
    void FASTCALL   maincpu_irq(void);
    /*
     * メインCPU IRQ割り込み要求
     */
    BOOL FASTCALL   maincpu_save(SDL_RWops *fileh);
    /*
     * メインCPU セーブ
     */
    BOOL FASTCALL   maincpu_load(SDL_RWops *fileh, int ver);
    /*
     * メインCPU ロード
     */

    /*
     * サブCPU(subcpu.c)
     */
    BOOL FASTCALL   subcpu_init(void);
    /*
     * サブCPU 初期化
     */
    void FASTCALL   subcpu_cleanup(void);
    /*
     * サブCPU クリーンアップ
     */
    void FASTCALL   subcpu_reset(void);
    /*
     * サブCPU リセット
     */
    void FASTCALL   subcpu_execline(void);
    /*
     * サブCPU １行実行
     */
    void FASTCALL   subcpu_exec(void);
    /*
     * サブCPU 実行
     */
    void FASTCALL   subcpu_nmi(void);
    /*
     * サブCPU NMI割り込み要求
     */
    void FASTCALL   subcpu_firq(void);
    /*
     * サブCPU FIRQ割り込み要求
     */
    void FASTCALL   subcpu_irq(void);
    /*
     * サブCPU IRQ割り込み要求
     */
    BOOL FASTCALL   subcpu_save(SDL_RWops *fileh);
    /*
     * サブCPU セーブ
     */
    BOOL FASTCALL   subcpu_load(SDL_RWops *fileh, int ver);
    /*
     * サブCPU ロード
     */

    /*
     *      CPU、その他主要ワークエリア
     */
    extern cpu6809_t maincpu;
    /*
     * メインCPU
     */
    extern cpu6809_t subcpu;
    /*
     * サブCPU
     */
#if XM7_VER == 1
    extern cpu6809_t jsubcpu;
    /*
     * 日本語サブCPU
     */
#endif
    extern DWORD    main_cycles;
    /*
     * デバッガ関連
     */
#define DISASM_BUF_SIZE 32768
    extern BOOL	     disasm_main_flag;
    extern int		 disasm_main_count;
    extern BOOL	     disasm_sub_flag;
    extern int		 disasm_sub_count;

//    extern int  FASTCALL disline(int cpu, WORD pcreg, char *buffer);
    /*
     * メインCPU実行サイクル数
     */
    extern int      fm7_ver;
    /*
     * 動作バージョン
     */
    extern breakp_t breakp[BREAKP_MAXNUM];
    /*
     * ブレークポイント
     */
    extern event_t  event[EVENT_MAXNUM];
    /*
     * イベント
     */
    extern BOOL     run_flag;
    /*
     * 動作フラグ
     */
    extern BOOL     stopreq_flag;
    /*
     * 停止要求フラグ
     */
    extern DWORD    main_speed;
    /*
     * メインCPUスピード
     */
    extern DWORD    mmr_speed;
    /*
     * メインCPU(MMR)スピード
     */
#if XM7_VER >= 3
    extern DWORD    fmmr_speed;
    /*
     * メインCPU(高速MMR)スピード
     */
#endif
    extern DWORD    sub_speed;
    /*
     * サブCPUスピード
     */
    extern DWORD    speed_ratio;
    /*
     * サブCPUスピード
     */
    extern BOOL     cycle_steal;
    /*
     * サイクルスチールフラグ
     */
    extern DWORD    vmtime;
    /*
     * VM仮想時間
     */
    extern BOOL     reset_flag;
    /*
     * システムリセットフラグ
     */
    extern BOOL     hotreset_flag;
    /*
     * メインホットリセットフラグ
     */
    extern WORD     main_overcycles;
    /*
     * メインCPUオーバーサイクル数
     */
    extern WORD     sub_overcycles;
    /*
     * サブCPUオーバーサイクル数
     */
#ifdef DEBUG
    extern DWORD    main_cycle;
    /*
     * メインCPU実行サイクル数カウンタ
     */
    extern DWORD    sub_cycle;
    /*
     * サブCPU実行サイクル数カウンタ
     */
#endif
    extern BYTE     fetch_op;
    /*
     * 直前にフェッチした命令コード
     */
#if XM7_VER == 1
    extern BYTE     fm_subtype;
    /*
     * ハードウェアサブバージョン
     */
    extern BOOL     lowspeed_mode;
    /*
     * CPU動作クロックモード
     */
    extern DWORD    main_speed_low;
    /*
     * メインCPUスピード(低速)
     */
    extern DWORD    sub_speed_low;
    /*
     * サブCPUスピード(低速)
     */
#ifdef JSUB
    extern DWORD    jsub_speed;
    /*
     * 日本語サブCPUスピード
     */
    extern WORD     jsub_overcycles;
    /*
     * 日本語サブCPUオーバーサイクル数
     */
#endif
#endif

    /*
     *      メモリ
     */
    extern BYTE    *mainram_a;
    /*
     * RAM (表RAM) $8000
     */
    extern BYTE    *mainram_b;
    /*
     * RAM (裏RAM) $7C80
     */
    extern BYTE    *basic_rom;
    /*
     * ROM (F-BASIC) $7C00
     */
    extern BYTE    *main_io;
    /*
     * メインCPU I/O $0100
     */
    extern BYTE    *vram_c;
    /*
     * VRAM(タイプC) $C000 ($30000)
     */
    extern BYTE    *subrom_c;
    /*
     * ROM (タイプC) $2800
     */
    extern BYTE    *sub_ram;
    /*
     * コンソールRAM $1680
     */
    extern BYTE    *sub_io;
    /*
     * サブCPU I/O $0100
     */
#if XM7_VER == 1
    extern BYTE    *basic_rom8;
    /*
     * ROM (F-BASIC1.0) $7C00
     */
    extern BYTE    *boot_bas;
    /*
     * BOOT (BASIC) $200
     */
    extern BYTE    *boot_dos;
    /*
     * BOOT (DOSモード) $200
     */
    extern BYTE    *boot_bas8;
    /*
     * BOOT (BASIC,FM-8) $200
     */
    extern BYTE    *boot_dos8;
    /*
     * BOOT (DOS,FM-8) $200
     */

    extern BOOL     available_fm7roms;
    /*
     * FM-7 ROM使用可能フラグ
     */
    extern BOOL     available_fm8roms;
    /*
     * FM-8 ROM使用可能フラグ
     */
#ifdef BUBBLE
    extern BOOL bubble_available;  /* バブル使用可能フラグ */
#endif
#endif
#if (XM7_VER == 1) || (XM7_VER >= 3)
    extern BYTE *boot_mmr;    /* BOOT (隠し)       $200 */
    extern BOOL available_mmrboot;
    /*
     * FM-77 MMRブートROM使用可能フラグ
     */
#endif
    extern BOOL     boot_mode;
    /*
     * 起動モード
     */
    extern BOOL     basicrom_en;
    /*
     * F-BASIC 3.0 ROM イネーブル
     */

    extern BYTE *boot_ram;
    /*
     * BOOT (RAM)        $200
     */
    extern BOOL bootram_rw;
    /*
     * ブートRAM 書き込み可能
     */

    /*
     *      メモリ (FM-77)
     */
#if XM7_VER == 1
    extern BYTE    *extram_a;
    /*
     * RAM (FM-77) $30000
     */
#ifdef L4CARD
    extern BYTE    *tvram_c;
    /*
     * テキストVRAM(L4) $1000
     */
    extern BYTE    *subrom_l4;
    /*
     * サブモニタ (L4) $4800
     */
    extern BYTE    *subcg_l4;
    /*
     * CGROM (L4) $1000
     */
    extern BOOL     enable_400linecard;
    /*
     * 400ライン有効フラグ
     */
    extern BOOL     detect_400linecard;
    /*
     * 400ラインカード発見フラグ
     */
extern BOOL detect_400linecard_tmp; /* 400ラインカード発見フラグ(tmp) */
#endif
#endif

    /*
     *      メモリ (FM77AV)
     */
#if XM7_VER >= 2
    extern BYTE    *extram_a;
    /*
     * RAM (FM77AV) $10000
     */
    extern BYTE    *init_rom;
    /*
     * イニシエートROM $2000
     */

    extern BYTE    *subrom_a;
    /*
     * ROM (タイプA) $2000
     */
    extern BYTE    *subrom_b;
    /*
     * ROM (タイプB) $2000
     */
    extern BYTE    *subromcg;
    /*
     * ROM (CG) $2000
     */
    extern BYTE    *vram_b;
    /*
     * VRAM (タイプA,B) $C000
     */

    extern BOOL     initrom_en;
    /*
     * イニシエータROM イネーブル
     */
    extern BYTE     subrom_bank;
    /*
     * サブシステムROM/RAMバンク
     */
    extern BYTE     cgrom_bank;
    /*
     * CGROMバンク
     */
#endif

    /*
     *      メモリ (FM77AV40・日本語カード部除く)
     */
#if XM7_VER >= 3
    extern BYTE    *extram_c;
    /*
     * RAM (FM77AV40) $40000
     */
    extern BYTE    *subramde;
    /*
     * RAM (TypeD/E) $2000
     */
    extern BYTE    *subramcg;
    /*
     * RAM (フォント) $4000
     */
    extern BYTE    *subramcn;
    /*
     * 裏コンソールRAM $2000
     */

    extern BYTE     cgram_bank;
    /*
     * CGRAMバンク
     */
    extern BYTE     consram_bank;
    /*
     * コンソールRAMバンク
     */
#endif

#ifdef __cplusplus
}
#endif
#endif				/* _xm7_h_ */

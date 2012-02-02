/*
* Memory definition for Debugger at XM7
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
*/
#ifndef MEMDEF_H_INCLUDED
#define MEMDEF_H_INCLUDED

#include "../vm/xm7.h"

#ifdef __cplusplus
extern "C" {
#endif


/* MAIN MAP from vm/mainmam.c */
/* Main-RAM 0x08000 : $30000-$37fff */
extern BYTE     *mainram_a;
/* URA-RAM 0x07c80 or 7e00(v1.0) : $38000-$8fc7f */
extern BYTE     *mainram_b;
/* BASIC ROM 0x07c00 */
extern BYTE     *basic_rom;
/* MAIN I/O 0x01000*/
extern BYTE     *main_io;
/* BASIC ROM Enable / Disable */
extern BOOL     basicrom_en;
// Only V1.0
#if XM7_VER == 1
/* ROM(V3.0) 0x07c00 */
extern BYTE     *basic_rom8;
/* BOOTROM (BASIC) 0x0200 */
extern BYTE     *boot_bas;
/* BOOT(DOS) 0x0200 */
extern BYTE     *boot_dos;
/* BOOT(BASIC,FM-8)  0x0200*/
extern BYTE     *boot_bas8;
/* BOOT(DOS,FM-8) 0x0200 */
extern BYTE     *boot_dos8;
/* BOOT(Hidden) 0x0200 */
extern BYTE     *boot_mmr;
#endif

/* BOOTRAM 0x0200 */
extern BYTE           *boot_ram;
/* Bootrom/ram WRITE FLAG */
extern BOOL            bootram_rw;
/* EXTRAM 0x010000 */
extern BYTE           *extram_a;	/* 拡張RAM $10000 */
#if XM7_VER >= 3
/* Extend RAM AV40 0x0c0000 */
extern BYTE           *extram_c;
/* Boot ROM(MMR) 0x200 */
extern BYTE           *boot_mmr;
#endif

#if XM7_VER >= 2
/* Initiator ROM Enabled (0x2000)*/
extern BYTE           *init_rom;
/* Initiator Rom Enabled */
extern BOOL            initrom_en;
#endif

/* Sub Memmap */
/* VRAM C (BANKED) 0x0c000 */
extern BYTE *vram_c;
/* SUBROM C 0x02800 */
extern BYTE *subrom_c;
/* CONSOLE RAM 0x01680 */
extern BYTE *sub_ram;
/* SUB IO 0x0100 */
extern BYTE *sub_io;

#if XM7_VER >= 3
/* SUBRAM (TypeD/E) 0x02000 */
extern BYTE *subramde;
/* FONTRAM 0x04000 */
extern BYTE *subramcg;
/* SUBRAM (URA) 0x02000 */
extern BYTE *subramcn;
#endif

#if XM7_VER >= 2
/* VRAM B (BANKED) 0x0c000 */
extern BYTE *vram_b;

/* SUBROM A 0x02000 */
extern BYTE *subrom_a;
/* SUBROM B  0x02000 */
extern BYTE *subrom_b;
/* FONT ROM 0x02000 */
extern BYTE *subromcg;

/* SUBROM BANK */
extern BYTE subrom_bank;
/* CGROM BANK */
extern BYTE cgrom_bank;
#if XM7_VER >= 3
/* CGRAM BANK */
extern BYTE  cgram_bank;
/* CONSOLE RAM BANK */
extern BYTE  consram_bank;
#endif	/* XM7_VER >= 3 */
#endif	/* XM7_VER >= 2 */

#if XM7_VER == 1
/* SUB ROM (FM-8) 0x02800 */
extern BYTE     *subrom_8;
#ifdef L4CARD
/* Text VRAM $1000 */
extern BYTE     *tvram_c;
/* ROM (400line) 0x04800 */
extern BYTE     *subrom_l4;
/* CG (400line)  0x01000 */
extern BYTE     *subcg_l4;
/*Enable 400line card. */
extern BOOL     enable_400linecard;
extern BOOL     detect_400linecard;
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif // MEMDEF_H_INCLUDED

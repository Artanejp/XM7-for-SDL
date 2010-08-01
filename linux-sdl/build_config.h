/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ ビルドコンフィグ ]
 */
/*
 * Toolkit
 */
#define USE_GTK
/*
 * Target
 */
#define _XWIN

/*
 *
 */
#ifndef UIFONT
#define UIFONT "/usr/share/fonts/truetype/ipafont-jisx0208/ipagui0208_for_legacy_compatibility.ttf"
#endif

#ifndef SHAREDIR
#define SHAREDIR "/usr/local/share/xm7/"
#endif

#ifndef UIDIR
#define UIDIR SHAREDIR
#endif

#ifndef RSSDIR
#define RSSDIR SHAREDIR
#endif

#ifndef FUNC_FONT
#define FUNC_FONT UIFONT
#endif

#ifndef STAT_FONT
#define STAT_FONT UIFONT
#endif

/*
 * エミュレーション設定
 */
#ifndef XM7_VER
#define XM7_VER 3
#endif

#define CPU_ASM
#define MOUSE
#define MR2
#define FDDSND


/*
 * デバッグ関連
 */
#undef MEMWATCH
#undef NODEBUG

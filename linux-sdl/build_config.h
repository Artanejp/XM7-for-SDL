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
//#define USE_GTK

/*
 * Target
 */
//#define _XWIN

/*
 *
 */
#ifndef FONTPATH
#define FONTPATH "/usr/local/share/xm7:/usr/share/fonts/X11/misc:/usr/share/fonts/X11/75dpi:/usr/share/fonts/truetype/ipafont:/usr/share/fonts/truetype/mona:/usr/share/fonts/truetype/misaki:.:./.xm7"
#endif

#ifndef FUNC_FONT
#define FUNC_FONT "ipagp.ttf"
#endif

#ifndef STAT_FONT
#define STAT_FONT "ipagui.ttf"
#endif

#ifndef CMT_FONT
#define CMT_FONT "ipagp.ttf"
#endif

#ifndef VFD_FONT
#define VFD_FONT "ipagp.ttf"
#endif

#ifndef UI_FONT
#define UI_FONT "ipagui.ttf"
#endif

#define FUNC_PT 15
#define VFD_PT  15
#define CMT_PT 15
#define STAT_PT 15
#define UI_PT  18

/*
 * GUIレイアウト
 */
#define OSD_HEIGHT 24
#define DRAW_OFSET 64

/*
 * Embedの場合にはサイズと表示レイアウトを変更する必要がある？
 */
#define LED_WIDTH 36
#define LED_HEIGHT 18

#define VFD_WIDTH 160
#define VFD_HEIGHT 18

#define CMT_WIDTH 80
#define CMT_HEIGHT 18

#define STAT_WIDTH 320
#define STAT_HEIGHT 18






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

#undef CPU_ASM
#define MOUSE
#define MR2
//#define FDDSND


/*
 * デバッグ関連
 */
//#define MEMWATCH
//#undef NODEBUG

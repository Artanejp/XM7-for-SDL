/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN
 * キーボード・ジョイスティック・マウス ] 
 */  
    
#ifdef _XWIN
    
#include<X11/Xlib.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<gdk/gdkkeysyms.h>
#include<memory.h>
#include <SDL/SDL.h>
#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "sdl_kbd.h"
    
    /*
     *  グローバル ワーク 
     */ 
    BYTE kbd_map[256];	/* キーボード マップ */
struct local_sdlkeymap kbd_table[256];	/* SDL用 キーマップ */
int            nJoyType[2];	/* ジョイスティックタイプ */
int            nJoyRapid[2][2];	/* 連射タイプ */
int            nJoyCode[2][7];	/* 生成コード */
BYTE nJoyRaw[2];
BOOL bKbdReal;			/* 疑似リアルタイムキースキャン 
				 */
BOOL bTenCursor;		/* テンキー変換 */
BOOL bArrow8Dir;		/* テンキー変換 8方向モード */

#ifdef MOUSE
    BYTE nMidBtnMode;		/* 中央ボタン状態取得モード */

#endif				/*  */
    
    /*
     *  スタティック ワーク 
     */ 
static DWORD    keytime;	/* キーボードポーリング時間 */
static DWORD   dmyreadtime;	/* キーコードダミーリードタイマ 
				 */
static BOOL    bDummyRead;	/* キーコードダミーリードフラグ 
				 */
static BYTE   kibuf[256];	/* キーボード押下状態 */
static BYTE    nKeyBuffer[KEYBUFFER_SIZE];	/* 内部キーバッファ 
						 */
static BYTE    nKeyReadPtr;	/* 内部キーバッファ読出ポインタ 
				 */
static BYTE    nKeyWritePtr;	/* 内部キーバッファ書込ポインタ 
				 */
static BYTE    nLastKey;	/* 最後に押されたキー(FULL) */
static BYTE    nLastKey2;	/* 最後に押されたキー(10KEY) */
static BYTE    nTenDir;	/* テンキー変換 方向データ */
static BYTE    nTenDir2;	/* テンキー変換 方向データ2 */
static BYTE   joydat[3];	/* ジョイスティックデータ */
static BYTE    joybk[2];	/* ジョイスティックバックアップ 
				 */
static int     joyrapid[2][2];	/* ジョイスティック連射カウンタ 
				 */
static DWORD   joytime;	/* ジョイスティックポーリング時間 
				 */
static DWORD   joytime2;	/* ジョイスティックポーリング時間 
				 */
static BOOL    joyplugged[2];	/* ジョイスティック接続フラグ 
				 */

#ifdef MOUSE
static DWORD    mostime;	/* マウスポーリング時間 */
static BOOL    bCapture;	/* マウスキャプチャフラグ(Win32) 
				 */
static int     nMouseX;	/* マウス X軸移動距離 */
static int     nMouseY;	/* マウス Y軸移動距離 */
static BYTE    nMouseButton;	/* マウス ボタン押下状態 */
static BYTE    nMouseButtons;	/* マウス ボタン数 */
static BYTE    nCenterButton;	/* マウス 中央ボタン押下状態 */
static BOOL    bMouseCursor;	/* マウス カーソル表示状態 */
static int     nMouseSX;	/* マウス X座標保存 */
static int     nMouseSY;	/* マウス Y座標保存 */
static int     nMousePX;	/* マウス X座標保存(一時) */
static int     nMousePY;	/* マウス Y座標保存(一時) */
static int     nDAreaW;	/* ドローイングエリアの幅 */
static int     nDAreaH;	/* ドローイングエリアの高さ */
static BOOL    rgbButtons[3];	/* マウスボタン押下状態 */
static GdkCursor *nullcursor;	/* 透明カーソル */

#endif				/*  */
    
    /*
     *  DirectInputコード→FM-7 物理コード
     * コード対照表(106キーボード用) 
     */ 
static struct local_sdlkeymap kbd_106_table2[] = { SDLK_ESCAPE, 0x5c,	/* BREAK(ESC) 
									 */ 
	SDLK_F1, 0x5d, /* PF1 */ 
	SDLK_F2, 0x5e, /* PF2 */ 
	SDLK_F3, 0x5f, /* PF3 */ 
	SDLK_F4, 0x60, /* PF4 */ 
	SDLK_F5, 0x61, /* PF5 */ 
	SDLK_F6, 0x62, /* PF6 */ 
	SDLK_F7, 0x63, /* PF7 */ 
	SDLK_F8, 0x64, /* PF8 */ 
	SDLK_F9, 0x65, /* PF9 */ 
	SDLK_F10, 0x66, /* PF10 */ 
	SDLK_FIRST, 0x01, /* ESC(半角/全角) */ 
	SDLK_1, 0x02, /* 1 */ 
	SDLK_2, 0x03, /* 2 */ 
	SDLK_3, 0x04, /* 3 */ 
	SDLK_4, 0x05, /* 4 */ 
	SDLK_5, 0x06, /* 5 */ 
	SDLK_6, 0x07, /* 6 */ 
	SDLK_7, 0x08, /* 7 */ 
	SDLK_8, 0x09, /* 8 */ 
	SDLK_9, 0x0a, /* 9 */ 
	SDLK_0, 0x0b, /* 0 */ 
	SDLK_MINUS, 0x0c, /* - */ 
	SDLK_CARET, 0x0d, /* ^ */ 
	SDLK_BACKSLASH, 0x0e, /* \ */ 
	SDLK_BACKSPACE, 0x0f, /* BS */ 
	SDLK_TAB, 0x10, /* TAB */ 
	SDLK_q, 0x11, /* Q */ 
	SDLK_w, 0x12, /* W */ 
	SDLK_e, 0x13, /* E */ 
	SDLK_r, 0x14, /* R */ 
	SDLK_t, 0x15, /* T */ 
	SDLK_y, 0x16, /* Y */ 
	SDLK_u, 0x17, /* U */ 
	SDLK_i, 0x18, /* I */ 
	SDLK_o, 0x19, /* O */ 
	SDLK_p, 0x1a, /* P */ 
	SDLK_AT, 0x1b, /* @ */ 
	SDLK_LEFTBRACKET, 0x1c, /* [ */ 
	SDLK_RETURN, 0x1d, /* CR */ 
	SDLK_LCTRL, 0x52, /* CTRL(左Ctrl) */ 
	SDLK_a, 0x1e, /* A */ 
	SDLK_s, 0x1f, /* S */ 
	SDLK_d, 0x20, /* D */ 
	SDLK_f, 0x21, /* F */ 
	SDLK_g, 0x22, /* G */ 
	SDLK_h, 0x23, /* H */ 
	SDLK_j, 0x24, /* J */ 
	SDLK_k, 0x25, /* K */ 
	SDLK_l, 0x26, /* L */ 
	SDLK_SEMICOLON, 0x27, /* ; */ 
	SDLK_COLON, 0x28, /* : */ 
	SDLK_RIGHTBRACKET, 0x29, /* ] */ 
	SDLK_LSHIFT, 0x53, /* 左SHIFT */ 
	SDLK_z, 0x2a, /* Z */ 
	SDLK_x, 0x2b, /* X */ 
	SDLK_c, 0x2c, /* C */ 
	SDLK_v, 0x2d, /* V */ 
	SDLK_b, 0x2e, /* B */ 
	SDLK_n, 0x2f, /* N */ 
	SDLK_m, 0x30, /* M */ 
	SDLK_COMMA, 0x31, /* , */ 
	SDLK_PERIOD, 0x32, /* . */ 
	SDLK_SLASH, 0x33, /* / */ 
	SDLK_UNDERSCORE, 0x34, /* _ */ 
	SDLK_RSHIFT, 0x54, /* 右SHIFT */ 
	SDLK_LALT, 0x55, /* CAP(左ALT) */ 
	// 0x83, 0x56, /* GRAPH(無変換) */
	SDLK_LSUPER, 0x56, /* WIN(無変換) for 109 */ 
	// 0x81, 0x57, /* 左SPACE(変換) */
	SDLK_RSUPER, 0x57, /* 左SPACE(変換) for 109 */ 
	SDLK_RALT, 0x58, /* 中SPACE(カタカナ) */ 
	// 101, 0x58, /* 中SPACE(カタカナ) */
	SDLK_SPACE, 0x35, /* 右SPACE(SPACE) */ 
	// 0x6d, 0x5a, /* かな(右Ctrl) */
	SDLK_RCTRL, 0x5a, /* かな(右Ctrl) for 109 */ 
	SDLK_INSERT, 0x48, /* INS(Insert) */ 
	SDLK_DELETE, 0x4b, /* DEL(Delete) */ 
	SDLK_UP, 0x4d, /* ↑ */ 
	SDLK_LEFT, 0x4f, /* ← */ 
	SDLK_DOWN, 0x50, /* ↓ */ 
	SDLK_RIGHT, 0x51, /* → */ 
	SDLK_HOME, 0x49, /* EL(Home) */ 
	SDLK_PAGEUP, 0x4a, /* CLS(Page Up) */ 
	SDLK_END, 0x4c, /* DUP(End) */ 
	SDLK_PAGEDOWN, 0x4e, /* HOME(Page Down) */ 
	SDLK_KP_MULTIPLY, 0x36, /* Tenkey * */ 
	SDLK_KP_DIVIDE, 0x37, /* Tenkey / */ 
	SDLK_KP_PLUS, 0x38, /* Tenkey + */ 
	SDLK_KP_MINUS, 0x39, /* Tenkey - */ 
	SDLK_KP7, 0x3a, /* Tenkey 7 */ 
	SDLK_KP8, 0x3b, /* Tenkey 8 */ 
	SDLK_KP9, 0x3c, /* Tenkey 9 */ 
	SDLK_KP4, 0x3e, /* Tenkey 4 */ 
	SDLK_KP5, 0x3f, /* Tenkey 5 */ 
	SDLK_KP6, 0x40, /* Tenkey 6 */ 
	SDLK_KP1, 0x42, /* Tenkey 1 */ 
	SDLK_KP2, 0x43, /* Tenkey 2 */ 
	SDLK_KP3, 0x44, /* Tenkey 3 */ 
	SDLK_KP0, 0x46, /* Tenkey 0 */ 
	SDLK_KP_PERIOD, 0x47, /* Tenkey . */ 
	SDLK_KP_ENTER, 0x45, /* Tenkey CR */ 
	0xffff, 0xff /* End */  
};

static BYTE   kbd_106_table[] = { 0x09, 0x5c, /* BREAK(ESC) */ 
	0x43, 0x5d, /* PF1 */ 
	0x44, 0x5e, /* PF2 */ 
	0x45, 0x5f, /* PF3 */ 
	0x46, 0x60, /* PF4 */ 
	0x47, 0x61, /* PF5 */ 
	0x48, 0x62, /* PF6 */ 
	0x49, 0x63, /* PF7 */ 
	0x4a, 0x64, /* PF8 */ 
	0x4b, 0x65, /* PF9 */ 
	0x4c, 0x66, /* PF10 */ 
	0x31, 0x01, /* ESC(半角/全角) */ 
	0x0a, 0x02, /* 1 */ 
	0x0b, 0x03, /* 2 */ 
	0x0c, 0x04, /* 3 */ 
	0x0d, 0x05, /* 4 */ 
	0x0e, 0x06, /* 5 */ 
	0x0f, 0x07, /* 6 */ 
	0x10, 0x08, /* 7 */ 
	0x11, 0x09, /* 8 */ 
	0x12, 0x0a, /* 9 */ 
	0x13, 0x0b, /* 0 */ 
	0x14, 0x0c, /* - */ 
	0x15, 0x0d, /* ^ */ 
	0x85, 0x0e, /* \ */ 
	0x16, 0x0f, /* BS */ 
	0x17, 0x10, /* TAB */ 
	0x18, 0x11, /* Q */ 
	0x19, 0x12, /* W */ 
	0x1a, 0x13, /* E */ 
	0x1b, 0x14, /* R */ 
	0x1c, 0x15, /* T */ 
	0x1d, 0x16, /* Y */ 
	0x1e, 0x17, /* U */ 
	0x1f, 0x18, /* I */ 
	0x20, 0x19, /* O */ 
	0x21, 0x1a, /* P */ 
	0x22, 0x1b, /* @ */ 
	0x23, 0x1c, /* [ */ 
	0x24, 0x1d, /* CR */ 
	0x25, 0x52, /* CTRL(左Ctrl) */ 
	0x26, 0x1e, /* A */ 
	0x27, 0x1f, /* S */ 
	0x28, 0x20, /* D */ 
	0x29, 0x21, /* F */ 
	0x2a, 0x22, /* G */ 
	0x2b, 0x23, /* H */ 
	0x2c, 0x24, /* J */ 
	0x2d, 0x25, /* K */ 
	0x2e, 0x26, /* L */ 
	0x2f, 0x27, /* ; */ 
	0x30, 0x28, /* : */ 
	0x33, 0x29, /* ] */ 
	0x32, 0x53, /* 左SHIFT */ 
	0x34, 0x2a, /* Z */ 
	0x35, 0x2b, /* X */ 
	0x36, 0x2c, /* C */ 
	0x37, 0x2d, /* V */ 
	0x38, 0x2e, /* B */ 
	0x39, 0x2f, /* N */ 
	0x3a, 0x30, /* M */ 
	0x3b, 0x31, /* , */ 
	0x3c, 0x32, /* . */ 
	0x3d, 0x33, /* / */ 
	0x7b, 0x34, /* _ */ 
	0x3e, 0x54, /* 右SHIFT */ 
	0x40, 0x55, /* CAP(左ALT) */ 
	// 0x83, 0x56, /* GRAPH(無変換) */
	102, 0x56, /* GRAPH(無変換) for 109 */ 
	// 0x81, 0x57, /* 左SPACE(変換) */
	100, 0x57, /* 左SPACE(変換) for 109 */ 
	0x78, 0x58, /* 中SPACE(カタカナ) */ 
	101, 0x58, /* 中SPACE(カタカナ) */ 
	0x41, 0x35, /* 右SPACE(SPACE) */ 
	// 0x6d, 0x5a, /* かな(右Ctrl) */
	105, 0x5a, /* かな(右Ctrl) for 109 */ 
	0x6a, 0x48, /* INS(Insert) */ 
	0x6b, 0x4b, /* DEL(Delete) */ 
	0x62, 0x4d, /* ↑ */ 
	0x64, 0x4f, /* ← */ 
	0x68, 0x50, /* ↓ */ 
	0x66, 0x51, /* → */ 
	0x61, 0x49, /* EL(Home) */ 
	0x63, 0x4a, /* CLS(Page Up) */ 
	0x67, 0x4c, /* DUP(End) */ 
	0x69, 0x4e, /* HOME(Page Down) */ 
	0x3f, 0x36, /* Tenkey * */ 
	0x70, 0x37, /* Tenkey / */ 
	0x56, 0x38, /* Tenkey + */ 
	0x52, 0x39, /* Tenkey - */ 
	0x4f, 0x3a, /* Tenkey 7 */ 
	0x50, 0x3b, /* Tenkey 8 */ 
	0x51, 0x3c, /* Tenkey 9 */ 
	0x53, 0x3e, /* Tenkey 4 */ 
	0x54, 0x3f, /* Tenkey 5 */ 
	0x55, 0x40, /* Tenkey 6 */ 
	0x57, 0x42, /* Tenkey 1 */ 
	0x58, 0x43, /* Tenkey 2 */ 
	0x59, 0x44, /* Tenkey 3 */ 
	0x5a, 0x46, /* Tenkey 0 */ 
	0x5b, 0x47, /* Tenkey . */ 
	0x6c, 0x45 /* Tenkey CR */  
};


    /*
     *  カーソルキー → テンキー変換
     * 物理コード対照表 
     */ 
static BYTE     TenDirTable[16] =
    { 0x00, 0x3b, 0x43, 0xff, 0x3e, 0x3a, 0x42, 0x3e, 0x40, 0x3c, 0x44,
    0x40, 0xff, 0x3b, 0x43, 0xff 
};


    /*
     *  デフォルトキーボードマップ取得 
     */ 
void           FASTCALL
GetDefMapKbd(local_sdlkeymap * pMap, int mode) 
{
    int            i;
    int            type;
    Uint16 num;
    Uint8 code;
    ASSERT(pMap);
    ASSERT((mode >= 0) && (mode <= 3));
    
	/*
	 * 初期化 
	 */ 
	memset(pMap, 0, sizeof(struct local_sdlkeymap) * 256);
    
	/*
	 * キーマップ設定 
	 */ 
	switch (mode) {
	
	    /*
	     * デフォルトキー 
	     */ 
    case 0:
	
	    // type = GetKeyboardType(1);
	    // if (type & 0xd00) {
	    // type = 0xd00;
	    // }
	    // else {
	    type = 0;
	
	    // }
	    break;
	
	    /*
	     * 106 
	     */ 
    case 1:
	type = 0;
	break;
	
	    /*
	     * PC-98 
	     */ 
	    // case 2:
	    // type = 0xd00;
	    // break;
	    /*
	     * 101 
	     */ 
	    // case 3:
	    // type = 1;
	    // break;
	    /*
	     * その他 
	     */ 
    default:
	ASSERT(FALSE);
	break;
    }
    
	/*
	 * PC98 
	 */ 
	// if (type & 0xd00) {
	// /* PC-98 */
	// for (i=0; i<sizeof(kbd_98_table)/2; i++) {
	// pMap[kbd_98_table[i * 2]] = kbd_98_table[i * 2 + 1];
	// }
	// return;
	// }
	if (type == 0) {
	
	    /*
	     * 106 
	     */ 
	    for (i = 0; i < 256; i++) {
	    num = kbd_106_table2[i].keysym;
	    if (num == 0xffff)
		break;
	    pMap[i].keysym = num;
	    pMap[i].code = kbd_106_table2[i].code;
	}
	pMap[i].keysym = 0xffff;
	pMap[i].code = 0xff;
    }
    
	// else {
	// /* 101 */
	// for (i=0; i<sizeof(kbd_101_table)/2; i++) {
	// pMap[kbd_101_table[i * 2]] = kbd_101_table[i * 2 + 1];
	// }
	// }
}


    /*
     *  キーマップ設定 
     */ 
void            FASTCALL
SetMapKbd(local_sdlkeymap * pMap) 
{
    ASSERT(pMap);
    int            i;
    
	/*
	 * キーテーブル設定 
	 */ 
	for (i = 0; i < 256; i++) {
	if (pMap[i].keysym == 0xffff)
	    break;
	kbd_table[i].keysym = pMap[i].keysym;
	kbd_table[i].code = pMap[i].code;
    }
    kbd_table[i].keysym = 0xffff;
    kbd_table[i].code = 0xff;
    
	/*
	 * NTキーボード 
	 */ 
	// if (bNTkbMode) {
	// kbd_table[DIK_KANJI] |= 0x80;
	// kbd_table[DIK_CAPITAL] |= 0x80;
	// kbd_table[DIK_KANA] |= 0x80;
	// }
}


    /*
     *  初期化 
     */ 
void            FASTCALL
InitKbd(void) 
{
    
#ifdef MOUSE
	/*
	 *  透明カーソルイメージ 
	 */ 
	GdkPixmap * source, *mask;
    GdkColor fg = {
    0, 0, 0, 0};
    GdkColor bg = {
    0, 0, 0, 0};
    gchar nullcursor_img[] = {
    0x00};
    gchar nullcursor_mask[] = {
    0x00};
    
#endif				/*  */
	/*
	 * SDL 
	 */ 
	
	/*
	 * ワークエリア初期化(キーボード) 
	 */ 
	memset(kibuf, 0, sizeof(kibuf));
    memset(kbd_map, 0, sizeof(kbd_map));
    keytime = 0;
    
	/*
	 * ワークエリア初期化(キーボード NT/PC-9801対策) 
	 */ 
	bDummyRead = TRUE;
    dmyreadtime = 0;
    
	/*
	 * ワークエリア初期化(内部キーバッファ/疑似リアルタイムキースキャン) 
	 */ 
	memset(nKeyBuffer, 0, sizeof(nKeyBuffer));
    nKeyReadPtr = 0;
    nKeyWritePtr = 0;
    nLastKey = 0;
    nLastKey2 = 0;
    bKbdReal = FALSE;
    GetDefMapKbd(kbd_table, 0);	/* デフォルトキーマップ読み込み 
					 */
    
	/*
	 * ワークエリア初期化(テンキー変換) 
	 */ 
	bArrow8Dir = TRUE;
    nTenDir = 0;
    nTenDir2 = 0;
    
	/*
	 * ワークエリア初期化(ジョイスティック) 
	 */ 
	joytime = 0;
    joytime2 = 0;
    memset(joydat, 0, sizeof(joydat));
    memset(joybk, 0, sizeof(joybk));
    memset(joyrapid, 0, sizeof(joyrapid));
    joyplugged[0] = TRUE;
    joyplugged[1] = FALSE;
    
	/*
	 * ワークエリア初期化(JS) 
	 */ 
	nJoyType[1] = 0;
    nJoyType[0] = 1;
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_JoystickOpen(0);
    memset(joydat, 0, sizeof(joydat));
    memset(joybk, 0, sizeof(joybk));
    memset(joyrapid, 0, sizeof(joyrapid));
    joyplugged[0] = TRUE;
    joyplugged[1] = TRUE;
    
#ifdef MOUSE
	/*
	 * ワークエリア初期化(マウス) 
	 */ 
	mostime = 0;
    bCapture = FALSE;
    nMouseX = 0;
    nMouseY = 0;
    nCenterButton = 0;
    bMouseCursor = TRUE;
    nMouseButton = 0xf0;
    nMidBtnMode = MOSCAP_NONE;
    nMouseButtons = 3;
    
	/*
	 * 透明カーソル生成 
	 */ 
	source = gdk_bitmap_create_from_data(NULL, nullcursor_img, 1, 1);
    mask = gdk_bitmap_create_from_data(NULL, nullcursor_mask, 1, 1);
    nullcursor = gdk_cursor_new_from_pixmap(source, mask, &fg, &bg, 0, 0);
    gdk_pixmap_unref(source);
    gdk_pixmap_unref(mask);
    
#endif				/*  */
	// printf("KBDInit\n");
	
	/*
	 * テンキーエミュレーション 
	 */ 
	bTenCursor = FALSE;
}


    /*
     *  クリーンアップ 
     */ 
void            FASTCALL
CleanKbd(void) 
{
} 
    /*
     *  セレクト 
     */ 
    BOOL FASTCALL SelectKbd(void) 
{
    return TRUE;
}


    /*
     *  キーボード 内部キーバッファ登録 
     */ 
static void     FASTCALL
PushKeyCode(BYTE code) 
{
    int            i;
    if (nKeyWritePtr < KEYBUFFER_SIZE) {
	nKeyBuffer[nKeyWritePtr++] = code;
    }
    
    else {
	for (i = 0; i < KEYBUFFER_SIZE; i++) {
	    nKeyBuffer[i] = nKeyBuffer[i + 1];
	}
	nKeyBuffer[KEYBUFFER_SIZE - 1] = code;
	if (nKeyReadPtr > 0) {
	    nKeyReadPtr--;
	}
    }
}


    /*
     *  キーボード テンキー変換  方向コード変換 
     */ 
static BYTE     FASTCALL
Cur2Ten_DirCode(BYTE code) 
{
    switch (code) {
    case 0x4d:		/* ↑ */
	return 0x01;
    case 0x4f:		/* ← */
	return 0x04;
    case 0x50:		/* ↓ */
	return 0x02;
    case 0x51:		/* → */
	return 0x08;
    }
    return 0;
}


    /*
     *  キーボード テンキー変換  マスクコード変換 
     */ 
static BYTE     FASTCALL
Cur2Ten_MaskCode(BYTE code) 
{
    switch (code) {
    case 0x4d:		/* ↑ */
    case 0x50:		/* ↓ */
	return 0xfc;
    case 0x4f:		/* ← */
    case 0x51:		/* → */
	return 0xf3;
    }
    return 0xff;
}


    /*
     *  キーボード カーソルキー→テンキー変換 Make 
     */ 
static BOOL     FASTCALL
Cur2Ten_Make(BYTE code) 
{
    BYTE nTenDirOld;
    BYTE dircode;
    
	/*
	 * 以前の方向コードを保存 
	 */ 
	nTenDirOld = nTenDir;
    
	/*
	 * キーコード変換 
	 */ 
	dircode = Cur2Ten_DirCode(code);
    if (dircode) {
	if (bArrow8Dir) {
	    
		/*
		 * 8方向 
		 */ 
		nTenDir2 =
		(BYTE) (nTenDir & (BYTE) ~ Cur2Ten_MaskCode(code));
	    nTenDir &= Cur2Ten_MaskCode(code);
	    nTenDir |= dircode;
	}
	
	else {
	    if (key_format == KEY_FORMAT_SCAN) {
		if (TenDirTable[dircode]) {
		    keyboard_make(TenDirTable[dircode]);
		}
		return TRUE;
	    }
	    
		/*
		 * 4方向 
		 */ 
		nTenDir2 =
		(BYTE) (nTenDir & (BYTE) Cur2Ten_MaskCode(code));
	    nTenDir = dircode;
	}
	
	    /*
	     * テーブル内容が0xffの場合、無効な組み合わせ 
	     */ 
	    if (TenDirTable[nTenDir] == 0xff) {
	    nTenDir = nTenDirOld;
	    return TRUE;
	}
	
	    /*
	     * キーコード発行 
	     */ 
	    if (nTenDir != nTenDirOld) {
	    if (TenDirTable[nTenDirOld]) {
		keyboard_break(TenDirTable[nTenDirOld]);
		PushKeyCode(TenDirTable[nTenDir]);
	    }
	    
	    else {
		keyboard_make(TenDirTable[nTenDir]);
	    }
	}
	return TRUE;
    }
    return FALSE;
}


    /*
     *  キーボード カーソルキー→テンキー変換 Break 
     */ 
static BOOL     FASTCALL
Cur2Ten_Break(BYTE code) 
{
    BYTE nTenDirOld;
    BYTE dircode;
    
	/*
	 * 以前の方向コードを保存 
	 */ 
	nTenDirOld = nTenDir;
    
	/*
	 * キーコード変換 
	 */ 
	dircode = Cur2Ten_DirCode(code);
    if (dircode) {
	if ((key_format == KEY_FORMAT_SCAN) && !bArrow8Dir) {
	    if (TenDirTable[dircode]) {
		keyboard_break(TenDirTable[dircode]);
	    }
	    return TRUE;
	}
	nTenDir2 &= (BYTE) ~ dircode;
	if (nTenDir2) {
	    nTenDir |= nTenDir2;
	    nTenDir2 = 0;
	}
	nTenDir &= (BYTE) ~ dircode;
	
	    /*
	     * テーブル内容が0xffの場合、無効な組み合わせ 
	     */ 
	    if (TenDirTable[nTenDir] == 0xff) {
	    nTenDir = nTenDirOld;
	    return TRUE;
	}
	
	    /*
	     * キーコード発行 
	     */ 
	    if (nTenDir != nTenDirOld) {
	    keyboard_break(TenDirTable[nTenDirOld]);
	    if (TenDirTable[nTenDir]) {
		PushKeyCode(TenDirTable[nTenDir]);
	    }
	    
	    else if (key_format != KEY_FORMAT_SCAN) {
		
		    /*
		     * 停止時は"5"を発行 
		     */ 
		    PushKeyCode(0x3f);
		PushKeyCode(0xbf);
	    }
	}
	return TRUE;
    }
    return FALSE;
}


    /*
     *  キーボード ポーリング 
     */ 
void            FASTCALL
PollKbd(void) 
{
    BYTE buf[256];
    int            i;
    BYTE fm7;
    BOOL bFlag;
    static BYTE    key_last = 0;
    
	/*
	 * アクティベートチェックはSDLの場合不要かも? 
	 */ 
	/*
	 * アクティベートチェック 
	 */ 
	// if (!bActivate) {
	// /* 非アクティブ時はダミーリードタイマ更新
	// */
	// bDummyRead = TRUE;
	// dmyreadtime = dwExecTotal;
	// return;
	// }
	
	/*
	 * ダミーリード時間チェック 
	 */ 
	if (bDummyRead && ((dwExecTotal - dmyreadtime) > 100000)) {
	bDummyRead = FALSE;
    }
    
	/*
	 * 時間チェック 
	 */ 
	if ((dwExecTotal - keytime) < 20000) {
	return;
    }
    keytime = dwExecTotal;
    
	/*
	 * キーバッファ内にコードがたまっている場合、先に処理 
	 */ 
	if (nKeyReadPtr < nKeyWritePtr) {
	fm7 = nKeyBuffer[nKeyReadPtr++];
	if (fm7 == 0xff) {
	    
		/*
		 * 0xff : キーリピートタイマ変更 
		 */ 
		keyboard_repeat();
	}
	
	else if (fm7 & 0x80) {
	    
		/*
		 * 0x80-0xfe : Breakコード 
		 */ 
		keyboard_break((BYTE) (fm7 & 0x7f));
	}
	
	else {
	    
		/*
		 * 0x00-0x7f : Makeコード 
		 */ 
		keyboard_make(fm7);
	}
	return;
    }
    
    else {
	nKeyReadPtr = 0;
	nKeyWritePtr = 0;
    }
    
	/*
	 * フラグoff 
	 */ 
	bFlag = FALSE;
    
	/*
	 * 今までの状態と比較して、順に変換する 
	 */ 
	for (i = 0; i < sizeof(kibuf); i++) {
	if (((kibuf[i] & 0x80) != (kbd_map[i] & 0x80)) && (!bFlag)) {
	    if (kibuf[i] & 0x80) {
		
		    /*
		     * キー押下 
		     */ 
		    fm7 = (BYTE) (i & 0x7f);
		if (fm7 > 0) {
		    if (TRUE) {
			if (!bDummyRead) {
			    if ((fm7 >= 0x4d) && (fm7 <= 0x51)
				  && bTenCursor) {
				
				    /*
				     * スキャンモード時は本来のコードも発行 
				     */ 
				    if ((key_format == KEY_FORMAT_SCAN)
					&& (fm7 != 0x4e)) {
				    PushKeyCode(fm7);
				}
				
				    /*
				     * スキャンコード変換 
				     */ 
				    if (Cur2Ten_Make(fm7)) {
				    fm7 = 0;
				}
			    }
			    if (fm7) {
				keyboard_make(fm7);
			    }
			    bDummyRead = FALSE;
			    
				/*
				 * 疑似リアルタイムキースキャン 
				 */ 
				if (bKbdReal
				    && (key_format != KEY_FORMAT_SCAN)) {
				if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
				    
					// PushKeyCode((BYTE)(fm7 |
					// 0x80));
					
					/*
					 * 以前押されていたキーがあれば再発行 
					 */ 
					if (nLastKey && key_repeat_flag) {
					PushKeyCode(nLastKey);
					
					    // PushKeyCode(0xff);
				    }
				    PushKeyCode(0xff);
				    
					/*
					 * 再発行用にキーコードを記憶 
					 */ 
					if (fm7 == 0x3f) {
					nLastKey2 = 0;
				    }
				    
				    else {
					nLastKey2 = fm7;
				    }
				}
				
				else {
				    
					/*
					 * 再発行用にキーコードを記憶 
					 */ 
					if (fm7 != 0x5c) {
					nLastKey = fm7;
				    }
				    
				    else {
					nLastKey = 0;
				    }
				}
			    }
			}
			if (fm7 != 0x5c) {
			    key_last = fm7;
			}
			bFlag = TRUE;
		    }
		}
	    }
	    
	    else {
		
		    /*
		     * キー離した 
		     */ 
		    fm7 = (BYTE) (i & 0x7f);
		if (fm7 > 0) {
		    if ((fm7 >= 0x4d) && (fm7 <= 0x51) && bTenCursor) {
			
			    /*
			     * スキャンモード時は本来のコードも発行 
			     */ 
			    if ((key_format == KEY_FORMAT_SCAN)
				&& (fm7 != 0x4e)) {
			    PushKeyCode((BYTE) (fm7 | 0x80));
			}
			
			    /*
			     * スキャンコード変換 
			     */ 
			    if (Cur2Ten_Break(fm7)) {
			    fm7 = 0;
			}
		    }
		    
			/*
			 * 疑似リアルタイムキースキャン 
			 */ 
			if (bKbdReal && (key_format != KEY_FORMAT_SCAN)) {
			if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
			    
				/*
				 * テンキーの場合 
				 */ 
				PushKeyCode((BYTE) (fm7 | 0x80));
			    if (nLastKey2 == fm7) {
				PushKeyCode(0x3f);
				PushKeyCode(0xbf);
				nLastKey2 = 0;
			    }
			    
				/*
				 * 以前テンキー以外が押されていた場合、再発行 
				 */ 
				if (nLastKey && key_repeat_flag) {
				PushKeyCode(nLastKey);
				PushKeyCode(0xff);
				key_last = nLastKey;
			    }
			}
			
			else {
			    
				/*
				 * テンキー以外の場合 
				 */ 
				keyboard_break(fm7);
			    if (nLastKey == fm7) {
				nLastKey = 0;
			    }
			    
				/*
				 * 以前テンキーが押されていた場合、再発行 
				 */ 
				if (nLastKey2 && (nLastKey2 != key_last)) {
				PushKeyCode(nLastKey2);
				PushKeyCode(0xff);
				
				    // PushKeyCode((BYTE)(nLastKey2 |
				    // 0x80));
				    key_last = nLastKey2;
			    }
			}
		    }
		    
		    else if (fm7) {
			keyboard_break(fm7);
		    }
		    bFlag = TRUE;
		}
	    }
	    
		/*
		 * データをコピー 
		 */ 
		kbd_map[i] = kibuf[i];
	}
    }
}


    /*
     *  キーボード ポーリング＆キー情報取得
     * ※VMのロックは行っていないので注意 
     */ 
    BOOL FASTCALL GetKbd(BYTE * pBuf) 
{
    int            i;
    ASSERT(pBuf);
    
	/*
	 * メモリクリア 
	 */ 
	memset(pBuf, 0, 256);
    return TRUE;
}


    /*
     *  ジョイスティック デバイスより読み込み 
     */ 
static BYTE     FASTCALL
GetJoy(int index, BOOL flag) 
{
    BYTE code = 0;
    if ((index > 1) || (index < 0))
	return 0;
    code = nJoyRaw[index];	/* いい加減だが… */
    
	// printf("JS: code = %04x\n",code);
	return code;
}


    /*
     *  JS関連イベントハンドラ 
     */ 
    BOOL OnMoveJoy(SDL_Event * event) 
{
    
	/*
	 * 感度設定とかリダイレクトとかあるけど取り合えず後;) 
	 */ 
	Uint8 num = event->jaxis.which;
    Uint8 axis = event->jaxis.axis;
    Sint16 value = event->jaxis.value;
    int            index;
    int            code = 0;
    
	// printf("JS:index = %d axis = %d num = %d value =
	// 0x%02x\n",index,axis,num,value); 
	/*
	 * JS番号は0,1のみ(暫定) 
	 */ 
	switch (num)
	 {
    case 0:
	index = 0;
	break;
    case 1:
	index = 1;
	break;
    default:
	return TRUE;
	break;
	}
    
	/*
	 * 軸は0,1のみ(暫定) 
	 */ 
	switch (axis)
	 {
    case 1:			/* Y軸? */
	if (value > 256) {
	    code = 0x02;	/* 下 */
	} else if (value < -256) {
	    code = 0x01;	/* 上 */
	}
	nJoyRaw[index] &= 0xfc;
	nJoyRaw[index] |= code;
	break;
    case 0:			/* X軸? */
	if (value > 256) {
	    code = 0x08;	/* 右 */
	} else if (value < -256) {
	    code = 0x04;	/* 左 */
	}
	nJoyRaw[index] &= 0xf3;
	nJoyRaw[index] |= code;
	break;
    default:
	break;
	}
    return TRUE;
}

BOOL OnPressJoy(SDL_Event * event) 
{
    
	/*
	 * 感度設定とかリダイレクトとかあるけど取り合えず後;) 
	 */ 
	Uint8 num = event->jbutton.which;
    Uint8 button = event->jbutton.button;
    int           index;
    int            code = 0;
    
	// printf("JS:index = %d num = %d value =
	// 0x%02x\n",index,num,button); 
	/*
	 * JS番号は0,1のみ(暫定) 
	 */ 
	switch (num)
	 {
    case 0:
	index = 0;
	break;
    case 1:
	index = 1;
	break;
    default:
	return TRUE;
	break;
	}
    
	/*
	 * ボタンは0,1のみ(暫定) 
	 */ 
	switch (button)
	 {
    case 1:			/* ボタン1? */
	code = 0x20;
	nJoyRaw[index] |= code;
	break;
    case 0:			/* ボタン0? */
	code = 0x10;
	nJoyRaw[index] |= code;
	break;
    default:
	break;
	}
    return TRUE;
}

BOOL OnReleaseJoy(SDL_Event * event) 
{
    
	/*
	 * 感度設定とかリダイレクトとかあるけど取り合えず後;) 
	 */ 
	Uint8 num = event->jbutton.which;
    Uint8 button = event->jbutton.button;
    int           index;
    int            code = 0;
    
	/*
	 * JS番号は0,1のみ(暫定) 
	 */ 
	switch (num)
	 {
    case 0:
	index = 0;
	break;
    case 1:
	index = 1;
	break;
    default:
	return TRUE;
	break;
	}
    
	/*
	 * ボタンは0,1のみ(暫定) 
	 */ 
	switch (button)
	 {
    case 1:			/* ボタン1? */
	code = 0x20;
	nJoyRaw[index] &= ~code;
	break;
    case 0:			/* ボタン0? */
	code = 0x10;
	nJoyRaw[index] &= ~code;
	break;
    default:
	break;
	}
    return TRUE;
}


    /*
     *  ジョイスティック
     * 連射カウンタテーブル(片側、20ms単位) 
     */ 
static const BYTE JoyRapidCounter[] = { 0, /* なし */ 
	25, /* 1ショット */ 
	12, /* 2ショット */ 
	8, /* 3ショット */ 
	6, /* 4ショット */ 
	5, /* 5ショット */ 
	4, /* 6ショット */ 
	3, /* 8ショット */ 
	2, /* 12ショット */ 
	1 /* 25ショット */  
};


    /*
     *  ジョイスティック
     * デバイスより読み込み(連射つき) 
     */ 
static BYTE     FASTCALL
GetRapidJoy(int index, BOOL flag) 
{
    int            i;
    BYTE bit;
    BYTE dat;
    
	/*
	 * assert 
	 */ 
	ASSERT((index == 0) || (index == 1));
    
	/*
	 * 非接続チェック1 (接続チェック時は通す) 
	 */ 
	if ((!flag) && (!joyplugged[index])) {
	return 0x00;
    }
    
	/*
	 * データ取得 
	 */ 
	dat = GetJoy(index, flag);
    
	/*
	 * 非接続チェック2 
	 */ 
	if (!joyplugged[index]) {
	return 0x00;
    }
    
	/*
	 * ボタンチェック 
	 */ 
	bit = 0x10;
    for (i = 0; i < 2; i++) {
	if ((dat & bit) && (nJoyRapid[index][i] > 0)) {
	    
		/*
		 * 連射ありで押されている。カウンタチェック 
		 */ 
		if (joyrapid[index][i] == 0) {
		
		    /*
		     * 初期カウンタを代入 
		     */ 
		    joyrapid[index][i] =
		    JoyRapidCounter[nJoyRapid[index][i]];
	    }
	    
	    else {
		
		    /*
		     * カウンタデクリメント 
		     */ 
		    joyrapid[index][i]--;
		if ((joyrapid[index][i] & 0xff) == 0) {
		    
			/*
			 * 反転タイミングなので、時間を加算して反転 
			 */ 
			joyrapid[index][i] +=
			JoyRapidCounter[nJoyRapid[index][i]];
		    joyrapid[index][i] ^= 0x100;
		}
	    }
	    
		/*
		 * ボタンが押されていないように振る舞う 
		 */ 
		if (joyrapid[index][i] >= 0x100) {
		dat &= (BYTE) (~bit);
	    }
	}
	
	else {
	    
		/*
		 * ボタンが押されてないので、連射カウンタクリア 
		 */ 
		joyrapid[index][i] = 0;
	}
	
	    /*
	     * 次のビットへ 
	     */ 
	    bit <<= 1;
    }
    return dat;
}


    /*
     *  ジョイスティック コード変換 
     */ 
static BYTE     FASTCALL
PollJoyCode(int code) 
{
    
	/*
	 * 0x70未満は無視 
	 */ 
	if (code < 0x70) {
	return 0;
    }
    
	/*
	 * 0x70から上下左右 
	 */ 
	switch (code) {
	
	    /*
	     * 上 
	     */ 
    case 0x70:
	return 0x01;
	
	    /*
	     * 下 
	     */ 
    case 0x71:
	return 0x02;
	
	    /*
	     * 左 
	     */ 
    case 0x72:
	return 0x04;
	
	    /*
	     * 右 
	     */ 
    case 0x73:
	return 0x08;
	
	    /*
	     * Aボタン 
	     */ 
    case 0x74:
	return 0x10;
	
	    /*
	     * Bボタン 
	     */ 
    case 0x75:
	return 0x20;
	
	    /*
	     * それ以外 
	     */ 
    default:
	ASSERT(FALSE);
	break;
    }
    return 0;
}


    /*
     *  ジョイスティック
     * ポーリング(ジョイスティック) 
     */ 
static BYTE     FASTCALL
PollJoySub(int index, BYTE dat) 
{
    int            i;
    BYTE ret;
    BYTE bit;
    
	/*
	 * assert 
	 */ 
	ASSERT((index == 0) || (index == 1));
    
	/*
	 * 終了データクリア 
	 */ 
	ret = 0;
    
	/*
	 * 方向 
	 */ 
	bit = 0x01;
    for (i = 0; i < 4; i++) {
	
	    /*
	     * ボタンが押されているか 
	     */ 
	    if (dat & bit) {
	    
		/*
		 * コード変換 
		 */ 
		ret |= PollJoyCode(nJoyCode[index][i]);
	}
	bit <<= 1;
    }
    
	/*
	 * センターチェック 
	 */ 
	if ((dat & 0x0f) == 0) {
	if ((joybk[index] & 0x0f) != 0) {
	    ret |= PollJoyCode(nJoyCode[index][4]);
	}
    }
    
	/*
	 * ボタン 
	 */ 
	if (dat & 0x10) {
	ret |= PollJoyCode(nJoyCode[index][5]);
    }
    if (dat & 0x20) {
	ret |= PollJoyCode(nJoyCode[index][6]);
    }
    
	// printf("Joy: %02x %02x\n", dat, ret);
	return ret;
}


    /*
     *  ジョイスティック ポーリング(キーボード) 
     */ 
static void     FASTCALL
PollJoyKbd(int index, BYTE dat) 
{
    BYTE bit;
    int            i;
    
	/*
	 * 上下左右 
	 */ 
	bit = 0x01;
    for (i = 0; i < 4; i++) {
	if (dat & bit) {
	    
		/*
		 * 初めて押されたら、make発行 
		 */ 
		if ((joybk[index] & bit) == 0) {
		if ((nJoyCode[index][i] > 0)
		     && (nJoyCode[index][i] <= 0x66)) {
		    keyboard_make((BYTE) nJoyCode[index][i]);
		}
	    }
	}
	
	else {
	    
		/*
		 * 初めて離されたら、break発行 
		 */ 
		if ((joybk[index] & bit) != 0) {
		if ((nJoyCode[index][i] > 0)
		     && (nJoyCode[index][i] <= 0x66)) {
		    keyboard_break((BYTE) nJoyCode[index][i]);
		}
	    }
	}
	bit <<= 1;
    }
    
	/*
	 * センターチェック 
	 */ 
	if ((dat & 0x0f) == 0) {
	if ((joybk[index] & 0x0f) != 0) {
	    
		/*
		 * make/breakを続けて出す 
		 */ 
		if ((nJoyCode[index][4] > 0)
		    && (nJoyCode[index][4] <= 0x66)) {
		keyboard_make((BYTE) nJoyCode[index][4]);
		keyboard_break((BYTE) nJoyCode[index][4]);
	    }
	}
    }
    
	/*
	 * ボタン 
	 */ 
	bit = 0x10;
    for (i = 0; i < 2; i++) {
	if (dat & bit) {
	    
		/*
		 * 初めて押さたら、make発行 
		 */ 
		if ((joybk[index] & bit) == 0) {
		if ((nJoyCode[index][i + 5] > 0)
		     && (nJoyCode[index][i + 5] <= 0x66)) {
		    keyboard_make((BYTE) nJoyCode[index][i + 5]);
		}
	    }
	}
	
	else {
	    
		/*
		 * 初めて離されたら、break発行 
		 */ 
		if ((joybk[index] & bit) != 0) {
		if ((nJoyCode[index][i + 5] > 0)
		     && (nJoyCode[index][i + 5] <= 0x66)) {
		    keyboard_break((BYTE) nJoyCode[index][i + 5]);
		}
	    }
	}
	bit <<= 1;
    }
}


    /*
     *  ジョイスティック ポーリング 
     */ 
void            FASTCALL
PollJoy(void) 
{
    BYTE dat;
    BOOL check;
    int            i;
    
	/*
	 * 間隔チェック(ポーリング用) 
	 */ 
	if ((dwExecTotal - joytime) < 10000) {
	return;
    }
    joytime = dwExecTotal;
    
	/*
	 * データをクリア 
	 */ 
	memset(joydat, 0, sizeof(joydat));
    
	/*
	 * 無効チェック 
	 */ 
	if ((nJoyType[0] == 0) && (nJoyType[1] == 0)) {
	return;
    }
    
	/*
	 * 間隔チェック(接続チェック用) 
	 */ 
	if ((dwExecTotal - joytime2) >= 500000) {
	joytime2 = dwExecTotal;
	check = TRUE;
    }
    
    else {
	check = FALSE;
    }
    
	/*
	 * デバイスループ 
	 */ 
	for (i = 0; i < 2; i++) {
	
	    /*
	     * データ取得(連射つき) 
	     */ 
	    dat = GetRapidJoy(i, check);
	
	    /*
	     * タイプ別 
	     */ 
	    switch (nJoyType[i]) {
	    
		/*
		 * ジョイスティックポート1 
		 */ 
	case 1:
	    joydat[0] = PollJoySub(i, dat);
	    break;
	    
		/*
		 * ジョイスティックポート2 
		 */ 
	case 2:
	    joydat[1] = PollJoySub(i, dat);
	    break;
	    
		/*
		 * キーボード 
		 */ 
	case 3:
	    PollJoyKbd(i, dat);
	    break;
	    
		/*
		 * 電波新聞社ジョイスティック 
		 */ 
	case 4:
	    joydat[2] = PollJoySub(i, dat);
	    break;
	}
	
	    /*
	     * データ更新 
	     */ 
	    joybk[i] = dat;
    }
}


    /*
     *  ジョイスティック データリクエスト 
     */ 
    BYTE FASTCALL joy_request(BYTE no) 
{
    
	/*
	 * ASSERT((no >= 0) && (no < 3)); 
	 */ 
	ASSERT(no < 3);
    return joydat[no];
}


#ifdef MOUSE
    /*
     *  マウス ポーリング 
     */ 
void            FASTCALL
PollMos(void) 
{
    int            x,
                    y,
                    lX,
                    lY;
    
	/*
	 * アクティベートチェック 
	 */ 
	if (!bActivate) {
	return;
    }
    
	/*
	 * 時間チェック 
	 */ 
	if ((dwExecTotal - mostime) < 10000) {
	return;
    }
    mostime = dwExecTotal;
    
	/*
	 * 移動距離・ボタン状態の設定 
	 */ 
	if (bCapture && mos_capture) {
	gdk_threads_enter();
	
	    // gtk_widget_get_pointer(drawArea, &x, &y);
	    lX = x - nMousePX;
	lY = y - nMousePY;
	nMousePX = x;
	nMousePY = y;
	if (x == 0 || y == 0 || x == nDAreaW - 1 || y == nDAreaH - 1) {
	    
		// XWarpPointer(
		// GDK_WINDOW_XDISPLAY(drawArea->window),
		// None,
		// GDK_WINDOW_XWINDOW(drawArea->window),
		// 0,
		// 0,
		// 0,
		// 0,
		// nDAreaW/2,
		// nDAreaH/2
		// );
		// nMousePX = nDAreaW/2;
		// nMousePY = nDAreaH/2;
	}
	
	    /*
	     * マウス移動距離を蓄積 
	     */ 
	    nMouseX -= lX;
	nMouseY -= lY;
	
	    /*
	     * ボタン状態を設定 
	     */ 
	    nMouseButton = 0xf0;
	if (rgbButtons[0]) {
	    
		/*
		 * 左ボタン押下 
		 */ 
		nMouseButton &= ~0x10;
	}
	if (rgbButtons[2]) {
	    
		/*
		 * 右ボタン押下 
		 */ 
		nMouseButton &= ~0x20;
	}
	gdk_threads_leave();
    }
    if (nMouseButtons >= 3) {
	
	    /*
	     * 中央ボタンが押されたらマウスモードを切り換える 
	     */ 
	    if (rgbButtons[1] && !nCenterButton) {
	    
		/*
		 * マウスキャプチャフラグを反転させてモード切り替え 
		 */ 
		mos_capture = (!mos_capture);
	    gdk_threads_enter();
	    SetMouseCapture(bActivate);
	    gdk_threads_leave();
	}
	
	    /*
	     * 現在の中央ボタンの状態を保存 
	     */ 
	    nCenterButton = rgbButtons[1];
    }
}


    /*
     *  マウス キャプチャ状態設定 
     */ 
void            FASTCALL
SetMouseCapture(BOOL en) 
{
    int            x,
                    y;
    GdkModifierType state;
    GdkCursor * cursor;
    
	/*
	 * キャプチャ停止中/VM停止中/非アクティブ時は強制的に無効 
	 */ 
	if (!mos_capture || stopreq_flag || !run_flag || !bActivate) {
	en = FALSE;
    }
    
	/*
	 * カーソル表示/消去 
	 */ 
	if (bMouseCursor == en) {
	if (en) {
	    cursor = nullcursor;
	} else {
	    cursor = NULL;
	}
	bMouseCursor = !en;
    }
    
	/*
	 * キャプチャ状態に変化がなければ帰る 
	 */ 
	if (bCapture == en) {
	return;
    }
    if (en) {
	
	    /*
	     * カーソル位置を保存 
	     */ 
	    // gtk_widget_get_pointer(drawArea, &x, &y);
	    nMouseSX = x;
	nMouseSY = y;
	
	    /*
	     * 描画ウィンドウのサイズを求める 
	     */ 
	    // gdk_window_get_size(drawArea->window, &nDAreaW, &nDAreaH);
	    
	    /*
	     * 中心座標を求める 
	     */ 
	    x = nDAreaW / 2;
	y = nDAreaH / 2;
	
	    /*
	     * カーソルをウィンドウ中央に固定 
	     */ 
	    // XWarpPointer(
	    // GDK_WINDOW_XDISPLAY(drawArea->window),
	    // None,
	    // GDK_WINDOW_XWINDOW(drawArea->window),
	    // 0,
	    // 0,
	    // 0,
	    // 0,
	    // x,
	    // y
	    // );
	    
	    // gdk_pointer_grab(
	    // wndMain->window,
	    // TRUE,
	    // GDK_POINTER_MOTION_MASK,
	    // drawArea->window,
	    // cursor,
	    // GDK_CURRENT_TIME
	    // );
	    nMousePX = x;
	nMousePY = y;
    }
    
    else {
	
	    /*
	     * クリップ解除 
	     */ 
	    gdk_pointer_ungrab(GDK_CURRENT_TIME);
	
	    /*
	     * カーソル位置を復元 
	     */ 
	    // XWarpPointer(
	    // / GDK_WINDOW_XDISPLAY(drawArea->window),
	    // None,
	    // GDK_WINDOW_XWINDOW(drawArea->window),
	    // 0,
	    // 0,
	    // 0,
	    // 0,
	    // nMouseSX,
	    // nMouseSY
	    // );
    }
    
	/*
	 * キャプチャ状態を保存 
	 */ 
	bCapture = en;
    
	/*
	 * マウス移動距離をクリア 
	 */ 
	nMouseX = 0;
    nMouseY = 0;
}


    /*
     *  マウス 移動データリクエスト 
     */ 
void            FASTCALL
mospos_request(BYTE * move_x, BYTE * move_y) 
{
    if (bCapture) {
	
	    /*
	     * 移動距離を符号付き８ビットの範囲に収める 
	     */ 
	    if (nMouseX > 127) {
	    nMouseX = 127;
	}
	
	else if (nMouseX < -127) {
	    nMouseX = -127;
	}
	if (nMouseY > 127) {
	    nMouseY = 127;
	}
	
	else if (nMouseY < -127) {
	    nMouseY = -127;
	}
	*move_x = (BYTE) nMouseX;
	*move_y = (BYTE) nMouseY;
    }
    
    else {
	
	    /*
	     * キャプチャ一時停止中は移動していないことにする 
	     */ 
	    *move_x = 0;
	*move_y = 0;
    }
    
	/*
	 * マウス移動距離をクリア 
	 */ 
	nMouseX = 0;
    nMouseY = 0;
}


    /*
     *  マウス ボタンデータリクエスト 
     */ 
    BYTE FASTCALL mosbtn_request(void) 
{
    
	/*
	 * ボタン情報を返す 
	 */ 
	return nMouseButton;
}


#endif	/* MOUSE */
    
/**[ アクションイベント ]***********************************************/ 
    
    /*
     *  キープレスアクション 
     */ 
    gboolean OnKeyPressGtk(GtkWidget * widget, GdkEventKey * event,
			    gpointer data) 
{
    int            i;
    for (i = 0; sizeof(kbd_106_table) / 2; i++) {
	if (kbd_106_table[i * 2] == event->hardware_keycode) {
	    if (kibuf[kbd_106_table[i * 2 + 1]] != 0x80) {
		kibuf[kbd_106_table[i * 2 + 1]] = 0x80;
	    }
	    break;
	}
    }
    return TRUE;
}

BOOL OnKeyPress(SDL_Event * event) 
{
    int            i = 0;
    SDLMod modifier = event->key.keysym.mod;
    SDLKey code = event->key.keysym.sym;
    Uint8 scan = event->key.keysym.scancode;
    for (i = 0; i < 255; i++) {
	if (kbd_table[i].keysym == 0xffff)
	    break;
	if (code == kbd_table[i].keysym) {
	    if (kibuf[kbd_table[i].code] != 0x80) {
		kibuf[kbd_table[i].code] = 0x80;
	    }
	    break;
	}
    }
    return TRUE;
}


    /*
     *  キーリリースアクション 
     */ 
    gboolean OnKeyReleaseGtk(GtkWidget * widget, GdkEventKey * event,
			      gpointer data) 
{
    int            i;
    for (i = 0; i < sizeof(kbd_106_table) / 2; i++) {
	if (kbd_106_table[i * 2] == event->hardware_keycode) {
	    kibuf[kbd_106_table[i * 2 + 1]] = 0x00;
	    break;
	}
    }
    
#ifdef MOUSE
	/*
	 * F11押下の場合はマウスキャプチャフラグを反転させてモード切り替え 
	 */ 
	if (event->hardware_keycode == 0x5f) {
	mos_capture = (!mos_capture);
	SetMouseCapture(bActivate);
    }
    
#endif				/*  */
	
	/*
	 * F12押下の場合はVMリセット 
	 */ 
	if (event->hardware_keycode == 0x60) {
	LockVM();
	system_reset();
	UnlockVM();
    }
    return TRUE;
}

BOOL OnKeyRelease(SDL_Event * event) 
{
    int            i;
    SDLMod modifier = event->key.keysym.mod;
    SDLKey code = event->key.keysym.sym;
    Uint8 scan = event->key.keysym.scancode;
    for (i = 0; i < 255; i++) {
	if (kbd_table[i].keysym == 0xffff)
	    break;
	if (code == kbd_table[i].keysym) {
	    kibuf[kbd_table[i].code] = 0x00;
	    break;
	}
    }
    
#ifdef MOUSE
	/*
	 * F11押下の場合はマウスキャプチャフラグを反転させてモード切り替え 
	 */ 
	if (code == SDLK_F11) {
	mos_capture = (!mos_capture);
	SetMouseCapture(bActivate);
    }
    
#endif				/*  */
	
	/*
	 * F12押下の場合はVMリセット 
	 */ 
	if (code == SDLK_F12) {
	LockVM();
	system_reset();
	UnlockVM();
    }
    return TRUE;
}


#ifdef MOUSE
    /*
     *  マウスボタンプレスアクション 
     */ 
    gboolean OnButtonPress(GtkWidget * widget, GdkEventButton * event,
			   gpointer data) 
{
    rgbButtons[event->button - 1] = TRUE;
    return FALSE;
}


    /*
     *  マウスボタンリリースアクション 
     */ 
    gboolean OnButtonRelease(GtkWidget * widget, GdkEventButton * event,
			     gpointer data) 
{
    rgbButtons[event->button - 1] = FALSE;
    return FALSE;
}


#endif				/*  */
    
#endif	/* _XWIN */

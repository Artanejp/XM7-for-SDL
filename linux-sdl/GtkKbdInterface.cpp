/*
 * GtkKbdInterface.cpp
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */

#include<X11/Xlib.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<gdk/gdkkeysyms.h>
#include<memory.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "sdl_kbd.h"

//#include "gtk_propkeyboard.h"
#include "GtkKbdInterface.h"



static BYTE   kbd_106_table[] = { 0x09, 0x5c, /* BREAK(ESC) */
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
//	0x85, 0x0e, /* \ */
	132, 0x0e, /* \ */
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
//	0x7b, 0x34, /* _ */
	97, 0x34, /* _ */
	0x3e, 0x54, /* 右SHIFT */
	0x40, 0x55, /* CAP(左ALT) */
	// 0x83, 0x56, /* GRAPH(無変換) */
	133, 0x56, /* GRAPH(LWIN) for 109 */
	// 0x81, 0x57, /* 左SPACE(変換) */
	100, 0x57, /* 左SPACE(変換) for 109 */
	0x78, 0x58, /* 中SPACE(カタカナ) */
	101, 0x58, /* 中SPACE(カタカナ) */
	0x41, 0x35, /* 右SPACE(SPACE) */
//	 0x6d, 0x5a, /* かな(右Ctrl) */
	105, 0x5a, /* かな(右Ctrl) for 109 */
//	0x6a, 0x48, /* INS(Insert) */
//	0x6b, 0x4b, /* DEL(Delete) */
//	0x62, 0x4d, /* ↑ */
//	0x64, 0x4f, /* ← */
///	0x68, 0x50, /* ↓ */
//	0x66, 0x51, /* → */
	118, 0x48, /* INS(Insert) */
	119, 0x4b, /* DEL(Delete) */
	111, 0x4d, /* ↑ */
	113, 0x4f, /* ← */
	116, 0x50, /* ↓ */
	114, 0x51, /* → */
	110, 0x49, /* EL(Home) */
	112, 0x4a, /* CLS(Page Up) */
	115, 0x4c, /* DUP(End) */
	117, 0x4e, /* HOME(Page Down) */

//	0x61, 0x49, /* EL(Home) */
//	0x63, 0x4a, /* CLS(Page Up) */
//	0x67, 0x4c, /* DUP(End) */
//	0x69, 0x4e, /* HOME(Page Down) */
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
	0xff, 0xff /* End Code 20100917 */
};
//static BYTE kbd_tbl_gtk[256 * 2];
struct KeyCode KeyCodeTable2[256];

GtkKbdInterface::GtkKbdInterface() {
	// TODO Auto-generated constructor stub
	InitKeyTable();
}

GtkKbdInterface::~GtkKbdInterface() {
	// TODO Auto-generated destructor stub
	CleanKbd();
}


void GtkKbdInterface::InitKeyTable(void)
{
	/*
	 * キーボードテーブルの転写
	 */
		memset(KeyCodeTable2,0x00,sizeof(KeyCodeTable2));
		for(i=0 ; i<256; i++) {
			if((kbd_106_table[i * 2] == 0xff) && (kbd_106_table[i * 2 + 1] == 0xff)) break;
			KeyCodeTable2[i].code = (Uint32)kbd_106_table[i * 2];
			KeyCodeTable2[i].mod = 0x0000;
			KeyCodeTable2[i].pushCode = (Uint8)kbd_106_table[i * 2 + 1];
		}
		if(i < 256) {
			KeyCodeTable2[i].code = 0xffff;
			KeyCodeTable2[i].mod = 0xffff;
			KeyCodeTable2[i].pushCode = 0xff;
		}
	/*
	 * マウスキャプチャの初期値はPF11
	 */
	MouseCapture.code = 0x5f;
	MouseCapture.mod = 0x00;
	/*
	 *    リセットキーの初期値はPF12
	 */
	ResetKey.code = 0x5f;
	ResetKey.mod = 0x00;
}

void OnPress(void *arg)
{
	int i;
	GdkEventKey *event = (GdkEventKey *)arg;
	guint16 scan = event->hardware_keycode;
	guint   mod = event->state;
	struct KeyCode *p;

    for (i = 0; i<255; i++) {
    	if(p[i].code == 0xffff) break;
    	if (p[i].code == (Uint32)scan) {
    		PushKeyData(p[i].pushCode, 0x00);
    		break;
		}
    }
    return TRUE;
}

/*
 *  キーリリースアクション
 */
void OnRelease(void *arg)
{
    int            i;
	GdkEventKey *event = (GdkEventKey *)arg;
	guint16 scan = event->hardware_keycode;
	guint   mod = event->state;

    for (i = 0; i < 256; i++) {
    	if(p[i].code == 0xffff) break;
    	if (p[i].code == (Uint32)scan) {
    		PushKeyData(p[i].pushCode, 0x80);
    		break;
		}
    }
	/*
	 * F11押下の場合はマウスキャプチャフラグを反転させてモード切り替え
	 */
	if ((scan == MouseCapture.sym) && (mod == MouseCapture.mod)) {
		GtkMouseInterface::ToggleMouseCapture();
    }

	/*
	 * F12押下の場合はVMリセット
	 */
	if ((scan == ResetKey.sym) && (mod == ResetKey.mod)) {
		LockVM();
		system_reset();
		UnlockVM();
    }


    return TRUE;
}



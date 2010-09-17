/*
 * KbdInterface.cpp
 *
 *  Created on: 2010/09/16
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *
 */

#include "xm7.h"
#include "KbdInterface.h"

#include "xm7.h"
#include <SDL/SDL.h>
#include "SDLKbdInterface.h"

/*
 * キーコードテーブル
 */
struct {
	SDLKey sym;
	Uint8 code;
} KeyCode2;

struct KeyCode KeyCodeTable2[256];

const struct KeyCode2 KeyTable106[] =
{
	       SDLK_ESCAPE, 0x5c,	/* BREAK(ESC)
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
		SDLK_LSUPER, 0x56, /* WIN(無変換) for 109 */
		SDLK_RSUPER, 0x57, /* 左SPACE(変換) for 109 */
		SDLK_RALT, 0x58, /* 中SPACE(カタカナ) */
		SDLK_SPACE, 0x35, /* 右SPACE(SPACE) */
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

/*
 * キーコードテーブルを初期値にする
 */
void InitKeyTable(void){
	int i;
	for(i = 0; i<256 ; i++)
	{
		KeyCodeTable2[i].code = KeyTable106[i].sym;
		KeyCodeTable2[i].mod = KMOD_NONE;
		KeyCodeTable2[i].pushCode = KeyTable106[i].code;
		if(KeyTable106[i].sym == 0xffff) return;
	}
	/*
	 * マウスキャプチャの初期値はPF11
	 */
	MouseCapture.code = SDLK_F11;
	MouseCapture.mod = KMOD_NONE;
	/*
	 *    リセットキーの初期値はPF12
	 */
	ResetKey.code = SDLK_F12;
	ResetKey.mod = KMOD_NONE;

}

/*
 * キーテーブルを読み込む
 */
void LoadKeyTable(void *pMap){
	int i;
	struct KeyCode *p = (struct KeyCode *)pMap;
	for(i = 0; i<256 ; i++)
	{
		KeyCodeTable2[i].code = p[i].sym;
		KeyCodeTable2[i].mod = p[i].mod;
		KeyCodeTable2[i].pushCode = p[i].code;
		if(p[i].sym == 0xffff) return;
	}

}

void InitLocalVar(void){
	kbd_snooped = FALSE;
}


void ResetKeyMap(void)
{
	InitKeyTable();
}

KbdInterface::KbdInterface() {
	// TODO Auto-generated constructor stub
	InitLocalVar();
	InitKeyTable();
	//LoadKeyTable();
}

KbdInterface::~KbdInterface() {
	// TODO Auto-generated destructor stub
}

void CleanKbd(void)
{
	~KbdInterFace();
}


void LoadKeyMap(void *pMap){
	int i;
	struct KeyCode *p = (struct KeyCode *)pMap;
	for(i = 0; i<256 ; i++)
	{
		KeyCodeTable2[i].code = p[i].sym;
		KeyCodeTable2[i].mod = p[i].mod;
		KeyCodeTable2[i].pushCode = p[i].code;
		if(p[i].sym == 0xffff) return;
	}

}

void InitLocalVar(void){
//	kbd_snooped = FALSE;
}

void ResetKeyMap(void)
{
	InitKeyTable();
}


Uint32 GetKeyCode(Uint8 num)
{
	struct KeyCode *q = KeyCodeTable2;
	int i;

	for(i = 0; i<256; i++){
		if(q[i].code == 0xffff) break;
		if(q[i].pushCode == num) return q[i].code;
	}
	return 0x0000;
}

Uint32 GetKeyMod(Uint8 num)
{
	struct KeyCode *q = KeyCodeTable2;
	int i;

	for(i = 0; i<256; i++){
		if(q[i].code == 0xffff) break;
		if(q[i].pushCode == num) return q[i].mod;
	}
	return 0x0000;
}


struct KeyCode *GetKeyMap(Uint8 pushCode)
{
	// キーコードの変換
	struct KeyCode *q = KeyCodeTable2;
	int i;

	for(i = 0; i<256; i++){
		if(q[i].code == 0xffff) break;
		if(q[i].pushCode == pushCode) return &q[i];
	}
	return NULL;
}

struct KeyCode *GetKeyMap(void)
{
	return KeyCodeTable2;
}

void SetKeymap(Uint32 keyCode, Uint32 nativeCode, Uint32 keyMod)
{
	struct KeyCode *q = KeyCodeTable2;
	Uint32 code = keyCode;
	Uint32 mod = keyMod;

	for(i = 0;i < 256; i++){
		/*
		 * 新規設定
		 */
		if(q[i].code == 0xffff) {// 終わりだよね
			if(i>= 255) break; // キーテーブル一杯だからなにもしない
			q[i].code = code;
			q[i].mod = mod;
			/*
			 * 終端コード新規
			 */
			q[i].pushCode = (Uint8)(nativeCode & 0x000000ff);
			q[i+1].code = (Uint32)0xffff;
			q[i+1].mod = (Uint32)0xffff;
			q[i].pushCode = 0xff;
			return;
		}
		/*
		 * 置換
		 */
		if(q[i].pushCode == (Uint8)(nativeCode & 0x000000ff)){
			q[i].code = code;
			q[i].mod = mod;
			return;
		}
	}
}

void SetKeymap(struct KeyCode *p)
{
	struct KeyCode *q = KeyCodeTable2;

	for(i = 0;i < 256; i++){
		/*
		 * 新規設定
		 */
		if(q[i].code == 0xffff) {// 終わりだよね
			if(i>= 255) break; // キーテーブル一杯だからなにもしない
			q[i].code = p->code;
			q[i].mod = p->mod;
			/*
			 * 終端コード新規
			 */
			q[i].pushCode = (Uint8)(nativeCode & 0x000000ff);
			q[i+1].code = (Uint32)0xffff;
			q[i+1].mod = (Uint32)0xffff;
			q[i].pushCode = 0xff;
			return;
		}
		/*
		 * 置換
		 */
		if(q[i].pushCode == p->pushCode){
			q[i].code = p->code;
			q[i].mod = p->mod;
			return;
		}
	}

}

void SetResetKey(Uint32 code, Uint32 mod)
{
	ResetKey.code = code;
	ResetKey.mod = mod;

}

void SetMouseKey(Uint32 code, Uint32 mod)
{
	MouseCapture.code = code;
	MouseCapture.mod = mod;
}



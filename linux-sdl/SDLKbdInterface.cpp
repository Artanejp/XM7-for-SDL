/*
 * SDLKbdInterface.cpp
 *
 *  Created on: 2010/09/16
 *      Author: whatisthis
 */

#include "xm7.h"
#include "sdl.h"
#include <SDL/SDL.h>
#include "SDLKbdInterface.h"

/*
 * キーコードテーブル
 */


//struct SpecialKey MouseCapture;
//struct SpecialKey ResetKey;
//struct XM7KeyCode KeyCodeTable2[256];
struct KeyCode2 KeyTableSDL[] = {
		{SDLK_ESCAPE, 0x5c},	/* BREAK(ESC) */
		{SDLK_F1, 0x5d}, /* PF1 */
		{SDLK_F2, 0x5e}, /* PF2 */
		{SDLK_F3, 0x5f}, /* PF3 */
		{SDLK_F4, 0x60}, /* PF4 */
		{SDLK_F5, 0x61}, /* PF5 */
		{SDLK_F6, 0x62}, /* PF6 */
		{SDLK_F7, 0x63}, /* PF7 */
		{SDLK_F8, 0x64}, /* PF8 */
		{SDLK_F9, 0x65}, /* PF9 */
		{SDLK_F10, 0x66}, /* PF10 */
		{SDLK_FIRST, 0x01}, /* ESC(半角/全角) */
		{SDLK_1, 0x02}, /* 1 */
		{SDLK_2, 0x03}, /* 2 */
		{SDLK_3, 0x04}, /* 3 */
		{SDLK_4, 0x05}, /* 4 */
		{SDLK_5, 0x06}, /* 5 */
		{SDLK_6, 0x07}, /* 6 */
		{SDLK_7, 0x08}, /* 7 */
		{SDLK_8, 0x09}, /* 8 */
		{SDLK_9, 0x0a}, /* 9 */
		{SDLK_0, 0x0b}, /* 0 */
		{SDLK_MINUS, 0x0c}, /* - */
		{SDLK_CARET, 0x0d}, /* ^ */
		{SDLK_BACKSLASH, 0x0e}, /* \ */
		{SDLK_BACKSPACE, 0x0f}, /* BS */
		{SDLK_TAB, 0x10}, /* TAB */
		{SDLK_q, 0x11}, /* Q */
		{SDLK_w, 0x12}, /* W */
		{SDLK_e, 0x13}, /* E */
		{SDLK_r, 0x14}, /* R */
		{SDLK_t, 0x15}, /* T */
		{SDLK_y, 0x16}, /* Y */
		{SDLK_u, 0x17}, /* U */
		{SDLK_i, 0x18}, /* I */
		{SDLK_o, 0x19}, /* O */
		{SDLK_p, 0x1a}, /* P */
		{SDLK_AT, 0x1b}, /* @ */
		{SDLK_LEFTBRACKET, 0x1c}, /* [ */
		{SDLK_RETURN, 0x1d}, /* CR */
		{SDLK_LCTRL, 0x52}, /* CTRL(左Ctrl) */
		{SDLK_a, 0x1e}, /* A */
		{SDLK_s, 0x1f}, /* S */
		{SDLK_d, 0x20}, /* D */
		{SDLK_f, 0x21}, /* F */
		{SDLK_g, 0x22}, /* G */
		{SDLK_h, 0x23}, /* H */
		{SDLK_j, 0x24}, /* J */
		{SDLK_k, 0x25}, /* K */
		{SDLK_l, 0x26}, /* L */
		{SDLK_SEMICOLON, 0x27}, /* ; */
		{SDLK_COLON, 0x28}, /* : */
		{SDLK_RIGHTBRACKET, 0x29}, /* ] */
		{SDLK_LSHIFT, 0x53}, /* 左SHIFT */
		{SDLK_z, 0x2a}, /* Z */
		{SDLK_x, 0x2b}, /* X */
		{SDLK_c, 0x2c}, /* C */
		{SDLK_v, 0x2d}, /* V */
		{SDLK_b, 0x2e}, /* B */
		{SDLK_n, 0x2f}, /* N */
		{SDLK_m, 0x30}, /* M */
		{SDLK_COMMA, 0x31}, /* , */
		{SDLK_PERIOD, 0x32}, /* . */
		{SDLK_SLASH, 0x33}, /* / */
		{SDLK_UNDERSCORE, 0x34}, /* _ */
		{SDLK_RSHIFT, 0x54}, /* 右SHIFT */
		{SDLK_LALT, 0x55}, /* CAP(左ALT) */
		{SDLK_LSUPER, 0x56}, /* WIN(無変換) for 109 */
		{SDLK_RSUPER, 0x57}, /* 左SPACE(変換) for 109 */
		{SDLK_RALT, 0x58}, /* 中SPACE(カタカナ) */
		{SDLK_SPACE, 0x35}, /* 右SPACE(SPACE) */
	    {SDLK_RCTRL, 0x5a}, /* かな(右Ctrl) for 109 */
		{SDLK_INSERT, 0x48}, /* INS(Insert) */
		{SDLK_DELETE, 0x4b}, /* DEL(Delete) */
		{SDLK_UP, 0x4d}, /* ↑ */
		{SDLK_LEFT, 0x4f}, /* ← */
		{SDLK_DOWN, 0x50}, /* ↓ */
		{SDLK_RIGHT, 0x51}, /* → */
		{SDLK_HOME, 0x49}, /* EL(Home) */
		{SDLK_PAGEUP, 0x4a}, /* CLS(Page Up) */
		{SDLK_END, 0x4c}, /* DUP(End) */
		{SDLK_PAGEDOWN, 0x4e}, /* HOME(Page Down) */
		{SDLK_KP_MULTIPLY, 0x36}, /* Tenkey * */
		{SDLK_KP_DIVIDE, 0x37}, /* Tenkey / */
		{SDLK_KP_PLUS, 0x38}, /* Tenkey + */
		{SDLK_KP_MINUS, 0x39}, /* Tenkey - */
		{SDLK_KP7, 0x3a}, /* Tenkey 7 */
		{SDLK_KP8, 0x3b}, /* Tenkey 8 */
		{SDLK_KP9, 0x3c}, /* Tenkey 9 */
		{SDLK_KP4, 0x3e}, /* Tenkey 4 */
		{SDLK_KP5, 0x3f}, /* Tenkey 5 */
		{SDLK_KP6, 0x40}, /* Tenkey 6 */
		{SDLK_KP1, 0x42}, /* Tenkey 1 */
		{SDLK_KP2, 0x43}, /* Tenkey 2 */
		{SDLK_KP3, 0x44}, /* Tenkey 3 */
		{SDLK_KP0, 0x46}, /* Tenkey 0 */
		{SDLK_KP_PERIOD, 0x47}, /* Tenkey . */
		{SDLK_KP_ENTER, 0x45}, /* Tenkey CR */
		{(SDLKey)0xffff, 0xff} /* End */
};

/*
 * キーコードテーブルを初期値にする
 */
void SDLKbdInterface::InitKeyTable(void){
	int i;

	memset(KeyCodeTable2, 0, sizeof(KeyCodeTable2));

	for(i = 0; i<256 ; i++)
	{
		KeyCodeTable2[i].code = KeyTableSDL[i].sym;
		KeyCodeTable2[i].mod = KMOD_NONE;
		KeyCodeTable2[i].pushCode = KeyTableSDL[i].code;
		if(KeyTableSDL[i].sym == 0xffff) return;
	}
	/*
	 * マウスキャプチャの初期値はPF11
	 */
	MouseCapture.sym = SDLK_F11;
	MouseCapture.mod = KMOD_NONE;
	/*
	 *    リセットキーの初期値はPF12
	 */
	ResetKey.sym = SDLK_F12;
	ResetKey.mod = KMOD_NONE;

}

/*
 * キーテーブルを読み込む
 */
void SDLKbdInterface::LoadKeyTable(void *pMap){
	int i;
	struct XM7KeyCode *p = (struct XM7KeyCode *)pMap;
	for(i = 0; i<256 ; i++)
	{
		KeyCodeTable2[i].code = p[i].code;
		KeyCodeTable2[i].mod = p[i].mod;
		KeyCodeTable2[i].pushCode = p[i].pushCode;
		if(p[i].code == 0xffff) return;
	}

}

void SDLKbdInterface::InitLocalVar(void){
	kbd_snooped = FALSE;
}


void SDLKbdInterface::ResetKeyMap(void)
{
	InitKeyTable();
}

SDLKbdInterface::SDLKbdInterface() {
	// TODO Auto-generated constructor stub
	InitLocalVar();
	InitKeyTable();
	//LoadKeyTable();
}

SDLKbdInterface::~SDLKbdInterface() {
	// TODO Auto-generated destructor stub
}


void SDLKbdInterface::OnPress(void *eventh)
{
    int            i = 0;
    SDL_Event *event = (SDL_Event *)eventh;
    SDLMod modifier = event->key.keysym.mod;
    SDLKey code = event->key.keysym.sym;
    Uint8 scan = event->key.keysym.scancode;
    struct XM7KeyCode  *p = KeyCodeTable2;

    if(kbd_snooped) {
            //return SnoopedOnKeyPressedCallback(event);
    }
    //printf("Key SDL:%04x\n",code);
    for (i = 0; i < 255; i++) {
	if (p[i].code == 0xffff)   break;
	if ((code == p[i].code) && (modifier == p[i].mod)){
			PushKeyData(p[i].pushCode, 0x80); /* Make */
			break;
		}
    }
    return;
}

void SDLKbdInterface::OnRelease(void *eventh)
{
    int            i;
    SDL_Event *event = (SDL_Event *)eventh;

    Uint32 modifier = (Uint32)event->key.keysym.mod;
    Uint32 code = (Uint32)event->key.keysym.sym;
    Uint8 scan = event->key.keysym.scancode;
    struct XM7KeyCode *p = KeyCodeTable2;

    for (i = 0; i < 255; i++) {
    	if (p[i].code == 0xffff)   break;
    	if ((code == p[i].code) && (modifier == p[i].mod)){
    			PushKeyData(p[i].pushCode, 0x00); /* Break */
    			break;
    		}
    }

	/*
	 * F11押下の場合はマウスキャプチャフラグを反転させてモード切り替え
	 */
	if ((scan == MouseCapture.sym) && (modifier == MouseCapture.mod)) {
		//SDLMouseInterface::ToggleMouseCapture();
    }

	/*
	 * F12押下の場合はVMリセット
	 */
	if ((scan == ResetKey.sym) && (modifier == ResetKey.mod)) {
		LockVM();
		system_reset();
		UnlockVM();
    }
}



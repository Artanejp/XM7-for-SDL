/*
 * AgarKbdInterface.cpp
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */

#include "AgarKbdInterface.h"

AgarKbdInterface::AgarKbdInterface() {
	// TODO Auto-generated constructor stub
	InitLocalVar();
	InitKeyTable();
// LoadKeyTable:
}

AgarKbdInterface::~AgarKbdInterface() {
	// TODO Auto-generated destructor stub
}


/*
 * キーコードテーブル
 */
const struct KeyCode2_AG KeyTableAG[] = {
		{AG_KEY_ESCAPE, 0x5c},	/* BREAK(ESC) */
		{AG_KEY_F1, 0x5d}, /* PF1 */
		{AG_KEY_F2, 0x5e}, /* PF2 */
		{AG_KEY_F3, 0x5f}, /* PF3 */
		{AG_KEY_F4, 0x60}, /* PF4 */
		{AG_KEY_F5, 0x61}, /* PF5 */
		{AG_KEY_F6, 0x62}, /* PF6 */
		{AG_KEY_F7, 0x63}, /* PF7 */
		{AG_KEY_F8, 0x64}, /* PF8 */
		{AG_KEY_F9, 0x65}, /* PF9 */
		{AG_KEY_F10, 0x66}, /* PF10 */
//		{AG_KEY_FIRST, 0x01}, /* ESC(半角/全角) */
		{AG_KEY_SCROLLOCK, 0x01}, /* ESC(ScrLk):半角/全角がIMに取られるため */
		{AG_KEY_1, 0x02}, /* 1 */
		{AG_KEY_2, 0x03}, /* 2 */
		{AG_KEY_3, 0x04}, /* 3 */
		{AG_KEY_4, 0x05}, /* 4 */
		{AG_KEY_5, 0x06}, /* 5 */
		{AG_KEY_6, 0x07}, /* 6 */
		{AG_KEY_7, 0x08}, /* 7 */
		{AG_KEY_8, 0x09}, /* 8 */
		{AG_KEY_9, 0x0a}, /* 9 */
		{AG_KEY_0, 0x0b}, /* 0 */
		{AG_KEY_MINUS, 0x0c}, /* - */
		{AG_KEY_CARET, 0x0d}, /* ^ */
		{AG_KEY_BACKSLASH, 0x0e}, /* \ */
		{AG_KEY_BACKSPACE, 0x0f}, /* BS */
		{AG_KEY_TAB, 0x10}, /* TAB */
		{AG_KEY_Q, 0x11}, /* Q */
		{AG_KEY_W, 0x12}, /* W */
		{AG_KEY_E, 0x13}, /* E */
		{AG_KEY_R, 0x14}, /* R */
		{AG_KEY_T, 0x15}, /* T */
		{AG_KEY_Y, 0x16}, /* Y */
		{AG_KEY_U, 0x17}, /* U */
		{AG_KEY_I, 0x18}, /* I */
		{AG_KEY_O, 0x19}, /* O */
		{AG_KEY_P, 0x1a}, /* P */
		{AG_KEY_AT, 0x1b}, /* @ */
		{AG_KEY_LEFTBRACKET, 0x1c}, /* [ */
		{AG_KEY_RETURN, 0x1d}, /* CR */
		{AG_KEY_LCTRL, 0x52}, /* CTRL(左Ctrl) */
		{AG_KEY_A, 0x1e}, /* A */
		{AG_KEY_S, 0x1f}, /* S */
		{AG_KEY_D, 0x20}, /* D */
		{AG_KEY_F, 0x21}, /* F */
		{AG_KEY_G, 0x22}, /* G */
		{AG_KEY_H, 0x23}, /* H */
		{AG_KEY_J, 0x24}, /* J */
		{AG_KEY_K, 0x25}, /* K */
		{AG_KEY_L, 0x26}, /* L */
		{AG_KEY_SEMICOLON, 0x27}, /* ; */
		{AG_KEY_COLON, 0x28}, /* : */
		{AG_KEY_RIGHTBRACKET, 0x29}, /* ] */
		{AG_KEY_LSHIFT, 0x53}, /* 左SHIFT */
		{AG_KEY_Z, 0x2a}, /* Z */
		{AG_KEY_X, 0x2b}, /* X */
		{AG_KEY_C, 0x2c}, /* C */
		{AG_KEY_V, 0x2d}, /* V */
		{AG_KEY_B, 0x2e}, /* B */
		{AG_KEY_N, 0x2f}, /* N */
		{AG_KEY_M, 0x30}, /* M */
		{AG_KEY_COMMA, 0x31}, /* , */
		{AG_KEY_PERIOD, 0x32}, /* . */
		{AG_KEY_SLASH, 0x33}, /* / */
		{AG_KEY_UNDERSCORE, 0x34}, /* _ */
		{AG_KEY_RSHIFT, 0x54}, /* 右SHIFT */
		{AG_KEY_LALT, 0x55}, /* CAP(左ALT) */
		{AG_KEY_LSUPER, 0x56}, /* WIN(無変換) for 109 */
		{AG_KEY_RSUPER, 0x57}, /* 左SPACE(変換) for 109 */
		{AG_KEY_RALT, 0x58}, /* 中SPACE(カタカナ) */
		{AG_KEY_SPACE, 0x35}, /* 右SPACE(SPACE) */
	    {AG_KEY_RCTRL, 0x5a}, /* かな(右Ctrl) for 109 */
		{AG_KEY_INSERT, 0x48}, /* INS(Insert) */
		{AG_KEY_DELETE, 0x4b}, /* DEL(Delete) */
		{AG_KEY_UP, 0x4d}, /* ↑ */
		{AG_KEY_LEFT, 0x4f}, /* ← */
		{AG_KEY_DOWN, 0x50}, /* ↓ */
		{AG_KEY_RIGHT, 0x51}, /* → */
		{AG_KEY_HOME, 0x49}, /* EL(Home) */
		{AG_KEY_PAGEUP, 0x4a}, /* CLS(Page Up) */
		{AG_KEY_END, 0x4c}, /* DUP(End) */
		{AG_KEY_PAGEDOWN, 0x4e}, /* HOME(Page Down) */
		{AG_KEY_KP_MULTIPLY, 0x36}, /* Tenkey * */
		{AG_KEY_KP_DIVIDE, 0x37}, /* Tenkey / */
		{AG_KEY_KP_PLUS, 0x38}, /* Tenkey + */
		{AG_KEY_KP_MINUS, 0x39}, /* Tenkey - */
		{AG_KEY_KP7, 0x3a}, /* Tenkey 7 */
		{AG_KEY_KP8, 0x3b}, /* Tenkey 8 */
		{AG_KEY_KP9, 0x3c}, /* Tenkey 9 */
		{AG_KEY_KP4, 0x3e}, /* Tenkey 4 */
		{AG_KEY_KP5, 0x3f}, /* Tenkey 5 */
		{AG_KEY_KP6, 0x40}, /* Tenkey 6 */
		{AG_KEY_KP1, 0x42}, /* Tenkey 1 */
		{AG_KEY_KP2, 0x43}, /* Tenkey 2 */
		{AG_KEY_KP3, 0x44}, /* Tenkey 3 */
		{AG_KEY_KP0, 0x46}, /* Tenkey 0 */
		{AG_KEY_KP_PERIOD, 0x47}, /* Tenkey . */
		{AG_KEY_KP_ENTER, 0x45}, /* Tenkey CR */
		{(AG_KeySym)0xffff, 0xff} /* End */
};

/*
 * キーコードテーブルを初期値にする
 */
void AgarKbdInterface::InitKeyTable(void){
	int i;

	memset(KeyCodeTable2, 0, sizeof(KeyCodeTable2));

	for(i = 0; i<256 ; i++)
	{
		if(KeyTableAG[i].sym == 0xffff) break;
		KeyCodeTable2[i].code = KeyTableAG[i].sym;
		KeyCodeTable2[i].mod = AG_KEYMOD_NONE;
		KeyCodeTable2[i].pushCode = KeyTableAG[i].code;
	}
	/*
	 * マウスキャプチャの初期値はPF11
	 */
	MouseCapture.sym = AG_KEY_F11;
	MouseCapture.mod = AG_KEYMOD_NONE;
	/*
	 *    リセットキーの初期値はPF12
	 */
	ResetKey.sym = AG_KEY_F12;
	ResetKey.mod = AG_KEYMOD_NONE;

}

/*
 * キーテーブルを読み込む
 */
void AgarKbdInterface::LoadKeyTable(void *pMap){
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

void AgarKbdInterface::InitLocalVar(void){
	kbd_snooped = FALSE;
}


void AgarKbdInterface::ResetKeyMap(void)
{
	InitKeyTable();
}


void AgarKbdInterface::OnPress(int sym, int mod, Uint32 unicode)
{
    int            i = 0;
    Uint32 modifier = (Uint32)mod;
    Uint32 code = (Uint32)sym;
//    Uint8 scan = event->key.keysym.scancode;
    struct XM7KeyCode  *p = KeyCodeTable2;

    if(kbd_snooped) {
            //return SnoopedOnKeyPressedCallback(event);
    }
    //printf("Key SDL:%04x\n",code);
    for (i = 0; i < 255; i++) {
	if (p[i].code == 0xffff)   break;
	if (code == (Uint32)p[i].code){
			PushKeyData(p[i].pushCode, 0x80); /* Make */
			break;
		}
    }
    return;
}

void AgarKbdInterface::OnRelease(int sym, int mod, Uint32 unicode)
{
    int            i;

    Uint32 modifier = (Uint32)mod;
    Uint32 code = (Uint32)sym;
    struct XM7KeyCode *p = KeyCodeTable2;

    for (i = 0; i < 255; i++) {
    	if (p[i].code == 0xffff)   break;
    	if (code == (Uint32)p[i].code){
    			PushKeyData(p[i].pushCode, 0x00); /* Break */
    			break;
    		}
    }

	/*
	 * F11押下の場合はマウスキャプチャフラグを反転させてモード切り替え->メニューにする
	 */
//	if ((code == MouseCapture.sym) && (modifier == MouseCapture.mod)) {
	if (code == MouseCapture.sym) {
    	if(bMouseCaptureFlag) {
			bMouseCaptureFlag = FALSE;
			SDL_WM_GrabInput(SDL_GRAB_OFF);
		} else {
			bMouseCaptureFlag = TRUE;
			SDL_WM_GrabInput(SDL_GRAB_ON);
		}

    }

	/*
	 * F12押下の場合はVMリセット
	 */
//	if ((code == ResetKey.sym) && (modifier == ResetKey.mod)) {
	if (code == ResetKey.sym) {
		LockVM();
		system_reset();
		UnlockVM();
    }
}



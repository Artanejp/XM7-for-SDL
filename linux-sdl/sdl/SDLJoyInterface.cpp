/*
 * SDLJoyInterface.cpp
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */

#include<X11/Xlib.h>
//#include<gtk/gtk.h>
//#include<gdk/gdkx.h>
//#include<gdk/gdkkeysyms.h>
#include<memory.h>
#include <SDL/SDL.h>
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_syswm.h>

#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "api_kbd.h"
#include "api_js.h"

#include "SDLJoyInterface.h"

extern int nJoyMAX;
SDL_Joystick *JoyEntry;
/*
 * nJoyRaw構成
 * b0 上
 * b1 下
 * b2 左
 * b3 右
 * b4 ボタン0
 * b5 ボタン1
 *
 */
BYTE nJoyRaw;

/*
 * nJoyRawExt構成
 * b0
 * b1
 * b2
 * b3
 * b4 ボタン2
 * b5 ボタン3
 *
 */

BYTE nJoyRawExt;

Uint8 XAXIS;
Uint8 YAXIS;


Uint8 BUTTON0;
Uint8 BUTTON1;
Uint8 BUTTON2;
Uint8 BUTTON3;
int JoyIndex;



SDLJoyInterface::SDLJoyInterface() {
	// TODO Auto-generated constructor stub
	JoyEntry = NULL;
	nJoyRaw = 0x00;
	nJoyRawExt = 0x00;

	YAXIS = 1;
	XAXIS = 0;
	BUTTON0 = 0;
	BUTTON1 = 1;
	BUTTON2 = 2;
	BUTTON3 = 3;
	JoyIndex = -1;
}

SDLJoyInterface::~SDLJoyInterface() {
	// TODO Auto-generated destructor stub
		Close();
		nJoyRaw = 0x00;
		nJoyRawExt = 0x00;

}



SDL_Joystick *SDLJoyInterface::Open(int physNo )
{
	if(SDL_WasInit(SDL_INIT_JOYSTICK) == 0) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	}
	if(SDL_JoystickOpened(physNo) != 0) return NULL; /* 既に使用されてる */
    JoyEntry = SDL_JoystickOpen(physNo);
	if(JoyEntry != NULL) JoyIndex = physNo;
    return JoyEntry;
}

/*
 * Open
 *
 */
SDL_Joystick *SDLJoyInterface::Open(char *name)
{
	int i,joys;
	char s[256];

	if(SDL_WasInit(SDL_INIT_JOYSTICK) == 0) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	}
	if(name == NULL) return NULL;
	joys = SDL_NumJoysticks();
	for(i = 0; i<joys; i++) {
		strcpy(s,SDL_JoystickName(i));
		if(s == NULL) continue;
		if(strcmp(s,name) != 0) continue;
		if(SDL_JoystickOpened(i) != 0) return NULL; /* 既に使用されてる */
			JoyEntry = SDL_JoystickOpen(i);
			if(JoyEntry != NULL) JoyIndex = i;
			return JoyEntry;
		}
	return NULL;
}

void SDLJoyInterface::Close(void)
{
	if(JoyEntry != NULL) {
		SDL_JoystickClose(JoyEntry);
		JoyEntry = NULL;
		JoyIndex = -1;
	}
}

SDL_Joystick *SDLJoyInterface::GetEntry(void)
{
	return JoyEntry;
}

int SDLJoyInterface::GetIndex(void)
{
	return JoyIndex;
}


BOOL SDLJoyInterface::Check(void)
{

	int index = SDL_JoystickIndex(JoyEntry);
	if(SDL_JoystickOpened(index) == 0) {
		// ジョイスティックが抜かれた
		//Close();
		return FALSE;
	}
	return TRUE;
}

BOOL SDLJoyInterface::RegEvent(void)
{
	// イベントハンドラの登録はしない(ハードコード)
	return TRUE;
}

BOOL SDLJoyInterface::UnRegEvent(void)
{
	// イベントハンドラの登録解除はしない(ハードコード)
	return TRUE;
}



/*
 *  ジョイスティック デバイスより読み込み
 */
BYTE SDLJoyInterface::GetJoy( BOOL flag)
{
	BYTE code = 0;
	code = nJoyRaw;	/* いい加減だが… */
	// printf("JS: code = %04x\n",code);
	return code;
}

/*
 *  ジョイスティック デバイスより読み込み(拡張ボタン)
 */

BYTE SDLJoyInterface::GetJoyExt( BOOL flag)
{
	BYTE code = 0;
	code = nJoyRawExt;	/* いい加減だが… */
	// printf("JS: code = %04x\n",code);
	return code;
}


void SDLJoyInterface::SetXAXIS(Uint8 val)
{
	XAXIS = val;
}

void SDLJoyInterface::SetYAXIS(Uint8 val)
{
	YAXIS = val;
}

void SDLJoyInterface::SetBUTTON0(Uint8 val)
{
	BUTTON0 = val;
}

void SDLJoyInterface::SetBUTTON1(Uint8 val)
{
	BUTTON1 = val;
}
void SDLJoyInterface::SetBUTTON2(Uint8 val)
{
	BUTTON2 = val;
}
void SDLJoyInterface::SetBUTTON3(Uint8 val)
{
	BUTTON3 = val;
}

Uint8 SDLJoyInterface::GetXAXIS(void)
{
	return XAXIS;
}

Uint8 SDLJoyInterface::GetYAXIS(void)
{
	return YAXIS;
}

Uint8 SDLJoyInterface::GetBUTTON0(void)
{
	return BUTTON0;
}

Uint8 SDLJoyInterface::GetBUTTON1(void)
{
	return BUTTON1;
}

Uint8 SDLJoyInterface::GetBUTTON2(void)
{
	return BUTTON2;
}

Uint8 SDLJoyInterface::GetBUTTON3(void)
{
	return BUTTON3;
}

void SDLJoyInterface::OnMove(SDL_Event *ev)
{
	Uint8 axis = ev->jaxis.axis;
	Sint16 value = ev->jaxis.value;
	Uint8 num = ev->jaxis.which;
	BYTE code = 0;

	if(num != SDL_JoystickIndex(JoyEntry)) {
			return;
	}

	/*
	 * 軸は0,1のみ(暫定)
	 */
	if(axis == YAXIS) {			/* Y軸? */
		if (value > 256) {
			code = 0x02;	/* 下 */
		} else if (value < -256) {
			code = 0x01;	/* 上 */
		}
		nJoyRaw &= 0xfc;
		nJoyRaw |= code;
	} else if(axis == XAXIS) {
		if (value > 256) {
			code = 0x08;	/* 右 */
		} else if (value < -256) {
			code = 0x04;	/* 左 */
		}
		nJoyRaw &= 0xf3;
		nJoyRaw |= code;
	}
	return;
}

void SDLJoyInterface::OnPress(SDL_Event *ev)
{
	Uint8 num = ev->jbutton.which;
	Uint8 button = ev->jbutton.button;
	int            code = 0;

	/*
	 * JS番号探知
	 */
	if(num != SDL_JoystickIndex(JoyEntry)) {
			return;
	}

	/*
	 * ボタンは0,1のみ(暫定)
	 */
	if(button == BUTTON1) {			/* ボタン1? */
		code = 0x20;
		nJoyRaw |= code;
	} else if(button == BUTTON0) {			/* ボタン0? */
		code = 0x10;
		nJoyRaw |= code;
	} else if(button ==BUTTON3) {			/* ボタン1? */
		code = 0x20;
		nJoyRawExt |= code;
	} else if(button ==BUTTON2) {			/* ボタン1? */
		code = 0x10;
		nJoyRawExt |= code;
	}
	return;
}

void SDLJoyInterface::OnRelease(SDL_Event *ev)
{
	Uint8 num = ev->jbutton.which;
	Uint8 button = ev->jbutton.button;
	int            code = 0;

	/*
	 * JS番号探知
	 */
	if(num != SDL_JoystickIndex(JoyEntry)) {
			return;
	}

	/*
	 * ボタンは0,1のみ(暫定)
	 */
	if(button == BUTTON1) {			/* ボタン1? */
		code = 0x20;
		nJoyRaw &= ~code;
	} else if(button == BUTTON0) {			/* ボタン0? */
		code = 0x10;
		nJoyRaw &= ~code;
	} else if(button ==BUTTON3) {			/* ボタン1? */
		code = 0x20;
		nJoyRawExt &= ~code;
	} else if(button ==BUTTON2) {			/* ボタン1? */
		code = 0x10;
		nJoyRawExt &= ~code;
	}

	return;

}

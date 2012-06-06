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
#include "xm7_sdl.h"
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


SDLJoyInterface::SDLJoyInterface() {
   Uint8 i;
        // TODO Auto-generated constructor stub
	JoyEntry = NULL;
	nJoyRawAxis = 0x00000000;
	nJoyRawButton = 0x00000000;
        *JoyName = '\0';
	YAXIS = 1;
	XAXIS = 0;
	for(i = 0; i < 16; i++) BUTTON[i] = i;
	JoyIndex = -1;
}

SDLJoyInterface::~SDLJoyInterface() {
	// TODO Auto-generated destructor stub
	Close();
	nJoyRawAxis = 0x00000000;
	nJoyRawButton = 0x00000000;
        *JoyName = '\0';
}




SDL_Joystick *SDLJoyInterface::Open(int physNo )
{
    if(SDL_WasInit(SDL_INIT_JOYSTICK) == 0) {
       SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    }
    if(SDL_JoystickOpened(physNo) != 0) return NULL; /* 既に使用されてる */
    JoyEntry = SDL_JoystickOpen(physNo);
    Buttons = 0;
    if(JoyEntry != NULL) {
       JoyIndex = physNo;
       Buttons = SDL_JoystickNumButtons(JoyEntry);
       if(Buttons > 6) Buttons = 6;
       if(SDL_JoystickName(physNo) != NULL) strncpy(JoyName, SDL_JoystickName(physNo),127);
    }
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
	        strncpy(JoyName, s, 127);
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
        if(JoyEntry != NULL) {
	   JoyIndex = SDL_JoystickIndex(JoyEntry);
	} else {
	   JoyIndex = -1;
	}
        return JoyIndex;
}

char *SDLJoyInterface::GetName(void)
{
	return JoyName;
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
DWORD SDLJoyInterface::GetJoyAxis( BOOL flag)
{
	DWORD code = 0;
	code = nJoyRawAxis;	/* いい加減だが… */
	 //printf("JS: code = %08x\n",code);
	return code;
}

/*
 *  ジョイスティック デバイスより読み込み(拡張ボタン)
 */

DWORD SDLJoyInterface::GetJoyButton( BOOL flag)
{
	DWORD code = 0;
	code = nJoyRawButton;	/* いい加減だが… */
	//printf("JS: code = %08x\n",code);
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

void SDLJoyInterface::SetBUTTON(Uint8 idx, Uint8 val)
{
   if(idx > Buttons) return;
	BUTTON[idx] = val;
}

Uint8 SDLJoyInterface::GetXAXIS(void)
{
	return XAXIS;
}

Uint8 SDLJoyInterface::GetYAXIS(void)
{
	return YAXIS;
}

Uint8 SDLJoyInterface::GetBUTTON(Uint8 idx)
{
   if(idx > Buttons) return 0;
	return BUTTON[idx];
}


void SDLJoyInterface::OnMove(SDL_Event *ev)
{
	Uint8 axis = ev->jaxis.axis;
	Sint16 value = ev->jaxis.value;
	Uint8 num = ev->jaxis.which;
	DWORD code = 0;

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
		nJoyRawAxis &= 0xfc;
		nJoyRawAxis |= code;
	} else if(axis == XAXIS) {
		if (value > 256) {
			code = 0x08;	/* 右 */
		} else if (value < -256) {
			code = 0x04;	/* 左 */
		}
		nJoyRawAxis &= 0xf3;
		nJoyRawAxis |= code;
	}
	return;
}

void SDLJoyInterface::OnPress(SDL_Event *ev)
{
	Uint8 num = ev->jbutton.which;
	Uint8 button = ev->jbutton.button;
	DWORD            code;
        int i;

	/*
	 * JS番号探知
	 */
	if(num != SDL_JoystickIndex(JoyEntry)) {
			return;
	}

	/*
	 * ボタンは0~16(MAX)
	 */
        code = 0x00010000;
        for(i = 0; i < Buttons; i++) { 
	   if(button == i) {
		nJoyRawButton |= code;
	   }
	   code <<= 1;
	}	
	return;
}

void SDLJoyInterface::OnRelease(SDL_Event *ev)
{
	Uint8 num = ev->jbutton.which;
	Uint8 button = ev->jbutton.button;
	DWORD            code;
        int i;

	/*
	 * JS番号探知
	 */
	if(num != SDL_JoystickIndex(JoyEntry)) {
			return;
	}

	/*
	 * ボタンは0~15(MAX)
	 */
        code = 0x00010000;
        for(i = 0; i < Buttons; i++) { 
	   if(button == i) {
		nJoyRawButton &= ~code;
	   }
	   code <<= 1;
	}	

	return;

}

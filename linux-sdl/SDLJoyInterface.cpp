/*
 * SDLJoyInterface.cpp
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
#include "api_kbd.h"
#include "api_js.h"

#include "SDLJoyInterface.h"

BYTE nJoyRaw[MAX_SDL_JOY];
SDL_JoyStick *JoyEntry[MAX_SDL_JOY];
int nJoyMAX;

SDLJoyInterface::SDLJoyInterface() {
	// TODO Auto-generated constructor stub
	int i;
	for(i = 0; i<MAX_SDL_JOY; i++) {
		nJoyRaw[i] = 0x00;
		JoyEntry[i] = NULL;
	}
	nJoyMAX = MAX_SDL_JOY;
}

SDLJoyInterface::~SDLJoyInterface() {
	int i;
	// TODO Auto-generated destructor stub
	for(i=0; i<MAX_SDL_JOY ; i++) {
		SDLJoyInterface::Close(i);
		nJoyRaw[i] = 0x00;
	}
	nJoyMAX = 0;
}



SDL_Joystick *SDLJoyInterface::Open(int emulNo,int physNo )
{
	if(SDL_WasInit(SDL_INIT_SUBSYSTEM) == 0) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	}
	if(SDL_JoystickOpened(physNo) != 0) return NULL; /* 既に使用されてる */
    JoyEntry[emulNo % MAX_SDL_JOY] = SDL_JoystickOpen(physNo);
    return JoyEntry[emulNo % MAX_SDL_JOY];
}

/*
 * Open
 *
 */
SDL_Joystick *SDLJoyInterface::Open(int emulNo, char *name)
{
	int i,joys;
	char *s;

	if(SDL_WasInit(SDL_INIT_SUBSYSTEM) == 0) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	}

	joys = SDL_NumJoisticks();
	for(i = 0; i < joys; i++) {
		s = SDL_JoystickName(i);
		if(s == NULL) continue;
		if(strcmp(s,name) == 0){
			if(SDL_JoystickOpened(i) != 0) return NULL; /* 既に使用されてる */
			JoyEntry[emulNo % MAX_SDL_JOY] = SDL_JoystickOpen(i);
			return JoyEntry[emulNo % MAX_SDL_JOY];
		}
	}
	return NULL;
}

void SDLJoyInterface::Close(int emulNo)
{
	if(JoyEntry[emulNo % MAX_SDL_JOY] != NULL) {
		SDL_JoystickClose(JoyEntry[emulNo % MAX_SDL_JOY]);
		JoyEntry[emulNo % MAX_SDL_JOY] = NULL;
	}
}

SDL_Joystick *SDLJoyInterface::GetEntry(int emulNo)
{
	return JoyEntry[emulNo % MAX_SDL_JOY];
}

SDL_Joystick *SDLJoyInterface::GetEntry(int emulNo)
{
	return JoyEntry[emulNo % MAX_SDL_JOY];
}


char *SDLJoyInterface::Check(void)
{
	int i;
	for(i = 0; i< MAX_SDL_JOY; i++){
		if(JoyEntry[i] == NULL) continue;
		if(SDL_JoystickOpened(JoyEntry[i]->which) == 0) {
			// ジョイスティックが抜かれた
			Close(i);
		}
	}
	return NULL;
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
 *  JS関連イベントハンドラ
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
	int i;

	// printf("JS:index = %d axis = %d num = %d value =
	// 0x%02x\n",index,axis,num,value);
	/*
	 * JS番号探知
	 */
	for(i = 0; i<MAX_SDL_JOY ; i++){
		if(JoyEntry[i] == NULL) continue;
		if(num == SDL_JoystickIndex(GetEntry(i))) {
			index = i;
			break;
		}
	}
	if(i>=2) return TRUE; /* 使用外のJS */

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
	int 			i;

	// printf("JS:index = %d num = %d value =
	// 0x%02x\n",index,num,button);
	/*
	 * JS番号探知
	 */
	for(i = 0; i<MAX_SDL_JOY ; i++){
		if(JoyEntry[i] == NULL) continue;
		if(num == SDL_JoystickIndex(GetEntry(i))) {
			index = i;
			break;
		}
	}
	if(i>=2) return TRUE; /* 使用外のJS */

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
	int 		i;
	/*
	 * JS番号探知
	 */
	for(i = 0; i<MAX_SDL_JOY ; i++){
		if(JoyEntry[i] == NULL) continue;
		if(num == SDL_JoystickIndex(GetEntry(i))) {
			index = i;
			break;
		}
	}
	if(i>=2) return TRUE; /* 使用外のJS */


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
 *  ジョイスティック デバイスより読み込み
 */
BYTE SDLJoyInterface::GetJoy(int index, BOOL flag)
{
	BYTE code = 0;
//	if ((index > 1) || (index < 0))
	if(index >= MAX_SDL_JOY)
		return 0;
	code = nJoyRaw[index];	/* いい加減だが… */

	// printf("JS: code = %04x\n",code);
	return code;
}

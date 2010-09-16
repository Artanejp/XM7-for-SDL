/*
 * SDLJoyInterface.h
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */

#ifndef SDLJOYINTERFACE_H_
#define SDLJOYINTERFACE_H_

#include <SDL/SDL.h>
#include "xm7.h"



#define MAX_SDL_JOY 2

class SDLJoyInterface {
public:
	SDLJoyInterface();
	~SDLJoyInterface();
	SDL_Joystick *Open(int emulNo,int physNo );
	SDL_Joystick *Open(int emulNo,char *name );
	void Close(int emulNo);
	BYTE GetJoy(int index, BOOL flag)
	SDL_Joystick *GetEntry(int emulNo);
	char *Check(void);
	BOOL RegEvent(void);
	BOOL UnRegEvent(void);
};


/*
 *  JS関連イベントハンドラ(グローバルだ)
 */
extern BYTE nJoyRaw[MAX_SDL_JOY];
extern SDL_JoyStick *JoyEntry[MAX_SDL_JOY];
extern int nJoyMAX;
extern BOOL OnMoveJoy(SDL_Event * event);
extern BOOL OnPressJoy(SDL_Event * event);
extern BOOL OnReleaseJoy(SDL_Event * event);


#endif /* SDLJOYINTERFACE_H_ */

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
	SDL_Joystick *Open(int physNo );
	SDL_Joystick *Open(char *name );
	void Close();

	void SetXAXIS(Uint8 val);
	void SetYAXIS(Uint8 val);
	void SetBUTTON(Uint8 idx, Uint8 val);


	Uint8 GetXAXIS(void);
	Uint8 GetYAXIS(void);
	Uint8 GetBUTTON(Uint8 idx);

	DWORD GetJoyAxis(BOOL flag);
	DWORD GetJoyButton(BOOL flag);

	SDL_Joystick *GetEntry(void);
	int GetIndex(void);
	char *GetName(void);

        BOOL Check(void);
	BOOL RegEvent(void);
	BOOL UnRegEvent(void);

	void OnMove(SDL_Event *event);
	void OnPress(SDL_Event *event);
	void OnRelease(SDL_Event *event);


private:
	SDL_Joystick *JoyEntry;
        char JoyName[128];
	int JoyIndex;
	DWORD nJoyRawButton;
        DWORD nJoyRawAxis;
//	BYTE nJoyRawExt;
	Uint8 XAXIS;
	Uint8 YAXIS;
        int  Buttons;
        Uint8 BUTTON[16];
};

#endif /* SDLJOYINTERFACE_H_ */

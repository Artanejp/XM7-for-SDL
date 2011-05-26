/*
 * SDLJoyInterface.h
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */

#ifndef SDLJOYINTERFACE_H_
#define SDLJOYINTERFACE_H_

#include <SDL.h>
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
	void SetBUTTON0(Uint8 val);
	void SetBUTTON1(Uint8 val);
	void SetBUTTON2(Uint8 val);
	void SetBUTTON3(Uint8 val);


	Uint8 GetXAXIS(void);
	Uint8 GetYAXIS(void);
	Uint8 GetBUTTON0(void);
	Uint8 GetBUTTON1(void);
	Uint8 GetBUTTON2(void);
	Uint8 GetBUTTON3(void);

	BYTE GetJoy(BOOL flag);
	BYTE GetJoyExt(BOOL flag);

	SDL_Joystick *GetEntry(void);
	int GetIndex(void);
	BOOL Check(void);
	BOOL RegEvent(void);
	BOOL UnRegEvent(void);

	void OnMove(SDL_Event *event);
	void OnPress(SDL_Event *event);
	void OnRelease(SDL_Event *event);


private:
	SDL_Joystick *JoyEntry;
	int JoyIndex;
	BYTE nJoyRaw;
	BYTE nJoyRawExt;
	Uint8 XAXIS;
	Uint8 YAXIS;
	Uint8 BUTTON0;
	Uint8 BUTTON1;
	Uint8 BUTTON2;
	Uint8 BUTTON3;

};

#endif /* SDLJOYINTERFACE_H_ */

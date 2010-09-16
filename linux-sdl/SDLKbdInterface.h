/*
 * SDLKbdInterface.h
 *
 *  Created on: 2010/09/16
 *      Author: whatisthis
 */

#ifndef SDLKBDINTERFACE_H_
#define SDLKBDINTERFACE_H_

#include "KbdInterface.h"


class SDLKbdInterface: public virtual KbdInterface {
public:
	SDLKbdInterface();
	virtual ~SDLKbdInterface();
	Uint32 GetKeymap(void *nativeCode);
	void SetKeymap(Uint32 keyCode, Uint32 nativeCode, Uint32 mod);
	struct KeyCode *GetKeyCode(int num);
	SDLKey GetKeyCode(int num);
	Uint8 GetKeyCode(int num);
	SDLMod GetKeyMod(int num);
	SDL_keysym GetNativeCode(void);
	void OnPress(void *eventh);
	void OnRelease(void *eventh);
	void SetKbdSnoop(BOOL t);
	void ResetKeyMap(void);
private:
	struct KeyCode KeyCodeTable2[256];
	void SDLKbdInterface::InitKeyTable(void);
	BOOL kbd_snooped = FALSE;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;

};

#endif /* SDLKBDINTERFACE_H_ */

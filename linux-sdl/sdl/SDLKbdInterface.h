/*
 * SDLKbdInterface.h
 *
 *  Created on: 2010/09/16
 *      Author: whatisthis
 */

#ifndef SDLKBDINTERFACE_H_
#define SDLKBDINTERFACE_H_

#include "KbdInterface.h"

#ifndef SDLK_FIRST
#define SDLK_FIRST SDLK_0
#endif

class SDLKbdInterface: public KbdInterface {
public:
	SDLKbdInterface();
	~SDLKbdInterface();
	void OnPress(void *eventh);
	void OnRelease(void *eventh);
	void InitKeyTable(void);
	void ResetKeyMap(void);
	void LoadKeyTable(void *pMap);
private:
	struct XM7KeyCode KeyCodeTable2[256];
	//struct KeyCode2 KeyTableSDL[];
	void InitLocalVar(void);
//	void InitKeyTable(void);
//	BOOL kbd_snooped;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;
};

#endif /* SDLKBDINTERFACE_H_ */

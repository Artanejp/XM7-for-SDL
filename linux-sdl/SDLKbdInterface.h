/*
 * SDLKbdInterface.h
 *
 *  Created on: 2010/09/16
 *      Author: whatisthis
 */

#ifndef SDLKBDINTERFACE_H_
#define SDLKBDINTERFACE_H_

#include "KbdInterface.h"


class SDLKbdInterface: public KbdInterface {
public:
	SDLKbdInterface();
	~SDLKbdInterface();
	void OnPress(void *eventh);
	void OnRelease(void *eventh);
	void InitKeyTable(void);
	void ResetKeyMap(void);
	void LoadKeyMap(void *pMap);
private:
	struct KeyCode KeyCodeTable2[256];
	const struct KeyCode2 KeyTable[];
//	void InitKeyTable(void);
	BOOL kbd_snooped = FALSE;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;

};

#endif /* SDLKBDINTERFACE_H_ */

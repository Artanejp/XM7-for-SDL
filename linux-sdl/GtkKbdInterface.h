/*
 * GtkKbdInterface.h
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */

#ifndef GTKKBDINTERFACE_H_
#define GTKKBDINTERFACE_H_

#include "xm7.h"
#include <SDL/SDL.h>
#include "KbdInterface.h"


class GtkKbdInterface : public virtual KbdInterface {
public:
	GtkKbdInterface();
	~GtkKbdInterface();
	Uint32 GetKeymap(void *nativeCode);
	void SetKeymap(Uint32 keyCode, Uint32 nativeCode, Uint32 mod);
	struct KeyCode *GetKeyCode(int num);
	Uint32 GetKeyCode(int num);
	Uint8 GetKeyCode(int num);
	Uint32 GetKeyMod(int num);
	void OnPress(void *eventh);
	void OnRelease(void *eventh);
	void SetKbdSnoop(BOOL t);
	void ResetKeyMap(void);
private:
	struct KeyCode KeyCodeTable2[256];
	void InitKeyTable(void);
	BOOL kbd_snooped = FALSE;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;
};

#endif /* GTKKBDINTERFACE_H_ */

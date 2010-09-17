/*
 * KbdInterface.h
 *
 *  Created on: 2010/09/16
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *      このクラスではキーボード廻りのテンプレートを設計する
 */

#ifndef KBDINTERFACE_H_
#define KBDINTERFACE_H_

#include <SDL/SDL.h>
#include "xm7.h"

struct {
		Uint32 code;
		Uint32 mod;
		Uint8 pushCode;
} KeyCode;

struct {
	Uint32 sym;
	Uint32 mod;
} SpecialKey;

extern void PushKeyData(Uint8 code,Uint8 MakeBreak);

class KbdInterface {
public:
	KbdInterface();
	~KbdInterface();
	void CleanKbd(void);

	Uint32 GetKeyCode(Uint8 num);
	Uint32 GetKeyMod(Uint8 num);

	struct KeyCode *GetKeyMap(Uint8 pushCode);
	struct KeyCode *GetKeyMap(void);
	void SetKeymap(Uint32 keyCode, Uint32 nativeCode, Uint32 mod);
	void SetKeymap(struct KeyCode *p);
//	virtual void SetKbdSnoop(BOOL t);
	void ResetKeyMap(void);
	void SetResetKey(Uint32 code, Uint32 mod);
	void SetMouseKey(Uint32 code, Uint32 mod);
	virtual void InitKeyTable(void);
	virtual void ResetKeyMap(void);
	virtual void LoadKeyMap(void *pMap);

private:
	struct KeyCode KeyCodeTable2[256];
	const struct KeyCode2 KeyTable106[];
	void InitKeyTable(void);
	void InitLocalVar(void);
	BOOL kbd_snooped = FALSE;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;
};

#endif /* KBDINTERFACE_H_ */

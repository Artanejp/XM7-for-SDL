/*
 * KbdInterface.h
 *
 *  Created on: 2010/09/16
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *      このクラスではキーボード廻りのテンプレートを設計する
 */

#ifndef KBDINTERFACE_H_
#define KBDINTERFACE_H_

#include <SDL.h>
#include "xm7.h"
#include "api_kbd.h"
#include "api_mouse.h"

struct XM7KeyCode {
		Uint32 code;
		Uint32 mod;
		Uint8 pushCode;
};

struct SpecialKey {
	Uint32 sym;
	Uint32 mod;
} ;

/*
 * キーコードテーブル
 */
struct KeyCode2 {
	SDLKey sym;
	Uint8 code;
};

extern void PushKeyData(Uint8 code,Uint8 MakeBreak);

class KbdInterface {
public:
	KbdInterface();
	~KbdInterface();
	void CleanKbd(void);

	Uint32 GetKeyCode(Uint8 num);
	Uint32 GetKeyMod(Uint8 num);

	struct XM7KeyCode *GetKeyMap(Uint8 pushCode);
	struct XM7KeyCode *GetKeyMap(void);
	void SetKeymap(Uint32 keyCode, Uint32 nativeCode, Uint32 mod);
	void SetKeymap(struct XM7KeyCode *p);
//	virtual void SetKbdSnoop(BOOL t);
	void SetResetKey(Uint32 code, Uint32 mod);
	void SetMouseKey(Uint32 code, Uint32 mod);
	void InitKeyTable(void);
	void ResetKeyMap(void);
	void LoadKeyTable(void *pMap);

private:
	struct XM7KeyCode KeyCodeTable2[256];
	struct KeyCode2 KeyTable106[];
	void InitLocalVar(void);
	BOOL kbd_snooped;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;
};

#endif /* KBDINTERFACE_H_ */

/*
 * KbdInterface.h
 *
 *  Created on: 2010/09/16
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *      このクラスではキーボード廻りのテンプレートを設計する
 */

#ifndef KBDINTERFACE_H_
#define KBDINTERFACE_H_
struct {
		Uint32 code,
		Uint32 mod,
		Uint8 pushCode
} KeyCode;

struct {
	Uint32 sym;
	Uint32 mod;
} SpecialKey;

extern void PushKeyData(Uint8 code,Uint8 MakeBreak);

class KbdInterface {
public:
	virtual KbdInterface();
	virtual ~KbdInterface();
	virtual Uint32 GetKeymap(void *nativeCode);
	virtual void SetKeymap(Uint32 keyCode, Uint32 nativeCode, Uint32 mod);
	struct KeyCode *GetKeyCode(int num);
	virtual Uint8 GetKeyCode(int num);
	virtual void OnPress(void *eventh);
	virtual void OnRelease(void *eventh);
	virtual void SetKbdSnoop(BOOL t);
	virtual void ResetKeyMap(void);
private:
	struct KeyCode KeyCodeTable2[256];
	void InitKeyTable(void);
	BOOL kbd_snooped = FALSE;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;
};

#endif /* KBDINTERFACE_H_ */

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


class GtkKbdInterface : public KbdInterface {
public:
	GtkKbdInterface();
	~GtkKbdInterface();

	void OnPress(void *eventh);
	void OnRelease(void *eventh);
	void InitKeyTable(void);
	void ResetKeyMap(void);
	void LoadKeyMap(void *pMap);
private:
	struct XM7KeyCode KeyCodeTable2[256];
	BOOL kbd_snooped;
	struct SpecialKey ResetKey;
	struct SpecialKey MouseCapture;

};

#endif /* GTKKBDINTERFACE_H_ */

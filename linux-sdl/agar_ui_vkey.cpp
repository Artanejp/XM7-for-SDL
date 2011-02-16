/*
 * agar_ui_vkey.cpp
 *
 *  Created on: 2011/02/01
 *      Author: whatisthis
 *      仮想キーボードを構築する
 */

#include <SDL.h>
#include <libintl.h>
extern "C" {
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
}
#include "xm7.h"
#include "fdc.h"
#include "AgarKbdInterface.h"

class VirtualKey {
	Uint8 keyCode;
	char *name;
	char *keytop;
	BOOL Enabled;
	void OnPress(AG_Event *event);
	void (* pressed)(Uint8 code);
	void SetPushedFunc(void( func)(Uint8));
	void SetKeyCode(Uint8 code);
	VirtualKey();
	~VirtualKey();
	AG_Button *btn;
};

void VirtualKey::OnPress(AG_Event *event) {
	AG_Button *button = (AG_Button *) AG_SELF();
	Uint code = AG_INT(1);
	if (pressed == NULL)
		return;
	VirtualKey::pressed(code);

}

VirtualKey::VirtualKey() {
}

VirtualKey::~VirtualKey() {

}

void VirtualKey::SetPushedFunc(void( func)(Uint8)) {
	this->pressed = func;
}

void VirtualKey::SetKeyCode(Uint8 code) {
	this->keyCode = code;
}

struct KeyCode_Vkey {
	const char *KeyName;
	Uint8 KeyCode;
};

const struct KeyCode_Vkey VKeyTableAG_1[] = { { "PF1", 0x5d }, /* PF1 */
{ "PF2", 0x5e }, /* PF2 */
{ "PF3", 0x5f }, /* PF3 */
{ "PF4", 0x60 }, /* PF4 */
{ "PF5", 0x61 }, /* PF5 */
};
const struct KeyCode_Vkey VKeyTableAG_2[] = { { "BRK", 0x5c }, /* BREAK(ESC) */
{ "PF6", 0x62 }, /* PF6 */
{ "PF7", 0x63 }, /* PF7 */
{ "PF8", 0x64 }, /* PF8 */
{ "PF9", 0x65 }, /* PF9 */
{ "PF10", 0x66 }, /* PF10 */
};
const struct KeyCode_Vkey VKeyTableAG_3[] = {
//		{AG_KEY_FIRST, 0x01}, /* ESC(半角/全角) */
		{ "ESC", 0x01 }, /* ESC(ScrLk):半角/全角がIMに取られるため */
		{ "1", 0x02 }, /* 1 */
		{ "2", 0x03 }, /* 2 */
		{ "3", 0x04 }, /* 3 */
		{ "4", 0x05 }, /* 4 */
		{ "5", 0x06 }, /* 5 */
		{ "6", 0x07 }, /* 6 */
		{ "7", 0x08 }, /* 7 */
		{ "8", 0x09 }, /* 8 */
		{ "9", 0x0a }, /* 9 */
		{ "0", 0x0b }, /* 0 */
		{ "-", 0x0c }, /* - */
		{ "^", 0x0d }, /* ^ */
		{ "¥", 0x0e }, /* \ */
		{ "BS", 0x0f }, /* BS */
};
const struct KeyCode_Vkey VKeyTableAG_4[] = { { "TAB", 0x10 }, /* TAB */
{ "Q", 0x11 }, /* Q */
{ "W", 0x12 }, /* W */
{ "E", 0x13 }, /* E */
{ "R", 0x14 }, /* R */
{ "T", 0x15 }, /* T */
{ "Y", 0x16 }, /* Y */
{ "U", 0x17 }, /* U */
{ "I", 0x18 }, /* I */
{ "O", 0x19 }, /* O */
{ "P", 0x1a }, /* P */
{ "@", 0x1b }, /* @ */
{ "[", 0x1c }, /* [ */
};
const struct KeyCode_Vkey VKeyTableAG_5[] = { { "RET", 0x1d }, /* CR */
};
const struct KeyCode_Vkey VKeyTableAG_6[] = { { "A", 0x1e }, /* A */
{ "S", 0x1f }, /* S */
{ "D", 0x20 }, /* D */
{ "F", 0x21 }, /* F */
{ "G", 0x22 }, /* G */
{ "H", 0x23 }, /* H */
{ "J", 0x24 }, /* J */
{ "K", 0x25 }, /* K */
{ "L", 0x26 }, /* L */
{ ";", 0x27 }, /* ; */
{ ":", 0x28 }, /* : */
{ "]", 0x29 }, /* ] */
};
const struct KeyCode_Vkey VKeyTableAG_7[] = { { "LSHIFT", 0x53 }, /* 左SHIFT */
{ "Z", 0x2a }, /* Z */
{ "X", 0x2b }, /* X */
{ "C", 0x2c }, /* C */
{ "V", 0x2d }, /* V */
{ "B", 0x2e }, /* B */
{ "N", 0x2f }, /* N */
{ "M", 0x30 }, /* M */
{ ",", 0x31 }, /* , */
{ ".", 0x32 }, /* . */
{ "/", 0x33 }, /* / */
{ "_", 0x34 }, /* _ */
{ "RSHIFT", 0x54 }, /* 右SHIFT */
};
const struct KeyCode_Vkey VKeyTableAG_8[] = { { "LCTRL", 0x52 }, /* CTRL(左Ctrl) */
{ "CAPS", 0x55 }, /* CAP(左ALT) */
{ "GRPH", 0x56 }, /* WIN(無変換) for 109 */
{ "変換", 0x57 }, /* 左SPACE(変換) for 109 */
{ "カナ", 0x58 }, /* 中SPACE(カタカナ) */
{ "SPACE", 0x35 }, /* 右SPACE(SPACE) */
{ "かな", 0x5a }, /* かな(右Ctrl) for 109 */

};
const struct KeyCode_Vkey VKeyTableAG_9[] = { { "INS", 0x48 }, /* INS(Insert) */
{ "↑", 0x4d }, /* ↑ */
{ "DEL", 0x4b }, /* DEL(Delete) */
};
const struct KeyCode_Vkey VKeyTableAG_10[] = { { "←", 0x4f }, /* ← */
{ "↓", 0x50 }, /* ↓ */
{ "→", 0x51 }, /* → */
};

const struct KeyCode_Vkey VKeyTableAG_11_1[] = { { "EL", 0x49 }, /* EL(Home) */
{ "CLS", 0x4a }, /* CLS(Page Up) */
};
const struct KeyCode_Vkey VKeyTableAG_11_2[] = { { "DUP", 0x4c }, /* DUP(End) */
{ "HOME", 0x4e }, /* HOME(Page Down) */
};
const struct KeyCode_Vkey VKeyTableAG_12[] = { { "*", 0x36 }, /* Tenkey * */
{ "/", 0x37 }, /* Tenkey / */
{ "+", 0x38 }, /* Tenkey + */
{ "-", 0x39 }, /* Tenkey - */
};
const struct KeyCode_Vkey VKeyTableAG_13[] = { { "7", 0x3a }, /* Tenkey 7 */
{ "8", 0x3b }, /* Tenkey 8 */
{ "9", 0x3c }, /* Tenkey 9 */
};
const struct KeyCode_Vkey VKeyTableAG_14[] = { { "4", 0x3e }, /* Tenkey 4 */
{ "5", 0x3f }, /* Tenkey 5 */
{ "6", 0x40 }, /* Tenkey 6 */
};
const struct KeyCode_Vkey VKeyTableAG_15[] = { { "1", 0x42 }, /* Tenkey 1 */
{ "2", 0x43 }, /* Tenkey 2 */
{ "3", 0x44 }, /* Tenkey 3 */
};
const struct KeyCode_Vkey VKeyTableAG_16[] = { { "0", 0x46 }, /* Tenkey 0 */
{ ".", 0x47 }, /* Tenkey . */
{ "RET", 0x45 }, /* Tenkey CR */
};

extern void PushKeyData(Uint8, Uint8); /* Make */

static void OnPressVkey(AG_Event *event) {
	AG_Button *button = (AG_Button *) AG_SELF();
	Uint code = AG_INT(1);
	Uint state = AG_INT(2);
	PushKeyData(code, 0x80); /* Make */
	AG_Delay(50);
	PushKeyData(code, 0x00); /* Break */
}

static void OnPressVModkey(AG_Event *event) {
	AG_Button *button = (AG_Button *) AG_SELF();
	Uint code = AG_INT(1);
	if(button->state) {
		PushKeyData(code, 0x80); /* Break */
	} else {
		PushKeyData(code, 0x00); /* Break */
	}
}


void VkeyBoard(AG_Event *event, void (func_press)(AG_Event *), void (func_modkey)(AG_Event *))
{
	AG_Button *p;
	AG_Window *w;
	AG_Box *box, *hbox, *vbox, *vbox2;
	int i;
	int j;

	w = AG_WindowNew(0);
	vbox = AG_BoxNewVert(w, 0);
	{
		vbox2 = AG_BoxNewVert(vbox, 0);
		box = AG_BoxNewHoriz(vbox2, 0);
		{
			p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
					"       ");
			AG_WidgetDisable(p);
			i = sizeof(VKeyTableAG_1) / sizeof(struct KeyCode_Vkey);
			for (j = 0; j < i; j++) {
				p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
						VKeyTableAG_1[j].KeyName);
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_1[j].KeyCode);
				AG_ButtonSetRepeatMode(p, 0);
			}
			p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
					"      ");
			AG_WidgetDisable(p);
			i = sizeof(VKeyTableAG_11_1) / sizeof(struct KeyCode_Vkey);
			for (j = 0; j < i; j++) {
				p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
						VKeyTableAG_11_1[j].KeyName);
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_11_1[j].KeyCode);
				AG_ButtonSetRepeatMode(p, 1);

			}
			p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
					"      ");
			AG_WidgetDisable(p);
			i = sizeof(VKeyTableAG_9) / sizeof(struct KeyCode_Vkey);
			for (j = 0; j < i; j++) {
				p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
						VKeyTableAG_9[j].KeyName);
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_9[j].KeyCode);

				AG_ButtonSetRepeatMode(p, 1);
			}
		}
		box = AG_BoxNewHoriz(vbox2, 0);
		{
			i = sizeof(VKeyTableAG_2) / sizeof(struct KeyCode_Vkey);
			for (j = 0; j < i; j++) {
				p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
						VKeyTableAG_2[j].KeyName);
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_2[j].KeyCode);
			}
			p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
					"    ");
			AG_WidgetDisable(p);
			i = sizeof(VKeyTableAG_11_2) / sizeof(struct KeyCode_Vkey);
			for (j = 0; j < i; j++) {
				p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
						VKeyTableAG_11_2[j].KeyName);
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_11_2[j].KeyCode);
			}
			p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
					"  ");
			AG_WidgetDisable(p);

			i = sizeof(VKeyTableAG_10) / sizeof(struct KeyCode_Vkey);
			for (j = 0; j < i; j++) {
				p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
						VKeyTableAG_10[j].KeyName);
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_10[j].KeyCode);
			}

		}

	}
	box = AG_BoxNewHoriz(vbox, 0);
	{
		i = sizeof(VKeyTableAG_3) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT, VKeyTableAG_3[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_3[j].KeyCode);
		}
		p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
				"         ");
		AG_WidgetDisable(p);
		i = sizeof(VKeyTableAG_12) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
					VKeyTableAG_12[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_12[j].KeyCode);
		}

	}
	box = AG_BoxNewHoriz(vbox, 0);
	{
		i = sizeof(VKeyTableAG_4) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT, VKeyTableAG_4[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_4[j].KeyCode);
		}
		i = sizeof(VKeyTableAG_5) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT, VKeyTableAG_5[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_5[j].KeyCode);
		}
		p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
				"            ");
		AG_WidgetDisable(p);
		i = sizeof(VKeyTableAG_13) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
					VKeyTableAG_13[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_13[j].KeyCode);
		}
	}
	box = AG_BoxNewHoriz(vbox, 0);
	{
		p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
				"         ");
		AG_WidgetDisable(p);
		i = sizeof(VKeyTableAG_6) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT, VKeyTableAG_6[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_6[j].KeyCode);
		}
		p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
				"                        ");
		AG_WidgetDisable(p);
		i = sizeof(VKeyTableAG_14) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
					VKeyTableAG_14[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_14[j].KeyCode);
		}

	}
	box = AG_BoxNewHoriz(vbox, 0);
	{
		i = sizeof(VKeyTableAG_7) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			//				hbox = AG_BoxNewHoriz(box, AG_HBOX_VFILL);
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT, VKeyTableAG_7[j].KeyName);
			switch(VKeyTableAG_7[j].KeyCode) {
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x56:
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_modkey, "%i", VKeyTableAG_7[j].KeyCode);
				AG_ButtonSetSticky(p, 0x01);
				break;
			default:
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_7[j].KeyCode);
				break;
			}
		}
		p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
				"         ");
		AG_WidgetDisable(p);
		i = sizeof(VKeyTableAG_15) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
					VKeyTableAG_15[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_15[j].KeyCode);
		}

	}
	box = AG_BoxNewHoriz(vbox, 0);
	{
		p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
				"  ");
		AG_WidgetDisable(p);
		AG_WidgetHide(p);
		i = sizeof(VKeyTableAG_8) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT, VKeyTableAG_8[j].KeyName);
			switch(VKeyTableAG_8[j].KeyCode) {
			case 0x52:
			case 0x53:
			case 0x54:
			case 0x56:
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_modkey, "%i", VKeyTableAG_8[j].KeyCode);
				AG_ButtonSetSticky(p, 0x01);
				break;
			default:
				AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_8[j].KeyCode);
				break;
			}
//			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_8[j].KeyCode);
		}
		p = AG_ButtonNew(box, AG_BUTTON_MOUSEOVER | AG_BUTTON_STICKY, "%s",
				"        ");
		AG_WidgetDisable(p);
		AG_WidgetHide(p);
		i = sizeof(VKeyTableAG_16) / sizeof(struct KeyCode_Vkey);
		for (j = 0; j < i; j++) {
			p = AG_ButtonNew(box, AG_BUTTON_REPEAT,
					VKeyTableAG_16[j].KeyName);
			AG_SetEvent(AGWIDGET(p), "button-pushed", func_press, "%i", VKeyTableAG_16[j].KeyCode);
		}

	}

	AG_WindowSetCaption(w, gettext("Virtual Keyboard"));
	AG_WindowShow(w);
}


void BuildVkeyBoard(AG_Event *event) {
	VkeyBoard(event, OnPressVkey, OnPressVModkey);
}

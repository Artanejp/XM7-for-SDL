/*
 * agar_gui_input.cpp
 *
 * Preferences Page : Input.
 *
 *  Created on: 2014/10/03
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include <libintl.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <SDL/SDL.h>

#include "xm7.h"
#include "device.h"
#include "fdc.h"
#include "tapelp.h"
#include "opn.h"
#include "keyboard.h"
#include "mmr.h"
#include "mouse.h"
#include "aluline.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "xm7_sdl.h"
#endif

#include "sdl_inifile.h"
#include "agar_cfg.h"
#include "xm7.h"

extern configdat_t localconfig;

extern void OnPushCancel(AG_Event *event);
extern void OnConfigApply(AG_Event *event);

static const char *JsTypeName[] = {
		"None",
		"Port 1",
		"Port 2",
		"JoyKey",
		"Dempa",
		NULL
};
static 	int jsType[2];


static void OnChangeJsType(AG_Event *event)
{
	int number = AG_INT(1);
	int type = AG_INT(2);

	localconfig.nJoyType[number] = type;
}

static void InputMenuJS(AG_NotebookTab *parent)
{
	AG_Box *box;
	AG_Radio *radio;
	AG_Box *vbox;
	AG_Label *lbl;


	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_VFILL);
	{
		vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
		{
		lbl = AG_LabelNew(AGWIDGET(vbox), 0, gettext("Joystick #0"));
		jsType[0] = localconfig.nJoyType[0];
		radio = AG_RadioNewFn(AGWIDGET(vbox), 0, JsTypeName, OnChangeJsType, "%i", 0);
		AG_BindInt(radio, "value",&jsType[0]);
		}

		vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
		{
		lbl = AG_LabelNew(AGWIDGET(vbox), 0, gettext("Joystick #1"));
		jsType[1] = localconfig.nJoyType[1];
		radio = AG_RadioNewFn(AGWIDGET(vbox), 0, JsTypeName, OnChangeJsType, "%i", 1);
		AG_BindInt(radio, "value",&jsType[1]);
		}
	}

}

extern void BuildVkeyBoard(AG_Event *event);
extern void SetKeyTable(AG_Event *event);

static void InputMenuKbd(AG_NotebookTab *parent)
{
	AG_Box *box;
	AG_Checkbox *check;
	AG_Box *vbox;
	AG_Button *btn;

	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);


	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Bind multi-pressed cursor as oblique direction"), &localconfig.bArrow8Dir);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Bind Cursor as Ten-Key"), &localconfig.bTenCursor);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Pseudo Realtime Scan"), &localconfig.bKbdReal);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Virtual Keyboard"), BuildVkeyBoard, NULL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Assign Keyboard"), SetKeyTable, NULL);
	}

}

static void InputMenuMouse(AG_NotebookTab *parent)
{
	AG_Box *box;
	AG_Checkbox *check;
	AG_Box *vbox;
	AG_Numerical *port;

	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);


	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Connect Inteligent Mouse"), &localconfig.bMouseCapture);
		port = AG_NumericalNewUintR(AGWIDGET(box), AG_NUMERICAL_HFILL, NULL ,gettext("Connected port"), &localconfig.nMousePort,
					   1, 2);
	}

}

void OnConfigInputMenu(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_Notebook *note;

	AG_NotebookTab *tab;
	AG_Box *box;
	AG_Button *btn;

	memcpy(&localconfig, &configdat, sizeof	(configdat_t));

	win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
//	AG_WindowSetMinSize(win, 320, 240);
    note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_HFILL);
    {
    	/*
    	 * 
    	 */

    	tab = AG_NotebookAddTab(note, gettext("Keyboard"), AG_BOX_HORIZ);
    	InputMenuKbd(tab);

    	tab = AG_NotebookAddTab(note, gettext("Joystick"), AG_BOX_HORIZ);
    	InputMenuJS(tab);

    	tab = AG_NotebookAddTab(note, gettext("Mouse"), AG_BOX_HORIZ);
        InputMenuMouse(tab);
    }
    box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);
    AG_WidgetSetSize(AGWIDGET(box), 320, 24);
    {
    	AG_Box *vbox;
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnConfigApply, NULL);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
        AG_WidgetSetSize(AGWIDGET(vbox), 80, 24);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Cancel"), OnPushCancel, NULL);
    }
	AG_WindowSetCaption(win, gettext("Configure Input"));
	AG_WindowShow(win);
}


/*
 * agar_gui_pref.cpp
 *
 * Preferences Page
 *
 *  Created on: 2010/11/25
 *      Author: whatisthis
 */

#include <libintl.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include "xm7.h"
#include "device.h"
#include "fdc.h"
#include "tapelp.h"
#include "opn.h"
//#include "whg.h"
//#include "thg.h"
#include "keyboard.h"
#include "mmr.h"
#include "mouse.h"
#include "aluline.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif
#include "sdl_inifile.h"
#include "agar_cfg.h"
#include "sdl_prop.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_bar.h"



extern "C" {
static configdat_t localconfig;
}

extern void OnPushCancel(AG_Event *event);
extern void KeyBoardSnoop(BOOL t);


static const char *EmuTypeName[] =
{
		"FM-7",
		"FM-77AV",
		"FM-77AVEX",
		NULL
};
enum EmuTypeNum  {
		FM7=1,
		FM77AV,
		FM77AVEX
};

static const char *EmuSpeedName[] =
{
		"FAST Mode",
		"SLOW Mode",
		NULL
};

enum EmuSpeedNum  {
		EMUSP_FAST=0,
		EMUSP_SLOW,
};
static const char *EmuCycleName[] =
{
		"Main CPU",
		"Main CPU using MMR",
		"Main CPU using MMR Fast",
		"Sub CPU",
		NULL
};

enum EmuCycleNum  {
	EMUCYCLE_MAIN = 0,
	EMUCYCLE_MAIN_MMR,
	EMUCYCLE_MAIN_FASTMMR,
	EMUCYCLE_SUB
};

void ConfigMenuEmulation(AG_Event *event)
{

}

static const char *ScreenSizeName[] =
{
		"320x200",
		"320x240",
		"640x400",
		"640x480",
		"800x500",
		"800x600",
		"1024x768",
		"1280x800",
		"1280x960",
		NULL
};

enum ScreenSizeVal
{
		SC_320x200 = 0,
		SC_320x240,
		SC_640x400,
		SC_640x480,
		SC_800x500,
		SC_800x600,
		SC_1024x768,
		SC_1280x800,
		SC_1280x960
};

static const int ScreenSizeWidth[] = {
		320,
		320,
		640,
		640,
		800,
		800,
		1024,
		1280,
		1280
};

static const int ScreenSizeHeight[] = {
		200,
		240,
		400,
		480,
		500,
		600,
		768,
		800,
		960
};

static void OnSetScreenReso(AG_Event *event)
{
	int number = AG_INT(1);
	localconfig.uWidth = ScreenSizeWidth[number];
	localconfig.uHeight = ScreenSizeHeight[number];
}

static void ConfigMenuScreen(AG_NotebookTab *parent)
{
	AG_Radio *radio;
	AG_Checkbox *check;
	AG_Numerical *fps;

	radio = AG_RadioNewFn(AGWIDGET(parent), AG_RADIO_VFILL, ScreenSizeName, OnSetScreenReso, NULL);
	fps = AG_NumericalNewUint16(AGWIDGET(parent), AG_NUMERICAL_HFILL, gettext("Frames per Second") ,gettext("Frame rate"), &localconfig.nDrawFPS);
	AG_NumericalSetRangeInt(fps, 2, 75);
	check = AG_CheckboxNewInt(AGWIDGET(parent), AG_CHECKBOX_HFILL, gettext("Full Scan (15KHz)"), &localconfig.bFullScan);
}


static void OnConfigApply(AG_Event *event)
{
	int ver;
	AG_Button *self = (AG_Button *)AG_SELF();


	LockVM();
	memcpy(&configdat, &localconfig, sizeof(configdat_t));
	ver = fm7_ver;
	ApplyCfg();
	/*
	 * VMヴァージョンが違ったら強制リセット
	 */
	if(ver != fm7_ver){
		system_reset();
	}
	/*
	 * ここにアイコン変更入れる
	 */

	/*
	 * 終了処理
	 */
	UnlockVM();
	KeyBoardSnoop(FALSE);
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}


void OnConfigMenu(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_Notebook *note;
	AG_NotebookTab *tab;
	AG_Box *box;
	AG_Button *btn;

	memcpy(&localconfig, &configdat, sizeof	(configdat_t));

	win= AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(win, 320, 240);
    KeyBoardSnoop(TRUE);
    note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_EXPAND);
    {
    	/*
    	 * ここに、メニューの実体を置く
    	 */
    	tab = AG_NotebookAddTab(note, gettext("Emulation"), AG_BOX_HORIZ);
    	tab = AG_NotebookAddTab(note, gettext("Sound"), AG_BOX_HORIZ);
    	tab = AG_NotebookAddTab(note, gettext("Screen"), AG_BOX_HORIZ);
    	ConfigMenuScreen(tab);

    	tab = AG_NotebookAddTab(note, gettext("Keyboard"), AG_BOX_HORIZ);
    	tab = AG_NotebookAddTab(note, gettext("Joystick"), AG_BOX_HORIZ);
    	tab = AG_NotebookAddTab(note, gettext("Keyboard"), AG_BOX_HORIZ);
    	tab = AG_NotebookAddTab(note, gettext("Misc"), AG_BOX_HORIZ);

    	tab = AG_NotebookAddTab(note, gettext("Volume"), AG_BOX_HORIZ);

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
	AG_WindowSetCaption(win, gettext("Preferences"));
	AG_WindowShow(win);
}

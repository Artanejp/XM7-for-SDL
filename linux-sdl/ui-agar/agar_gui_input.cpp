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


extern void OnPushCancel2(AG_Event *event);

static const char *JsTypeName[] = {
		"None",
		"Port 1",
		"Port 2",
		"JoyKey",
		"Dempa",
		NULL
};


static void OnConfigApplyInput(AG_Event *event)
{
        int ver;
	AG_Button *self = (AG_Button *)AG_SELF();
	struct gui_input *cfg = AG_PTR(1);

	if(cfg == NULL) return;
	LockVM();
	{
	  int i, j;

	  for (i=0; i<256; i++) {
	    configdat.KeyMap[i].code = cfg->KeyMap[i].code;
	    if(cfg->KeyMap[i].code == -1) continue;
	    configdat.KeyMap[i].pushCode = cfg->KeyMap[i].pushCode;
	    configdat.KeyMap[i].mod = cfg->KeyMap[i].mod;
	  }
	  configdat.bKbdReal = cfg->bKbdReal;
	  configdat.bTenCursor = cfg->bTenCursor;
	  configdat.bArrow8Dir = cfg->bArrow8Dir;
	  for(i = 0;i < 2; i++) configdat.nJoyType[i] = cfg->nJoyType[i];
	  for(i = 0; i < 2; i++) {
	    for(j = 0; j < 2; j++) configdat.nJoyRapid[i][j] = cfg->nJoyRapid[i][j];
	  }
	  for(i = 0; i < 2; i++) {
	    for(j = 0; j < 7; j++) configdat.nJoyCode[i][j] = cfg->nJoyCode[i][j];
	  }
#ifdef MOUSE
	  configdat.bMouseCapture = cfg->bMouseCapture;
	  configdat.nMousePort = cfg->nMousePort;
	  configdat.nMidBtnMode = cfg->nMidBtnMode;
#endif
	  free(cfg);
	}
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

	if(self != NULL) {
	  AG_WindowHide(self->wid.window);
	  AG_ObjectDetach(self);
	}

}

static void OnChangeJsType(AG_Event *event)
{
	struct gui_input *cfg = AG_PTR(1);
	int number = AG_INT(2);
	int type = AG_INT(3);

	if(cfg == NULL) return;
	cfg->nJoyType[number] = type;
}

static void InputMenuJS(AG_NotebookTab *parent, struct gui_input *cfg)
{
	AG_Box *box;
	AG_Radio *radio;
	AG_Box *vbox;
	AG_Label *lbl;
	
	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_VFILL);
	{
	  char sep[32];
	  int i;
	  for(i = 0; i < 2; i++) {
		vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
		{
		  sprintf(sep, "Joystick #%d", i);
		  lbl = AG_LabelNew(AGWIDGET(vbox), 0, gettext(sep));
		  radio = AG_RadioNewFn(AGWIDGET(vbox), 0, JsTypeName, OnChangeJsType, "%p,%i", cfg, i);
		  AG_SetInt(radio, "value", cfg->nJoyType[i]);
		}
	  }
	}

}

extern void BuildVkeyBoard(AG_Event *event);
extern void SetKeyTable(AG_Event *event);

static void InputMenuKbd(AG_NotebookTab *parent, struct gui_input *cfg)
{
	AG_Box *box;
	AG_Checkbox *check;
	AG_Box *vbox;
	AG_Button *btn;

	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
	  check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Bind multi-pressed cursor as oblique direction"), &(cfg->bArrow8Dir));
	  check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Bind Cursor as Ten-Key"), &(cfg->bTenCursor));
	  check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Pseudo Realtime Scan"), &(cfg->bKbdReal));
	  btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Virtual Keyboard"), BuildVkeyBoard, "%p", cfg);
	  btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Assign Keyboard"), SetKeyTable, "%p", cfg);
	}
}

static void InputMenuMouse(AG_NotebookTab *parent, struct gui_input *cfg)
{
	AG_Box *box;
	AG_Checkbox *check;
	AG_Box *vbox;
	AG_Numerical *port;
#ifdef MOUSE
	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);


	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
	  check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, 
				    gettext("Connect Inteligent Mouse"), &(cfg->bMouseCapture));
	  port = AG_NumericalNewUintR(AGWIDGET(box), AG_NUMERICAL_HFILL, NULL ,
				      gettext("Connected port"), &(cfg->nMousePort),
				      1, 2);
	}
#endif
}

void OnConfigInputMenu(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_Notebook *note;

	AG_NotebookTab *tab;
	AG_Box *box;
	AG_Button *btn;
	struct gui_input *cfg;

	cfg = malloc(sizeof(struct gui_input));
	if(cfg == NULL) return;
	memset(cfg, 0x00, sizeof(struct gui_input));
	{
	  int i, j;

	  for (i=0; i<256; i++) {
	    if(configdat.KeyMap[i].code == -1) continue;
	    cfg->KeyMap[i].code = configdat.KeyMap[i].code;
	    cfg->KeyMap[i].pushCode = configdat.KeyMap[i].pushCode;
	    cfg->KeyMap[i].mod = configdat.KeyMap[i].mod;
	  }
	  cfg->bKbdReal = configdat.bKbdReal;
	  cfg->bTenCursor = configdat.bTenCursor;
	  cfg->bArrow8Dir = configdat.bArrow8Dir;
	  for(i = 0;i < 2; i++) cfg->nJoyType[i] = configdat.nJoyType[i];
	  for(i = 0; i < 2; i++) {
	    for(j = 0; j < 2; j++) cfg->nJoyRapid[i][j] = configdat.nJoyRapid[i][j];
	  }
	  for(i = 0; i < 2; i++) {
	    for(j = 0; j < 7; j++) cfg->nJoyCode[i][j] = configdat.nJoyCode[i][j];
	  }

#ifdef MOUSE
	  cfg->bMouseCapture = configdat.bMouseCapture;
	  cfg->nMousePort = configdat.nMousePort;
	  cfg->nMidBtnMode = configdat.nMidBtnMode;
#endif
	}

	win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
//	AG_WindowSetMinSize(win, 320, 240);
	note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_HFILL);
	{
    	/*
    	 * 
    	 */

	  tab = AG_NotebookAddTab(note, gettext("Keyboard"), AG_BOX_HORIZ);
	  InputMenuKbd(tab, cfg);

	  tab = AG_NotebookAddTab(note, gettext("Joystick"), AG_BOX_HORIZ);
	  InputMenuJS(tab, cfg);

#ifdef MOUSE
	  tab = AG_NotebookAddTab(note, gettext("Mouse"), AG_BOX_HORIZ);
	  InputMenuMouse(tab, cfg);
#endif
	}
	box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);
	AG_WidgetSetSize(AGWIDGET(box), 320, 24);
	{
	  AG_Box *vbox;
	  vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
	  btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnConfigApplyInput, "%p", cfg);
	  vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
	  AG_WidgetSetSize(AGWIDGET(vbox), 80, 24);
	  vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
	  btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Cancel"), OnPushCancel2, "%p", cfg);
	}

	AG_SetEvent(win, "window-close", OnPushCancel2, "%p", cfg);
	AG_WindowSetCaption(win, gettext("Configure Input"));
	AG_WindowShow(win);
}


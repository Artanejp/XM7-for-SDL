/*
 * agar_gui_menu.cpp
 *
 *  Agar: GUIパート : メニュー
 *  Created on: 2010/11/24
 *      Author: whatisthis
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

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#include "agar_cmd.h"
#else
#include "sdl.h"
#include "sdl_cfg.h"
#endif

//#include "sdl_bar.h"
//#include "api_kbd.h"
//#include "sdl_sch.h"
//#include "api_snd.h"
//#include "sdl_inifile.h"
#include "api_draw.h"
//#include "sdl_gtkdlg.h"
#include "agar_toolbox.h"

void Create_FileMenu(AG_MenuItem *self);
extern void Create_Drive0Menu(AG_MenuItem *self);
extern void Create_Drive1Menu(AG_MenuItem *self);
extern void Create_TapeMenu(AG_MenuItem *self);
extern void Create_ToolsMenu(AG_MenuItem *parent);
extern void Create_DebugMenu(AG_MenuItem *parent);
extern void Create_AboutMenu(AG_MenuItem *self);
extern void OnPushCancel(AG_Event *event);

extern "C" {
extern void InitInstance(void);
extern void OnDestroy(AG_Event *event);
extern void OnDestroy2(void);
AG_Window *MainWindow;
AG_Menu *MenuBar;
}


void RaiseMenu(AG_Event *event)
{
    if(MenuBar != NULL) {
        AG_WidgetFocus(AGWIDGET(MenuBar));
    }
}

void UnraiseMenu(AG_Event *event)
{
    if(MainWindow == NULL) return;
    AG_WindowFocus(MainWindow);
}

void AgarGuiMenuInit(AG_Widget *Parent)
{

}

static void SetBootMode(AG_Event *event)
{
	AG_Button *self = (AG_Button *)AG_SELF();
	int DosMode = AG_INT(1);

	if(DosMode) {
		// DOSモード
		OnDos(event);
	} else {
		OnBasic(event);
	}
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}

static void FileMenu_BootMode(AG_Event *event)
{
	AG_Menu *self = (AG_Menu *)AG_SELF();
	AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
	AG_Window *w;
	AG_Button   *btn[3];
	AG_Box *box;
	AG_Box *box2;
	char *label;
	AG_Label *lbl;

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 230, 80);
	label = gettext("Select mode (Notice: If select , reboot.)");
	AG_WindowSetMinSize(w, 230, 80);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s", label);
	box = AG_BoxNewVert(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);

	box2 = AG_BoxNewHoriz(box, 0);
	btn[0] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("BASIC"), SetBootMode, "%i", FALSE);
	box2 = AG_BoxNewVert(box, 0);
	btn[1] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("DOS"), SetBootMode, "%i", TRUE);
	box2 = AG_BoxNewVert(box, 0);
	btn[2] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("Cancel"), OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(w), "window-close", OnPushCancel, NULL);
	AG_WindowSetCaption(w, gettext("Boot Mode"));
	AG_WindowShow(w);

}

void Create_FileMenu(AG_MenuItem *parent)
{
	AG_MenuItem *item;

	item = AG_MenuAction(parent , gettext("Quick Save"), NULL, OnQuickSave, NULL);
	item = AG_MenuAction(parent , gettext("Quick Load"), NULL, OnQuickLoad, NULL);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent , gettext("Save As..."), NULL, OnSaveAs, NULL);
	item = AG_MenuAction(parent , gettext("Load"), NULL, OnLoadStatus, NULL);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent, gettext("Cold Reset"), NULL, OnReset, NULL);
	item = AG_MenuAction(parent, gettext("Hot Reset"), NULL, OnHotReset, NULL);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent, gettext("Boot Mode"), NULL, FileMenu_BootMode, NULL);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent , gettext("Quit"), NULL, OnDestroy, NULL);
}

void Create_AGMainBar(AG_Widget *Parent)
{
	AG_MenuItem *item;

//	MenuBar = AG_MenuNew(Parent, AG_MENU_HFILL);
//	MenuBar = AG_MenuNewGlobal(AG_MENU_HFILL);
	if(!MenuBar) return;

	AG_LockVFS(AGOBJECT(MenuBar));
	item = AG_MenuNode(MenuBar->root , "File", NULL);
	Create_FileMenu(item);

	item = AG_MenuNode(MenuBar->root, "Drive 1", NULL);
	Create_Drive1Menu(item);

	item = AG_MenuNode(MenuBar->root, "Drive 0", NULL);
	Create_Drive0Menu(item);

 	item = AG_MenuNode(MenuBar->root, "Tape", NULL);
	Create_TapeMenu(item);

 	item = AG_MenuNode(MenuBar->root, "Debug", NULL);
    Create_DebugMenu(item);

 	item = AG_MenuNode(MenuBar->root, "Tools", NULL);
 	Create_ToolsMenu(item);
 	item = AG_MenuNode(MenuBar->root, "Help", NULL);
 	item = AG_MenuNode(MenuBar->root, "About", NULL);
 	Create_AboutMenu(item);
	AG_UnlockVFS(AGOBJECT(MenuBar));
	// F12 -> Menu 閉じる
	AG_ActionFn(AGWIDGET(MenuBar), "close-menu", UnraiseMenu, NULL);
    AG_ActionOnKeyDown(AGWIDGET(MenuBar), AG_KEY_F12, AG_KEYMOD_NONE, "close-menu");

}


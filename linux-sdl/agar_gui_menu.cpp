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
#else
#include "sdl.h"
#include "sdl_cfg.h"
#endif

#include "sdl_bar.h"
#include "api_kbd.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_inifile.h"
#include "api_draw.h"
//#include "sdl_gtkdlg.h"
#include "agar_toolbox.h"

void Create_FileMenu(AG_MenuItem *self);
extern void Create_Drive0Menu(AG_MenuItem *self);
extern void Create_Drive1Menu(AG_MenuItem *self);
extern void Create_TapeMenu(AG_MenuItem *self);
extern void Create_ToolsMenu(AG_MenuItem *parent);

extern "C" {
extern void InitInstance(void);
extern void OnDestroy(AG_Event *event);
extern void OnDestroy2(void);
extern void InitGL(int w, int h);
extern void OnReset(AG_Event *event);
extern void OnHotReset(AG_Event *event);

AG_Window *MainWindow;
AG_Menu *MenuBar;
AG_GLView *DrawArea;
}



void AgarGuiMenuInit(AG_Widget *Parent)
{

}

static void FileMenu_BootMode(AG_Event *ev)
{

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
	//Menu_File_BootMode = AG_MenuAction(Menu_File, gettext("Boot Mode"), NULL, OnBootMode, NULL);
	//AG_MenuSeparator(Menu_File);
	item = AG_MenuAction(parent , gettext("Quit"), NULL, OnDestroy, NULL);
}




void Create_AGMainBar(AG_Widget *Parent)
{
	AG_MenuItem *item;

	MenuBar = AG_MenuNew(Parent, AG_MENU_HFILL);
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
 	item = AG_MenuNode(MenuBar->root, "Tools", NULL);
 	Create_ToolsMenu(item);
 	item = AG_MenuNode(MenuBar->root, "Help", NULL);
 	item = AG_MenuNode(MenuBar->root, "About", NULL);
	AG_UnlockVFS(AGOBJECT(MenuBar));
}



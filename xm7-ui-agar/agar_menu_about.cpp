/*
 * agar_menu_about.cpp
 *
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
#include "tapelp.h"


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
#include "agar_cmd.h"

extern void OnPushCancel(AG_Event *event);
extern void KeyBoardSnoop(BOOL t);


static void Dialog_OnPushOK(AG_Event *event)
{
    AG_Button *button = (AG_Button *)AG_SELF();
    AG_Surface *mark = (AG_Surface *)AG_PTR(1);
    AG_WindowHide(button->wid.window);
    AG_ObjectDetach(button->wid.window);
}

void OnAboutDialog(AG_Event *event)
{

	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_HBox  *hbox;
	AG_VBox *vbox1, *vbox2;
	AG_Button *btn;
	AG_Label *label;
	AG_Surface *mark = NULL;

	char string[256];
	char iconpath[1024];
	char file[MAXPATHLEN];

	win = AG_WindowNew(0);
	if(win == NULL) return;
	AG_WindowSetCaption(win, "%s", gettext("About XM7/SDL"));

	hbox = AG_HBoxNew(win, AG_HBOX_HFILL);

	iconpath[0] = '\0'; //あとでConfigから読めるようにする
#ifdef RSSDIR
	strcat(iconpath, RSSDIR);
#else
	strcat(path, "./.xm7/");
#endif
	strcpy(file, iconpath); // 複数ディレクトリサーチに置き換える
	strcat(file, "xm7.png"); // アイコンはPNGで…
	if(AG_FileExists(file)) {
		mark = AG_SurfaceFromPNG(file);
		if(mark != NULL) {
			vbox1 = AG_VBoxNew(hbox, AG_VBOX_VFILL);
			AG_PixmapFromSurface(vbox1, 0, mark);
		}
	}

	vbox2 = AG_VBoxNew(hbox, AG_VBOX_VFILL);
	label = AG_LabelNew(vbox2, 0, "FM-7/77AV/SX Emulateor \"XM7\"""\n "VERSTR);
	AG_LabelJustify(label, AG_TEXT_RIGHT);


	hbox = AG_HBoxNew(win, AG_HBOX_HFILL);
	AG_LabelNew(hbox, 0, AUTSTR);

	btn = AG_ButtonNewFn (AGWIDGET(win), 0, gettext("OK"), Dialog_OnPushOK, "%p", (void *)mark);

	AG_WindowShow(win);
}

void Create_AboutMenu(AG_MenuItem *self)
{
	AG_MenuItem *item;
	item = AG_MenuAction(self, gettext("About XM7"), NULL, OnAboutDialog, NULL);
}


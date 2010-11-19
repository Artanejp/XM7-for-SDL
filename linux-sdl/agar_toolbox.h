/*
 * agar_toolbox.h
 *
 *  Created on: 2010/11/09
 *      Author: whatisthis
 */



#ifndef AGAR_TOOLBOX_H_
#define AGAR_TOOLBOX_H_

#include <agar/core.h>
#include <agar/gui.h>

#ifdef __cplusplus
extern          "C" {
#endif				/*  */

extern AG_Window *MainWindow;
extern AG_Menu *ToolBarMenu;
extern AG_GLView *DrawArea;
#ifdef __cplusplus
}
#endif				/*  */

#define DLG_OK 1					/* ダイアログＯＫ */
#define DLG_CANCEL 2				/* ダイアログＣＡＮＣＥＬ */
#define DLG_NONE 0					/* ダイアログ未確定 */

#ifdef USE_AGAR
		extern void StateLoad(char *path);
        extern void OnLoadStatus(AG_Event *);
        extern void OnQuickLoad(AG_Event *);
        extern void OnSaveAs(AG_Event *);
        extern void OnQuickSave(AG_Event *);
        extern void OnDiskPopup(AG_Event *);
#endif

#ifdef USE_GTK

        extern void StateLoad(char *path);
        extern void OnLoadStatus(void);
        extern void OnQuickLoad(char *s);
        extern void OnSaveAs(void);
        extern void OnQuickSave(char *s);
        extern void OnDiskPopup(AG_Event *event);
#if 0
        extern void    OnDiskOpen(GtkWidget * widget, gpointer data);
        extern void    OnDiskBoth(GtkWidget * widget, gpointer data);
        extern void    CreateFileMenu(GtkBuilder *gbuilder);
#endif
        extern void    CreateDiskMenu_0 (void);
        extern void    CreateDiskMenu_1 (void);
#if 0
        extern void    OnTapePopup(GtkWidget * widget, gpointer data);
        extern void    OnTapeOpen(GtkWidget * widget, gpointer data);
        extern void    CreateTapeMenu(GtkBuilder *gbuilder);
        extern void    CreateHelpMenu(GtkWidget * widget, gpointer data);
        extern void    OnVersion(GtkWidget * widget, gpointer data);
        extern void    OnDebugPopup(GtkWidget * widget, gpointer data);
        extern void    CreateDebugmenu(GtkWidget *menu_bar, GtkAccelGroup *accel_group);
        extern void    OnToolPopup(GtkWidget * widget, gpointer data);
#endif
        extern void    CreateMenu(void);

#endif

//#ifdef __cplusplus
//}
//#endif				/*  */


#endif /* AGAR_TOOLBOX_H_ */

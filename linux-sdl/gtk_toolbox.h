/*
 *   FM-7 EMULATOR "XM7"
 *   Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp) 
 *   Copyright (C) 2001-2003 Ryu Takegami 
 *   Copyright (C) 2004 GIMONS
 *   Copyright (C) 2010 K.Ohta (whatisthis.sowhat@gmail.com) 
 *   [GTK Toolbox ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_cmd_h_
#define _xw_cmd_h_
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
    
#ifdef USE_GTK

        extern void    StateLoad(char *path);
        extern void    OnDiskOpen(GtkWidget * widget, gpointer data);
        extern void    OnDiskBoth(GtkWidget * widget, gpointer data);
        extern void    CreateFileMenu(GtkBuilder *gbuilder); 
        extern void    CreateDiskMenu_0 (GtkBuilder* gbuilder);
        extern void    CreateDiskMenu_1 (GtkBuilder* gbuilder);
        extern void    OnTapePopup(GtkWidget * widget, gpointer data); 
        extern void    OnTapeOpen(GtkWidget * widget, gpointer data);
        extern void    CreateTapeMenu(GtkBuilder *gbuilder);  
        extern void    CreateHelpMenu(GtkWidget * widget, gpointer data); 
        extern void    OnVersion(GtkWidget * widget, gpointer data); 
        extern void    OnDebugPopup(GtkWidget * widget, gpointer data); 
        extern void    CreateDebugmenu(GtkWidget *menu_bar, GtkAccelGroup *accel_group); 
        extern void    OnToolPopup(GtkWidget * widget, gpointer data);
        extern void    CreateMenu(GtkWidget *parent);

#endif

#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_cmd_h_ */
#endif	/* _XWIN */

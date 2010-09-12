/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta  
 * [ XWIN/SDLキーボード設定プログラム - GTK+パート] 
 */  
    

    
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/param.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
    
#include "xm7.h"
#ifdef USE_GTK

#include "mouse.h"
#include "tapelp.h"
#include "keyboard.h"
#include "sdl.h"
#include "sdl_bar.h"
#include "sdl_draw.h"
#include "sdl_kbd.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_cfg.h"
#include "sdl_inifile.h"
#include "gtk_propkeyboard.h"

// extern BYTE kbd_map[256];

GtkWidget *ksetProperty;

static struct local_sdlkeymap local_kbd_map[256];
static BYTE local_target_code; /* 変更対象のキーコード */
#ifdef USE_GTK
static GtkWidget *getwindow;
static GtkWidget *keyMapWindow;
static GtkWidget *vbox1;
//static GtkWidget *vbox11;
#endif /* USE_GTK */

static BOOL 
SetKeyCode(Uint16 keysym, BYTE code)
{
        int i;
        i = 0;
        do{
                if(local_kbd_map[i].code == code) {
                        local_kbd_map[i].keysym = keysym;
                        return TRUE;
                }
                if(local_kbd_map[i].keysym == 0xffff) {
                        /* キーテーブルにターゲットのKeySymがない */
                        return FALSE;
                }
                i++;
        } while(i<0x100);
        return FALSE;
}        

/*
 *
 */
BOOL 
SnoopedOnKeyPressedCallback(SDL_Event *event)
{
        Uint16 code = (Uint16)event->key.keysym.sym;
        char stmp[64];

        printf("Snoop Key: %03x\n",code);
        sprintf(stmp, "%04x", code);
        gtk_label_set_text(GTK_LABEL(vbox1), stmp);
        SetKeyCode(code, local_target_code); 
        return TRUE;
}




/*
 * キーイベントを（見た目)snoopする
 */

static void
SnoopKeyEvent(BOOL flag)
{
        if(flag == TRUE) {
                kbd_snooped = TRUE;
        } else {
                kbd_snooped = FALSE;
        }
}

static void 
KbdSetOnClicked(GtkWidget *widget, gpointer data)
{

        int i;
        SnoopKeyEvent(FALSE);
        do{
                kbd_table[i].keysym = local_kbd_map[i].keysym;
                kbd_table[i].code = local_kbd_map[i].code;
                
                if(local_kbd_map[i].keysym == 0xffff) {
                        /* キーテーブルにターゲットのKeySymがない */
                        kbd_table[i].keysym = 0xffff;
                        kbd_table[i].code = 0xff;
                        return;
                }
                i++;
        } while(i < 256);
        if(i < 256) return;
        kbd_table[i].keysym = 0xffff;
        kbd_table[i].code = 0xff;

}
                


/*
 * コードを取り込み始める。
 */

void 
//StartGetKeycodeForProp(Uint16 keysym, BYTE code)
StartGetKeycodeForProp(GtkWidget *widget, gpointer data)
{
        
        int i;
        BYTE stmp[64];
        BYTE code = 0x30;

        
        for(i = 0; i<256 ; i++) {
                if(kbd_table[i].keysym == 0xffff) {
                        break;
                }
                local_kbd_map[i].keysym = kbd_table[i].keysym;
                local_kbd_map[i].code = kbd_table[i].code;
        }
        local_kbd_map[i].keysym = 0xffff;
        local_kbd_map[i].code = 0xff;
        local_target_code = code;

        /*
         * ここにWindow作る
         */
#ifdef USE_GTK
        getwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        sprintf(stmp, "Set KeyCode 0x%02x, then CLICK RIGHT.", code);
        gtk_window_set_title(GTK_WINDOW(getwindow), stmp);
        gtk_window_set_position(GTK_WINDOW(getwindow), GTK_WIN_POS_CENTER);
        gtk_window_set_modal(GTK_WINDOW(getwindow), TRUE);

        sprintf(stmp, "%04x", code);
        vbox1 = gtk_label_new(stmp);
        //vbox1 = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(vbox1);
        gtk_container_add(GTK_CONTAINER(getwindow), vbox1);
        local_target_code = code;
        SnoopKeyEvent(TRUE);
        /*
         * 入力領域は120x120(仮)
         */
        gtk_widget_set_usize(vbox1, 120, 120);
        gtk_signal_connect((gpointer)vbox1, "clicked", 
                           GTK_SIGNAL_FUNC(KbdSetOnClicked), NULL);
        g_signal_connect(getwindow, "destroy",
                         GTK_SIGNAL_FUNC(gtk_widget_destroy), NULL);
        gtk_signal_connect_object((gpointer) vbox1, "clicked",
                                  GTK_SIGNAL_FUNC(gtk_widget_destroy),
                                  (gpointer) getwindow);
#endif

        return;
}

static GtkWidget *ShowKeyMap(void);

void
OnClick_KeyMap(GtkWidget *widget, gpointer data)
{
        ksetProperty = ShowKeyMap();
        gtk_widget_show(ksetProperty);
}

static GtkWidget
*ShowKeyMap(void)
{
        int i;
        GtkWidget   *keyWindow;
        GtkWidget   *btnOk, *btnCancel;
        GtkBuilder  *key; 
        GObject     *keymap;

        for(i = 0; i<256 ; i++) {
                if(kbd_table[i].keysym == 0xffff) {
                        break;
                }
                local_kbd_map[i].keysym = kbd_table[i].keysym;
                local_kbd_map[i].code = kbd_table[i].code;
        }
//        key = gtk_builder_new();
        keymap = gtk_builder_get_object(gbuilderMain, "dialog_keycode");
        
        keyWindow = GTK_WIDGET(keymap);
        //g_object_set_data(G_OBJECT(keyWindow),  "window_keymap", (gpointer) keymap);
        //keyWindow = GTK_WIDGET(keymap);
        gtk_window_set_title(GTK_WINDOW(keyWindow), "KEYMAP");
        gtk_window_set_position(GTK_WINDOW(keyWindow), GTK_WIN_POS_CENTER);
        gtk_window_set_modal(GTK_WINDOW(keyWindow), TRUE);
        gtk_window_set_deletable(GTK_WINDOW(keyWindow), FALSE);

        btnOk = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_Key_OK"));
        btnCancel = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "button_Key_Cancel"));

/*
 * OK or キャンセル...共通ボタン
 */

        g_signal_connect(btnOk, "clicked",
                         GTK_SIGNAL_FUNC(OnCancelPressed),
                         (gpointer)keyWindow);

        g_signal_connect(btnCancel, "clicked",
                         GTK_SIGNAL_FUNC(OnCancelPressed),
                         (gpointer)keyWindow);

        g_signal_connect(keyWindow, "destroy",
                         GTK_SIGNAL_FUNC(OnCancelPressed),
                         (gpointer)keyWindow);

        g_signal_connect(keyWindow, "remove",
                         GTK_SIGNAL_FUNC(OnCancelPressed),
                         (gpointer)keyWindow);

        g_signal_connect(keyWindow, "delete_event",
                         GTK_SIGNAL_FUNC(OnCancelPressed),
                         (gpointer)keyWindow);

//
//        g_signal_connect(keyWindow, "close",
//                         GTK_SIGNAL_FUNC(OnCancelPressed),
//                         (gpointer)keyWindow);

        //g_signal_connect(btnCancel, "clicked",
        //                 GTK_SIGNAL_FUNC(OnCancelPressed),
        //                 (gpointer)keyWindow);

        //       g_signal_connect(btnOk, "clicked",
        //                 GTK_SIGNAL_FUNC(OnCancelPressed),
        //                 (gpointer)keyWindow);

//        gtk_widget_show(keyWindow);
        return keyWindow;


}

void ListKeyMap(struct local_sdlkeymap *pMap)
{



        keyMapWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(keyMapWindow), "キーマップ");
        gtk_window_set_position(GTK_WINDOW(keyMapWindow), GTK_WIN_POS_CENTER);
        gtk_window_set_modal(GTK_WINDOW(keyMapWindow), TRUE);
}

        
#endif /* USE_GTK */

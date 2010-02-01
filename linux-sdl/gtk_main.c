/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta  
 * [ XWIN/SDLメインプログラム - GTK+パート] 
 */  
    
#ifdef USE_GTK
    
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/param.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
    
#include "xm7.h"
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

GtkWidget       *wndMain;		/* メインウィンドウ */
GtkWidget       *gtkDrawArea;

/*
 * ドロー領域の生成 GTKパート
 */

void
CreateDrawGTK(GtkWidget * parent) 
{
    GtkWidget * hbox;
    
    /*
     * ドローウィンドウの土台を生成 
     */ 
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(parent), hbox, TRUE, TRUE, 0);
    gtk_widget_show(hbox);
    
    /*
     * スクリーン描画領域の生成 
     */ 
	// gtkDrawArea = gtk_drawing_area_new();
	// gtk_widget_set_events(gtkDrawArea,
	// GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
    gtkDrawArea = gtk_socket_new();
    gtk_widget_set_usize(gtkDrawArea, 640, 400);
    gtk_container_add(GTK_CONTAINER(parent), gtkDrawArea);
    
    /*
     * すべて表示 
     */ 
    gtk_widget_show(gtkDrawArea);
} 

/*
 *  キーボード：GTK->SDL Wrapper 
 */ 
static gboolean FASTCALL
OnKeyPressGTK(GtkWidget * widget, GdkEventKey * event, gpointer data) 
{
    Uint8 keycode = (Uint8) event->hardware_keycode;
    SDL_Event sdlevent;
    sdlevent.type = SDL_KEYDOWN;
    sdlevent.key.type = SDL_KEYDOWN;
    sdlevent.key.state = SDL_PRESSED;
    sdlevent.key.keysym.scancode = keycode;
    SDL_PushEvent(&sdlevent);
    return TRUE;
}

static gboolean FASTCALL
OnKeyReleaseGTK(GtkWidget * widget, GdkEventKey * event, gpointer data) 
{
    Uint8 keycode = (Uint8) event->hardware_keycode;
    SDL_Event sdlevent;
    sdlevent.type = SDL_KEYUP;
    sdlevent.key.type = SDL_KEYUP;
    sdlevent.key.state = SDL_RELEASED;
    sdlevent.key.keysym.scancode = keycode;
    SDL_PushEvent(&sdlevent);
    return TRUE;
}

/*-[ メインウインドウ ]-----------------------------------------------------*/ 


/*
 * GTK:
 * スクリーン解像度変更の時にはSDLを終了→初期化→再生成する。
 */ 
static void
OnScreenPlugged(void) 
{ 
        SDL_SysWMinfo sdlinfo;
        char          EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
    
/*
 * SDL初期化 
 */ 
        if ((drawArea == NULL) || (displayArea == NULL)) {
                sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
                        gdk_x11_drawable_get_xid(gtkDrawArea->window));
        
                // sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
                // gdk_x11_drawable_get_xid(wndMain->window));
                SDL_putenv(EnvMainWindow);
                SDL_InitSubSystem(SDL_INIT_VIDEO);
                CreateDrawSDL();
        }
}

static void
OnScreenUnPlugged(void) 
{
        SDL_SysWMinfo sdlinfo;
  
/*
  * SDL-VIDEO終了 
  */ 
// sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
        // gdk_x11_drawable_get_xid(wndMain->window));
        if (displayArea) {
                SDL_FreeSurface(displayArea);
                displayArea = NULL;
        }
        if (drawArea) {
                SDL_FreeSurface(drawArea);
                drawArea = NULL;
        }
}

    /*
     *  ウインドウデリート 
     */ 
static gboolean FASTCALL
OnDelete(GtkWidget * widget, GdkEvent * event, gpointer data) 
{
    
/*
 * フラグアップ 
 */ 
	LockVM();
    bCloseReq = TRUE;
    UnlockVM();
    return FALSE;
}


    /*
     *  ウインドウ削除 
     */ 
static void     FASTCALL
OnDestroy(GtkWidget * widget, gpointer data) 
{
    
	/*
	 * サウンド停止 
	 */ 
	StopSnd();
    
	/*
	 * コンポーネント クリーンアップ 
	 */ 
#ifdef FDDSND
	CleanFDDSnd();
    
#endif				/*  */
	CleanSch();
    CleanKbd();
    CleanSnd();
    CleanDraw();
    SaveCfg();
    
	/*
	 * 仮想マシン クリーンアップ 
	 */ 
	system_cleanup();
    gtk_main_quit();
} 
/*-[ アプリケーション ]-----------------------------------------------------*/ 
    
    /*
     *  ドローウインドウへのフォーカスインイベント 
     */ 
static void     FASTCALL
OnFocusIn(GtkWidget * widget, gpointer data)
{
    bActivate = TRUE;
    
#ifdef MOUSE
	SetMouseCapture(bActivate);
    
#endif				/*  */
} 
    /*
     * 
     * ドローウインドウへのフォーカスアウトイベント 
     */ 
static void     FASTCALL
OnFocusOut(GtkWidget * widget, gpointer data)
{
    bActivate = FALSE;
    
#ifdef MOUSE
	SetMouseCapture(bActivate);
    
#endif				/*  */
} 


/*
 *  インスタンス初期化 
 */ 
void     
InitInstanceGtk(void) 
{
    GtkWidget   *vbox;
    GError      *error = NULL;
    char        EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
    char        icon_path[MAXPATHLEN];
    SDL_SysWMinfo sdlinfo;
    
/*
 * ウィンドウ生成 
 */ 
    wndMain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(wndMain), "XM7");
    gtk_container_border_width(GTK_CONTAINER(wndMain), 0);
    gtk_window_set_resizable(GTK_WINDOW(wndMain), FALSE);
    gtk_window_set_position(GTK_WINDOW(wndMain), GTK_WIN_POS_CENTER);
    
/*
 * 土台の土台となる垂直ボックス 
 */ 
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(wndMain), vbox);
    gtk_widget_show(vbox);
    gtk_signal_connect(GTK_OBJECT(wndMain), "delete-event",
		 GTK_SIGNAL_FUNC(OnDelete), NULL);
    gtk_signal_connect(GTK_OBJECT(wndMain), "destroy",
		 GTK_SIGNAL_FUNC(OnDestroy), NULL);
    gtk_signal_connect(GTK_OBJECT(wndMain), "focus-in-event",
		 GTK_SIGNAL_FUNC(OnFocusIn), NULL);
    gtk_signal_connect(GTK_OBJECT(wndMain), "focus-out-event",
		 GTK_SIGNAL_FUNC(OnFocusOut), NULL);
    gtk_signal_connect(GTK_OBJECT(wndMain), "key-press-event",
		 GTK_SIGNAL_FUNC(OnKeyPressGtk), NULL);
    gtk_signal_connect(GTK_OBJECT(wndMain), "key-release-event",
		 GTK_SIGNAL_FUNC(OnKeyReleaseGtk), NULL);
    gtk_signal_connect(GTK_OBJECT(wndMain), "plug-added",
		 GTK_SIGNAL_FUNC(OnScreenPlugged), NULL);
    gtk_signal_connect(GTK_OBJECT(wndMain), "plug-removed",
		GTK_SIGNAL_FUNC(OnScreenUnPlugged), NULL);
    
// g_idle_add_full(G_PRIORITY_HIGH, &GtkEventHandler, NULL,
// NULL);
    gtk_widget_show(wndMain);
    
/*
 * イベント 
 */ 
    OnCreate(vbox);
    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_TIMER);
    SDL_GetWMInfo(&sdlinfo);
    gtk_socket_add_id(GTK_SOCKET(gtkDrawArea), sdlinfo.info.x11.window);
    
    sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
            gdk_x11_drawable_get_xid(gtkDrawArea->window));
    
    // sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
    // gdk_x11_drawable_get_xid(wndMain->window));
    SDL_putenv(EnvMainWindow);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    
    CreateDrawSDL();
    strcpy(icon_path, ModuleDir);
    SetIconGtk(fm7_ver);        


    return;
}

/*
 * Verに応じたアイコンをセットする
 */
void 
SetIconGtk(int ver)
{
    char        icon_path[MAXPATHLEN];
    strcpy(icon_path, ModuleDir);
    
    switch (fm7_ver) {
    case 1:
	strcat(icon_path, "resource/tamori.ico");
	break;
    case 2:
	strcat(icon_path, "resource/app_av.ico");
	break;
    case 3:
	strcat(icon_path, "resource/app_ex.ico");
	break;
    default:
	icon_path[0] = '\0';
    }
    if (icon_path[0] != '\0' && strcmp(icon_path, ModuleDir) != 0)
	gtk_window_set_icon_from_file(GTK_WINDOW(wndMain), icon_path,
                                      (GError **) NULL);
}

/*
 * UIのアップデート
 */
void
ui_update_gtk(void)
{
	while (gtk_events_pending())
	gtk_main_iteration_do(FALSE);
}


void 
InitGtk(int argc, char *argv[])
{
/*
 * スレッドの初期化 
 */ 
    g_thread_init(NULL);
    gdk_threads_init();
    gtk_set_locale();
    gtk_init(&argc, &argv);
}

void 
MainLoopGtk(int argc, char *argv[])
{
/*
 * エラーコード別 
 */ 
        switch (nErrorCode) {
                /*
                 * エラーなし 
                 */ 
        case 0:
                /*
                 * 実行開始 
                 */ 
                stopreq_flag = FALSE;
                run_flag = TRUE;
                /*
                 * コマンドライン処理 
                 */ 
                if (argc > 1) {
                        OnCmdLine(argv[1]);
	}
	break;
        /*
         * VM初期化エラー 
         */ 
    case 1:
	gtk_widget_hide(wndMain);
	OpenErrorMessageDialog("XM7", "仮想マシンを初期化できません");
	break;
	
    /*
     * コンポーネント初期化エラー 
     */ 
    case 2:
	gtk_widget_hide(wndMain);
	OpenErrorMessageDialog("XM7","コンポーネントを初期化できません");
	break;
    }
    atexit(SDL_Quit);
    
   /*
    * GTK のメインループに入る 
    */ 
    gtk_main();
}

#endif /* USE_GTK */

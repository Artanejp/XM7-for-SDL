/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta  
 * [ XWIN/SDLメインプログラム - GTK+パート] 
 */  
    

    
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <sys/param.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <SDL/SDL_opengl.h>

#include "xm7.h"
#include "mouse.h"
#include "tapelp.h"
#include "keyboard.h"
#include "display.h"
#include "sdl.h"
#include "sdl_bar.h"
#include "sdl_draw.h"
#include "api_kbd.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_cfg.h"
#include "sdl_inifile.h"

#ifdef USE_GTK
GtkWidget       *wndMain;		/* メインウィンドウ */
GtkWidget       *gtkDrawArea;
GtkBuilder      *gbuilderMain;
static BOOL wasInit;

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
    
    /*
     * スクリーン描画領域の生成 
     */ 
    hbox = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_drawing"));
    gtkDrawArea = gtk_socket_new();
    gtk_widget_set_size_request(hbox, 640, 520);
    gtk_widget_set_size_request(gtkDrawArea, 1280, 880);

    gtk_container_add(GTK_CONTAINER(hbox), gtkDrawArea);
    
    /*
     * すべて表示 
     */ 
    gtk_widget_show(hbox);
    gtk_widget_show(gtkDrawArea);
    wasInit = FALSE;
} 

/*
 *  キーボード：GTK->SDL Wrapper 
 */ 
static Uint16
ConvertKcodeGtk2SDL(guint kcode)
{
	Uint16 k = 0;
	if((kcode >=GDK_BackSpace) && (kcode <= GDK_Escape)){
		k = kcode - 0xff00;
	} else if((kcode >= GDK_at) && (kcode <= GDK_asciitilde)){
		k = gdk_keyval_to_lower(kcode);
	} else if((kcode >= GDK_exclam ) && (kcode <= GDK_apostrophe)) {
		k = kcode - GDK_exclam + SDLK_0; // "!" - "'"
	} else if((kcode >= GDK_parenleft)&&(kcode <= GDK_parenright)) {
		k = kcode - GDK_parenleft + SDLK_8;
	}
}

static gboolean
OnKeyPressGTK_S(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
    guint keycode =  event->keyval;
    Uint32 k = gdk_keyval_to_unicode(keycode);
    SDLKey sym;
    SDLMod mod;
    printf("KEY Push %s\n",gdk_keyval_name(keycode));
    SDL_Event sdlevent;

//    sym = ConvertKcodeGtk2SDL(keycode);
    sym = event->hardware_keycode;
    sdlevent.type = SDL_KEYDOWN;
    sdlevent.key.type = SDL_KEYDOWN;
    sdlevent.key.state = SDL_PRESSED;
    sdlevent.key.keysym.mod = mod;
    sdlevent.key.keysym.sym = sym;
    sdlevent.key.keysym.unicode = k;
    SDL_PushEvent(&sdlevent);
    return TRUE;
}

static gboolean
OnKeyReleaseGTK_S(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
    guint keycode =  event->keyval;
    Uint32 k = gdk_keyval_to_unicode(keycode);
    SDL_Event sdlevent;
    SDLKey sym;
    SDLMod mod;

//    sym = ConvertKcodeGtk2SDL(keycode);
    sym = event->hardware_keycode;
     sdlevent.type = SDL_KEYUP;
    sdlevent.key.type = SDL_KEYUP;
    sdlevent.key.state = SDL_RELEASED;
    sdlevent.key.keysym.mod = mod;
    sdlevent.key.keysym.sym = sym;
    sdlevent.key.keysym.unicode = k;
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
//        SDL_SysWMinfo sdlinfo;
        char          EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
    
/*
 * SDL初期化 
 */ 
        if ((drawArea == NULL) || (displayArea == NULL)) {
                sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
                        gdk_x11_drawable_get_xid(gtkDrawArea->window));
        
                SDL_putenv(EnvMainWindow);
                SDL_InitSubSystem(SDL_INIT_VIDEO);
                CreateDrawSDL();
        }
}

static void
OnScreenUnPlugged(void) 
{
//        SDL_SysWMinfo sdlinfo;
  
/*
  * SDL-VIDEO終了 
  */ 
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
static gboolean
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
void
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
static void 
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
static void
OnFocusOut(GtkWidget * widget, gpointer data)
{
    bActivate = FALSE;
    
#ifdef MOUSE
	SetMouseCapture(bActivate);
    
#endif				/*  */
} 

/*
 * 表示解像度を変更する
 */
extern void InitGL(int w, int h);

void 
ChangeResolutionGTK(int width, int height, int oldwidth, int oldheight)
{       
        char            EnvMainWindow[64];
        int             tmpHeight, tmpWidth;
        GtkWidget        *hbox;
        SDL_Surface     *tmpSurface;
        SDL_Rect        srcrect, dstrect;
        SDL_SysWMinfo        sdlinfo;
        int rgb_size[4];
        int tmpHeight2, tmpWidth2;

/*
 * まずは現在のサーフェイスを退避する 
 */ 
#ifndef USE_OPENGL
        tmpSurface =
                SDL_CreateRGBSurface(SDL_SWSURFACE, oldwidth,
                                     oldheight, 24, 0, 0, 0, 0);
        displayArea = SDL_GetVideoSurface();
        srcrect.x = 0;
        srcrect.y = 0;
        srcrect.w = displayArea->w;
        srcrect.h = displayArea->h;
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = tmpSurface->w;
        dstrect.h = tmpSurface->h;
        if (srcrect.w > dstrect.w)
                srcrect.w = dstrect.w;
        if (srcrect.h > dstrect.h)
                srcrect.h = dstrect.h;
        SDL_BlitSurface(displayArea, &srcrect, tmpSurface, &dstrect);
        /*
         * 1度サーフェースを削除する(クラッシュ対策)
         */
#endif  
        if(displayArea != NULL) {
	   SDL_FreeSurface(displayArea);
	           displayArea = NULL;
	}


/*
 * 表示部分のリサイズ : GTK依存部分につき変更？
 */ 
#ifdef USE_GTK
        switch(height) {
        case 240:
                tmpHeight = height + 40;
	   tmpHeight2 = 480;
                break;
        default:
        case 400:
        		tmpHeight = 480;
	   tmpHeight2 = 480;
        		break;
        case 800:
                tmpHeight = height + 40;
                tmpHeight2 = 960;
                break;
        }
        hbox = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_drawing"));
        gtk_widget_set_size_request(hbox, width, tmpHeight2);
        SDL_Delay(100);
        gtk_widget_set_size_request(gtkDrawArea, width, tmpHeight2);


        if((gtkDrawArea != NULL) && (gtkDrawArea->window != NULL)) {
                        sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
                                gdk_x11_drawable_get_xid(gtkDrawArea->window));
                        printf("RESO CHG: %d x %d -> %d x %d\n", oldwidth,
                               oldheight, width, height);
	    
#endif				/*  */
                        SDL_putenv(EnvMainWindow);

#if XM7_VER >= 3
#ifndef USE_OPENGL
                        switch(screen_mode) {
                        case SCR_400LINE:
                        case SCR_200LINE:
                        displayArea =
                                SDL_SetVideoMode(width, tmpHeight, 32,
                                                 SDL_HWSURFACE | SDL_ANYFORMAT |
                                                 SDL_RESIZABLE | SDL_DOUBLEBUF |
                                                 SDL_ASYNCBLIT | SDL_HWPALETTE |
                                                 0);

                        break;
                        case SCR_4096:
                        displayArea =
                                SDL_SetVideoMode(width, tmpHeight, 32,
                                                 SDL_HWSURFACE | SDL_ANYFORMAT | 
                                                 SDL_RESIZABLE | SDL_DOUBLEBUF |
                                                 SDL_ASYNCBLIT | SDL_HWPALETTE |
                                                 0);
                        break;
                        case SCR_262144:
                        default:
                        displayArea =
                                SDL_SetVideoMode(width, tmpHeight, 32,
                                                 SDL_HWSURFACE | SDL_ANYFORMAT | 
                                                 SDL_RESIZABLE | SDL_DOUBLEBUF |
                                                 SDL_ASYNCBLIT | SDL_HWPALETTE |
                                                 0);
                        break;
                        }
#else
                        InitGL(width, tmpHeight2);
#endif
#else
                        if(mode320) {
                                displayArea =
                                        SDL_SetVideoMode(width, tmpHeight, 24,
                                                 SDL_HWSURFACE | SDL_ANYFORMAT |
                                                 SDL_RESIZABLE | SDL_DOUBLEBUF |
                                                 SDL_ASYNCBLIT | SDL_HWPALETTE | 0);
                        } else {
                                displayArea = 
                                        SDL_SetVideoMode(width, tmpHeight, 16,
                                                 SDL_HWSURFACE | SDL_ANYFORMAT |
                                                 SDL_RESIZABLE | SDL_DOUBLEBUF |
                                                 SDL_ASYNCBLIT | SDL_HWPALETTE | 0);
                        }
#endif                        
                        /*
                         * Debuhg Code
                         */
//                        vinfo = SDL_GetVideoInfo();
//                        printf("DBG:SCREEN: Changed to BPP %d HW %d HW CC %d\n", displayArea->format->BitsPerPixel, vinfo->hw_available, vinfo->blit_hw_CC);

                } else {
                        displayArea = SDL_GetVideoSurface();
                }
                	    
/*
 * 退避したエリアの復帰（原寸…) 
 */ 
#ifndef USE_OPENGL
        dstrect.x = 0;
        dstrect.y = 0;
        dstrect.w = displayArea->w;
        dstrect.h = displayArea->h;
        srcrect.x = 0;
        srcrect.y = 0;
        srcrect.w = tmpSurface->w;
        srcrect.h = tmpSurface->h;
        if (srcrect.w > dstrect.w)
                srcrect.w = dstrect.w;
        if (srcrect.h > dstrect.h)
                srcrect.h = dstrect.h;
        SDL_BlitSurface(tmpSurface, &srcrect, displayArea, &dstrect);
        SDL_FreeSurface(tmpSurface);
#endif
/*
 * ステータス表示
 */
        wasInit = TRUE;

/*
 * 以下に、全画面強制再描画処理を入れる 
 */
#ifndef USE_OPENGL
        SDL_GetWMInfo(&sdlinfo);
        gtk_socket_add_id(GTK_SOCKET(gtkDrawArea), sdlinfo.info.x11.window);
        realDrawArea = SDL_GetVideoSurface();
#endif

        /*
         * ステータスラインの強制再描画
         */
#ifndef USE_OPENGL
        DrawStatusForce();
        if (!bFullScan) {
                RenderSetOddLine();
        } else {
                RenderFullScan();
        }
#endif
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
    char        tmpStr[1024];
    
/*
 * ウィンドウ生成 
 */ 

    gbuilderMain =  gtk_builder_new();
#ifdef UIDIR
    strcpy(tmpStr, UIDIR);
    strcat(tmpStr, "gtk_prop.ui");
    printf("ui.config = %s\n",tmpStr);
    gtk_builder_add_from_file(gbuilderMain, tmpStr, NULL);
#else
    gtk_builder_add_from_file(gbuilderMain, "./gtk_prop.ui", NULL);
#endif
    wndMain = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "window_main"));
    gtk_window_set_title(GTK_WINDOW(wndMain), "XM7");
    /* This is important */
    gtk_builder_connect_signals (gbuilderMain, NULL);

    gtk_container_border_width(GTK_CONTAINER(wndMain), 0);
    gtk_window_set_resizable(GTK_WINDOW(wndMain), FALSE);
//    gtk_window_set_position(GTK_WINDOW(wndMain), GTK_WIN_POS_CENTER);
    
    
/*
 * 土台の土台となる垂直ボックス 
 */ 
    vbox = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_drawing"));
    gtk_widget_show(vbox);
    g_signal_connect(GTK_OBJECT(wndMain), "delete-event",
		 GTK_SIGNAL_FUNC(OnDelete), NULL);
    g_signal_connect(GTK_OBJECT(wndMain), "destroy",
		 GTK_SIGNAL_FUNC(OnDestroy), NULL);
    g_signal_connect(GTK_OBJECT(wndMain), "focus-in-event",
		 GTK_SIGNAL_FUNC(OnFocusIn), NULL);
    g_signal_connect(GTK_OBJECT(wndMain), "focus-out-event",
		 GTK_SIGNAL_FUNC(OnFocusOut), NULL);
    g_signal_connect(GTK_OBJECT(wndMain), "key-press-event",
		 GTK_SIGNAL_FUNC(OnKeyPressGtk), NULL);
    g_signal_connect(GTK_OBJECT(wndMain), "key-release-event",
		 GTK_SIGNAL_FUNC(OnKeyReleaseGtk), NULL);
    g_signal_connect(GTK_OBJECT(wndMain), "plug-added",
		 GTK_SIGNAL_FUNC(OnScreenPlugged), NULL);
    g_signal_connect(GTK_OBJECT(wndMain), "plug-removed",
		GTK_SIGNAL_FUNC(OnScreenUnPlugged), NULL);
    
// g_idle_add_full(G_PRIORITY_HIGH, &GtkEventHandler, NULL,
// NULL);
    
//    gtk_builder_connect_signals(gbuilderMain, NULL);
    gtk_widget_show(wndMain);
/*
 * イベント 
 */ 
    OnCreate(vbox);

    SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_TIMER | 0);        

    SDL_GetWMInfo(&sdlinfo);
//    gtk_socket_add_id(GTK_SOCKET(gtkDrawArea), sdlinfo.info.x11.window);
//    sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x",
//            gdk_x11_drawable_get_xid(gtkDrawArea->window));
    
//    SDL_putenv(EnvMainWindow);
       SDL_InitSubSystem(SDL_INIT_VIDEO); 


    
    CreateDrawSDL();
#ifdef RSSDIR
    strcpy(icon_path, RSSDIR);
#else
    strcpy(icon_path, ModuleDir);
#endif
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
#ifdef RSSDIR
    strcpy(icon_path, RSSDIR);
#else
    strcpy(icon_path, ModuleDir);
#endif
    
    switch (fm7_ver) {
    case 1:
	strcat(icon_path, "tamori.ico");
	break;
    case 2:
	strcat(icon_path, "app_av.ico");
	break;
    case 3:
	strcat(icon_path, "app_ex.ico");
	break;
    default:
	icon_path[0] = '\0';
    }
#ifdef RSSDIR
    if (icon_path[0] != '\0' && strcmp(icon_path, RSSDIR) != 0)
	gtk_window_set_icon_from_file(GTK_WINDOW(wndMain), icon_path,
                                      (GError **) NULL);
#else
    if (icon_path[0] != '\0' && strcmp(icon_path, ModuleDir) != 0)
	gtk_window_set_icon_from_file(GTK_WINDOW(wndMain), icon_path,
                                      (GError **) NULL);
#endif
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

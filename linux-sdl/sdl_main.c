/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN/SDL メインプログラム ]
 */

#ifdef _XWIN

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/param.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#if defined(USE_OPENGL)
#include <SDL/SDL_opengl.h>
#endif /* USE_OPENGL */

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

/*
 *	グローバル ワーク
 */
char ModuleDir[MAXPATHLEN];				/* XM7実行モジュールディレクトリ */
GtkWidget *wndMain;						/* メインウィンドウ */
//GtkWidget *drawArea;					/* スクリーン描画エリア */
SDL_Surface *drawArea = NULL;                           /* スクリーン描画エリア */
SDL_Surface *displayArea = NULL;
GtkWidget *gtkDrawArea;
int nErrorCode;							/* エラーコード */
BOOL bMenuLoop;							/* メニューループ中 */
BOOL bCloseReq;							/* 終了要求フラグ */
BOOL bSync;								/* 実行に同期 */
BOOL bSyncDisasm[2];					/* 逆アセンブルをPCに同期 */
BOOL bActivate;							/* アクティベートフラグ */
int nAppIcon;							/* アイコン番号(1,2,3) */
BOOL bMMXflag;							/* MMXサポートフラグ */
BOOL bCMOVflag;							/* CMOVサポートフラグ(現状未使用) */
#if ((XM7_VER <= 2) && defined(FMTV151))
BOOL bFMTV151;							/* チャンネルコールフラグ */
#endif


/*
 *	スタティック ワーク
 */

/*
 *	アセンブラ関数のためのプロトタイプ宣言
 */
//extern BOOL CheckMMX(void);				/* MMX命令サポートチェック */
//extern BOOL CheckCMOV(void);			/* CMOV命令サポートチェック */


/*-[ 同期 ]-----------------------------------------------------------------*/
                                                                                
/*
 *  VMをロック
 */
void FASTCALL LockVM(void)
{
}
                                                                                
/*
 *  VMをアンロック
 */
void FASTCALL UnlockVM(void)
{
}

/*-[ ドローウインドウ ]-----------------------------------------------------*/


/*
 * SDL関連フラグ
 */

#define XM7_DRAWMODE_SDL SDL_SWSURFACE
#define XM7_DRAW_WIDTH 640
#define XM7_DRAW_HEIGHT 400
#define XM7_DRAW_MAX_BPP 24 /* 32bitもやるか？遅いが */

#if SDL_BYTEORDER == SDL_BIG_ENDIAN /* BIGENDIAN */
#if XM7_DRAW_MAX_BPP == 32
#define XM7_DRAW_RMASK 0xff000000 /* R */
#define XM7_DRAW_GMASK 0x00ff0000 /* G */
#define XM7_DRAW_BMASK 0x0000ff00 /* B */
#define XM7_DRAW_AMASK 0x000000ff /* ALPHA */
#elif XM7_DRAW_MAX_BPP == 24 /* 24bit */
#define XM7_DRAW_RMASK 0x00ff0000
#define XM7_DRAW_GMASK 0x0000ff00
#define XM7_DRAW_BMASK 0x000000ff
#define XM7_DRAW_AMASK 0x00000000
#else
#define XM7_DRAW_RMASK 0x0000f000
#define XM7_DRAW_GMASK 0x00000f00
#define XM7_DRAW_BMASK 0x000000f0
#define XM7_DRAW_AMASK 0x0000000f
#endif
#else /* SDL_BYTEORDER */
#if XM7_DRAW_MAX_BPP ==32
#define XM7_DRAW_RMASK 0x000000ff
#define XM7_DRAW_GMASK 0x0000ff00
#define XM7_DRAW_BMASK 0x00ff0000
#define XM7_DRAW_AMASK 0xff000000
#elif XM7_DRAW_MAX_BPP == 24 /* 24bit */
#define XM7_DRAW_RMASK 0x00000000
#define XM7_DRAW_GMASK 0x00000000
#define XM7_DRAW_BMASK 0x00000000
#define XM7_DRAW_AMASK 0x00000000
#else /* not 32bit */
#define XM7_DRAW_RMASK 0x0000000f
#define XM7_DRAW_GMASK 0x000000f0
#define XM7_DRAW_BMASK 0x00000f00
#define XM7_DRAW_AMASK 0x0000f000
#endif
#endif

/*
 *	ドローウインドウ作成(SDL)
 */

static void CreateDrawSDL(GtkWidget *parent)
{

  SDL_Surface *ret;

#if defined(USE_OPENGL)
  drawArea = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			1024, 512, /* OpenGL使う場合はw,hは2^nでなければならない */ 
			32,
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN /* OpenGL RGBA masks */
			0x000000FF, 
			0x0000FF00, 
			0x00FF0000, 
			0xFF000000
#else
			0xFF000000,
			0x00FF0000, 
			0x0000FF00, 
			0x000000FF
#endif
		       );
#else
  drawArea = SDL_CreateRGBSurface(
                                  SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | 0 ,
                                  XM7_DRAW_WIDTH, XM7_DRAW_HEIGHT, 
                                  32,
#if SDL_BYTEORDER == SDL_LITTLE_ENDIAN /* OpenGL RGBA masks */
                                  0x000000FF, 
                                  0x0000FF00, 
                                  0x00FF0000, 
                                  0xFF000000
#else
                                  0xFF000000,
                                  0x00FF0000, 
                                  0x0000FF00, 
                                  0x000000FF
#endif
                                  );
#endif /* USE_OPENGL */
        //drawArea = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_ASYNCBLIT, XM7_DRAW_WIDTH, XM7_DRAW_HEIGHT, 
        //                           32, 0, 0, 0, 0 );
   
   if(drawArea == NULL) {
	perror("SDL can't get drawing area"); /* 最終的にはWidget独立でPopup出す */
	exit(1);
     }

#if defined(USE_OPENGL)
   ret = SDL_SetVideoMode(640 , 400 , 32 ,
   		  SDL_OPENGL);

   SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
   SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
   SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
   SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#else
   ret = SDL_SetVideoMode(640 , 400 , 24 ,
   		  SDL_HWSURFACE | SDL_ANYFORMAT | SDL_RESIZABLE | SDL_DOUBLEBUF | SDL_ASYNCBLIT | 0);
#endif /* USE_OPENGL */
   
   if(ret == NULL) {
        perror("SDL can't get display area."); /* 最終的にはWidget独立でPopup出す */
        SDL_FreeSurface(drawArea);
        exit(1);
   }
   displayArea = SDL_GetVideoSurface();


   /* マウス処理やWIndowとの接続をここに入れる */
   //SDL_UpdateRect(displayArea ,0 ,0 ,640 ,480);
   
}

static void CreateDrawGTK(GtkWidget *parent)
{
        GtkWidget *hbox;

        /* ドローウィンドウの土台を生成 */
        hbox = gtk_hbox_new( FALSE, 0 );
        gtk_box_pack_start (GTK_BOX(parent), hbox, TRUE, TRUE, 0);
        gtk_widget_show( hbox );

        /* スクリーン描画領域の生成 */
        //gtkDrawArea = gtk_drawing_area_new();
        //gtk_widget_set_events(gtkDrawArea, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);

        gtkDrawArea = gtk_socket_new();
        gtk_widget_set_usize(gtkDrawArea, 640, 400);
        gtk_container_add(GTK_CONTAINER(parent), gtkDrawArea);
               /* すべて表示 */
        gtk_widget_show(gtkDrawArea);

}

   
/*-[ メインウインドウ ]-----------------------------------------------------*/

/*
 *	ウインドウ作成
 */
static void FASTCALL OnCreate(GtkWidget *parent)
{
	BOOL flag;
                 char EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
	/* ドローウインドウ、ステータスバーを作成 */	
	CreateMenu(parent);
	CreateDrawGTK(parent);
	CreateStatus(parent);

	/* ワークエリア初期化 */
	nErrorCode = 0;
	bMenuLoop = FALSE;
	bCloseReq = FALSE;
	bSync = TRUE;
	bSyncDisasm[0] = TRUE;
	bSyncDisasm[1] = TRUE;
	bActivate = FALSE;




	/* コンポーネント初期化 */
	LoadCfg();

	InitDraw();
	InitSnd();
	InitKbd();
	InitSch();
        //#ifdef FDDSND
        //	InitFDDSnd();
        //#endif

	/* 仮想マシン初期化 */
	if (!system_init()) {
		nErrorCode = 1;
		return;
	}
	/* 直後、リセット */
	ApplyCfg();
	system_reset();

	/* コンポーネントセレクト */
	flag = TRUE;
	if (!SelectDraw()) {
		flag = FALSE;
	}
	if (!SelectSnd()) {
		flag = FALSE;
	}
	if (!SelectKbd()) {
		flag = FALSE;
	}
	if (!SelectSch()) {
		flag = FALSE;
	}

	/* エラーコードをセットさせ、スタート */
	if (!flag) {
		nErrorCode = 2;
	}
}

/*
 *	ウインドウデリート
 */
static gboolean FASTCALL OnDelete( GtkWidget *widget, GdkEvent  *event, gpointer data )
{
	/* フラグアップ */
	LockVM();
	bCloseReq = TRUE;
	UnlockVM();
	return FALSE;
}

/*
 *	ウインドウ削除
 */
static void FASTCALL OnDestroy( GtkWidget *widget, gpointer   data )
{
	/* サウンド停止 */
	StopSnd();

	/* コンポーネント クリーンアップ */
#ifdef FDDSND
	CleanFDDSnd();
#endif
	CleanSch();
	CleanKbd();
	CleanSnd();
	CleanDraw();
	SaveCfg();

	/* 仮想マシン クリーンアップ */
	system_cleanup();

    gtk_main_quit ();
}

/*-[ アプリケーション ]-----------------------------------------------------*/

/*
 *	ドローウインドウへのフォーカスインイベント
 */
static void FASTCALL OnFocusIn(GtkWidget *widget, gpointer data) {
    bActivate = TRUE;
#ifdef MOUSE
    SetMouseCapture(bActivate);
#endif
}

/*
 *	ドローウインドウへのフォーカスアウトイベント
 */
static void FASTCALL OnFocusOut(GtkWidget *widget, gpointer data) {
    bActivate = FALSE;
#ifdef MOUSE
    SetMouseCapture(bActivate);
#endif
}

/*
 * GTK: スクリーン解像度変更の時にはSDLを終了→初期化→再生成する。
 */
static void OnScreenPlugged(void)
{
  SDL_SysWMinfo sdlinfo;
  char EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
  /* SDL初期化 */
  if((drawArea == NULL) || (displayArea ==NULL)) {
   sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(gtkDrawArea->window));
   //        sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(wndMain->window));
   SDL_putenv(EnvMainWindow);
   SDL_InitSubSystem(SDL_INIT_VIDEO);
   CreateDrawSDL(wndMain);
  }

}


static void OnScreenUnPlugged(void)
{
  SDL_SysWMinfo sdlinfo;
  /* SDL-VIDEO終了 */
   //        sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(wndMain->window));
  if(displayArea) {
    SDL_FreeSurface(displayArea);
    displayArea = NULL;
  }
  if(drawArea) {
    SDL_FreeSurface(drawArea);
    drawArea = NULL;
  }


}
   
/*
 * キーボード：GTK->SDL Wrapper
 */
static gboolean FASTCALL OnKeyPressGTK( GtkWidget *widget, GdkEventKey  *event, gpointer data )
{
  Uint8 keycode = (Uint8)event->hardware_keycode;
  SDL_Event sdlevent;
  sdlevent.type = SDL_KEYDOWN;
  sdlevent.key.type = SDL_KEYDOWN;
  sdlevent.key.state = SDL_PRESSED;
  sdlevent.key.keysym.scancode = keycode;
  SDL_PushEvent(&sdlevent);
  return TRUE;
}

static gboolean FASTCALL OnKeyReleaseGTK( GtkWidget *widget, GdkEventKey  *event, gpointer data )
{
  Uint8 keycode = (Uint8)event->hardware_keycode;
  SDL_Event sdlevent;
  sdlevent.type = SDL_KEYUP;
  sdlevent.key.type = SDL_KEYUP;
  sdlevent.key.state = SDL_RELEASED;
  sdlevent.key.keysym.scancode = keycode;
  SDL_PushEvent(&sdlevent);
  return TRUE;
}

/*
 *	インスタンス初期化
 */
static void FASTCALL InitInstance(void)
{
	GtkWidget *vbox;
	GError *error = NULL;
	char icon_path[MAXPATHLEN];
                 char EnvMainWindow[64]; /* メインウィンドウのIDを取得して置く環境変数 */
                 SDL_SysWMinfo sdlinfo;

	/* ウィンドウ生成 */
	wndMain = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (wndMain), "XM7");
	gtk_container_border_width (GTK_CONTAINER (wndMain), 0);
	gtk_window_set_resizable (GTK_WINDOW (wndMain), FALSE);
	gtk_window_set_position (GTK_WINDOW (wndMain), GTK_WIN_POS_CENTER);

	/* 土台の土台となる垂直ボックス */
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (wndMain), vbox);
	gtk_widget_show( vbox );

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

//        g_idle_add_full(G_PRIORITY_HIGH, &GtkEventHandler, NULL, NULL);
	gtk_widget_show(wndMain);

        /* イベント */

   
	OnCreate(vbox);
                 SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_TIMER);
                 //gtk_widget_realize(wndMain);

                 SDL_GetWMInfo(&sdlinfo);
                 gtk_socket_add_id(gtkDrawArea, sdlinfo.info.x11.window);
                 sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(gtkDrawArea->window));
                 //sprintf(EnvMainWindow, "SDL_WINDOWID=0x%08x", gdk_x11_drawable_get_xid(wndMain->window));
                 SDL_putenv(EnvMainWindow);
                 SDL_InitSubSystem(SDL_INIT_VIDEO);
                 CreateDrawSDL(wndMain);


	strcpy(icon_path, ModuleDir);
	switch(fm7_ver) {
		case 1:
			strcat(icon_path,"resource/tamori.ico");
			break;
		case 2:
			strcat(icon_path,"resource/app_av.ico");
			break;
		case 3:
			strcat(icon_path,"resource/app_ex.ico");
			break;
		default:
			icon_path[0] = '\0';
	}

	if ( icon_path[0] != '\0' && strcmp(icon_path, ModuleDir)!=0 )
		gtk_window_set_icon_from_file (GTK_WINDOW(wndMain), icon_path, (GError **)NULL);

	return;
}

/* 
 * Update UI
 */
void ui_update(void)
{
   /* GTK使う */
   while (gtk_events_pending())
            gtk_main_iteration_do(FALSE);
}



/*
 * メイン関数
 */
int main( int argc, char *argv[] )
{

	/* 実行モジュールのディレクトリを取得 */
	char *p;

	if ( realpath(argv[0], ModuleDir) == NULL ){
		perror("Can't get module real path.");
		exit(1);
	} else {
		p = strrchr(ModuleDir, '/');
		p[1] = '\0';
	}

	/* 各種フラグを取得・設定 */
	//bMMXflag = CheckMMX();
	//bCMOVflag = CheckCMOV();
	/* x86依存命令除去 */
	bMMXflag = FALSE;
	bCMOVflag = FALSE;
#if ((XM7_VER <= 2) && defined(FMTV151))
	bFMTV151 = TRUE;
#endif

	/* スレッドの初期化 */
	g_thread_init(NULL);
	gdk_threads_init();

	gtk_set_locale();
	gtk_init(&argc, &argv);

	/* アプリケーション初期化 */

	InitInstance();

	/* エラーコード別 */
	switch (nErrorCode) {
		/* エラーなし */
		case 0:
			/* 実行開始 */
			stopreq_flag = FALSE;
			run_flag = TRUE;

			/* コマンドライン処理 */
			if (argc > 1) {
				OnCmdLine(argv[1]);
			}
			break;

		/* VM初期化エラー */
		case 1:
			gtk_widget_hide(wndMain);
			OpenErrorMessageDialog("XM7", "仮想マシンを初期化できません");
			break;

		/* コンポーネント初期化エラー */
		case 2:
			gtk_widget_hide(wndMain);
			OpenErrorMessageDialog("XM7", "コンポーネントを初期化できません");
			break;
	}
        atexit(SDL_Quit);



	/* GTK のメインループに入る */
	gtk_main();
   

   
	return nErrorCode;
}

#endif /* _XWIN */

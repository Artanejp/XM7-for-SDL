/*
 * gtk_kbd.c
 *
 *  Created on: 2010/08/25
 *      Author: whatisthis
 */
#include<X11/Xlib.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<gdk/gdkkeysyms.h>
#include<memory.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"
#include "sdl.h"
#include "sdl_sch.h"
#include "sdl_kbd.h"
#include "gtk_propkeyboard.h"

extern BYTE   kibuf[256];	/* キーボード押下状態 */

/*
 *  DirectInputコード→FM-7 物理コード
 * コード対照表(109キーボード用)
 */

static BYTE   kbd_106_table[] = { 0x09, 0x5c, /* BREAK(ESC) */
	0x43, 0x5d, /* PF1 */
	0x44, 0x5e, /* PF2 */
	0x45, 0x5f, /* PF3 */
	0x46, 0x60, /* PF4 */
	0x47, 0x61, /* PF5 */
	0x48, 0x62, /* PF6 */
	0x49, 0x63, /* PF7 */
	0x4a, 0x64, /* PF8 */
	0x4b, 0x65, /* PF9 */
	0x4c, 0x66, /* PF10 */
	0x31, 0x01, /* ESC(半角/全角) */
	0x0a, 0x02, /* 1 */
	0x0b, 0x03, /* 2 */
	0x0c, 0x04, /* 3 */
	0x0d, 0x05, /* 4 */
	0x0e, 0x06, /* 5 */
	0x0f, 0x07, /* 6 */
	0x10, 0x08, /* 7 */
	0x11, 0x09, /* 8 */
	0x12, 0x0a, /* 9 */
	0x13, 0x0b, /* 0 */
	0x14, 0x0c, /* - */
	0x15, 0x0d, /* ^ */
//	0x85, 0x0e, /* \ */
	132, 0x0e, /* \ */
	0x16, 0x0f, /* BS */
	0x17, 0x10, /* TAB */
	0x18, 0x11, /* Q */
	0x19, 0x12, /* W */
	0x1a, 0x13, /* E */
	0x1b, 0x14, /* R */
	0x1c, 0x15, /* T */
	0x1d, 0x16, /* Y */
	0x1e, 0x17, /* U */
	0x1f, 0x18, /* I */
	0x20, 0x19, /* O */
	0x21, 0x1a, /* P */
	0x22, 0x1b, /* @ */
	0x23, 0x1c, /* [ */
	0x24, 0x1d, /* CR */
	0x25, 0x52, /* CTRL(左Ctrl) */
	0x26, 0x1e, /* A */
	0x27, 0x1f, /* S */
	0x28, 0x20, /* D */
	0x29, 0x21, /* F */
	0x2a, 0x22, /* G */
	0x2b, 0x23, /* H */
	0x2c, 0x24, /* J */
	0x2d, 0x25, /* K */
	0x2e, 0x26, /* L */
	0x2f, 0x27, /* ; */
	0x30, 0x28, /* : */
	0x33, 0x29, /* ] */
	0x32, 0x53, /* 左SHIFT */
	0x34, 0x2a, /* Z */
	0x35, 0x2b, /* X */
	0x36, 0x2c, /* C */
	0x37, 0x2d, /* V */
	0x38, 0x2e, /* B */
	0x39, 0x2f, /* N */
	0x3a, 0x30, /* M */
	0x3b, 0x31, /* , */
	0x3c, 0x32, /* . */
	0x3d, 0x33, /* / */
//	0x7b, 0x34, /* _ */
	97, 0x34, /* _ */
	0x3e, 0x54, /* 右SHIFT */
	0x40, 0x55, /* CAP(左ALT) */
	// 0x83, 0x56, /* GRAPH(無変換) */
	102, 0x56, /* GRAPH(無変換) for 109 */
	// 0x81, 0x57, /* 左SPACE(変換) */
	100, 0x57, /* 左SPACE(変換) for 109 */
	0x78, 0x58, /* 中SPACE(カタカナ) */
	101, 0x58, /* 中SPACE(カタカナ) */
	0x41, 0x35, /* 右SPACE(SPACE) */
//	 0x6d, 0x5a, /* かな(右Ctrl) */
	105, 0x5a, /* かな(右Ctrl) for 109 */
//	0x6a, 0x48, /* INS(Insert) */
//	0x6b, 0x4b, /* DEL(Delete) */
//	0x62, 0x4d, /* ↑ */
//	0x64, 0x4f, /* ← */
///	0x68, 0x50, /* ↓ */
//	0x66, 0x51, /* → */
	118, 0x48, /* INS(Insert) */
	119, 0x4b, /* DEL(Delete) */
	111, 0x4d, /* ↑ */
	113, 0x4f, /* ← */
	116, 0x50, /* ↓ */
	114, 0x51, /* → */
	110, 0x49, /* EL(Home) */
	112, 0x4a, /* CLS(Page Up) */
	115, 0x4c, /* DUP(End) */
	117, 0x4e, /* HOME(Page Down) */

//	0x61, 0x49, /* EL(Home) */
//	0x63, 0x4a, /* CLS(Page Up) */
//	0x67, 0x4c, /* DUP(End) */
//	0x69, 0x4e, /* HOME(Page Down) */
	0x3f, 0x36, /* Tenkey * */
	0x70, 0x37, /* Tenkey / */
	0x56, 0x38, /* Tenkey + */
	0x52, 0x39, /* Tenkey - */
	0x4f, 0x3a, /* Tenkey 7 */
	0x50, 0x3b, /* Tenkey 8 */
	0x51, 0x3c, /* Tenkey 9 */
	0x53, 0x3e, /* Tenkey 4 */
	0x54, 0x3f, /* Tenkey 5 */
	0x55, 0x40, /* Tenkey 6 */
	0x57, 0x42, /* Tenkey 1 */
	0x58, 0x43, /* Tenkey 2 */
	0x59, 0x44, /* Tenkey 3 */
	0x5a, 0x46, /* Tenkey 0 */
	0x5b, 0x47, /* Tenkey . */
	0x6c, 0x45 /* Tenkey CR */
};
static BYTE *kbd_tbl_gtk;
static GdkCursor *nullcursor;	/* 透明カーソル */

#ifdef MOUSE
static DWORD    mostime;	/* マウスポーリング時間 */
static BOOL    bCapture;	/* マウスキャプチャフラグ(Win32)
				 */
static int     nMouseX;	/* マウス X軸移動距離 */
static int     nMouseY;	/* マウス Y軸移動距離 */
static BYTE    nMouseButton;	/* マウス ボタン押下状態 */
static BYTE    nMouseButtons;	/* マウス ボタン数 */
static BYTE    nCenterButton;	/* マウス 中央ボタン押下状態 */
static BOOL    bMouseCursor;	/* マウス カーソル表示状態 */
static int     nMouseSX;	/* マウス X座標保存 */
static int     nMouseSY;	/* マウス Y座標保存 */
static int     nMousePX;	/* マウス X座標保存(一時) */
static int     nMousePY;	/* マウス Y座標保存(一時) */
static int     nDAreaW;	/* ドローイングエリアの幅 */
static int     nDAreaH;	/* ドローイングエリアの高さ */
static BOOL    rgbButtons[3];	/* マウスボタン押下状態 */

//static SDL_Cursor *nullcursor;	/* 透明カーソル */


#endif				/*  */

void
InitKbd_Gtk(void)
{
#ifdef MOUSE
	/*
	 *  透明カーソルイメージ
	 */
//    Uint8 nullcursor_img[] = {0x00};
//    Uint8 nullcursor_mask[] = {0x00};
    gchar nullcursor_img[] = {0x00};
    gchar nullcursor_mask[] = {0x00};
    GdkBitmap *mask, *source;
    GdkColor fg, bg;
#endif
	/*
	 * キーボードテーブルの転写
	 */
	kbd_tbl_gtk =(BYTE *)malloc(256*2); // 実際にキーを読み込む領域
	if(kbd_tbl_gtk) {
		memset(kbd_tbl_gtk,0,256*2);
		memcpy(kbd_tbl_gtk,kbd_106_table,sizeof(kbd_106_table));
		}
#ifdef MOUSE
	/*
	 * ワークエリア初期化(マウス)
	 */
    mostime = 0;
    bCapture = FALSE;
    nMouseX = 0;
    nMouseY = 0;
    nCenterButton = 0;
    bMouseCursor = TRUE;
    nMouseButton = 0xf0;
    nMidBtnMode = MOSCAP_NONE;
    nMouseButtons = 3;

    //nullcursor = SDL_CreateCursor(nullcursor_img, nullcursor_mask, 8, 8, 0, 0);
    //nullcursor = NULL;
#endif				/*  */


#ifdef MOUSE
/*
 * 透明カーソル生成
 */
source = gdk_bitmap_create_from_data(NULL, nullcursor_img, 1, 1);
mask = gdk_bitmap_create_from_data(NULL, nullcursor_mask, 1, 1);
nullcursor = gdk_cursor_new_from_pixmap(source, mask, &fg, &bg, 0, 0);
gdk_pixmap_unref(source);
gdk_pixmap_unref(mask);
#endif

}



/**[ アクションイベント ]***********************************************/

    /*
     *  キープレスアクション
     */
gboolean
OnKeyPressGtk(GtkWidget * widget, GdkEventKey * event,
		    gpointer data)
{
    int            i;
      // printf("Key - GTK: %04x\n", event->hardware_keycode);
    for (i = 0; i<255; i++) {
	if (kbd_tbl_gtk[i * 2] == event->hardware_keycode) {
	    if (kibuf[kbd_tbl_gtk[i * 2 + 1]] != 0x80) {
		kibuf[kbd_tbl_gtk[i * 2 + 1]] = 0x80;
	    }
	    break;
		}
    }
    return TRUE;
}


/*
 *  キーリリースアクション
 */
gboolean OnKeyReleaseGtk(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
{
    int            i;
    for (i = 0; i < 256; i++) {
	if (kbd_tbl_gtk[i * 2] == event->hardware_keycode) {
	    kibuf[kbd_tbl_gtk[i * 2 + 1]] = 0x00;
	    break;
	}
    }

#ifdef MOUSE
	/*
	 * F11押下の場合はマウスキャプチャフラグを反転させてモード切り替え
	 */
	if (event->hardware_keycode == 0x5f) {
	mos_capture = (!mos_capture);
	SetMouseCapture(bActivate);
    }

#endif				/*  */

	/*
	 * F12押下の場合はVMリセット
	 */
	if (event->hardware_keycode == 0x60) {
	LockVM();
	system_reset();
	UnlockVM();
    }
    return TRUE;
}

/*
 *  マウス キャプチャ状態設定
 */
void
SetMouseCapture(BOOL en)
{
int            x,
                y;
GdkModifierType state;
GdkCursor * cursor;
//    SDL_Cursor * cursor;
//SDL_SysWMinfo sdlinfo;
/*
 * キャプチャ停止中/VM停止中/非アクティブ時は強制的に無効
 */
if (!mos_capture || stopreq_flag || !run_flag || !bActivate) {
en = FALSE;
}

/*
 * カーソル表示/消去
 */
if (bMouseCursor == en) {
if (en) {
    cursor = nullcursor;
} else {
    cursor = NULL;
}
bMouseCursor = !en;
}

/*
 * キャプチャ状態に変化がなければ帰る
 */
if (bCapture == en) {
return;
}
if (en) {

    /*
     * カーソル位置を保存
     */
    // gtk_widget_get_pointer(drawArea, &x, &y);
    nMouseSX = x;
nMouseSY = y;

    /*
     * 描画ウィンドウのサイズを求める
     */
    // gdk_window_get_size(drawArea->window, &nDAreaW, &nDAreaH);

    /*
     * 中心座標を求める
     */
    x = nDAreaW / 2;
y = nDAreaH / 2;

    /*
     * カーソルをウィンドウ中央に固定
     */
    // XWarpPointer(
    // GDK_WINDOW_XDISPLAY(drawArea->window),
    // None,
    // GDK_WINDOW_XWINDOW(drawArea->window),
    // 0,
    // 0,
    // 0,
    // 0,
    // x,
    // y
    // );
        SDL_WM_GrabInput(SDL_GRAB_ON);
//if(SDL_GetWMInfo(&sdlinfo)) {
//        XGrabPointer(sdlinfo.info.x11.display, sdlinfo.info.x11.window, true,  0, GrabModeSync,  GrabModeSync,  sdlinfo.info.x11.window, None, CurrentTime);
//}

}
else {

    /*
     * クリップ解除
     */
        //gdk_pointer_ungrab(GDK_CURRENT_TIME);
        //        if(SDL_GetWMInfo(&sdlinfo)) {
        //        XUngrabPointer(sdlinfo.info.x11.display, CurrentTime);
        //}
        SDL_WM_GrabInput(SDL_GRAB_OFF);
    /*
     * カーソル位置を復元
     */
    // XWarpPointer(
    // / GDK_WINDOW_XDISPLAY(drawArea->window),
    // None,
    // GDK_WINDOW_XWINDOW(drawArea->window),
    // 0,
    // 0,
    // 0,
    // 0,
    // nMouseSX,
    // nMouseSY
    // );

}

/*
 * キャプチャ状態を保存
 */
bCapture = en;

/*
 * マウス移動距離をクリア
 */
nMouseX = 0;
nMouseY = 0;
}


/*
 *  マウス 移動データリクエスト
 */
void
mospos_request(BYTE * move_x, BYTE * move_y)
{
if (bCapture) {

    /*
     * 移動距離を符号付き８ビットの範囲に収める
     */
    if (nMouseX > 127) {
    nMouseX = 127;
}

else if (nMouseX < -127) {
    nMouseX = -127;
}
if (nMouseY > 127) {
    nMouseY = 127;
}

else if (nMouseY < -127) {
    nMouseY = -127;
}
*move_x = (BYTE) nMouseX;
*move_y = (BYTE) nMouseY;
}

else {

    /*
     * キャプチャ一時停止中は移動していないことにする
     */
    *move_x = 0;
*move_y = 0;
}

/*
 * マウス移動距離をクリア
 */
nMouseX = 0;
nMouseY = 0;
}

#ifdef MOUSE
    /*
     *  マウス ポーリング
     */
void            FASTCALL
PollMos(void)
{
    int            x,
                    y,
                    lX,
                    lY;

	/*
	 * アクティベートチェック
	 */
	if (!bActivate) {
	return;
    }

	/*
	 * 時間チェック
	 */
	if ((dwExecTotal - mostime) < 10000) {
	return;
    }
    mostime = dwExecTotal;

	/*
	 * 移動距離・ボタン状態の設定
	 */
	if (bCapture && mos_capture) {
	gdk_threads_enter();


	    // gtk_widget_get_pointer(drawArea, &x, &y);
        lX = x - nMousePX;
	lY = y - nMousePY;
	nMousePX = x;
	nMousePY = y;
	if (x == 0 || y == 0 || x == nDAreaW - 1 || y == nDAreaH - 1) {

		// XWarpPointer(
		// GDK_WINDOW_XDISPLAY(drawArea->window),
		// None,
		// GDK_WINDOW_XWINDOW(drawArea->window),
		// 0,
		// 0,
		// 0,
		// 0,
		// nDAreaW/2,
		// nDAreaH/2
		// );

		// nMousePX = nDAreaW/2;
		// nMousePY = nDAreaH/2;
	}

	    /*
	     * マウス移動距離を蓄積
	     */
	    nMouseX -= lX;
	nMouseY -= lY;

	    /*
	     * ボタン状態を設定
	     */
	    nMouseButton = 0xf0;
	if (rgbButtons[0]) {

		/*
		 * 左ボタン押下
		 */
		nMouseButton &= ~0x10;
	}
	if (rgbButtons[2]) {

		/*
		 * 右ボタン押下
		 */
		nMouseButton &= ~0x20;
	}
	gdk_threads_leave();
    }
    if (nMouseButtons >= 3) {

	    /*
	     * 中央ボタンが押されたらマウスモードを切り換える
	     */
	    if (rgbButtons[1] && !nCenterButton) {

		/*
		 * マウスキャプチャフラグを反転させてモード切り替え
		 */
		mos_capture = (!mos_capture);
	    gdk_threads_enter();
	    SetMouseCapture(bActivate);
	    gdk_threads_leave();
	}

	    /*
	     * 現在の中央ボタンの状態を保存
	     */
	    nCenterButton = rgbButtons[1];
    }
}




    /*
     *  マウス ボタンデータリクエスト
     */
BYTE
mosbtn_request(void)
{

	/*
	 * ボタン情報を返す
	 */
	return nMouseButton;
}


#endif	/* MOUSE */
#ifdef MOUSE
    /*
     *  マウスボタンプレスアクション
     */
gboolean OnButtonPress(GtkWidget * widget, GdkEventButton * event,
			   gpointer data)
{
    rgbButtons[event->button - 1] = TRUE;
    return FALSE;
}


/*
 *  マウスボタンリリースアクション
 */
gboolean OnButtonRelease(GtkWidget * widget, GdkEventButton * event,
			     gpointer data)
{
    rgbButtons[event->button - 1] = FALSE;
    return FALSE;
}


#endif				/*  */

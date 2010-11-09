/*
 * api_mouse.cpp
 *
 *　マウス共通API
 *  Created on: 2010/09/18
 *      Author: K.Ohta<whatisthis.sowhat@gmail.com>
 */

#ifdef USE_GTK
#include<X11/Xlib.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<gdk/gdkkeysyms.h>
#endif

#include<memory.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "sdl.h"
#endif
#include "sdl_sch.h"

#include "api_kbd.h"
#include "api_js.h"
#include "api_mouse.h"

#include "gtk_propkeyboard.h"

extern "C" {
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
static int		nMousePX;
static int     nMousePY;	/* マウス Y座標保存(一時) */
static int     nDAreaW;	/* ドローイングエリアの幅 */
static int     nDAreaH;	/* ドローイングエリアの高さ */
static BOOL    rgbButtons[3];	/* マウスボタン押下状態 */

BYTE nMidBtnMode;
BOOL bMouseCaptureFlag;

//static SDL_Cursor *nullcursor;	/* 透明カーソル */


#endif				/*  */



void InitMouse()
{

#ifdef MOUSE
	/*
	 * ワークエリア初期化(マウス)
	 */
    mostime = 0;
    bCapture = FALSE;
    bMouseCaptureFlag = FALSE;
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
#endif

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
		 /*
		  * マウス移動距離を蓄積
		  */
//		 nMouseX -= lX;
//		 nMouseY -= lY;

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
#ifdef USE_GTK
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
#endif
#ifdef USE_AGAR
void OnButtonPress(AG_Event *event)
{

}


void OnButtonRelease(AG_Event *event)
{

}
#endif
#endif

/*
 * マウスクリックした(SDL)
 */
void OnClickPress(SDL_Event *event)
{
	Uint8 state = event->button.state;
	Uint8 button = event->button.button;
	switch(button){
	case SDL_BUTTON_LEFT:
		rgbButtons[0] = TRUE;
		break;
	case SDL_BUTTON_MIDDLE:
		rgbButtons[1] = TRUE;
		break;
	case SDL_BUTTON_RIGHT:
		rgbButtons[2] = TRUE;
		break;
	default:
		break;
	}
}

void OnClickRelease(SDL_Event *event)
{
	Uint8 state = event->button.state;
	Uint8 button = event->button.button;
	switch(button){
	case SDL_BUTTON_LEFT:
		rgbButtons[0] =FALSE;
		break;
	case SDL_BUTTON_MIDDLE:
		rgbButtons[1] =FALSE;
		break;
	case SDL_BUTTON_RIGHT:
		rgbButtons[2] =FALSE;
		break;
	default:
		break;
	}

}

/*
 * マウス移動した(SDL)
 */
void OnMoveMouse(SDL_Event *event)
{
	Uint16 x = event->motion.x;
	Uint16 y = event->motion.x;
	Sint16 xrel = event->motion.xrel;
	Sint16 yrel = event->motion.yrel;
	Uint8 state = event->motion.state;
	nMouseX += (int)xrel;
	nMouseY += (int)yrel;

}
}

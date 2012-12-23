/*
 * api_mouse.cpp
 *
 *　マウス共通API
 *  Created on: 2010/09/18
 *      Author: K.Ohta<whatisthis.sowhat@gmail.com>
 */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/cursors.h>

#include<memory.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"

#include "agar_xm7.h"
#include "sdl_sch.h"
#include "agar_sdlview.h"

#include "api_kbd.h"
#include "api_js.h"
#include "api_mouse.h"

extern void InitMouseSub(void);
void GetMousePos(int *x, int *y);

extern "C" {
extern AG_GLView *GLDrawArea;
extern XM7_SDLView *DrawArea;
   
#ifdef MOUSE
extern int     nMouseX;	/* マウス X軸移動距離 */
extern int     nMouseY;	/* マウス Y軸移動距離 */
extern BYTE    nMouseButton;	/* マウス ボタン押下状態 */

BOOL    bMouseCaptureFlag;	/* マウスキャプチャフラグ(Win32)*/

static DWORD    mostime;	/* マウスポーリング時間 */
				
static BYTE    nMouseButtons;	/* マウス ボタン数 */
static BYTE    nCenterButton;	/* マウス 中央ボタン押下状態 */
static int		nMousePX;
static int     nMousePY;	/* マウス Y座標保存(一時) */
static int     nDAreaW;	/* ドローイングエリアの幅 */
static int     nDAreaH;	/* ドローイングエリアの高さ */
static BOOL    rgbButtons[3];	/* マウスボタン押下状態 */

BYTE nMidBtnMode;


#endif				/*  */



void InitMouse()
{

#ifdef MOUSE
	/*
	 * ワークエリア初期化(マウス)
	 */
    mostime = 0;
    bMouseCaptureFlag = FALSE;
    nMouseX = 0;
    nMouseY = 0;
    nCenterButton = 0;
    nMouseButton = 0xf0;
    nMidBtnMode = MOSCAP_NONE;
    nMouseButtons = 3;
    InitMouseSub();
#endif				/*  */


#ifdef MOUSE
/*
 * 透明カーソル生成
 */

#endif

}


/*
 *  マウス 移動データリクエスト
 */
void mospos_request(BYTE * move_x, BYTE * move_y)
{
	if (bMouseCaptureFlag) {

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
//        printf("Pos: %d %d\n", *move_x, *move_y);
}

#ifdef MOUSE
/*
 *  マウス ポーリング
 */
void FASTCALL PollMos(void)
{
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
	 if (bMouseCaptureFlag && mos_capture) {
		 /*
		  * マウス移動距離を蓄積
		  */

		 /*
		  * ボタン状態を設定
		  */
	 }
   
//	 if (nMouseButtons >= 3) {
//
//		 /*
//		  * 中央ボタンが押されたらマウスモードを切り換える
//		  */
//		 if (rgbButtons[1] && !nCenterButton) {
//
//			 /*
//			  * マウスキャプチャフラグを反転させてモード切り替え
//			  */
//			 mos_capture = (!mos_capture);
//			 SetMouseCapture(bActivate);
//		 }

//		 /*
//		  * 現在の中央ボタンの状態を保存
//		  */
//		 nCenterButton = rgbButtons[1];
//	 }
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
	return (BYTE)nMouseButton;
}


#endif	/* MOUSE */
#ifdef MOUSE
/*
 *  マウスボタンプレスアクション
 */
#endif

}

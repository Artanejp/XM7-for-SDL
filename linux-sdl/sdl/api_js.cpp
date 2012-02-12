/*
 * api_js.cpp
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */


#include<X11/Xlib.h>
//#include<gtk/gtk.h>
//#include<gdk/gdkx.h>
//#include<gdk/gdkkeysyms.h>
#include<memory.h>
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>
#include <vector>
#include "SDLJoyInterface.h"

#ifdef __cplusplus
extern "C" {
#endif

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


int            nJoyType [_JOY_MAX_PLUG];	/* ジョイスティックタイプ */
int            nJoyRapid[_JOY_MAX_PLUG][_JOY_MAX_BUTTONS];	/* 連射タイプ */
int            nJoyCode [_JOY_MAX_PLUG][_JOY_MAX_AXIS * 4 + _JOY_MAX_BUTTONS + 1];	/* 生成コード */   
//BYTE nJoyRaw[MAX_SDL_JOY];

static DWORD   joydat[3];	/* ジョイスティックデータ */
static DWORD   joyaxis[3];	/* ジョイスティックデータ */
static DWORD    joybkaxis[_JOY_MAX_PLUG];	/* ジョイスティックバックアップ(カーソル) */
static DWORD    joybkbutton[_JOY_MAX_PLUG];	/* ジョイスティックバックアップ(ボタン) */
static int     joyrapid[_JOY_MAX_PLUG][_JOY_MAX_BUTTONS];	/* ジョイスティック連射カウンタ */
static DWORD   joytime;	/* ジョイスティックポーリング時間	 */
static DWORD   joytime2;	/* ジョイスティックポーリング時間	 */
static BOOL    joyplugged[2];	/* ジョイスティック接続フラグ	 */
static SDLJoyInterface *SDLDrv; /* SDL JSドライバー */
static DWORD nJoyKeyCode[MAX_SDL_JOY][12 + _JOY_MAX_BUTTONS]; /* ジョイスティックキーコードアサイン */


/*
 *  ジョイスティック
 * 連射カウンタテーブル(片側、20ms単位)
 */
static const BYTE JoyRapidCounter[] = { 0, /* なし */
		25, /* 1ショット */
		12, /* 2ショット */
		8, /* 3ショット */
		6, /* 4ショット */
		5, /* 5ショット */
		4, /* 6ショット */
		3, /* 8ショット */
		2, /* 12ショット */
		1 /* 25ショット */
};


/*
 *  ジョイスティック
 * デバイスより読み込み(連射つき)
 */
static DWORD FASTCALL GetRapidJoy(int index, BOOL flag)
{
	int            i;
	DWORD bit;
	DWORD dat;

	/*
	 * assert
	 */
	ASSERT((index == 0) || (index == 1));

	/*
	 * 非接続チェック1 (接続チェック時は通す)
	 */
	if ((!flag) && (!joyplugged[index])) {
		return 0x00000000;
	}

	/*
	 * データ取得
	 */
	if(index >= MAX_SDL_JOY) return 0x00000000;
	if(SDLDrv == NULL) return 0x00000000; /* ドライバ初期化前にはダミー返す */
	dat = SDLDrv[index].GetJoyButton(flag);

	/*
	 * 非接続チェック2
	 */
	if (!joyplugged[index]) {
		return 0x00;
	}

	/*
	 * ボタンチェック
	 */
	bit = 0x00010000;
	for (i = 0; i < _JOY_MAX_BUTTONS; i++) {
		if ((dat & bit) && (nJoyRapid[index][i] > 0)) {

			/*
			 * 連射ありで押されている。カウンタチェック
			 */
			if (joyrapid[index][i] == 0) {

				/*
				 * 初期カウンタを代入
				 */
				joyrapid[index][i] =
						JoyRapidCounter[nJoyRapid[index][i]];
			}

			else {

				/*
				 * カウンタデクリメント
				 */
				joyrapid[index][i]--;
				if ((joyrapid[index][i] & 0xff) == 0) {

					/*
					 * 反転タイミングなので、時間を加算して反転
					 */
					joyrapid[index][i] +=
							JoyRapidCounter[nJoyRapid[index][i]];
					joyrapid[index][i] ^= 0x100;
				}
			}

			/*
			 * ボタンが押されていないように振る舞う
			 */
			if (joyrapid[index][i] >= 0x100) {
				dat &= (DWORD) (~bit);
			}
		}

		else {

			/*
			 * ボタンが押されてないので、連射カウンタクリア
			 */
			joyrapid[index][i] = 0;
		}

		/*
		 * 次のビットへ
		 */
		bit <<= 1;
	}
//        printf("DBG:Joy:Button 0x%08x\n", dat);
	return dat;
}

static DWORD FASTCALL GetAxis(int index, BOOL flag)
{
	/*
	 * assert
	 */
	ASSERT((index == 0) || (index == 1));

	/*
	 * 非接続チェック1 (接続チェック時は通す)
	 */
	if ((!flag) && (!joyplugged[index])) {
		return 0x00000000;
	}

	/*
	 * データ取得
	 */
   if(index >= MAX_SDL_JOY) return 0x00000000;
   if(SDLDrv == NULL) return 0x00000000; /* ドライバ初期化前にはダミー返す */
   return SDLDrv[index].GetJoyAxis(flag);
}
   
void InitJoyCode(int *p)
{
	p[0] = 0x70; // 上
	p[1] = 0x71; // 下
	p[2] = 0x72; // 左
	p[3] = 0x73; // 右
	p[4] = 0x00; // 
	p[5] = 0x74; // ボタン1
	p[6] = 0x75; // ボタン2
        p[7] = 0x77; // リザーブ
}
/*
 *  ジョイスティック コード変換
 */
static DWORD FASTCALL PollJoyCode(int code)
{

	/*
	 * 0x70未満は無視
	 */
	if (code < 0x70) {
		return 0;
	}

	/*
	 * 0x70から上下左右
	 */
	switch (code) {

	/*
	 * 上
	 */
	case 0x70:
		return 0x01;

		/*
		 * 下
		 */
	case 0x71:
		return 0x02;

		/*
		 * 左
		 */
	case 0x72:
		return 0x04;

		/*
		 * 右
		 */
	case 0x73:
		return 0x08;

		/*
		 * Aボタン
		 */
	case 0x74:
		return 0x10;

		/*
		 * Bボタン
		 */
	case 0x75:
		return 0x20;

		/*
		 * それ以外
		 */
	default:
//		ASSERT(FALSE);
		break;
	}
	return 0;
}


/*
 *  ジョイスティック
 * ポーリング(ジョイスティック)
 */
static DWORD  FASTCALL PollJoySub(int index, DWORD axis, DWORD dat)
{
	int            i;
	DWORD ret;
	DWORD bit;

	/*
	 * assert
	 */
	ASSERT((index == 0) || (index == 1));

	/*
	 * 終了データクリア
	 */
	ret = 0;

	/*
	 * 方向
	 */
	bit = 0x00000001;
	for (i = 0; i < 4; i++) {

		/*
		 * ボタンが押されているか
		 */
		if (axis & bit) {

			/*
			 * コード変換
			 */
			ret |= PollJoyCode(nJoyCode[index][i]);
		}
		bit <<= 1;
	}

	/*
	 * センターチェック
	 */
	if ((axis & 0x0f) == 0) {
		if ((joybkaxis[index] & 0x0f) != 0) {
			ret |= PollJoyCode(nJoyCode[index][4]);
		}
	} 
        joybkaxis[index] = axis;
   
	/*
	 * ボタン
	 */
        bit = 0x00010000;
	if (dat & bit) {
		ret |= PollJoyCode(nJoyCode[index][5]);
	}
        bit <<= 1;
	if (dat & bit) {
		ret |= PollJoyCode(nJoyCode[index][6]);
	}
        bit <<= 1;
	if (dat & bit) {
		ret |= PollJoyCode(nJoyCode[index][5]);
	}
        bit <<= 1;
	if (dat & bit) {
		ret |= PollJoyCode(nJoyCode[index][6]);
	}
        bit <<= 1;
//	 printf("Joy: %08x %08x %08x\n", axis, dat, ret);
        joybkbutton[index] = dat;
	return ret;
}

static void PollJoyKbdSub(int index, DWORD axis, DWORD dat, BYTE MakeBreak)
{
   DWORD db;
   switch(axis & 0x0f)
	{
	case 1: /* 上 */
		PushKeyData(nJoyKeyCode[index][0], MakeBreak);
		break;
	case 2: /* 下 */
		PushKeyData(nJoyKeyCode[index][1], MakeBreak);
		break;
	case 4: /* 左 */
		PushKeyData(nJoyKeyCode[index][2], MakeBreak);
		break;
	case 8: /* 右 */
		PushKeyData(nJoyKeyCode[index][3], MakeBreak);
		break;
	case 5: /* 左上 */
		PushKeyData(nJoyKeyCode[index][8], MakeBreak);
		break;
	case 9: /* 右上 */
		PushKeyData(nJoyKeyCode[index][9], MakeBreak);
		break;
	case 6: /* 左下 */
		PushKeyData(nJoyKeyCode[index][10], MakeBreak);
		break;
	case 10: /* 右下 */
		PushKeyData(nJoyKeyCode[index][11], MakeBreak);
		break;
	default:
	case 0: /* 押されてない */
		PushKeyData(nJoyKeyCode[index][4], MakeBreak);
		break;
	}
        db =  (dat & 0xffff0000) >> 16;
        switch(db) {
	case 0x01: 
		PushKeyData(nJoyKeyCode[index][5], MakeBreak);
		break;
	case 0x02: 
		PushKeyData(nJoyKeyCode[index][6], MakeBreak);
		break;
	case 0x04: 
		PushKeyData(nJoyKeyCode[index][12], MakeBreak);
		break;
	case 0x08: 
		PushKeyData(nJoyKeyCode[index][13], MakeBreak);
		break;
	case 0x10: 
		PushKeyData(nJoyKeyCode[index][14], MakeBreak);
		break;
	case 0x20: 
		PushKeyData(nJoyKeyCode[index][15], MakeBreak);
		break;
	}
   
//        printf("DBG:Joy:Button 0x%08x\n", axis);
	   

}

/*
 *  ジョイスティック ポーリング(キーボード)
 */
static void PollJoyKbd(int index, DWORD axis, DWORD dat)
{
	DWORD bit;
	BYTE diff;
	int            i;


	/*
	 * 上下左右
	 */

	bit = 0x00000001;
	diff = axis ^ joybkaxis[index];
	if(diff != 0) {
		/*
		 * 前キーbreak
		 */
		PollJoyKbdSub(index, joybkaxis[index], dat, 0x00);
		PollJoyKbdSub(index, axis , dat, 0x80);
	        joybkaxis[index] = axis;
	} else {
		if(((axis & 0xff) == 0) && (axis != joybkaxis[index])){
			/*
			 * 強制的に方向キー解除
			 */
		   PollJoyKbdSub(index, joybkaxis[index], dat, 0x00);
		   PollJoyKbdSub(index, 0, dat, 0x80);
		   joybkaxis[index] = 0;
		}  else {
//		   if((joybkaxis[index] & 0xff) == 0) PollJoyKbdSub(index, 0, dat, 0x00);
		   joybkaxis[index] = axis;
		}
   
	}
	/*
	 * ボタン
	 */
	bit = 0x00010000;
	for (i = 0; i < _JOY_MAX_BUTTONS; i++) {
		if (dat & bit) {

			/*
			 * 初めて押さたら、make発行
			 */
			if ((joybkbutton[index] & bit) == 0) {
			         if(i >= 2) {
				    PushKeyData(nJoyKeyCode[index][i + 12 - 2], 0x80);
				 } else {
				    PushKeyData(nJoyKeyCode[index][i + 5], 0x80);
				 }
			   
				
//				}
			}
		}

		else {

			/*
			 * 初めて離されたら、break発行
			 */
			if ((joybkbutton[index] & bit) != 0) {
			         if(i >= 2) {
				    PushKeyData(nJoyKeyCode[index][i + 12 - 2], 0x00);
				 } else {
				    PushKeyData(nJoyKeyCode[index][i + 5], 0x00);
				 }
			}
		}
		bit <<= 1;
	}
        joybkbutton[index] = dat;
}


/*
 *  ジョイスティック ポーリング
 */
void FASTCALL PollJoy(void)
{
	DWORD dat;
        DWORD axis;
	BOOL check;
	int            i;

	/*
	 * 間隔チェック(ポーリング用)
	 */
	if ((dwExecTotal - joytime) < 10000) {
		return;
	}
	joytime = dwExecTotal;

	/*
	 * データをクリア
	 */
	memset(joydat, 0, sizeof(joydat));
//	memset(joyaxis, 0, sizeof(joyaxis));

	/*
	 * 無効チェック
	 */
	if ((nJoyType[0] == 0) && (nJoyType[1] == 0)) {
		return;
	}

	/*
	 * 間隔チェック(接続チェック用)
	 */
	if ((dwExecTotal - joytime2) >= 500000) {
		joytime2 = dwExecTotal;
		check = TRUE;
	}

	else {
		check = FALSE;
	}

	/*
	 * デバイスループ
	 */
	for (i = 0; i < _JOY_MAX_PLUG; i++) {

		/*
		 * データ取得(連射つき)
		 */
		dat = GetRapidJoy(i, check);
	        axis = GetAxis(i, check); 
		/*
		 * タイプ別
		 */
		switch (nJoyType[i]) {
		/*
		 * ジョイスティックポート1
		 */
		case 1:
			joydat[0] = PollJoySub(i, axis, dat);
			break;

			/*
			 * ジョイスティックポート2
			 */
		case 2:
			joydat[1] = PollJoySub(i, axis, dat);
			break;

			/*
			 * キーボード
			 */
		case 3:
			PollJoyKbd(i, axis, dat);
			break;

			/*
			 * 電波新聞社ジョイスティック
			 */
		case 4:
			joydat[2] = PollJoySub(i, axis, dat);
			break;
		}

		/*
		 * データ更新
		 */
	}
}


/*
 *  ジョイスティック データリクエスト
 */
BYTE FASTCALL joy_request(BYTE no)
{

	/*
	 * ASSERT((no >= 0) && (no < 3));
	 */
	ASSERT(no < 3);
//	printf("JOY: %02x %02x %02x\n", joydat[0], joydat[1], joydat[2]);
	return joydat[no];
}

/*
 * 仮のものです
 */
static void OpenJoyInit(void)
{
	int i;
	for(i = 0;i<MAX_SDL_JOY; i++) SDLDrv[i].Open(i);
//    SDL_JoystickEventState(SDL_ENABLE);
}
/*
 * ジョイスティック初期化
 */

BOOL FASTCALL InitJoy(void)
{
	int index;
	/*
	 * ワークエリア初期化(ジョイスティック)
	 */
    joytime = 0;
    joytime2 = 0;
    memset(joydat, 0, sizeof(joydat));
    memset(joybkaxis, 0, sizeof(joybkaxis));
    memset(joybkbutton, 0, sizeof(joybkbutton));
    memset(joyrapid, 0, sizeof(joyrapid));
    joyplugged[0] = TRUE;
    joyplugged[1] = FALSE;

	/*
	 * ワークエリア初期化(JS)
	 */
    nJoyType[1] = 0;
    nJoyType[0] = 1;
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    memset(joydat, 0, sizeof(joydat));
    joyplugged[0] = TRUE;
    joyplugged[1] = TRUE;

    /*
     * キーコードエミュレーション領域に初期値入れる
     */
    for(index = 0; index < 2 ; index++) {
    	nJoyKeyCode[index][0] = 0x3b;/* 上 : KP8*/
    	nJoyKeyCode[index][1] = 0x43;/* 下  : KP2*/
    	nJoyKeyCode[index][2] = 0x3e;/* 左 : KP4*/
    	nJoyKeyCode[index][3] = 0x40;/* 右 : KP6*/
    	nJoyKeyCode[index][4] = 0x3f;/* センター : KP5 */
    	nJoyKeyCode[index][5] = 0x35;/* ボタン1 : 右SPACE*/
    	nJoyKeyCode[index][6] = 0x5c;/* ボタン2 : BREAK*/
    	nJoyKeyCode[index][8] = 0x3a;/* 左上 : KP7*/
    	nJoyKeyCode[index][9] = 0x3c;/* 右上  : KP9*/
    	nJoyKeyCode[index][10] = 0x42;/* 左下 : KP1*/
    	nJoyKeyCode[index][11] = 0x44;/* 右下 : KP3*/
        nJoyKeyCode[index][12] = 0x2a;/* ボタン3: Z */
        nJoyKeyCode[index][13] = 0x2b;/* ボタン4: X */
        nJoyKeyCode[index][14] = 0x2c;/* ボタン5: C */
        nJoyKeyCode[index][15] = 0x56;/* ボタン6: GRPH */
    	// 内部コード初期化
    	InitJoyCode(nJoyCode[index]);
    }
    SDLDrv = new SDLJoyInterface[MAX_SDL_JOY];
/*
 * ジョイスティック初期化
 */
    OpenJoyInit();

	return TRUE;
}



/*
 * ジョイスティック分離
 */
void FASTCALL cleanJoy(void)
{
    SDL_JoystickEventState(SDL_IGNORE);
    delete [] SDLDrv;
}


/*
 *  JS関連イベントハンドラ
 */
BOOL OnMoveJoy(SDL_Event * event)
{

	/*
	 * 感度設定とかリダイレクトとかあるけど取り合えず後;)
	 */
	int 			i;
	for(i = 0; i< MAX_SDL_JOY; i++) SDLDrv[i].OnMove(event);
	return TRUE;
}

BOOL OnPressJoy(SDL_Event * event)
{

	/*
	 * 感度設定とかリダイレクトとかあるけど取り合えず後;)
	 */
	int 			i;
	for(i = 0; i< MAX_SDL_JOY; i++) SDLDrv[i].OnPress(event);
	return TRUE;
}

BOOL OnReleaseJoy(SDL_Event * event)
{

	/*
	 * 感度設定とかリダイレクトとかあるけど取り合えず後;)
	 */
	int 			i;
	for(i = 0; i< MAX_SDL_JOY; i++) SDLDrv[i].OnRelease(event);
	return TRUE;
}
#ifdef __cplusplus
}
#endif

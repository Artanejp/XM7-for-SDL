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


int            nJoyType[2];	/* ジョイスティックタイプ */
int            nJoyRapid[2][2];	/* 連射タイプ */
int            nJoyCode[2][7];	/* 生成コード */
//BYTE nJoyRaw[MAX_SDL_JOY];

static BYTE   joydat[3];	/* ジョイスティックデータ */
static BYTE    joybk[2];	/* ジョイスティックバックアップ */
static int     joyrapid[2][2];	/* ジョイスティック連射カウンタ */
static DWORD   joytime;	/* ジョイスティックポーリング時間	 */
static DWORD   joytime2;	/* ジョイスティックポーリング時間	 */
static BOOL    joyplugged[2];	/* ジョイスティック接続フラグ	 */
static SDLJoyInterface *SDLDrv; /* SDL JSドライバー */
static BYTE nJoyKeyCode[MAX_SDL_JOY][16]; /* ジョイスティックキーコードアサイン */


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
static BYTE     FASTCALL
GetRapidJoy(int index, BOOL flag)
{
	int            i;
	BYTE bit;
	BYTE dat;

	/*
	 * assert
	 */
	ASSERT((index == 0) || (index == 1));

	/*
	 * 非接続チェック1 (接続チェック時は通す)
	 */
	if ((!flag) && (!joyplugged[index])) {
		return 0x00;
	}

	/*
	 * データ取得
	 */
	if(index >= MAX_SDL_JOY) return 0x00;
	if(SDLDrv == NULL) return 0x00; /* ドライバ初期化前にはダミー返す */
	dat = SDLDrv[index].GetJoy(flag);

	/*
	 * 非接続チェック2
	 */
	if (!joyplugged[index]) {
		return 0x00;
	}

	/*
	 * ボタンチェック
	 */
	bit = 0x10;
	for (i = 0; i < 2; i++) {
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
				dat &= (BYTE) (~bit);
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
	return dat;
}

/*
 *  ジョイスティック 拡張ボタン
 * デバイスより読み込み(連射つき)
 */
static BYTE FASTCALL GetRapidJoyExt(int index, BOOL flag)
{
	int            i;
	BYTE bit;
	BYTE dat;

	/*
	 * assert
	 */
	ASSERT((index == 0) || (index == 1));

	/*
	 * 非接続チェック1 (接続チェック時は通す)
	 */
	if ((!flag) && (!joyplugged[index])) {
		return 0x00;
	}

	/*
	 * データ取得
	 */
	if(index >= MAX_SDL_JOY) return 0x00;
	dat = SDLDrv[index].GetJoyExt(flag);

	/*
	 * 非接続チェック2
	 */
	if (!joyplugged[index]) {
		return 0x00;
	}

	/*
	 * ボタンチェック
	 */
	bit = 0x10;
	for (i = 0; i < 2; i++) {
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
				dat &= (BYTE) (~bit);
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
	return dat;
}


void InitJoyCode(int *p)
{
	p[0] = 0x70; // 上
	p[1] = 0x71; // 下
	p[2] = 0x72; // 左
	p[3] = 0x73; // 右
	p[4] = 0x74; // ボタン1
	p[5] = 0x75; // ボタン2
	p[6] = 0x76; // リザーブ
}
/*
 *  ジョイスティック コード変換
 */
static BYTE     FASTCALL
PollJoyCode(int code)
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
		ASSERT(FALSE);
		break;
	}
	return 0;
}


/*
 *  ジョイスティック
 * ポーリング(ジョイスティック)
 */
static BYTE     FASTCALL
PollJoySub(int index, BYTE dat)
{
	int            i;
	BYTE ret;
	BYTE bit;

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
	bit = 0x01;
	for (i = 0; i < 4; i++) {

		/*
		 * ボタンが押されているか
		 */
		if (dat & bit) {

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
	if ((dat & 0x0f) == 0) {
		if ((joybk[index] & 0x0f) != 0) {
			ret |= PollJoyCode(nJoyCode[index][4]);
		}
	}

	/*
	 * ボタン
	 */
	if (dat & 0x10) {
		ret |= PollJoyCode(nJoyCode[index][5]);
	}
	if (dat & 0x20) {
		ret |= PollJoyCode(nJoyCode[index][6]);
	}

	// printf("Joy: %02x %02x\n", dat, ret);
	return ret;
}

static void
PollJoyKbdSub(int index, BYTE dat, BYTE MakeBreak)
{
	switch(dat & 0x0f)
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
}

/*
 *  ジョイスティック ポーリング(キーボード)
 */
static void
PollJoyKbd(int index, BYTE dat)
{
	BYTE bit;
	BYTE diff;
	int            i;


	/*
	 * 上下左右
	 */

	bit = 0x01;
	diff = (dat ^ joybk[index]) & 0x0f;
	if(diff != 0) {
		/*
		 * 前キーbreak
		 */
		PollJoyKbdSub(index, joybk[index] & 0x0f, 0x00);
		PollJoyKbdSub(index, dat & 0x0f, 0x80);
	} else {
		if((dat & 0x0f) == 0){
			/*
			 * 強制的に方向キー解除
			 */
			PushKeyData(nJoyKeyCode[index][4], 0x00);
		}
	}
	/*
	 * ボタン
	 */
	bit = 0x10;
	for (i = 0; i < 2; i++) {
		if (dat & bit) {

			/*
			 * 初めて押さたら、make発行
			 */
			if ((joybk[index] & bit) == 0) {
//				if ((nJoyCode[index][i + 5] > 0)
//						&& (nJoyCode[index][i + 5] <= 0x66)) {
					PushKeyData(nJoyKeyCode[index][i + 5], 0x80);
//				}
			}
		}

		else {

			/*
			 * 初めて離されたら、break発行
			 */
			if ((joybk[index] & bit) != 0) {
//				if ((nJoyCode[index][i + 5] > 0)
//						&& (nJoyCode[index][i + 5] <= 0x66)) {
					PushKeyData(nJoyKeyCode[index][i + 5], 0x00);
//				}
			}
		}
		bit <<= 1;
	}
	/* ここに拡張ボタンを */

	joybk[index] = dat;
}


/*
 *  ジョイスティック ポーリング
 */
void            FASTCALL
PollJoy(void)
{
	BYTE dat;
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
	for (i = 0; i < 2; i++) {

		/*
		 * データ取得(連射つき)
		 */
		dat = GetRapidJoy(i, check);

		/*
		 * タイプ別
		 */
		switch (nJoyType[i]) {
		/*
		 * ジョイスティックポート1
		 */
		case 1:
			joydat[0] = PollJoySub(i, dat);
			break;

			/*
			 * ジョイスティックポート2
			 */
		case 2:
			joydat[1] = PollJoySub(i, dat);
			break;

			/*
			 * キーボード
			 */
		case 3:
			PollJoyKbd(i, dat);
			break;

			/*
			 * 電波新聞社ジョイスティック
			 */
		case 4:
			joydat[2] = PollJoySub(i, dat);
			break;
		}

		/*
		 * データ更新
		 */
		joybk[i] = dat;
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
    SDL_JoystickEventState(SDL_ENABLE);
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
    memset(joybk, 0, sizeof(joybk));
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
    memset(joybk, 0, sizeof(joybk));
    memset(joyrapid, 0, sizeof(joyrapid));
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

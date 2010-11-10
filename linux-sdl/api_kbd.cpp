/*
 * api_kbd.cpp
 *
 *　キーボード共通API
 *  Created on: 2010/09/17
 *      Author: K.Ohta<whatisthis.sowhat@gmail.com>
 */
extern "C" {
#ifdef USE_GTK
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#endif
#include <memory.h>
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
#include "KbdInterface.h"
#include "SDLKbdInterface.h"
#ifdef USE_GTK
#include "GtkKbdInterface.h"
#include "gtk_propkeyboard.h"
#endif
#ifdef USE_AGAR
#include "AgarKbdInterface.h"
#endif

BOOL bKbdReal;			/* 疑似リアルタイムキースキャン
 */
BOOL bTenCursor;		/* テンキー変換 */
BOOL bArrow8Dir;		/* テンキー変換 8方向モード */
BYTE   kibuf[256];	/* キーボード押下状態 */
BYTE   kbd_map[256];

/*
 *  スタティック ワーク
 */
static DWORD    keytime;	/* キーボードポーリング時間 */
static DWORD   dmyreadtime;	/* キーコードダミーリードタイマ
 */
static BOOL    bDummyRead;	/* キーコードダミーリードフラグ
 */
static BYTE    nKeyBuffer[KEYBUFFER_SIZE];	/* 内部キーバッファ
 */
static BYTE    nKeyReadPtr;	/* 内部キーバッファ読出ポインタ
 */
static BYTE    nKeyWritePtr;	/* 内部キーバッファ書込ポインタ
 */
static BYTE    nLastKey;	/* 最後に押されたキー(FULL) */
static BYTE    nLastKey2;	/* 最後に押されたキー(10KEY) */
static BYTE    nTenDir;	/* テンキー変換 方向データ */
static BYTE    nTenDir2;	/* テンキー変換 方向データ2 */
static SDL_sem *KeySem; /* キーバッファセマフォ */
static BOOL bCursorGrabbed;

/*
 * ドライバクラス
 */
static SDLKbdInterface *SDLDrv;
#ifdef USE_GTK
static GtkKbdInterface *GTKDrv;
#endif
#ifdef USE_AGAR
static AgarKbdInterface *AGARDrv;
#endif



/*
 *  カーソルキー → テンキー変換
 * 物理コード対照表
 */
static BYTE     TenDirTable[16] =
{ 0x00, 0x3b, 0x43, 0xff, 0x3e, 0x3a, 0x42, 0x3e, 0x40, 0x3c, 0x44,
		0x40, 0xff, 0x3b, 0x43, 0xff
};

/*
 *  初期化
 */
void            FASTCALL
InitKbd(void)
{

#ifdef MOUSE
	/*
	 *  透明カーソルイメージ
	 */
#endif				/*  */
	/*
	 * SDL
	 */

	/*
	 * ワークエリア初期化(キーボード)
	 */
	memset(kibuf, 0, sizeof(kibuf));
	memset(kbd_map, 0, sizeof(kbd_map));

	keytime = 0;
	/*
	 * ここにドライバー作成する
	 */
	SDLDrv = new SDLKbdInterface();
#ifdef USE_GTK
	GTKDrv = new GtkKbdInterface();
#endif
#ifdef USE_AGAR
	AGARDrv = new AgarKbdInterface();
#endif
	/*
	 * ワークエリア初期化(キーボード NT/PC-9801対策)
	 */
	bDummyRead = TRUE;
	dmyreadtime = 0;


	/*
	 * ワークエリア初期化(内部キーバッファ/疑似リアルタイムキースキャン)
	 */
	memset(nKeyBuffer, 0, sizeof(nKeyBuffer));
	nKeyReadPtr = 0;
	nKeyWritePtr = 0;
	nLastKey = 0;
	nLastKey2 = 0;
	bKbdReal = FALSE;
	//GetDefMapKbd(kbd_table, 0);	/* デフォルトキーマップ読む     */
	kbd_snooped = FALSE; /* キーボードは通常こちらで処理する */

	/*
	 * ワークエリア初期化(テンキー変換)
	 */
	bArrow8Dir = TRUE;
	nTenDir = 0;
	nTenDir2 = 0;

	bCursorGrabbed = FALSE;
	/*
	 * テンキーエミュレーション
	 */
	bTenCursor = FALSE;
	/*
	 * キーボード入力に排他かける
	 */
	if(KeySem == NULL) {
		KeySem = SDL_CreateSemaphore(1);
		if(KeySem == NULL) return;
	}

	/*
	 * ジョイスティック初期化(やる？）
	 */
	// printf("KBDInit\n");

}

/*
 *  クリーンアップ
 */
void            FASTCALL
CleanKbd(void)
{
	if(SDLDrv != NULL) delete SDLDrv;
#ifdef USE_GTK
	if(GTKDrv != NULL) delete GTKDrv;
#endif
#ifdef USE_AGAR
	if(AGARDrv != NULL) delete AGARDrv;
#endif
	if(KeySem != NULL){
		SDL_DestroySemaphore(KeySem);
		KeySem = NULL;
	}

}
/*
 *  セレクト
 */
BOOL
SelectKbd(void)
{
	return TRUE;
}


void
PushKeyData(Uint8 code,Uint8 MakeBreak)
{
	if(KeySem == NULL) return; // セマフォなければ何もしない
	SDL_SemWait(KeySem);
	if(MakeBreak == 0x00) {
		kibuf[code] = 0x00;
	} else {
		if (kibuf[code] != 0x80) {
			kibuf[code] = 0x80;
		}
	}
//	printf("KEY: PUSH #%02x as %02x\n", code, MakeBreak);
	SDL_SemPost(KeySem);
}


/*
 *  キーボード 内部キーバッファ登録
 */
static void
PushKeyCode(BYTE code)
{
	int            i;
	if (nKeyWritePtr < KEYBUFFER_SIZE) {
		nKeyBuffer[nKeyWritePtr++] = code;
	}else {
		for (i = 0; i < KEYBUFFER_SIZE - 1; i++) {
			nKeyBuffer[i] = nKeyBuffer[i + 1];
		}
		nKeyBuffer[KEYBUFFER_SIZE - 1] = code;
		if (nKeyReadPtr > 0) {
			nKeyReadPtr--;
		}
	}
}


/*
 *  キーボード テンキー変換  方向コード変換
 */
static BYTE     FASTCALL
Cur2Ten_DirCode(BYTE code)
{
	switch (code) {
	case 0x4d:		/* ↑ */
		return 0x01;
	case 0x4f:		/* ← */
		return 0x04;
	case 0x50:		/* ↓ */
		return 0x02;
	case 0x51:		/* → */
		return 0x08;
	}
	return 0;
}


/*
 *  キーボード テンキー変換  マスクコード変換
 */
static BYTE     FASTCALL
Cur2Ten_MaskCode(BYTE code)
{
	switch (code) {
	case 0x4d:		/* ↑ */
	case 0x50:		/* ↓ */
		return 0xfc;
	case 0x4f:		/* ← */
	case 0x51:		/* → */
		return 0xf3;
	}
	return 0xff;
}


/*
 *  キーボード カーソルキー→テンキー変換 Make
 */
static BOOL     FASTCALL
Cur2Ten_Make(BYTE code)
{
	BYTE nTenDirOld;
	BYTE dircode;

	/*
	 * 以前の方向コードを保存
	 */
	nTenDirOld = nTenDir;

	/*
	 * キーコード変換
	 */
	dircode = Cur2Ten_DirCode(code);
	if (dircode) {
		if (bArrow8Dir) {

			/*
			 * 8方向
			 */
			nTenDir2 =
					(BYTE) (nTenDir & (BYTE) ~ Cur2Ten_MaskCode(code));
			nTenDir &= Cur2Ten_MaskCode(code);
			nTenDir |= dircode;
		} else {
			if (key_format == KEY_FORMAT_SCAN) {
				if (TenDirTable[dircode]) {
					keyboard_make(TenDirTable[dircode]);
				}
				return TRUE;
			}

			/*
			 * 4方向
			 */
			nTenDir2 =
					(BYTE) (nTenDir & (BYTE) Cur2Ten_MaskCode(code));
			nTenDir = dircode;
		}

		/*
		 * テーブル内容が0xffの場合、無効な組み合わせ
		 */
		if (TenDirTable[nTenDir] == 0xff) {
			nTenDir = nTenDirOld;
			return TRUE;
		}

		/*
		 * キーコード発行
		 */
		if (nTenDir != nTenDirOld) {
			if (TenDirTable[nTenDirOld]) {
				keyboard_break(TenDirTable[nTenDirOld]);
				PushKeyCode(TenDirTable[nTenDir]);
			} else {
				keyboard_make(TenDirTable[nTenDir]);
			}
		}
		return TRUE;
	}
	return FALSE;
}


/*
 *  キーボード カーソルキー→テンキー変換 Break
 */
static BOOL     FASTCALL
Cur2Ten_Break(BYTE code)
{
	BYTE nTenDirOld;
	BYTE dircode;

	/*
	 * 以前の方向コードを保存
	 */
	nTenDirOld = nTenDir;

	/*
	 * キーコード変換
	 */
	dircode = Cur2Ten_DirCode(code);
	if (dircode) {
		if ((key_format == KEY_FORMAT_SCAN) && !bArrow8Dir) {
			if (TenDirTable[dircode]) {
				keyboard_break(TenDirTable[dircode]);
			}
			return TRUE;
		}
		nTenDir2 &= (BYTE) ~ dircode;
		if (nTenDir2) {
			nTenDir |= nTenDir2;
			nTenDir2 = 0;
		}
		nTenDir &= (BYTE) ~ dircode;

		/*
		 * テーブル内容が0xffの場合、無効な組み合わせ
		 */
		if (TenDirTable[nTenDir] == 0xff) {
			nTenDir = nTenDirOld;
			return TRUE;
		}

		/*
		 * キーコード発行
		 */
		if (nTenDir != nTenDirOld) {
			keyboard_break(TenDirTable[nTenDirOld]);
			if (TenDirTable[nTenDir]) {
				PushKeyCode(TenDirTable[nTenDir]);
			} else if (key_format != KEY_FORMAT_SCAN) {

				/*
				 * 停止時は"5"を発行
				 */
				PushKeyCode(0x3f);
				PushKeyCode(0xbf);
			}
		}
		return TRUE;
	}
	return FALSE;
}


/*
 *  キーボード ポーリング
 */
void
PollKbd(void)
{
	//    BYTE buf[256];
	int            i;
	BYTE fm7;
	BOOL bFlag;
	static BYTE    key_last = 0;

	/*
	 * アクティベートチェックはSDLの場合不要かも?
	 */
	/*
	 * アクティベートチェック
	 */
	// if (!bActivate) {
	// /* 非アクティブ時はダミーリードタイマ更新
	// */
	// bDummyRead = TRUE;
	// dmyreadtime = dwExecTotal;
	// return;
	// }

	/*
	 * ダミーリード時間チェック
	 */
	if (bDummyRead && ((dwExecTotal - dmyreadtime) > 100000)) {
		bDummyRead = FALSE;
	}

	/*
	 * 時間チェック
	 */
	if ((dwExecTotal - keytime) < 20000) {
		return;
	}
	keytime = dwExecTotal;

	/*
	 * キーバッファ内にコードがたまっている場合、先に処理
	 */
	if (nKeyReadPtr < nKeyWritePtr) {
		fm7 = nKeyBuffer[nKeyReadPtr++];
		if (fm7 == 0xff) {

			/*
			 * 0xff : キーリピートタイマ変更
			 */
			keyboard_repeat();
		}	else if (fm7 & 0x80) {

			/*
			 * 0x80-0xfe : Breakコード
			 */
			keyboard_break((BYTE) (fm7 & 0x7f));
		} else {
			/*
			 * 0x00-0x7f : Makeコード
			 */
			keyboard_make(fm7);
		}
		return;
	}	else {
		nKeyReadPtr = 0;
		nKeyWritePtr = 0;
	}

	/*
	 * フラグoff
	 */
	bFlag = FALSE;

	/*
	 * 今までの状態と比較して、順に変換する
	 */
	for (i = 0; i < sizeof(kibuf); i++) {
		if (((kibuf[i] & 0x80) != (kbd_map[i] & 0x80)) && (!bFlag)) {
			if (kibuf[i] & 0x80) {
				/*
				 * キー押下
				 */
				fm7 = (BYTE) (i & 0x7f);
				if (fm7 > 0) {
					if (TRUE) {
						if (!bDummyRead) {
							if ((fm7 >= 0x4d) && (fm7 <= 0x51)
									&& bTenCursor) {

								/*
								 * スキャンモード時は本来のコードも発行
								 */
								if ((key_format == KEY_FORMAT_SCAN)
										&& (fm7 != 0x4e)) {
									PushKeyCode(fm7);
								}

								/*
								 * スキャンコード変換
								 */
								if (Cur2Ten_Make(fm7)) {
									fm7 = 0;
								}
							}
							if (fm7) {
								keyboard_make(fm7);
							}
							bDummyRead = FALSE;

							/*
							 * 疑似リアルタイムキースキャン
							 */
							if (bKbdReal   && (key_format != KEY_FORMAT_SCAN)) {
								if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
									// PushKeyCode((BYTE)(fm7 |
									// 0x80));

									/*
									 * 以前押されていたキーがあれば再発行
									 */
									if (nLastKey && key_repeat_flag) {
										PushKeyCode(nLastKey);
										// PushKeyCode(0xff);
									}
									PushKeyCode(0xff);

									/*
									 * 再発行用にキーコードを記憶
									 */
									if (fm7 == 0x3f) {
										nLastKey2 = 0;
									}   else {
										nLastKey2 = fm7;
									}
								} else {
									/*
									 * 再発行用にキーコードを記憶
									 */
									if (fm7 != 0x5c) {
										nLastKey = fm7;
									} else {
										nLastKey = 0;
									}
								}
							}
						}
						if (fm7 != 0x5c) {
							key_last = fm7;
						}
						bFlag = TRUE;
					}
				}
			} else {

				/*
				 * キー離した
				 */
				fm7 = (BYTE) (i & 0x7f);
				if (fm7 > 0) {
					if ((fm7 >= 0x4d) && (fm7 <= 0x51) && bTenCursor) {
						/*
						 * スキャンモード時は本来のコードも発行
						 */
						if ((key_format == KEY_FORMAT_SCAN)
								&& (fm7 != 0x4e)) {
							PushKeyCode((BYTE) (fm7 | 0x80));
						}
						/*
						 * スキャンコード変換
						 */
						if (Cur2Ten_Break(fm7)) {
							fm7 = 0;
						}
					}
					/*
					 * 疑似リアルタイムキースキャン
					 */
					if (bKbdReal && (key_format != KEY_FORMAT_SCAN)) {
						if ((fm7 >= 0x3a) && (fm7 <= 0x46)) {
							/*
							 * テンキーの場合
							 */
							PushKeyCode((BYTE) (fm7 | 0x80));
							if (nLastKey2 == fm7) {
								PushKeyCode(0x3f);
								PushKeyCode(0xbf);
								nLastKey2 = 0;
							}

							/*
							 * 以前テンキー以外が押されていた場合、再発行
							 */
							if (nLastKey && key_repeat_flag) {
								PushKeyCode(nLastKey);
								PushKeyCode(0xff);
								key_last = nLastKey;
							}
						} else {
							/*
							 * テンキー以外の場合
							 */
							keyboard_break(fm7);
							if (nLastKey == fm7) {
								nLastKey = 0;
							}

							/*
							 * 以前テンキーが押されていた場合、再発行
							 */
							if (nLastKey2 && (nLastKey2 != key_last)) {
								PushKeyCode(nLastKey2);
								PushKeyCode(0xff);

								// PushKeyCode((BYTE)(nLastKey2 |
								// 0x80));
								key_last = nLastKey2;
							}
						}
					}

					else if (fm7) {
						keyboard_break(fm7);
					}
					bFlag = TRUE;
				}
			}

			/*
			 * データをコピー
			 */
			kbd_map[i] = kibuf[i];
		}
	}
}


/*
 *  キーボード ポーリング＆キー情報取得
 * ※VMのロックは行っていないので注意
 */
BOOL
GetKbd(BYTE * pBuf)
{
	//    ASSERT(pBuf);

	/*
	 * メモリクリア
	 */
	memset(pBuf, 0, 256);
	return TRUE;
}

/*
 * ここから先、Toolkit依存Wrapper
 */
/**[ アクションイベント ]***********************************************/

#ifdef USE_GTK
    /*
     *  キープレスアクション
     */
gboolean OnKeyPressGtk(GtkWidget * widget, GdkEventKey * event,
		    gpointer data)
{
	if(GTKDrv == NULL) return FALSE;
	GTKDrv->OnPress((void *)event);
    return TRUE;
}


/*
 *  キーリリースアクション
 */
gboolean OnKeyReleaseGtk(GtkWidget * widget, GdkEventKey * event,
		gpointer data)
{
	if(GTKDrv == NULL) return FALSE;
	GTKDrv->OnRelease((void *)event);
    return TRUE;
}
#endif

/*
 * SDL
 */
BOOL OnKeyPress(SDL_Event *event)
{
	if(SDLDrv == NULL) return FALSE;
	SDLDrv->OnPress((void *)event);
	return TRUE;
}

BOOL OnKeyRelease(SDL_Event *event)
{
	if(SDLDrv == NULL) return FALSE;
	SDLDrv->OnRelease((void *)event);
	return TRUE;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef USE_AGAR
BOOL OnKeyPressAG(int sym, int mod, Uint32 unicode)
{
	if(AGARDrv == NULL) return FALSE;
	AGARDrv->OnPress(sym, mod, unicode);
	return TRUE;
}

BOOL OnKeyReleaseAG(int sym, int mod, Uint32 unicode)
{
	if(AGARDrv == NULL) return FALSE;
	AGARDrv->OnRelease(sym, mod, unicode);
	return TRUE;
}
#endif

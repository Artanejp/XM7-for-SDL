/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *	Copyright (C) 2001-2003 Ryu Takegami
 *	Copyright (C) 2004      GIMONS
 *
 *	[ XWIN キーボード・ジョイスティック・マウス ]
 */

#ifdef _XWIN

#ifndef _xw_kbd_h_
#define _xw_kbd_h_

/*
 *	定数、型定義
 */
#define KNT_KANJI		0
#define KNT_KANA		1
#define KNT_CAPS		2
#define KEYBUFFER_SIZE	64

#define MOSCAP_NONE		0
#define MOSCAP_DINPUT	1
#define MOSCAP_WMESSAGE	2

#define VKNT_CAPITAL	0xf0
#define VKNT_KANA		0xf2
#define VKNT_KANJI1		0xf3
#define VKNT_KANJI2		0xf4

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	主要エントリ
 */
BOOL OnKeyPress(SDL_Event *event);
gboolean OnKeyPressGtk(GtkWidget *widget, GdkEventKey *event, gpointer data);
                                        /* キープレスアクション */
BOOL OnKeyRelease(SDL_Event *event);
gboolean OnKeyReleaseGtk(GtkWidget *widget, GdkEventKey *event, gpointer data);
/* キーリリースアクション */

/* JS動かした */
extern BOOL OnMoveJoy(SDL_Event *event);
extern BOOL OnPressJoy(SDL_Event *event);
extern BOOL OnReleaseJoy(SDL_Event *event);
#ifdef MOUSE
gboolean OnButtonPress(GtkWidget *widget,
                                        GdkEventButton *event,
                                        gpointer user_data);
                                        /* キープレスアクション */
gboolean OnButtonRelease(GtkWidget *widget,
                                        GdkEventButton *event,
                                        gpointer user_data);
                                        /* キーリリースアクション */
#endif

void FASTCALL InitKbd(void);
										/* 初期化 */
void FASTCALL CleanKbd(void);
										/* クリーンアップ */
BOOL FASTCALL SelectKbd(void);
										/* セレクト */
void FASTCALL PollKbd(void);
										/* キーボードポーリング */
void FASTCALL PollJoy(void);
										/* ジョイスティックポーリング */
void FASTCALL GetDefMapKbd(BYTE *pMap, int mode);
										/* デフォルトマップ取得 */
void FASTCALL SetMapKbd(BYTE *pMap);
										/* マップ設定 */
BOOL FASTCALL GetKbd(BYTE *pBuf);
										/* ポーリング＆キー情報取得 */
#ifdef MOUSE
void FASTCALL PollMos(void);
										/* マウスポーリング */
void FASTCALL SetMouseCapture(BOOL en);
										/* マウスキャプチャ設定 */
#endif

/*
 *	主要ワーク
 */
struct local_sdlkeymap {
   Uint16 keysym;
   BYTE code;
} ;
   
extern BYTE kbd_map[256];
										/* キーボード マップ */
extern struct local_sdlkeymap kbd_table[256];
										/* 対応するFM-7物理コード */
extern int nJoyType[2];
										/* ジョイスティックタイプ */
extern int nJoyRapid[2][2];
										/* 連射タイプ */
extern int nJoyCode[2][7];
										/* 生成コード */
extern BOOL bKbdReal;
										/* 擬似リアルタイムキースキャン */
extern BOOL bTenCursor;
										/* テンキー変換 */
extern BOOL bArrow8Dir;
										/* テンキー変換 8方向モード */
extern BOOL bNTkeyPushFlag[3];
										/* キー押下フラグ(NT) */
extern BOOL bNTkeyMakeFlag[128];
										/* キーMake中フラグ(NT) */
extern BOOL bNTkbMode;
										/* NT対策中フラグ */
#ifdef MOUSE
extern BYTE nMidBtnMode;
										/* 中央ボタン状態取得モード */
#endif
#ifdef __cplusplus
}
#endif

#endif	/* _xw_kbd_h_ */
#endif	/* _XWIN */

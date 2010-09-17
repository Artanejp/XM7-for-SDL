/*
 * api_kbd.h
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */

#ifndef API_KBD_H_
#define API_KBD_H_


    /*
     *  定数、型定義
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
extern          "C" {

#endif				/*  */
        BOOL OnKeyPress(SDL_Event * event);
        BOOL OnKeyRelease(SDL_Event * event);
#ifdef USE_GTK
        gboolean OnKeyPressGtk(GtkWidget * widget, GdkEventKey * event,
                               gpointer data);
        gboolean OnKeyReleaseGtk(GtkWidget * widget, GdkEventKey * event,
                                 gpointer data);
#endif
        extern BOOL kbd_snooped;
#ifdef MOUSE
	gboolean OnButtonPress(GtkWidget * widget, GdkEventButton * event,
			       gpointer user_data);

	gboolean OnButtonRelease(GtkWidget * widget,
				 GdkEventButton * event,
				 gpointer user_data);

#endif				/*  */
     void FASTCALL InitKbd(void);
    void FASTCALL   CleanKbd(void);
    BOOL FASTCALL SelectKbd(void);
    void FASTCALL   PollKbd(void);
    void        GetDefMapKbd(BYTE * pMap, int mode);
    void        SetMapKbd(BYTE *pMap);
    BOOL FASTCALL GetKbd(BYTE * pBuf);
    void PushKeyData(Uint8 code,Uint8 MakeBreak);


#ifdef MOUSE
        void FASTCALL   PollMos(void);
        void FASTCALL   SetMouseCapture(BOOL en);
#endif				/*  */

/*
 *  主要ワーク
 */
        struct local_sdlkeymap {
                Uint16 keysym;
                BYTE code;
        };
    extern BYTE kbd_map[256];

	/*
	 * キーボード マップ
	 */
//    extern struct local_sdlkeymap kbd_table[256];
	/*
	 * 対応するFM-7物理コード
	 */
    extern BOOL     bKbdReal;
    extern BOOL     bTenCursor;
    extern BOOL     bArrow8Dir;
    extern BOOL     bNTkeyPushFlag[3];
    extern BOOL     bNTkeyMakeFlag[128];
    extern BOOL     bNTkbMode;

#ifdef MOUSE
    extern BYTE     nMidBtnMode;

	/*
	 * 中央ボタン状態取得モード
	 */
#endif				/*  */
#ifdef __cplusplus
}
#endif				/*  */


#endif /* API_KBD_H_ */

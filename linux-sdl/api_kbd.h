/*
 * api_kbd.h
 *
 *  Created on: 2010/09/17
 *      Author: whatisthis
 */

#ifndef API_KBD_H_
#define API_KBD_H_
#ifdef USE_GTK
#include<X11/Xlib.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<gdk/gdkkeysyms.h>
#endif
#include<memory.h>

#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "device.h"
#include "mouse.h"
#include "event.h"
#ifdef USE_AGAR
#include <SDL.h>
#include <SDL_syswm.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include "agar_xm7.h"
#else
#include <SDL.h>
#include <SDL_syswm.h>
#include "sdl.h"
#endif
#include "sdl_sch.h"
//#include "sdl_kbd.h"
#include "gtk_propkeyboard.h"




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
    extern BOOL OnKeyPress(SDL_Event * event);
    extern BOOL OnKeyRelease(SDL_Event * event);

#ifdef USE_GTK
        gboolean OnKeyPressGtk(GtkWidget * widget, GdkEventKey * event,
                               gpointer data);
        gboolean OnKeyReleaseGtk(GtkWidget * widget, GdkEventKey * event,
                                 gpointer data);
#endif

     extern BOOL kbd_snooped;
    void FASTCALL InitKbd(void);
    void FASTCALL   CleanKbd(void);
    BOOL FASTCALL SelectKbd(void);
    void FASTCALL   PollKbd(void);
    void        GetDefMapKbd(BYTE * pMap, int mode);
    void        SetMapKbd(BYTE *pMap);
    BOOL FASTCALL GetKbd(BYTE * pBuf);
    void PushKeyData(Uint8 code,Uint8 MakeBreak);
#ifdef USE_AGAR
    BOOL OnKeyPressAG(int sym, int mod, Uint32 unicode);
    BOOL OnKeyReleaseAG(int sym, int mod, Uint32 unicode);
    void SetKeyCodeAG(Uint8 code, int sym, int mod);
    void GetKeyCodeAG(Uint8 code, void *p);
#endif



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

#ifdef __cplusplus
}
#endif				/*  */

#endif /* API_KBD_H_ */

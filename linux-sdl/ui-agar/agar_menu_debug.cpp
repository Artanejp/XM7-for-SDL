/*
 * agar_menu_debug.cpp
 *
 *  Created on: 2011/10/25
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include <SDL.h>
#include <libintl.h>
extern "C" {
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
}
#include "xm7.h"
#include "tapelp.h"


#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#else
#include "sdl.h"
#include "sdl_cfg.h"
#endif

#include "sdl_bar.h"
#include "api_kbd.h"
#include "sdl_sch.h"
#include "sdl_snd.h"
#include "sdl_inifile.h"
#include "api_draw.h"
//#include "sdl_gtkdlg.h"
#include "agar_toolbox.h"

extern void OnPushCancel(AG_Event *event);
extern void KeyBoardSnoop(BOOL t);

enum {
    MEM_MAIN = 0,
    MEM_SUB,
    MEM_JSUB
};

static DWORD uMainAddr;
static DWORD uSubAddr;
static DWORD uJsubAddr;

struct MemDumpWid {
    AG_Window *win;
    AG_Widget *parent;
    DWORD addr;
    AG_Label *l[16][16];
    BOOL8 Editable;
    BOOL8 Updated;
    volatile BYTE FASTCALL (*rf)(WORD);
    void FASTCALL (*wf)(WORD, BYTE);
    BOOL (Seek)(WORD);
    BOOL (ReadSec)(WORD);
    BOOL (*WriteSec)(WORD, BYTE);
} ;



static int c2h(char c)
{
    if((c >= '0') && (c <= '9'))
    {
        return (int)(c - '0');
    }
    if((c >= 'A') && (c <= 'F'))
    {
        return (int)((c - 'A') + 10);
    }
    if((c >= 'a') && (c <= 'f'))
    {
        return (int)((c - 'a') + 10);
    }
    return -1;
}

static BOOL sanity_hexa(char *s, int *v)
{
    DWORD u = 0;
    int l;
    int i;
    int t;
    int tt;

    l = strlen(s);
    if((l <= 0) || (l > 8)) {
        *v = 0x00000000;
        return FALSE;
    }

    for(i = l - 1; i >= 0; i--) {
        tt = c2h(s[i]);
        if(tt < 0) {
            *v = 0x00000000;
            return FALSE;
        }
        t |= tt;
        t <<= 4;
    }
    *v = (DWORD)t;
    return TRUE;
}



static void UpdateDumpMemRead(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    int addr = AG_INT(1);
    void *p = AG_PTR(2);
    volatile BYTE FASTCALL (*rf)(WORD);

    rf = (volatile BYTE FASTCALL (*)(WORD))p;
    AG_TextboxPrintf(t, "%02x", rf(addr));
}

static void CreateDumpMem(AG_Widget *s, WORD addr, volatile BYTE FASTCALL (*rf)(WORD), void FASTCALL (*wf)(WORD, BYTE), int w, int h)
{
    char stmp[8];
    int x;
    int y;
    AG_VBox *vb;
    AG_HBox *hb;
    AG_Textbox *l;

    for(y = 0; y < h; y++){
        hb = AG_HBoxNew(s, 0);
        for(x = 0; x < w ; x++) {
//            vb = AG_VBoxNew(hb, 0);
            l = AG_TextboxNew(hb, 0, "");
            AG_TextboxSizeHint(l, "XX");
            AG_TextboxPrintf(l, "%02x", rf(addr + x + w * y));
            AG_SetEvent(l, "window-update", UpdateDumpMemRead, "%i,%p", addr + x + w * y, (void *)rf);
        }
    }
}

static void OnChangeAddr(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    void *p = AG_PTR(1);
    AG_Widget *wi;
    char *pp;
    char text[16];
    int i;

    if(t == NULL) return;
    if(p == NULL) return;
    pp = (char *)p;
    AG_TextboxCopyString(t, text, 15);
    if(strlen(text) < 4) return;

    if(sanity_hexa(text, &i)) {
        *pp = i & 0xffff;
    }

}

static void DestroyDumpWindow(AG_Event *event)
{
    void *p = AG_PTR(1);
    if(p == NULL) return;
    free(p);
    p = NULL;
}

static void CreateDump(AG_Event *event)
{
    AG_Window *w;

	AG_Menu *self = (AG_Menu *)AG_SELF();
	AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
    int type = AG_INT(1);
    DWORD *pAddr = (DWORD *)AG_PTR(2);
    DWORD addr;

    volatile BYTE FASTCALL (*readFunc)(WORD);
    void FASTCALL (*writeFunc)(WORD, BYTE);
    AG_Textbox *addrVar;
    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;
    struct MemDumpWid *mp;
    char sbuf[16];

    if(pAddr == NULL) return;
//    *pAddr = 0x8000;
    addr = *pAddr;

    mp = (struct MemDumpWid *)malloc(sizeof(struct MemDumpWid));
    if(mp == NULL) return;

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 230, 80);
	vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);
    switch(type){
    case MEM_MAIN:
            addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "MAIN MEM ADDR =");
            AG_TextboxSizeHint(addrVar, "XXXXXX");
            AG_TextboxPrintf(addrVar, "%04x", addr);
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p,%p", pAddr, mp);
            readFunc = mainmem_readb;
            writeFunc = mainmem_writeb;
            break;
    case MEM_SUB:
            addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "SUB  MEM ADDR =");
            AG_TextboxSizeHint(addrVar, "XXXXXX");
            AG_TextboxPrintf(addrVar, "%04x", addr);
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p,%p", pAddr, mp);
            readFunc = submem_readb;
            writeFunc = submem_writeb;
            break;
    default:
            readFunc = NULL;
            writeFunc = NULL;
            break;
    }
    if((readFunc != NULL) && (writeFunc != NULL)) {
            AG_TextboxPrintf(addrVar, "%04x", addr);
            AG_WidgetShow(AGWIDGET(addrVar));
            CreateDumpMem(AGWIDGET(vb), addr, readFunc, writeFunc, 16, 16);
    }
   	box = AG_BoxNewHoriz(vb, 0);
   	box = AG_BoxNewHoriz(vb, 0);
	btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
   	box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyDumpWindow, "%p", mp);
    AG_WindowShow(w);

}


void Create_DebugMenu(AG_MenuItem *parent)
{
   	AG_MenuItem *item ;

	item = AG_MenuBool(parent, gettext("Pause"), NULL, &run_flag, 1);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent, gettext("Dump Main-Memory"), NULL, CreateDump, "%i,%p", MEM_MAIN,(void *)&uMainAddr);
}

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

    l = strlen(s);
    if((l <= 0) || (l > 8)) {
        *v = 0x00000000;
        return FALSE;
    }
    t = 0;
    sscanf(s, "%x", &t);
    *v = t;
    return TRUE;
}

extern void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD Segment, WORD addr, int bytes, BOOL SegFlag, BOOL AddrFlag);


static void UpdateDumpMemRead(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    int addr = AG_INT(1);
    DWORD *p = (DWORD *)AG_PTR(2);
    volatile BYTE FASTCALL (*rf)(WORD);

    rf = (volatile BYTE FASTCALL (*)(WORD))p;
    AG_TextboxPrintf(t, "%02x", rf(addr));
}

static void ReadMemLine(Uint8 *p, WORD addr, volatile BYTE FASTCALL (*rf)(WORD), int w)
{
    WORD i;
    if(w <= 0) return;

    for(i = 0; i < w ; i++){
        p[i] = rf(addr + i);
    }
}

static void DumpMem(AG_Textbox *t, WORD addr, volatile BYTE FASTCALL (*rf)(WORD), int w, int h)
{
    char *stmp;
    Uint8 *buf;
    int i;

    buf = (Uint8 *)malloc(sizeof(Uint8) * (w + 1));
    if(buf == NULL) return;

    stmp = (char *)malloc(sizeof(char) * ((w + 1) * 3 + 4 + 64) * h);
    if(stmp == NULL){
        free(buf);
        return;
    }
    strcpy(stmp, "");

    for(i = 0; i < h ; i++){
        ReadMemLine(buf, addr + (w * i) , rf, w);
        // DBG_HexDumpMemory(char *str, Uint8 *buf, WORD Segment, WORD addr, int bytes, BOOL SegFlag, BOOL AddrFlag)
        DBG_HexDumpMemory(stmp, buf, 0x0000, addr + (i * w), w, FALSE, TRUE);
        strcat(stmp, "\n");
    }
    AG_TextboxPrintf(t, "%s\n", stmp);
    free(stmp);
    free(buf);
}

static void CreateDumpMem(AG_Textbox *t, WORD addr, volatile BYTE FASTCALL (*rf)(WORD), void FASTCALL (*wf)(WORD, BYTE), int w, int h)
{
    char *stmp;
    int i;


    if(w <= 0) return;
    if(h <= 0) return;

    if(t == NULL) return;

    stmp = (char *)malloc(sizeof(char) * ((w + 1) * 3 + 4 + 64));
    if(stmp == NULL){
        return;
    }
    strcpy(stmp, "XXXXXX");
    for(i = 0 ; i < w; i++) {
        strcat(stmp, "XXX");
    }
    strcat(stmp, "XXXXXXXX");
    AG_TextboxSizeHint(t, stmp);
    AG_TextboxSizeHintLines(t, h);
    DumpMem(t, addr, rf, w, h);
    free(stmp);
}

static void OnChangeAddr(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    DWORD *p = (DWORD *)AG_PTR(1);
    AG_Textbox *td = (AG_Textbox *)AG_PTR(2);
    struct MemDumpWid *mp = (struct MemDumpWid *)AG_PTR(3);
    int w = AG_INT(4);
    int h = AG_INT(5);

    AG_Widget *wi;
    char text[16];
    int i;

    if(t == NULL) return;
    if(td == NULL) return;
    if(p == NULL) return;
    AG_TextboxCopyString(t, text, 15);
    if(strlen(text) < 4) return;

    if(sanity_hexa(text, &i)) {
        i = i & 0xffff;
    }
    *p = i;
    DumpMem(td, i, mp->rf, w, h);
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
    AG_Textbox *dumpVar;

    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;
    struct MemDumpWid *mp;
    char sbuf[16];

    if(pAddr == NULL) return;
    addr = *pAddr;

    mp = (struct MemDumpWid *)malloc(sizeof(struct MemDumpWid));
    if(mp == NULL) return;

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 230, 80);
	vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

    mp->addr = addr;
    mp->Editable = FALSE;
    mp->Updated = TRUE;
    mp->win = w;
//    mp->Seek = NULL;
//    mp->ReadSec = NULL;
//    mp->WriteSec = NULL;

    switch(type){
    case MEM_MAIN:
            addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "MAIN MEM ADDR =");
            AG_TextboxSizeHint(addrVar, "XXXXXX");
            AG_TextboxPrintf(addrVar, "%04x", addr);
            hb = AG_HBoxNew(vb, 0);
            dumpVar = AG_TextboxNew(hb, AG_TEXTBOX_MULTILINE | AG_TEXTBOX_EXPAND, "");
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p,%p,%p,%i,%i", pAddr, (void *)dumpVar, mp, 16, 16);
            readFunc = mainmem_readb;
            writeFunc = mainmem_writeb;
            break;
    case MEM_SUB:
            addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "SUB  MEM ADDR =");
            AG_TextboxSizeHint(addrVar, "XXXXXX");
            AG_TextboxPrintf(addrVar, "%04x", addr);
            hb = AG_HBoxNew(vb, 0);
            dumpVar = AG_TextboxNew(hb, AG_TEXTBOX_MULTILINE | AG_TEXTBOX_EXPAND, "");
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p,%p,%p,%i,%i", pAddr, (void *)dumpVar, mp, 16, 16);
            readFunc = submem_readb;
            writeFunc = submem_writeb;
            break;
    default:
            readFunc = NULL;
            writeFunc = NULL;
            break;
    }
    mp->rf = readFunc;
    mp->wf = writeFunc;
    if((readFunc != NULL) && (writeFunc != NULL)) {
            AG_TextboxPrintf(addrVar, "%04x", addr);
            AG_WidgetShow(AGWIDGET(addrVar));
            CreateDumpMem(dumpVar, addr, readFunc, writeFunc, 16, 16);
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
	item = AG_MenuAction(parent, gettext("Dump Sub-Memory"), NULL, CreateDump, "%i,%p", MEM_SUB,(void *)&uSubAddr);
}

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

#include "sdl_sch.h"
#include "agar_toolbox.h"
#include "agar_debugger.h"
#include <time.h>

#include "../xm7-debugger/memread.h"

extern void OnPushCancel(AG_Event *event);
extern void KeyBoardSnoop(BOOL t);

enum {
    MEM_MAIN = 0,
    MEM_SUB,
    MEM_JSUB
};

struct XM7_MemDumpDesc {
    XM7_DbgDump *dump;
    unsigned int addr;
    BYTE (*rf)(WORD);
    void (*wf)(WORD, BYTE);
    AG_Timeout to;
    Uint32 to_tick;
};

static void readmem(struct XM7_MemDumpDesc *p)
{
    BYTE *buf;
    int x,y;
    int ofset;

    if(p == NULL) return;
    if(p->rf == NULL) return;
    if(p->dump->buf == NULL) return;
    ofset = 0;
    buf = p->dump->buf;
    for(x = 0; x < 16 ; x++){
        for(y = 0; y < 16; y++){
            buf[ofset] = p->rf(p->addr + ofset);
            ofset++;
        }
    }
}


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

static inline char hex2chr(Uint8 b)
{
    Uint8 bb = b & 0x0f;
    char c;

    if(bb < 10) {
        c = '0' + bb;
    } else {
        c = 'A' + (bb - 10);
    }
    return c;
}


extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 w);
extern void DBG_Bin2Hex4(char *str, Uint32 dw);
extern void DBG_DumpAsc(char *str, Uint8 b);



static Uint32 UpdateDisasmMemRead(void *obj, Uint32 ival, void *arg )
{
}

static void OnChangeAddrDisasm(AG_Event *event)
{
}


/*
* Auto Update (Timer)
*/
static Uint32 UpdateDumpMemRead(void *obj, Uint32 ival, void *arg )
{

    struct XM7_MemDumpDesc *mp = (struct XM7_MemDumpDesc *)arg;
    char *str;


    if(mp == NULL) return ival;
//    readmem(mp);
    XM7_DbgDumpMem(mp->dump, mp->addr);
//    {
//        str = "aaaaaaaa";
//        mp->dump->dump->PutString(str);
//    }

    return mp->to_tick;
}

static void OnChangePollDump(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    struct XM7_MemDumpDesc *mp = (struct XM7_MemDumpDesc *)AG_PTR(1);
    char text[16];
    int i;


    if(mp == NULL) return;
    if(t == NULL) return;
    AG_TextboxCopyString(t, text, 15);
    sscanf(text, "%d", &i);
    if(i < 50) return;
    if(i > 4000) i = 4000;
    mp->to_tick = i;
//    AG_LockTimeouts(AGOBJECT(mp));
//    AG_ScheduleTimeout(AGOBJECT(t->wid.window), &(mp->to), i);
//    AG_UnlockTimeouts(AGOBJECT(t->wid.window));
    return;
}

static void OnChangeAddr(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    struct XM7_MemDumpDesc *mp = (struct XM7_MemDumpDesc *)AG_PTR(1);

    AG_Widget *wi;
    char text[16];
    int i;

    if(mp == NULL) return;
    if(t == NULL) return;


    AG_TextboxCopyString(t, text, 15);
    if(strlen(text) < 4) return;

    if(sanity_hexa(text, &i)) {
        i = i & 0xffff;
    }
    mp->addr = i;
    readmem(mp);
    XM7_DbgDumpMem(mp->dump, mp->addr);
}


static void DestroyDumpWindow(AG_Event *event)
{
    struct XM7_MemDumpDesc *mp = (struct XM7_MemDumpDesc *)AG_PTR(1);
    if(mp == NULL) return;
    if(mp->dump != NULL) XM7_DbgDumpMemDetach(mp->dump);

    free(mp);
    mp = NULL;
}


static void CreateDump(AG_Event *event)
{
    AG_Window *w;

   AG_Menu *self = (AG_Menu *)AG_SELF();
   AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
   int type = AG_INT(1);
    int disasm = AG_INT(2);
    AG_Textbox *pollVar;
    AG_Textbox *addrVar;
    struct XM7_MemDumpDesc *mp;


    BYTE (*readFunc)(WORD);
    void FASTCALL (*writeFunc)(WORD, BYTE);
    XM7_DbgDump *dump;

    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;

    mp = (XM7_MemDumpDesc *)malloc(sizeof(struct XM7_MemDumpDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_MemDumpDesc));

//    if(pAddr == NULL) return;
    mp->addr = 0x0000;
    mp->to_tick = 200;
    w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

    switch(type){
    case MEM_MAIN:
            readFunc = rb_main;
            writeFunc = mainmem_writeb;
            AG_WindowSetCaption(w, "Dump Main memory");
            break;
    case MEM_SUB:
            readFunc = rb_sub;
            writeFunc = submem_writeb;
            AG_WindowSetCaption(w, "Dump Sub memory");
            break;
    default:
            readFunc = NULL;
            writeFunc = NULL;
            break;
    }
    addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "Addr");
    AG_TextboxSizeHint(addrVar, "XXXXXX");
    AG_TextboxPrintf(addrVar, "%04x", mp->addr);

   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, "Poll");
    AG_TextboxSizeHint(pollVar, "XXXXXX");
    AG_TextboxPrintf(pollVar, "%4d", mp->to_tick);


    hb = AG_HBoxNew(vb, 0);
    if((readFunc != NULL) && (writeFunc != NULL)) {
        dump = XM7_DbgDumpMemInit(hb, readFunc, writeFunc);
        if(dump == NULL) return;
        mp->dump = dump;
        mp->rf = readFunc;
        mp->wf = writeFunc;
    }


    box = AG_BoxNewHoriz(vb, 0);
    box = AG_BoxNewHoriz(vb, 0);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
    box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyDumpWindow, "%p", mp);
    AG_SetEvent(pollVar, "textbox-postchg", OnChangePollDump, "%p", mp);
    AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p", mp);

    AG_SetTimeout(&(mp->to), UpdateDumpMemRead, (void *)mp, AG_CANCEL_ONDETACH | AG_CANCEL_ONLOAD);
    AG_ScheduleTimeout(AGOBJECT(w) , &(mp->to), mp->to_tick);
    AG_WindowShow(w);
}


void Create_DebugMenu(AG_MenuItem *parent)
{
   	AG_MenuItem *item ;

	item = AG_MenuBool(parent, gettext("Pause"), NULL, &run_flag, 1);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent, gettext("Dump Main-Memory"), NULL, CreateDump, "%i,%i", MEM_MAIN, 0);
	item = AG_MenuAction(parent, gettext("Dump Sub-Memory"), NULL, CreateDump, "%i,%i", MEM_SUB, 0);
	AG_MenuSeparator(parent);
//	item = AG_MenuAction(parent, gettext("Disasm Main-Memory"), NULL, CreateDump, "%i,%i", MEM_MAIN, 1);
//	item = AG_MenuAction(parent, gettext("Disasm Sub-Memory"), NULL, CreateDump, "%i,%i", MEM_SUB, 1);
}

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

//#include "sdl_bar.h"
//#include "api_kbd.h"
#include "sdl_sch.h"
//#include "sdl_snd.h"
//#include "sdl_inifile.h"
//#include "api_draw.h"
//#include "sdl_gtkdlg.h"
#include "agar_toolbox.h"
#include "../xm7-debugger/memread.h"

extern void OnPushCancel(AG_Event *event);
extern void KeyBoardSnoop(BOOL t);

enum {
    MEM_MAIN = 0,
    MEM_SUB,
    MEM_JSUB
};


struct MemDumpWid {
    AG_Window *win;
    AG_Widget *parent;
    AG_Textbox *dumpBox;
    AG_Font *dumpFont;
    char *text;
    AG_Timeout to;
    int to_tick;

    DWORD addr;
    BOOL8 Editable;
    BOOL8 Updated;

    int w;
    int h;
    int lines;
    int cpu;

    BYTE (*rf)(WORD);
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

extern  void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD addr, int sum, int bytes, BOOL AddrFlag);
extern void DBG_PrintYSum(char *str, int *sum, int totalSum, int width);

extern int DBG_DisAsm1op(int cpuno, Uint16 pc, char *s, Uint8 *membuf);

static void DumpMem(AG_Textbox *t, WORD addr, BYTE (*rf)(WORD), int w, int h);



static void Disassemble(AG_Textbox *t, WORD addr, struct MemDumpWid *mp, int lines)
{
    char stmp[128];
    char *stmp2;
    Uint8 membuf[16];
    int i;
    int pc;
    int bytes;

    if(mp == NULL) return;
    if(t == NULL) return;
    stmp2 = (char *)malloc(64 * lines * sizeof(char));
    if(stmp2 == NULL) return;
    pc = addr;
    *stmp2 = '\0';
    for(i = 0;  i < lines; i++) {
        bytes = DBG_DisAsm1op(mp->cpu, pc, stmp, membuf);
        strcat(stmp2, stmp);
        strcat(stmp2, "\n");
        pc += bytes;
        if(pc > 0xffff) pc -= 0x10000;
    }
    AG_TextboxPrintf(t, "%s", stmp2);
    free(stmp2);
}

static Uint32 UpdateDisasmMemRead(void *obj, Uint32 ival, void *arg )
{
    struct MemDumpWid *mp = (struct MemDumpWid *)arg;
    AG_Textbox *td;

    if(mp == NULL) return ival;
//    if(mp->Updated == FALSE) return;
    td = mp->dumpBox;
    if(mp->Updated) Disassemble(td, mp->addr, mp, mp->lines);
    return mp->to_tick;
}

static void OnChangeAddrDisasm(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    AG_Textbox *td;
    struct MemDumpWid *mp = (struct MemDumpWid *)AG_PTR(1);
    int l;
    char text[16];
    int i;

    if(mp == NULL) return;
    if(t == NULL) return;
    td = mp->dumpBox;
    l = mp->lines;

    if(td == NULL) return;
    AG_TextboxCopyString(t, text, 15);
    if(strlen(text) < 4) return;

    if(sanity_hexa(text, &i)) {
        i = i & 0xffff;
    }
    mp->addr = i;
    Disassemble(td, i, mp, l);
}

/*
* Auto Update (Timer)
*/
static Uint32 UpdateDumpMemRead(void *obj, Uint32 ival, void *arg )
{
    struct MemDumpWid *mp = (struct MemDumpWid *)arg;
    AG_Textbox *td;

    if(mp == NULL) return ival;
//    if(mp->Updated == FALSE) return;
    td = mp->dumpBox;
    if(mp->Updated) DumpMem(td, mp->addr, mp->rf, mp->w, mp->h);
    return mp->to_tick;
}

static void ReadMemLine(Uint8 *p, WORD addr, BYTE (*rf)(WORD), int w)
{
    WORD i;
    if(w <= 0) return;

    for(i = 0; i < w ; i++){
        p[i] = rf(addr + i);
    }
}

static int sum_line(Uint8 *p, int *ysum, int w)
{
    int sum = 0;
    int i;
    for(i = 0; i < w; i++){
        sum += p[i];
        ysum[i] += p[i];
    }
    return sum;
}

static void DumpMem(AG_Textbox *t, WORD addr, BYTE (*rf)(WORD), int w, int h)
{
    char *stmp;
    Uint8 *buf;
    int i;
    int xsum;
    int *ysum;

    buf = (Uint8 *)malloc(sizeof(Uint8) * (w + 1));
    if(buf == NULL) return;

    ysum = (int *)malloc(sizeof(int) * (w + 1));
    if(ysum == NULL) {
        free(buf);
        return;
    }
    stmp = (char *)malloc(sizeof(char) * ((w + 1) * 5 + 16 + 5) * h);
    if(stmp == NULL){
        free(buf);
        free(ysum);
        return;
    }

    strcpy(stmp, "");

    for(i = 0; i < h ; i++){
//        strcpy(stmp, "");
        ReadMemLine(buf, addr + (w * i) , rf, w);
        xsum = sum_line(buf, ysum, w);
        DBG_HexDumpMemory(stmp, buf, addr + (i * w), xsum, w, TRUE);
//        AG_TextboxPrintf(t, "%s", stmp);
        strcat(stmp, "\n");
    }
    AG_TextboxPrintf(t, "%s\n", stmp);
    free(stmp);
    free(ysum);
    free(buf);
}

static void CreateDumpMem(AG_Textbox *t, WORD addr, BYTE (*rf)(WORD), void FASTCALL (*wf)(WORD, BYTE), int w, int h)
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
    for(i = 0 ; i < w; i++) {
        strcat(stmp, "X");
    }
    AG_TextboxSizeHint(t, stmp);
    AG_TextboxSizeHintLines(t, h);

    DumpMem(t, addr, rf, w, h);
    free(stmp);
}

static void OnChangePollDump(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    struct MemDumpWid *mp = (struct MemDumpWid *)AG_PTR(1);
    char text[16];
    int i;


    if(mp == NULL) return;
    if(t == NULL) return;
    AG_TextboxCopyString(t, text, 15);
    sscanf(text, "%d", &i);
    if(i < 50) return;
    if(i > 4000) i = 4000;
    mp->to_tick = i;
//    AG_LockTimeouts(AGOBJECT(t->wid.window));
//    AG_ScheduleTimeout(AGOBJECT(t->wid.window), &(mp->to), i);
//    AG_UnlockTimeouts(AGOBJECT(t->wid.window));
    return;
}

static void OnChangeAddr(AG_Event *event)
{
    AG_Textbox *t = (AG_Textbox *)AG_SELF();
    AG_Textbox *td;
    struct MemDumpWid *mp = (struct MemDumpWid *)AG_PTR(1);
    int w;
    int h;

    AG_Widget *wi;
    char text[16];
    int i;

    if(mp == NULL) return;
    if(t == NULL) return;
    td = mp->dumpBox;
    w = mp->w;
    h = mp->h;

    if(td == NULL) return;
    AG_TextboxCopyString(t, text, 15);
    if(strlen(text) < 4) return;

    if(sanity_hexa(text, &i)) {
        i = i & 0xffff;
    }
    mp->addr = i;
    DumpMem(td, i, mp->rf, w, h);
}


static void DestroyDumpWindow(AG_Event *event)
{
    struct MemDumpWid *mp = (struct MemDumpWid *)AG_PTR(1);
    if(mp == NULL) return;

    if(mp->dumpFont != NULL) AG_DestroyFont(mp->dumpFont);
    if(mp->text != NULL) free(mp->text);
    free(mp);
    mp = NULL;
}

static AG_Timeout tto;

static void CreateDump(AG_Event *event)
{
    AG_Window *w;

	AG_Menu *self = (AG_Menu *)AG_SELF();
	AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
    int type = AG_INT(1);
    int disasm = AG_INT(2);
    DWORD addr;

    BYTE (*readFunc)(WORD);
    void FASTCALL (*writeFunc)(WORD, BYTE);
    AG_Textbox *addrVar;
    AG_Textbox *dumpVar;
    AG_Textbox *pollVar;

    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;
    struct MemDumpWid *mp;
    char sbuf[16];
    int cpu;

//    if(pAddr == NULL) return;
    addr = 0x0000;

    mp = (struct MemDumpWid *)malloc(sizeof(struct MemDumpWid));
    if(mp == NULL) return;

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 230, 80);
	vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);
    mp->text = (char *)malloc(17 * 17 * 8 + 16 * 8);
    if(mp->text == NULL) {
        free(mp);
        return;
    }
    mp->addr = addr;
    mp->Editable = FALSE;
    mp->Updated = TRUE;
    mp->to_tick = 100; // 100ms
    mp->win = w;
    mp->dumpFont = AG_FetchFont("F-Font_400line.ttf", 16, 0);
//    mp->Seek = NULL;
//    mp->ReadSec = NULL;
//    mp->WriteSec = NULL;

    switch(type){
    case MEM_MAIN:
            addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "MAIN MEM ADDR =");
            AG_TextboxSizeHint(addrVar, "XXXXXXXXXX");
            AG_TextboxPrintf(addrVar, "%04x", addr);
            readFunc = rb_main;
            writeFunc = mainmem_writeb;
            mp->cpu = MAINCPU;
            break;
    case MEM_SUB:
            addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "SUB  MEM ADDR =");
            AG_TextboxSizeHint(addrVar, "XXXXXXXXXX");
            AG_TextboxPrintf(addrVar, "%04x", addr);
            readFunc = rb_sub;
            writeFunc = submem_writeb;
            mp->cpu = SUBCPU;
            break;
    default:
            readFunc = NULL;
            writeFunc = NULL;
            break;
    }
    pollVar = AG_TextboxNew(AGWIDGET(hb), 0, "Poll");
    AG_TextboxSizeHint(pollVar, "XXXX");
    AG_TextboxPrintf(pollVar, "%04", mp->to_tick);

    if(disasm == 0) {
        AG_SetEvent(pollVar, "textbox-postchg", OnChangePollDump, "%p", mp);
    } else {
        AG_SetEvent(pollVar, "textbox-postchg", OnChangePollDump, "%p", mp);
    }
    mp->rf = readFunc;
    mp->wf = writeFunc;
    dumpVar = AG_TextboxNew(vb, AG_TEXTBOX_MULTILINE | AG_TEXTBOX_EXPAND, "");
    mp->dumpBox = dumpVar;
    mp->w = 16;
    mp->h = 16;
    mp->lines = 20;
    AG_TextboxBindUTF8(mp->dumpBox, mp->text, 17 * 17 * 8 + 16 * 8);

    if(mp->dumpFont != NULL) AG_TextboxSetFont(mp->dumpBox, mp->dumpFont);

    if((readFunc != NULL) && (writeFunc != NULL)) {
        AG_TextboxPrintf(addrVar, "%04x", addr);
        AG_WidgetShow(AGWIDGET(addrVar));
        if(disasm == 0) {
            CreateDumpMem(dumpVar, addr, readFunc, writeFunc, mp->w, mp->h);
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p", mp);
        } else {
            CreateDumpMem(dumpVar, addr, readFunc, writeFunc, mp->w, mp->lines);
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddrDisasm, "%p", mp);
        }
    }
    hb = AG_HBoxNew(vb, 0);


   	box = AG_BoxNewHoriz(vb, 0);
   	box = AG_BoxNewHoriz(vb, 0);
	btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
   	box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyDumpWindow, "%p", mp);

    if(disasm == 0){
        AG_SetTimeout(&(mp->to), UpdateDumpMemRead, (void *)mp, AG_CANCEL_ONDETACH | AG_CANCEL_ONLOAD);
    } else {
        AG_SetTimeout(&(mp->to), UpdateDisasmMemRead, (void *)mp, AG_CANCEL_ONDETACH | AG_CANCEL_ONLOAD);
    }
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
	item = AG_MenuAction(parent, gettext("Disasm Main-Memory"), NULL, CreateDump, "%i,%i", MEM_MAIN, 1);
	item = AG_MenuAction(parent, gettext("Disasm Sub-Memory"), NULL, CreateDump, "%i,%i", MEM_SUB, 1);
}

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
    Uint8 *oldbuf;

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


extern void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD addr, int sum, int bytes, BOOL AddrFlag);
extern void DBG_PrintYSum(char *str, int *sum, int totalSum, int width);
extern int DBG_DisAsm1op(int cpuno, Uint16 pc, char *s, Uint8 *membuf);
extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 w);
extern void DBG_Bin2Hex4(char *str, Uint32 dw);
extern void DBG_DumpAsc(char *str, Uint8 b);


static void DumpMem(AG_Textbox *t, WORD addr, struct MemDumpWid *mp);
static void UpdateDumpMem(AG_Textbox *t, WORD addr, struct MemDumpWid *mp);

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
    td = mp->dumpBox;
    DumpMem(td, mp->addr, mp);
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

static void decodecursor_dump(int pos, int *x, int *y, int w, int h)
{
    pos -= 3 * w + 6 + 1;
    pos -= (w * 3 + 8 + 1);
    if(pos <= 0) {
        *x = 0;
        *y = 0;
        return;
    }
    *y = pos / (w * 4 + 12);
    pos -= ((w * 4 + 12) * *y + 5);
    *x = pos / 3;
}

static int movecursor_dump(int x, int y, int w, int h)
{
    int pos;
    pos = 3 * w + 6 + 1;
    pos += (w * 3 + 8 + 1);
    pos += (y * (w * 4 + 12));
    pos += x * 3 + 5;
    return pos;
}

static int movecursor_ascii(int x, int y, int w, int h)
{
    int pos;
    pos = 3 * w + 6 + 1;
    pos += (w * 3 + 8 + 1);
    pos += (y * (w * 4 + 12));
    pos += w * 3 + 5 + 1;
    pos += x;
    return pos;
}


static void UpdateDumpMem(AG_Textbox *t, WORD addr, struct MemDumpWid *mp)
{
    int w;
    int h;
    int x;
    int y;
    Uint8 *buf;
    Uint8 *oldbuf;
    Uint8 b;
    int ptr = 0;
    char cbuf[16];

    if(mp == NULL) return;
    w = mp->w;
    h = mp->h;
    oldbuf = mp->oldbuf;

    if(w <= 0) return;
    if(h <= 0) return;

    buf = (Uint8 *)malloc(w * h + 4);
    cbuf[0] = '\0';
    ReadMemLine(buf, addr , mp->rf, w * h);
    for(y = 0; y < h; y++){
        for(x = 0; x < w; x++){
            b = buf[ptr];
            if((oldbuf == NULL) || (b != oldbuf[ptr])){
                AG_TextboxSetCursorPos(t, movecursor_dump(x, y, w, h));
                DBG_Bin2Hex1(cbuf, b);
                AG_TextboxSetString(t, cbuf);

                AG_TextboxSetCursorPos(t, movecursor_ascii(x, y, w, h));
                DBG_DumpAsc(cbuf, b);
                AG_TextboxSetString(t, cbuf);
            }
            ptr++;
        }
    }
    if(oldbuf != NULL) memcpy(oldbuf, buf, w * h);
}

static void DumpMem(AG_Textbox *t, WORD addr, struct MemDumpWid *mp)
{
    char *stmp;
    char *stmp2;
    Uint8 *buf;
    Uint8 *oldbuf;
    int i;
    int j;
    int xsum;
    int *ysum;
    int tsum = 0;
    BYTE (*rf)(WORD);
    int w;
    int h;

    if(mp == NULL) return;
    rf = mp->rf;
    oldbuf = mp->oldbuf;
    w = mp->w;
    h = mp->h;


    if(w <= 0) return;
    if(h <= 0) return;
    if(rf == NULL) return;

    buf = (Uint8 *)malloc(sizeof(Uint8) * (w + 1));
    if(buf == NULL) return;

    ysum = (int *)malloc(sizeof(int) * (w + 1));
    if(ysum == NULL) {
        free(buf);
        return;
    }
    memset(ysum, 0x00, sizeof(int) * (w + 1));

    stmp = (char *)malloc(sizeof(char) * ((w + 1) * 5 + 16 + 5) * h);
    if(stmp == NULL){
        free(buf);
        free(ysum);
        return;
    }
    stmp2 = (char *)malloc(sizeof(char) * ((w + 1)* 3 + 12));
    if(stmp2 == NULL){
        free(buf);
        free(ysum);
        free(stmp);
        return;
    }
    stmp[0] = '\0';
    stmp2[0] = '\0';

    strcpy(stmp2, "ADDR  ");
    j = 6;
    for(i = 0; i < w; i++) {
        stmp2[j] = '+';
        stmp2[j + 1] = (char)hex2chr((Uint8)(i & 0x0f));
        stmp2[j + 2] = ' ';
        j += 3;
    }
    stmp2[j] = '\n';
    stmp2[j + 1] = '\0';
    strcpy(stmp, stmp2);

    for(i = 0; i < (w * 3 + 8); i++) {
        stmp2[i] = '-';
    }
    stmp2[i] = '\n';
    stmp2[i + 1] = '\0';
    strcat(stmp, stmp2);

    for(i = 0; i < h ; i++){
        ReadMemLine(buf, addr + (w * i) , rf, w);
        if(oldbuf != NULL) {
            memcpy(&oldbuf[w * i], buf, w);
        }
        xsum = sum_line(buf, ysum, w);
        tsum += xsum;
        DBG_HexDumpMemory(stmp, buf, addr + (i * w), xsum, w, TRUE);
//        AG_TextboxPrintf(t, "%s", stmp);
        strcat(stmp, "\n");
    }
    strcat(stmp, stmp2);

    DBG_PrintYSum(stmp, ysum, tsum, w);
    AG_TextboxPrintf(t, "%s\n", stmp);

    AG_TextboxSetCursorPos(t, movecursor_dump(15, 15, w, h));
    mp->Updated = 0;
    free(stmp);
    free(stmp2);
    free(ysum);
    free(buf);
}

static void CreateDumpMem(AG_Textbox *t, WORD addr, struct MemDumpWid *mp)
{
    char *stmp;
    int i;
    int w;
    int h;
    BYTE (*rf)(WORD);

    if(mp == NULL) return;
    w = mp->w;
    h = mp->h;
    rf = mp->rf;
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

    DumpMem(t, addr, mp);
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
    DumpMem(td, i, mp);
}


static void DestroyDumpWindow(AG_Event *event)
{
    struct MemDumpWid *mp = (struct MemDumpWid *)AG_PTR(1);
    if(mp == NULL) return;

    if(mp->dumpFont != NULL) AG_DestroyFont(mp->dumpFont);
    if(mp->text != NULL) free(mp->text);
    if(mp->oldbuf != NULL) free(mp->oldbuf);
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
    mp->oldbuf = (Uint8 *)malloc(sizeof(Uint8) * 16 * 16);
    if(mp->oldbuf == NULL){
        free(mp);
        return;
    }
    memset(mp->oldbuf, 0x00, sizeof(Uint8) * 16 * 16);

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 230, 80);
	vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);
    mp->text = (char *)malloc(17 * 17 * 8 + 16 * 8);
    if(mp->text == NULL) {
        free(mp->oldbuf);
        free(mp);
        return;
    }
    mp->addr = addr;
    mp->Editable = FALSE;
    mp->Updated = TRUE;
    mp->to_tick = 200; // 200ms
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
            CreateDumpMem(dumpVar, addr, mp);
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p", mp);
        } else {
            mp->h = 20;
            CreateDumpMem(dumpVar, addr, mp);
            AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddrDisasm, "%p", mp);
        }
    }
    hb = AG_HBoxNew(vb, 0);


   	box = AG_BoxNewHoriz(vb, 0);
   	box = AG_BoxNewHoriz(vb, 0);
	btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
   	box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyDumpWindow, "%p", mp);
    mp->Updated = TRUE;

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

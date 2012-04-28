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


extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 w);
extern void DBG_Bin2Hex4(char *str, Uint32 dw);
extern void DBG_DumpAsc(char *str, Uint8 b);



static Uint32 UpdateDisasm(void *obj, Uint32 ival, void *arg )
{
   struct XM7_DbgDisasmDesc *mp = (struct XM7_DbgDisasmDesc *)arg;
   char *str;
   if(mp == NULL) return ival;
   XM7_DbgMemDisasm(mp->disasm);
   return mp->to_tick;

}

static void OnChangeAddrDisasm(AG_Event *event)
{
     AG_Textbox *t = (AG_Textbox *)AG_SELF();
    struct XM7_DbgDisasmDesc *mp = (struct XM7_DbgDisasmDesc *)AG_PTR(1);

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
    mp->disasm->addr = i;
    XM7_DbgMemDisasm(mp->disasm);
}


static void DestroyDisasmWindow(AG_Event *event)
{
    struct XM7_DbgDisasmDesc *mp = (struct XM7_DbgDisasmDesc *)AG_PTR(1);
    if(mp == NULL) return;
    if(mp->disasm != NULL) XM7_DbgDisasmDetach(mp->disasm);
    free(mp);
    mp = NULL;
}



/*
* Auto Update (Timer)
*/
static Uint32 UpdateDumpMemRead(void *obj, Uint32 ival, void *arg )
{

    struct XM7_MemDumpDesc *mp = (struct XM7_MemDumpDesc *)arg;
    char *str;


   if(mp == NULL) return ival;
    XM7_DbgDumpMem(mp->dump);
    return mp->to_tick;
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
    mp->dump->addr = i;
    mp->dump->edaddr = i;
    readmem(mp);
    XM7_DbgDumpMem(mp->dump);
}


static void DestroyDumpWindow(AG_Event *event)
{
    struct XM7_MemDumpDesc *mp = (struct XM7_MemDumpDesc *)AG_PTR(1);
    if(mp == NULL) return;
    if(mp->dump != NULL) XM7_DbgDumpMemDetach(mp->dump);

    free(mp);
    mp = NULL;
}


static Uint32 UpdateRegDump(void *obj, Uint32 ival, void *arg )
{

    struct XM7_DbgRegDumpDesc *mp = (struct XM7_DbgRegDumpDesc *)arg;
    char *str;


   if(mp == NULL) return ival;
    XM7_DbgDumpRegs(mp->dump);
    return mp->to_tick;
}


static void DestroyRegDumpWindow(AG_Event *event)
{
    struct XM7_DbgRegDumpDesc *mp = (struct XM7_DbgRegDumpDesc *)AG_PTR(1);
    if(mp == NULL) return;
    if(mp->dump != NULL) XM7_DbgRegDumpDetach(mp->dump);

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
    AG_TextboxPrintf(addrVar, "%04x", 0x0000);

   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, "Poll");
    AG_TextboxSizeHint(pollVar, "XXXXXX");
    AG_TextboxPrintf(pollVar, "%4d", mp->to_tick);


    hb = AG_HBoxNew(vb, 0);
    if((readFunc != NULL) && (writeFunc != NULL)) {
        dump = XM7_DbgDumpMemInit(hb, readFunc, writeFunc);
        if(dump == NULL) return;
        mp->dump = dump;
        mp->dump->rb = readFunc;
        mp->dump->wb = writeFunc;
        mp->dump->addr = 0x0000;
        mp->dump->edaddr = 0x0000;
    }


    box = AG_BoxNewHoriz(vb, 0);
    box = AG_BoxNewHoriz(vb, 0);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
    box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyDumpWindow, "%p", mp);
    AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p", mp);
    AG_SetEvent(dump->draw, "key-down", XM7_DbgKeyPressFn, "%p", mp);

    AG_SetTimeout(&(mp->to), UpdateDumpMemRead, (void *)mp, AG_CANCEL_ONDETACH | AG_CANCEL_ONLOAD);
    AG_ScheduleTimeout(AGOBJECT(w) , &(mp->to), mp->to_tick);
    AG_WindowShow(w);
}

static void CreateDisasm(AG_Event *event)
{
    AG_Window *w;

   AG_Menu *self = (AG_Menu *)AG_SELF();
   AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
   int type = AG_INT(1);
    int cputype;
    AG_Textbox *pollVar;
    AG_Textbox *addrVar;
    struct XM7_DbgDisasmDesc *mp;


    BYTE (*readFunc)(WORD);
    void FASTCALL (*writeFunc)(WORD, BYTE);
    XM7_DbgDisasm *disasm;

    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;

    mp = (XM7_DbgDisasmDesc *)malloc(sizeof(struct XM7_DbgDisasmDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_DbgDisasmDesc));

//    if(pAddr == NULL) return;
    mp->to_tick = 200;
    w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

    switch(type){
    case MEM_MAIN:
            cputype = MAINCPU;
            AG_WindowSetCaption(w, "Disasm Main memory");
            break;
    case MEM_SUB:
            cputype = SUBCPU;
            AG_WindowSetCaption(w, "Disasm Sub memory");
            break;
    default:
            cputype = -1;
            break;
    }
    addrVar = AG_TextboxNew(AGWIDGET(hb), 0, "Addr");
    AG_TextboxSizeHint(addrVar, "XXXXXX");
    AG_TextboxPrintf(addrVar, "%04x", 0x0000);

   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, "Poll");
   AG_TextboxSizeHint(pollVar, "XXXXXX");
   AG_TextboxPrintf(pollVar, "%4d", mp->to_tick);


   hb = AG_HBoxNew(vb, 0);
   disasm = XM7_DbgDisasmInit(hb, NULL, NULL);
   if(disasm == NULL) return;
   mp->disasm = disasm;
   mp->disasm->addr = 0x0000;
   mp->disasm->cputype = cputype;

    box = AG_BoxNewHoriz(vb, 0);
    box = AG_BoxNewHoriz(vb, 0);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
    box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyDisasmWindow, "%p", mp);
    AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddrDisasm, "%p", mp);
    AG_SetEvent(disasm->draw, "key-down", XM7_DbgDisasmKeyPressFn, "%p", mp);

    AG_SetTimeout(&(mp->to), UpdateDisasm, (void *)mp, AG_CANCEL_ONDETACH | AG_CANCEL_ONLOAD);
    AG_ScheduleTimeout(AGOBJECT(w) , &(mp->to), mp->to_tick);
    AG_WindowShow(w);
}

static void CreateRegDump(AG_Event *event)
{
    AG_Window *w;

   AG_Menu *self = (AG_Menu *)AG_SELF();
   AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
   int type = AG_INT(1);
   int cputype;
   const char *title;
   AG_Textbox *pollVar;
   struct XM7_DbgRegDumpDesc *mp;
   cpu6809_t *cpu;

   XM7_DbgRegDump *regdump;

    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;

    mp = (XM7_DbgRegDumpDesc *)malloc(sizeof(struct XM7_DbgRegDumpDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_DbgRegDumpDesc));

//    if(pAddr == NULL) return;
    mp->to_tick = 200;
    w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

    switch(type){
    case MEM_MAIN:
            cputype = MAINCPU;
            AG_WindowSetCaption(w, "Main CPU Registers");
            cpu = &maincpu;
            title = "Main Regs";
            break;
    case MEM_SUB:
            cputype = SUBCPU;
            AG_WindowSetCaption(w, "Sub CPU Registers");
            cpu = &subcpu;
            title = "Sub Regs";
            break;
    default:
            cputype = -1;
            cpu = NULL;
            title = NULL;
            break;
    }
   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, "Poll");
   AG_TextboxSizeHint(pollVar, "XXXXXX");
   AG_TextboxPrintf(pollVar, "%4d", mp->to_tick);


   hb = AG_HBoxNew(vb, 0);
   regdump = XM7_DbgRegDumpInit(hb, cpu, (char *)title);
   if(regdump == NULL) return;
   mp->dump = regdump;

    box = AG_BoxNewHoriz(vb, 0);
    box = AG_BoxNewHoriz(vb, 0);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
    box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyRegDumpWindow, "%p", mp);

    AG_SetTimeout(&(mp->to), UpdateRegDump, (void *)mp, AG_CANCEL_ONDETACH | AG_CANCEL_ONLOAD);
    AG_ScheduleTimeout(AGOBJECT(w) , &(mp->to), mp->to_tick);
    AG_WindowShow(w);
}

static Uint32 UpdateFdcDump(void *obj, Uint32 ival, void *arg )
{

    struct XM7_DbgFdcDumpDesc *mp = (struct XM7_DbgFdcDumpDesc *)arg;
    char *str;


    if(mp == NULL) return ival;
    XM7_DbgDumpFdc(mp->dump);
    return mp->to_tick;
}


static void DestroyFdcDumpWindow(AG_Event *event)
{
    struct XM7_DbgFdcDumpDesc *mp = (struct XM7_DbgFdcDumpDesc *)AG_PTR(1);
    if(mp == NULL) return;
    if(mp->dump != NULL) XM7_DbgFdcDumpDetach(mp->dump);

    free(mp);
    mp = NULL;
}

static void CreateFdcDump(AG_Event *event)
{
    AG_Window *w;

   AG_Menu *self = (AG_Menu *)AG_SELF();
   AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
   AG_Textbox *pollVar;
   struct XM7_DbgFdcDumpDesc *mp;

   XM7_DbgFdcDump *fdcdump;

    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;

    mp = (XM7_DbgFdcDumpDesc *)malloc(sizeof(struct XM7_DbgFdcDumpDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_DbgFdcDumpDesc));

//    if(pAddr == NULL) return;
    mp->to_tick = 200;
    w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

   AG_WindowSetCaption(w, "FDC Registers");
   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, "Poll");
   AG_TextboxSizeHint(pollVar, "XXXXXX");
   AG_TextboxPrintf(pollVar, "%4d", mp->to_tick);


   hb = AG_HBoxNew(vb, 0);
   fdcdump = XM7_DbgFdcDumpInit(hb);
   if(fdcdump == NULL) return;
   mp->dump = fdcdump;

    box = AG_BoxNewHoriz(vb, 0);
    box = AG_BoxNewHoriz(vb, 0);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
    box = AG_BoxNewHoriz(vb, 0);

    AG_SetEvent(w, "window-close", DestroyFdcDumpWindow, "%p", mp);

    AG_SetTimeout(&(mp->to), UpdateFdcDump, (void *)mp, AG_CANCEL_ONDETACH | AG_CANCEL_ONLOAD);
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
	item = AG_MenuAction(parent, gettext("Disasm Main-Memory"), NULL, CreateDisasm, "%i,%i", MEM_MAIN, 0);
	item = AG_MenuAction(parent, gettext("Disasm Sub-Memory"), NULL, CreateDisasm, "%i,%i", MEM_SUB, 0);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent, gettext("Dump Main-Regs"), NULL, CreateRegDump, "%i,%i", MEM_MAIN, 0);
	item = AG_MenuAction(parent, gettext("Dump Sub-Regs"), NULL, CreateRegDump, "%i,%i", MEM_SUB, 0);
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent, gettext("Dump FDC Regs"), NULL, CreateFdcDump, NULL);
}

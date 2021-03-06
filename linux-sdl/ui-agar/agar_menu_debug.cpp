/*
 * agar_menu_debug.cpp
 *
 *  Created on: 2011/10/25
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include <SDL/SDL.h>
#include <libintl.h>
extern "C" {
#include <agar/core.h>
//#include <agar/core/types.h>
//#include <agar/gui.h>
//#include <agar/gui/opengl.h>
}
#include "xm7.h"
#include "tapelp.h"


#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#else
#include "xm7_sdl.h"
#include "sdl_cfg.h"
#endif

#include "sdl_sch.h"
#include "agar_toolbox.h"
#include "agar_debugger.h"
#include "agar_surfaceconsole.h"
#include "agar_logger.h"
#include <time.h>

#include "../xm7-debugger/memread.h"

extern void OnPushCancel(AG_Event *event);
extern void OnPushCancel2(AG_Event *event);

#ifdef _WITH_DEBUGGER

extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 w);
extern void DBG_Bin2Hex4(char *str, Uint32 dw);
extern void DBG_DumpAsc(char *str, Uint8 b);

#ifdef _WITH_DEBUGGER
   char DebuggerTextFont[MAXPATHLEN + 1];
   char DebuggerSymFont[MAXPATHLEN + 1];
#endif // _WITH_DEBUGGER

/*
 * Fix Fonts
 */

static void DbgInitFont(void)
{
   
}

static void DbgDetachFont(void)
{

}



static Uint32 UpdateDisasm(AG_Timer *timer, AG_Event *event)
{
   struct XM7_DbgDisasmDesc *mp = (struct XM7_DbgDisasmDesc *)AG_PTR(1);
   char *str;
   if(timer == NULL) return 0;
   if(mp == NULL) return timer->ival;
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


static void OnClosedDisasmWindow(AG_Event *event)
{
    struct XM7_DbgDisasmDesc *mp = (struct XM7_DbgDisasmDesc *)AG_PTR(1);
    if(mp != NULL) {
       if(mp->disasm != NULL) XM7_DbgDisasmDetach(mp->disasm);
       free(mp);
    }
}


static void DestroyDisasmWindow(AG_Event *event)
{
    struct XM7_DbgDisasmDesc *mp = (struct XM7_DbgDisasmDesc *)AG_PTR(1);
//    AG_Timer *timer = (AG_Timer *)AG_PTR(2);
    AG_Button *self = (AG_Button *)AG_SELF();


     OnClosedDisasmWindow(event);
     if(self != NULL) {
      AG_WindowHide(self->wid.window);
      AG_ObjectDetach(self->wid.window);
   }
}



/*
* Auto Update (Timer)
*/
static Uint32 UpdateDumpMemRead(AG_Timer *timer, AG_Event *event)
{

    struct XM7_MemDumpDesc *mp = (struct XM7_MemDumpDesc *)AG_PTR(1);
    char *str;

   if(timer == NULL) return 0;
   if(mp == NULL) return timer->ival;
    XM7_DbgDumpMem(mp->dump);
    return timer->ival;
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
    void *self = AG_SELF();
    if(mp == NULL) return;

    mp->to = NULL;
    if(mp->dump != NULL) XM7_DbgDumpMemDetach(mp->dump);
    free(mp);
}


static Uint32 UpdateRegDump(AG_Timer *timer, AG_Event *event)
{
    struct XM7_DbgRegDumpDesc *mp = (struct XM7_DbgRegDumpDesc *)AG_PTR(1);

    if(timer == NULL) return 0;
    if(mp == NULL) return timer->ival;
    XM7_DbgDumpRegs(mp->dump);
    return mp->to_tick;
}


static void DestroyRegDumpWindow(AG_Event *event)
{
    struct XM7_DbgRegDumpDesc *mp = (struct XM7_DbgRegDumpDesc *)AG_PTR(1);
    void *self = AG_SELF();
    if(mp == NULL) return;
    if(mp->dump != NULL) XM7_DbgRegDumpDetach(mp->dump);
    free(mp);

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
    volatile void FASTCALL (*writeFunc)(WORD, BYTE);
    XM7_DbgDump *dump;
    AG_Timer *timerhdr;
   
    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;

    mp = (XM7_MemDumpDesc *)malloc(sizeof(struct XM7_MemDumpDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_MemDumpDesc));

    mp->to_tick = 200;
//    w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOCLOSE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
    w = AG_WindowNew(AG_WINDOW_NOBACKGROUND | AG_WINDOW_MODAL);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

    switch(type){
    case MEM_MAIN:
            readFunc = rb_main;
            writeFunc = mainmem_writeb;
            AG_WindowSetCaption(w, gettext("Dump Main memory"));
            break;
    case MEM_SUB:
            readFunc = rb_sub;
            writeFunc = submem_writeb;
            AG_WindowSetCaption(w, gettext("Dump Sub memory"));
            break;
    default:
            readFunc = NULL;
            writeFunc = NULL;
            break;
    }
    addrVar = AG_TextboxNew(AGWIDGET(hb), 0, gettext("Addr"));
    AG_TextboxSizeHint(addrVar, "XXXXXX");
    AG_TextboxPrintf(addrVar, "%04x", 0x0000);

   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, gettext("Poll"));
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

    timerhdr = AG_AddTimerAuto (AGOBJECT(w), mp->to_tick, UpdateDumpMemRead, "%p", (void *)mp);

    box = AG_BoxNewHoriz(vb, 0);
    box = AG_BoxNewHoriz(vb, 0);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
    box = AG_BoxNewHoriz(vb, 0);

    AG_AddEvent(w, "window-close", DestroyDumpWindow, "%p", mp);
    AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddr, "%p", mp);
    AG_SetEvent(dump->draw, "key-down", XM7_DbgKeyPressFn, "%p", mp);

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
    AG_Timer *timerhdr;

    mp = (XM7_DbgDisasmDesc *)malloc(sizeof(struct XM7_DbgDisasmDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_DbgDisasmDesc));

    mp->to_tick = 200;
//    w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOCLOSE | AG_WINDOW_NOMAXIMIZE | FILEDIALOG_WINDOW_DEFAULT);
    w = AG_WindowNew(FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

    switch(type){
    case MEM_MAIN:
            cputype = MAINCPU;
            AG_WindowSetCaption(w, gettext("Disasm Main memory"));
            break;
    case MEM_SUB:
            cputype = SUBCPU;
            AG_WindowSetCaption(w, gettext("Disasm Sub memory"));
            break;
    default:
            cputype = -1;
            break;
    }
    addrVar = AG_TextboxNew(AGWIDGET(hb), 0, gettext("Addr"));
    AG_TextboxSizeHint(addrVar, "XXXXXX");
    AG_TextboxPrintf(addrVar, "%04x", 0x0000);

   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, gettext("Poll"));
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

    timerhdr = AG_AddTimerAuto(AGOBJECT(w), mp->to_tick, UpdateDisasm, "%p", (void *)mp);
    //btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel2, "%p", timerhdr);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), DestroyDisasmWindow, "%p", mp);
    box = AG_BoxNewHoriz(vb, 0);

    AG_AddEvent(w, "window-close", OnClosedDisasmWindow, "%p", mp);
    AG_SetEvent(addrVar, "textbox-postchg", OnChangeAddrDisasm, "%p", mp);
    AG_SetEvent(disasm->draw, "key-down", XM7_DbgDisasmKeyPressFn, "%p", mp);

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
   AG_Timer *timerhdr;
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

    mp->to_tick = 200;
    w = AG_WindowNew(FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

    switch(type){
    case MEM_MAIN:
            cputype = MAINCPU;
            AG_WindowSetCaption(w, gettext("Main CPU Registers"));
            cpu = &maincpu;
            title = "Main Regs";
            break;
    case MEM_SUB:
            cputype = SUBCPU;
            AG_WindowSetCaption(w, gettext("Sub CPU Registers"));
            cpu = &subcpu;
            title = "Sub Regs";
            break;
    default:
            cputype = -1;
            cpu = NULL;
            title = NULL;
            break;
    }
   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, gettext("Poll"));
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

    AG_AddEvent(w, "window-close", DestroyRegDumpWindow, "%p", mp);

    timerhdr = AG_AddTimerAuto(AGOBJECT(w), mp->to_tick, UpdateRegDump, "%p", (void *)mp);

    AG_WindowShow(w);
}

static Uint32 UpdateFdcDump(AG_Timer *timer, AG_Event *event)
{

    struct XM7_DbgFdcDumpDesc *mp = (struct XM7_DbgFdcDumpDesc *)AG_PTR(1);

    if(timer == NULL) return 0;
    if(mp == NULL) return timer->ival;
    XM7_DbgDumpFdc(mp->dump);
    return mp->to_tick;
}


static void DestroyFdcDumpWindow(AG_Event *event)
{
    struct XM7_DbgFdcDumpDesc *mp = (struct XM7_DbgFdcDumpDesc *)AG_PTR(1);
    void *self = (void *)AG_SELF();

    if(mp == NULL) return;
    if(mp->dump != NULL) XM7_DbgFdcDumpDetach(mp->dump);
    free(mp);
}

static void CreateFdcDump(AG_Event *event)
{
    AG_Window *w;

   AG_Menu *self = (AG_Menu *)AG_SELF();
   AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
   AG_Textbox *pollVar;
   struct XM7_DbgFdcDumpDesc *mp;

   XM7_DbgFdcDump *fdcdump;
   AG_Timer *timerhdr;
   
    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;

    mp = (XM7_DbgFdcDumpDesc *)malloc(sizeof(struct XM7_DbgFdcDumpDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_DbgFdcDumpDesc));

    mp->to_tick = 200;
    w = AG_WindowNew(FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

   AG_WindowSetCaption(w, gettext("FDC Registers"));
   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, gettext("Poll"));
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

    AG_AddEvent(w, "window-close", DestroyFdcDumpWindow, "%p", mp);
    timerhdr = AG_AddTimerAuto(AGOBJECT(w), mp->to_tick, UpdateFdcDump, "%p", (void *)mp);

    AG_WindowShow(w);
}



static Uint32 UpdateMMRDump(AG_Timer *timer, AG_Event *event)
{

    struct XM7_DbgMMRDumpDesc *mp = (struct XM7_DbgMMRDumpDesc *)AG_PTR(1);
    if(timer == NULL) return 0;
    if(mp == NULL) return timer->ival;
    XM7_DbgDumpMMR(mp->dump);
    return mp->to_tick;
}


static void DestroyMMRDumpWindow(AG_Event *event)
{
    struct XM7_DbgMMRDumpDesc *mp = (struct XM7_DbgMMRDumpDesc *)AG_PTR(1);
    void *self = AG_SELF();

    if(mp == NULL) return;
    if(mp->dump != NULL) XM7_DbgDumpMMRDetach(mp->dump);
    free(mp);
}



static void CreateMMRDump(AG_Event *event)
{
    AG_Window *w;

   AG_Menu *self = (AG_Menu *)AG_SELF();
   AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
   AG_Textbox *pollVar;
   struct XM7_DbgMMRDumpDesc *mp;

   XM7_DbgMMRDump *mmrdump;
   AG_Timer *timerhdr;
   
    AG_HBox *hb;
    AG_VBox *vb;
    AG_Box *box;
    AG_Button *btn;

    mp = (XM7_DbgMMRDumpDesc *)malloc(sizeof(struct XM7_DbgMMRDumpDesc));
    if(mp == NULL) return;
    memset(mp, 0x00, sizeof(struct XM7_DbgMMRDumpDesc));

    mp->to_tick = 200;
    w = AG_WindowNew(FILEDIALOG_WINDOW_DEFAULT);
    AG_WindowSetMinSize(w, 230, 80);
    vb =AG_VBoxNew(w, 0);

    hb = AG_HBoxNew(vb, 0);

   AG_WindowSetCaption(w, gettext("MMR Bank-map"));
   pollVar = AG_TextboxNew(AGWIDGET(hb), 0, gettext("Poll"));
   AG_TextboxSizeHint(pollVar, "XXXXXX");
   AG_TextboxPrintf(pollVar, "%4d", mp->to_tick);


   hb = AG_HBoxNew(vb, 0);
   mmrdump = XM7_DbgDumpMMRInit(hb);
   if(mmrdump == NULL) return;
   mp->dump = mmrdump;

    box = AG_BoxNewHoriz(vb, 0);
    box = AG_BoxNewHoriz(vb, 0);
    btn = AG_ButtonNewFn (AGWIDGET(box), 0, gettext("Close"), OnPushCancel, NULL);
    box = AG_BoxNewHoriz(vb, 0);

    AG_AddEvent(w, "window-close", DestroyMMRDumpWindow, "%p", mp);

    AG_AddTimerAuto(AGOBJECT(w), mp->to_tick, UpdateMMRDump, "%p", (void *)mp);
    AG_WindowShow(w);
}
#endif // _WITH_DEBUGGER

extern "C" {
   extern BOOL            bLogSTDOUT;
   extern BOOL            bLogSYSLOG;
}


static void OnChangeLogStatus(AG_Event *event)
{
   AG_Menu *parent = (AG_Menu *)AG_SELF();
   AG_MenuItem *my = (AG_MenuItem *)AG_SENDER();
   BOOL flag = (BOOL)AG_INT(1);
   BOOL *tg  = (BOOL *)AG_PTR(2);
   char *target = (char *)AG_STRING(3);
   if(flag == FALSE) {
        *tg = FALSE;
        AG_SetInt(agConfig, target, FALSE);
   } else {
        *tg = TRUE;
        AG_SetInt(agConfig, target, TRUE);
	if(XM7_LogGetStatus() == FALSE) {
	   XM7_OpenLog(bLogSYSLOG, bLogSTDOUT);
	}
   }
   XM7_SetLogStdOut(bLogSTDOUT);   
}


static void DisplayLogStatus(AG_Event *event)
{
   AG_Menu *parent = (AG_Menu *)AG_SELF();
   AG_MenuItem *my = (AG_MenuItem *)AG_SENDER();
   BOOL *flag = (BOOL *)AG_PTR(1);
   
   if(*flag == FALSE) {
      AG_MenuSetLabel(my, "　OFF");
   } else {
      AG_MenuSetLabel(my, "■ON");
   }
}


void Create_DebugMenu(AG_MenuItem *parent)
{
   	AG_MenuItem *item;
   	AG_MenuItem *subitem ;
        AG_Toolbar *toolbar;

#ifdef _WITH_DEBUGGER
        DbgInitFont(); //

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
	AG_MenuSeparator(parent);
	item = AG_MenuAction(parent, gettext("Dump MMR"), NULL, CreateMMRDump, NULL);
	AG_MenuSeparator(parent);
#endif // _WITH_DEBUGGER
	item = AG_MenuNode(parent, gettext("Log to STDOUT"), NULL); 
        subitem = AG_MenuAction(item, gettext("ON"),  NULL, OnChangeLogStatus, "%i%p%s", TRUE,  &bLogSTDOUT, "logger.stdout"); 
        subitem = AG_MenuAction(item, gettext("OFF"), NULL, OnChangeLogStatus, "%i%p%s", FALSE, &bLogSTDOUT, "logger.stdout"); 
        AG_MenuToolbar(item, NULL);
//        item =AG_MenuDynamicItem(parent, "", NULL, DisplayLogStatus,"%p", &bLogSTDOUT);
	AG_MenuSeparator(parent);
   
	item = AG_MenuNode(parent, gettext("Log to SYSLOG"), NULL); 
        subitem = AG_MenuAction(item, "ON",  NULL, OnChangeLogStatus, "%i%p%s", TRUE,  &bLogSYSLOG, "logger.syslog"); 
        subitem = AG_MenuAction(item, "OFF", NULL, OnChangeLogStatus, "%i%p%s", FALSE, &bLogSYSLOG, "logger.syslog"); 
        AG_MenuToolbar(item, NULL);
//        item =AG_MenuDynamicItem(parent, "", NULL, DisplayLogStatus,"%p", &bLogSYSLOG);
//	AG_MenuSeparator(parent);
}


void Detach_DebugMenu(void)
{  
#ifdef _WITH_DEBUGGER
  DbgDetachFont();
#endif
}


  

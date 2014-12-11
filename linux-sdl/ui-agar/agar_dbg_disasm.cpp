/*
 * Debugger for XM-7/SDL : Disassembler
 * (C) 2012 K.Ohta <whatisthis.sowhar@gmail.com>
 * History:
 *        24 Apr 2012 : Branch from agar_debugger.cpp
 */

#include "agar_surfaceconsole.h"
#include "agar_debugger.h"
#include "xm7.h"

// DisAsm
extern "C" {
extern int FASTCALL disline2(int cpu, WORD pcreg, cpu6809_t *cpuset, char *buffer);
   
static void XM7_DbgDisasmDrawFn(AG_Event *event)
{
   XM7_SDLView *view = (XM7_SDLView *)AG_SELF();
   struct XM7_DbgDisasm *p = (struct XM7_DbgDisasm *)AG_PTR(1);
   BOOL forceredraw = AG_INT(2);

   if(p == NULL) return;
   XM7_ConsoleUpdate(view, p->cons, forceredraw);
}

}


struct XM7_DbgDisasm *XM7_DbgDisasmInit(void *parent, BYTE (*rf)(WORD), void (*wf)(WORD, BYTE))
{
    struct XM7_DbgDisasm *obj;
    XM7_SDLView *view;
    DumpObject  *cons;
    AG_Box      *box;
    BYTE        *buf;
    AG_SizeAlloc a;
    AG_Surface *s;
    int chrw,chrh;

    if(parent == NULL) return NULL;
    obj = (struct XM7_DbgDisasm *)malloc(sizeof(struct XM7_DbgDisasm));
    if(obj == NULL) return NULL;
    memset(obj, 0x00, sizeof(struct XM7_DbgDisasm));
    cons = new DumpObject;
    if(cons == NULL) {
        free(obj);
        return NULL;
    }
    buf = (BYTE *)malloc(sizeof(BYTE) * 20 * 5);
    if(buf == NULL){
        free(obj);
        delete cons;
        return NULL;
    }
    memset(buf, 0x00, sizeof(BYTE) * 20 * 5);
    box = AG_BoxNew(parent, AG_BOX_HORIZ, 0);
    view = XM7_SDLViewNew(AGWIDGET(box), NULL, "");
    obj->addr = 0x0000;
    obj->cons = cons;
    obj->draw = view;
    obj->buf = buf;
    obj->forceredraw = FALSE;
   
    XM7_ConsoleSetup(view, cons, obj, XM7_DbgDisasmDrawFn, 40, 22, FALSE);
    return obj;
}

void XM7_DbgDisasmDetach(struct XM7_DbgDisasm *dbg)
{
    if(dbg == NULL) return;
    if(dbg->draw != NULL) {
        AG_ObjectDetach(dbg->draw);
    }
    if(dbg->buf != NULL) free(dbg->buf);
    if(dbg->cons != NULL) delete dbg->cons;
    free(dbg);
}


static void XM7_DbgDisasmSetAddress(AG_Event *event)
{
   XM7_SDLView *disp = (XM7_SDLView *)AG_SELF();
   struct XM7_DbgDisasmDesc *p = (struct XM7_DbgDisasmDesc *)AG_PTR(1);
   int sym = AG_INT(2);
   int mod = AG_INT(3);
   Uint32  unicode = (Uint32)AG_ULONG(4);
   DumpObject *cons;
   char strbuf[16];
   Uint16 hb = 0;

   if(p == NULL) return;
   if(p->disasm == NULL) return;
   if(p->disasm->cons == NULL) return;
   cons = p->disasm->cons;
   strbuf[0] = '\0';
   
   if(p->disasm->paused == TRUE) {
	p->disasm->paused = FALSE;
        cons->ClearScreen();
        return;
   }
   p->disasm->editAddr = TRUE;

   switch(sym) {
    case AG_KEY_ESCAPE:
      p->disasm->baddr = 0x0000;
      p->disasm->bdata = 0x0000;
      p->disasm->editAddr = FALSE;
      return;
      break;
    case AG_KEY_0:
    case AG_KEY_KP0:
      hb = 0;
      p->disasm->baddr++;
      break;
    case AG_KEY_1:
    case AG_KEY_KP1:
      hb = 1;
      p->disasm->baddr++;
      break;
    case AG_KEY_2:
    case AG_KEY_KP2:
      hb = 2;
      p->disasm->baddr++;
      break;
    case AG_KEY_3:
    case AG_KEY_KP3:
      hb = 3;
      p->disasm->baddr++;
      break;
    case AG_KEY_4:
    case AG_KEY_KP4:
      hb = 4;
      p->disasm->baddr++;
      break;
    case AG_KEY_5:
    case AG_KEY_KP5:
      hb = 5;
      p->disasm->baddr++;
      break;
    case AG_KEY_6:
    case AG_KEY_KP6:
      hb = 6;
      p->disasm->baddr++;
      break;
    case AG_KEY_7:
    case AG_KEY_KP7:
      hb = 7;
      p->disasm->baddr++;
      break;
    case AG_KEY_8:
    case AG_KEY_KP8:
      hb = 8;
      p->disasm->baddr++;
      break;
    case AG_KEY_9:
    case AG_KEY_KP9:
      hb = 9;
      p->disasm->baddr++;
      break;
    case AG_KEY_A:
      hb = 0x0a;
      p->disasm->baddr++;
      break;
    case AG_KEY_B:
      hb = 0x0b;
      p->disasm->baddr++;
      break;
    case AG_KEY_C:
      hb = 0x0c;
      p->disasm->baddr++;
      break;
    case AG_KEY_D:
      hb = 0x0d;
      p->disasm->baddr++;
      break;
    case AG_KEY_E:
      hb = 0x0e;
      p->disasm->baddr++;
      break;
    case AG_KEY_F:
      hb = 0x0f;
      p->disasm->baddr++;
      break;
    default:
      return;
      break;
   }

   p->disasm->bdata |= hb << ((4 - p->disasm->baddr) * 4);
   {
      Uint8 c;
      c = hex2chr(hb);
      cons->MoveDrawPos(12 + p->disasm->baddr, 0);
      cons->PutChar(c);
   }
   
   
   if(p->disasm->baddr >= 4) {
      p->disasm->addr = p->disasm->bdata & 0x0000ffff;
      p->disasm->bdata = 0;
      p->disasm->baddr = 0;
      p->disasm->editAddr = FALSE;
   }
   
}


void XM7_DbgDisasmKeyPressFn(AG_Event *event)
{
   	// キーハンドラー
   XM7_SDLView *disp = (XM7_SDLView *)AG_SELF();
   struct XM7_DbgDisasmDesc *p = (struct XM7_DbgDisasmDesc *)AG_PTR(1);
   int sym = AG_INT(2);
   int mod = AG_INT(3);
   Uint32  unicode = (Uint32)AG_ULONG(4);
   DumpObject *cons;
   
//   printf("Key Press %04x %04x %08x\n", sym, mod, p);
   if(p == NULL) return;
   if(p->disasm == NULL) return;
   if(p->disasm->cons == NULL) return;
   cons = p->disasm->cons;
   
   if(p->disasm->paused == TRUE) {
	p->disasm->paused = FALSE;
        cons->ClearScreen();
        return;
   }
   if(p->disasm->editAddr == TRUE) {
	XM7_DbgDisasmSetAddress(event);
        return;
   }
   
	
   switch(sym) {
    case AG_KEY_H:
      // Help
      cons->ClearScreen();
      cons->MoveDrawPos(0, 0);
      cons->PutString("disasm Memory Help:");
      cons->MoveDrawPos(0, 1);
      cons->PutString("J/j or / : Set Address");
      cons->MoveDrawPos(0, 2);
      cons->PutString("UP        : Roll UP");
      cons->MoveDrawPos(0, 3);
      cons->PutString("DOWN      : Roll Down");
      cons->MoveDrawPos(0, 4);
      cons->PutString("Page Up   : Page UP");
      cons->MoveDrawPos(0, 5);
      cons->PutString("Page Down : Page Down");
      cons->MoveDrawPos(0, 21);
      cons->PutString("Press Any Key");
      p->disasm->paused = TRUE;
      break;
    case AG_KEY_J:
    case AG_KEY_SLASH:
      // Set Address
      cons->MoveDrawPos(0, 0);
      cons->PutString("Set Address >>         ");
      cons->MoveDrawPos(0, 0);
      p->disasm->editAddr = TRUE;
      break;
    case AG_KEY_UP:
         p->disasm->nextaddr = p->disasm->addr;
	 if((p->disasm->addr > (p->disasm->beforeaddr + 5)) || (p->disasm->addr < (p->disasm->beforeaddr - 5))) {
	    p->disasm->addr -= 1;
	    p->disasm->beforeaddr = p->disasm->addr - 1;
	 } else {
	   p->disasm->addr = p->disasm->beforeaddr;
	   p->disasm->beforeaddr = p->disasm->addr - 1;
	 }
	 p->disasm->addr &= 0x0000ffff;
         p->disasm->beforeaddr &= 0x0000ffff;
      break;
    case AG_KEY_DOWN:
         p->disasm->beforeaddr = p->disasm->addr;
         p->disasm->addr = p->disasm->nextaddr;
         p->disasm->nextaddr += 1;
	 p->disasm->addr &= 0x0000ffff;
	 p->disasm->nextaddr &= 0x0000ffff;
      break;
    default:
      break;
   }
   
}

void XM7_DbgMemDisasm(void *p)
{
    int a;
    int Hb,Wb;
    int i,j;
    int wd,hd;
    char strbuf[256];
    BYTE *buf;
    int addr;
    int next;
    XM7_DbgDisasm *obj;

    obj = (XM7_DbgDisasm *)p;

    if(obj == NULL) return;
    if(obj->cons == NULL) return;
    if(obj->draw == NULL) return;
    if(obj->paused == TRUE) return;
    addr = obj->addr;

    Wb = obj->cons->GetWidth();
    Hb = obj->cons->GetHeight();
    wd = 40;
    hd = 18;
    obj->cons->MoveDrawPos(30, 0);
    obj->cons->PutString("H: Help");

    if(obj->editAddr) return;
	
   obj->cons->MoveDrawPos(0, 0);
   sprintf(strbuf, "DisAssemble at %04x", obj->addr);
    obj->cons->PutString(strbuf);

    obj->cons->MoveDrawPos(0, 1);
    if(wd >= (Wb / 5)) wd = Wb / 5;
    if(hd >= (Hb - 3)) hd = Hb - 3;
    buf = obj->buf;
    for(i = 0; i < Wb - 2; i++){
        obj->cons->MoveDrawPos(i, 1);
        obj->cons->PutChar('-');
    }
    for(i = 0;i < hd; i++){
        strbuf[0] = '\0';
        obj->cons->MoveDrawPos(0, 2 + i);
        next = disline(obj->cputype, addr, strbuf);
        if(i == 0) obj->nextaddr = next;
        strcat(strbuf, "       ");
        obj->cons->PutString(strbuf);
        addr = next;
    }
    obj->cons->MoveDrawPos(0, 0);

}

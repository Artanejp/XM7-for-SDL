/*
* Debugger for XM7
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* History:
* 21 Mar,2012 Initial
*/

#include "agar_surfaceconsole.h"
#include "agar_debugger.h"



/*
* WIDGET
*/
extern void DBG_PrintYSum(char *str, int *sum, int totalSum, int width);
extern void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD addr, int sum, int bytes, BOOL AddrFlag);
extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 b);
extern void DBG_Bin2Hex4(char *str, Uint32 b);

extern "C" {

static void XM7_DbgDumpDrawFn(AG_Event *event)
{
   XM7_SDLView *view = (XM7_SDLView *)AG_SELF();
    struct XM7_DbgDump *p = (struct XM7_DbgDump *)AG_PTR(1);
    BOOL forceredraw = AG_INT(2);

    if(p == NULL) return;
    p->dump->Draw(forceredraw);
    AG_WidgetUpdateSurface(AGWIDGET(view), view->mySurface);

}

static void XM7_DbgDumpEditMem(AG_Event *event)
{
   XM7_SDLView *disp = (XM7_SDLView *)AG_SELF();
   struct XM7_MemDumpDesc *p = (struct XM7_MemDumpDesc *)AG_PTR(1);
   int sym = AG_INT(2);
   int mod = AG_INT(3);
   Uint32  unicode = (Uint32)AG_ULONG(4);
   DumpObject *cons;
   Uint8 hb = 0;

   if(p == NULL) return;
   if(p->dump == NULL) return;
   if(p->dump->baddr >= 2) {
      p->dump->baddr = 0;
      p->dump->bdata = 0;
   }
   
   switch(sym) {
    case AG_KEY_ESCAPE:
      if(p->dump->writeEnable) {
	 p->dump->writeEnable = FALSE;
      } else {
	 p->dump->writeEnable = TRUE;
      }
      p->dump->baddr = 0x0000;
      p->dump->bdata = 0x0000;
      p->dump->editHex = FALSE;
      return;
      break;
    case AG_KEY_0:
    case AG_KEY_KP0:
      hb = 0;
      p->dump->baddr++;
      break;
    case AG_KEY_1:
    case AG_KEY_KP1:
      hb = 1;
      p->dump->baddr++;
      break;
    case AG_KEY_2:
    case AG_KEY_KP2:
      hb = 2;
      p->dump->baddr++;
      break;
    case AG_KEY_3:
    case AG_KEY_KP3:
      hb = 3;
      p->dump->baddr++;
      break;
    case AG_KEY_4:
    case AG_KEY_KP4:
      hb = 4;
      p->dump->baddr++;
      break;
    case AG_KEY_5:
    case AG_KEY_KP5:
      hb = 5;
      p->dump->baddr++;
      break;
    case AG_KEY_6:
    case AG_KEY_KP6:
      hb = 6;
      p->dump->baddr++;
      break;
    case AG_KEY_7:
    case AG_KEY_KP7:
      hb = 7;
      p->dump->baddr++;
      break;
    case AG_KEY_8:
    case AG_KEY_KP8:
      hb = 8;
      p->dump->baddr++;
      break;
    case AG_KEY_9:
    case AG_KEY_KP9:
      hb = 9;
      p->dump->baddr++;
      break;
    case AG_KEY_A:
      hb = 0x0a;
      p->dump->baddr++;
      break;
    case AG_KEY_B:
      hb = 0x0b;
      p->dump->baddr++;
      break;
    case AG_KEY_C:
      hb = 0x0c;
      p->dump->baddr++;
      break;
    case AG_KEY_D:
      hb = 0x0d;
      p->dump->baddr++;
      break;
    case AG_KEY_E:
      hb = 0x0e;
      p->dump->baddr++;
      break;
    case AG_KEY_F:
      hb = 0x0f;
      p->dump->baddr++;
      break;
    case AG_KEY_J:
      hb = 0x00;
      p->dump->baddr = 0;
      p->dump->bdata = 0;
      p->dump->editAddr = TRUE;
      return;
      break;
    case AG_KEY_UP:
      p->dump->baddr = 0;
      p->dump->bdata = 0;
      p->dump->edaddr -= 16;
      p->dump->edaddr &= 0x0000ffff;
      break;
    case AG_KEY_DOWN:
      p->dump->baddr = 0;
      p->dump->bdata = 0;
      p->dump->edaddr += 16;
      p->dump->edaddr &= 0x0000ffff;
      break;
    case AG_KEY_LEFT:
      p->dump->baddr = 0;
      p->dump->bdata = 0;
      p->dump->edaddr -= 1;
      p->dump->edaddr &= 0x0000ffff;
      break;
    case AG_KEY_RIGHT:
      p->dump->baddr = 0;
      p->dump->bdata = 0;
      p->dump->edaddr += 1;
      p->dump->edaddr &= 0x0000ffff;
      break;
    default:
      return;
      break;
   }


   p->dump->bdata |= hb << ((2 - p->dump->baddr)  * 4);
   
   if(p->dump->baddr >= 2) {
      if(p->dump->writeEnable) {
	   if(p->dump->wb != NULL) {
	      p->dump->wb((WORD)p->dump->edaddr & 0x0ffff, (BYTE)(p->dump->bdata & 0x0ff));
	   }
      }
      p->dump->baddr = 0;
      p->dump->bdata = 0;
      p->dump->edaddr = (p->dump->edaddr + 1) & 0x0000ffff;

   }
   if(((p->dump->addr + 256) & 0x0000ffff) <= p->dump->edaddr){
      p->dump->addr = ((p->dump->addr & 0xff)  + (p->dump->edaddr & 0xff00)) & 0x0000ffff;
   }else if(p->dump->addr > p->dump->edaddr) { 
      p->dump->addr = ((p->dump->addr & 0xff)  + (p->dump->edaddr & 0xff00)) & 0x0000ffff;
   }

   
	
}

static void XM7_DbgDumpSetAddress(AG_Event *event)
{
   XM7_SDLView *disp = (XM7_SDLView *)AG_SELF();
   struct XM7_MemDumpDesc *p = (struct XM7_MemDumpDesc *)AG_PTR(1);
   int sym = AG_INT(2);
   int mod = AG_INT(3);
   Uint32  unicode = (Uint32)AG_ULONG(4);
   DumpObject *cons;
   char strbuf[16];
   Uint16 hb = 0;

   if(p == NULL) return;
   if(p->dump == NULL) return;
   if(p->dump->dump == NULL) return;
   cons = p->dump->dump;
   strbuf[0] = '\0';
   
   if(p->dump->paused == TRUE) {
	p->dump->paused = FALSE;
        cons->ClearScreen();
        return;
   }
   p->dump->editAddr = TRUE;

   switch(sym) {
    case AG_KEY_ESCAPE:
      p->dump->baddr = 0x0000;
      p->dump->bdata = 0x0000;
      p->dump->editAddr = FALSE;
      return;
      break;
    case AG_KEY_0:
    case AG_KEY_KP0:
      hb = 0;
      p->dump->baddr++;
      break;
    case AG_KEY_1:
    case AG_KEY_KP1:
      hb = 1;
      p->dump->baddr++;
      break;
    case AG_KEY_2:
    case AG_KEY_KP2:
      hb = 2;
      p->dump->baddr++;
      break;
    case AG_KEY_3:
    case AG_KEY_KP3:
      hb = 3;
      p->dump->baddr++;
      break;
    case AG_KEY_4:
    case AG_KEY_KP4:
      hb = 4;
      p->dump->baddr++;
      break;
    case AG_KEY_5:
    case AG_KEY_KP5:
      hb = 5;
      p->dump->baddr++;
      break;
    case AG_KEY_6:
    case AG_KEY_KP6:
      hb = 6;
      p->dump->baddr++;
      break;
    case AG_KEY_7:
    case AG_KEY_KP7:
      hb = 7;
      p->dump->baddr++;
      break;
    case AG_KEY_8:
    case AG_KEY_KP8:
      hb = 8;
      p->dump->baddr++;
      break;
    case AG_KEY_9:
    case AG_KEY_KP9:
      hb = 9;
      p->dump->baddr++;
      break;
    case AG_KEY_A:
      hb = 0x0a;
      p->dump->baddr++;
      break;
    case AG_KEY_B:
      hb = 0x0b;
      p->dump->baddr++;
      break;
    case AG_KEY_C:
      hb = 0x0c;
      p->dump->baddr++;
      break;
    case AG_KEY_D:
      hb = 0x0d;
      p->dump->baddr++;
      break;
    case AG_KEY_E:
      hb = 0x0e;
      p->dump->baddr++;
      break;
    case AG_KEY_F:
      hb = 0x0f;
      p->dump->baddr++;
      break;
    default:
      return;
      break;
   }

   cons->MoveDrawPos(0, 0);
   p->dump->bdata |= hb << ((4 - p->dump->baddr) * 4);
   sprintf(strbuf, "Set Address>>%04x", p->dump->bdata);
   cons->PutString(strbuf);
   
   if(p->dump->baddr >= 4) {
      p->dump->edaddr = p->dump->bdata & 0x0000ffff;
      p->dump->bdata = 0;
      p->dump->baddr = 0;
      p->dump->editAddr = FALSE;
   }
   
   if(((p->dump->addr + 256) & 0x0000ffff) <= p->dump->edaddr){
      p->dump->addr = ((p->dump->addr & 0xff)  + (p->dump->edaddr & 0xff00)) & 0x0000ffff;
   }else if(p->dump->addr > p->dump->edaddr) { 
      p->dump->addr = ((p->dump->addr & 0xff)  + (p->dump->edaddr & 0xff00)) & 0x0000ffff;
   }
}
   
void XM7_DbgKeyPressFn(AG_Event *event)
{
   	// キーハンドラー
   XM7_SDLView *disp = (XM7_SDLView *)AG_SELF();
   struct XM7_MemDumpDesc *p = (struct XM7_MemDumpDesc *)AG_PTR(1);
   int sym = AG_INT(2);
   int mod = AG_INT(3);
   Uint32  unicode = (Uint32)AG_ULONG(4);
   DumpObject *cons;
   
//   printf("Key Press %04x %04x %08x\n", sym, mod, p);
   if(p == NULL) return;
   if(p->dump == NULL) return;
   if(p->dump->dump == NULL) return;
   cons = p->dump->dump;
   
   if(p->dump->paused == TRUE) {
	p->dump->paused = FALSE;
        cons->ClearScreen();
        return;
   }
   if(p->dump->editAddr == TRUE) {
	XM7_DbgDumpSetAddress(event);
        return;
   }
   if(p->dump->editHex == TRUE) {
	XM7_DbgDumpEditMem(event);
        return;
   }
   
	
   switch(sym) {
    case AG_KEY_H:
      // Help
      cons->ClearScreen();
      cons->MoveDrawPos(0, 0);
      cons->PutString("Dump Memory Help:");
      cons->MoveDrawPos(0, 1);
      cons->PutString("J/j       : Set Address");
      cons->MoveDrawPos(40, 1);
      cons->PutString("ESC       : Toggle Write");
      cons->MoveDrawPos(0, 3);
      cons->PutString("Shift+UP  : Roll UP");
      cons->MoveDrawPos(0, 4);
      cons->PutString("Shift+DOWN: Roll Down");
      cons->MoveDrawPos(40, 3);
      cons->PutString("Page Up   : Page UP");
      cons->MoveDrawPos(40, 4);
      cons->PutString("Page Up   : Page UP");
      cons->MoveDrawPos(0, 10);
      cons->PutString("Edit Mode");
      cons->MoveDrawPos(0, 11);
      cons->PutString("0-9,A-F   : Write Memory");
      cons->MoveDrawPos(40, 21);
      cons->PutString("Press Any Key");
      p->dump->paused = TRUE;
      break;
    case AG_KEY_J:
      // Set Address
      cons->MoveDrawPos(0, 0);
      cons->PutString("Set-Address>>");
      p->dump->editAddr = TRUE;
      break;
    case AG_KEY_ESCAPE:
      if(p->dump->writeEnable) {
	 p->dump->writeEnable = FALSE;
      } else {
	 p->dump->writeEnable = TRUE;
      }
      p->dump->editHex = TRUE;
      break;
    case AG_KEY_UP:
      if(((AG_KEYMOD_LSHIFT | AG_KEYMOD_RSHIFT) & mod) != 0){
	 p->dump->addr -= 16;
	 p->dump->addr &= 0x0000ffff;
      } else {
	 p->dump->edaddr -= 16;
	 p->dump->edaddr &= 0x0000ffff;
      }
      break;
    case AG_KEY_DOWN:
      if(((AG_KEYMOD_LSHIFT | AG_KEYMOD_RSHIFT) & mod) != 0){
	 p->dump->addr += 16;
	 p->dump->addr &= 0x0000ffff;
      } else {
	 p->dump->edaddr += 16;
	 p->dump->edaddr &= 0x0000ffff;
      }
      break;
    case AG_KEY_LEFT:
      if(((AG_KEYMOD_LSHIFT | AG_KEYMOD_RSHIFT) & mod) != 0){
	 p->dump->addr -= 1;
	 p->dump->addr &= 0x0000ffff;
      } else {
	 p->dump->edaddr -= 1;
	 p->dump->edaddr &= 0x0000ffff;
      }
      break;
    case AG_KEY_RIGHT:
      if(((AG_KEYMOD_LSHIFT | AG_KEYMOD_RSHIFT) & mod) != 0){
	 p->dump->addr += 1;
	 p->dump->addr &= 0x0000ffff;
      } else {
	 p->dump->edaddr += 1;
	 p->dump->edaddr &= 0x0000ffff;
      }
      
      break;
    case AG_KEY_PAGEUP:
      p->dump->addr -= 256;
      p->dump->addr &= 0x0000ffff;
      p->dump->edaddr -= 256;
      p->dump->edaddr &= 0x0000ffff;
      break;
    case AG_KEY_PAGEDOWN:
      p->dump->addr += 256;
      p->dump->addr &= 0x0000ffff;
      p->dump->edaddr += 256;
      p->dump->edaddr &= 0x0000ffff;
      break;
    default:
      break;
   }
   if(((p->dump->addr + 256) & 0x0000ffff) <= p->dump->edaddr){
      p->dump->addr = ((p->dump->addr & 0xff)  + (p->dump->edaddr & 0xff00)) & 0x0000ffff;
   }else if(p->dump->addr > p->dump->edaddr) { 
      p->dump->addr = ((p->dump->addr & 0xff)  + (p->dump->edaddr & 0xff00)) & 0x0000ffff;
   }
   
}
   
}

void XM7_DbgDumpMem(void *p)
{
    int a;
    int Hb,Wb;
    int i,j;
    int wd,hd;
    char strbuf[256];
    BYTE *buf;
    int addr;
    unsigned int ysum[16];
    unsigned int xsum;
    unsigned int sum;
    XM7_DbgDump *obj;

    obj = (XM7_DbgDump *)p;

    if(obj == NULL) return;
    if(obj->dump == NULL) return;
    if(obj->rb == NULL) return;
    if(obj->draw == NULL) return;
    if(obj->paused == TRUE) return;
    addr = obj->addr;

    Wb = obj->dump->GetWidth();
    Hb = obj->dump->GetHeight();
    wd = 16;
    hd = 16;
    obj->dump->MoveDrawPos(65, 0);
    obj->dump->PutString("H: Help");

    if(obj->editAddr) return;
	
   obj->dump->MoveDrawPos(0, 0);
   if(obj->writeEnable) {
       sprintf(strbuf, "EDIT: %04x =  %02x    ", obj->edaddr, obj->rb(obj->edaddr));
    } else {
       sprintf(strbuf, "      %04x =  %02x    ", obj->edaddr, obj->rb(obj->edaddr));
    }
    obj->dump->PutString(strbuf);

    obj->dump->MoveDrawPos(0, 1);
    if(wd >= (Wb / 5)) wd = Wb / 5;
    if(hd >= (Hb - 4)) hd = Hb - 4;
    buf = obj->buf;


    obj->dump->PutString("ADDR:");
    for(i = 0; i < wd; i++){
        strbuf[0] = '\0';
        sprintf(strbuf, "+%x ", i);
        obj->dump->PutString(strbuf);
    }
    obj->dump->PutString(":SUM");
    
    for(i = 0; i < Wb - 2; i++){
        obj->dump->MoveDrawPos(i, 2);
        obj->dump->PutChar('-');
    }
    sum = 0;
    for(i = 0;i < wd; i++){
        ysum[i] = 0;
    }

    for(i = 0;i < hd; i++){
        xsum = 0;
        for(j = 0; j < wd; j++) {
            buf[j] = obj->rb((WORD)((addr + j) & 0xffff));
            sum += (unsigned int)buf[j];
            xsum += (unsigned int)buf[j];
	    ysum[j] += buf[j];
        }
        strbuf[0] = '\0';
        obj->dump->MoveDrawPos(0, 3 + i);
        DBG_HexDumpMemory(strbuf, buf, (WORD)addr, xsum & 0xff, wd, TRUE);
        obj->dump->PutString(strbuf);
        addr += wd;
    }
    // Footer
    obj->dump->MoveDrawPos(0, 3 + hd);
    for(i = 0; i < Wb - 2; i++){
        obj->dump->MoveDrawPos(i, 3 + hd);
        obj->dump->PutChar('-');
    }
    // Sum
    obj->dump->MoveDrawPos(0, 4 + hd);
    strbuf[0] = '\0';
    DBG_PrintYSum(strbuf, (int *)ysum, (int) (sum & 0xff), wd);
    obj->dump->PutString(strbuf);
    obj->dump->MoveDrawPos(0, 0);

}

struct XM7_DbgDump *XM7_DbgDumpMemInit(void *parent, BYTE (*rf)(WORD), void (*wf)(WORD, BYTE))
{
    struct XM7_DbgDump *obj;
    XM7_SDLView *view;
    DumpObject  *dump;
    BYTE        *buf;
    AG_SizeAlloc a;
    AG_Surface *s;
    int chrw,chrh;

    if(parent == NULL) return NULL;
    obj = (struct XM7_DbgDump *)malloc(sizeof(struct XM7_DbgDump));
    if(obj == NULL) return NULL;
    memset(obj, 0x00, sizeof(struct XM7_DbgDump));
    dump = new DumpObject;
    if(dump == NULL) {
        free(obj);
        return NULL;
    }
    buf = (BYTE *)malloc(sizeof(BYTE) * 16 * 16);
    dump->InitConsole(80, 22); // ステータス表示分
    if(buf == NULL){
        free(obj);
        delete dump;
        return NULL;
    }
    memset(buf, 0x00, sizeof(BYTE) * 16 * 16);
    view = XM7_SDLViewNew(parent, NULL, "");
    obj->addr = 0x0000;
    obj->edaddr = 0x0000;
    obj->dump = dump;
    obj->draw = view;
    obj->buf = buf;
    obj->forceredraw = FALSE;
    obj->rb = rf;
    obj->wb = wf;
   
    s = dump->GetScreen();
    a.w = s->w;
    a.h = s->h;
    a.x = 0;
    a.y = 0;
    AG_WidgetSizeAlloc(AGWIDGET(view), &a);
    XM7_SDLViewDrawFn(view, XM7_DbgDumpDrawFn, "%p,%i", (void *)obj, FALSE); //
    XM7_SDLViewLinkSurface(view, s);
    return obj;
}

void XM7_DbgDumpMemDetach(struct XM7_DbgDump *dbg)
{
    if(dbg == NULL) return;
    if(dbg->draw != NULL) {
        AG_ObjectDetach(dbg->draw);
    }
    if(dbg->buf != NULL) free(dbg->buf);
    if(dbg->dump != NULL) delete dbg->dump;
    free(dbg);
}

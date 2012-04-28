/*
 * Debugger for XM-7/SDL : Register-Dump
 * (C) 2012 K.Ohta <whatisthis.sowhar@gmail.com>
 * History:
 *        24 Apr 2012 : Branch from agar_debugger.cpp
 */

#include "agar_surfaceconsole.h"
#include "agar_debugger.h"
#include "xm7.h"

static BOOL XM7_DbgRegCopyReg(struct XM7_DbgRegDump *p)
{
   if(p == NULL) return FALSE;
   if(p->reg == NULL) return FALSE;
   // LockVMする?
   memcpy(&(p->buf), p->reg, sizeof(cpu6809_t));
   // UnlockVMする?
   return TRUE;
}


static void XM7_DbgRegDumpDrawFn(AG_Event *event)
{
   XM7_SDLView *view = (XM7_SDLView *)AG_SELF();
    XM7_DbgRegDump *p = (XM7_DbgRegDump *)AG_PTR(1);
    BOOL forceredraw = AG_INT(2);

    if(p == NULL) return;
    p->cons->Draw(forceredraw);
    AG_WidgetUpdateSurface(AGWIDGET(view), view->mySurface);

}

void XM7_DbgDumpRegs(XM7_DbgRegDump *dbg)
{
   char strbuf[16];
   DumpObject *cons;

   if(XM7_DbgRegCopyReg(dbg) == FALSE) return;
   if(dbg->cons == NULL) return;
   cons = dbg->cons;

   cons->MoveDrawPos(0,0);
   cons->PutString(dbg->title);
//   cons->MoveDrawPos(20,0);
   // PC
   word2chr(dbg->buf.pc, strbuf);
   cons->MoveDrawPos(0,1);
   cons->PutString("PC:");
   cons->MoveDrawPos(5,1);
   cons->PutString(strbuf);
   // EA
   word2chr(dbg->buf.ea, strbuf);
   cons->MoveDrawPos(12,1);
   cons->PutString("EA:");
   cons->MoveDrawPos(17,1);
   cons->PutString(strbuf);
   // S
   word2chr(dbg->buf.s, strbuf);
   cons->MoveDrawPos(0,2);
   cons->PutString("S:");
   cons->MoveDrawPos(5,2);
   cons->PutString(strbuf);
   // U
   word2chr(dbg->buf.u, strbuf);
   cons->MoveDrawPos(12,2);
   cons->PutString("U:");
   cons->MoveDrawPos(17,2);
   cons->PutString(strbuf);
   // X
   word2chr(dbg->buf.x, strbuf);
   cons->MoveDrawPos(0,3);
   cons->PutString("X:");
   cons->MoveDrawPos(5,3);
   cons->PutString(strbuf);
   // Y
   word2chr(dbg->buf.y, strbuf);
   cons->MoveDrawPos(12,3);
   cons->PutString("Y:");
   cons->MoveDrawPos(17,3);
   cons->PutString(strbuf);
   // AB
   cons->MoveDrawPos(0,4);
   cons->PutString("AB:");
   cons->MoveDrawPos(5,4);
   byte2chr(dbg->buf.acc.h.a, strbuf);
   cons->PutString(strbuf);
   cons->MoveDrawPos(7,4);
   byte2chr(dbg->buf.acc.h.b, strbuf);
   cons->PutString(strbuf);
   // DP
   byte2chr(dbg->buf.ea, strbuf);
   cons->MoveDrawPos(12,4);
   cons->PutString("DP:");
   cons->MoveDrawPos(17,4);
   cons->PutString(strbuf);
   // CC
   {// 7 EFHINZVC 0
      BYTE cc = dbg->buf.cc;
      if(cc & 0x80) {
	   strbuf[0] = 'E';
      } else {
	   strbuf[0] = '-';
      }
      if(cc & 0x40) {
	   strbuf[1] = 'F';
      } else {
	   strbuf[1] = '-';
      }
     if(cc & 0x20) {
	   strbuf[2] = 'H';
      } else {
	   strbuf[2] = '-';
      }

     if(cc & 0x10) {
	   strbuf[3] = 'I';
      } else {
	   strbuf[3] = '-';
      }
      if(cc & 0x08) {
	   strbuf[4] = 'N';
      } else {
	   strbuf[4] = '-';
      }
      if(cc & 0x04) {
	   strbuf[5] = 'Z';
      } else {
	   strbuf[5] = '-';
      }
      if(cc & 0x02) {
	   strbuf[6] = 'V';
      } else {
	   strbuf[6] = '-';
      }
      if(cc & 0x01) {
	   strbuf[7] = 'C';
      } else {
	   strbuf[7] = '-';
      }
   }

      strbuf[8] = '\0';
      cons->MoveDrawPos(0,5);
      cons->PutString("CC:");
      cons->MoveDrawPos(5,5);
      cons->PutString(strbuf);
      // Interrupt
      cons->MoveDrawPos(0,6);
      cons->PutString("INTR:");
      cons->MoveDrawPos(5,6);
      {
	 WORD intr = dbg->buf.intr;
	 if(intr & INTR_NMI) {
	      strbuf[0] = 'N';
	 } else {
	      strbuf[0] = ' ';
	 }
	 if(intr & INTR_FIRQ) {
	      strbuf[1] = 'F';
	 } else {
	      strbuf[1] = ' ';
	 }
	 if(intr & INTR_IRQ) {
	      strbuf[2] = 'I';
	 } else {
	      strbuf[2] = ' ';
	 }
	 strbuf[3] = ' ';
 	 if(intr & INTR_HALT) {
	      strbuf[4] = 'H';
	 } else {
	      strbuf[4] = ' ';
	 }
	 strbuf[5] = '\0';
      }

      cons->PutString(strbuf);
      cons->MoveDrawPos(0, 0);
}


XM7_DbgRegDump *XM7_DbgRegDumpInit(void *parent, cpu6809_t *cpu, char *title)
{
    struct XM7_DbgRegDump *obj;
    XM7_SDLView *view;
    DumpObject  *cons;
    AG_SizeAlloc a;
    AG_Surface *s;
    int chrw,chrh;

    if(parent == NULL) return NULL;
    if(cpu == NULL) return NULL;
    obj = (struct XM7_DbgRegDump *)malloc(sizeof(struct XM7_DbgRegDump));
    if(obj == NULL) return NULL;
    memset(obj, 0x00, sizeof(struct XM7_DbgRegDump));
    cons = new DumpObject;
    if(cons == NULL) {
        free(obj);
        return NULL;
    }
    memset(&(obj->buf), 0x00, sizeof(cpu6809_t));
    cons->InitConsole(26, 7); // ステータス表示分
    view = XM7_SDLViewNew(parent, NULL, "");
    obj->cons = cons;
    obj->draw = view;
    obj->reg = cpu;
    obj->forceredraw = FALSE;
    memcpy(&(obj->buf), cpu, sizeof(cpu6809_t));
    if(title != NULL) strncpy(obj->title, title, 12 - 1);
    s = cons->GetScreen();
    a.w = s->w;
    a.h = s->h;
    a.x = 0;
    a.y = 0;
    AG_WidgetSizeAlloc(AGWIDGET(view), &a);
    XM7_SDLViewDrawFn(view, XM7_DbgRegDumpDrawFn, "%p,%i", (void *)obj, FALSE); //
    XM7_SDLViewLinkSurface(view, s);
    return obj;
}

void XM7_DbgRegDumpDetach(XM7_DbgRegDump *dbg)
{
    if(dbg == NULL) return;
    if(dbg->draw != NULL) {
        AG_ObjectDetach(dbg->draw);
    }
    if(dbg->cons != NULL) delete dbg->cons;
    free(dbg);
}




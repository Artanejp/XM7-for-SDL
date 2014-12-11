/*
 * Debugger for XM-7/SDL : Register-Dump
 * (C) 2012 K.Ohta <whatisthis.sowhar@gmail.com>
 * History:
 *        24 Apr 2012 : Branch from agar_debugger.cpp
 */

#include "agar_surfaceconsole.h"
#include "agar_debugger.h"
#include "xm7.h"

static BOOL XM7_DbgFdcCopyReg(struct XM7_DbgFdcDump *p)
{
   BYTE drv = fdc_drvreg;
   if(p == NULL) return FALSE;
   // LockVMする?
   p->fdc_command = fdc_command;    /* FDCコマンド */
   p->fdc_status = fdc_status;     /* FDCステータス */
   p->fdc_trkreg[drv] = fdc_trkreg;     /* トラックレジスタ */
   p->fdc_secreg[drv] = fdc_secreg;     /* セクタレジスタ */
   p->fdc_sidereg[drv] = fdc_sidereg;    /* サイドレジスタ */
   p->fdc_drvreg = drv;     /* 論理ドライブ */
   // UnlockVMする?
   return TRUE;
}


static void XM7_DbgFdcDumpDrawFn(AG_Event *event)
{
   XM7_SDLView *view = (XM7_SDLView *)AG_SELF();
    XM7_DbgRegDump *p = (XM7_DbgRegDump *)AG_PTR(1);
    BOOL forceredraw = AG_INT(2);

    if(p == NULL) return;
    XM7_ConsoleUpdate(view, p->cons, forceredraw);
}

void XM7_DbgDumpFdc(XM7_DbgFdcDump *dbg)
{
   char strbuf[64];
   int drv;
   DumpObject *cons;

   if(XM7_DbgFdcCopyReg(dbg) == FALSE) return;
   if(dbg->cons == NULL) return;
   cons = dbg->cons;

   cons->MoveDrawPos(0,0);
   cons->PutString("FDC Dump");
   cons->MoveDrawPos(0,1);
   cons->PutString("--------------------");
   cons->MoveDrawPos(0,2);
   sprintf(strbuf, "Now Drive:%02d    CMD:%02x STAT:%02x", dbg->fdc_drvreg, dbg->fdc_command, dbg->fdc_status); 
   cons->PutString(strbuf);
   for(drv = 0; drv < 4; drv++) {
      cons->MoveDrawPos(0,3 + drv);
      if(dbg->fdc_drvreg == drv) {
	   cons->PutString(">");
      } else {
	   cons->PutString(" ");
      }
      
      cons->MoveDrawPos(1,3 + drv);
      sprintf(strbuf, "Drive:%1d Trk %02d Side %1d Sec $%02x", drv, dbg->fdc_trkreg[drv], dbg->fdc_sidereg[drv], dbg->fdc_secreg[drv] ); 
      cons->PutString(strbuf);
   }
   
}


XM7_DbgFdcDump *XM7_DbgFdcDumpInit(void *parent)
{
    struct XM7_DbgFdcDump *obj;
    XM7_SDLView *view;
    DumpObject  *cons;
    AG_SizeAlloc a;
    AG_Surface *s;
    int chrw,chrh;

    if(parent == NULL) return NULL;
    obj = (struct XM7_DbgFdcDump *)malloc(sizeof(struct XM7_DbgFdcDump));
    if(obj == NULL) return NULL;
    memset(obj, 0x00, sizeof(struct XM7_DbgFdcDump));
    cons = new DumpObject;
    if(cons == NULL) {
        free(obj);
        return NULL;
    }
    view = XM7_SDLViewNew(parent, NULL, "");
    obj->cons = cons;
    obj->draw = view;
    obj->forceredraw = FALSE;
    XM7_ConsoleSetup(view, cons, obj, XM7_DbgFdcDumpDrawFn, 40, 8, FALSE);
    return obj;
}

void XM7_DbgFdcDumpDetach(XM7_DbgFdcDump *dbg)
{
    if(dbg == NULL) return;
    if(dbg->draw != NULL) {
        AG_ObjectDetach(dbg->draw);
    }
    if(dbg->cons != NULL) delete dbg->cons;
    free(dbg);
}




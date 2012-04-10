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

}

void XM7_DbgDumpMem(void *p, int addr)
{
    int a;
    int Hb,Wb;
    int i,j;
    int wd,hd;
    char strbuf[256];
    BYTE *buf;
    unsigned int ysum[16];
    unsigned int xsum;
    unsigned int sum;
    XM7_DbgDump *obj;

    obj = (XM7_DbgDump *)p;

    if(obj == NULL) return;
    if(obj->dump == NULL) return;
    if(obj->rb == NULL) return;
    if(obj->draw == NULL) return;

    Wb = obj->dump->GetWidth();
    Hb = obj->dump->GetHeight();
    wd = 16;
    hd = 16;
    obj->dump->MoveDrawPos(0, 0);
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
        obj->dump->MoveDrawPos(i, 1);
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
        obj->dump->MoveDrawPos(0, 2 + i);
        DBG_HexDumpMemory(strbuf, buf, (WORD)addr, xsum & 0xff, wd, TRUE);
        obj->dump->PutString(strbuf);
        addr += wd;
    }
    // Footer
    obj->dump->MoveDrawPos(0, 2 + hd);
    for(i = 0; i < Wb - 2; i++){
        obj->dump->MoveDrawPos(i, 2 + hd);
        obj->dump->PutChar('-');
    }
    // Sum
    obj->dump->MoveDrawPos(0, 3 + hd);
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
    dump->InitConsole(80, 20);
    if(buf == NULL){
        free(obj);
        delete dump;
        return NULL;
    }
    memset(buf, 0x00, sizeof(BYTE) * 16 * 16);
    view = XM7_SDLViewNew(parent, NULL, "");
    obj->addr = 0x0000;
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

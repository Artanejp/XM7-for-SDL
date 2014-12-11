/*
* Debugger for XM7
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* History:
* 29 Mar,2012 Copy From agar_dbg_memdump.cpp
*/

#include "agar_surfaceconsole.h"
#include "agar_debugger.h"
#include "mmr.h"


/*
* WIDGET
*/
extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 b);
extern void DBG_Bin2Hex4(char *str, Uint32 b);

/*
 * 
 */

static void XM7_DbgMMRDrawFn(AG_Event *event)
{
   XM7_SDLView *view = (XM7_SDLView *)AG_SELF();
    struct XM7_DbgMMRDump *p = (struct XM7_DbgMMRDump *)AG_PTR(1);
    BOOL forceredraw = AG_INT(2);

    if(p == NULL) return;
    XM7_ConsoleUpdate(view, p->cons, forceredraw);
}

   

void XM7_DbgDumpMMR(void *p)
{
    int a;
    int i,j;
    int wd,hd;
    char strbuf[256];
    BYTE *buf;
    int addr;
    unsigned int ysum[16];
    unsigned int xsum;
    unsigned int sum;
    XM7_DbgMMRDump *obj;

    obj = (XM7_DbgMMRDump *)p;

    if(obj == NULL) return;
    if(obj->cons == NULL) return;
    if(obj->draw == NULL) return;
    if(obj->paused == TRUE) return;

    wd = 16;
#if XM7_VER >= 3
   hd = 8;
   memcpy(obj->mmr_reg, mmr_reg, 80);
#else
   hd = 4;
   memcpy(obj->mmr_reg, mmr_reg, 40);
#endif
    obj->cons->MoveDrawPos(0, 0);
    sprintf(strbuf, "MMR: ");
    obj->cons->PutString(strbuf);

    obj->cons->MoveDrawPos(0, 0);
    obj->cons->PutString("SEG: $");
#if XM7_VER >= 3   
    if(mmr_ext) {
         DBG_Bin2Hex1(strbuf, mmr_seg);
    } else {
	DBG_Bin2Hex1(strbuf, mmr_seg & 0x03);
    }
#else
   DBG_Bin2Hex1(strbuf, mmr_seg & 0x03);
#endif
   obj->cons->PutString(strbuf);
    obj->cons->MoveDrawPos(4, 1);

   for(i = 0; i < wd; i++){
        strbuf[0] = '\0';
        sprintf(strbuf, "+%x ", i);
        obj->cons->PutString(strbuf);
    }
    
    for(i = 0; i < (wd * 3) + 2; i++){
        obj->cons->MoveDrawPos(i, 2);
        obj->cons->PutChar('-');
    }
    for(i = 0; i < hd; i++){
       obj->cons->MoveDrawPos(0 , 3 + i);
       if(mmr_ext || (i <= 3)) {
#if XM7_VER >= 3
	  if((mmr_seg == i) && (mmr_ext)) {
	       obj->cons->PutChar('>');
	  } else if(((mmr_seg & 3) == i) && (!mmr_ext)) {
	       obj->cons->PutChar('>');
	  } else {
	       obj->cons->PutChar(' ');
	  }
#else
          if((mmr_seg & 3) == i) {
	       obj->cons->PutChar('>');
	  } else {
	       obj->cons->PutChar(' ');
	  }
#endif  
	  DBG_Bin2Hex1(strbuf, i  + 0xb0);
	  obj->cons->MoveDrawPos(1 , 3 + i);
	  obj->cons->PutString(strbuf);
	  obj->cons->MoveDrawPos(3 , 3 + i);
	  obj->cons->PutChar(' ');
	  for(j = 0; j < wd; j++) {
	     DBG_Bin2Hex1(strbuf, obj->mmr_reg[i * 16 + j]);
	     obj->cons->MoveDrawPos(j * 3 + 4, 3 + i);
	     obj->cons->PutString(strbuf);
	  }
       }
    }
   obj->cons->MoveDrawPos(0 , 0);

}

XM7_DbgMMRDump *XM7_DbgDumpMMRInit(void *parent)
{
    struct XM7_DbgMMRDump *obj;
    XM7_SDLView *view;
    DumpObject  *dump;
    BYTE        *buf;
    AG_SizeAlloc a;
    AG_Surface *s;
    int chrw,chrh;

    if(parent == NULL) return NULL;
    obj = (XM7_DbgMMRDump *)malloc(sizeof(XM7_DbgMMRDump));
    if(obj == NULL) return NULL;
    memset(obj, 0x00, sizeof(XM7_DbgMMRDump));
    dump = new DumpObject;
    if(dump == NULL) {
        free(obj);
        return NULL;
    }
    view = XM7_SDLViewNew(parent, NULL, "");
    obj->cons = dump;
    obj->draw = view;
    obj->forceredraw = FALSE;
#if XM7_VER >= 3
    XM7_ConsoleSetup(view, dump, obj, XM7_DbgMMRDrawFn, 64, 11, FALSE);
#else
    XM7_ConsoleSetup(view, dump, obj, XM7_DbgMMRDrawFn, 64, 7, FALSE);
#endif
    return obj;
}

void XM7_DbgDumpMMRDetach(struct XM7_DbgMMRDump *dbg)
{
    if(dbg == NULL) return;
    if(dbg->draw != NULL) {
        AG_ObjectDetach(dbg->draw);
    }
    if(dbg->cons != NULL) delete dbg->cons;
    free(dbg);
}



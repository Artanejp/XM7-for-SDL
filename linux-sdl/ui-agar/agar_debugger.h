#ifndef AGAR_DEBUGGER_H_INCLUDED
#define AGAR_DEBUGGER_H_INCLUDED

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include "xm7.h"
#include "agar_surfaceconsole.h"
#include "agar_sdlview.h"

extern void DumpMem2(DumpObject *obj, int addr);

enum {
    MEM_MAIN = 0,
    MEM_SUB,
    MEM_JSUB
};

/* Structure describing an instance of the XM7_SDLView. */
typedef struct  XM7_DbgDump {
    BYTE (*rb)(WORD);       // Read Access
    void (*wb)(WORD, BYTE);       // Write Access
    unsigned int addr;
    unsigned int edaddr;
    DumpObject *dump;   // Internal Dump Object
    int forceredraw;
    XM7_SDLView *draw;
    BOOL writeEnable;
    BOOL paused;
    BOOL editHex;
    BOOL editAddr;
    unsigned int baddr;
    unsigned int bdata;
    BYTE *buf;
};

struct XM7_MemDumpDesc {
    XM7_DbgDump *dump;
    AG_Timeout to;
    Uint32 to_tick;
};



extern void XM7_DbgDumpMem(void *p);
extern void XM7_DbgDumpMemDetach(struct XM7_DbgDump *dbg);
extern struct XM7_DbgDump *XM7_DbgDumpMemInit(void *parent, BYTE (*rf)(WORD), void (*wf)(WORD, BYTE));
extern "C" {
   extern void XM7_DbgKeyPressFn(AG_Event *event);
}

static void readmem(struct XM7_MemDumpDesc *p)
{
    BYTE *buf;
    int x,y;
    int ofset;

    if(p == NULL) return;
    if(p->dump == NULL) return;
    if(p->dump->buf == NULL) return;
    ofset = 0;
    buf = p->dump->buf;
    for(x = 0; x < 16 ; x++){
        for(y = 0; y < 16; y++){
            buf[ofset] = p->dump->rb(p->dump->addr + ofset);
            ofset++;
        }
    }
}


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



#endif // AGAR_DEBUGGER_H_INCLUDED

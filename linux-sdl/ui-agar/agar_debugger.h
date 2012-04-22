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

/* flag bits in the cc register */
#define CC_C	0x01        /* Carry */
#define CC_V	0x02        /* Overflow */
#define CC_Z    0x04        /* Zero */
#define CC_N    0x08        /* Negative */
#define CC_II   0x10        /* Inhibit IRQ */
#define CC_H    0x20        /* Half (auxiliary) carry */
#define CC_IF   0x40        /* Inhibit FIRQ */
#define CC_E    0x80        /* entire state pushed */

extern "C" 
{
   extern cpu6809_t       maincpu;
   extern cpu6809_t       subcpu;
#if (XM7_VER == 1 && defined(JSUB))
   extern cpu6809_t       jsubcpu;
#endif // JSUB && XM7_VER==1
}

   

typedef struct  XM7_DbgDisasm {
//    BYTE (*rb)(WORD);       // Read Access
//    void (*wb)(WORD, BYTE);       // Write Access
    unsigned int addr;
    unsigned int nextaddr;
    unsigned int beforeaddr;
    int cputype;
    DumpObject *cons;   // Internal Dump Object
    int forceredraw;
    XM7_SDLView *draw;
    unsigned int baddr;
    unsigned int bdata;
    BOOL editAddr;
    BOOL paused;
    BYTE *buf;
};



struct XM7_DbgDisasmDesc {
    XM7_DbgDisasm *disasm;
    AG_Timeout to;
    Uint32 to_tick;
};

typedef struct  XM7_DbgRegdump {
    DumpObject *dump;   // Internal Dump Object
    int forceredraw;
    XM7_SDLView *draw;
    cpu6809_t mainRegs;
    cpu6809_t subRegs;
#if (XM7_VER == 1 && defined(JSUB))
    cpu6809_t jsubRegs;   
#endif // JSUB && XM7_VER==1

    BOOL paused;
    BYTE *buf;
};

struct XM7_DbgRegDumpDesc {
    XM7_DbgDisasm *dump;
    AG_Timeout to;
    Uint32 to_tick;
};




extern void XM7_DbgDumpMem(void *p);
extern void XM7_DbgDumpMemDetach(struct XM7_DbgDump *dbg);
extern struct XM7_DbgDump *XM7_DbgDumpMemInit(void *parent, BYTE (*rf)(WORD), void (*wf)(WORD, BYTE));
extern "C" {
   extern void XM7_DbgKeyPressFn(AG_Event *event);


}
void XM7_DbgMemDisasm(void *p);
void XM7_DbgDisasmKeyPressFn(AG_Event *event);
void XM7_DbgDisasmDetach(struct XM7_DbgDisasm *dbg);
struct XM7_DbgDisasm *XM7_DbgDisasmInit(void *parent, BYTE (*rf)(WORD), void (*wf)(WORD, BYTE));


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

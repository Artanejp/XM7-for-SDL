#ifndef AGAR_DEBUGGER_H_INCLUDED
#define AGAR_DEBUGGER_H_INCLUDED

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include "xm7.h"
#include "agar_surfaceconsole.h"
#include "agar_sdlview.h"

extern void DumpMem2(DumpObject *obj, int addr);

/* Structure describing an instance of the XM7_SDLView. */
typedef struct  XM7_DbgDump {
    BYTE (*rb)(WORD);       // Read Access
    void (*wb)(WORD, BYTE);       // Write Access
    unsigned int addr;
	DumpObject *dump;   // Internal Dump Object
	int forceredraw;
	XM7_SDLView *draw;
	BYTE *buf;
};

extern void XM7_DbgDumpMem(void *p, int addr);
extern void XM7_DbgDumpMemDetach(struct XM7_DbgDump *dbg);
extern struct XM7_DbgDump *XM7_DbgDumpMemInit(void *parent, BYTE (*rf)(WORD), void (*wf)(WORD, BYTE));



#endif // AGAR_DEBUGGER_H_INCLUDED

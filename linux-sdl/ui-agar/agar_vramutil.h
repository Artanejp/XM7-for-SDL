#ifndef AGAR_VRAMUTIL_H_INCLUDED
#define AGAR_VRAMUTIL_H_INCLUDED

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
//#include <agar/gui/opengl.h>

#include <SDL.h>
#include "api_draw.h"
#include "api_scaler.h"
#include "api_vram.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"

struct VirtualVram {
    Uint32 pVram[640][400];
};
extern struct VirtualVram *pVirtualVram;
extern Uint32 *pVram2;
extern BOOL InitVideo;

// Functions
extern void InitVirtualVram();

extern "C" {
extern void LockVram(void);
extern void UnlockVram(void);
extern void InitVramSemaphore(void);
extern void DetachVramSemaphore(void);
}
// External Memories
extern Uint8 *vram_pb;
extern Uint8 *vram_pr;
extern Uint8 *vram_pg;

#endif // AGAR_VRAMUTIL_H_INCLUDED

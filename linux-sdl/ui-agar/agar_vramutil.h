#ifndef AGAR_VRAMUTIL_H_INCLUDED
#define AGAR_VRAMUTIL_H_INCLUDED

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
//#include <agar/gui/opengl.h>

#include <SDL/SDL.h>
#include "api_draw.h"
//#include "api_scaler.h"
#include "api_vram.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"

//struct VirtualVram {
//    Uint32 pVram[640][400];
//};
//extern struct VirtualVram *pVirtualVram;
extern Uint32 *pVram2;
extern BOOL InitVideo;
extern BOOL bVramUpdateFlag;

// Functions
extern void InitVirtualVram();

#ifdef __cplusplus
extern "C" {
#endif
extern void LockVram(void);
extern void UnlockVram(void);
extern void InitVramSemaphore(void);
extern void DetachVramSemaphore(void);
extern BOOL CheckVramSemaphore(void);
extern Uint8 *vram_pb;
extern Uint8 *vram_pr;
extern Uint8 *vram_pg;
extern void CreateVirtualVram8_Line(Uint32 *p, int ybegin, int yend, int mode);
extern void CreateVirtualVram8_WindowedLine(Uint32 *p, int ybegin, int yend, int xbegin, int xend, int mode);

#ifdef __cplusplus
}
#endif
// External Memories
#ifdef __cplusplus

extern void PutVram_AG_SP(AG_Surface *p, int x, int y, int w, int h,  Uint32 mpage);
extern void SetVramReader_GL2(void p(Uint32, Uint32 *, Uint32), int w, int h);
extern void BuildVirtualVram_Raster(Uint32 *pp, int y, int mode);
extern void BuildVirtualVram_RasterWindow(Uint32 *pp, int xbegin, int xend, int y, int mode);

extern Uint32 *GetVirtualVram(void);
#endif

#endif // AGAR_VRAMUTIL_H_INCLUDED

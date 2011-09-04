/*
 * Utilities for Virtualvram
 */
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"

#include "agar_vramutil.h"

static SDL_sem *pVideoSem;
Uint32 *pVram2;
BOOL InitVideo;
struct VirtualVram *pVirtualVram;
BYTE bModeOld;
BOOL bVramUpdateFlag;

extern "C" {

void LockVram(void)
{
    if(pVideoSem == NULL) return;
    SDL_SemWait(pVideoSem);
}

void UnlockVram(void)
{
    if(pVideoSem == NULL) return;
    SDL_SemPost(pVideoSem);
}

BOOL CheckVramSemaphore(void)
{
    if(pVideoSem == NULL) return FALSE;
    return TRUE;
}

void InitVramSemaphore(void)
{
    if(pVideoSem != NULL) return;
	pVideoSem = SDL_CreateSemaphore(1);
	if(pVideoSem) SDL_SemPost(pVideoSem);
}

void DetachVramSemaphore(void)
{
    if(pVideoSem != NULL) {
        SDL_DestroySemaphore(pVideoSem);
        pVideoSem = NULL;
    }
}

}

void InitVirtualVram()
{
    if(pVirtualVram != NULL) return;
    pVirtualVram = (struct VirtualVram *)malloc(sizeof(struct VirtualVram));
    if(pVirtualVram == NULL) return;
    pVram2 = (Uint32 *)malloc(640*400*sizeof(Uint32));
    if(pVram2 == NULL) {
        free(pVirtualVram);
        pVirtualVram = NULL;
    }
    bVramUpdateFlag= FALSE;
   // Phase 1
    memset(pVirtualVram, 0x00, sizeof(struct VirtualVram) );
    memset(pVram2, 0x00, sizeof(640*400*sizeof(Uint32)));
    // Phase 2
    bModeOld = bMode;
}

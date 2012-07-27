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
   int size;
    if(pVirtualVram != NULL) return;
#ifndef _WINDOWS
    if(posix_memalign((void **)&pVirtualVram, 32, sizeof(struct VirtualVram)) < 0) return;
   if(posix_memalign((void **)&pVram2, 32, sizeof(Uint32) * 640 * 400) < 0) {
      free(pVirtualVram);
      pVirtualVram = NULL;
   }
   
#else
    size = ((sizeof(struct VirtualVram) + 31 ) / 32) * 32;
    pVirtualVram = (struct VirtualVram *)malloc(size);
    if(pVirtualVram == NULL) return;
    size = ((640 * 400 * sizeof(Uint32) + 31 ) / 32) * 32;
    pVram2 = (Uint32 *)malloc(size);
    if(pVram2 == NULL) {
        free(pVirtualVram);
        pVirtualVram = NULL;
    }
#endif   
    bVramUpdateFlag= FALSE;
   // Phase 1
    memset(pVirtualVram, 0x00, sizeof(struct VirtualVram) );
    memset(pVram2, 0x00, sizeof(640*400*sizeof(Uint32)));
    // Phase 2
    bModeOld = bMode;
}

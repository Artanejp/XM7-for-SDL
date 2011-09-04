/*
 * XM7 : 断片化VRAMでの書き込み
 * 2011 (C) K.Ohta <whatithis.sowhat@gmail.com>
 */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
//#include <agar/gui/opengl.h>

#include <SDL.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include "api_draw.h"
#include "api_scaler.h"
#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"

// void CreateVirtualVram256k_1Pcs(Uint32 *p, int x, int y, int pitch, int mpage)

BOOL CheckDrawMode(void)
{
    BOOL t;
    int x;
    int y;

    if((SDLDrawFlag.APaletteChanged
               | SDLDrawFlag.DPaletteChanged
               | SDLDrawFlag.ForcaReDraw
               | bClearFlag) == TRUE){
        t = TRUE;
    } else {
        t = FALSE;
    }

    if(t){
        LockVram();
        for(y = 0; y < 50; y++){
            for(x = 0; x < 80 ; x++){
                SDLDrawFlag.write[x][y] = TRUE;
            }
        }
        UnlockVram();
    }
    return t;
}

static void BuildVirtualVram8(Uint32 *pp, int x, int y, int  w, int h, int mode)
{
    int xx;
    int yy;
    int ww;
    int hh;
    Uint32 *p;

    if(pp == NULL) return;
	ww = (w + x) >> 3;
	hh = (h + y) >> 3;

    LockVram();
    for(yy = (y >> 3); yy < hh ; yy++) {
        for(xx = (x >> 3); xx < ww ; xx++) {
            if(SDLDrawFlag.write[xx][yy]) {
               	p = &pp[(xx + (80 * yy <<3)) << 3];
                CreateVirtualVram8_1Pcs(p, xx, yy << 3, (640 * sizeof(Uint32)), mode);
                SDLDrawFlag.write[xx][yy] = FALSE;
            }
        }
    }
    bVramUpdateFlag = TRUE;
    UnlockVram();
}

static void BuildVirtualVram4096(Uint32 *pp, int x, int y ,int  w, int h, int mode)
{
    int xx;
    int yy;
    int ww;
    int hh;
    Uint32 *p;

    if(pp == NULL) return;
	ww = (w + x) >> 3;
	hh = (h + y) >> 3;

    LockVram();
    for(yy = (y >> 3); yy < hh ; yy++) {
        for(xx = (x >> 3); xx < ww ; xx++) {
            if(SDLDrawFlag.write[xx][yy]) {
                p = &pp[(xx  + (320 * yy))<<3];
                CreateVirtualVram4096_1Pcs(p, xx, yy << 3, (320 * sizeof(Uint32)), mode);
                SDLDrawFlag.write[xx][yy] = FALSE;
            }
        }
    }
    bVramUpdateFlag = TRUE;
    UnlockVram();
}

static void BuildVirtualVram256k(Uint32 *pp, int x, int y, int  w, int h, int mpage)
{
    int xx;
    int yy;
    int ww;
    int hh;
    Uint32 *p;

    if(pp == NULL) return;
	ww = (w + x) >> 3;
	hh = (h + y) >> 3;

    LockVram();
    for(yy = (y >> 3); yy < hh ; yy++) {
        for(xx = (x >> 3); xx < ww ; xx++) {
            if(SDLDrawFlag.write[xx][yy]) {
                p = &pp[(xx + (320 * yy)) << 3];
                CreateVirtualVram256k_1Pcs(p, xx, yy << 3, (320 * sizeof(Uint32)), mpage);
                SDLDrawFlag.write[xx][yy] = FALSE;
            }
        }
    }
    bVramUpdateFlag = TRUE;
    UnlockVram();
}

 void PutVram_AG_SP(SDL_Surface *p, int x, int y, int w, int h,  Uint32 mpage)
{
	int xx, yy;
	int hh, ww;
	int addr;
	int ofset;
	int size;
	Uint32 c[8];
	Uint32 *pp;

	// Test

    if(pVirtualVram == NULL) return;
    pp = &(pVirtualVram->pVram[0][0]);

    if(pp == NULL) return;
	if((vram_pb == NULL) || (vram_pg == NULL) || (vram_pr == NULL)) return;

    LockVram();

    if((bClearFlag)) {
        memset(pp, 0x00, 640 * 400 * sizeof(Uint32)); // モードが変更されてるので仮想VRAMクリア
        SetDrawFlag(TRUE);
        bClearFlag = FALSE;
    }
	switch (bMode) {
	case SCR_400LINE:
        BuildVirtualVram8(pp, x, y, w, h, bMode);
		break;
	case SCR_262144:
        BuildVirtualVram256k(pp, x, y, w, h, mpage);
		break;
	case SCR_4096:
        BuildVirtualVram4096(pp, x, y, w, h, mpage);
		break;
	case SCR_200LINE:
        BuildVirtualVram8(pp, x, y, w, h, bMode);
		break;
	}
	UnlockVram();
}

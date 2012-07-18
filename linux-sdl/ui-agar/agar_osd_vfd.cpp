/*
 * agar_osd_vfd.cpp
 *
 *  Created on: 2012/06/28
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 *     History: June,28,2012 Split from agar_osd.cpp
 */

#include <SDL/SDL.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <iconv.h>

#include "xm7.h"
#include "keyboard.h"
#include "tapelp.h"
#include "display.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "fdc.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_gldraw.h"
#else
#include "xm7_sdl.h"
#endif

#include "agar_osd.h"
#include "sdl_sch.h"
#include "api_draw.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_sdlview.h"


enum {
    OSD_VFD_EMPTY = 0,
    OSD_VFD_NORM,
    OSD_VFD_READ,
    OSD_VFD_WRITE
};

struct OsdVFDPack {
    int drive;
    char VFDLetter[64];
    AG_Surface *pRead;
    AG_Surface *pWrite;
    AG_Surface *pNorm;
    AG_Surface *pEmpty;
    int OldStat;
    int stat;
    BOOL init;
    BOOL Changed;
    AG_Mutex mutex;
};

static char    szDrive[2][16 + 1];	/* フロッピードライブ */
static char    szOldDrive[2][16+1];       /* フロッピードライブ(過去) */
static BOOL     old_writep[4];  /* 過去のライトプロテクトフラグ */
static struct OsdVFDPack *pVFDStat[2];
int nDrive[4];

static XM7_SDLView *pwVFD[2];
static int nVFDWidth;
static int nVFDHeight;

extern int getnFontSize(void);
extern void SetPixelFormat(AG_PixelFormat *fmt);
extern AG_Font *pStatusFont;

static BOOL UpdateVFD(AG_Surface *dst, struct OsdVFDPack *pStatus);

extern "C" {
static void DrawVFDFn(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   struct OsdVFDPack *disp = (struct OsdVFDPack *)AG_PTR(1);
   AG_Surface *dst;

   if((disp == NULL)  || (my == NULL)) return;
   dst = XM7_SDLViewGetSrcSurface(my);
   if(dst == NULL) return;
   AG_MutexLock(&(disp->mutex));
   if((disp->stat == disp->OldStat)
       && (disp->init == FALSE) && (disp->Changed == FALSE)) {
    disp->init = FALSE;
    disp->OldStat = disp->stat;
    disp->Changed = FALSE;
    AG_MutexUnlock(&(disp->mutex));
    return;
   }
   if(UpdateVFD(dst, disp)) {
//   AG_SurfaceCopy(dst, pDrawCMT);
      AG_WidgetUpdateSurface(AGWIDGET(my), my->mySurface);
   }
   
   disp->init = FALSE;
   disp->OldStat = disp->stat;
   disp->Changed = FALSE;
   AG_MutexUnlock(&(disp->mutex));
}

}



static BOOL UpdateVFD(AG_Surface *dst, struct OsdVFDPack *pStatus)
{
   AG_Rect rect;
   int i;
   int x;
   int y;
   int w;
   int h;
   int size;
   char string[64];
   AG_Color r, b, n, black;

   if((dst == NULL) || (pStatusFont == NULL)
    || (pStatus == NULL)) return FALSE;
   r.r = 255;
   r.g = 0;
   r.b = 0;
   r.a = 255;

   black.r = 0;
   black.g = 0;
   black.b = 0;
   black.a = 255;

   b.r = 0;
   b.g = 0;
   b.b = 255;
   b.a = 255;

   n.r = 255;
   n.g = 255;
   n.b = 255;
   n.a = 255;
//   AG_SurfaceLock(dst);
   if((pStatus->Changed == TRUE) || (pStatus->stat != pStatus->OldStat)) 
    {
        AG_Color fg, bg;
        AG_Surface *tmp;
        size = getnFontSize();
        AG_PushTextState();
        AG_TextFont(pStatusFont);
        AG_TextFontPts(size);
        switch(pStatus->stat){
        case OSD_VFD_EMPTY:
            bg = black;
            fg = black;
            break;
        case OSD_VFD_NORM:
            bg = black;
            fg = n;
            break;
        case OSD_VFD_READ:
            bg = r;
            fg = black;
            break;
        case OSD_VFD_WRITE:
            bg = b;
            fg = black;
            break;
        default:
            bg = black;
            fg = black;
            pStatus->stat = OSD_VFD_EMPTY;
            break;
        }
        if(pStatus->stat != OSD_VFD_EMPTY){
            AG_TextColor(fg);
            AG_TextBGColor(bg);
            tmp = AG_TextRender(pStatus->VFDLetter);
            AG_FillRect(dst, NULL, bg);
            AG_SurfaceBlit(tmp, NULL, dst, 4, 4);
            AG_SurfaceFree(tmp);
        } else {
            AG_FillRect(dst, NULL, bg);
        }

        AG_PopTextState();
    } else {
       AG_SurfaceUnlock(dst);
       pStatus->OldStat = pStatus->stat;
       return FALSE;
    }
   
    //AG_SurfaceUnlock(dst);
    pStatus->OldStat = pStatus->stat;
   return TRUE;
}


static void CreateVFD(AG_Widget *parent, int drive, bool initflag)
{
    AG_Surface *out;
    AG_Color black;

    black.r = black.g = black.b = 0;
    black.a = 255;
    pwVFD[drive] = XM7_SDLViewNew(parent, NULL, NULL);
    if(pwVFD[drive] == NULL) return;
    out = XM7_SDLViewSurfaceNew(pwVFD[drive], nVFDWidth, nVFDHeight);
    AG_FillRect(out, NULL, black);
    XM7_SDLViewDrawFn(pwVFD[drive], DrawVFDFn,"%p", pVFDStat[drive]);
    AG_WidgetSetSize(pwVFD[drive], nVFDWidth, nVFDHeight);
    AG_WidgetShow(pwVFD[drive]);
}


void InitVFD(AG_Widget *parent)
{
   int i;
    if(parent == NULL) return;
    nVFDHeight = VFD_HEIGHT * 2;
    nVFDWidth = VFD_WIDTH * 2;
    for(i = 1; i >= 0 ;i--) {
       old_writep[i] = FALSE;
       szOldDrive[i][0] = '\0';
       szDrive[i][0] = '\0';
       
       pVFDStat[i] = (struct OsdVFDPack *)malloc(sizeof(struct OsdVFDPack));
       if(pVFDStat[i] == NULL) return;
       memset(pVFDStat[i], 0x00, sizeof(struct OsdVFDPack));
       pVFDStat[i]->init = TRUE;
       pVFDStat[i]->stat = OSD_VFD_EMPTY;
       pVFDStat[i]->OldStat = -1;
       pVFDStat[i]->VFDLetter[0] = '\0';
       pVFDStat[i]->drive = i;
       AG_MutexInit(&(pVFDStat[i]->mutex));
       CreateVFD(parent, i, TRUE);
    }
   
}

void DestroyVFD(void)
{
   int i;
   for(i = 1; i >= 0; i--) {
    if(pVFDStat[i] != NULL){
        if(pVFDStat[i]->pEmpty != NULL) AG_SurfaceFree(pVFDStat[i]->pEmpty);
        if(pVFDStat[i]->pNorm != NULL) AG_SurfaceFree(pVFDStat[i]->pNorm);
        if(pVFDStat[i]->pRead != NULL) AG_SurfaceFree(pVFDStat[i]->pRead);
        if(pVFDStat[i]->pWrite != NULL) AG_SurfaceFree(pVFDStat[i]->pWrite);
        AG_MutexDestroy(&(pVFDStat[i]->mutex));
        free(pVFDStat[i]);
        pVFDStat[i] = NULL;
    }
   }
   
   if(pwVFD[i] != NULL) AG_ObjectDetach(AGOBJECT(pwVFD[i]));
   pwVFD[i] = NULL;
}


void LinkSurfaceVFD(void)
{
}

void ResizeVFD(AG_Widget *parent, int w, int h)
{
       int total =  STAT_WIDTH + VFD_WIDTH * 2 * 2
                + CMT_WIDTH + LED_WIDTH * 3 + 50;
       float ww = (float)w;
       float wVFD = (float)VFD_WIDTH / (float)total;
       AG_Surface *surface;
       int i;

       nVFDWidth = (int)(ww * wVFD);
       nVFDHeight = h;
       for(i = 0; i < 2; i++) {
	  if((pVFDStat[i] == NULL) || (pwVFD[i] == NULL)) return;
       }
   

       for(i = 0; i < 2; i++) {
	  AG_MutexLock(&(pVFDStat[i]->mutex));
	  surface = XM7_SDLViewGetSrcSurface(pwVFD[i]);
	  AG_ObjectLock(pwVFD[i]);
	  if(surface != NULL) AG_SurfaceResize(surface, nVFDWidth, nVFDHeight);
	  AG_ObjectUnlock(pwVFD[i]);
	  AG_WidgetSetSize(pwVFD[i], nVFDWidth, nVFDHeight);
	  AG_WidgetSetPosition(pwVFD[i], (int)(((float)(STAT_WIDTH + VFD_WIDTH * (i - 1)) / (float)total) *  ww), 0);
	  AG_MutexUnlock(&(pVFDStat[i]->mutex));
       }
   
}

void ClearVFD(void)
{
   int i;
   for(i = 0; i < 2; i++) {
	szDrive[i][0] = '\0';
	szOldDrive[i][0] = '\0';
        if(pVFDStat[i] != NULL) {
	   
	   AG_MutexLock(&(pVFDStat[i]->mutex));
	   pVFDStat[i]->VFDLetter[0] = '\0';
	   pVFDStat[i]->stat = OSD_VFD_EMPTY;
	   pVFDStat[i]->OldStat = -1;
	   pVFDStat[i]->Changed = TRUE;
	   pVFDStat[i]->drive = i;
	   AG_MutexUnlock(&(pVFDStat[i]->mutex));
	}
      
   }
   
}


/*
 *  テープ描画
 */
void DrawDrive(int drive, BOOL override)
{
	int            num;
	int i;
	char          name[128];
	char          string[64];
	char          utf8[256];
	char		outstr[300];
	char          *pIn, *pOut;
	iconv_t       hd;
	size_t        in, out;
	AG_Surface *tmp;
	AG_Rect		rect;
        int stat;
        BOOL changed = override;

         if(pVFDStat[drive] == NULL) return;
	memset(string, 0x00, sizeof(string));
	memset(utf8, 0x00, sizeof(utf8));
	memset(outstr, 0x00, sizeof(outstr));


	ASSERT((drive >= 0) && (drive <= 1));


	/*
	 * 番号セット
	 */
	 if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
		 num = 255;
	 }  else {
		 num = fdc_access[drive];
		 if (num == FDC_ACCESS_SEEK) {
			 num = FDC_ACCESS_READY;
		 }
	 }

	 /*
	  * 名前取得
	  */
	 name[0] = '\0';
	 utf8[0] = '\0';
	 if (fdc_ready[drive] == FDC_TYPE_D77) {
		 strncpy(name, fdc_name[drive][fdc_media[drive]], 126);
	 }
	 if (fdc_ready[drive] == FDC_TYPE_2D) {
		 strcpy(name, "2D DISK");
	 }
	 if (fdc_ready[drive] == FDC_TYPE_VFD) {
		 strcpy(name, "VFD DISK");
	 }

	 /*
	  * 描画
	  */
	 memset(szDrive[drive], 0, 16);
	 strncpy(szDrive[drive], name, 16);
	 if (num == 255) {
		 strcpy(string, "");
	 } else {
		 strcpy(string, szDrive[drive]);
	 }

   
         if((fdc_ready[drive] == FDC_TYPE_NOTREADY) && (nDrive[drive] == num)){
	    stat = OSD_VFD_EMPTY;
	 } else if (nDrive[drive] == FDC_ACCESS_READ) {
	    stat = OSD_VFD_READ;
	 } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
	    stat = OSD_VFD_WRITE;
	 } else {
	    stat = OSD_VFD_NORM;
	 }
   
	 nDrive[drive] = num;
         AG_MutexLock(&(pVFDStat[drive]->mutex));
         pVFDStat[drive]->stat = stat;
	 if((stat != pVFDStat[drive]->OldStat) || (strcmp(szDrive[drive], szOldDrive[drive]) != 0) || 
	    (old_writep[drive] != fdc_writep[drive]) || (override == TRUE)) {
		 /*
		  * 過去のファイルネームと違う
		  */
		 memset(szOldDrive[drive], 0, 16);
		 strncpy(szOldDrive[drive], szDrive[drive], 16);
		 pIn = string;
		 pOut = utf8;
		 in = strlen(pIn);

		 out = 256;
		 hd = iconv_open("utf8", "cp932");
		 if(hd >= 0) {
			 while(in>0) {
				 iconv(hd, &pIn, &in, &pOut, &out);
			 }
			 iconv_close(hd);
		 }
		 if(strlen(utf8) >0) {
			 if(fdc_writep[drive]) {
				 sprintf(outstr, "■ %s", utf8); // 書込み禁止
			 } else {
				 sprintf(outstr, "　 %s", utf8); // 書込み許可
			 }
		 }
		 old_writep[drive] = fdc_writep[drive];
	         strncpy(pVFDStat[drive]->VFDLetter, outstr, 63);
	         pVFDStat[drive]->Changed = TRUE;
	         AG_WidgetUpdateSurface(AGWIDGET(pwVFD[drive]), pwVFD[drive]->mySurface);
	 }
	 AG_MutexUnlock(&(pVFDStat[drive]->mutex));
         
         AG_Redraw(pwVFD[drive]);

}

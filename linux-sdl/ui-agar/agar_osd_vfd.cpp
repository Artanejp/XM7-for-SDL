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
    
    AG_Surface *pSurface[4];
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

static BOOL UpdateVFD(XM7_SDLView *my, AG_Surface *dst, struct OsdVFDPack *pStatus);

extern "C" {
static void DrawVFDFn(AG_Event *event)
{
   XM7_SDLView *my = (XM7_SDLView *)AG_SELF();
   struct OsdVFDPack *disp = (struct OsdVFDPack *)AG_PTR(1);
   AG_Surface *dst;

   if((disp == NULL)  || (my == NULL)) return;
   if(disp->init != TRUE) return;
   dst = XM7_SDLViewGetSrcSurface(my);
   if(dst == NULL) return;
   if((disp->stat == disp->OldStat)
       && (disp->init == FALSE) && (disp->Changed == FALSE)) {
      disp->init = FALSE;
      disp->OldStat = disp->stat;
      disp->Changed = FALSE;
    return;
   }
   if(UpdateVFD(my, dst, disp)) {
      AG_SurfaceBlit(disp->pSurface[disp->stat], NULL, dst, 4, 4);
      AG_WidgetUpdateSurface(AGWIDGET(my), my->mySurface);
   }
}

}

static void UpdateVFDChanged(struct OsdVFDPack *pStatus, AG_Color *fg, AG_Color *bg, int status)
{
   AG_Surface *tmp;
   AG_Surface *dst;
   AG_Color bb;
   int size;
        
   if(pStatus == NULL) return;
   if(pStatus->init != TRUE) return;
   if((status > OSD_VFD_WRITE) || (status < 0)) return;
   dst = pStatus->pSurface[status];
   if(dst == NULL) return;
   if(dst->pixels == NULL) return;
   if(pStatusFont == NULL) return;
   
   bb.r = 0;
   bb.g = 0;
   bb.b = 0;
   bb.a = 0;
   AG_PushTextState();
        
   size = getnFontSize();
   AG_TextFont(pStatusFont);
   AG_TextFontPts(size);
   AG_TextColor(*fg);
   AG_TextBGColor(bb);
   tmp = AG_TextRender(pStatus->VFDLetter);
   AG_FillRect(dst, NULL, *bg);
   if(tmp != NULL){
      AG_SurfaceBlit(tmp, NULL, dst, 4, 4);
      AG_SurfaceFree(tmp);
   }

   AG_PopTextState();
        
}


static BOOL UpdateVFD(XM7_SDLView *my, AG_Surface *dst, struct OsdVFDPack *pStatus)
{
   AG_Rect rect;
   int i;
   int x;
   int y;
   int w;
   int h;
   int size;

   if((dst == NULL) || (pStatusFont == NULL)
    || (pStatus == NULL)) return FALSE;
   if((pStatus->stat > OSD_VFD_WRITE) || (pStatus->stat  < 0)){
      pStatus->stat = OSD_VFD_EMPTY;
   }
   if((pStatus->Changed != TRUE) && (pStatus->stat == pStatus->OldStat)) {
       pStatus->OldStat = pStatus->stat;
       pStatus->Changed = FALSE;
       return FALSE;
    }

   pStatus->Changed = FALSE;
   pStatus->OldStat = pStatus->stat;
   return TRUE;
}


static void CreateVFD(AG_Widget *parent, int drive)
{
    AG_Surface *out;
    AG_Color black;
    int i;

    black.r = black.g = black.b = 0;
    black.a = 255;
    pwVFD[drive] = XM7_SDLViewNew(parent, NULL, NULL);
    if(pwVFD[drive] == NULL) return;
    out = XM7_SDLViewSurfaceNew(pwVFD[drive], nVFDWidth, nVFDHeight);
   AG_FillRect(out, NULL, black);
   XM7_SDLViewDrawFn(pwVFD[drive], DrawVFDFn,"%p", pVFDStat[drive]);
   AG_WidgetSetSize(pwVFD[drive], nVFDWidth, nVFDHeight);
   for(i = 0; i < 4; i++) {
      pVFDStat[drive]->pSurface[i] = AG_SurfaceStdRGBA(nVFDWidth, nVFDHeight);
   }
   AG_WidgetShow(pwVFD[drive]);
}


void InitVFD(AG_Widget *parent)
{
   int i;
   int j;
    if(parent == NULL) return;
    nVFDHeight = VFD_HEIGHT;
    nVFDWidth = VFD_WIDTH;
    for(i = 1; i >= 0 ;i--) {
       old_writep[i] = FALSE;
       szOldDrive[i][0] = '\0';
       szDrive[i][0] = '\0';
       
       pVFDStat[i] = (struct OsdVFDPack *)malloc(sizeof(struct OsdVFDPack));
       if(pVFDStat[i] == NULL) return;
       memset(pVFDStat[i], 0x00, sizeof(struct OsdVFDPack));
       pVFDStat[i]->stat = OSD_VFD_EMPTY;
       pVFDStat[i]->OldStat = -1;
       pVFDStat[i]->VFDLetter[0] = '\0';
       pVFDStat[i]->drive = i;
       
       AG_MutexInit(&(pVFDStat[i]->mutex));
       CreateVFD(parent, i);
       pVFDStat[i]->init = TRUE;
    }
   
}

void DestroyVFD(void)
{
   int i;
   int j;
   for(i = 1; i >= 0; i--) {
    if(pVFDStat[i] != NULL){
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
       int total =  STAT_WIDTH + VFD_WIDTH * 2
                + CMT_WIDTH + LED_WIDTH * 3;
       float ww = (float)w;
       float wVFD = (float)VFD_WIDTH / (float)total;
       AG_Surface *surface;
       int i;
       int j;

       nVFDWidth = (int)(ww / 640.0f * (float)VFD_WIDTH);
       nVFDHeight = (int)(ww / 640.0f * (float)STAT_HEIGHT);
       for(i = 0; i < 2; i++) {
	  if((pVFDStat[i] == NULL) || (pwVFD[i] == NULL)) return;
       }
   

       for(i = 0; i < 2; i++) {
	  surface = XM7_SDLViewGetSrcSurface(pwVFD[i]);
	  AG_ObjectLock(pwVFD[i]);
	  if(surface != NULL) AG_SurfaceResize(surface, nVFDWidth, nVFDHeight);
	  AG_ObjectUnlock(pwVFD[i]);
	  pVFDStat[i]->init = TRUE;
	  AG_WidgetSetSize(pwVFD[i], nVFDWidth, nVFDHeight);
	  for(j = 0; j < 4; j++) {
	     if(pVFDStat[i]->pSurface[j] != NULL) AG_SurfaceFree(pVFDStat[i]->pSurface[j]);
	     pVFDStat[i]->pSurface[j] = AG_SurfaceStdRGBA(nVFDWidth, nVFDHeight);
	  }
	  pVFDStat[i]->Changed = TRUE;
       }

}

void ClearVFD(void)
{
   int i;
   for(i = 0; i < 2; i++) {
	szDrive[i][0] = '\0';
	szOldDrive[i][0] = '\0';
        if(pVFDStat[i] != NULL) {
	   pVFDStat[i]->VFDLetter[0] = '\0';
	   pVFDStat[i]->stat = OSD_VFD_EMPTY;
	   pVFDStat[i]->OldStat = -1;
	   pVFDStat[i]->Changed = TRUE;
	   pVFDStat[i]->drive = i;
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
        int stat = OSD_VFD_EMPTY;
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
		 strcpy(name, "*2D*");
	 }
	 if (fdc_ready[drive] == FDC_TYPE_VFD) {
		 strcpy(name, "VFD DISK");
	 }
         
	 /*
	  * 描画
	  */
         if(strlen(name) < 0) {
	    sprintf(name, "*INSERTED*");
	 }
   
	 memset(szDrive[drive], 0, 16);
	 strncpy(szDrive[drive], name, 16);
	 if (num == 255) {
	         char dstr[32];
	         dstr[0] = '\0';
		 strcpy(string, dstr);
	 } else {
		 strcpy(string, szDrive[drive]);
	 }

         if(fdc_ready[drive] == FDC_TYPE_NOTREADY){
	    stat = OSD_VFD_EMPTY;
	 } else if (nDrive[drive] == FDC_ACCESS_READ) {
	    stat = OSD_VFD_READ;
	 } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
	    stat = OSD_VFD_WRITE;
	 } else {
	    stat = OSD_VFD_NORM;
	 }
   
	 nDrive[drive] = num;
         pVFDStat[drive]->stat = stat;

         if((strcmp(szDrive[drive], szOldDrive[drive]) != 0) 
	    || (old_writep[drive] != fdc_writep[drive]) || (override == TRUE)) {
		 /*
		  * 過去のファイルネームと違う
		  */
		 memset(szOldDrive[drive], 0, 16);
		 strncpy(szOldDrive[drive], szDrive[drive], 16);
		 pIn = string;
		 pOut = utf8;
		 in = strlen(pIn);

		 out = 256;
		 hd = iconv_open("utf-8", "cp932");
		 if((hd >= 0) && (in > 0)){
			 while(in>0) {
				 iconv(hd, &pIn, &in, &pOut, &out);
			 }
			 iconv_close(hd);
		 }
		 if(strlen(utf8) >0) {
			 if(fdc_writep[drive]) {
				 sprintf(outstr, "■ %d:%s", drive, utf8); // 書込み禁止
			 } else {
				 sprintf(outstr, "　 %d:%s", drive, utf8); // 書込み許可
			 }
		 } else {
		    if(stat == OSD_VFD_EMPTY) {
         			 sprintf(outstr, "□ %d:*EMPTY*", drive); // 書込み禁止
		    } else {
			 if(fdc_writep[drive]) {
				 sprintf(outstr, "■ %d:*INSERTED*", drive); // 書込み禁止
			 } else {
				 sprintf(outstr, "　 %d:*INSERTED*", drive); // 書込み許可
			 }
		    }
		    
		    
		 }
	    
	    old_writep[drive] = fdc_writep[drive];
	    strncpy(pVFDStat[drive]->VFDLetter, outstr, 63);
	      {
		 AG_Color fg, bg;
		 AG_Color r, b, n, black;
		 AG_Surface *target;

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

		 bg = black;
		 fg = n;
		 UpdateVFDChanged(pVFDStat[drive],&fg, &bg, OSD_VFD_EMPTY);

		 bg = black;
		 fg = n;
		 UpdateVFDChanged(pVFDStat[drive],&fg, &bg, OSD_VFD_NORM);

		 bg = r;
		 fg = black;
		 UpdateVFDChanged(pVFDStat[drive],&fg, &bg, OSD_VFD_READ);

		 bg = b;
		 fg = black;
		 UpdateVFDChanged(pVFDStat[drive],&fg, &bg, OSD_VFD_WRITE);
	      } 	    // CheckDebug 20130119
	    pVFDStat[drive]->Changed = TRUE;
	    AG_WidgetUpdate(pwVFD[drive]);         
	 }
}

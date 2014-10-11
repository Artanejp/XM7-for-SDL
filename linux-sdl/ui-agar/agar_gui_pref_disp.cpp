/*
 * agar_gui_pref.cpp
 *
 * Preferences Page
 *
 *  Created on: 2010/11/25
 *      Author: whatisthis
 */

#include <libintl.h>
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <SDL/SDL.h>

#include "xm7.h"
#include "device.h"
#include "fdc.h"
#include "tapelp.h"
#include "opn.h"
#include "keyboard.h"
#include "mmr.h"
#include "mouse.h"
#include "aluline.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "display.h"
#include "device.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#else
#include "xm7_sdl.h"
#endif

#include "sdl_inifile.h"
#include "agar_cfg.h"
#include "xm7.h"


extern void OnPushCancel2(AG_Event *event);
extern void ConfigMenuOpenCL(struct gui_disp *cfg, AG_NotebookTab *parent);

static const char *ScreenSizeName[] =
{
		"320x200",
		"320x240",
		"640x400",
		"640x480",
		"800x500",
		"800x600",
		"960x600",
		"960x720",
		"1280x800",
		"1280x960",
                "1440x900",
                "1440x1080",
		"1600x1000",
                "1600x1200",
                "1920x1200",
                "1920x1440",
		NULL
};


static const WORD ScreenSizeWidth[] = {
		320,
		320,
		640,
		640,
		800,
		800,
		960,
		960,
		1280,
		1280,
                1440,
                1440,
                1600,
                1600,
                1920,
                1920
};

static const WORD ScreenSizeHeight[] = {
		200,
		240,
		400,
		480,
		500,
		600,
		600,
                720,
		800,
		960,
                900,
                1080,
                1000,
                1200,
                1200,
                1440
};


static void OnConfigApplyDisp(AG_Event *event)
{
        int ver;
	AG_Button *self = (AG_Button *)AG_SELF();
	struct gui_disp *cfg = AG_PTR(1);

	LockVM();
	if(cfg != NULL){
	  configdat.nDrawFPS = cfg->nDrawFPS;
	  configdat.nEmuFPS = cfg->nEmuFPS;
	  configdat.nBrightness = cfg->nBrightness;
	  configdat.nRenderMethod = cfg->nRenderMethod;
	  configdat.uWidth = cfg->uWidth;
	  configdat.uHeight = cfg->uHeight;
	  configdat.bFullScan = cfg->bFullScan;
	  configdat.bFullScanFS = cfg->bFullScanFS;
	  configdat.bSmoosing = cfg->bSmoosing;
	  configdat.bUseSIMD = cfg->bUseSIMD; 
#ifdef _USE_OPENCL
	  configdat.bUseOpenCL = cfg->bUseOpenCL;
	  configdat.nCLGlobalWorkThreads = cfg->nCLGlobalWorkThreads;
	  configdat.bCLSparse = cfg->bCLSparse; 
#endif
	  free(cfg);
	}
	ver = fm7_ver;
	ApplyCfg();
	/*
	 * VMヴァージョンが違ったら強制リセット
	 */
	if(ver != fm7_ver){
		system_reset();
	}
	/*
	 * ここにアイコン変更入れる
	 */

	/*
	 * 終了処理
	 */
	UnlockVM();

	if(self != NULL) {
	  AG_WindowHide(self->wid.window);
	  AG_ObjectDetach(self);
	}
}

static void OnChangeScreenReso(AG_Event *event)
{
        struct gui_disp *cfg = AG_PTR(1);
	int number = AG_INT(2);
   
        if(cfg == NULL) return;

	cfg->uWidth = ScreenSizeWidth[number];
	cfg->uHeight = ScreenSizeHeight[number];
        
        AG_SetVideoResizeCallback(NULL);
	ResizeWindow_Agar2(ScreenSizeWidth[number], ScreenSizeHeight[number]);
	if(cfg->nDrawFPS <= 1) {
		cfg->nDrawFPS = 2;
	}
        AG_SetVideoResizeCallback(ResizeWindow_Agar);
}


static void RenderMethodSelected(AG_Event *event)
{
   AG_TlistItem *list = (AG_TlistItem *)AG_PTR(2);
   struct gui_disp *cfg = AG_PTR(1);
   int method = -1;
   
   if(list == NULL) return;
   if(cfg == NULL) return;
   
   if(strcmp(gettext("Full Draw"), list->text) == 0) method = RENDERING_FULL;
   if(strcmp(gettext("Diff Block"), list->text) == 0) method = RENDERING_BLOCK;
   if(strcmp(gettext("Diff Raster"), list->text) == 0) method = RENDERING_RASTER;
   
   if(method < 0) return;
   if(method >= RENDERING_END) return;
   cfg->nRenderMethod = method;
   return;
}


static void OnConfigMenuScreen(struct gui_disp *cfg, AG_NotebookTab *parent)
{
	AG_Radio *radio;
	AG_Checkbox *check;
	AG_Numerical *fps;
	AG_Box *box;
	AG_Box *box2;
        AG_Combo *combo;
        AG_TlistItem *TlistItem[RENDERING_END];
	int i;
	int limit;
	
	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_VFILL);
	{
		radio = AG_RadioNewFn(AGWIDGET(box), 0, ScreenSizeName, OnChangeScreenReso, "%p", cfg);
		limit = sizeof(ScreenSizeHeight) / sizeof(WORD);
		for(i = 0; i <= limit; i++){
		  if((ScreenSizeWidth[i] == cfg->uWidth) && (ScreenSizeHeight[i] == cfg->uHeight)) break;
		}
		if(i >= limit) i = 2;
		AG_SetInt(radio, "value", i);
		box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_HFILL);
		fps = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, NULL ,gettext("Display rate"));
		AG_BindUint16(fps, "value", &(cfg->nDrawFPS));
		AG_NumericalSetRangeInt(fps, 2, 75);
		AG_NumericalSetIncrement(fps, 1);

		fps = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, NULL ,gettext("Emulation rate"));
		AG_BindUint16(fps, "value", &(cfg->nEmuFPS));
		AG_NumericalSetRangeInt(fps, 2, 75);
		AG_NumericalSetIncrement(fps, 1);
	   
		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		combo = AG_ComboNewS(AGWIDGET(box2), AG_COMBO_SCROLLTOSEL, gettext("Rendering Method"));
	        AG_ComboSizeHint(combo, "XXXXXXXXXXXX", RENDERING_END); 
	        TlistItem[0] = AG_TlistAddS(combo->list, NULL, gettext("Full Draw"));
	        TlistItem[1] = AG_TlistAddS(combo->list, NULL, gettext("Diff Block"));
	        TlistItem[2] = AG_TlistAddS(combo->list, NULL, gettext("Diff Raster"));
	        for(i = RENDERING_FULL; i < RENDERING_END; i++) {
		   if(i == cfg->nRenderMethod) {
			AG_ComboSelect(combo, TlistItem[i]);
		   }
		}
	   
	        AG_SetEvent(combo, "combo-selected", RenderMethodSelected, "%p", cfg);
	   
		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("Full Scan (15KHz)"), &(cfg->bFullScan));
		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("SMOOSING"), &(cfg->bSmoosing));
		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("Use OpenCL(Need REBOOT)"), &(cfg->bUseOpenCL));
		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("Use SIMD instructions(Need REBOOT)"), &(cfg->bUseSIMD));
	}
}


#ifdef USE_OPENGL
extern void SetBrightRGB_AG_GL2(float r, float g, float b);
static void OnChangeBright(AG_Event *event)
{
	AG_Slider *self = (AG_Slider *)AG_SELF();
        struct gui_disp *cfg = AG_PTR(1);
        WORD bright = AG_GetUint16(self, "value");
        float fBright0;
   
        if(cfg == NULL) return;
        if(bright < 0) bright = 0;
        if(bright > 255) bright = 255;
        fBright0 = (float)bright / 255.0f;
        SetBrightRGB_AG_GL2(fBright0, fBright0, fBright0);
        cfg->nBrightness = bright;
}
#endif /* USE_OPENGL */

static void ConfigMenuBright(struct gui_disp *cfg, AG_NotebookTab *parent)
{
	AG_Slider *slider;
	AG_Box *box;
	AG_Box *vbox;
	AG_Label *lbl;

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("Bright"));
//	slider = AG_SliderNewFltR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &fBright0, 0.0f, 1.0f);
	slider = AG_SliderNew(AGWIDGET(parent), AG_SLIDER_HORIZ, AG_SLIDER_HFILL);
        AG_SetUint16(slider, "value", cfg->nBrightness);
        AG_SetUint16(slider, "min", 0);
        AG_SetUint16(slider, "max", 255);
#ifdef USE_OPENGL
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeBright, "%p", cfg);
#endif /* USE_OPENGL */
	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_HFILL);
}

void OnConfigDisplayMenu(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_Notebook *note;
	AG_Notebook *note2;

	AG_NotebookTab *tab;
	AG_NotebookTab *tab2;
	AG_Box *box;
	AG_Button *btn;
        struct gui_disp *cfg;
   
        cfg = malloc(sizeof(struct gui_disp));
        if(cfg == NULL) return;
	{
	  LockVM();
	  cfg->nDrawFPS = configdat.nDrawFPS;
	  cfg->nEmuFPS = configdat.nEmuFPS;
	  cfg->nBrightness = configdat.nBrightness;
	  cfg->nRenderMethod = configdat.nRenderMethod;
	  cfg->uWidth = configdat.uWidth;
	  cfg->uHeight = configdat.uHeight;
	  cfg->bFullScan = configdat.bFullScan;
	  cfg->bFullScanFS = configdat.bFullScanFS;
	  cfg->bSmoosing = configdat.bSmoosing;
	  cfg->bUseSIMD = configdat.bUseSIMD; 
#ifdef _USE_OPENCL
	  cfg->bUseOpenCL = configdat.bUseOpenCL;
	  cfg->nCLGlobalWorkThreads = configdat.nCLGlobalWorkThreads;
	  cfg->bCLSparse = configdat.bCLSparse; 
#endif
	  UnlockVM();
	}

	win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
        note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_HFILL);

        {
    	tab = AG_NotebookAddTab(note, gettext("Screen"), AG_BOX_HORIZ);
    	OnConfigMenuScreen(cfg, tab);
#ifdef USE_OPENGL
    	tab = AG_NotebookAddTab(note, gettext("Display"), AG_BOX_HORIZ);
    	ConfigMenuBright(cfg, tab);
#endif /* USE_OPENGL */

#ifdef _USE_OPENCL
    	tab = AG_NotebookAddTab(note, gettext("OpenCL"), AG_BOX_HORIZ);
    	ConfigMenuOpenCL(cfg, tab);
#endif /* USE_OPENGL */
    }
    box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);
    AG_WidgetSetSize(AGWIDGET(box), 320, 24);
    {
    	AG_Box *vbox;
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnConfigApplyDisp, "%p", cfg);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
        AG_WidgetSetSize(AGWIDGET(vbox), 80, 24);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Cancel"), OnPushCancel2, "%p", cfg);
    }
    AG_SetEvent(win, "window-close", OnPushCancel2, "%p", cfg);
    AG_WindowSetCaption(win, gettext("Display"));
    AG_WindowShow(win);
}


/*
 * agar_gui_pref.cpp
 *
 * Preferences Page : OpenCL.
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


#include "agar_glcl.h"
#include "sdl_inifile.h"
#include "agar_cfg.h"
#include "xm7.h"

#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
#endif

#ifdef _USE_OPENCL
static void OnChangeGWS(AG_Event *event)
{
   AG_Numerical *me = (AG_Numerical *)AG_SELF();
   struct gui_disp *cfg = AG_PTR(1);
   int i;
   
   if(me == NULL) return;
   if(me->input == NULL) return;
   if(cfg == NULL) return;

   i = AG_TextboxInt(me->input);
   if(i <= 0) i = 1;
   if(i >= 256) i = 255;
   cfg->nCLGlobalWorkThreads = i;
}

static void OnChangePlatform(AG_Event *event)
{
   AG_Numerical *me = (AG_Numerical *)AG_SELF();
   struct gui_disp *cfg = AG_PTR(1);
   int i;
   
   if(me == NULL) return;
   if(me->input == NULL) return;
   if(cfg == NULL) return;

   i = AG_TextboxInt(me->input);
   if(i < 0)  i = 0;
   if(i >= 8) i = 7;
   cfg->nCLPlatformNum = i;
}

static void OnChangeCLDevice(AG_Event *event)
{
  struct gui_disp *cfg = (struct gui_disp *)AG_PTR(1);
  int i = AG_INT(2);
   
  if(cfg == NULL) return;

   if(i <= 0) i = 0;
   if(i >= 8) i = 7;
   cfg->nCLDeviceNum = i;
}



static void ConfigCLDevices(struct gui_disp *cfg, AG_Box *parent)
{
	AG_Radio *radio;
	AG_Box *box;
	AG_Box *box2;
        AG_Combo *combo;
	int i;
	int numdevices;
	int limit;
	int num_using = 0;
	num_using = cfg->nCLDeviceNum;
	//	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_VFILL);
	{
	  radio = AG_RadioNewFn(AGWIDGET(parent), 0, cfg->sDeviceName, OnChangeCLDevice, "%p", cfg);
	  AG_SetInt(radio, "value", num_using);
	}
}
#endif

void ConfigMenuOpenCL(struct gui_disp *cfg, AG_NotebookTab *parent)
{
#ifdef _USE_OPENCL
   AG_Slider *slider;
	AG_Box *box;
	AG_Box *box2;
	AG_Label *lbl;
        AG_Numerical *gws;
	AG_Numerical *platform;
	AG_Checkbox *check;
        AG_Event *ev;
   
	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_VFILL);
	{
	   box2 = AG_BoxNewVert(AGWIDGET(box), AG_BOX_HFILL);
	   check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("Multi threaded OpenCL"), &(cfg->bCLSparse));
	   lbl = AG_LabelNew(AGWIDGET(box2), AG_TEXT_LEFT, "%s", gettext("Global Work Items")); 
	   gws = AG_NumericalNewS(AGWIDGET(box2), AG_NUMERICAL_HFILL, NULL ,"");
	   AG_BindSint32(gws, "value", &(cfg->nCLGlobalWorkThreads));
	   AG_NumericalSetRangeInt(gws, 1, 255);
	   AG_NumericalSetIncrement(gws, 1);
	   AG_NumericalSetWriteable(gws, 1);
	   ev = AG_SetEvent (AGOBJECT(gws), "numerical-return", OnChangeGWS, NULL);

	   check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("Interoperability with OpenGL(Need to reboot)"), &(cfg->bCLInteropGL));

	   lbl = AG_LabelNew(AGWIDGET(box2), AG_TEXT_LEFT, "%s", gettext("CL Platform No.(Need to reboot)")); 
	   platform = AG_NumericalNewS(AGWIDGET(box2), AG_NUMERICAL_HFILL, NULL ,"");
	   AG_BindSint32(platform, "value", &(cfg->nCLPlatformNum));
	   AG_NumericalSetRangeInt(platform, 0, 7);
	   AG_NumericalSetIncrement(platform, 1);
	   AG_NumericalSetWriteable(platform, 1);
	   ev = AG_SetEvent (AGOBJECT(platform), "numerical-return", OnChangePlatform, NULL);

	   lbl = AG_LabelNew(AGWIDGET(box2), AG_TEXT_LEFT, "%s", gettext("Using CL device(Need to reboot)")); 
	   ConfigCLDevices(cfg, box2);
	}
	   
#endif
}

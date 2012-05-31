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
#include <SDL.h>
#include <SDL/SDL_mixer.h>

#include "xm7.h"
#include "device.h"
#include "fdc.h"
#include "tapelp.h"
#include "opn.h"
//#include "whg.h"
//#include "thg.h"
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
#include "sdl.h"
#endif

#include "sdl_inifile.h"
#include "agar_cfg.h"
//#include "sdl_prop.h"
//#include "sdl_sch.h"
//#include "sdl_snd.h"
//#include "sdl_bar.h"
#include "xm7.h"
//#include "api_draw.h"
//#include "api_scaler.h"

extern "C" {
static configdat_t localconfig;
}

extern void OnPushCancel(AG_Event *event);

extern "C" {
extern void ResizeGL(int w, int h);
}

static int EmuVMTypeSelected;
static int EmuCyclestealMode;

static int ScreenResoSelected;
static int ScreenAspectSelected;

static void OnConfigApply(AG_Event *event)
{
	int ver;
	AG_Button *self = (AG_Button *)AG_SELF();


	LockVM();
	memcpy(&configdat, &localconfig, sizeof(configdat_t));
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
	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);
}


static const char *EmuTypeName[] =
{
		"FM-7",
		"FM-77AV",
		"FM-77AVEX",
		NULL
};
enum EmuTypeNum  {
		FM7=1,
		FM77AV,
		FM77AVEX
};

static const char *EmuSpeedName[] =
{
		"SLOW Mode",
		"FAST Mode",
		NULL
};
enum EmuSpeedNum  {
		EMUSP_SLOW=0,
		EMUSP_FAST,
};
static const char *EmuCycleName[] =
{
		"Main CPU",
		"Main CPU using MMR",
		"Main CPU using MMR Fast",
		"Sub CPU",
		NULL
};

enum EmuCycleNum  {
	EMUCYCLE_MAIN = 0,
	EMUCYCLE_MAIN_MMR,
	EMUCYCLE_MAIN_FASTMMR,
	EMUCYCLE_SUB
};

static void OnSetEmulationMode(AG_Event *event)
{
	int number = AG_INT(1);
	localconfig.fm7_ver = number + 1;
}

static void OnSetCyclestealMode(AG_Event *event)
{
	int number = AG_INT(1);
	localconfig.cycle_steal = number;
}

void ConfigMenuEmulation(AG_NotebookTab *parent)
{
	AG_Radio *radio;
	AG_Checkbox *check;
	AG_Box *box;
	AG_Label *lbl;

	box = AG_BoxNewVert(AGWIDGET(parent), 0);
	{
	EmuVMTypeSelected = localconfig.fm7_ver - 1;
	lbl = AG_LabelNew(AGWIDGET(box), 0, gettext("Emulation Type"));
	radio = AG_RadioNewFn(AGWIDGET(box), 0, EmuTypeName, OnSetEmulationMode, NULL);
	AG_BindInt(radio, "value", &EmuVMTypeSelected);

	EmuCyclestealMode = localconfig.cycle_steal;
	lbl = AG_LabelNew(AGWIDGET(box), 0, gettext("Cycle Steal"));
	radio = AG_RadioNewFn(AGWIDGET(box), 0, EmuSpeedName, OnSetCyclestealMode, NULL);
	AG_BindInt(radio, "value", &EmuCyclestealMode);
	}

	box = AG_BoxNewVert(AGWIDGET(parent), 0);
	{
		check = AG_CheckboxNewInt (AGWIDGET(box), 0, gettext("Auto Speed Adjust"), &localconfig.bSpeedAdjust);
		check = AG_CheckboxNewInt (AGWIDGET(box), 0, gettext("Full Speed"), &localconfig.bCPUFull);
		check = AG_CheckboxNewInt (AGWIDGET(box), 0, gettext("Disable Speed adjust when motor on"), &localconfig.bTapeMode);
		check = AG_CheckboxNewInt (AGWIDGET(box), 0, gettext("Full Speed adjust when motor on"), &localconfig.bTapeFull);
		check = AG_CheckboxNewInt (AGWIDGET(box), 0, gettext("Wait on accessing FDD"), &localconfig.bFddWait);
	}

}

static AG_Numerical *NumMain;
static AG_Numerical *NumSub;
static AG_Numerical *NumMainMMR;
static AG_Numerical *NumMainFMMR;

static void OnResetCycles(AG_Event *event)
{
//	localconfig.main_speed = MAINCYCLES;
//	localconfig.sub_speed = SUBCYCLES;
//	localconfig.mmr_speed = MAINCYCLES_MMR;
//	localconfig.fmmr_speed = MAINCYCLES_FMMR;

	AG_NumericalSetValue(NumMain, (double)MAINCYCLES);
	AG_NumericalSetValue(NumSub, (double)SUBCYCLES);
	AG_NumericalSetValue(NumMainMMR, (double)MAINCYCLES_MMR);
	AG_NumericalSetValue(NumMainFMMR, (double)MAINCYCLES_FMMR);
        AG_WidgetUpdate(AGWIDGET(NumMain));
	localconfig.main_speed = MAINCYCLES;
	localconfig.sub_speed = SUBCYCLES;
	localconfig.mmr_speed = MAINCYCLES_MMR;
	localconfig.fmmr_speed = MAINCYCLES_FMMR;
}

void ConfigMenuVMSpeed(AG_NotebookTab *parent)
{
	AG_Box *box;
	AG_Label *lbl;
	AG_Button *btn;

	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	NumMain = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, gettext("cycles"), gettext("Main CPU"));
	AG_NumericalSetRangeInt(NumMain, 2, 9999);
	AG_NumericalSetIncrement(NumMain, 1);
	AG_BindUint32(NumMain, "value", &localconfig.main_speed);

   
   
	NumSub = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, gettext("cycles"), gettext("Sub CPU") );
	AG_NumericalSetRangeInt(NumSub, 2, 9999);
	AG_NumericalSetIncrement(NumSub, 1);
	AG_BindUint32(NumSub, "value", &localconfig.sub_speed);

	NumMainMMR = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, gettext("cycles"), gettext("Main CPU MMR"));
	AG_NumericalSetRangeInt(NumMainMMR, 2, 9999);
	AG_NumericalSetIncrement(NumMainMMR, 1);
	AG_BindUint32(NumMainMMR, "value", &localconfig.mmr_speed);

	NumMainFMMR = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, gettext("cycles"), gettext("Main CPU Fast MMR"));
	AG_NumericalSetRangeInt(NumMainFMMR, 2, 9999);
	AG_NumericalSetIncrement(NumMainFMMR, 1);
	AG_BindUint32(NumMainFMMR, "value", &localconfig.fmmr_speed);

	btn = AG_ButtonNew(AGWIDGET(parent), 0, gettext("Reset Default"), OnResetCycles, NULL);

}


static const char *ScreenSizeName[] =
{
		"320x200",
		"320x240",
		"640x400",
		"640x480",
		"800x500",
		"800x600",
		"1024x768",
		"1280x800",
		"1280x960",
		NULL
};

enum ScreenSizeVal
{
		SC_320x200 = 0,
		SC_320x240,
		SC_640x400,
		SC_640x480,
		SC_800x500,
		SC_800x600,
		SC_1024x768,
		SC_1280x800,
		SC_1280x960
};

static const WORD ScreenSizeWidth[] = {
		320,
		320,
		640,
		640,
		800,
		800,
		1024,
		1280,
		1280
};

static const WORD ScreenSizeHeight[] = {
		200,
		240,
		400,
		480,
		500,
		600,
		768,
		800,
		960
};
static const char *ScreenAspectName[] =
{
		"1:1",
		"4:3",
		"Free",
		NULL
};

static void OnChangeScreenReso(AG_Event *event)
{
	int number = AG_INT(1);
	ScreenResoSelected = number;
	localconfig.uWidth = ScreenSizeWidth[number];
	localconfig.uHeight = ScreenSizeHeight[number];
	ResizeWindow_Agar(ScreenSizeWidth[number], ScreenSizeHeight[number]);
	if(localconfig.nDrawFPS <= 1) {
		localconfig.nDrawFPS = 2;
	}
}

static void OnChangeScreenAspect(AG_Event *event)
{
	int number = AG_INT(1);
	ScreenAspectSelected = number;
	if(ScreenAspectSelected < 0) {
		ScreenAspectSelected = 0;
	}
	if(ScreenAspectSelected > nAspectFree) {
		ScreenAspectSelected = nAspectFree;
	}
	switch(ScreenAspectSelected){
	case nAspect11:
		if(localconfig.uWidth > localconfig.uHeight) {
			localconfig.uHeight = (localconfig.uWidth * 5) / 8;
		} else {
			localconfig.uWidth = (localconfig.uHeight * 8) / 5;
		}
		break;
	case nAspect43:
		if(localconfig.uWidth > localconfig.uHeight) {
			localconfig.uHeight = (localconfig.uWidth * 6) / 8;
		} else {
			localconfig.uWidth = (localconfig.uHeight * 8) / 6;
		}
		break;
	}
	ResizeWindow_Agar(localconfig.uWidth, localconfig.uHeight);
}



void OnConfigMenuScreen(AG_NotebookTab *parent)
{
	AG_Radio *radio;
	AG_Checkbox *check;
	AG_Numerical *fps;
	AG_Box *box;
	AG_Box *box2;
	int i;
	int limit;

	limit = sizeof(ScreenSizeHeight) / sizeof(WORD);
	for(i = 0; i <= limit; i++){
		if((ScreenSizeWidth[i] == localconfig.uWidth) && (ScreenSizeHeight[i] == localconfig.uHeight)) break;
	}
	if(i >= limit) i = 2;
	ScreenResoSelected = i;


	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_VFILL);
	{
		radio = AG_RadioNewFn(AGWIDGET(box), 0, ScreenSizeName, OnChangeScreenReso, NULL);
		radio = AG_RadioNewFn(AGWIDGET(box), 0, ScreenAspectName, OnChangeScreenAspect, NULL);
		AG_BindInt(radio, "value",&ScreenResoSelected);
		box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_HFILL);
//		fps = AG_NumericalNewUint16(AGWIDGET(box), AG_NUMERICAL_HFILL, gettext("Frames per Second") ,gettext("Display rate"), &localconfig.nDrawFPS);
		fps = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, NULL ,gettext("Display rate"));
		AG_BindUint16(fps, "value", &localconfig.nDrawFPS);
		AG_NumericalSetRangeInt(fps, 2, 75);
		AG_NumericalSetIncrement(fps, 1);

//		fps = AG_NumericalNewUint16(AGWIDGET(box), AG_NUMERICAL_HFILL, gettext("Frames per Second") ,gettext("Emulation rate"), &localconfig.nEmuFPS);
		fps = AG_NumericalNewS(AGWIDGET(box), AG_NUMERICAL_HFILL, NULL ,gettext("Emulation rate"));
		AG_BindUint16(fps, "value", &localconfig.nEmuFPS);
		AG_NumericalSetRangeInt(fps, 2, 75);
		AG_NumericalSetIncrement(fps, 1);

		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("Full Scan (15KHz)"), &localconfig.bFullScan);
		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("Sync to VSYNC"), &localconfig.bSyncToVSYNC);
		box2 = AG_BoxNewHoriz(AGWIDGET(box), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box2), AG_CHECKBOX_HFILL, gettext("SMOOSING"), &localconfig.bSmoosing);
	}
}

static float fBright0;
#ifdef USE_OPENGL
extern void SetBrightRGB_AG_GL2(float r, float g, float b);
static void OnChangeBright(AG_Event *event)
{
	AG_Slider *self = (AG_Slider *)AG_SELF();
        if(fBright0 < 0.0f) fBright0 = 0.0f;
        if(fBright0 > 1.0f) fBright0 = 1.0f;
        SetBrightRGB_AG_GL2(fBright0, fBright0, fBright0);
   
}
#endif /* USE_OPENGL */

static void ConfigMenuBright(AG_NotebookTab *parent)
{
	AG_Slider *slider;
	AG_Box *box;
	AG_Box *vbox;
	AG_Label *lbl;

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("Bright"));
	slider = AG_SliderNewFltR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &fBright0, 0.0f, 1.0f);
#ifdef USE_OPENGL
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeBright, NULL);
#endif /* USE_OPENGL */
	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_HFILL);

}

void OnConfigEmulationMenu(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_Notebook *note;
	AG_Notebook *note2;

	AG_NotebookTab *tab;
	AG_NotebookTab *tab2;
	AG_Box *box;
	AG_Button *btn;

	memcpy(&localconfig, &configdat, sizeof	(configdat_t));

	win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(win, 320, 240);
    note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_HFILL);
    {
    	/*
    	 * ここに、メニューの実体を置く
    	 */
    	tab = AG_NotebookAddTab(note, gettext("Emulation"), AG_BOX_HORIZ);
    	ConfigMenuEmulation(tab);

    	tab = AG_NotebookAddTab(note, gettext("Cycle"), AG_BOX_HORIZ);
    	ConfigMenuVMSpeed(tab);

    	tab = AG_NotebookAddTab(note, gettext("Screen"), AG_BOX_HORIZ);
    	OnConfigMenuScreen(tab);
#ifdef USE_OPENGL
    	tab = AG_NotebookAddTab(note, gettext("Display"), AG_BOX_HORIZ);
    	ConfigMenuBright(tab);
#endif /* USE_OPENGL */
    }
    box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);
    AG_WidgetSetSize(AGWIDGET(box), 320, 24);
    {
    	AG_Box *vbox;
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnConfigApply, NULL);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
        AG_WidgetSetSize(AGWIDGET(vbox), 80, 24);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Cancel"), OnPushCancel, NULL);
    }
	AG_WindowSetCaption(win, gettext("Preferences"));
	AG_WindowShow(win);
}

static const char *JsTypeName[] = {
		"None",
		"Port 1",
		"Port 2",
		"JoyKey",
		"Dempa",
		NULL
};
static 	int jsType[2];


static void OnChangeJsType(AG_Event *event)
{
	int number = AG_INT(1);
	int type = AG_INT(2);

	localconfig.nJoyType[number] = type;
}

static void InputMenuJS(AG_NotebookTab *parent)
{
	AG_Box *box;
	AG_Radio *radio;
	AG_Box *vbox;
	AG_Label *lbl;


	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_VFILL);
	{
		vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
		{
		lbl = AG_LabelNew(AGWIDGET(vbox), 0, gettext("Joystick #0"));
		jsType[0] = localconfig.nJoyType[0];
		radio = AG_RadioNewFn(AGWIDGET(vbox), 0, JsTypeName, OnChangeJsType, "%i", 0);
		AG_BindInt(radio, "value",&jsType[0]);
		}

		vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
		{
		lbl = AG_LabelNew(AGWIDGET(vbox), 0, gettext("Joystick #1"));
		jsType[1] = localconfig.nJoyType[1];
		radio = AG_RadioNewFn(AGWIDGET(vbox), 0, JsTypeName, OnChangeJsType, "%i", 1);
		AG_BindInt(radio, "value",&jsType[1]);
		}
	}

}

extern void BuildVkeyBoard(AG_Event *event);
extern void SetKeyTable(AG_Event *event);

void InputMenuKbd(AG_NotebookTab *parent)
{
	AG_Box *box;
	AG_Checkbox *check;
	AG_Box *vbox;
	AG_Button *btn;

	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);


	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Bind multi-pressed cursor as oblique direction"), &localconfig.bArrow8Dir);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Bind Cursor as Ten-Key"), &localconfig.bTenCursor);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Pseudo Realtime Scan"), &localconfig.bKbdReal);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Virtual Keyboard"), BuildVkeyBoard, NULL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Assign Keyboard"), SetKeyTable, NULL);
	}

}

void OnConfigInputMenu(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_Notebook *note;

	AG_NotebookTab *tab;
	AG_Box *box;
	AG_Button *btn;

	memcpy(&localconfig, &configdat, sizeof	(configdat_t));

	win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(win, 320, 240);
    note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_HFILL);
    {
    	/*
    	 * ここに、メニューの実体を置く
    	 */

    	tab = AG_NotebookAddTab(note, gettext("Keyboard"), AG_BOX_HORIZ);
    	InputMenuKbd(tab);

    	tab = AG_NotebookAddTab(note, gettext("Joystick"), AG_BOX_HORIZ);
    	InputMenuJS(tab);

    	tab = AG_NotebookAddTab(note, gettext("Misc"), AG_BOX_HORIZ);
    }
    box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);
    AG_WidgetSetSize(AGWIDGET(box), 320, 24);
    {
    	AG_Box *vbox;
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnConfigApply, NULL);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
        AG_WidgetSetSize(AGWIDGET(vbox), 80, 24);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Cancel"), OnPushCancel, NULL);
    }
	AG_WindowSetCaption(win, gettext("Configure Input"));
	AG_WindowShow(win);
}

static int SampleRateNum;
extern int iTotalVolume;   /* 全体ボリューム */
extern void  SetSoundVolume2(UINT uSp, int nFM, int nPSG, int nBeep, int nCMT, int nWav);
extern void SetTotalVolume(int vol);


static const char *SampleRateName[] = {
		"96000",
		"88200",
		"48000",
		"44100",
		"24000",
		"22050",
		"None",
		NULL
};
static const int SampleRates[] ={
		96000, 88200, 48000, 44100, 24000, 22050, 0
};

static void OnChangeSampleRate(AG_Event *event)
{
	int num = AG_INT(1);
	if(num > 6 ){
		num = 6;
	}
	localconfig.nSampleRate = SampleRates[num];
}

static void SoundMenu(AG_NotebookTab *parent)
{
	AG_Radio *radio;
	AG_Checkbox *check;
	AG_Numerical *num;
	AG_Label *lbl;
	AG_Box *box;
	AG_Box *box2;
	int i;


	for(i = 0; i < 6 ; i++) {
		if(localconfig.nSampleRate == SampleRates[i]) break;
	}
	if(i > 6) i = 6;
	SampleRateNum = i;

	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
		lbl = AG_LabelNew(AGWIDGET(box), 0, "%s", gettext("Sample Rate"));
		radio = AG_RadioNewFn(AGWIDGET(box), 0, SampleRateName, OnChangeSampleRate, NULL);
		AG_BindInt(radio, "value", &SampleRateNum);
		box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_HFILL);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("HQ Rendering"), &localconfig.bFMHQmode);
	}
	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
		num = AG_NumericalNewInt(AGWIDGET(box), AG_NUMERICAL_HFILL, gettext("Per Second") ,gettext("Sound Buffer"), &localconfig.nSoundBuffer);
		AG_NumericalSetRangeInt(num, 30, 2000);
		AG_NumericalSetIncrement(num, 10.0);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Force Stereo"), &localconfig.nStereoOut);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("FDD Seek & Motor"), &localconfig.bFddSound);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("CMT Monitor"), &localconfig.bTapeMon);
	}
}

static void SoundMenu2(AG_NotebookTab *parent)
{
	AG_Radio *radio;
	AG_Checkbox *check;
	AG_Numerical *num;
	AG_Label *lbl;
	AG_Box *box;
	AG_Box *box2;
	int i;



	box = AG_BoxNewVert(AGWIDGET(parent), AG_BOX_VFILL);
	{
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Enable OPN"), &localconfig.bOPNEnable);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Enable WHG"), &localconfig.bWHGEnable);
		check = AG_CheckboxNewInt(AGWIDGET(box), AG_CHECKBOX_HFILL, gettext("Enable THG"), &localconfig.bTHGEnable);
	}
}


static void OnChangeVolume(AG_Event *event)
{
	AG_Slider *self = (AG_Slider *)AG_SELF();

		SetSoundVolume2(localconfig.uChSeparation, localconfig.nFMVolume,
				localconfig.nPSGVolume, localconfig.nBeepVolume,
				localconfig.nCMTVolume, localconfig.nWaveVolume);
}

static void OnChangeTotalVolume(AG_Event *event)
{
	AG_Slider *self = (AG_Slider *)AG_SELF();
	SetTotalVolume(localconfig.iTotalVolume);

}


static void VolumeMenu(AG_NotebookTab *parent)
{
	AG_Slider *slider;
	AG_Box *box;
	AG_Box *vbox;
	AG_Label *lbl;

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("Main Volume"));
	slider = AG_SliderNewIntR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &localconfig.iTotalVolume, 0, 128);
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeTotalVolume, NULL);

	box = AG_BoxNewHoriz(AGWIDGET(parent), AG_BOX_HFILL);
	AG_WidgetSetSize(AGWIDGET(box), 320, 12);
	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("PSG"));
	slider = AG_SliderNewIntR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &localconfig.nPSGVolume, -40, 12);
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeVolume, NULL);

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("FM"));
	slider = AG_SliderNewIntR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &localconfig.nFMVolume, -40, 12);
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeVolume, NULL);

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("BEEP"));
	slider = AG_SliderNewIntR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &localconfig.nBeepVolume, -35, 12);
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeVolume, NULL);

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("CMT"));
	slider = AG_SliderNewIntR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &localconfig.nCMTVolume, -35, 6);
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeVolume, NULL);

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("SFX"));
	slider = AG_SliderNewIntR(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &localconfig.nWaveVolume, -35, 12);
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeVolume, NULL);

	lbl = AG_LabelNew(AGWIDGET(parent), 0, "%s", gettext("Channel Separation"));
	slider = AG_SliderNewUint32R(AGWIDGET(parent),AG_SLIDER_HORIZ, AG_SLIDER_HFILL, &localconfig.uChSeparation, 0, 16);
	AG_SetEvent(AGOBJECT(slider), "slider-changed", OnChangeVolume, NULL);

}
static void SoundMiscMenu(AG_NotebookTab *parent)
{

}

void OnConfigSoundMenu(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_Notebook *note;
	AG_Notebook *note2;

	AG_NotebookTab *tab;
	AG_NotebookTab *tab2;
	AG_Box *box;
	AG_Button *btn;

	memcpy(&localconfig, &configdat, sizeof	(configdat_t));

	win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(win, 320, 240);
    note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_HFILL);
    {
    	/*
    	 * ここに、メニューの実体を置く
    	 */

    	tab = AG_NotebookAddTab(note, gettext("Volume"), AG_BOX_VERT);
    	VolumeMenu(tab);
    	tab = AG_NotebookAddTab(note, gettext("Rendering"), AG_BOX_HORIZ);
    	SoundMenu(tab);
    	tab = AG_NotebookAddTab(note, gettext("Misc"), AG_BOX_HORIZ);
    	SoundMenu2(tab);

    }
    box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);
    AG_WidgetSetSize(AGWIDGET(box), 320, 24);
    {
    	AG_Box *vbox;
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnConfigApply, NULL);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
        AG_WidgetSetSize(AGWIDGET(vbox), 80, 24);
        vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    	btn = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("Cancel"), OnPushCancel, NULL);
    }
	AG_WindowSetCaption(win, gettext("Sound Preferences"));
	AG_WindowShow(win);
}


/*
 * agar_gui_sound.cpp
 *
 * Preferences Page of Sound.
 *
 *  Created on: 2014/10/03
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
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

extern configdat_t localconfig;

extern void OnPushCancel(AG_Event *event);
extern void OnConfigApply(AG_Event *event);

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
//	AG_WindowSetMinSize(win, 320, 240);
    note = AG_NotebookNew(AGWIDGET(win), AG_NOTEBOOK_HFILL);
    {
    	/*
    	 * 
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


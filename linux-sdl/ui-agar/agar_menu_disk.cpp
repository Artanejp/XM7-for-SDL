/*
 * agar_gui_diskmenu.cpp
 *
 *  Created on: 2010/11/24
 *      Author: whatisthis
 */
#include <SDL/SDL.h>
#include <libintl.h>
#include <iconv.h>

extern "C" {
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
}
#include "xm7.h"
#include "fdc.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#else
#include "xm7_sdl.h"
#include "sdl_cfg.h"
#endif

//#include "sdl_bar.h"
//#include "api_kbd.h"
//#include "sdl_sch.h"
//#include "sdl_snd.h"
#include "sdl_inifile.h"
//#include "api_draw.h"
//#include "sdl_gtkdlg.h"
#include "agar_toolbox.h"

/*
 * Drive 1/0 Menu
 */
extern void OnPushCancel(AG_Event *event);



AG_MenuItem *Menu_DiskImages[FDC_DRIVES];

/*-[ ディスクメニュー ]-----------------------------------------------------*/


/*
 *  ディスクイジェクト
 */
static void OnDiskEject(AG_Event *event)
{
    int            Drive = AG_INT(1);

	/*
	 * イジェクト
	 */
	LockVM();
    fdc_setdisk(Drive, NULL);
    UnlockVM();
}
    /*
     *  ディスク一時取り出し
     */
static void OnDiskTemp(AG_Event *event)
{
    int            Drive = AG_INT(1);

	/*
	 * 書き込み禁止切り替え
	 */
    LockVM();
    if (fdc_teject[Drive]) {
	fdc_teject[Drive] = FALSE;
    }

    else {
	fdc_teject[Drive] = TRUE;
    }
    UnlockVM();
}



/*
 *  メディア切り替え
 */
void OnMediaChange(AG_Event *event)
{
    int            Drive = AG_INT(1);
    int            Media = AG_INT(2);
    AG_Button *self = (AG_Button *)AG_SELF();

	/*
	 * 書き込み禁止切り替え
	 */
    LockVM();
    fdc_setmedia(Drive, Media);
    ResetSch();
    UnlockVM();

	AG_WindowHide(self->wid.window);
	AG_ObjectDetach(self->wid.window);

}

static void OnOpenDiskSub(int Drive, char *sFilename)
{
	char *p;
    /*
     * セット
     */
    LockVM();
    fdc_setdisk(Drive, sFilename);
    ResetSch();
    UnlockVM();
#ifdef _WINDOWS
    p = _mbsrchr(sFilename, '\\');
#else
    p = strrchr(sFilename, '/');
#endif
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[0], sFilename);
    }
}


static void OnOpenDiskSubEv(AG_Event *event)
{
    AG_FileDlg *dlg = (AG_FileDlg *)AG_SELF();
    char  *sFilename = AG_STRING(2);
    AG_FileType *ft = (AG_FileType *)AG_PTR(3);
    AG_MenuItem *parent;
    int Drv = AG_INT(1);

    OnOpenDiskSub(Drv, sFilename);
}

static void OnOpenDiskBothSub(char *sFilename)
{
	char *p;
    LockVM();
    fdc_setdisk(0, sFilename);
    fdc_setdisk(1, NULL);
    if ((fdc_ready[0] != FDC_TYPE_NOTREADY) && (fdc_medias[0] >= 2)) {
	fdc_setdisk(1, sFilename);
	fdc_setmedia(1, 1);
    }
    ResetSch();
    UnlockVM();
#ifdef _WINDOWS
    p = _mbsrchr(sFilename, '\\');
#else
    p = strrchr(sFilename, '/');
#endif
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[0], sFilename);
    }

}

static void OnOpenDiskBothSubEv(AG_Event *event)
{
    AG_FileDlg *dlg = (AG_FileDlg *)AG_SELF();
    char  *sFilename = AG_STRING(1);
    AG_FileType *ft = (AG_FileType *)AG_PTR(2);

    OnOpenDiskBothSub(sFilename);
}


static void OnOpenDisk(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *dlgWin;
	AG_FileDlg *dlg;
	int Drive = AG_INT(1);
	dlgWin = AG_WindowNew(0);
	if(dlgWin == NULL) return;
	AG_WindowSetCaption(dlgWin, "%s %d:", gettext("Open Disk Image"), Drive);
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
//    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_SAVE);
	if(dlg == NULL) return;
	AG_FileDlgSetDirectory (dlg, "%s", InitialDir[0]);
	AG_WidgetFocus(dlg);
	AG_FileDlgAddType(dlg, "D77 Disk Image File", "*.d77,*.D77", OnOpenDiskSubEv, "%i", Drive);
	AG_FileDlgAddType(dlg, "D88 Disk Image File", "*.d88,*.D88", OnOpenDiskSubEv, "%i", Drive);
	AG_FileDlgAddType(dlg, "2D Disk Image File", "*.2d,*.2D", OnOpenDiskSubEv, NULL);
	AG_FileDlgAddType(dlg, "VFD Disk Image File", "*.vfd,*.VFD", OnOpenDiskSubEv, NULL);
    AG_ActionFn(AGWIDGET(dlgWin), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
    AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
    AG_WindowShow(dlgWin);
}

static void OnOpenDiskBoth(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *dlgWin;
	AG_FileDlg *dlg;
	dlgWin = AG_WindowNew(0);
	if(dlgWin == NULL) return;
	AG_WindowSetCaption(dlgWin, "%s ", gettext("Open Disk Image Both"));
//	dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC );
    dlg = AG_FileDlgNew(dlgWin, AG_FILEDLG_LOAD | AG_FILEDLG_SAVE | AG_FILEDLG_ASYNC | AG_FILEDLG_CLOSEWIN);
	if(dlg == NULL) return;
	AG_FileDlgSetDirectoryS(dlg,  InitialDir[0]);
	AG_WidgetFocus(dlg);
	AG_FileDlgAddType(dlg, "D77 Disk Image File", "*.d77,*.D77", OnOpenDiskBothSubEv, NULL);
	AG_FileDlgAddType(dlg, "D88 Disk Image File", "*.d88,*.D88", OnOpenDiskBothSubEv, NULL);
	AG_FileDlgAddType(dlg, "2D Disk Image File", "*.2d,*.2D", OnOpenDiskBothSubEv, NULL);
	AG_FileDlgAddType(dlg, "VFD Disk Image File", "*.vfd,*.VFD", OnOpenDiskBothSubEv, NULL);
    AG_ActionFn(AGWIDGET(dlgWin), "window-close", OnPushCancel, NULL);
    AG_ActionFn(AGWIDGET(dlg), "window-close", OnPushCancel, NULL);
	AG_FileDlgCancelAction (dlg, OnPushCancel,NULL);
	AG_WindowShow(dlgWin);
}


static void DisplayWriteProtectDisk(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
	char Label[128];
	int Drive = AG_INT(1);

	strcpy(Label,gettext("Write Protect"));
	strcat(Label, " ");
	if(fdc_writep[Drive]) {
		strcat(Label, "ON");
	} else {
		strcat(Label, "Off");
	}
	AG_MenuSetLabel(item, Label);
}

/*
 * Set write-protect flag on logical-volume.
 */
static void SetWriteProtectDisk(AG_Event *event)
{
	BOOL flag = AG_INT(1);
	int Drive = AG_INT(2);
	AG_Button *self = (AG_Button *)AG_SELF();

    LockVM();
//	fdc_writep[Drive] = flag;
    fdc_setwritep(Drive, flag);
    ResetSch();
    UnlockVM();
    AG_WindowHide(self->wid.window);
    AG_ObjectDetach(self->wid.window);
}

static void OnWriteProtectDisk(AG_Event *event)
{
	AG_Menu *self = (AG_Menu *)AG_SELF();
	AG_MenuItem *item = (AG_MenuItem *)AG_SENDER();
	AG_Window *w;
	AG_Button   *btn[3];
	AG_Box *box;
	AG_Box *box2;
	char Label[128];
	int Drive = AG_INT(1);
	char *caption;
	AG_Label *lbl;
	int id;

	if(fdc_ready[Drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	sprintf(Label, "Drive %d:", Drive);
	caption = gettext("Switch Write Protect on");

	w = AG_WindowNew(AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE | AG_WINDOW_NORESIZE);
	AG_WindowSetMinSize(w, 230, 80);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s %d:", caption, Drive );
	AG_LabelSizeHint (lbl, 2, caption);
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 8);
	box = AG_BoxNewHoriz(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);
	box2 = AG_BoxNewVert(box, 0);
	btn[0] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("ON"), SetWriteProtectDisk, "%i,%i", TRUE, Drive);
	box2 = AG_BoxNewVert(box, 0);
	btn[1] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("OFF"), SetWriteProtectDisk, "%i,%i", FALSE, Drive);
	box2 = AG_BoxNewVert(box, 0);
	btn[2] = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("Cancel"), OnPushCancel, NULL);

	AG_WindowSetCaption(w, gettext(Label));
	AG_WindowShow(w);
}



static void OnSelectDiskMedia(AG_Event *event)
{
	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	int Drive = AG_INT(1);
	AG_Window *w;
	AG_Box *box;
	AG_Box *box2;
	AG_Label *lbl;
	AG_Button *btn;
	char *caption;
	int i;

	if(fdc_ready[Drive] == FDC_TYPE_NOTREADY) {
		return;
	}

	caption = gettext("Select Disk Image on ");
	w = AG_WindowNew(DIALOG_WINDOW_DEFAULT);
	AG_WindowSetMinSize(w, 250, 80);
	box = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
	AG_WidgetSetSize(box, 230, 32);
	lbl = AG_LabelNew(AGWIDGET(box), AG_LABEL_EXPAND, "%s %d:", caption, Drive );
	AG_LabelSizeHint (lbl, 2, caption);
	{
	 char utf8[512];
	 char          *pIn, *pOut;
	 iconv_t       hd;
	 size_t        in, out;
	 memset(utf8, 0x00, sizeof(utf8));

	 if(fdc_medias[Drive] <= 1) return;
	 for(i = 0; i < fdc_medias[Drive]; i++) {
		 box2 = AG_BoxNewHorizNS(w, AG_BOX_HFILL);
         pIn =  fdc_name[Drive][i];
         pOut = utf8;
	 pOut[0] = '\0';
          in = strlen(pIn);
          out = 512;
          hd = iconv_open("utf-8", "cp932");
          if((hd >= 0) && (in > 0)){
                  while(in>0) {
                          iconv(hd, &pIn, &in, &pOut, &out);
                  }
                  iconv_close(hd);
          }
		 btn = AG_ButtonNewFn (AGWIDGET(box2), 0, utf8 , OnMediaChange, "%i,%i",  Drive, i);

		}
	}
	box2 = AG_BoxNewVert(box, 0);
	btn = AG_ButtonNewFn (AGWIDGET(box2), 0, gettext("Cancel"), OnPushCancel, NULL);
	AG_WindowSetCaption(w, gettext("Select Image"));
	AG_WindowShow(w);
}


void CreateDiskMenu(AG_MenuItem *self, int Drive)
{
	AG_MenuItem *item;
	if((Drive >= FDC_DRIVES) || (Drive <0)) return;

	item = AG_MenuAction(self, gettext("Open"), NULL, OnOpenDisk, "%i", Drive);
	item = AG_MenuAction(self, gettext("Open Both"), NULL, OnOpenDiskBoth, NULL);
	AG_MenuSeparator(self);
	item = AG_MenuAction(self, gettext("Eject"), NULL, OnDiskEject, "%i", Drive);
	AG_MenuSeparator(self);
	/*
	 * 一時取り出し
	 */
	item = AG_MenuAction(self, gettext("Eject Temp"), NULL, OnDiskTemp, "%i", Drive);
	/*
	 * ライトプロテクト
	 */
	AG_MenuSeparator(self);
	item = AG_MenuAction(self, gettext("Write Protect"), NULL, OnWriteProtectDisk, "%i", Drive);
	/*
	 * ディスクイメージ選択
	 */
	AG_MenuSeparator(self);
	Menu_DiskImages[Drive] = AG_MenuAction(self, gettext("Image Select"), NULL, OnSelectDiskMedia, "%i", Drive);
}

void Create_Drive0Menu(AG_MenuItem *self)
{
	CreateDiskMenu(self, 0);
}

void Create_Drive1Menu(AG_MenuItem *self)
{
	CreateDiskMenu(self, 1);
}

/*
 * agar_menu_about.cpp
 *
 *  Created on: 2010/11/24
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
#include <libintl.h>
extern "C" {
#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/opengl.h>
}
#include "xm7.h"
#include "sdl_cpuid.h"

#ifdef USE_AGAR
#include "agar_xm7.h"
#include "agar_cfg.h"
#else
#include "xm7_sdl.h"
#include "sdl_cfg.h"
#endif

#include "sdl_sch.h"
#include "sdl_inifile.h"
#include "agar_toolbox.h"
#include "agar_cmd.h"
#ifdef _USE_OPENCL
#include "agar_glcl.h"
extern class GLCLDraw *cldraw;
#endif

extern void OnPushCancel(AG_Event *event);
#ifdef USE_OPENGL
extern AG_GLView *GLDrawArea;
#endif
extern "C" {
   extern int fm7_ver;
   extern BOOL cycle_steal;
   extern struct  XM7_CPUID *pCpuID;           /* CPUフラグ */
   extern BOOL bUseSIMD;
}

static void Dialog_OnPushOK(AG_Event *event)
{
    AG_Button *button = (AG_Button *)AG_SELF();
    AG_Surface *mark = (AG_Surface *)AG_PTR(1);
    AG_WindowHide(button->wid.window);
    AG_ObjectDetach(button->wid.window);
}

void OnAboutDialog(AG_Event *event)
{

	AG_MenuItem *self = (AG_MenuItem *)AG_SELF();
	AG_Window *win;
	AG_HBox  *hbox;
	AG_VBox *vbox1, *vbox2;
	AG_Button *btn;
	AG_Label *label;
	AG_Surface *mark = NULL;


        char string[768];
        char string2[1024];
        int l;
	char iconpath[MAXPATHLEN];
	char file[MAXPATHLEN];

	win = AG_WindowNew(DIALOG_WINDOW_DEFAULT);
	if(win == NULL) return;
	AG_WindowSetCaption(win, "%s", gettext("About XM7/SDL"));

	hbox = AG_HBoxNew(win, AG_HBOX_HFILL);

	iconpath[0] = '\0'; //あとでConfigから読めるようにする
#ifdef RSSDIR
	strcat(iconpath, RSSDIR);
#else
	strcat(iconpath, "./.xm7/");
#endif
	strcpy(file, iconpath); // 複数ディレクトリサーチに置き換える
        switch(fm7_ver) {
	 case 1: // FM-7/77
	   if(cycle_steal == TRUE) {
		strcat(file, "tenfm77.png");
	   } else {
		strcat(file, "tenfm7.png");
	   }
	   break;
	 case 2: // FM77AV
	   strcat(file, "tenfm77av.png");
	   break;
	 case 3:  // FM77AV40EX
	   strcat(file, "ten40ex.png");
	   break;
	 default:
	    strcat(file, "xm7.png");
	    break;
	}
   
        if(!AG_FileExists(file)) { // Fallback
	   strcpy(file, iconpath); // 複数ディレクトリサーチに置き換える
	   strcat(file, "xm7.png"); // アイコンはPNGで…
	}
   
	if(AG_FileExists(file)) {
		mark = AG_SurfaceFromPNG(file);
		if(mark != NULL) {
			vbox1 = AG_VBoxNew(hbox, AG_VBOX_VFILL);
			AG_PixmapFromSurface(vbox1, 0, mark);
		}
	}
   
#ifdef USE_OPENGL   
       if(GLDrawArea != NULL) {
#ifdef _USE_OPENCL
	  if(cldraw != NULL) {	
	     strcpy(string, "Render: OpenCL+OpenGL");
	  } else {
	     strcpy(string, "Render: OpenGL");
	  }
#else
	  strcpy(string, "Render: OpenGL");
#endif	  
       } else {
#endif // USE_OPENGL
	  strcpy(string, "Render: DirectDraw/SDLFB");
#ifdef USE_OPENGL
       }
#endif   
	  
	vbox2 = AG_VBoxNew(hbox, AG_VBOX_VFILL);
        strcat(string, "\n\nRendering mode:\n");
        switch(nRenderMethod) {
	 case RENDERING_FULL:
	   strcat(string, "FULL per frame");
	   break;
	 case RENDERING_BLOCK:
	   strcat(string, "BLOCK");
	   break;
	 case RENDERING_RASTER:
	   strcat(string, "RASTER");
	   break;
	 default:
	   strcat(string, "Undefined");
	   break;
	}
	label = AG_LabelNew(vbox2, 0, "FM-7/77AV/SX Emulateor \"XM7\"""\n "VERSTR"%s\n", string);
        AG_LabelJustify(label, AG_TEXT_RIGHT);
   
        strcpy(string2, "SIMD Features: \n");
// 以下、CPU依存なので移植の場合は同等の関数を作ること
#if defined(__x86_64__) || defined(__i386__)
   if(pCpuID->use_mmx) strcat(string2, "MMX ");
   if(pCpuID->use_mmxext) strcat(string2, "MMXEXT ");
   strcat(string, "\n");
   if(pCpuID->use_sse) strcat(string2, "SSE ");
   if(pCpuID->use_sse2) strcat(string2, "SSE2 ");
   if(pCpuID->use_sse3) strcat(string2, "SSE3 ");
   if(pCpuID->use_ssse3) strcat(string2, "SSSE3 ");
   strcat(string, "\n");
   if(pCpuID->use_sse41) strcat(string2, "SSE4.1 ");
   if(pCpuID->use_sse42) strcat(string2, "SSE4.2 ");
   if(pCpuID->use_sse4a) strcat(string2, "SSE4a ");
   strcat(string, "\n");
   if(pCpuID->use_3dnow) strcat(string2, "3DNOW ");
   if(pCpuID->use_3dnowp) strcat(string2, "3DNOWP ");
   if(pCpuID->use_abm) strcat(string2, "ABM ");
   if(pCpuID->use_avx) strcat(string2, "AVX ");
   if(pCpuID->use_cmov) strcat(string2, "CMOV ");
#else
#endif

   strcat(string2, "\n\nUsing SIMD:\n");
// 以下、CPU依存なので移植の場合は同等の関数を作ること
#if defined(__x86_64__) || defined(__i386__)
   if(bUseSIMD) {
      if(pCpuID->use_sse2) {
	   strcat(string2, " SSE2");
      }
      if(pCpuID->use_mmx) {
	   strcat(string2, " MMX");
      }
      
   } else {
     strcat(string2, " NONE");
   }
#else
#endif
	label = AG_LabelNew(vbox2, 0, "%s\n", string2);
	AG_LabelJustify(label, AG_TEXT_LEFT);

	hbox = AG_HBoxNew(win, AG_HBOX_HFILL);
	AG_LabelNew(hbox, 0, AUTSTR);

	btn = AG_ButtonNewFn (AGWIDGET(win), 0, gettext("OK"), Dialog_OnPushOK, "%p", (void *)mark);

	AG_WindowShow(win);
}

void Create_AboutMenu(AG_MenuItem *self)
{
	AG_MenuItem *item;
	item = AG_MenuAction(self, gettext("About XM7"), NULL, OnAboutDialog, NULL);
}


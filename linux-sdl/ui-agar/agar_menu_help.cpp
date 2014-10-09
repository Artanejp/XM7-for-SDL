/*
 * agar_menu_help.cpp
 *
 * Created on: 2014/10/06
 *     Author: Kyuma Ohta <whatisthis.sowhat at gmail.com>
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

#include "agar_menu_help.h"

/*
 */

static SDL_RWops *OpenInfoByLocale(void)
{
  char loc[4];
  char fullpath[MAXPATHLEN + 1];
  char *infopath;
  char ip[MAXPATHLEN*5];
  char *p;
  char *locptr;
  char *fileptr;
  struct HelpFileNamesType *v = HelpFileNames;
  SDL_RWops *fp = NULL;
  int i = 0;
  int pos;

  /*
   * Set file name of info.
   */
  p = getenv("LC_ALL");
  if(p != NULL) {
    strncpy(loc, p, 2);
  } else {
    strncpy(loc, "C", 2);
  }

  do {
    if(v[i].localename == NULL) {
      i = 0;
      break;
    }
    if(strncasecmp(loc, v[i].helpfile, 2) == 0) break;
    i++;
  } while(0);
  locptr = v[i].localename;
  fileptr = v[i].helpfile;

  /*
   * Search path
   */
  infopath = getenv("INFOPATH");
  if(infopath == NULL) {
    infopath = Help_DefaultInfoPath;
  }
  strncpy(ip, infopath, sizeof(ip) - 1);
  p = ip;
  do {
    infopath = strtok_r(p, ":,", &p);
    if(infopath != NULL) {
      strncpy(fullpath, infopath, MAXPATHLEN - strlen(fileptr) - 1);
      strcat(fullpath, "/");
      strcat(fullpath, fileptr);
      fp = SDL_RWFromFile(fullpath, "rb");
      if(fp != NULL) break;
    } else {
      strncpy(fullpath, fileptr, MAXPATHLEN - 1);
      fp = SDL_RWFromFile(fullpath, "rb");
      break;
    }
  } while(infopath != NULL);

  return fp;
}

static Uint8 *ReadInfoFile(SDL_RWops *fp, int *size)
{
  Uint8 *buf = NULL;
  Uint8 *p = NULL;
  int i;
  
  if(size != NULL) *size = 0;
  if(fp == NULL) return NULL;
  SDL_RWseek(fp, 0, RW_SEEK_END);
  i = SDL_RWtell(fp) + 1; 
  if(i <= 0) return NULL;
  SDL_RWseek(fp, 0, RW_SEEK_SET);

  buf = malloc(i + 1024);
  if(buf == NULL) {
    SDL_RWclose(fp);
    return NULL;
  }
  p = buf;
  SDL_RWread(fp, p, i, 1);
  p[i] = '\0';
  if(size != NULL) *size = i;
  SDL_RWclose(fp);
  return buf;
}

static void OnCloseHelpViewer(AG_Event *event)
{
  AG_Button *self = AG_SELF();
  Uint8 *rawdata = AG_PTR(1);
  
  if(rawdata != NULL) free(rawdata);
  if(self != NULL) {
    AG_WindowHide(self->wid.window);
    AG_ObjectDetach(self);
  }
}

static void OnHelpDialog(AG_Event *event)
{
  AG_MenuItem *self = AG_SELF();
  AG_Window *win;
  AG_Button *button;
  AG_Box *box;
  AG_Textbox *textbox;
  SDL_RWops *fp;
  Uint8 *buf;
  int size = 0;

  win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
  box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);

  fp = OpenInfoByLocale();
  buf = ReadInfoFile(fp, &size);
//  textbox = AG_TextboxNew(box, AG_TEXTBOX_MULTILINE | AG_TEXTBOX_READONLY | AG_TEXTBOX_VFILL, "%p", buf);
  if(buf == NULL) {
    const char *defaultstr = "Error opening info file.";
    buf = malloc(strlen(defaultstr) + 10);
    strcpy(buf, defaultstr);
    size = strlen(defaultstr);
    //AG_TextboxSetString(textbox, buf);
  }
//  textbox = AG_TextboxNewS(box,  AG_TEXTBOX_MULTILINE | AG_TEXTBOX_CATCH_TAB | AG_TEXTBOX_EXPAND | AG_TEXTBOX_EXCL, NULL);
  {
     char hint[256];
     memset(hint, "x", sizeof(char) * 200);
     hint[200] = '\0';
     textbox = AG_TextboxNew(box,  AG_TEXTBOX_READONLY | AG_TEXTBOX_MULTILINE | AG_TEXTBOX_CATCH_TAB | AG_TEXTBOX_EXPAND, NULL);
     AG_TextboxBindUTF8(textbox, buf, size);
     AG_TextboxSizeHint(textbox, hint);
     AG_TextboxSizeHintLines(textbox, 25);
  }
   
  {
    AG_Box *vbox;
    vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    button = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnCloseHelpViewer, "%p", buf);
  }
  AG_WindowSetCaption(win, gettext("HELP"));
  AG_WindowShow(win);
}
  

void Create_HelpMenu(AG_MenuItem *self)
{
	AG_MenuItem *item;
	item = AG_MenuAction(self, gettext("HELP"), NULL, OnHelpDialog, NULL);
}

  

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

struct index_list{
  int index;
  char *buf;
  char indexname[512];
};

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
  AG_Button           *self = AG_SELF();
  Uint8               *rawdata = AG_PTR(1);
  struct index_list   *indexs = AG_PTR(2);
  int                 indexnum = AG_INT(3);
  int i;
  if(rawdata != NULL) free(rawdata);
  if(indexs != NULL)  {
    for(i = 0; i < indexnum; i++) free(indexs[i].buf);
    free(indexs);
  }
  if(self != NULL) {
    AG_WindowHide(self->wid.window);
    AG_ObjectDetach(self);
  }
}

static void OnClickNode(AG_Event *event)
{
  AG_Widget *self = AG_SELF();
  AG_Tlist *tbl = AG_PTR(1);
  AG_Textbox *textbox = AG_PTR(2);
  struct index_list *index;
  
  if(self == NULL) return;
  if(tbl == NULL) return;
  index = AG_TlistSelectedItemPtr(tbl);
  if((index != NULL) && (textbox != NULL)){
    //    AG_TextboxSetCursorPos(textbox, index->index);
    AG_TextboxSetString(textbox, index->buf);
  }
}


static int GetIndexLines(char *begin, char *end, struct index_list **indexs)
{
  int i = 0;
  char *p = begin;
  char *OneLineHead;
  char *nodehead, *nodename, *nodepos;
  struct index_list *index1, *index2;

  index1 = malloc(sizeof(struct index_list));
  
  do {
    index1[i].index = 0;
    index1[i].indexname[0] = '\0';
    index1[i].buf = NULL;
    p = strstr(p, "Node: ");
    if(p >= end) break;
    if(p == NULL) break;
    
    OneLineHead = p;
    nodehead = strtok_r(OneLineHead, " ", &OneLineHead);
    nodename = strtok_r(OneLineHead, "\x7f", &OneLineHead); 
    nodepos  = strtok_r(OneLineHead, "\n", &OneLineHead);
    p = OneLineHead; 
    //    if((strcmp(nodehead, "Node") != 0) || (nodename == NULL) || (nodepos == NULL)) continue;
    if((nodename == NULL) || (nodepos == NULL)) continue;
    index1[i].index = strtol(nodepos, &nodepos, 10);
    strncpy(index1[i].indexname, nodename, 511);
    index2 = realloc(index1, (i + 2) * sizeof(struct index_list));
    if(index2 == NULL) {
      if(index1 != NULL) free(index1);
      index1 = NULL;
      break;
    }
    index1 = index2;
    i++;
  } while(1);

  if((index1 != NULL) && (i > 0)){
    int index_size = i * sizeof(struct index_list);
    index2 = malloc(index_size);
    if(index2 != NULL) memcpy(index2, index1, index_size);
    free(index1);
    if(indexs != NULL) *indexs = index2;
    return i;
  }
  if(index1 != NULL) free(index1);
  if(indexs != NULL) *indexs = NULL;
  return 0;
}

static AG_Tlist *GenIndex(AG_Widget *parent, char *buf, char **buf2, struct index_list **lines, int *num)
{
  char *p, *q, *pp;
  int i;
  AG_Tlist *tbl;
  AG_TlistItem *item;
  char *OneLineHead;
  char *nodehead, *nodename, *nodepos;
  long pos;
  int linecnt;
  struct index_list *indexs;
  int bufsize;
  int len;
  
  const char *header = "\x1f\x0aTag Table:";
  const char *footer = "End Tag Table";
  if(buf == NULL) return NULL;
  if(buf2 == NULL) return NULL;
  p = strstr(buf, header);
  q = strstr(p, footer);

  if(p != NULL) {
    len = p - buf;
    pp = malloc((len + 32) * sizeof(char));
    strncpy(pp, buf, len);
    *buf2 = pp;
    pp[len] = '\0';
  } 
  if((p == NULL) || (q == NULL)) {
    return NULL;
  }

  /*
   * Parse index as right WIDGET.
   */
  tbl = AG_TlistNew(parent, AG_TLIST_TREE | AG_TLIST_VFILL);
  AG_TlistSizeHint(tbl, "xxxxxxxxxxxxxxxxxx", 25);
  
  linecnt = GetIndexLines(p, q, &indexs);
  
  i = 0;
    for(i = 0; i < linecnt; i++) {
    if((indexs != NULL) && (i <= linecnt)) {
      bufsize = indexs[i + 1].index - indexs[i].index;
      if(bufsize <= 0) bufsize = len - indexs[i].index;
      if(bufsize > 0) {
	indexs[i].buf = malloc(bufsize + 1);
      } else {
	indexs[i].buf = NULL;
      }
      if(indexs[i].buf != NULL) {
	memcpy(indexs[i].buf, &buf[indexs[i].index], bufsize);
	indexs[i].buf[bufsize] = '\0';
      }
      item = AG_TlistAddPtr(tbl, NULL, indexs[i].indexname, &indexs[i]);
    }
  }
  if(lines != NULL) *lines = indexs;
  if(num != NULL) *num = linecnt;
  return tbl;
}

static void OnHelpDialog(AG_Event *event)
{
  AG_MenuItem *self = AG_SELF();
  AG_Window *win;
  AG_Button *button;
  AG_Box *box;
  AG_Textbox *textbox;
  AG_Tlist *index = NULL;
  SDL_RWops *fp;
  Uint8 *buf = NULL;
  Uint8 *buf2 = NULL;
  struct index_list *indexs = NULL;
  int size = 0;
  int linecnt;

  win= AG_WindowNew(DIALOG_WINDOW_DEFAULT);
  box = AG_BoxNewHoriz(AGWIDGET(win), AG_BOX_HFILL);

  fp = OpenInfoByLocale();
  buf = ReadInfoFile(fp, &size);
  if(buf != NULL) {
    index = GenIndex(AGWIDGET(box), buf, &buf2, &indexs, &linecnt);
    free(buf);
  }
  
  if(buf2 == NULL) {
    const char *defaultstr = "Error opening info file.";
    buf2 = malloc(strlen(defaultstr) + 10);
    strcpy(buf2, defaultstr);
    size = strlen(defaultstr);
  }
  {
     char hint[256];
     memset(hint, 'X', sizeof(char) * 80);
     hint[80] = '\0';
     textbox = AG_TextboxNew(box,  AG_TEXTBOX_READONLY | AG_TEXTBOX_MULTILINE | 
			           AG_TEXTBOX_CATCH_TAB | AG_TEXTBOX_EXPAND, NULL);
     AG_TextboxBindUTF8(textbox, buf2, size);
     if(indexs != NULL) AG_TextboxSetString(textbox, indexs[0].buf);
     AG_TextboxSizeHint(textbox, hint);
     AG_TextboxSizeHintLines(textbox, 25);
  }
  if(index != NULL) AG_TlistSetDblClickFn(index, OnClickNode, "%p,%p", index, textbox);

  {
    AG_Box *vbox;
    vbox = AG_BoxNewVert(AGWIDGET(box), AG_BOX_VFILL);
    button = AG_ButtonNewFn(AGWIDGET(box), 0, gettext("OK"), OnCloseHelpViewer, "%p,%p,%i", buf2, indexs, linecnt);
  }
  AG_WindowSetCaption(win, gettext("HELP"));
  AG_WindowShow(win);
}
  

void Create_HelpMenu(AG_MenuItem *self)
{
	AG_MenuItem *item;
	item = AG_MenuAction(self, gettext("HELP"), NULL, OnHelpDialog, NULL);
}

  

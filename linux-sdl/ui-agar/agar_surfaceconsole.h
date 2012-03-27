/*
* Console Class for AG_Surface
* (Not Widget)
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* License: (WIP)
* History:
* 21 Mar, 2012 Initial
*/

#ifndef AGAR_SURFACECONSOLE_H_INCLUDED
#define AGAR_SURFACECONSOLE_H_INCLUDED

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>
#include "xm7.h"

class DumpObject {
public:
    DumpObject();
    ~DumpObject();
    BOOL InitConsole(int w, int h);
    void MoveCursor(int x, int y);
    void MoveDrawPos(int x, int y);
    BOOL Txt2UTF8(BYTE b, BYTE *disp);
    void PutCharScreen(BYTE c);
    void PutChar(BYTE c);
    int PutString(char *str);
    void PollDraw(void);

    void Draw(BOOL redraw);
    void DrawCursor(BOOL Flag);
    int  SizeAlloc(AG_SizeAlloc *a);

    int GetWidth(void);
    int GetHeight(void);
    int GetX(void);
    int GetY(void);
    int GetCurX(void);
    int GetCurY(void);
    AG_Surface *GetScreen(void);

private:
    unsigned char *ConsoleBuf;
    unsigned char *BackupConsoleBuf;
    BOOL  Changed;
    AG_Surface *Screen;
    AG_Font *TextFont;
    AG_Font *SymFont;
    AG_Mutex mutex;
    int CURX;
    int CURY;
    int X;
    int Y;
    int W;
    int H;
    int CHRW;
    int CHRH;

    int curpos;
    AG_Color fgColor;
    AG_Color bgColor;
    void InitFont(void);
    BOOL ExistSym;
};



#endif // AGAR_SURFACECONSOLE_H_INCLUDED

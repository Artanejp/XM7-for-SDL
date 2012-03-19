/*
* Debugger for XM-7/SDL
* (C) 2012 K.Ohta
* History:
*  20120319 : Initial
*/

#include <agar/core/types.h>
#include <agar/core.h>
#include <agar/gui.h>

#include "agar_xm7.h"
#include "xm7-debugger/memdef.h"
#include "xm7-debugger/memread.h"

// DumpObject: Charcode = 00h-ffh
class DumpObject {
    DumpObject();
    ~DumpObject();
    BOOL InitConsole(int w, int h);
    void MoveCursor(int x, int y);
    BOOL Txt2UTF8(BYTE b, BYTE *disp);
    void PutChar(BYTE c);

    void SetReader(BYTE (*rf)(WORD));
    void SetWriter(void FASTCALL (*wf)(WORD, BYTE));
private:
    unsigned int *ConsoleBuf;
    BOOL  Changed;
    AG_Surface *Screen;
    AG_Font *TextFont;
    AG_Font *SymFont;
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
    BYTE (*readfunc)(WORD);
    void FASTCALL (*writefunc)(WORD, BYTE);
    BOOL ExistSym;
};

static void SetPixelFormat(AG_PixelFormat *fmt)
{
   if(fmt == NULL) return;
   // Surfaceつくる
	fmt->BitsPerPixel = 32;
	fmt->BytesPerPixel = 4;
#ifdef AG_BIG_ENDIAN
	fmt->Rmask = 0x000000ff; // R
	fmt->Gmask = 0x0000ff00; // G
	fmt->Bmask = 0x00ff0000; // B
	fmt->Amask = 0xff000000; // A
#else
	fmt->Rmask = 0x00ff0000; // R
	fmt->Gmask = 0x0000ff00; // G
	fmt->Bmask = 0xff000000; // B
	fmt->Amask = 0x000000ff; // A
#endif
	fmt->Rshift = 0;
	fmt->Gshift = 8;
	fmt->Bshift = 16;
	fmt->Ashift = 24;
	fmt->Rloss = 0;
	fmt->Gloss = 0;
	fmt->Bloss = 0;
	fmt->Aloss = 0;
	fmt->palette = NULL;

}


DumpObject::DumpObject()
{
    ConsoleBuf = NULL;
    Changed = FALSE;
    TextFont = NULL;
    SymFont = NULL;
    X = 0;
    Y = 0;
    W = 0;
    H = 0;
    CHRW = 0;
    CHRH = 0;
    Screen = NULL;
    InitFont();
}

DumpObject::~DumpObject()
{
    if(TextFont != NULL) AG_DestroyFont(TextFont);
    if(SymFont != NULL) AG_DestroyFont(SymFont);
    if(ConsoleBuf != NULL) delete [] ConsoleBuf;
}

void DumpObject::InitFont(void)
{
    AG_Surface *dummy;
    char c[3];
    AG_PushTextState();
    TextFont = AG_TextFontLookup("F-Font.ttf", 16, 0);
    if(TextFont == NULL) { // Fallback
        TextFont = AG_TextFontPts(16); //  16pts
    }

    SymFont = AG_TextFontLookup("F-Font_Symbol.ttf", 16, 0);
    if(SymFont == NULL) { // Fallback
        SymFont = AG_TextFontPts(16);//  16pts
        ExistSym = FALSE; // NO SYMBOL FONT
    } else {
        ExistSym = TRUE; // SYMBOL Exists
    }
    if(TextFont != NULL){
        c[0] = 'A';
        c[1] = '\0';
        AG_TextFont(TextFont);
        dummy = AG_TextRender(c);
        if(dummy != NULL) {
            CHRW = dummy->w;
            CHRH = dummy->h;
            AG_SurfaceFree(dummy);
        }
    }
    AG_PopTextState();
}

void DumpObject::SetReader(BYTE (*rf)(WORD))
{
    readfunc = rf;
}

void DumpObject::SetWriter(void FASTCALL (*wf)(WORD, BYTE))
{
    writefunc = wf;
}

BOOL DumpObject::InitConsole(int w, int h)
{
    int size;
    AG_PixelFormat fmt;

    if((w <= 0) || (h <= 0)) return FALSE;
    W = w;
    H = h;
    X = 0;
    Y = 0;
    size = w * h;
    ConsoleBuf = new unsigned int[size];
    SetPixelFormat(&fmt);
    Screen = AG_SurfaceNew(AG_SURFACE_PACKED, W * CHRW, H * CHRH, &fmt, AG_SRCALPHA);
}

void DumpObject::MoveCursor(int x, int y)
{
    X = x;
    Y = y;
    if(X >= W) X = W - 1;
    if(Y >= H) Y = H - 1;
    if(X <= 0) X = 0;
    if(Y <= 0) Y = 0;

    curpos = X + Y * W;
}

BOOL DumpObject::Txt2UTF8(BYTE b, BYTE *disp)
{
   unsigned int utf;
   if(b <= 0x7f) {
       disp[0] = b;
       disp[1] = '\0';
       return TRUE;
   } else if((b <= 0xdf) || (b >= 0xa1)) {
       utf = 0xfec0 + (unsigned int)b;
        disp[0] = ((utf >> 12) & 0x0f) | 0xe0;
        disp[1] = ((utf >> 6) & 0x3f) | 0x80;
        disp[2] = (utf & 0x3f) | 0x80;
        disp[3] = '\0';
        return TRUE;
   } else {
       disp[0] = b;
       disp[1] = '\0';
       return FALSE;
   }

}

void DumpObject::PutChar(BYTE c)
{
    BYTE utf8[8];
    AG_Surface *r = NULL;

    utf8[0] = '\0';
    if(Txt2UTF8(c, utf8)) {
        if(TextFont != NULL){
            AG_PushTextState();
            AG_TextFont(TextFont);
            AG_TextColor(fgColor);
            AG_TextBGColor(bgColor);
            AG_TextValign(AG_TEXT_MIDDLE);
            r = AG_TextRender((const char *)utf8);
            AG_PopTextState();
        }
    } else { // Symbol
        if(c < 0x20){
            Txt2UTF8(' ', utf8);
        }
        if(SymFont != NULL){
            AG_PushTextState();
            AG_TextFont(SymFont);
            AG_TextColor(fgColor);
            AG_TextBGColor(bgColor);
            AG_TextValign(AG_TEXT_MIDDLE);
            r = AG_TextRender((const char *)utf8);
            AG_PopTextState();
        }
    }
    if(r != NULL){
        if(Screen != NULL){
            AG_SurfaceBlit(r, NULL, Screen, X * CHRW, Y * CHRH);
        }
        AG_SurfaceFree(r);
    }
}

/*
* Debugger for XM-7/SDL
* (C) 2012 K.Ohta
* History:
*  20120319 : Initial
*/

#include "agar_surfaceconsole.h"

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
    BackupConsoleBuf = NULL;
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
    AG_MutexInit(&mutex);
    InitFont();
}

DumpObject::~DumpObject()
{
    AG_MutexLock(&mutex);
    if(TextFont != NULL) AG_DestroyFont(TextFont);
    if(SymFont != NULL) AG_DestroyFont(SymFont);
    if(ConsoleBuf != NULL) delete [] ConsoleBuf;
    if(BackupConsoleBuf != NULL) delete [] BackupConsoleBuf;
    AG_MutexDestroy(&mutex);
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

BOOL DumpObject::InitConsole(int w, int h)
{
    int size;
    AG_PixelFormat fmt;
    int xx;
    int yy;

    if((w <= 0) || (h <= 0)) return FALSE;
    AG_MutexLock(&mutex);
    W = w;
    H = h;
    X = 0;
    Y = 0;
    size = w * h;
    if(ConsoleBuf != NULL) delete [] ConsoleBuf;
    if(BackupConsoleBuf != NULL) delete [] ConsoleBuf;
    if(Screen != NULL) AG_SurfaceFree(Screen);
    ConsoleBuf = new unsigned char[size];
    BackupConsoleBuf = new unsigned char[size];
    for(yy = 0; yy < H; yy++){
        for(xx = 0; xx < W; xx++){
            ConsoleBuf[xx + yy * W] = 0x00;
            BackupConsoleBuf[xx + yy * W] = 0x00;
        }
    }
    SetPixelFormat(&fmt);
    Screen = AG_SurfaceNew(AG_SURFACE_PACKED, W * CHRW, H * CHRH, &fmt, AG_SRCALPHA);
    AG_MutexUnlock(&mutex);
}

void DumpObject::MoveCursor(int x, int y)
{
    AG_MutexLock(&mutex);
    X = x;
    Y = y;
    if(X >= W) X = W - 1;
    if(Y >= H) Y = H - 1;
    if(X <= 0) X = 0;
    if(Y <= 0) Y = 0;
    AG_MutexUnlock(&mutex);
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

void DumpObject::PutCharScreen(BYTE c)
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

void DumpObject::PutChar(BYTE c)
{
    curpos = X + Y * W;
    AG_MutexLock(&mutex);
    ConsoleBuf[curpos] = c;
    AG_MutexUnlock(&mutex);
}


void DumpObject::Draw(BOOL redraw)
{
    int xx;
    int yy;
    int Xb;
    int Yb;
    int pos;

    // Backup X,Y
    AG_MutexLock(&mutex);

    Xb = X;
    Yb = Y;
    if(redraw){
        for(yy = 0; yy < H; yy++){
            pos = yy * W;
            for(xx = 0; xx < W; xx++){
                X = xx;
                Y = yy;
                PutCharScreen(ConsoleBuf[pos + xx]);
                BackupConsoleBuf[pos + xx] = ConsoleBuf[pos + xx];
            }
        }
    } else { // Diff
        for(yy = 0; yy < H; yy++){
            pos = yy * W;
            for(xx = 0; xx < W; xx++){
                if(ConsoleBuf[pos + xx] != BackupConsoleBuf[pos + xx]){
                    X = xx;
                    Y = yy;
                    PutCharScreen(ConsoleBuf[pos + xx]);
                }
                BackupConsoleBuf[pos + xx] = ConsoleBuf[pos + xx];
            }
        }
    }
    // Restore Original X,Y
    X = Xb;
    Y = Yb;
    AG_MutexUnlock(&mutex);
}

int  DumpObject::SizeAlloc(AG_SizeAlloc *a)
{
    int w,h;
    if(a == NULL) return -1;
    w = a->w / CHRW;
    h = a->h / CHRH;
    if((w <= 0) || (h <= 0)) return -1;
    InitConsole(w, h);
    return 0;
}

int DumpObject::PutString(char *str)
{
    int len;
    int cp = 0;
    if(str == NULL) return -1;
    len = strlen(str);
    if(len <= 0) return -1;
    do {
        PutChar(str[cp]);
        cp++;
    } while(cp < len);
    return len;
}

void DumpObject::DrawCursor(BOOL Flag)
{
    AG_Rect rec;
    AG_Color col;
    AG_Color fg,bg;
    AG_Surface *s;


    rec.h = CHRH;
    rec.w = CHRW;
    rec.x = CHRW * X;
    rec.y = CHRH * Y;
    col = fgColor;
    col.a = 64;
    if(Flag){
        AG_MutexLock(&mutex);
        fg = fgColor;
        bg = bgColor;
        fgColor = bg;
        bgColor = fg;
        PutCharScreen(ConsoleBuf[Y * W + X]);
        AG_FillRect(Screen, &rec, col);
        fgColor = fg;
        bgColor = bg;
        AG_MutexUnlock(&mutex);
    }
}

void DumpObject::PollDraw(void)
{
    Draw(FALSE);
    DrawCursor(TRUE);
}

int DumpObject::GetWidth(void)
{
   return W;
}

int DumpObject::GetHeight(void)
{
   return H;
}

int DumpObject::GetX(void)
{
   return X;
}

int DumpObject::GetY(void)
{
   return Y;
}

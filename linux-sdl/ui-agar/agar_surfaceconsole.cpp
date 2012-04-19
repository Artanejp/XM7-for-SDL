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
    fgColor.r = 255;
    fgColor.g = 255;
    fgColor.b = 255;
    fgColor.a = 255;
    bgColor.r = 0;
    bgColor.g = 0;
    bgColor.b = 0;
    bgColor.a = 255;

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
    TextFont = AG_TextFontLookup("F-Font_400line.ttf", 16, 0);
    if(TextFont == NULL) { // Fallback
        TextFont = AG_TextFontPts(16); //  16pts
    }
    AG_PopTextState();

    AG_PushTextState();
    SymFont = AG_TextFontLookup("F-Font_Symbol_Unicode.ttf", 16, 0);
    if(SymFont == NULL) { // Fallback
        SymFont = AG_TextFontPts(16);//  16pts
    }
    AG_PopTextState();

    AG_PushTextState();
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
    if(Screen != NULL) {
       Screen = NULL;
       AG_SurfaceFree(Screen);
    }

    ConsoleBuf = new BYTE[size];
    BackupConsoleBuf = new BYTE[size];
    for(yy = 0; yy < H; yy++){
        for(xx = 0; xx < W; xx++){
            ConsoleBuf[xx + yy * W] = 0x00000000;
            BackupConsoleBuf[xx + yy * W] = 0x00000000;
        }
    }
    SetPixelFormat(&fmt);
    Screen = AG_SurfaceNew(AG_SURFACE_PACKED, W * CHRW, H * CHRH, &fmt, AG_SRCALPHA);
    {
        AG_Color col;
        AG_Rect rec;
        col.r = 0;
        col.g = 0;
        col.b = 0;
        col.a = 255;
        rec.x = 0;
        rec.y = 0;
        rec.w = Screen->w;
        rec.h = Screen->h;
        AG_FillRect(Screen, &rec, col);
    }
    AG_MutexUnlock(&mutex);
}

void DumpObject::MoveCursor(int x, int y)
{
    AG_MutexLock(&mutex);
    X = x;
    Y = y;
    if(CURX >= W) CURX = W - 1;
    if(CURY >= H) CURY = H - 1;
    if(CURX <= 0) CURX = 0;
    if(CURY <= 0) CURY = 0;
    AG_MutexUnlock(&mutex);
}

void DumpObject::MoveDrawPos(int x, int y)
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

// F-Font_Symbol_400line.ttf はコードスペースの問題でレンダリングされないので、
// Unicode(ISO10646-1)に直したのを使う必要がある(F-Font_Symbol_400line_Unicode.ttfなど)

static Uint32 SymFontList80[] =
{
   // 0080-
   0x00c4,0x00c5,0x00c7,0x00c9,0x00d1,0x00d6,0x00dc,0x00e1,
   0x00e0,0x00e2,0x00e4,0x00e3,0x00e5,0x00e7,0x00e9,0x00e8,
   // 0090-
   0x00ea,0x00eb,0x00ed,0x00ec,0x00ee,0x00ef,0x00f1,0x00f3,
   0x00f2,0x00f4,0x00f6,0x00f5,0x00fa,0x00f9,0x00fb,0x00fc,
   // 00a0
   0x2020
};

static Uint32 SymFontListE0[] =
{
   // 00e0-
   0x2021,0x00b7,0x201a,0x201e,0x2030,0x00c2,0x00ca,0x00c1,
   0x00cb,0x00c8,0x00cd,0x00ce,0x00cf,0x00cc,0x00d3,0x00d4,
   // 00f0-
   0xe01e,0x00d2,0x00da,0x00db,0x00d9,0x0131,0x02c6,0x02dc,
   0x00af,0x02d8,0x02d9,0x02da,0x00b8,0x02dd,0x02db,0x02c7,
};



BOOL DumpObject::Sym2UCS4(BYTE b, Uint32 *disp)
{
   Uint32 ucs;
   disp[0] = '\0';
   if((b >= 0x80) && (b <= 0xa0)){
     ucs = SymFontList80[b - 0x80];
//     ucs = b;
  } else if((b >= 0xd0) && (b <= 0xff)) {  
     ucs = SymFontListE0[b - 0xe0];
  } else {
      return FALSE;
  }
   
   disp[0] = ucs;
   disp[1] = 0x00000000;
   return TRUE;
   
}

BOOL DumpObject::Txt2UCS4(BYTE b, Uint32 *disp)
{
   Uint32 ucs;
   if((b <= 0x7f) && ( b>= 0x20)) {
       disp[0] = b;
       disp[1] = '\0';
       return TRUE;
   } else if((b <= 0xdf) || (b >= 0xa1)) {
       disp[0] = 0xfec0 + (unsigned int)b;
       disp[1] = '\0';
        return TRUE;
   } else if(b < 0x20) {
	disp[0] = '.';
       disp[1] = '\0';
      return TRUE;
   }
   return FALSE;
}

void DumpObject::PutCharScreen(BYTE c)
{
    Uint32 ucs[2];
    AG_Surface *r = NULL;

    if(Sym2UCS4(c, ucs)) {
        if(SymFont != NULL){
            AG_PushTextState();
            AG_TextFont(SymFont);
            AG_TextColor(fgColor);
            AG_TextBGColor(bgColor);
            AG_TextValign(AG_TEXT_MIDDLE);
            r = AG_TextRenderUCS4((const Uint32 *)ucs);
            AG_PopTextState();
        }
    } else if(Txt2UCS4(c, ucs)) {
        if(TextFont != NULL){
            AG_PushTextState();
            AG_TextFont(TextFont);
            AG_TextColor(fgColor);
            AG_TextBGColor(bgColor);
            AG_TextValign(AG_TEXT_MIDDLE);
            r = AG_TextRenderUCS4((const Uint32 *)ucs);
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

    if(Screen == NULL) return;
    // Backup X,Y
    AG_MutexLock(&mutex);

    Xb = X;
    Yb = Y;
    AG_SurfaceLock(Screen);
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
    AG_SurfaceUnlock(Screen);

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
        X++;
        if((X >= W) || (X < 0)){
            X = 0;
            Y++;
            if((Y >= H) || (Y < 0)){
                Y = 0;
            }
        }
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
    rec.x = CHRW * CURX;
    rec.y = CHRH * CURY;
    col = fgColor;
    col.a = 64;
    if(Flag){
        AG_MutexLock(&mutex);
        fg = fgColor;
        bg = bgColor;
        fgColor = bg;
        bgColor = fg;
        PutCharScreen(ConsoleBuf[CURY * W + CURX]);
        AG_FillRect(Screen, &rec, col);
        fgColor = fg;
        bgColor = bg;
        AG_MutexUnlock(&mutex);
    }
}

void DumpObject::ClearScreen(void)
{
   int xx;
   int yy;
   int pos;
   AG_Rect rect;
  
   AG_MutexLock(&mutex);
      pos = 0;
      for(yy = 0; yy < H; yy++) {
	 for(xx = 0; xx < W; xx++) {
	    ConsoleBuf[pos + xx] = BackupConsoleBuf[pos + xx] = 0x00;
	 }
	 pos += W;
      }
   if(Screen != NULL) {
//      AG_ObjectLock(AGOBJECT(Screen));
      rect.w = Screen->w;
      rect.h = Screen->h;
      rect.x = 0;
      rect.y = 0;
      AG_FillRect(Screen, &rect, bgColor);
//      AG_ObjectUnlock(AGOBJECT(Screen));
   }
   AG_MutexUnlock(&mutex);
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

int DumpObject::GetCurX(void)
{
    return CURX;
}

int DumpObject::GetCurY(void)
{
    return CURY;
}


AG_Surface *DumpObject::GetScreen(void)
{
    return Screen;
}

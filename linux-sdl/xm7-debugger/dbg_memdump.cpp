/*
 * XM7/Debugger : MemoryDump
 */

#include <SDL/SDL.h>
#include "../vm/xm7.h"

#define FONTSIZE_X 8;
#define FONTSIZE_Y 8;


struct DBG_ANKFontData{
    int w;
    int h;
    Uint8 *buf;
};

/*
 * 1行読む
 * 正の数 : 文字数
 * 0: エラーもしくはEOF
 */
static int ReadLN(SDL_RWops *f, Uint8 *buf, int limit)
{
    int i;

    if(limit < 0) return 0;

    do {
        if(SDL_RWread(f, &buf[i], 1, 1) < 0) {
            return i;
        }
        if(buf[i] == '\n') {
            buf[i] = 0x00;
            break;
        }
        i++;
        if(i >= limit) break;
    } while(TRUE);
    return i;
}

static int chr2hex(Uint8 s)
{
    int i;
         switch(s){
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            i = s - '0';
            break;
         case 'A':
         case 'B':
         case 'C':
         case 'D':
         case 'E':
         case 'F':
            i = s - 'A' + 10;
            break;
         case 'a':
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'f':
            i = s - 'a' + 10;
            break;
         default:
            i = 0;
            break;
         }
    return i;
}

/*
 * ANKフォントを作成する。
 * 互換ROMのソースコードのfont.txtが必要
 * (ライセンス的な問題が解決したらBuiltinする)
 */
 static void mkankfontsub(SDL_RWops *f, struct DBG_ANKFontData *p, int chnum)
 {
     int x;
     int y;
     int addr = 0;
     int base;
     int seekPtr;
     int ll;
     Uint8 LineBuf[FONTSIZE_X + 2];
     Uint8 *data;
     int xdup;
     int ydup;
     int pitch;
     Uint8 *buf;
     int i,j;

     xdup = p->w / FONTSIZE_X;
     ydup = p->h / FONTSIZE_Y;
     buf = p->buf;

     seekPtr = SDL_RWtell(f); // Push Pointer
     ReadLN(f, LineBuf, FONTSIZE_X + 1);
     if((LineBuf[0] == '$') && (strlen(LineBuf) >= 3)) {
         // フォントポインタ
         addr = chr2hex(LineBuf[1]) * 16 + chr2hex(LineBuf[2]);
     } else {
         SDL_RWseek(f, seekPtr,SEEK_SET); // Pop Pointer
     }

     pitch = xdup * FONTSIZE_X / 8;
     base = addr * (FONTSIZE_Y * FONTSIZE_X / 8);
     for(y = 0; y < FONTSIZE_Y; y++){
         if(ReadLN(f, LineBuf, FONTSIZE_X + 1) <= 0) {
             return;
         }
         ll = strlen(LineBuf)>FONTSIZE_X?FONTSIZE_X:strlen(LineBuf);
         // とりあえず、8Dotまで
         j = 0;
         for(x = 0; x < ll; x++) {
            if(LineBuf[x] == '@'){
                for(i = 0; i < xdup; i++) {
                    buf[base + (j / 8)] |= 0x01;
                    buf[base + (j / 8)] <<= 1;
                    j++;
                }
            } else {
                for(i = 0; i < xdup; i++) {
                    buf[base + (j / 8)] &= 0x01;
                    buf[base + (j / 8)] <<= 1;
                    j++;
                }
            }
         }
         base += pitch;
     }
 }

struct DBG_ANKFontData *DBG_GenANKFont(SDL_RWops *f, int w, int h)
{
    struct DBG_ANKFontData *p;
    int i;

    p = (struct DBG_ANKFontData *)malloc(sizeof(struct DBG_ANKFontData));
    if(p == NULL) return NULL;
    if((w % 8) != 0) {
        p->w = w + 8 - (w % 8);
    } else {
        p->w = w;
    }
    p->h = h;
    p->buf = malloc((p->w * p->h * 256) / 8);
    if(p->buf == NULL) {
        free((void *)p);
        return NULL;
    }
    for(i = 0; i < 256; i++) {
        mkankfontsub(f, p, i);
    }
    return p;
}

void DBG_FreeANKFont(struct DBG_ANKFontData *p)
{
    free((void *)p);
}

/*
 * 一文字書きこむ
 * 32bpp前提
 */
 static inline void DBG_PutWord(Uint32 *p, int width,Uint8 *data, Uint32 pixel, Uint32 zeropixel)
 {
     Uint8 dd;
     Uint32 px;
     Uint32 zpx;
     int j;
     int i;
     int k;


     px = pixel;
     zpx = zeropixel;
     j = 0;
     for(k = 0; k < (width / 8); k++) {
         dd = data[k];
        for(i = 0; i < width ; i++) {
            if((dd & 0x80) == 1) {
                p[j] = px;
            } else {
                p[j] = zpx;
            }
            j += 1;
            dd <<= 1;
        }
     }
 }


void DBG_PutChar(Uint8 c, struct DBG_ANKFontData *font, SDL_Surface *buf, int x, int y, SDL_Color *col)
{
    int xx;
    int yy;
    int base;
    int w;
    int ofset;
    Uint8 *p;

    if(font == NULL) return;
    if(buf == NULL) return;
    base = (font->h * font->w * c) / 8;

    ofset = y * buf->pitch + x * buf->format->BytesPerPixel;
    for(yy = 0; yy < font->h; yy++) {
        DBG_PutWord(buf->pixels[ofset] , font->w, &(font->buf[base]), pixel, zeropixel);
        ofset += buf->pitch;
    }

}
/*
 * メモリをダンプしてSurfaceに表示する
 */
 void DBG_DumpMemory(volatile BYTE FASTCALL (*readb)(WORD), SDL_Surface *s, int x, int y, Uint16 addr, int chars, Uint8 *font)
 {

 }

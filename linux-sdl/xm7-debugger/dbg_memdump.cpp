/*
 * XM7/Debugger : MemoryDump
 */

#include <SDL/SDL.h>
#include <iconv.h>
#include "xm7.h"

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

void DBG_GetMem(volatile BYTE FASTCALL (*readb)(WORD), int bytes, WORD addr, Uint8 *buf)
{
    int i;
    WORD ad = addr;

    for(i = 0; i < bytes; i++) {
        buf[i] = readb(ad);
        ad++;
    }
}

static inline char hex2chr(Uint8 b)
{
    Uint8 bb = b & 0x0f;
    char c;

    if(bb < 10) {
        c = '0' + bb;
    } else {
        c = 'A' + (bb - 10);
    }
    return c;
}



void DBG_Bin2Hex1(char *str, Uint8 b)
{
    str[0] = hex2chr((b & 0xf0) >>4);
    str[1] = hex2chr(b & 0x0f);
    str[2] = '\0';
}

void DBG_Bin2Hex2(char *str, Uint16 w)
{
    str[0] = hex2chr((w & 0xf000) >>12);
    str[1] = hex2chr((w & 0x0f00) >>8);
    str[2] = hex2chr((w & 0x00f0) >>4);
    str[3] = hex2chr(w & 0x000f);
    str[4] = '\0';
}

void DBG_Bin2Hex4(char *str, Uint32 dw)
{
    str[0] = hex2chr((dw & 0xf0000000) >>28);
    str[1] = hex2chr((dw & 0x0f000000) >>24);
    str[2] = hex2chr((dw & 0x00f00000) >>20);
    str[3] = hex2chr((dw & 0x000f0000) >>16);

    str[4] = hex2chr((dw & 0x0000f000) >>12);
    str[5] = hex2chr((dw & 0x00000f00) >>8);
    str[6] = hex2chr((dw & 0x000000f0) >>4);
    str[7] = hex2chr(dw & 0x0000000f);
    str[8] = '\0';
}

void DBG_DumpAsc(char *str, Uint8 b)
{
   char c;
   int uc;

   if((b < 0x7f) && (b > 0x1f)) { // Alphabet
        str[0] = (char)b;
        str[1] = '\0';
  } else if((b <= 0xdf) && (b >= 0xa1)){
        uc = 0xfec0 + (int)b; // カナ： U+ff61 - U + ff9f
        str[0] = ((uc>>12) & 0x0f) | 0xe0;
        str[1] = ((uc>>6) & 0x3f) | 0x80;
        str[2] = (uc & 0x3f) | 0x80;
        str[3] = '\0';
    } else {
        str[0] = '.';
        str[1] = '\0';
    }
}

static int convertascii(Uint8 *buf, char *dst, int bytes)
{
    int i;
    int len;
    int uc;

    len = 0;
    for(i = 0; i < bytes; i++){
        if((buf[i] < 0x7f) && (buf[i] > 0x1f) && (buf[i] != 0x7f)){
            dst[len] = buf[i];
            len += 1;
        } else if((buf[i] <= 0xdf) && (buf[i] >= 0xa1)){ // Alphabet
            uc = 0xff00 + (int)buf[i] - 0x40; // カナ： U+ff61 - U + ff9f
            dst[len + 0] = ((uc>>12) & 0x0f) | 0xe0;
            dst[len + 1] = ((uc>>6) & 0x3f) | 0x80;
            dst[len + 2] = (uc & 0x3f) | 0x80;
            len += 3;
        } else {
            dst[len] = '.';
            len += 1;
        }
    }
    dst[len] = '\0';
    return len;
}

/*
 * Bufに格納したメモリの写しをダンプしてstrに表示する
 */
 void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD addr, int sum, int bytes, BOOL AddrFlag)
 {
    char cb[16];
    int i;

    if(AddrFlag) {
        DBG_Bin2Hex2(cb, addr);
        strcat(str, cb);
        strcat(str, " ");
    }
    for(i = 0; i < bytes; i++){
            DBG_Bin2Hex1(cb, buf[i]);
            cb[2] = ' ';
            cb[3] = '\0';
            strcat(str, cb);
    }
    // 20120318 Checksum(X)
    strcat(str, ":");
    DBG_Bin2Hex1(cb, sum & 0xff);
    cb[2] = ' ';
    cb[3] = '\0';
    strcat(str, cb);
    // Ascii Dump
    {
        char ibuf[1024];
        char *pIn;
        int len;
        pIn = ibuf;

        len = convertascii(buf, ibuf, bytes);
        ibuf[len] = '\0';
        strcat(str, ibuf);
    }
    strcat(str, "  ");
 }

void DBG_PrintYSum(char *str, int *sum, int totalSum, int width)
{
    char cb[5];
    int i;

    strcat(str, "SUM: ");
    for(i = 0; i < width; i++){
            DBG_Bin2Hex1(cb, sum[i] & 0xff);
            cb[2] = ' ';
            cb[3] = '\0';
            strcat(str, cb);
    }
    strcat(str, ":");
    DBG_Bin2Hex1(cb, totalSum & 0xff);
    cb[2] = '\0';
    strcat(str, cb);

}

/*
 * XM7/Debugger : MemoryDump
 */

#include <SDL/SDL.h>
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



/*
 * Bufに格納したメモリの写しをダンプしてstrに表示する
 */
 void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD Segment, WORD addr, int bytes, BOOL SegFlag, BOOL AddrFlag)
 {
    char cb[16];
    int i;

    if(SegFlag) {
        DBG_Bin2Hex2(cb, Segment);
        strcat(str, cb);
        strcat(str, ":");
    }
    if(AddrFlag) {
        DBG_Bin2Hex2(cb, addr);
        strcat(str, cb);
    }
    if(AddrFlag | SegFlag) {
        strcat(str, " ");
    }
    for(i = 0; i < bytes; i++){
            DBG_Bin2Hex1(cb, buf[i]);
            cb[2] = ' ';
            cb[3] = '\0';
            strcat(str, cb);
    }

 }

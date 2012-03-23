/*
* Debugger for XM7
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
* History:
* 21 Mar,2012 Initial
*/

#include "agar_surfaceconsole.h"

extern void DBG_PrintYSum(char *str, int *sum, int totalSum, int width);
extern void DBG_HexDumpMemory(char *str, Uint8 *buf, WORD addr, int sum, int bytes, BOOL AddrFlag);
extern void DBG_Bin2Hex1(char *str, Uint8 b);
extern void DBG_Bin2Hex2(char *str, Uint16 b);
extern void DBG_Bin2Hex4(char *str, Uint32 b);


void DumpMem2(DumpObject *obj, BYTE (* rb)(WORD), int addr)
{
    int a;
    int Xb,Yb,Hb,Wb;
    int i,j;
    int wd,hd;
    char *strbuf;
    BYTE *buf;
    unsigned int *ysum;
    unsigned int xsum;
    unsigned int sum;

    if(obj == NULL) return;
    if(rb == NULL) return;
    Xb = obj->GetX();
    Yb = obj->GetY();
    Hb = obj->GetHeight();
    Wb = obj->GetWidth();
    wd = 16;
    hd = 16;
    strbuf = new char [Wb + 4];
    obj->MoveCursor(0, 0);
    if(wd >= (Wb / 5)) wd = Wb / 5;
    if(hd >= (Hb - 4)) hd = Hb - 4;
    buf = new BYTE [wd + 2];
    ysum = new unsigned int [hd + 2];

    obj->PutString("ADDR  ");
    for(i = 0; i < wd; i++){
        sprintf(strbuf, "+%x", i);
        obj->PutString(strbuf);
    }
    obj->MoveCursor(0,1);
    for(i = 0; i < Wb - 2; i++){
        obj->PutChar('-');
    }
    sum = 0;
    for(i = 0;i < wd; i++){
        ysum[i] = 0;
    }

    for(i = 0;i < hd; i++){
        xsum = 0;
        for(j = 0; j < wd; j++) {
            buf[j] = rb((WORD)((addr + j) & 0xffff));
            sum += (unsigned int)buf[j];
            xsum += (unsigned int)buf[j];
        }
        obj->MoveCursor(0, 2 + i);
        DBG_HexDumpMemory(strbuf, buf, addr, xsum & 0xff, wd, TRUE);
    }
    // Footer
    obj->MoveCursor(0,3+i);
    for(i = 0; i < Wb - 2; i++){
        obj->PutChar('-');
    }
    // Sum
    obj->MoveCursor(0, 3+i);
    DBG_PrintYSum(strbuf, (int *)ysum, (int) (sum & 0xff), wd);
    obj->PutString(strbuf);

    delete [] buf;
    delete [] strbuf;
    delete [] ysum;
}

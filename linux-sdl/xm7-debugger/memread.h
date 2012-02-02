/*
* Memory access routines for DEBUGGER
* (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
*/

#ifndef MEMREAD_H_INCLUDED
#define MEMREAD_H_INCLUDED

#include "xm7.h"
#include "memdef.h"

extern BYTE rb_main(WORD addr);
extern void memread_main(BYTE *buf, DWORD start, DWORD bytes);
extern BYTE rb_sub(WORD addr);
extern void memread_sub(BYTE *buf, DWORD start, DWORD bytes);


#endif // MEMREAD_H_INCLUDED

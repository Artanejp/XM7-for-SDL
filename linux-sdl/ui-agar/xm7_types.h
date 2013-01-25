/*
 * xm7_types.h
 *  VarTypes for XM7/Agar
 * (C)2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * 
 */
#include <stdint.h>
#include <stddef.h>

#ifdef _WINDOWS
#include <windef.h>
#endif

#ifndef __XM7_TYPES_H
#define __XM7_TYPES_H 1
/*
 * 基本型定義 -> SDL定義にする(コンパイラ依存の吸収 20100802 α66)
 */

#ifndef _WINDOWS
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
#endif
typedef int     BOOL;

// Vector
#if defined(_X86INTRIN_H_INCLUDED) 
typedef uint16_t v4si __attribute__ ((__vector_size__(16), aligned(16)));
typedef uint16_t v8si __attribute__ ((__vector_size__(32), aligned(32)));
typedef union 
{
        v4si v;

        uint32_t i[4];
        uint16_t s[8];
        uint8_t  b[16];
        int32_t si[4];
        int16_t ss[8];
        int8_t  sb[16];
} v4hi;

typedef union 
{
        v8si v;
        
        uint32_t i[8];
        uint16_t s[16];
        uint8_t    b[32];
        int32_t si[8];
        int16_t ss[16];
        int8_t  sb[32];
} v8hi_t;

#else // Normal
typedef uint16_t v4si __attribute__ ((__vector_size__(16), aligned(16)));
typedef uint16_t v8si __attribute__ ((__vector_size__(32), aligned(32)));
typedef union 
{
        v4si v;

        uint32_t i[4];
        uint16_t s[8];
        uint8_t    b[16];
        int32_t si[4];
        int16_t ss[8];
        int8_t  sb[16];
} v4hi;

typedef union 
{
        v8si v;
        
        uint32_t i[8];
        uint16_t s[16];
        uint8_t  b[32];
        int32_t si[8];
        int16_t ss[16];
        int8_t  sb[32];
} v8hi_t;

#endif // !defined(_X86INTRIN_H_INCLUDED)
#endif //#ifndef __XM7_TYPES_H

/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ FM音源ユニット接続 ]
 */

#ifndef _cisc_h_
#define _cisc_h_

#include <math.h>
#include <string.h>

#ifndef assert
#define assert(exp)	(void(0))
#endif

#define LOG0(m)		void (0)
#define LOG1(m,a)	void (0)
#define LOG2(m,a,b)	void (0)
#define LOG3(m,a,b,c)	void (0)
#define LOG4(m,a,b,c,d)	void (0)
#define LOG5(m,a,b,c,d,e)	void (0)
#define LOG6(m,a,b,c,d,e,f)	void (0)
#define LOG7(m,a,b,c,d,e,f,g)	void (0)
#define LOG8(m,a,b,c,d,e,f,g,h)	void (0)
#define LOG9(m,a,b,c,d,e,f,g,h,i)	void (0)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef signed char sint8;
typedef signed short sint16;
typedef signed int sint32;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

inline int
Max(int x, int y)
{
    return (x > y) ? x : y;
}

inline int
Min(int x, int y)
{
    return (x < y) ? x : y;
}

inline int
Abs(int x)
{
    return x >= 0 ? x : -x;
}

inline int
Limit(int v, int max, int min)
{
    return v > max ? max : (v < min ? min : v);
}

#if !defined(FASTCALL)
#if defined(_WIN32) && (defined(__BORLANDC__) || (defined(_MSC_VER) && defined(_M_IX86)))
#define FASTCALL		__fastcall
#else
#define FASTCALL
#endif
#endif

#endif				/* _cisc_h_ */

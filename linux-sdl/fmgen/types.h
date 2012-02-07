#if !defined(__gcc_types_h)
#include <stdint.h>
#include <stddef.h>

# define __gcc_types_h

//  固定長型とか
typedef uint8_t uchar;
typedef uint16_t ushort;
typedef uint32_t uint;
//typedef uint32_t ulong;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t  uint32;

typedef int8_t sint8;
typedef int16_t sint16;
typedef int32_t sint32;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

// Vector方
typedef uint16_t v4si __attribute__ ((__vector_size__(16), aligned(16)));
typedef uint16_t v8si __attribute__ ((__vector_size__(32), aligned(16)));
typedef union 
{
        v4si v;
        uint32_t i[4];
        uint16_t s[8];
} v4hi;

typedef union 
{
        v8si v;
        uint32_t i[8];
        uint16_t s[16];
} v8hi;

#endif // __gcc_types_h

/*** m6809: Portable 6809 emulator ******************************************/

#pragma once

#ifndef __M6809_H__
#define __M6809_H__

//#include "cpuintrf.h"
#include <stdint.h>
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;




//#define LSB_FIRST

//#ifndef PAIR
typedef union
{
   
   #ifdef LSB_FIRST
           struct 
     {
	 UINT8 l,h,h2,h3; 
     }
    b;
           struct 
     {
	 UINT16 l,h; 
     }
    w;
           struct 
     {
	 INT8 l,h,h2,h3; 
     }
    sb;
           struct 
     {
	 INT16 l,h; 
     }
    sw;
   #else
           struct 
     {
	 UINT8 h3,h2,h,l; 
     }
    b;
           struct 
     {
	 INT8 h3,h2,h,l; 
     }
    sb;
           struct 
     {
	 UINT16 h,l; 
     }
    w;
           struct 
     {
	 INT16 h,l; 
     }
    sw;
   #endif
           UINT32 d;
           INT32 sd;
}
 PAIR;
//#endif

/* CPU interface functions */
//#define CPU_GET_INFO_NAME(name)                 cpu_get_info_##name
#define CPU_GET_INFO(name)                      void CPU_GET_INFO_NAME(name)(const device_config *device, UINT32 state, cpuinfo *info)
//#define CPU_GET_INFO_CALL(name)                 CPU_GET_INFO_NAME(name)(device, state, info)



enum
{
	M6809_PC=1, M6809_S, M6809_CC ,M6809_A, M6809_B, M6809_U, M6809_X, M6809_Y,
	M6809_DP
};

#define M6809_IRQ_LINE	0	/* IRQ line number */
#define M6809_FIRQ_LINE 1   /* FIRQ line number */

CPU_GET_INFO( m6809 );
#define CPU_M6809 CPU_GET_INFO_NAME( m6809 )

/* M6809e has LIC line to indicate opcode/data fetch */
CPU_GET_INFO( m6809e );
#define CPU_M6809E CPU_GET_INFO_NAME( m6809e )


CPU_DISASSEMBLE( m6809 );

typedef struct _m6809_config m6809_config;
struct _m6809_config
{
	UINT8	encrypt_only_first_byte;		/* encrypt only the first byte in 10 xx and 11 xx opcodes */
};

   

#endif /* __M6809_H__ */

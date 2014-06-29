/*
 * vm_sub: C記述
 * (C) 2010 K.Ohta
 * 20120624 Rebuild to simd
 */
#include <string.h>
#include <SDL/SDL.h>
#include "xm7_types.h"
/*
 *
 */

void memcpy400l(void *dest, void *src, int count)
{
    int             i;
//    uint16_t         *d = (uint16_t *) dest;
//    uint16_t         *s = (uint16_t *) src;
//    for(i = 0; i < count / 2; i++) {
//        d[i] = s[i];
//    }
     uint8_t *d = (uint8_t *)dest;
     uint8_t *s = (uint8_t *)src;
     count >>= 1;
     while(count){
	*d = *s;
	d += 2;
	s += 2;
	count--;
     }
   
}

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
    int            c1 = count / 16;
    int            c2;
    v4si         *d = (v4si *) dest;
    v4si         *s = (v4si *) src;

    c2 = (count - c1) & 0xfffffffe;
    for(i = 0; i < c1; i++){
        *d++ = *s++;
    }
    if(c2 != 0){
        uint16_t *src = (uint16_t *)s;
        uint16_t *dst = (uint16_t *)d;
        for(i = 0; i < c2; i+= 2){
            *dst++ = *src++;
        }
    }
}

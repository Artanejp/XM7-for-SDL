/*
 * vm_sub: C記述
 * (C) 2010 K.Ohta
 */
#include <string.h>
#include <SDL/SDL.h>

/*
 *
 */

void memcpy400l(void *dest, void *src, int count)
{
  int c = count;
  Uint16 *d = (Uint16 *)dest;
  Uint16 *s = (Uint16 *)src;

  for(c = 0; c< count/2 ; c++)
    {
      d[c] = s[c];
    } 
}

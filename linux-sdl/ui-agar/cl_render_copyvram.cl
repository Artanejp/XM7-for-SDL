//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0

__kernel void CopyVram(__global uchar *to, __global uchar *from, int size, int multithread)
{
  int t;
  int gid;
//  int lid;
  int pbegin;
  int pb1;
  int ww;
  int i;
  int l_mod, r_mod;
  int ww2, ww3;
  __global uint8 *p32, *q32;
  __global uchar *p, *q;
  
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      //lid = get_local_id(0);
      pbegin = (gid * size) / t; 
      pb1 = ((gid + 1) * size) / t;
      if(pb1 > size) {
         ww = size - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
  } else {
      pbegin = 0;
      ww = size;
      gid = 0;
  }
  p = &(from[pbegin]);
  q = &(to[pbegin]);

  l_mod = (32 - pbegin % 32) % 32;
  
  ww2 = ww - l_mod;
  ww3 = ww2 / 32;
  r_mod = ww2 - ww3 * 32;
  
  for(i = 0; i < l_mod; i++) *q++ = *p++;

  p32 = (__global uint8 *)p;
  q32 = (__global uint8 *)q;
  for(i = 0; i < ww3; i++) *q32++ = *p32++;

  p = (__global uchar *)p32;
  q = (__global uchar *)q32;
  for(i = 0; i < r_mod; i++) *q++ = *p++;
}

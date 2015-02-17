//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0

__kernel void CreateTable(__global ushort8 *table, int pages)
{
  int i;
  int j;
  ushort8 v;
  for(j = 0; j < 256; j++) {
     v.s0 = (j & 0x80) >> 7;
     v.s1 = (j & 0x40) >> 6;
     v.s2 = (j & 0x20) >> 5;
     v.s3 = (j & 0x10) >> 4;
     v.s4 = (j & 0x08) >> 3;
     v.s5 = (j & 0x04) >> 2;
     v.s6 = (j & 0x02) >> 1;
     v.s7 = j & 0x01;
     for(i = 0; i < pages; i++) {
       table[i * 256 + j] = v;
       v = v * (ushort8){2, 2, 2, 2, 2, 2, 2, 2};
     }
  }
}


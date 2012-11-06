//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0

  
uint8 putpixel(uint8 n, 
               uint rmask, uint gmask, uint bmask,
	       uint rshift, uint gshift, uint bshift)
{
  uint8 rbuf;
  uint8 gbuf;
  uint8 bbuf;
  uint8 abuf;
  uint8 ret;
  uint8 rmask8;
  uint8 gmask8;
  uint8 bmask8;
  uint n1;
  
  abuf = (uint8){0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff};
  rmask8 = (uint8){rmask, rmask, rmask, rmask, rmask, rmask, rmask, rmask};
  gmask8 = (uint8){gmask, gmask, gmask, gmask, gmask, gmask, gmask, gmask};
  bmask8 = (uint8){bmask, bmask, bmask, bmask, bmask, bmask, bmask, bmask};
  rbuf = (n & rmask8) >> rshift;
  gbuf = (n & gmask8) >> gshift;
  bbuf = (n & bmask8) >> bshift;
  ret = (rbuf << 0) + (gbuf << 16) + (bbuf << 8) + (abuf << 24); 
  return ret;
}

__kernel void getvram8(__global uchar *src, int w, int h, __global uchar4 *out,
                       __global uint *pal, __global uint *table)
{
  int ofset = 640 * 200 / 8;
  int x;
  int y;
  int ybegin = 0;
  int yend;
  int xbegin = 0;
  int xend;
  int hh;
  int ww;
  int xx;
  int t;
  int gid;
  uint addr;
  uint addr2;
  uchar rc,bc,gc;
  uint8 av;
  uint8 palette;
  __global uint8 *tbl8;
  uint8 c8;
  __global uint8 *p8;
  uint rmask, gmask, bmask;
  uint rshift, gshift, bshift;

  rmask = 0x000000ff;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  rshift = 0;
  gshift = 16;
  bshift = 8;
  
  t = get_global_size(0);
  gid = get_global_id(0);
  
  
  ww = w >> 3;
  ybegin = (gid * h) / t; 
  hh = ((gid + 1) * h) / t;
  if(hh > h) {
     hh = h - ybegin;
  } else {
     hh = hh - ybegin;
  }

  palette.s0 = pal[0];
  palette.s1 = pal[1];
  palette.s2 = pal[2];
  palette.s3 = pal[3];
  palette.s4 = pal[4];
  palette.s5 = pal[5];
  palette.s6 = pal[6];
  palette.s7 = pal[7];
  
  yend = ybegin + hh;
  xend = xbegin + ww;
  if(h > 200) ofset = 640 * 400 / 8;

  tbl8 = (__global uint8 *)table;
  
  for(y = ybegin; y < yend; y++) {
     addr = y * ww + xbegin; 
     addr2 = addr << 3;
     //p = (__global uchar4 *)(&(out[addr2]));
     p8 = (__global uint8 *)(&(out[addr2]));
     for(x = xbegin; x < xend; x++) {
        bc = src[addr];
	rc = src[addr + ofset];
	gc = src[addr + ofset + ofset];
        c8 = tbl8[bc] | tbl8[rc + 256] | tbl8[gc + 256 * 2];
	c8 &= (uint8){0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};
	av = shuffle(palette, c8);
	*p8 = putpixel(av, rmask, gmask, bmask, rshift, gshift, bshift);
	p8++;
        addr++;
	}
    }
}	
	

__kernel void getvram4096(__global uchar *src, int w, int h, 
                          __global uchar4 *out, __global uint *pal,
			  __global uint *table)
{
  int ofset = (4 * 320 * 200)  / 8;
  int x;
  int y;
  int ybegin = 0;
  int yend;
  int xbegin = 0;
  int xend;
  int hh;
  int ww;
  int t;
  int gid;
  uint addr;
  uint addr2;
  uint r0, r1, r2, r3;
  uint b0, b1, b2, b3;
  uint g0, g1, g2, g3;
  
  uint8 r8, g8, b8;
  __global uchar *r, *g, *b;
  uint8 av;
  uint8 cv;
  __global uint8 *p8;
  __global uint8 *tbl8;
  uint rmask, gmask, bmask;
  uint rshift, gshift, bshift;

  rmask = 0x000000ff;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  rshift = 0;
  gshift = 16;
  bshift = 8;
  tbl8 = (__global uint8 *)table;


  t = get_global_size(0);
  gid = get_global_id(0);

  
  ww = w >> 3;
  ybegin = (gid * h) / t; 
  hh = ((gid + 1) * h) / t;
  if(hh > h) {
     hh = h - ybegin;
  } else {
     hh = hh - ybegin;
  }



  yend = ybegin + hh;

  xend = xbegin + ww;
  for(y = ybegin; y < yend; y++) {
     addr = y * ww + xbegin; 
     addr2 = addr << 3;
     p8 = (__global uint8 *)(&(out[addr2]));
     for(x = xbegin; x < xend; x++) {
        b = &src[addr];
	r = &src[addr + ofset];
	g = &src[addr + ofset + ofset];
	b3 = (uint)(b[0    ]) + 0x300;
	b2 = (uint)(b[8000 ]) + 0x200;
	b1 = (uint)(b[16000]) + 0x100;
	b0 = (uint)(b[24000]) + 0x000;
	
	b8 =  tbl8[b0];
	b8 |= tbl8[b1];
	b8 |= tbl8[b2];
	b8 |= tbl8[b3];


	r3 = (uint)(r[0    ]) + 0x700;
	r2 = (uint)(r[8000 ]) + 0x600;
	r1 = (uint)(r[16000]) + 0x500;
	r0 = (uint)(r[24000]) + 0x400;
	
	r8 =  tbl8[r0];
	r8 |= tbl8[r1];
	r8 |= tbl8[r2];
	r8 |= tbl8[r3];

	
	g3 = (uint)(g[0    ]) + 0xb00;
	g2 = (uint)(g[8000 ]) + 0xa00;
	g1 = (uint)(g[16000]) + 0x900;
	g0 = (uint)(g[24000]) + 0x800;
	
	g8 =  tbl8[g0];
	g8 |= tbl8[g1];
	g8 |= tbl8[g2];
	g8 |= tbl8[g3];

	
	cv = (b8 | r8 | g8) & (uint8){0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff};
	av.s0 = pal[cv.s0];
	av.s1 = pal[cv.s1];
	av.s2 = pal[cv.s2];
	av.s3 = pal[cv.s3];
	av.s4 = pal[cv.s4];
	av.s5 = pal[cv.s5];
	av.s6 = pal[cv.s6];
	av.s7 = pal[cv.s7];
	
	*p8 = putpixel(av, rmask, gmask, bmask, rshift, gshift, bshift);
	p8++;
        addr++;
	}
    }
}	
	

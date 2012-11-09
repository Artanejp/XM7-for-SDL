//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0


#if __ENDIAN_LITTLE__
#define  rmask 0x000000ff
#define  gmask 0x00ff0000
#define  bmask 0x0000ff00
#define  amask 0xff000000
#define  rshift 0
#define  gshift 16
#define  bshift 8
#define  ashift 24
#else
#define  amask 0x000000ff
#define  bmask 0x00ff0000
#define  gmask 0x0000ff00
#define  rmask 0xff000000
#define  rshift 24
#define  gshift 8
#define  bshift 16
#define  ashift 0
#endif

inline uint8 putpixel(uint8 n, uint8 abuf)
{
  uint8 ret;
  

//  rmask8 = (uint8){rmask, rmask, rmask, rmask, rmask, rmask, rmask, rmask};
//  gmask8 = (uint8){gmask, gmask, gmask, gmask, gmask, gmask, gmask, gmask};
//  bmask8 = (uint8){bmask, bmask, bmask, bmask, bmask, bmask, bmask, bmask};
//  rbuf = (n & rmask8);
//  gbuf = (n & gmask8);
//  bbuf = (n & bmask8);
//  ret = (rbuf << rshift)  + (gbuf << gshift) + (bbuf << bshift) + (abuf << ashift); 
  ret = n | abuf;
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
  uint8 abuf = (uint8){amask, amask, amask, amask, amask, amask, amask, amask};
  uint8 palette;
  __global uint8 *tbl8;
  uint8 c8;
  __global uint8 *p8;

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
	*p8 = putpixel(av, abuf);
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
  uint8 abuf = (uint8){amask, amask, amask, amask, amask, amask, amask, amask};

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
	
	b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3];


	r3 = (uint)(r[0    ]) + 0x700;
	r2 = (uint)(r[8000 ]) + 0x600;
	r1 = (uint)(r[16000]) + 0x500;
	r0 = (uint)(r[24000]) + 0x400;
	
	r8 =  tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3];

	
	g3 = (uint)(g[0    ]) + 0xb00;
	g2 = (uint)(g[8000 ]) + 0xa00;
	g1 = (uint)(g[16000]) + 0x900;
	g0 = (uint)(g[24000]) + 0x800;
	
	g8 =  tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3];

	
	cv = (b8 | r8 | g8) & (uint8){0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff};
	av.s0 = pal[cv.s0];
	av.s1 = pal[cv.s1];
	av.s2 = pal[cv.s2];
	av.s3 = pal[cv.s3];
	av.s4 = pal[cv.s4];
	av.s5 = pal[cv.s5];
	av.s6 = pal[cv.s6];
	av.s7 = pal[cv.s7];
	
	*p8 = putpixel(av, abuf);
	p8++;
        addr++;
	}
    }
}	
	
__kernel void getvram256k(__global uchar *src, int w, int h, 
                          __global uchar4 *out, __global uint *pal,
			  __global uint *table, uint mpage)
{
  int ofset = (6 * 320 * 200)  / 8;
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
  uint r0, r1, r2, r3, r4, r5;
  uint b0, b1, b2, b3, b4, b5;
  uint g0, g1, g2, g3, g4, g5;
  uint8 r8, g8, b8;
  __global uchar *r, *g, *b;
  uint8 av;
  uint8 cv;
  __global uint8 *p8;
  __global uint8 *tbl8;
  uint8 abuf = (uint8){amask, amask, amask, amask, amask, amask, amask, amask};
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
	
	if(!(mpage & 0x10)) {
	    b5 = (uint)(b[0    ]) + 0x500;
	    b4 = (uint)(b[8000 ]) + 0x400;
	    b3 = (uint)(b[16000]) + 0x300;
	    b2 = (uint)(b[24000]) + 0x200;
	    b1 = (uint)(b[32000]) + 0x100;
	    b0 = (uint)(b[40000]) + 0x000;
	    b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3] | tbl8[b4] | tbl8[b5];
	    b8 <<= 2; // 6bit -> 8bit
	} else {
	    //b5 = b4 = b3 = b2 = b1 = b0 = 0;
	    b8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
	}
	if(!(mpage & 0x20)) {
	    r5 = (uint)(r[0    ]) + 0xb00;
	    r4 = (uint)(r[8000 ]) + 0xa00;
	    r3 = (uint)(r[16000]) + 0x900;
	    r2 = (uint)(r[24000]) + 0x800;
	    r1 = (uint)(r[32000]) + 0x700;
	    r0 = (uint)(r[40000]) + 0x600;
	    r8 =  tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3] | tbl8[r4] | tbl8[r5];
	    r8 <<= 4;
	} else {
	    //b5 = b4 = b3 = b2 = b1 = b0 = 0;
	    r8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
	}

	if(!(mpage & 0x40)) {
	    g5 = (uint)(g[0    ]) + 0x1100;
	    g4 = (uint)(g[8000 ]) + 0x1000;
	    g3 = (uint)(g[16000]) + 0x0f00;
	    g2 = (uint)(g[24000]) + 0x0e00;
	    g1 = (uint)(g[32000]) + 0x0d00;
	    g0 = (uint)(g[40000]) + 0x0c00;
	    g8 =  tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3] | tbl8[g4] | tbl8[g5];
	    g8 <<= 6;
	} else {
	    //b5 = b4 = b3 = b2 = b1 = b0 = 0;
	    g8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
	}
	

	
	cv = b8 | r8 | g8 ;
	*p8 = putpixel(cv, abuf);
	p8++;
        addr++;
	}
    }
}	
	

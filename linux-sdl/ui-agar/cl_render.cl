//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0


//#if __ENDIAN_LITTLE__
#define  rmask 0x000000ff
#define  gmask 0x00ff0000
#define  bmask 0x0000ff00
#define  amask 0xff000000
#define  rshift 0
#define  gshift 16
#define  bshift 8
#define  ashift 24
//#else
//#define  amask 0x000000ff
//#define  bmask 0x00ff0000
//#define  gmask 0x0000ff00
//#define  rmask 0xff000000
//#define  rshift 24
//#define  gshift 8
//#define  bshift 16
//#define  ashift 0
//#endif

inline uint8 putpixel(uint8 n, uint8 abuf)
{
  uint8 ret;
  ret = n | abuf;
  return ret;
}

// Not need alpha.
inline uint8 putpixel2(uint8 n)
{
  uint8 ret;
  ret = n;
  return ret;
}

__kernel void getvram8(__global uchar *src, int w, int h, __global uchar4 *out,
                       __global uint *pal, __global uint *table,
		       int multithread, int dummy, int need_alpha)
{
  int ofset = 0x4000;
  int x;
  int y;
  int hh;
  int ww;
  int xx;
  int t;
  int gid;
  uint addr;
  uint addr2;
  uchar rc,bc,gc;
  uint8 av;
  const uint8 abuf = (uint8){amask, amask, amask, amask, amask, amask, amask, amask};
  uint8 palette;
  __global uint8 *tbl8;
  uint8 c8;
  __global uint8 *p8;
  uint pb1, pbegin, col;
  __global uchar *src_r;
  __global uchar *src_g;
  __global uchar *src_b;

  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      col = ww * h;
      pbegin = (gid * col) / t; 
      pb1 = ((gid + 1) * col) / t;
      if(pb1 > col) {
         ww = col - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
      addr = pbegin; 
      addr2 = pbegin << 3;
  } else {
      addr = addr2 = 0;
      ww = ww * h;
  }
  
  p8 = (__global uint8 *)(&(out[addr2]));

  palette = (uint8){pal[0], pal[1], pal[2], pal[3],
                    pal[4], pal[5], pal[6], pal[7]};  
//  palette.s0 = pal[0];
//  palette.s1 = pal[1];
//  palette.s2 = pal[2];
//  palette.s3 = pal[3];
//  palette.s4 = pal[4];
//  palette.s5 = pal[5];
//  palette.s6 = pal[6];
//  palette.s7 = pal[7];
  
  if(h > 200) ofset = 0x8000;

  tbl8 = (__global uint8 *)table;
  //p = (__global uchar4 *)(&(out[addr2]));
  p8 = (__global uint8 *)(&(out[addr2]));
  src_r = &src[addr + ofset];
  src_g = &src[addr + ofset + ofset];
  src_b = &src[addr];
  
  //prefetch(tbl8, 256 * 3);
  //prefetch(src_r, ww);
  //prefetch(src_g, ww);
  //prefetch(src_b, ww);
  for(x = 0; x < ww; x++) {
        bc = *src_b;
	rc = *src_r;
	gc = *src_g;
        c8 = tbl8[bc] | tbl8[rc + 256] | tbl8[gc + 256 * 2];
#if 0
        av = shuffle(palette, c8);
#else
	c8 &= (uint8){0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};
	av.s0 = pal[c8.s0];
	av.s1 = pal[c8.s1];
	av.s2 = pal[c8.s2];
	av.s3 = pal[c8.s3];
	av.s4 = pal[c8.s4];
	av.s5 = pal[c8.s5];
	av.s6 = pal[c8.s6];
	av.s7 = pal[c8.s7];
#endif  
//	if(need_alpha != 0) {
	   *p8 = putpixel(av, abuf);
//	} else {
//	   *p8 = putpixel2(av);
//	}
	p8++;
        src_r++;
        src_g++;
        src_b++;
	}
}	
	

__kernel void getvram4096(__global uchar *src, int w, int h, 
                          __global uchar4 *out, __global uint *pal,
			  __global uint *table,
			  int multithread, int dummy, int need_alpha)
{
  int ofset = 0x8000;
  int x;
  int y;
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
  uint pb1, pbegin, col;
  const uint8 abuf = (uint8){amask, amask, amask, amask, amask, amask, amask, amask};

  tbl8 = (__global uint8 *)table;



  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      col = ww * h;
      pbegin = (gid * col) / t; 
      pb1 = ((gid + 1) * col) / t;
      if(pb1 > col) {
         ww = col - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
      addr = pbegin; 
      addr2 = pbegin << 3;
  } else {
      addr = addr2 = 0;
      ww = ww * h;
  }
  
  p8 = (__global uint8 *)(&(out[addr2]));
  src = &src[addr];
  //prefetch(&src[0], ww);
  //prefetch(&src[0x2000], ww);
  //prefetch(&src[0x4000], ww);
  //prefetch(&src[0x6000], ww);
  //prefetch(&src[0      + ofset], ww);
  //prefetch(&src[0x2000 + ofset], ww);
  //prefetch(&src[0x4000 + ofset], ww);
  //prefetch(&src[0x6000 + ofset], ww);
  //prefetch(&src[0      + ofset << 1], ww);
  //prefetch(&src[0x2000 + ofset << 1], ww);
  //prefetch(&src[0x4000 + ofset << 1], ww);
  //prefetch(&src[0x6000 + ofset << 1], ww);
  //prefetch(pal, 4096);
  //prefetch(tbl8, 0xc00);
  for(x = 0; x < ww; x++) {
        b = &src[0];
	r = &src[ofset];
	g = &src[ofset << 1];
	b3 = (uint)(b[0x0    ]) + 0x300;
	b2 = (uint)(b[0x2000]) + 0x200;
	b1 = (uint)(b[0x4000]) + 0x100;
	b0 = (uint)(b[0x6000]) + 0x000;
	
	b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3];

	r3 = (uint)(r[0x0    ]) + 0x700;
	r2 = (uint)(r[0x2000]) + 0x600;
	r1 = (uint)(r[0x4000]) + 0x500;
	r0 = (uint)(r[0x6000]) + 0x400;
	
	r8 =  tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3];

	
	g3 = (uint)(g[0x0     ]) + 0xb00;
	g2 = (uint)(g[0x2000]) + 0xa00;
	g1 = (uint)(g[0x4000]) + 0x900;
	g0 = (uint)(g[0x6000]) + 0x800;
	
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
	
//	if(need_alpha != 0) {
	   *p8 = putpixel(av, abuf);
//	} else {
//	   *p8 = putpixel2(av);
//	}
	p8++;
        src++;
	}
}	
	
__kernel void getvram256k(__global uchar *src, int w, int h, 
                          __global uchar4 *out, __global uint *pal,
			  __global uint *table, uint mpage,
			  int multithread, int need_alpha)
{
  int ofset = 0xc000;
  int x;
  int y;
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
  const uint8 abuf = (uint8){amask, amask, amask, amask, amask, amask, amask, amask};
  tbl8 = (__global uint8 *)table;
  uint col;
  uint pbegin, pb1;


  
  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      col = ww * h;
      pbegin = (gid * col) / t; 
      pb1 = ((gid + 1) * col) / t;
      if(pb1 >= col) {
         ww = col - pbegin;
      } else {
         ww = pb1 - pbegin;
      }
      if(ww <= 0) return;
      addr = pbegin; 
      addr2 = pbegin << 3;
  } else {
      addr = addr2 = 0;
      ww = ww * h;
  }
  
  p8 = (__global uint8 *)(&(out[addr2]));
//  src = &src[addr];
  //prefetch(&src[0], ww);
  //prefetch(&src[0x2000], ww);
  //prefetch(&src[0x4000], ww);
  //prefetch(&src[0x6000], ww);
  //prefetch(&src[0x8000], ww);
  //prefetch(&src[0xa000], ww);
  //prefetch(&src[0      + ofset], ww);
  //prefetch(&src[0x2000 + ofset], ww);
  //prefetch(&src[0x4000 + ofset], ww);
  //prefetch(&src[0x6000 + ofset], ww);
  //prefetch(&src[0x8000 + ofset], ww);
  //prefetch(&src[0xa000 + ofset], ww);
  //prefetch(&src[0      + ofset << 1], ww);
  //prefetch(&src[0x2000 + ofset << 1], ww);
  //prefetch(&src[0x4000 + ofset << 1], ww);
  //prefetch(&src[0x6000 + ofset << 1], ww);
  //prefetch(&src[0x8000 + ofset << 1], ww);
  //prefetch(&src[0xa000 + ofset << 1], ww);
  //prefetch(tbl8, 0x500);
  for(x = 0; x < ww; x++) {
        b = &src[addr];
	r = &src[addr + ofset];
	g = &src[addr + ofset + ofset];
	
	if(!(mpage & 0x10)) {
	    b5 = (uint)(b[0     ]) + 0x500;
	    b4 = (uint)(b[0x2000]) + 0x400;
	    b3 = (uint)(b[0x4000]) + 0x300;
	    b2 = (uint)(b[0x6000]) + 0x200;
	    b1 = (uint)(b[0x8000]) + 0x100;
	    b0 = (uint)(b[0xa000]) + 0x000;
	    b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3] | tbl8[b4] | tbl8[b5];
//#if __ENDIAN_LITTLE__
            b8 <<= 18; // 6bit -> 8bit
//#else
//            b8 <<= 10; // 6bit -> 8bit
//#endif
	} else {
	    //b5 = b4 = b3 = b2 = b1 = b0 = 0;
	    b8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
	}
	if(!(mpage & 0x20)) {
	    r5 = (uint)(r[0     ]) + 0x500;
	    r4 = (uint)(r[0x2000]) + 0x400;
	    r3 = (uint)(r[0x4000]) + 0x300;
	    r2 = (uint)(r[0x6000]) + 0x200;
	    r1 = (uint)(r[0x8000]) + 0x100;
	    r0 = (uint)(r[0xa000]) + 0x000;
	    r8 =  tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3] | tbl8[r4] | tbl8[r5];
//#if __ENDIAN_LITTLE__
            r8 <<= 2; // 6bit -> 8bit
//#else
//            r8 <<= 26; // 6bit -> 8bit
//#endif
	} else {
	    //b5 = b4 = b3 = b2 = b1 = b0 = 0;
	    r8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
	}

	if(!(mpage & 0x40)) {
	    g5 = (uint)(g[0     ]) + 0x500;
	    g4 = (uint)(g[0x2000]) + 0x400;
	    g3 = (uint)(g[0x4000]) + 0x300;
	    g2 = (uint)(g[0x6000]) + 0x200;
	    g1 = (uint)(g[0x8000]) + 0x100;
	    g0 = (uint)(g[0xa000]) + 0x000;
	    g8 =  tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3] | tbl8[g4] | tbl8[g5];
//#if __ENDIAN_LITTLE__
            g8 <<= 10; // 6bit -> 8bit
//#else
//            g8 <<= 18; // 6bit -> 8bit
//#endif
	} else {
	    //b5 = b4 = b3 = b2 = b1 = b0 = 0;
	    g8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
	}
	cv = b8 | r8 | g8 ;
//	if(need_alpha != 0) {
	   *p8 = putpixel(cv, abuf);
//	} else {
//	   *p8 = putpixel2(cv);
//	}
	p8++;
        addr++;
	}
}	
	

//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0

#ifndef __ENDIAN_LITTLE__
#define __ENDIAN_LITTLE__ 1
#endif

#if (__ENDIAN_LITTLE__==1)
#define  rmask 0x000000ff
#define  gmask 0x0000ff00
#define  bmask 0x00ff0000
#define  amask 0xff000000
#define  rshift 0
#define  gshift 8
#define  bshift 16
#define  ashift 24
#else
#define  amask 0x000000ff
#define  gmask 0x00ff0000
#define  bmask 0x0000ff00
#define  rmask 0xff000000
#define  rshift 24
#define  gshift 16
#define  bshift 8
#define  ashift 0
#endif

inline uint8 putpixel(uint8 n, uint8 abuf)
{
  uint8 ret;
  ret = n | abuf;
  return ret;
}


   
uchar4 ttlpalet2rgb(__global uchar *pal, uint index)
{
    uchar4 ret = {0, 0, 0, 255};
    uchar dat = pal[index];
    if(dat & 0x01) ret.s2 = 255;
    if(dat & 0x02) ret.s0 = 255;
    if(dat & 0x04) ret.s1 = 255;
    return ret;
}


__kernel void CreateTable(__global uint8 *table, int pages)
{
  int i;
  int j;
  uint8 v;
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
       v <<= 1;
     }
  }
}


void setup_ttlpalette(__global uchar *pal, __local uint *palette, float4 bright, uint vpage)
{
   int i;
   uchar4 v;
   uint r, g, b, a;
   for(i = 0; i < 8; i++) {
      v = ttlpalet2rgb(pal, i & vpage);
      r = v.s0 * bright.s0;
      g = v.s1 * bright.s1;
      b = v.s2 * bright.s2;
      a = v.s3 * bright.s3;
      palette[i] = (r << rshift) |
      		   (g << gshift) |
		   (b << bshift) |
		   (a << ashift);
   }
}

void clearscreen(int w,  __global uint8 *out, float4 bright)
{
   int i;
   __global uint8 *p = out;
   uint a = 255 * bright.s3;
   uint8 abuf;

   a = (a & 255) << ashift;
   abuf = (uint8){a, a, a, a, a, a, a, a};

   for(i = 0; i < w; i++) {
      *p++ = abuf;
   }
}


__kernel void getvram8(__global uchar *src, int w, int h, __global uchar4 *out,
                       __global uchar *pal, __global uint8 *table,
		       int multithread, int crtflag, uint vpage, float4 bright)
{
  int ofset = 0x4000;
  int x;
  int ww;
  int t, q, rr;
  int gid;
  uint addr;
  uint addr2;
  uchar rc,bc,gc;
  uint8 av;
  __local uint mask;
  __local uint8 mask8;
  __local uint8 tbl8[3 * 256];
  uint8 c8;
  __global uint8 *p8;
  uint pb1, pbegin, col;
  __global uchar *src_r;
  __global uchar *src_g;
  __global uchar *src_b;
  __local uint palette[8];

  int lid = 0;
  int i;

  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      lid = get_local_id(0);
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
      gid = 0;
  }
  
  p8 = (__global uint8 *)(&(out[addr2]));

  if(crtflag == 0) {
    clearscreen(ww, p8, bright);
    //barrier(CLK_GLOBAL_MEM_FENCE);
    return;
  }

  if(h > 200) ofset = 0x8000;

  //tbl8 = (__global uint8 *)table;
  src_r = (__global uchar *)&src[addr + ofset];
  src_g = (__global uchar *)&src[addr + ofset + ofset];
  src_b = (__global uchar *)&src[addr];
  //prefetch(tbl8, 256 * 3);
  //prefetch(src_r, ww);
  //prefetch(src_g, ww);
  //prefetch(src_b, ww);

  if(lid == 0) {
     mask = 0x07 & vpage;
     setup_ttlpalette(pal, palette, bright, mask);
     mask8 = (uint8){mask, mask, mask, mask, mask, mask, mask, mask};
  }
  barrier(CLK_LOCAL_MEM_FENCE);

  t = (256 * 3) / get_local_size(0);
  q = t * lid;
  rr = q + t;
  if(q > (256 * 3)) q = 256 * 3;
  if(rr > (256 * 3)) rr = 256 * 3;
  for(i = q; i < rr; i++) tbl8[i] = table[i]; // Prefetch palette
  barrier(CLK_LOCAL_MEM_FENCE);

  for(x = 0; x < ww; x++) {
        bc = *src_b++;
	rc = *src_r++;
	gc = *src_g++;
        c8 = tbl8[bc] | tbl8[rc + 256] | tbl8[gc + 256 * 2];
	c8 &= mask8;
	av.s0 = palette[c8.s0];
	av.s1 = palette[c8.s1];
	av.s2 = palette[c8.s2];
	av.s3 = palette[c8.s3];
	av.s4 = palette[c8.s4];
	av.s5 = palette[c8.s5];
	av.s6 = palette[c8.s6];
	av.s7 = palette[c8.s7];
        *p8++ = av;
	}
//    barrier(CLK_GLOBAL_MEM_FENCE);
    return;
}	
	
inline uint get_apalette(__global uchar *pal, uint col, float4 bright)
{
   uint rc, gc, bc, ac;
   uint dat;
   rc = (uint)(pal[col + 4096] * bright.s0) << (rshift + 4);
   gc = (uint)(pal[col + 8192] * bright.s1) << (gshift + 4);
   bc = (uint)(pal[col] * bright.s2)        << (bshift + 4);
   ac = (uint)(255 * bright.s3) << ashift;
   dat = rc | gc | bc | ac;
   return dat;
}   

   

__kernel void getvram4096(__global uchar *src, int w, int h, 
                          __global uchar4 *out, __global uchar *pal,
			  __global uint8 *table, 
			  uint multithread, int crtflag, int mpage, float4 bright)
{
  int ofset = 0x8000;
  int x;
  int ww;
  int t, q, rr;
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
  __global uint8 *tbl8 = table;
  uint pb1, pbegin, col;
  __local uint8 mask8;
  __local uint mask;
  int lid = 0;
  int i;
  __local uint palette[4096];  

  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      gid = get_global_id(0);
      lid = get_local_id(0);
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
      gid = 0;
  }


  p8 = (__global uint8 *)(&(out[addr2]));
  src = &src[addr];
  if(crtflag == 0) {
     clearscreen(ww, p8, bright);
//     barrier(CLK_GLOBAL_MEM_FENCE);
     return;
  }



    if(lid == 0) {
      mask = 0;
      if(!(mpage & 0x10)) mask  = 0x000f;
      if(!(mpage & 0x20)) mask |= 0x00f0;
      if(!(mpage & 0x40)) mask |= 0x0f00;
      mask8 = (uint8){mask, mask, mask, mask, mask, mask, mask, mask};
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    t = 4096 / get_local_size(0);
    q = t * lid;
    rr = q + t;
    if(q > 4096) q = 4096;
    if(rr > 4096) rr = 4096;
    for(i = q; i < rr; i++) palette[i] = get_apalette(pal, i & mask, bright); // Prefetch palette

    t = (4 * 256) / get_local_size(0);
    q = t * lid;
    rr = q + t;
    if(q > (4 * 256)) q = 4 * 256;
    if(rr > (4 * 256)) rr = 4 * 256;
    for(i = q; i < rr; i++) tbl8[i] = table[i]; // Prefetch table

    barrier(CLK_LOCAL_MEM_FENCE);

    b = src;
    r = &src[ofset];
    g = &r[ofset];
    for(x = 0; x < ww; x++) {
	b3 = (uint)(b[0x0   ]) + 0x300;
	b2 = (uint)(b[0x2000]) + 0x200;
	b1 = (uint)(b[0x4000]) + 0x100;
	b0 = (uint)(b[0x6000]) + 0x000;
	
	b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3];

	r3 = (uint)(r[0x0   ]) + 0x700;
	r2 = (uint)(r[0x2000]) + 0x600;
	r1 = (uint)(r[0x4000]) + 0x500;
	r0 = (uint)(r[0x6000]) + 0x400;
	
	r8 =  tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3];

	g3 = (uint)(g[0x0   ]) + 0xb00;
	g2 = (uint)(g[0x2000]) + 0xa00;
	g1 = (uint)(g[0x4000]) + 0x900;
	g0 = (uint)(g[0x6000]) + 0x800;
	
	g8 =  tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3];
	
	cv = (b8 | r8 | g8) & mask8;
        av.s0 = palette[cv.s0];
        av.s1 = palette[cv.s1];
        av.s2 = palette[cv.s2];
        av.s3 = palette[cv.s3];
        av.s4 = palette[cv.s4];
        av.s5 = palette[cv.s5];
        av.s6 = palette[cv.s6];
        av.s7 = palette[cv.s7];
        *p8++ = av;
	b++;
	r++;
	g++;
	}
//    barrier(CLK_GLOBAL_MEM_FENCE);
    return;
}	
	
__kernel void getvram256k(__global uchar *src, int w, int h, 
                          __global uchar4 *out,
			  __global uint *table, 
			  uint mpage,
			  int multithread, int crtflag, float4 bright)
{
  int ofset = 0xc000;
  int x;
  int ww;
  int t;
  int gid;
  uint addr;
  uint addr2;
  uint r0, r1, r2, r3, r4, r5;
  uint b0, b1, b2, b3, b4, b5;
  uint g0, g1, g2, g3, g4, g5;
  uint8 r8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  uint8 g8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  uint8 b8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  int rdraw, gdraw, bdraw;
  __global uchar *r, *g, *b;
  uint8 cv;
  __global uint8 *p8;
  __global uint8 *tbl8;
  tbl8 = (__global uint8 *)table;
  uint col;
  uint pbegin, pb1;
  uint bright_r, bright_g, bright_b, bright_a;
  uint8 bright_r8, bright_g8, bright_b8, bright_a8;
  uint8 mask8 = (uint8){0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff};

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
  
#if 1
  p8 = (__global uint8 *)(&(out[addr2]));
  if(crtflag == 0) {
     clearscreen(ww, p8, bright);
//     barrier(CLK_GLOBAL_MEM_FENCE);
     return;
  }
#endif
  
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
  b = &src[addr];
  r = &src[addr + ofset];
  g = &src[addr + ofset + ofset];
  bright_r = bright.s0 * 255;
  bright_g = bright.s1 * 255;
  bright_b = bright.s2 * 255;
  bright_a = bright.s3 * 255;

  bright_r8 = (uint8){bright_r, bright_r, bright_r, bright_r, bright_r, bright_r, bright_r, bright_r};
  bright_g8 = (uint8){bright_g, bright_g, bright_g, bright_g, bright_g, bright_g, bright_g, bright_g};
  bright_b8 = (uint8){bright_b, bright_b, bright_b, bright_b, bright_b, bright_b, bright_b, bright_b};
  bright_a8 = (uint8){bright_a, bright_a, bright_a, bright_a, bright_a, bright_a, bright_a, bright_a};

  bright_a8 <<= ashift;

  r8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  g8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};
  b8 = (uint8){0, 0, 0, 0, 0, 0, 0, 0};

  gdraw = 0; if((mpage & 0x40) == 0) gdraw = -1; 
  rdraw = 0; if((mpage & 0x20) == 0) rdraw = -1; 
  bdraw = 0; if((mpage & 0x10) == 0) bdraw = -1; 
  for(x = 0; x < ww; x++) {
	if(bdraw) {
	    b5 = (uint)(b[0     ]) + 0x500;
	    b4 = (uint)(b[0x2000]) + 0x400;
	    b3 = (uint)(b[0x4000]) + 0x300;
	    b2 = (uint)(b[0x6000]) + 0x200;
	    b1 = (uint)(b[0x8000]) + 0x100;
	    b0 = (uint)(b[0xa000]) + 0x000;
	    b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3] | tbl8[b4] | tbl8[b5];
	    b8 <<= 2;
	    b8 = ((b8 * bright_b8) >> 8) & mask8;
#if (__ENDIAN_LITTLE__==1)
            b8 <<= 16;
#else
            b8 <<= 8;
#endif
	}
	if(rdraw) {
	    r5 = (uint)(r[0     ]) + 0x500;
	    r4 = (uint)(r[0x2000]) + 0x400;
	    r3 = (uint)(r[0x4000]) + 0x300;
	    r2 = (uint)(r[0x6000]) + 0x200;
	    r1 = (uint)(r[0x8000]) + 0x100;
	    r0 = (uint)(r[0xa000]) + 0x000;
	    r8 =  tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3] | tbl8[r4] | tbl8[r5];
	    r8 <<= 2;
	    r8 = ((r8 * bright_r8) >> 8) & mask8;
#if (__ENDIAN_LITTLE__==1)
            r8 <<= 0; // 6bit -> 8bit
#else
            r8 <<= 24; // 6bit -> 8bit
#endif
	}
	if(gdraw) {
	    g5 = (uint)(g[0     ]) + 0x500;
	    g4 = (uint)(g[0x2000]) + 0x400;
	    g3 = (uint)(g[0x4000]) + 0x300;
	    g2 = (uint)(g[0x6000]) + 0x200;
	    g1 = (uint)(g[0x8000]) + 0x100;
	    g0 = (uint)(g[0xa000]) + 0x000;
	    g8 =  tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3] | tbl8[g4] | tbl8[g5];
	    g8 <<= 2;
	    g8 = ((g8 * bright_g8) >> 8) & mask8;
#if (__ENDIAN_LITTLE__==1)
            g8 <<= 8; // 6bit -> 8bit
#else
            g8 <<= 16; // 6bit -> 8bit
#endif
	}
	cv = b8 | r8 | g8 | bright_a8;
	*p8++ = cv;
	b++;
	g++;
	r++;
   }
//   barrier(CLK_GLOBAL_MEM_FENCE);
   return;
}	
	

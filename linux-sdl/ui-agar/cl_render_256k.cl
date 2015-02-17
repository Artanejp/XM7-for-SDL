//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0

#if (_CL_KERNEL_LITTLE_ENDIAN==1)
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


struct apalettetbl_t {
   uchar line_h;
   uchar line_l;
   uchar mpage;
   uchar r_4096[4096];
   uchar g_4096[4096];
   uchar b_4096[4096];
} __attribute__((packed));

struct dpalettetbl_t {
   uchar line_h;
   uchar line_l;
   uchar mpage;
   uchar tbl[8];
}__attribute__((packed));

struct palettebuf_t {
   uchar alines_h;
   uchar alines_l;
   uchar dlines_h;
   uchar dlines_l;
   struct apalettetbl_t atbls[200];
   struct dpalettetbl_t dtbls[400];
}__attribute__((packed));

inline void clearscreen(int w,  __global uint8 *out, float4 bright)
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

__kernel void getvram256k(__global uchar *src, int w, int h, 
                          __global uchar4 *out,
			  __global ushort8 *table, 
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
  __local  ushort8 tbl8[256 * 6];
  uint col;
  int lid = 0;
  uint pbegin, pb1;
  uint bright_r, bright_g, bright_b, bright_a;
  __local uint8 bright_r8, bright_g8, bright_b8, bright_a8;
  uint8 mask8 = (uint8){0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff, 0x00ff};
  int i;

  ww = w >> 3;
  if(multithread != 0){
      t = get_global_size(0);
      lid = get_local_id(0);
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
  if(crtflag == 0) {
     clearscreen(ww, p8, bright);
     barrier(CLK_GLOBAL_MEM_FENCE);
     return;
  }

  if(lid == 0) {
      for(i = 0; i < (256 * 6); i++) tbl8[i] = table[i]; // Prefetch palette
      bright_r = bright.s0 * 255;
      bright_g = bright.s1 * 255;
      bright_b = bright.s2 * 255;
      bright_a = bright.s3 * 255;
      
      bright_r8 = (uint8){bright_r, bright_r, bright_r, bright_r, bright_r, bright_r, bright_r, bright_r};
      bright_g8 = (uint8){bright_g, bright_g, bright_g, bright_g, bright_g, bright_g, bright_g, bright_g};
      bright_b8 = (uint8){bright_b, bright_b, bright_b, bright_b, bright_b, bright_b, bright_b, bright_b};
      bright_a8 = (uint8){bright_a, bright_a, bright_a, bright_a, bright_a, bright_a, bright_a, bright_a};
      bright_a8 <<= ashift;
  }

  
  b = &src[addr];
  r = &src[addr + ofset];
  g = &src[addr + ofset + ofset];

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
	    b8 =  convert_uint8(tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3] | tbl8[b4] | tbl8[b5]);
	    b8 <<= 2;
	    b8 = ((b8 * bright_b8) >> 8) & mask8;
            b8 <<= bshift;
	}
	if(rdraw) {
	    r5 = (uint)(r[0     ]) + 0x500;
	    r4 = (uint)(r[0x2000]) + 0x400;
	    r3 = (uint)(r[0x4000]) + 0x300;
	    r2 = (uint)(r[0x6000]) + 0x200;
	    r1 = (uint)(r[0x8000]) + 0x100;
	    r0 = (uint)(r[0xa000]) + 0x000;
	    r8 =  convert_uint8(tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3] | tbl8[r4] | tbl8[r5]);
	    r8 <<= 2;
	    r8 = ((r8 * bright_r8) >> 8) & mask8;
            r8 <<= rshift; // 6bit -> 8bit
	}
	if(gdraw) {
	    g5 = (uint)(g[0     ]) + 0x500;
	    g4 = (uint)(g[0x2000]) + 0x400;
	    g3 = (uint)(g[0x4000]) + 0x300;
	    g2 = (uint)(g[0x6000]) + 0x200;
	    g1 = (uint)(g[0x8000]) + 0x100;
	    g0 = (uint)(g[0xa000]) + 0x000;
	    g8 =  convert_uint8(tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3] | tbl8[g4] | tbl8[g5]);
	    g8 <<= 2;
	    g8 = ((g8 * bright_g8) >> 8) & mask8;
            g8 <<= gshift; // 6bit -> 8bit
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

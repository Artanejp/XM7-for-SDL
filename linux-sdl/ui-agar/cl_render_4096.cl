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

uint get_apalette(__global struct apalettetbl_t *pal, uint col, uint4 bright)
{
   uint4 rgba;
   uint dat;
   rgba.s0 = (uint)(pal->r_4096[col] * bright.s0) & 0x00ff00;
   rgba.s1 = (uint)(pal->g_4096[col] * bright.s1) & 0x00ff00;
   rgba.s2 = (uint)(pal->b_4096[col] * bright.s2) & 0x00ff00;
   rgba.s3 = 0;
   rgba >>= 4;
   rgba.s0 <<= rshift;
   rgba.s1 <<= gshift;
   rgba.s2 <<= bshift;
   rgba.s3  = (uint)((255 * bright.s3) >> 8 ) << ashift;
   dat = rgba.s0 | rgba.s1 | rgba.s2 | rgba.s3;
   return dat;
}   

__kernel void getvram4096(__global uchar *src, int w, int h, 
                          __global uchar4 *out, __global struct palettebuf_t *pal,
			  __global ushort8 *table, 
			  uint multithread, int crtflag, float4 bright)
{
  int ofset = 0x8000;
  int x;
  int ww;
  int t;
  int gid;
  uint addr;
  uint addr2;
  uint r0, r1, r2, r3;
  uint b0, b1, b2, b3;
  uint g0, g1, g2, g3;
  ushort8 r8; 
  ushort8 g8;
  ushort8 b8;
  __global uchar *r, *g, *b;
  uint8 av;
  ushort8 cv;
  __global uint8 *p8;
  __local  ushort8 tbl8[256 * 4];
  uint pb1, pbegin, col;
  ushort8 mask8;
  ushort mask;
  int lid = 0;
  int i;
  uint palette[4096];
  int mpage;
  int line;
  int nextline = 200;
  int oldline = 0;
  int lines;
  int wrap;
  int line2;
  int palette_changed = 0;
  __local uint4 bright4;
  
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
      lid = 0;
  }


  p8 = (__global uint8 *)(&(out[addr2]));
  line = addr2 / 40;
  src = &src[addr];
  if(crtflag == 0) {
     clearscreen(ww, p8, bright);
     barrier(CLK_GLOBAL_MEM_FENCE);
     return;
  }

  if(lid == 0) {
     bright4 = convert_uint4((float4){256.0, 256.0, 256.0, 256.0} * bright);
     for(i = 0; i < 1024; i++) tbl8[i] = table[i]; // prefetch palette;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  {
      lines = (pal->alines_h << 8) | pal->alines_l;
      for(i = 0; i < lines; i++) {
          line2 = pal->atbls[i].line_h * 256 + pal->atbls[i].line_l;
          if((line2 < 0) || (line2 > 199)) break;
          if(line2 >= line) break;
       }
       
       oldline = i - 1;
       if(oldline < 0) oldline = 0;
       if(oldline >= lines) oldline = lines - 1;
       mpage = pal->atbls[oldline].mpage;
       mask = 0x0000;
       if(!(mpage & 0x10)) mask |= 0x000f;
       if(!(mpage & 0x20)) mask |= 0x00f0;
       if(!(mpage & 0x40)) mask |= 0x0f00;
       mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
       
       for(i = 0; i < 4096; i++) palette[i] = get_apalette(&(pal->atbls[oldline]), i & mask, bright4); // Prefetch palette
       
       if(oldline < (lines - 1)) {
           nextline = pal->atbls[oldline + 1].line_h * 256 + pal->atbls[oldline + 1].line_l;
	   if(nextline > 200) {
	        nextline = 200;
	   } 
	   oldline++;
	 } else {
	   nextline = 200;
	 }
    }
    if(((addr + ww - 1) / 40) >= nextline) {
         palette_changed = -1;
    }


    
  b = src;
  r = &src[ofset];
  g = &r[ofset];
  wrap = line;
  for(x = 0; x < ww; x++) {
    if(palette_changed != 0) {
       line = (x + addr) / 40;
       if((wrap != line) && (line >= nextline)) {
	      if(oldline < (lines - 1)) {
	        nextline = pal->atbls[oldline + 1].line_h * 256 + pal->atbls[oldline + 1].line_l;
		oldline++;
	      } else {
	        nextline = 200;
	      }
	      if(line <= nextline) palette_changed = 0;
	      wrap = line;
	      mpage = pal->atbls[oldline].mpage;
	      mask = 0x0000;
	      //mask = 0x0fff;
	      if(!(mpage & 0x10)) mask |= 0x000f;
	      if(!(mpage & 0x20)) mask |= 0x00f0;
	      if(!(mpage & 0x40)) mask |= 0x0f00;
	      for(i = 0; i < 4096; i++) palette[i] = get_apalette(&(pal->atbls[oldline]), i & mask, bright4); // Prefetch palette
	      mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
       }
    }
	b3 = (uint)(b[0x0   ]) + 0x300;
	b2 = (uint)(b[0x2000]) + 0x200;
	b1 = (uint)(b[0x4000]) + 0x100;
	b0 = (uint)(b[0x6000]) + 0x000;
	
	b8 =  tbl8[b0] | tbl8[b1] | tbl8[b2] | tbl8[b3];

	r3 = (uint)(r[0x0   ]) + 0x300;
	r2 = (uint)(r[0x2000]) + 0x200;
	r1 = (uint)(r[0x4000]) + 0x100;
	r0 = (uint)(r[0x6000]) + 0x000;
	
	r8 = tbl8[r0] | tbl8[r1] | tbl8[r2] | tbl8[r3];
	r8 = r8 * (ushort8){16, 16, 16, 16, 16, 16, 16, 16};
	
	g3 = (uint)(g[0x0   ]) + 0x300;
	g2 = (uint)(g[0x2000]) + 0x200;
	g1 = (uint)(g[0x4000]) + 0x100;
	g0 = (uint)(g[0x6000]) + 0x000;
	g8 = tbl8[g0] | tbl8[g1] | tbl8[g2] | tbl8[g3];
	g8 = g8 * (ushort8){256, 256, 256, 256, 256, 256, 256, 256};
	
	
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

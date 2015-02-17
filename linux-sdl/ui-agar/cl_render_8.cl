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
//   uint a = 255;
   uint8 abuf;

   a = (a & 255) << ashift;
   abuf = (uint8){a, a, a, a, a, a, a, a};

   for(i = 0; i < w; i++) {
      *p++ = abuf;
   }
}

uint4 ttlpalet2rgb(__global uchar *pal, uint index)
{
    uchar dat = pal[index];
    uint4 ret;

    ret.s2 = ((dat & 0x01) == 0)?0:255; // B
    ret.s0 = ((dat & 0x02) == 0)?0:255; // R
    ret.s1 = ((dat & 0x04) == 0)?0:255; // G
    ret.s3 = 255; // A
    return ret;
}

void setup_ttlpalette(__global uchar *pal, uint *palette, uint4 bright, uint vpage)
{
   int i;
   uint4 v;
   uint4  rgba_int;
   uint4  palette_int[8];
   uint8  *palette8;
   uint8 r8, g8, b8, a8;
   
   for(i = 0; i < 8; i++) {
      v = ttlpalet2rgb(pal, i & vpage);
      rgba_int = ((v * bright) & (uint4){0x0000ff00, 0x0000ff00, 0x0000ff00, 0x0000ff00}) >> 8;
      palette_int[i] = rgba_int;
   }
   r8 = (uint8) {palette_int[0].s0, palette_int[1].s0,
                 palette_int[2].s0, palette_int[3].s0,
		 palette_int[4].s0, palette_int[5].s0,
		 palette_int[6].s0, palette_int[7].s0} << rshift;
   g8 = (uint8) {palette_int[0].s1, palette_int[1].s1,
                 palette_int[2].s1, palette_int[3].s1,
		 palette_int[4].s1, palette_int[5].s1,
		 palette_int[6].s1, palette_int[7].s1} << gshift;
   b8 = (uint8) {palette_int[0].s2, palette_int[1].s2,
                 palette_int[2].s2, palette_int[3].s2,
		 palette_int[4].s2, palette_int[5].s2,
		 palette_int[6].s2, palette_int[7].s2} << bshift;
   a8 = (uint8) {palette_int[0].s3, palette_int[1].s3,
                 palette_int[2].s3, palette_int[3].s3,
		 palette_int[4].s3, palette_int[5].s3,
		 palette_int[6].s3, palette_int[7].s3} << ashift;
   palette8 = (uint8 *)palette;
   *palette8 = r8 | g8 | b8 | a8;
		 
}

__kernel void getvram8(__global uchar *src, int w, int h, __global uchar4 *out,
                       __global struct palettebuf_t *pal, __global ushort8 *table,
		       int multithread, int crtflag, float4 bright)
{
  int ofset = 0x4000;
  int x;
  int ww;
  int t, q, rr;
  int gid;
  uint addr;
  uint addr2;
  uchar rc,bc,gc;
  uint8   av;
  ushort  mask;
  ushort8 mask8;
  __local  ushort8 tbl8[3 * 256];
  ushort8 c8;
  __global uint8 *p8;
  __global uchar *src_r;
  __global uchar *src_g;
  __global uchar *src_b;
  uint pb1, pbegin, col;
  uint palette[8];
  int line;
  int lid = 0;
  int i;
  int oldline = 0;
  int lines;
  int nextline = h;
  int wrap;
  uchar mpage;
  __local uint4 bright4;
  int line2;
  int palette_changed = 0;
  
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
      t = 1;
      gid = 0;
      lid = 0;
  }
  
  p8 = (__global uint8 *)(&(out[addr2]));

  if(crtflag == 0) {
    clearscreen(ww, p8, bright);
    barrier(CLK_GLOBAL_MEM_FENCE);
    return;
  }

  if(h > 200) ofset = 0x8000;

  src_r = (__global uchar *)&src[addr + ofset];
  src_g = (__global uchar *)&src[addr + ofset + ofset];
  src_b = (__global uchar *)&src[addr];

  q = 0;
  rr = 256 * 3;
  if(lid == 0) {
      for(i = q; i < rr; i++) tbl8[i] = table[i]; // Prefetch palette
      bright4 = convert_uint4((float4){255.0, 255.0, 255.0, 255.0} * bright);
//        bright4 = (uint4){255, 255, 255, 255};
  }
  barrier(CLK_LOCAL_MEM_FENCE);

  line = addr / 80;

  {
       lines = pal->dlines_h * 256 + pal->dlines_l;
       i = 0;
       //nextline = pal->dtbls[0].line_h * 256 + pal->dtbls[0].line_l;
       for(i = 0; i < lines; i++) {
          line2 = pal->dtbls[i].line_h * 256 +  pal->dtbls[i].line_l;
          if((line2 < 0) || (line2 >= h)) break;
          if(line2 >= line) break;
       }
       oldline = i - 1;
       if(oldline < 0) oldline = 0;
       if(oldline >= lines) oldline = lines - 1; 
       mpage = pal->dtbls[oldline].mpage;
       mask = (~(mpage >> 4)) & 0x07;
       mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
       
       setup_ttlpalette(pal->dtbls[oldline].tbl, palette, bright4, mask);
       
       if(oldline < (lines - 1)) {
	     nextline = pal->dtbls[oldline + 1].line_h * 256 + pal->dtbls[oldline + 1].line_l;
	     if(nextline > h) {
	       nextline = h;
	     }
	     oldline++;
       } else {
	     nextline = h;
       }
       if(((addr + ww - 1) / 80) >= nextline) {
         palette_changed = -1;
       }
  }

  //barrier(CLK_LOCAL_MEM_FENCE);

  wrap = line;
  for(x = 0; x < ww; x++) {

     if(palette_changed != 0) {
      line = (x + addr) / 80;
      if((wrap != line) && (line >= nextline)) {
	   mpage = pal->dtbls[oldline].mpage;
	   mask = (~(mpage >> 4)) & 0x07;
	   mask8 = (ushort8){mask, mask, mask, mask, mask, mask, mask, mask};
	   setup_ttlpalette(pal->dtbls[oldline].tbl, palette, bright4, mask);
	   wrap = line;
 	   if(oldline < (lines - 1)) {
	        nextline = pal->dtbls[oldline + 1].line_h * 256 + pal->dtbls[oldline + 1].line_l;
		oldline++;
	    } else {
	        nextline = h;
	   }
	   if(line <= nextline) palette_changed = 0;
       }
     }


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

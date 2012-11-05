//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0

struct DisplayDesc {
  int w  __attribute__((endian(host)));
  int h  __attribute__((endian(host)));
  int window_open __attribute__((endian(host)));
  int window_dx1  __attribute__((endian(host)));
  int window_dx2  __attribute__((endian(host)));
  int window_dy1  __attribute__((endian(host)));
  int window_dy2  __attribute__((endian(host)));
  int vramoffset  __attribute__((endian(host)));
};
  
inline uchar4 putpixel(int n)
{
  uchar4 cbuf;
#if 0
  cbuf.s0 = (n & 0x000000ff); // R
  cbuf.s1 = (n & 0x0000ff00) >> 8; // G
  cbuf.s2 = (n & 0x00ff0000) >> 16; // B
  cbuf.s3 = 0xff;  // A
#else
  cbuf.s0 = (n & 0x000000ff) ; // R
  cbuf.s2 = (n & 0x00ff0000) >> 16; // G
  cbuf.s1 = (n & 0x0000ff00) >> 8; // B
  cbuf.s3 = 0xff; //A 
#endif
  return cbuf;
}

inline uint8 pixeldecode8(uchar rc, uchar gc, uchar bc, uint8 pal)
{
  uint8 av;
  uint8 mask;
	mask.s0 = ((bc & 0x80) >> 7) + ((rc & 0x80) >> 6) + ((gc & 0x80) >> 5);
	mask.s1 = ((bc & 0x40) >> 6) + ((rc & 0x40) >> 5) + ((gc & 0x40) >> 4);
	mask.s2 = ((bc & 0x20) >> 5) + ((rc & 0x20) >> 4) + ((gc & 0x20) >> 3);
	mask.s3 = ((bc & 0x10) >> 4) + ((rc & 0x10) >> 3) + ((gc & 0x10) >> 2);
	mask.s4 = ((bc & 0x08) >> 3) + ((rc & 0x08) >> 2) + ((gc & 0x08) >> 1);
	mask.s5 = ((bc & 0x04) >> 2) + ((rc & 0x04) >> 1) + (gc & 0x04);
	mask.s6 = ((bc & 0x02) >> 1) + (rc & 0x02)      + ((gc & 0x02) << 1);
	mask.s7 = (bc & 0x01)      + ((rc & 0x01) << 1) + ((gc & 0x01) << 2);
	mask &= (uint8){0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};
	av = shuffle(pal, mask);
  return av;
}

__kernel void getvram8(__global uchar *src, int w, int h, __global uchar4 *out, __global uint *pal)
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
  int t;
  int gid;
  uint addr;
  uint addr2;
  uchar rc,bc,gc;
  uint8 av;
  uint8 palette;
  __global uchar4 *p;


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
  for(y = ybegin; y < yend; y++) {
     addr = y * ww + xbegin; 
     addr2 = addr << 3;
     p = &(out[addr2]);
     for(x = xbegin; x < xend; x++) {
        bc = src[addr];
	rc = src[addr + ofset];
	gc = src[addr + ofset + ofset];
	av = pixeldecode8(rc, gc, bc, palette);
	*p++ = putpixel(av.s0);
	*p++ = putpixel(av.s1);
	*p++ = putpixel(av.s2);
	*p++ = putpixel(av.s3);
	*p++ = putpixel(av.s4);
	*p++ = putpixel(av.s5);
	*p++ = putpixel(av.s6);
	*p++ = putpixel(av.s7);

        addr++;
	}
    }
}	
	
uint8 pixeldecode4096(__global uchar *r, __global uchar *g, __global uchar *b, __global uint *pal)
{
  uint cv;
  uint8 av;
  uint4 mask;
  uint4 rc = (uint4){r[0], r[8000], r[16000], r[24000]};
  uint4 gc = (uint4){g[0], g[8000], g[16000], g[24000]};
  uint4 bc = (uint4){b[0], b[8000], b[16000], b[24000]};
  uint4 rr, gg, bb;
  
  mask = (uint4){0x0080, 0x0080, 0x0080, 0x0080};
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 4) + (gg.s1 << 3) + (gg.s2 << 2) + (gg.s3 << 1) +
           rr.s0       + (rr.s1 >> 1) + (rr.s2 >> 2) + (rr.s3 >> 3) +
          (bb.s0 >> 4) + (bb.s1 >> 5) + (bb.s2 >> 6) + (bb.s3 >> 7);
  av.s0 = pal[cv];
	  
  mask = mask >> 1;
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 5) + (gg.s1 << 4) + (gg.s2 << 3) + (gg.s3 << 2) +
          (rr.s0 << 1) + (rr.s1 >> 0) + (rr.s2 >> 1) + (rr.s3 >> 2) +
          (bb.s0 >> 3) + (bb.s1 >> 4) + (bb.s2 >> 5) + (bb.s3 >> 6);
  av.s1 = pal[cv];
  
  mask = mask >> 1;
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 6) + (gg.s1 << 5) + (gg.s2 << 4) + (gg.s3 << 3) +
          (rr.s0 << 2) + (rr.s1 << 1) + (rr.s2 >> 0) + (rr.s3 >> 1) +
          (bb.s0 >> 2) + (bb.s1 >> 3) + (bb.s2 >> 4) + (bb.s3 >> 5);
  av.s2 = pal[cv];
  
  mask = mask >> 1;
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 7) + (gg.s1 << 6) + (gg.s2 << 5) + (gg.s3 << 4) +
          (rr.s0 << 3) + (rr.s1 << 2) + (rr.s2 << 1) + (rr.s3 >> 0) +
          (bb.s0 >> 1) + (bb.s1 >> 2) + (bb.s2 >> 3) + (bb.s3 >> 4);
  av.s3 = pal[cv];

  mask = mask >> 1;
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 8) + (gg.s1 << 7) + (gg.s2 << 6) + (gg.s3 << 5) +
          (rr.s0 << 4) + (rr.s1 << 3) + (rr.s2 << 2) + (rr.s3 << 1) +
          (bb.s0 >> 0) + (bb.s1 >> 1) + (bb.s2 >> 2) + (bb.s3 >> 3);
  av.s4 = pal[cv];

  mask = mask >> 1;
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 9) + (gg.s1 << 8) + (gg.s2 << 7) + (gg.s3 << 6) +
          (rr.s0 << 5) + (rr.s1 << 4) + (rr.s2 << 3) + (rr.s3 << 2) +
          (bb.s0 << 1) + (bb.s1 >> 0) + (bb.s2 >> 1) + (bb.s3 >> 2);
  av.s5 = pal[cv];

  mask = mask >> 1;
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 10) + (gg.s1 << 9) + (gg.s2 << 8) + (gg.s3 << 7) +
          (rr.s0 << 6) + (rr.s1 << 5) + (rr.s2 << 4) + (rr.s3 << 3) +
          (bb.s0 << 2) + (bb.s1 << 1) + (bb.s2 >> 0) + (bb.s3 >> 1);
  av.s6 = pal[cv];

  mask = mask >> 1;
  rr = rc & mask;
  gg = gc & mask;
  bb = bc & mask;
  cv = (gg.s0 << 11) + (gg.s1 << 10) + (gg.s2 << 9) + (gg.s3 << 8) +
          (rr.s0 << 7) + (rr.s1 << 6) + (rr.s2 << 5) + (rr.s3 << 4) +
          (bb.s0 << 3) + (bb.s1 << 2) + (bb.s2 << 1) + (bb.s3);
  av.s7 = pal[cv];

return av;
}

__kernel void getvram4096(__global uchar *src, int w, int h, __global uchar4 *out, __global uint *pal)
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
  uchar rc,bc,gc;
  __global uchar *r, *g, *b;
  uint8 av;
  __global uchar4 *p;


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
     p = &(out[addr2]);
     for(x = xbegin; x < xend; x++) {
        b = &src[addr];
	r = &src[addr + ofset];
	g = &src[addr + ofset + ofset];
	av = pixeldecode4096(r, g, b, pal);
	*p++ = putpixel(av.s0);
	*p++ = putpixel(av.s1);
	*p++ = putpixel(av.s2);
	*p++ = putpixel(av.s3);
	*p++ = putpixel(av.s4);
	*p++ = putpixel(av.s5);
	*p++ = putpixel(av.s6);
	*p++ = putpixel(av.s7);

        addr++;
	}
    }
}	
	

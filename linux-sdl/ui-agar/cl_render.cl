//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0


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
  cbuf.s1 = (n & 0x00ff0000) >> 16; // G
  cbuf.s2 = (n & 0x0000ff00) >> 8; // B
  cbuf.s3 = 0xff; //A 
#endif
  return cbuf;
}

inline uint8 pixeldecode8(uchar rc, uchar gc, uchar bc, __global uint *pal)
{
  uint8 av;
	av.s0 = pal[((bc & 0x80) >> 7) + ((rc & 0x80) >> 6) + ((gc & 0x80) >> 5)];
	av.s1 = pal[((bc & 0x40) >> 6) + ((rc & 0x40) >> 5) + ((gc & 0x40) >> 4)];
	av.s2 = pal[((bc & 0x20) >> 5) + ((rc & 0x20) >> 4) + ((gc & 0x20) >> 3)];
	av.s3 = pal[((bc & 0x10) >> 4) + ((rc & 0x10) >> 3) + ((gc & 0x10) >> 2)];
	av.s4 = pal[((bc & 0x08) >> 3) + ((rc & 0x08) >> 2) + ((gc & 0x08) >> 1)];
	av.s5 = pal[((bc & 0x04) >> 2) + ((rc & 0x04) >> 1) + (gc & 0x04)];
	av.s6 = pal[((bc & 0x02) >> 1) + (rc & 0x02)      + ((gc & 0x02) << 1)];
	av.s7 = pal[(bc & 0x01)      + ((rc & 0x01) << 1) + ((gc & 0x01) << 2)];
  return av;
}

__kernel void getvram8(__global uchar *src, int w, int h, __global uchar4 *out, __global uint *pal)
{
  int ofset = 0x4000;
  int x;
  int y;
  int ybegin = 0;
  int yend;
  int xbegin = 0;
  int xend;
  int hh;
  int ww;
  uint addr;
  uint addr2;
  uchar rc,bc,gc;
  uint8 av;
  uint palette[8];
  __global uchar4 *p;
  
  hh = h;
  ww = w >> 3;
  yend = ybegin + hh;

  xend = xbegin + ww;
  if(h > 200) ofset = 0x8000;
  for(y = ybegin; y < yend; y++) {
     addr = y * ww + xbegin; 
     addr2 = addr << 3;
     p = &(out[addr2]);
     for(x = xbegin; x < xend; x++) {
        bc = src[addr];
	rc = src[addr + ofset];
	gc = src[addr + ofset + ofset];
	av = pixeldecode8(rc, bc, gc, pal);
	*p++ = putpixel(av.s0);
	*p++ = putpixel(av.s1);
	*p++ = putpixel(av.s2);
	*p++ = putpixel(av.s3);
	*p++ = putpixel(av.s4);
	*p++ = putpixel(av.s5);
	*p++ = putpixel(av.s6);
	*p++ = putpixel(av.s7);

        addr++;
	addr2 += 8;
	}
    }
}	
	
	
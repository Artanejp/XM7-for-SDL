//
// CL renderer
// (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
// History: Nov 01,2012 Initial.
// License: Apache License 2.0

__kernel void getvram8(__global uchar *src, int w, int h, __global uint *out, __global uint *pal)
{
  int ofset = 0x4000;
  int i;
  int x;
  int y;
  int ybegin = 0;
  int yend;
  int xbegin = 0;
  int xend;
  int hh;
  int ww;
  int addr;
  uint r,g,b;
  uchar rc,bc,gc;
  uint8 cv;
  uint8 av;
  uint8 palette;
  __global uint *p;
  __global uint8 *pv;
  
  hh = h;
  yend = ybegin + hh;
  addr = ybegin * w; 
  ww = w;
  xend = xbegin + ww;
  palette.s0 = pal[0];
  palette.s1 = pal[1];
  palette.s2 = pal[2];
  palette.s3 = pal[3];
  palette.s4 = pal[4];
  palette.s5 = pal[5];
  palette.s6 = pal[6];
  palette.s7 = pal[7];
  
  if(h > 200) ofset = 0x8000;
  for(y = ybegin; y< yend; y++) {
     for(x = xbegin; x < xend; x++) {
        bc = src[addr];
	rc = src[addr + ofset];
	gc = src[addr + ofset + ofset];
	p = &out[y * 640 + x * 8];
	pv = (__global uint8 *)p;
	b = (uint)bc;
	g = (uint)gc;
	r = (uint)rc;
	//cv.s0 = (b & 0x80) >> 7 + (r & 0x80) >> 6 + (g & 0x80) >> 5;
	//cv.s1 = (b & 0x40) >> 6 + (r & 0x40) >> 5 + (g & 0x40) >> 4;
	//cv.s2 = (b & 0x20) >> 5 + (r & 0x20) >> 4 + (g & 0x20) >> 3;
	//cv.s3 = (b & 0x10) >> 4 + (r & 0x10) >> 3 + (g & 0x10) >> 2;
	//cv.s4 = (b & 0x08) >> 3 + (r & 0x08) >> 2 + (g & 0x08) >> 1;
	//cv.s5 = (b & 0x04) >> 2 + (r & 0x04) >> 1 + (g & 0x04);
	//cv.s6 = (b & 0x02) >> 1 + (r & 0x02)      + (g & 0x02) << 1;
	//cv.s7 = (b & 0x01)      + (r & 0x01) << 1 + (g & 0x01) << 2;
	//*pv = shuffle(palette, cv);
	cv = (uint8){0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	             0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
	*pv = cv;
	}
    }
}	
	
	
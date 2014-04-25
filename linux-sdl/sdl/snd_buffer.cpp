/*
 * snd_buffer.cpp
 * Manage Sound Buffer
 *   by K.Ohta <whatisthis.sowhat@gmail.com>
 * History: 28 Jan,2012 : Split from api_snd2.cpp
 */


#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "xm7_sdl.h"
#include "sdl_sch.h"
#include "xm7_types.h"

#include "api_snd.h"
#include "api_wavwriter.h"


#include "SndDrvIF.h"
#include "SndDrvBeep.h"
#include "SndDrvWav.h"
#include "SndDrvOpn.h"
#include "SndDrvCMT.h"



static void CopyChunkSub(Sint16 *buf, Sint16 *src, int count)
{
    int i;
    int ic;
    Sint16 *s = src;
    Sint16 *b = buf;
    Sint32 dw;
    v8hi_t *hs;
    v8hi_t *hb;
    v8hi_t t;

    if(s == NULL) return;
    if(b == NULL) return;
   hb = (v8hi_t *)buf;
   hs = (v8hi_t *)src;

   ic = (count / 8) << 3;
   for(i = 0; i < ic ; i+=8){
      hb->v = hb->v + hs->v;
      hb++;
      hs++;
   }
   for(i = ic; i < count; i++) {
        dw = (Sint32)s[i];
        dw += (Sint32)b[i];
        b[i] = (Sint16)dw;
    }

}

/*
* レンダリングしたバッファをコピーする(WAVキャプチャ用)
*/
int CopyChunk(struct SndBufType *p, Sint16 *buf, int offset)
{
	int i = p->nChunkNo;
	int j;
	int count = 0;
	int channels = 2;
        int samples;
        int r;

    if(buf == NULL) return -1;
    if(offset < 0) offset = 0;

    if(p->nWritePTR > (p->nReadPTR + offset)) {
        samples = p->nSize + p->nReadPTR + offset - p->nWritePTR;
    } else{
        samples = p->nReadPTR + offset - p->nWritePTR;
    }
    r = samples;
    while(samples > 0) {
        j = p->nSize - p->nReadPTR;
        if(j > samples) {
	        // 分割不要
	    CopyChunkSub(&buf[count * channels + offset], &p->pBuf[(p->nReadPTR + count) * channels], samples * channels);
	    count += samples;
            samples = 0;
        } else {
	    CopyChunkSub(&buf[count * channels + offset], &p->pBuf[(p->nReadPTR + count)* channels], j * channels);
	    count += j;
            samples -= j;
        }
        if(count >= p->nSize) {
		   count = 0;
		}
        i++;
        if(i >= p->nChunks) i = 0;
    }
//    return i;
    return r;
}

/*
* レンダリングしたバッファをコピーする(WAVキャプチャ用)
*/
int MoveChunk(struct SndBufType *p, Sint16 *buf, int offset)
{
    int i = p->nChunkNo;
    int j;
    int count = 0;
    int channels = 2;
    int samples;
    int r;

    if(buf == NULL) return -1;
    if(offset < 0) offset = 0;

//    if(p->nWritePTR > (p->nReadPTR + offset)) {
    if(p->nWritePTR < (p->nReadPTR + offset)) {
        samples = p->nSize + p->nReadPTR + offset - p->nWritePTR;
    } else{
        samples = p->nReadPTR + offset - p->nWritePTR;
    }
    r = samples;
    while(samples > 0) {
        j = p->nSize - p->nReadPTR;
        if(j > samples) {
	        // 分割不要
	    CopyChunkSub(&buf[count * channels + offset], &p->pBuf[(p->nReadPTR + count) * channels], samples * channels);
	    count += samples;
	    p->nReadPTR += samples;
            samples = 0;
        } else {
	    CopyChunkSub(&buf[count * channels + offset], &p->pBuf[(p->nReadPTR + count)* channels], j * channels);
	    p->nReadPTR += j;
	    if(p->nReadPTR >= p->nSize) p->nReadPTR = 0;
	    count += j;
            samples -= j;
        }
        if(count >= p->nSize) {
		   count = 0;
	}
        i++;
        if(i >= p->nChunks) i = 0;
    }
    return r;
}

int MoveChunkChunk(struct SndBufType *dst, struct SndBufType *src, BOOL inc, BOOL bZero)
{
   int i;
   int j;
   int k;
   int toWrite;
   int toRead;

   if(dst == NULL) return -1;
   if(src == NULL) return -1;
   toRead = src->nWritePTR - src->nReadPTR;
   if(toRead < 0) { // Wrap
      toRead = src->nSize - src->nWritePTR + src->nReadPTR;
   }
   if(toRead <= 0) return 0;
//   toWrite = dst->nReadPTR - dst->nWritePTR;
//   if(toWrite <= 0){
//      toWrite = dst->nSize - dst->nReadPTR + dst->nWritePTR;
//   }
//   if(toRead > toWrite) {
//      toRead = toWrite; // Shrink
//   }
   i = toRead;
   k = dst->nWritePTR;
   do {
       if(i <= 0) break;
       if(i < (dst->nSize - k)) {
	  j = i;
       } else {
	  j =  dst->nSize - k;
       }
       if(bZero) {
	   memset(&(dst->pBuf[k]), 0x00, j);
       }
       MoveChunk(src, &(dst->pBuf[k]), 0);
       if(inc) dst->nWritePTR += j;
       if(dst->nWritePTR >= dst->nSize) dst->nWritePTR = 0;
       k += j;
       if(k >= dst->nSize) k = 0;
      i -= j;
   } while(1);
   return toRead;
}


/*
 *
 */

/*
 * サウンドレンダリング本体
 * レンダラは上書きしていく(!)
 */


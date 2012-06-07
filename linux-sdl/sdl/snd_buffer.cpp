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


/*
 * サウンドバッファの概要を初期化する
 */
struct SndBufType *InitBufferDesc(void)
{
	struct SndBufType *p;
	void *cp;
	int i;

	p = (struct SndBufType *)malloc(sizeof(struct SndBufType));
	if(p){
		memset(p, 0x00, sizeof(struct SndBufType));
		cp = malloc(sizeof(Mix_Chunk *) * CHUNKS);
		if(cp) {
			memset(cp, 0x00, sizeof(Mix_Chunk *) * CHUNKS);
		}
		p->mChunk = (Mix_Chunk **)cp;
		for(i = 0; i < CHUNKS ; i++) {
			p->mChunk[i] = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
			if(p->mChunk[i]) {
				memset((void *)p->mChunk[i], 0x00, sizeof(Mix_Chunk));
			}
		}
		p->nChunks = CHUNKS;
		p->nChunkNo = 0;
		p->nHeadChunk = 0;
		p->nLastChunk = 0;
	}
	return p;
}

/*
 * サウンドバッファの概要を消す
 */
void DetachBufferDesc(struct SndBufType *p)
{
	int i;

	if(p){
		DetachBuffer(p);
		for(i = 0; i < p->nChunks ; i++) {
			if(p->mChunk[i]) {
				free(p->mChunk[i]);
				p->mChunk[i] = NULL;
			}
		}
		p->nChunks = 0;
		free(p);
		p = NULL;
	}
	return;
}


/*
 * 実バッファのALLOCATE
 */
void SetupBuffer(struct SndBufType *p, int members, BOOL flag16, BOOL flag32)
{
    int size;
    int channels = 2;
    void *pv;

	if(p == NULL) return;
	if(flag16) {
		size = (members + 1) * sizeof(Sint16) * channels;
#ifdef _WINDOWS
		size = ((size + 15) / 16 ) * 16;
		pv = malloc(size);
		if(pv != NULL) {
#else // POSIX
	        if(posix_memalign(&pv, 16, size) == 0) {
#endif

//		p->pBuf = (Sint16 *)malloc(size);
		p->pBuf = (Sint16 *)pv;
		memset(p->pBuf, 0x00, size);
		p->nSize = members;
	    }
	   
	}
	if(flag32) {
		size = (members + 1) * sizeof(Sint32) * channels;
#ifdef _WINDOWS
		size = ((size + 15) / 16 ) * 16;
		pv = malloc(size);
		if(pv != NULL) {	
#else
	        if(posix_memalign(&pv, 16, size) == 0) {
#endif
		        p->pBuf32 = (Sint32 *)pv;
			memset(p->pBuf32, 0x00, size);
			p->nSize = members;
		}
	}
	p->nReadPTR = 0;
	p->nWritePTR = 0;

}

void DetachBuffer(struct SndBufType *p)
{
	if(p == NULL) return;
	if(p->pBuf) {
		free(p->pBuf);
		p->pBuf = NULL;
	}
	if(p->pBuf32) {
		free(p->pBuf32);
		p->pBuf32 = NULL;
	}
}

static void CopyChunkSub(Sint16 *buf, Sint16 *src, int count)
{
    int i;
    int ic;
    Sint16 *s = src;
    Sint16 *b = buf;
    Sint32 dw;
    v8hi *hs;
    v8hi *hb;
    v8hi t;

    if(s == NULL) return;
    if(b == NULL) return;
   hb = (v8hi *)buf;
   hs = (v8hi *)src;

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

DWORD RenderSub(struct SndBufType *p, SndDrvIF *drv, DWORD ttime, int samples, BOOL bZero)
{
   int j;
   if(p == NULL) return 0;
   if(drv == NULL) return 0;
   if(samples <= 0) return 0;

	j = samples;

	if((j + p->nWritePTR) >= p->nSize){
		// バッファオーバフローの時は分割する
		int k;

		k = p->nSize - p->nWritePTR;
		if(k > 0) {
			drv->Render(p->pBuf32, p->pBuf, p->nWritePTR , k,  FALSE, bZero);
			j = j - k;
//		   printf("SND:DBG:RENDER:%08x Size=%d\n", p->pBuf, k);
		}
		p->nWritePTR = 0;
		if(j > 0) {
			drv->Render(p->pBuf32, p->pBuf, 0, j, FALSE, bZero);
			p->nWritePTR = j;
//		   printf("SND:DBG:RENDER:%08x Size=%d\n", p->pBuf, j);
		}
	} else {
		if(j > 0) {
			drv->Render(p->pBuf32, p->pBuf, p->nWritePTR, j,  FALSE, bZero);
			p->nWritePTR += j;
			if(p->nWritePTR >= p->nSize) p->nWritePTR -= p->nSize;
//			if(p->nWritePTR >= p->nSize) p->nWritePTR = 0;
//		       printf("SND:DBG:RENDER:%08x Size=%d\n", p->pBuf, j);
		}
	}
	p->nLastTime = ttime;
	return samples;
}

BOOL SndFlushSub(struct SndBufType *p, SndDrvIF *drv, DWORD ttime,  BOOL bZero, int maxchunk)
{
   int chunksize;
   
   if(p == NULL) return FALSE;
   if(drv == NULL) return FALSE;
   if(maxchunk <= 0) return FALSE;
   chunksize = maxchunk  - (p->nWritePTR % maxchunk);
   if(chunksize <= 0) return TRUE;
	/*
	 * オーバーフロー対策込
	 */
   if(RenderSub(p, drv, ttime, chunksize, bZero) != 0) {
	 return TRUE;
   }
   return FALSE;
}


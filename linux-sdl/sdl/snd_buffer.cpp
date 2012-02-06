/*
 * snd_buffer.cpp
 * Manage Sound Buffer
 *   by K.Ohta <whatisthis.sowhat@gmail.com>
 * History: 28 Jan,2012 : Split from api_snd2.cpp
 */


#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else				/* */

#include <linux/soundcard.h>
#endif				/* */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "sdl.h"
#include "sdl_sch.h"

#include "api_snd.h"
#include "api_wavwriter.h"

#include "SndDrvIF.h"
#include "SndDrvBeep.h"
#include "SndDrvWav.h"
#include "SndDrvOpn.h"
#include "SndDrvCMT.h"
//#include "util_ringbuffer.h"

typedef uint16_t v4si __attribute__ ((__vector_size__(16), aligned(16)));
typedef uint16_t v8si __attribute__ ((__vector_size__(32), aligned(16)));
typedef union {
     v4si v;
     uint32_t i[4];
     uint16_t s[8];
} v4hi;


typedef union {
     v8si v;
     uint32_t i[8];
     uint16_t s[16];
} v8hi;


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

	if(p == NULL) return;
	if(flag16) {
		size = (members + 1) * sizeof(Sint16) * channels;
		p->pBuf = (Sint16 *)malloc(size);
		if(p->pBuf) {
			memset(p->pBuf, 0x00, size);
			p->nSize = members;
		}
	}
	if(flag32) {
		size = (members + 1) * sizeof(Sint32) * channels;
		p->pBuf32 = (Sint32 *)malloc(size);
		if(p->pBuf32) {
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
    v4hi *hs;
    v4hi *hb;
    v4hi t;

    if(s == NULL) return;
    if(b == NULL) return;
#if 0
   for(i = 0; i < count; i++) {
        dw = (Sint32)s[i];
        dw += (Sint32)b[i];
        if(dw > 32767) dw = 32768;
        if(dw < -32767) dw = 32767;
        b[i] = (Sint16)dw;
    }
#else
   hb = (v4hi *)buf;
   hs = (v4hi *)src;
   
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
#endif
   
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

    if(buf == NULL) return -1;
    if(offset < 0) offset = 0;

    if(p->nWritePTR > (p->nReadPTR + offset)) {
        samples = p->nSize + p->nReadPTR + offset - p->nWritePTR;
    } else{
        samples = p->nReadPTR + offset - p->nWritePTR;
    }
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
    return i;
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
		}
		p->nWritePTR = 0;
		if(j > 0) {
			drv->Render(p->pBuf32, p->pBuf, 0, j, FALSE, bZero);
			p->nWritePTR = j;
		}
	} else {
		if(j > 0) {
			drv->Render(p->pBuf32, p->pBuf, p->nWritePTR, j,  FALSE, bZero);
			p->nWritePTR += j;
			if(p->nWritePTR >= p->nSize) p->nWritePTR -= p->nSize;
		}
	}
	p->nLastTime = ttime;
	return j;

}


BOOL FlushCMTSub(struct SndBufType *p, SndDrvIF *drv, DWORD ttime,  BOOL bZero, int maxchunk)
{
	int chunksize;

        if(p == NULL) return FALSE;
        if(drv == NULL) return FALSE;
	if(maxchunk <= 0) return FALSE;
	chunksize = maxchunk - (p->nWritePTR % maxchunk);
        if(chunksize <= 0) return TRUE;
	/*
	 * オーバーフロー対策込
	 */
      if(RenderSub(p, drv, ttime, chunksize, bZero) != 0) {
	 return TRUE;
      }
   return FALSE;
}

/*
 * バッファを一定時間の所まで埋める
 * TRUE : 埋めた
 * FALSE : 埋める必要がなかった
 */
BOOL FlushBeepSub(struct SndBufType *p, SndDrvIF *drv, DWORD ttime,  BOOL bZero, int maxchunk)
{
	int chunksize;
        if(p == NULL) return FALSE;
        if(drv == NULL) return FALSE;
	if(maxchunk <= 0) return FALSE;

	chunksize = maxchunk - (p->nWritePTR % maxchunk);
        if(chunksize <= 0) return TRUE;
	/*
	 * オーバーフロー対策込
	 */
      if(RenderSub(p, drv, ttime, chunksize, bZero) != 0) {
	 return TRUE;
      }
   return FALSE;
}

BOOL FlushOpnSub(struct SndBufType *p, SndDrvIF *drv, DWORD ttime,  BOOL bZero, int maxchunk)
{
        int chunksize;

        if(p == NULL) return FALSE;
        if(drv == NULL) return FALSE;
	if(maxchunk <= 0) return FALSE;
	chunksize = maxchunk - (p->nWritePTR % maxchunk);
        if(chunksize <= 0) return TRUE;

      if(RenderSub(p, drv, ttime, chunksize, bZero) != 0) {
	 return TRUE;
      }
   return FALSE;
}


/*
 * util_ringbuffer.c
 *
 *  Created on: 2010/09/27
 *      Author: whatisthis
 */


#include "util_ringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RingBufferDesc *CreateRingBuffer(int chunkSize, int chunks)
{
	int size;
	int i;
	struct RingBufferIndex **p;
	struct RingBufferIndex *pp;
	struct RingBufferDesc *q;
	q = malloc(sizeof(struct RingBufferDesc));
	if(q == NULL) return NULL;
	q->chunks = chunks;
	q->chunkSize = chunkSize;
	q->sem = SDL_CreateSemaphore(1);

	size = chunks * sizeof(struct RingBufferIndex *);
	p = (struct RingBufferIndex *)malloc(size);
	if(p == NULL) {
		free(q);
		return NULL;
	}
	memset(p, 0, size);
	q->index = p;
		for(i = 0; i< chunks; i++) {
			size = chunks * sizeof(struct RingBufferIndex);
			pp = malloc(size);
			if(pp == NULL) {
				free(p);
				free(q);
				return NULL;
			}
			memset(pp, 0x00, size);
			p[i] = pp;
			pp->num = i;
			pp->use = FALSE;
			pp->buffer = malloc(chunkSize);
			if(pp->buffer == NULL) {
				pp->num = -1;
				continue;
			}
			memset(pp->buffer, 0x00, chunkSize);
		}
	return q;
}

void DeleteRingBuffer(struct RingBufferDesc *q)
{
	int i;
	int chunks;
	struct RingBufferIndex **p;
	struct RingBufferIndex *pp;

	if(q == NULL) return;
	if(q->index == NULL) return;
	p = q->index;
	if(q->sem != NULL) {
		SDL_SemWait(q->sem);
	}
	chunks = q->chunks;
	for(i = 0; i < chunks; i++) {
		pp = p[i];
		if(pp != NULL) {
			if(pp->buffer != NULL) {
				free(pp->buffer);
				pp->buffer = NULL;
			}
			free(pp);
			pp = NULL;
		}
	}
	free(p);
	if(q->sem != NULL)	 {
		SDL_DestroySemaphore(q->sem);
		q->sem = NULL;
	}
	free(q);
}


int WriteRingBuffer(struct RingBufferDesc *q, void *p)
{
	int i;
	int chunks;


	if(q == NULL) return -1;
	if(q->index == NULL) return -1;
	chunks = q->chunks;
	if(q->sem == NULL) return -1;

	SDL_SemWait(q->sem);

	for(i = 0; i <chunks; i++) {
		if(q->index[i] != NULL) {
			if((!q->index[i]->use) && (q->index[i]->buffer != NULL)) {
				memcpy(q->index[i]->buffer, (Uint8 *)p, q->chunkSize);
				q->index[i]->use = TRUE;
				SDL_SemPost(q->sem);
				return q->chunkSize;
			}
		} else {
			SDL_SemPost(q->sem);
			return -1;
		}
	}
	/*
	 * キューが一杯なので
	 */
	SDL_SemPost(q->sem);
	return 0; /* 書けませんでした */
}

int WriteRingBufferLimited(struct RingBufferDesc *q, void *p, int len)
{
	int i;
	int chunks;
	int len2;

	if(q == NULL) return -1;
	if(q->index == NULL) return -1;
	if(q->sem == NULL) return -1;

	chunks = q->chunks;
	SDL_SemWait(q->sem);
	len2 = q->chunkSize;
	if(len2 > len) {
		len2 = len;
	}

	for(i = 0; i <chunks; i++) {
		if(q->index != NULL) {
			if((!q->index[i]->use) && (q->index[i]->buffer != NULL)) {
				memcpy(q->index[i]->buffer, (Uint8 *)p, len2);
				if(len2 < q->chunkSize) {
					Uint8 *pp = (Uint8 *)p;
					pp = &pp[len2];
					memset(pp, 0x00, q->chunkSize - len2); // NULLで埋める
				}
				q->index[i]->use = TRUE;
				SDL_SemPost(q->sem);
				return len2;
			}
		} else {
			SDL_SemPost(q->sem);
			return -1;
		}
	}
	/*
	 * キューが一杯なので
	 */
	SDL_SemPost(q->sem);
	return 0; /* 書けませんでした */
}


static inline void Copy32to16(Uint32 *from, Uint16 *to, int size)
{
        int         i, j, k;
        Sint32       *p = (Sint32 *) from;
        Sint16       *t = (Sint16 *) to;
        Sint32       tmp1;

        if (p == NULL) {
                return;
        }
        if (t == NULL) {
                return;
        }
        i = (size / 4) * 4;
        for (j = 0; j < i; j += 4) {
                tmp1 = p[j];
                t[j] = (Sint16) tmp1 ;
                tmp1 = p[j + 1];
                t[j + 1] = (Sint16) tmp1;
                tmp1 = p[j + 2];
                t[j + 2] = (Sint16) tmp1 ;
                tmp1 = p[j + 3];
                t[j + 3] = (Sint16) tmp1;
        }
        k = size % 4;
        if(k == 0) return; // 剰余数なし
        i = size - k;
        for (j = 0; j < k; j++, i++) {
                tmp1 = p[i];
                t[i] = (Sint16)tmp1;
        }
}


/*
 * 32bitデータを16bitにして書き込む(Soundなど)
 */
int WriteRingBuffer32to16(struct RingBufferDesc *q, void *p)
{
	int i;
	int chunks;

	if(q == NULL) return -1;
	if(q->index == NULL) return -1;
	if(q->sem == NULL) return -1;

	chunks = q->chunks;
	SDL_SemWait(q->sem);

	for(i = 0; i <chunks; i++) {
		if(q->index != NULL) {
			if((!q->index[i]->use) && (q->index[i]->buffer != NULL)) {
				/*
				 *
				 */
				Copy32to16((Uint32 *)p, (Uint16 *)q->index[i]->buffer, q->chunkSize);
				q->index[i]->use = TRUE;
				SDL_SemPost(q->sem);
				return q->chunkSize;
			}
		} else {
			SDL_SemPost(q->sem);
			return -1;
		}
	}
	/*
	 * キューが一杯なので
	 */
	SDL_SemPost(q->sem);
	return 0; /* 書けませんでした */
}


int ReadRingBuffer(struct RingBufferDesc *q, void *p)
{
	int i;

	if(q == NULL) return -1;
	if(q->index == NULL) return -1;
	if(q->sem == NULL) return -1;

	SDL_SemWait(q->sem);

	for(i = 0; i <q->chunks; i++) {
		if(q->index != NULL) {
			if(q->index[i]->use && (q->index[i]->buffer != NULL)) {
				memcpy((Uint8 *)p, q->index[i]->buffer,  q->chunkSize);
				q->index[i]->use = FALSE;
				SDL_SemPost(q->sem);
				return q->chunkSize;
			}
		} else {
			SDL_SemPost(q->sem);
			return -1;
		}
	}
	/*
	 * キューが空でした
	 */
	SDL_SemPost(q->sem);
	return 0;
}

/*
 * Read only len Bytes
 */
int ReadRingBufferLimited(struct RingBufferDesc *q, void *p, int len)
{
	int i;
	int len2;

	if(q == NULL) return -1;
	if(q->index == NULL) return -1;
	if(q->sem == NULL) return -1;

	SDL_SemWait(q->sem);

	for(i = 0; i <q->chunks; i++) {
		if(q->index != NULL) {
			if(q->index[i]->use && (q->index[i]->buffer != NULL)) {
				len2 = q->chunkSize;
				if(len < len2) len2 = len;
				memcpy((Uint8 *)p, q->index[i]->buffer,  len2);
				q->index[i]->use = FALSE;
				SDL_SemPost(q->sem);
				return len2;
			}
		} else {
			SDL_SemPost(q->sem);
			return -1;
		}
	}
	/*
	 * キューが空でした
	 */
	SDL_SemPost(q->sem);
	return 0;
}


#ifdef __cplusplus
}
#endif

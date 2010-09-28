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
	struct RingBufferIndex *p;
	struct RingBufferDesc *q;
	q = malloc(sizeof(struct RingBufferDesc));
	if(q == NULL) return NULL;
	q->chunks = chunks;
	q->chunkSize = chunkSize;
	q->sem = SDL_CreateSemaphore(1);

	size = (chunks * (sizeof(struct RingBufferIndex))/8 + 1)* 8;
	p = malloc(size);
	if(p != NULL) {
		memset(p, 0, size);
		for(i = 0; i< chunks; i++) {
			p[i].num = i;
			p[i].use = FALSE;
			p[i].buffer = malloc(chunkSize);
			if(p[i].buffer == NULL) {
				p[i].num = -1;
				continue;
			}
			memset(p[i].buffer, 0x00, chunkSize);
		}
	}
	q->index = p;
	return q;
}

void DeleteRingBuffer(struct RingBufferDesc *q)
{
	int i;
	int chunks;
	struct RingBufferIndex *p;

	if(q == NULL) return;
	if(q->index == NULL) return;
	p = q->index;
	if(q->sem != NULL) {
		SDL_SemWait(q->sem);
	}
	chunks = q->chunks;
	for(i = 0; i < chunks; i++) {
		if(p[i].buffer != NULL) {
			free(p[i].buffer);
			p[i].buffer = NULL;
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
	SDL_SemWait(q->sem);

	for(i = 0; i <chunks; i++) {
		if(q->index != NULL) {
			if((!q->index[i].use) && (q->index[i].buffer != NULL)) {
				memcpy(q->index[i].buffer, (Uint8 *)p, q->chunkSize);
				q->index[i].use = TRUE;
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
	SDL_SemWait(q->sem);

	for(i = 0; i <q->chunks; i++) {
		if(q->index != NULL) {
			if(q->index[i].use && (q->index[i].buffer != NULL)) {
				memcpy((Uint8 *)p, q->index[i].buffer,  q->chunkSize);
				q->index[i].use = FALSE;
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

#ifdef __cplusplus
}
#endif

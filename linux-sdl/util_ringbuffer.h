/*
 * util_ringbuffer.h
 *
 *  Created on: 2010/09/27
 *      Author: whatisthis
 */

#ifndef UTIL_RINGBUFFER_H_
#define UTIL_RINGBUFFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int     BOOL;

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef TRUE
#define TRUE (!FALSE)
#endif /* TRUE */

struct RingBufferIndex {
	int num;
	BOOL use;
	char *buffer;
};

struct RingBufferDesc {
	int chunks;
	int chunkSize;
	struct RingBufferIndex *index;
	SDL_sem *sem;
};

extern struct RingBufferDesc *CreateRingBuffer(int chunkSize, int chunks);
extern void DeleteRingBuffer(struct RingBufferDesc *q);
extern int WriteRingBuffer(struct RingBufferDesc *q, void *p);
extern int ReadRingBuffer(struct RingBufferDesc *q, void *p);
extern int WriteRingBuffer32to16(struct RingBufferDesc *q, void *p);

#ifdef __cplusplus
}
#endif


#endif /* UTIL_RINGBUFFER_H_ */

/*
 * SndDrvWav.cpp
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_rwops.h>
#include "xm7.h"

#include "SndDrvWav.h"

Uint8 *buf;
int bufSize;
int samples;
int channels;
int playCh;
int srate;
int howlong; /* 実際の演奏秒数 */
Mix_Chunk chunk;
Mix_Chunk *chunkP;
BOOL enable;
int counter;



SndDrvWav::SndDrvWav() {
	// TODO Auto-generated constructor stub
	bufSize = 0;
	buf = NULL;
	ms = 0;
	srate = nSampleRate;
	channels = 1;
	playCh = -1;
	bufSize = 0;
	howlong = 0;
	chunk.abuf = buf;
	chunk.alen = bufSize;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}

SndDrvWav::~SndDrvWav() {
	// TODO Auto-generated destructor stub
	DeleteBuffer();
}

Uint8 *SndDrvWav::NewBuffer(void)
{
	Uint8 *p;
	return NULL;
}

void SndDrvWav::DeleteBuffer(void)
{
	if(chunkP == NULL) return;
	do{
	} while(Mix_Playing(playCh));

	Mix_FreeChunk(chunkP);
	playCh = -1;
	srate = 0;
	ms = 0;
	bufSize = 0;
	howlong = 0;
	channels = 1;
	chunk.abuf = NULL;
	chunk.alen = 0;
	chunk.allocated = 0; /* アロケートされてる */
	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}

/*
 * RAM上のWAVデータを演奏可能な形で読み込む
 */
Uint8 *SndDrvWav::QuickLoad(Uint8 *rbuf)
{
	if(chunkP != NULL) return NULL; /* 既にこのChunkは使われている */

	chunkP = Mix_QuickLoadWav(rbuf);
	if(chunkP == NULL) return NULL;
	chunk.abuf = chunkP->abuf;
	chunk.alen = chunkP->alen;
	chunk.volume = chunkP->volume;
	chunk.allocated = chunkP->allocated;
	bufSize = chunkP->alen;
	buf = chunkP->abuf;
	ms = chunkP->alen / (srate * channels * sizeof(int16));
	enable = FALSE;
	counter = 0;
	howlong = ms;
	return chunkP;
}




/*
 * レンダリング
 */
void SndDrvWav::Render(int msec, BOOL clear)
{
	int s = msec;
	int i,j;
	int size;
	int samples;
	int dat;
	int16          *wbuf = (int16 *) buf;


	if(buf == NULL) return;
	if(!enable) return;
	if(s>ms) s = ms;
	do {
		SDL_Delay(1); /* Waitいるか? */
	} while(playCh < -1);
	size = (uRate * s * channels * sizeof(int16)) / 1000;
	samples = size / sizeof(int16);

//	if(clear) memset(wbuf, 0x00, size);
	howlong = s;
}

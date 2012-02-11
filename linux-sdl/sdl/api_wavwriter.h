/*
 * api_wavwriter.h
 * WAVファイルを書きこむ(汎用ルーチン)
 *  Created on: 2011/05/16
 *      Author: Kyuma Ohta <whatisthis.sowhat@gmail.com>
 */

#ifndef API_WAVWRITER_H_
#define API_WAVWRITER_H_

#define _USE_SDL_MIXER


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <SDL/SDL.h>
#ifdef _USE_SDL_MIXER
#include <SDL/SDL_mixer.h>
#endif
#include <math.h>

#if !defined(BOOL)
typedef int BOOL;
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif


struct WavPCMFmtDesc {
	char HEADID[4]; // = {'f', 'm', 't', ' ');
	// Start of Format desc.
	uint32_t Size; //
	uint16_t FmtType; // LPCM=01;
	uint16_t Channels;
	uint32_t SampleRate;
	uint32_t Speed; // Bytes per second.
	uint16_t SampleBits;
	uint16_t ExtraSize; // Extra Area
	// End of Format Header
} __attribute__ ((packed));


struct WavHeader {
	char ID1[4]; // = {'R', 'I', 'F', 'F'};
	uint32_t totalSize; // size without ID1 & size
	char ID2[4]; // = {'W', 'A', 'V', 'E'};
	struct WavPCMFmtDesc fmt;
	char DATAID[4]; // = {'d', 'a', 't', 'a'};
	uint32_t DataSize; //
} __attribute__ ((packed));


struct WavDesc {
	struct WavHeader header;
	uint32_t totalSize;
	uint32_t dataSize;
	Sint16 *data;
	SDL_RWops *file;
};

// Utilityルーチン
// エンディアンをIntelオーダに変換する
static inline uint32_t EndianChangeUint32(uint32_t a)
{
#if SDL_LIL_ENDIAN
	return a;
#else
	uint32_t b;
	b = (a  & 0x000000ff)<<24 + (a & 0x0000ff00)<<8 + (a & 0x00ff0000)>>8 + (a & 0xff000000)>>24;
	return b;
#endif
}

static inline uint16_t EndianChangeUint16(uint16_t a)
{
#if SDL_LIL_ENDIAN
	return a;
#else
	uint16_t b;
	b = (a & 0xff00)>>8 + (a & 0x00ff)<<8;
	return b;
#endif
}

// 関数群
/*
 *  WAVの書き込み開始(OPEN)
 */
extern struct WavDesc *StartWavWrite(char *path, uint32_t nSampleRate);
/*
 * WAVの書き込み終了
 */
extern BOOL EndWriteWavData(WavDesc *desc);
/*
 * WAVの書き込み(Sint16型配列)
 */
extern int WriteWavDataSint16(struct WavDesc *desc, Sint16 *data, int size);
/*
 * WAVの書き込み(Byte型)
 */
extern int WriteWavDataByte(struct WavDesc *desc, Uint8 *data, int size);

/*
 * WAVの合成(クリッピング付き)
 */
extern Sint16 *WavMixClip(Sint16 **srcs, Sint16 *dst, int members, int bufSize, Sint16 clipping);
/*
 * WAVの合成(クリッピングなし)
 */
extern Sint16 *WavMix(Sint16 **srcs, Sint16 *dst, int members, int bufSize);



/*
 * Mix_ChunkでのWAVの書き込み
 */
#ifdef _USE_SDL_MIXER
extern int WriteWavDataMixChunk(struct WavDesc *desc, Mix_Chunk *data);
#endif

#ifdef __cplusplus
}
#endif

#endif /* API_WAVWRITER_H_ */



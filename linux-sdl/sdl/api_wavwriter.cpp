/*
 * api_wavwriter.cpp
 * WAVファイルを書きこむ(汎用ルーチン/SDL使用)
 *  Created on: 2011/05/16
 *      Author: Kyuma Ohta <whatisthis.sowhat@gmail.com>
 */
#include "api_wavwriter.h"

#ifdef __cplusplus
extern "C" {
#endif
	/*
	 * WAVを書きこむ（開始）
	 */
struct WavDesc *StartWavWrite(char *path, uint32_t nSampleRate)
{
	struct WavDesc *w;
	struct WavHeader *h;
	int result;
	FILE *file;

	if(path == NULL) return NULL;

	w = (struct WavDesc *)malloc(sizeof(struct WavDesc));
	if(w == NULL) return NULL;
	memset((void *)w, 0x00, sizeof(struct WavDesc));
	h = &(w->header);
	// Open File
#if 0 
	file = fopen(path, "wb");
	if(file == NULL) {
		fclose(file);
		goto err;
	}

	w->file = SDL_RWFromFP(file, 1);
#else
        w->file = SDL_RWFromFile(path,  "wb");
#endif   
	if(w->file == NULL) goto err;
	// RIFF ヘッダ開始
	h->ID1[0] = 'R';
	h->ID1[1] = 'I';
	h->ID1[2] = 'F';
	h->ID1[3] = 'F';
	// Format ID
	h->fmt.HEADID[0] = 'f';
	h->fmt.HEADID[1] = 'm';
	h->fmt.HEADID[2] = 't';
	h->fmt.HEADID[3] = ' ';
	// 以下、エンディアンで数値を変える(一般的なWavヘッダはLittle Endian のため)
	h->fmt.Size = EndianChangeUint32(0x10);  // フォーマットヘッダサイズ=16Bytes
	h->fmt.FmtType = EndianChangeUint16(0x0001); //
	h->fmt.Channels = EndianChangeUint16(0x0002); // 2ch
	h->fmt.SampleRate = EndianChangeUint32(nSampleRate); //
	h->fmt.Speed = EndianChangeUint32(nSampleRate * 2 * sizeof(Sint16));
	h->fmt.SampleBits = EndianChangeUint16(16);
	h->fmt.ExtraSize = EndianChangeUint16(0);
	// データ本体
	h->DATAID[0] = 'd';
	h->DATAID[1] = 'a';
	h->DATAID[2] = 't';
	h->DATAID[3] = 'a';
	h->DataSize = EndianChangeUint32(0x00000000);
	// とりあえず仮ヘッダを書き込む
	w->totalSize = sizeof(struct WavHeader) - sizeof(uint32_t) - 4; // Total - ID1 - totalSize
	h->totalSize = EndianChangeUint32(w->totalSize);
	result = w->file->write(w->file, (void *)h, sizeof(struct WavHeader), 1);
	if(result != 1) goto err;
success:
	return w;
err:
	if(w->file) SDL_RWclose(w->file);
        printf("ERR: Open file to write %s\n",path);
	free(w);
	return NULL;
}


BOOL EndWriteWavData(WavDesc *desc)
{
	int result;
	int seekPtr;

	if(desc == NULL) return FALSE;
	desc->header.totalSize = EndianChangeUint32(desc->totalSize);
	desc->header.DataSize = EndianChangeUint32(desc->dataSize);
	if(desc->file) {
		seekPtr = 4;
		result = desc->file->seek(desc->file, seekPtr, SEEK_SET);
		if(result != seekPtr) goto err;
		result = desc->file->write(desc->file, &(desc->header.totalSize), sizeof(uint32_t), 1);
		if(result != 1) goto err;

		seekPtr = sizeof(struct WavHeader) - sizeof(uint32_t);
		result = desc->file->seek(desc->file, seekPtr, SEEK_SET);
		if(result != seekPtr) goto err;
		result = desc->file->write(desc->file, &(desc->header.DataSize), sizeof(uint32_t), 1);
		if(result != 1) goto err;
	success:
		desc->file->close(desc->file);
		free(desc);
		return TRUE;
	err:
		desc->file->close(desc->file);
		free(desc);
		return FALSE;
	}
	// そもそもファイルポインタがないのだからFAILで返す
	return FALSE;
}

/*
 * データ本体を書き込む：バイト数を返す
 */
int WriteWavDataSint16(struct WavDesc *desc, Sint16 *data, int size)
{
	int s = size * sizeof(Sint16);
	int result;

	result = desc->file->write(desc->file, (void *)data, sizeof(Sint16), size);
	if(result != size){
		//SDL_RWclose(desc->file);
	        printf("ERR: Writing WAV file.\n");
		EndWriteWavData(desc);
		return -1;
	}
//	printf("Wrote: Addr=0x%08x Size=%d result=%d\n", data, size, result);
	desc->dataSize += s;
	desc->totalSize += s;
	return s;
}

int WriteWavDataByte(struct WavDesc *desc, Uint8 *data, int size)
{
	int result;

	if(desc == NULL) return -1;
	if(data == NULL) return -1;
	result = desc->file->write(desc->file, (void *)data, sizeof(Uint8), size);
	if(result != size){
		//SDL_RWclose(desc->file);
		EndWriteWavData(desc);
	        printf("ERR: Writing WAV file.\n");
		return -1;
	}
	desc->dataSize += size;
	desc->totalSize += size;
	return size;
}


#ifdef _USE_SDL_MIXER
int WriteWavDataMixChunk(struct WavDesc *desc, Mix_Chunk *data)
{
	int size;
	int result;

	if(desc == NULL) return -1;
	if(data == NULL) return -1;
	size = data->alen;
	result = desc->file->write(desc->file, (void *)data->abuf, sizeof(Uint8), size);
//	printf("Wrote: Addr=0x%08x Size=%d result=%d\n", data->abuf, size, result);

	if(result != size){
		//SDL_RWclose(desc->file);
		EndWriteWavData(desc);
	        printf("ERR: Writing WAV file.\n");
		return -1;
	}
	desc->dataSize += size;
	desc->totalSize += size;
	return size;
}


#endif // _USE_SDL_MIXER

/*
 * WAVデータの合成
 * srcsのバッファをDstsにmix(均等ボリューム)
 */
Sint16 *WavMixClip(Sint16 **srcs, Sint16 *dst, int members, int bufSize, Sint16 clipping)
{
	int i;
	int j;
	Sint32 clip32_low = -(Sint32)abs(clipping);
	Sint32 clip32_high = (Sint32)abs(clipping);
	Sint32 tmp;
	Sint16 *p;

	if(dst == NULL) return NULL;
	if(srcs == NULL) return NULL;
	memset((void *)dst, 0x00, bufSize * sizeof(Sint16));
	for(i = 0; i < members; i++) {
		if(srcs[i] == NULL) return NULL;
		p = srcs[i];
		for(j = 0; j < bufSize; j++){
			tmp = (Sint32)dst[j];
			tmp += (Sint32)p[j];
			if(tmp > clip32_high) tmp = clip32_high;
			if(tmp < clip32_low) tmp = clip32_low;
			dst[j] =(Sint16) tmp;
		}
	}
	return dst;
}

/*
 * WAVデータの合成(クリッピングなし、但しMAX/MIN(Sint16)超えはチェック
 * srcsのバッファをDstsにmix(均等ボリューム)
 */
Sint16 *WavMix(Sint16 **srcs, Sint16 *dst, int members, int bufSize)
{
	int i;
	int j;
	Sint32 tmp;
	Sint16 *p;

	if(dst == NULL) return NULL;
	if(srcs == NULL) return NULL;
	memset((void *)dst, 0x00, bufSize * sizeof(Sint16));
	for(i = 0; i < members; i++) {
		if(srcs[i] == NULL) return NULL;
		p = srcs[i];
		for(j = 0; j < bufSize; j++){
			tmp = (Sint32)dst[j];
			tmp += (Sint32)p[j];
			if(tmp > 32767) tmp = 32767;
			if(tmp < -32767) tmp = -32767;
			dst[j] = (Sint16) tmp;
		}
	}
	return dst;
}


#ifdef __cplusplus
}
#endif

// EOF

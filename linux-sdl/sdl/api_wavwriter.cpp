/*
 * api_wavwriter.cpp
 * WAVファイルを書きこむ(汎用ルーチン/SDL使用)
 *  Created on: 2011/05/16
 *      Author: Kyuma Ohta <whatisthis.sowhat@gmail.com>
 */
#include "api_wavwriter.h"

extern "C" {
   
	/*
	 * WAVを書きこむ（開始）
	 */
struct WavDesc *StartWavWrite(char *path, uint32_t nSampleRate)
{
	struct WavDesc *w;
	struct WavHeader *h;
	int result;

	if(path == NULL) return NULL;

	w = (struct WavDesc *)malloc(sizeof(struct WavDesc));
	if(w == NULL) return NULL;
	memset((void *)w, 0x00, sizeof(struct WavDesc));
	h = &(w->header);
	// Open File
        w->file = SDL_RWFromFile(path,  "wb");
	if(w->file == NULL) goto err;
	// RIFF ヘッダ開始
	h->ID1[0] = 'R';
	h->ID1[1] = 'I';
	h->ID1[2] = 'F';
	h->ID1[3] = 'F';
        // ID2
        h->ID2[0] = 'W';
	h->ID2[1] = 'A';
	h->ID2[2] = 'V';
	h->ID2[3] = 'E';

        // Format ID
	h->fmt.HEADID[0] = 'f';
	h->fmt.HEADID[1] = 'm';
	h->fmt.HEADID[2] = 't';
	h->fmt.HEADID[3] = ' ';
	// 以下、エンディアンで数値を変える(一般的なWavヘッダはLittle Endian のため)
	//h->fmt.Size = EndianChangeUint32(0x10);  // フォーマットヘッダサイズ=16Bytes
	h->fmt.Size = EndianChangeUint32(sizeof(struct WavPCMFmtDesc) - 8);  // フォーマットヘッダサイズ=24Bytes
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
       if(w->file) {
	   SDL_RWclose(w->file);
//	   SDL_FreeRW(w->file);
	}
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
#if 1
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
#else
	        result = desc->file->seek(desc->file, 0, SEEK_SET);
		if(result != 0) goto err;
		result = desc->file->write(desc->file, &(desc->header), sizeof(struct WavHeader), 1);
		if(result != 1) goto err;
#endif
       success:
	        printf("DBG: Success Writing TotalSize = %08x DataSize = %08x.\n", desc->header.totalSize, desc->header.DataSize);
	        if(desc->file) {
		   SDL_RWclose(desc->file);
//		   SDL_FreeRW(desc->file);
		}
//		desc->file->close(desc->file);
		free(desc);
		return TRUE;
	err:
	        if(desc->file) {
		   SDL_RWclose(desc->file);
//		   SDL_FreeRW(desc->file);
		}
//		free(desc);
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
	int s;
	int result;
   
        size = (size / 2 )*2; // align 4bytes(2words)
        s = size * sizeof(Sint16);
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

}

// EOF

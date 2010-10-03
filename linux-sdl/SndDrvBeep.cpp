/*
 * SndDrvBeep.cpp
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "xm7.h"
#include "SndDrvBeep.h"



SndDrvBeep::SndDrvBeep() {
	// TODO Auto-generated constructor stub
	int i;
	int uStereo = nStereoOut %4;
	if ((uStereo > 0) || bForceStereo) {
		channels = 2;
	} else {
		channels = 1;
	}
	ms = nSoundBuffer;
	srate = nSampleRate;
	bufSlot = BEEP_SLOT;
	lastslot = 0;

	srate = nSampleRate;
	bufSize = (ms * srate * channels *sizeof(Sint16)) / 1000;
	for(i = 0 ; i< bufSlot; i++) {
		buf[i] = NULL;
		chunk[i].abuf = buf[i];
		chunk[i].alen = bufSize;
		chunk[i].allocated = 0; /* アロケートされてる */
		chunk[i].volume = 128; /* 一応最大 */
	}
	enable = FALSE;
	counter = 0;
	nLevel = 32767;
	volume = MIX_MAX_VOLUME;
	RenderSem = SDL_CreateSemaphore(1);
}

SndDrvBeep::~SndDrvBeep() {
	// TODO Auto-generated destructor stub
	int i;
	enable = FALSE;
	for(i = 0; i < BEEP_SLOT; i++) {
		DeleteBuffer(i);
	}
	if(RenderSem) SDL_DestroySemaphore(RenderSem);
}


Uint8 *SndDrvBeep::NewBuffer(void)
{
	int i;
	for(i = 0; i< bufSlot; i++) {
		NewBuffer(i);
	}
	return buf[0];
}


Uint8 *SndDrvBeep::NewBuffer(int slot)
{
	int uChannels;

	if(slot > bufSlot) return NULL;

	if(buf[slot] != NULL) {
		return NULL; /* バッファがあるよ？Deleteしましょう */
	}


	buf[slot] = (Uint8 *)malloc(bufSize);
	if(buf[slot] == NULL) return NULL; /* バッファ取得に失敗 */

	memset(buf[slot], 0x00, bufSize); /* 初期化 */
	return buf[slot];
}

void SndDrvBeep::DeleteBuffer(void)
{
	int i;
	for(i = 0; i <bufSlot ; i++)
	{
		DeleteBuffer(i);
	}
}

void SndDrvBeep::DeleteBuffer(int slot)
{

	if(slot > bufSlot) return;
	if(buf[slot] != NULL) free(buf[slot]);
	buf[slot] = NULL;

	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = 0;
	chunk[slot].allocated = 0; /* アロケートされてる */
	chunk[slot].volume = volume; /* 一応最大 */
}




Uint8  *SndDrvBeep::Setup(int tick)
{
	UINT uChannels;
	int i;

	uStereo = nStereoOut %4;
	if ((uStereo > 0) || bForceStereo) {
		uChannels = 2;
	} else {
		uChannels = 1;
	}

//	if((nSampleRate == srate) && (channels == uChannels)
//			&& (tick == ms)) return buf[0];
	channels = uChannels;
	ms = tick;
	srate = nSampleRate;
	bufSize = (ms * srate * channels *sizeof(Sint16)) / 1000;

	for(i = 0; i < BEEP_SLOT; i++) {
		if(buf[i] == NULL) {
			/*
			 * バッファが取られてない == 初期状態
			 */
			buf[i] = NewBuffer(i);
		} else {
			/*
			 * バッファが取られてる == 初期状態ではない
			 */
			DeleteBuffer(i); /* 演奏終了後バッファを潰す */
			buf[i] = NewBuffer(i);
		}
		chunk[i].abuf = buf[i];
		chunk[i].alen = bufSize;
		chunk[i].allocated = 1; /* アロケートされてる */
		chunk[i].volume = 128; /* 一応最大 */
	}
	enable = FALSE;
	counter = 0;
	return buf[0];
}


void SndDrvBeep::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}

void SndDrvBeep::SetLRVolume(void)
{
}

void SndDrvBeep::SetVolume(Uint8 level)
{
	int i;
	volume = level;
	if(volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
	for(i = 0; i<bufSlot; i++) {
		chunk[i].volume = volume;
	}
}


void SndDrvBeep::SetRate(int rate)
{
	srate = rate;
}


/*
 * BZERO : 指定領域を0x00で埋める
 */
int SndDrvBeep::BZero(int start, int uSamples, int slot, BOOL clear)
{
	int sSamples = uSamples;
	int s ;
	int ss,ss2;
	Sint16          *wbuf;

	if((slot > BEEP_SLOT) || (slot < 0)) return 0;
	s = (ms * srate)/1000;

	if(buf[slot] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	if(RenderSem == NULL) return 0;
	SDL_SemWait(RenderSem);
	wbuf = (Sint16 *)&buf[slot][start * channels];
	lastslot = slot;
	memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	//	if(!enable) return 0;

	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = (ss2 + start) * channels * sizeof(Sint16);
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = 128; /* 一応最大 */
	SDL_SemPost(RenderSem);

	return ss2;

}

Mix_Chunk *SndDrvBeep::GetChunk(void)
{
	return &chunk[0];
}

Mix_Chunk *SndDrvBeep::GetChunk(int slot)
{
	if((slot > bufSlot) || (slot < 0)) return NULL;
	return &chunk[slot];
}



void SndDrvBeep::Enable(BOOL flag)
{
	enable = flag;
}

/*
 * レンダリング
 */
int SndDrvBeep::Render(int start, int uSamples, int slot, BOOL clear)
{
	int i;
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	Sint16          *wbuf;
	Sint16 level = (Sint16)nLevel;
	int sf;


	if(slot > bufSlot) return 0;
//	s = bufSize / (channels * sizeof(Sint16));
	s = (ms * srate)/1000;

	if(buf[slot] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if((uSamples + start) >s) {
		ss2 = s - start;
	} else {
		ss2 = uSamples;
	}
	if(RenderSem == NULL) return 0;
	SDL_SemWait(RenderSem);
	wbuf = (Sint16 *)buf[slot];
	wbuf = &wbuf[start * channels];
	lastslot = slot;
//if(clear)  memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	if(clear)  memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
	if(enable) {

		/*
		 * ここにレンダリング関数ハンドリング
		 */

		/*
		 * サンプル書き込み
		 */
		for (i = 0; i < ss2; i++) {

			/*
			 * 矩形波を作成
			 */
			sf = (int) (counter * nBeepFreq );
			sf /= (int) srate;

			/*
			 * 偶・奇に応じてサンプル書き込み
			 */
			if (channels == 1) {
				if (sf & 1) {
					*wbuf++ = level;
				}
				else {
					*wbuf++ = -level;
				}
			}

			else {
				if (sf & 1) {
					*wbuf++ = level;
					*wbuf++ = level;
				}

				else {
					*wbuf++ = -level;
					*wbuf++ = -level;
				}
			}

			/*
			 * カウンタアップ
			 */
			counter+=2;
			if (counter >= srate) {
				counter = 0;
			}
		}
	}
	//    bufSize = ss2 + start;
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = (ss2 + start) * channels * sizeof(Sint16);
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = volume; /* 一応最大 */
	SDL_SemPost(RenderSem);
	return ss2;
}

void SndDrvBeep::Play(int ch,  int slot)
	{
		if(slot >= bufSlot) return;
		if(chunk[slot].abuf == NULL) return;
		if(chunk[slot].alen <= 0) return;
		if(!enable) return;
		if(RenderSem == NULL) return;
		SDL_SemWait(RenderSem);
		if(chunk[slot].abuf) Mix_PlayChannel(ch, &chunk[slot], 0);
		SDL_SemPost(RenderSem);
	}

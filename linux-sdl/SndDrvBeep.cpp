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


namespace {
Uint8 *buf[BEEP_SLOT];
int bufSize;
int samples;
UINT channels;
UINT srate;
UINT ms;
int nLevel;
Mix_Chunk chunk[BEEP_SLOT];
BOOL enable;
SDL_sem *RenderSem;
UINT counter;
}

SndDrvBeep::SndDrvBeep() {
	// TODO Auto-generated constructor stub
	int i;

	bufSize = 0;
	ms = 0;
	srate = nSampleRate;
	channels = 1;
	bufSize = 0;
	for(i = 0 ; i< BEEP_SLOT; i++) {
		buf[i] = NULL;
		chunk[i].abuf = buf[i];
		chunk[i].alen = bufSize;
		chunk[i].allocated = 0; /* アロケートされてる */
		chunk[i].volume = 128; /* 一応最大 */
	}
	enable = FALSE;
	counter = 0;
	nLevel = 0;
}

SndDrvBeep::~SndDrvBeep() {
	// TODO Auto-generated destructor stub
	int i;
	for(i = 0; i < BEEP_SLOT; i++) {
		DeleteBuffer(i);
	}
}


Uint8 *SndDrvBeep::NewBuffer(int slot)
{
	int uStereo,uChannels;
	if((slot>=BEEP_SLOT) || (slot <0)) return NULL;
	if(buf[slot] != NULL) return NULL; /* バッファがあるよ？Deleteしましょう */
	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
    channels = uChannels;

	bufSize = (ms * srate * channels * sizeof(Sint16)) / 1000;
	buf[slot] = (Uint8 *)malloc(bufSize);
	if(buf == NULL) return NULL; /* バッファ取得に失敗 */
	memset(buf[slot], 0x00, bufSize); /* 初期化 */
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = bufSize;
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
	return buf[slot];
}

void SndDrvBeep::DeleteBuffer(void)
{

	if(buf[0] != NULL) free(buf[0]);
	buf[0] = NULL;
	srate = 0;
	ms = 0;
	bufSize = 0;
	channels = 1;
	chunk[0].abuf = buf[0];
	chunk[0].alen = 0;
	chunk[0].allocated = 0; /* アロケートされてる */
	chunk[0].volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}

void SndDrvBeep::DeleteBuffer(int slot)
{
	if((slot >= BEEP_SLOT) || (slot < 0)) return;
	if(buf[slot] != NULL) free(buf[slot]);
	buf[slot] = NULL;
	srate = 0;
	ms = 0;
	bufSize = 0;
	channels = 1;
//	chunk.abuf = buf;
//	chunk.alen = 0;
//	chunk.allocated = 0; /* アロケートされてる */
//	chunk.volume = 128; /* 一応最大 */
	enable = FALSE;
	counter = 0;
}


Uint8  *SndDrvBeep::Setup(void *p)
{
	int uStereo;
	UINT uChannels;
	int i;

	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }

	   if((nSampleRate == srate) && (channels == uChannels)
			   && (nSoundBuffer == ms)) return buf[0];
	   channels = uChannels;
	   for(i = 0; i < BEEP_SLOT; i++) {
		   if(buf[i] == NULL) {
		   /*
		    * バッファが取られてない == 初期状態
		    */
			   ms = nSoundBuffer;
			   srate = nSampleRate;
			   buf[i] = NewBuffer(i);
		   } else {
			   /*
			    * バッファが取られてる == 初期状態ではない
			    */
			   DeleteBuffer(i); /* 演奏終了後バッファを潰す */
			   ms = nSoundBuffer;
			   srate = nSampleRate;
			   buf[i] = NewBuffer(i);
		   }
	   }
	   return buf[0];
}

void SndDrvBeep::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
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
	s = chunk[slot].alen / (sizeof(Sint16) * channels);

	if(buf[slot] == NULL) return 0;
	 wbuf = (Sint16 *) buf[slot];
	if(start > s) return 0; /* 開始点にデータなし */
	if(!enable) return 0;
	if(sSamples > s) sSamples = s;

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));

	return ss2;

}

Mix_Chunk *SndDrvBeep::GetChunk(void)
{
//	chunk.abuf = buf;
//	chunk.alen = samples * channels * sizeof(Sint16);
//	chunk.allocated = 1;
//	chunk.volume = 128;
	return &chunk[0];
}

Mix_Chunk *SndDrvBeep::GetChunk(int slot)
{
  	if((slot > BEEP_SLOT) || (slot < 0)) return NULL;
	chunk[slot].abuf = buf[slot];
  	chunk[slot].alen = bufSize * channels * sizeof(Sint16);
	chunk[slot].allocated = 1;
	chunk[slot].volume = 128;
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
	int sf;


  	if((slot > BEEP_SLOT) || (slot < 0)) return 0;
	s = chunk[slot].alen / (sizeof(Sint16) * channels);

	if(buf[slot] == NULL) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(!enable) return 0;
	if(sSamples > s) sSamples = s;

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	wbuf = (Sint16 *)&buf[slot][start];
	if(clear)  memset(wbuf, 0x00, ss2 * channels * sizeof(Sint16));
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
		sf = (int) (counter * nBeepFreq * 2);
		sf /= (int) srate;

		/*
		 * 偶・奇に応じてサンプル書き込み
		 */
		if (channels == 1) {
			if (sf & 1) {
				*wbuf++ = nLevel;
			}

			else {
				*wbuf++ = -nLevel;
			}
		}

		else {
			if (sf & 1) {
				*wbuf++ = nLevel;
				*wbuf++ = nLevel;
			}

			else {
				*wbuf++ = -nLevel;
				*wbuf++ = -nLevel;
			}
		}

		/*
		 * カウンタアップ
		 */
		counter++;
		if (counter >= srate) {
			counter = 0;
		}
	}
    bufSize = ss2 + start;
  	chunk[slot].abuf = buf[slot];
  	chunk[slot].alen = bufSize * channels * sizeof(Sint16);
  	chunk[slot].allocated = 1; /* アロケートされてる */
  	chunk[slot].volume = 128; /* 一応最大 */
	return ss2;
}


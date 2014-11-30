/*
 * SndDrvWav.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVWAV_H_
#define SNDDRVWAV_H_
#include <SDL/SDL.h>
//#include <SDL/SDL_mixer.h>
#include <math.h>

#include "xm7.h"
#include "xm7_sdl.h"
//#include "sdl_snd.h"

#include "SndDrvIF.h"

#define WAV_SLOT 3

class SndDrvWav {
public:
	SndDrvWav();
	virtual ~SndDrvWav();
	Uint8 *NewBuffer(int slot);
	void DeleteBuffer(int slot);
	void Enable(BOOL flag);
	void SetRenderVolume(int level);
	Uint8 *Setup(void);
	Uint8 *Setup(char *p);
        void SetRate(int rate);
        int Render(Sint16 *pBuf, int start, int sSamples, BOOL clear, BOOL bZero);
        int Render(Sint32 *pBuf32, Sint16 *pBuf, int start, int sSamples, BOOL clear, BOOL bZero);
	int BZero(int start, int uSamples, int slot, BOOL clear);
        BOOL IsPlaying(void);
        BOOL IsEnabled(void);
private:
	void SetRenderVolume(int level, int slot);
	Uint32 bufSize;
        SDL_AudioSpec RawSpec;
	Sint16 *buf = NULL;
        Uint8 *RawBuf = NULL;
	int samples;
	UINT channels;
	UINT srate;
	UINT ms;
	UINT uStereo;
	int bufSlot;
	int nLevel; /* レンダリングの音量 */
	Uint8 volume; /* 出力する音量 */
	BOOL enable;
        SDL_AudioCVT cvt;
        Uint32 ppos;
        Uint32 plen;
        BOOL playing;
   
	SDL_sem *RenderSem;
};

#endif /* SNDDRVWAV_H_ */

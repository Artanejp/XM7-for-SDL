/*
 * SndDrvTmpl.cpp
 *
 *  Created on: 2010/09/25
 *      Author: K.Ohta <whatisthis.sowhat@gmail.com>
 */
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "xm7.h"


#include "SndDrvTmpl.h"


SndDrvTmpl::SndDrvTmpl() {
	uStereo = nStereoOut %4;
	channels = 2;
	ms = nSoundBuffer;
    srate = nSampleRate;
	enable = FALSE;
	counter = 0;
	nLevel = 32767;
	RenderSem = SDL_CreateSemaphore(1);
}

SndDrvTmpl::~SndDrvTmpl() {
	// TODO Auto-generated destructor stub
	enable = FALSE;
	if(RenderSem != NULL) {
		SDL_SemWait(RenderSem);
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
}

void SndDrvTmpl::SetRate(int rate)
{
	srate = rate;
}

void SndDrvTmpl::SetChannels(int c)
{
	channels = c;
}


void SndDrvTmpl::Setup(int tick)
{
	UINT uChannels;
	int i;
	uStereo = nStereoOut %4;
	uChannels = 2;
	channels = uChannels;
	ms = tick;

	enable = FALSE;
	counter = 0;
	return;
}

void SndDrvTmpl::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}

void SndDrvTmpl::Enable(BOOL flag)
{
	enable = flag;
}

/*
 * レンダリング
 */
int SndDrvTmpl::Render(Sint16 *pBuf, int start, int samples,  BOOL clear, BOOL bZero)
{

	SDL_SemWait(RenderSem);
	//	if(clear)  memset(buf, 0x00, size);

	/*
	 * ここにレンダリング関数ハンドリング
	 */
	/*
	 * ここではヌルレンダラ
	 */
	SDL_SemPost(RenderSem);
	return samples;
}


/*
 * SndDrvOpn.cpp
 *
 *  Created on: 2010/09/28
 *      Author: whatisthis
 */

#include "SndDrvOpn.h"

static int              l_vol[3][4] = {
		{	16,	23,	 9,	16	},
		{	16,	 9,	23,	 9	},
		{	16,	16,	16,	23	},
};
static int r_vol[3][4] = {
		{	16,	 9,	23,	16	},
		{	16,	23,	 9,	23	},
		{	16,	16,	16,	 9	},
};

void SndDrvOpn::CopySoundBufferGeneric(DWORD * from, WORD * to, int size)
{
        int         i, j, k;
        Sint32       *p = (Sint32 *) from;
        Sint16       *t = (Sint16 *) to;
        Sint32       tmp1;

        if (p == NULL) {
                return;
        }
        if (t == NULL) {
                return;
        }
#if 1
        i = (size / 4) * 4;
        for (j = 0; j < i; j += 4) {
                tmp1 = p[j];
//                tmp1 >>=1;
                t[j] = (Sint16) tmp1 ;
                tmp1 = p[j + 1];
//                tmp1 >>=1;
                t[j + 1] = (Sint16) tmp1;
                tmp1 = p[j + 2];
//                tmp1 >>=1;
                t[j + 2] = (Sint16) tmp1 ;
                tmp1 = p[j + 3];
//                tmp1 >>=1;
                t[j + 3] = (Sint16) tmp1;
        }
       k = size % 4;
        for (j = i; j < size; j++) {
                tmp1 = p[j];
//                tmp1 >>=1;
                t[j] = (Sint16)tmp1;
        }
#else
        for (j = 0; j < size; j++) {
                tmp1 = p[j];
                tmp1 >>=1;
                t[j] = (Sint16)tmp1;
        }

#endif
}


void SndDrvOpn::InitOpn(void)
{
	int opn;
	if(!pOPN) return;
	for(opn = 0; opn < 3 ; opn++) {
		pOPN[opn].Reset();
		pOPN[opn].Init(OPN_CLOCK * 100, srate, bFMHQmode, NULL);
		pOPN[opn].SetReg(0x27, 0);
		uCh3Mode[opn] = 0xff;
	}
}

BYTE SndDrvOpn::GetCh3Mode(int opn)
{
	if((opn<0) || (opn>3)) return 0;
	return uCh3Mode[opn];
}

void SndDrvOpn::SetCh3Mode(int opn, Uint8 dat)
{
	if((opn<0) || (opn>3)) return;
	uCh3Mode[opn] = dat;
}


void SndDrvOpn::SetReg(int opn, BYTE reg, BYTE dat)
{
	if((opn<0) || (opn>3)) return;
	pOPN[opn].SetReg((uint) reg,(uint) dat);
}

BYTE SndDrvOpn::GetReg(int opn, BYTE reg)
{
	if((opn<0) || (opn>3)) return 0;

	return 	opn_reg[opn][reg];

}


void SndDrvOpn::SetReg(int opn, BYTE *reg)
{
    int       i;

    /*
     * PSG
     */
    for (i = 0; i < 16; i++) {
    	pOPN[opn].SetReg((BYTE) i, reg[i]);
    }

    /*
     * FM音源キーオフ
     */
    for (i = 0; i < 3; i++) {
    	pOPN[opn].SetReg(0x28, (BYTE) i);
    }

    /*
     * FM音源レジスタ
     */
    for (i = 0x30; i < 0xb4; i++) {
    	pOPN[opn].SetReg((BYTE) i, reg[i]);
    }

    /*
     * FM音源動作モード
     */
    pOPN[opn].SetReg(0x27, reg[0x27] & 0xc0);
}



SndDrvOpn::SndDrvOpn(void) {
	// TODO Auto-generated constructor stub
	int i;
	bufSlot = DEFAULT_SLOT;
	for(i = 0; i<bufSlot; i++) {
		bufSize[i] = (ms * srate * channels *sizeof(Sint16)) / 1000;
		buf32[i] = NULL;
	}


	enable = TRUE;
	counter = 0;
	uChanSep = uChSeparation;
	for(i = 0; i<3; i++) {
		uCh3Mode[i] = 0;
	}
	volume = MIX_MAX_VOLUME;
	if(RenderSem == NULL) RenderSem = SDL_CreateSemaphore(1);
	SDL_SemPost(RenderSem);
	pOPN = new FM::OPN[3];
	InitOpn();
}

void SndDrvOpn::DeleteOpn(void)
{
	delete [] pOPN;
}

SndDrvOpn::~SndDrvOpn() {
	// TODO Auto-generated destructor stub
	if(RenderSem) SDL_SemWait(RenderSem);
	enable = FALSE;
	DeleteOpn();

}



/*
 * ボリューム設定: XM7/Win32 v3.4L30より
 */
void SndDrvOpn::SetRenderVolume(void)
{
	int ch;
	for(ch = 0; ch < 3; ch++) {
		SetRenderVolume(ch, (int)nFMVolume, (int)nPSGVolume);
	}
}

void SndDrvOpn::SetRenderVolume(int ch, int level)
{
	/* FM音源/PSGボリューム設定 */
	SetRenderVolume(ch, level, level);
}

void SndDrvOpn::SetRenderVolume(int ch, int fm, int psg)
{
	if((ch<0) || (ch>3)) return;
	/* FM音源/PSGボリューム設定 */
		if (pOPN != NULL) {
			pOPN[ch].SetVolumeFM(fm );
			pOPN[ch].psg.SetVolume(psg);
		}
		SetLRVolume();
}

void SndDrvOpn::SetLRVolume(void)
{
    l_vol[0][1] = l_vol[1][2] = l_vol[2][3] =
    r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = 16 + uChanSep;
    r_vol[0][1] = r_vol[1][2] = r_vol[2][3] =
    l_vol[1][1] = l_vol[0][2] = l_vol[1][3] = 16 - 	uChanSep ;
}

int *SndDrvOpn::GetLVolume(int num)
{
	if((num >3) || (num<0)) return NULL;
	return l_vol[num];
}

int *SndDrvOpn::GetRVolume(int num)
{
	if((num >3) || (num<0)) return NULL;
	return r_vol[num];
}

Uint8 *SndDrvOpn::NewBuffer(void)
{
	int i;
	for(i = 0; i< bufSlot; i++) {
		NewBuffer(i);
	}
	return buf[0];
}

Uint8 *SndDrvOpn::NewBuffer(int slot)
{
	int uChannels;

	if(slot >= bufSlot) return NULL;

    if(buf[slot] != NULL) {
    	return NULL; /* バッファがあるよ？Deleteしましょう */
    }
    if(buf32[slot] != NULL) {
    	return NULL; /* バッファがあるよ？Deleteしましょう */
    }



	uStereo = nStereoOut %4;
//    if ((uStereo > 0) || bForceStereo) {
//    	uChannels = 2;
//   } else {
//    	uChannels = 1;
//    }
	uChannels = 2;
	if(RenderSem == NULL) {
		return NULL;
	}
	SDL_SemWait(RenderSem);
    channels = uChannels;

    bufSize[slot] = (ms * srate * channels *sizeof(Sint16)) / 1000;
	buf[slot] = (Uint8 *)malloc((ms * srate * channels *sizeof(Sint16)) / 1000);
	if(buf[slot] == NULL) return NULL; /* バッファ取得に失敗 */
	buf32[slot] = (Uint32 *)malloc((ms * srate * channels *sizeof(Sint32)) / 1000);
	if(buf32[slot] == NULL) {
		free(buf[slot]);
		return NULL;
	}
	memset(buf[slot], 0x00, bufSize[slot]); /* 初期化 */
	memset(buf32[slot], 0x00, bufSize[slot] * 2); /* 初期化 */
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = bufSize[slot];
	chunk[slot].allocated = 1; /* アロケートされてる */
	chunk[slot].volume = volume; /* 一応最大 */
	enable = TRUE;
	counter = 0;
	SDL_SemPost(RenderSem);
	return buf[slot];
}

void SndDrvOpn::DeleteBuffer(void)
{
	int i;
	for(i = 0; i <bufSlot ; i++)
	{
		DeleteBuffer(i);
	}
}

void SndDrvOpn::DeleteBuffer(int slot)
{
	enable = FALSE;
	if(slot > bufSlot) return;
	if(RenderSem == NULL) return;
	SDL_SemWait(RenderSem);
	if(buf[slot] != NULL) free(buf[slot]);
	buf[slot] = NULL;
	if(buf32[slot] != NULL) free(buf32[slot]);
	buf32[slot] = NULL;

	chunk[slot].abuf = NULL;
	chunk[slot].alen = 0;
	chunk[slot].allocated = 0; /* アロケートされてる */
	chunk[slot].volume = 0;
	SDL_SemPost(RenderSem);
}




Uint8  *SndDrvOpn::Setup(int tick)
{
	UINT uChannels;
	int i;

	uStereo = nStereoOut %4;
	uChannels = 2;
    channels = uChannels;
	   if(tick > 0) {
		   ms = (UINT)tick;
	   } else {
		   ms = nSoundBuffer;
	   }
	   srate = nSampleRate;
		uChanSep = uChSeparation;

	   InitOpn();
	   for(i = 0; i < bufSlot; i++) {
		   if(buf[i] == NULL) {
				bufSize[i] = (ms * srate * channels * sizeof(Sint16)) / 1000 ;
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
	   }
	   return buf[0];
}


int SndDrvOpn::GetBufSlots(void)
{
	return bufSlot;
}


/*
 * BZERO : 指定領域を0x00で埋める
 */
int SndDrvOpn::BZero(int start, int uSamples, int slot, BOOL clear)
{
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	DWORD *q;
	s = (ms * srate)/1000;
	if(slot > bufSlot) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(RenderSem == NULL) {
		return 0;
	}
	SDL_SemWait(RenderSem);
	q = buf32[slot];
	q = &q[start * channels];
	if((start <= 0) &&  (clear != TRUE)){
		memset(buf32[slot], 0x00, (ms * srate * channels * sizeof(Sint32)) / 1000 - sizeof(Sint32));
		memset(buf[slot], 0x00, (ms * srate * channels * sizeof(Sint16)) / 1000 - sizeof(Sint16));
	}
	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	memset(q, 0, sizeof(DWORD) * ss2 * channels);
	SDL_SemPost(RenderSem);
	return ss2;

}

DWORD *SndDrvOpn::GetBuf32(int slot)
{
	return (DWORD *)buf32[slot];
}


/*
 * レンダリング
 */
int SndDrvOpn::Render(int start, int uSamples, int slot, BOOL clear)
{
	int sSamples = uSamples;
	int s;
	int ss,ss2;
	int opn;
	Uint32 *q;

	s = (ms * srate)/1000;
	if(slot > bufSlot) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;

	q = buf32[slot];
	q = &q[start * channels];
	if((start <= 0) && (clear!=TRUE)){
		memset(buf32[slot], 0x00, (ms * srate * channels * sizeof(Sint32)) / 1000 - sizeof(Sint32));
		memset(buf[slot], 0x00, (ms * srate * channels * sizeof(Sint16)) / 1000 - sizeof(Sint16));
	}

	ss = sSamples + start;
	if(ss > s) {
		ss2 = s - start;
	} else {
		ss2 = sSamples;
	}
	if(ss2 <= 0) return 0;
	if(RenderSem == NULL) {
		return 0;
	}
	SDL_SemWait(RenderSem);
	if(clear)         memset(q, 0, sizeof(DWORD) * ss2 * channels);
	if(enable) {
		if (channels == 1) {
			/* モノラル */
			pOPN[OPN_STD].Mix((int32*)q, ss2);
			if (whg_use){
				pOPN[OPN_WHG].Mix((int32*)q, ss2);
			}
			if (thg_use) {
				pOPN[OPN_THG].Mix((int32*)q, ss2);
			}
			if ((!thg_use) && (fm7_ver == 1) ) {
				pOPN[OPN_STD].psg.Mix((int32*)q, ss2);
			}
		} else {
			/* ステレオ */
			if (!whg_use && !thg_use) {
				/* WHG/THGを使用していない(強制モノラル) */
				pOPN[OPN_STD].Mix2((int32*)q, ss2, 16, 16);
				if (fm7_ver == 1) {
					pOPN[OPN_STD].psg.Mix2((int32*)q, ss2, 16, 16);
				}
			}
			else {
				/* WHGまたはTHGを使用中 */
				pOPN[OPN_STD].Mix2((int32*)q, ss2,
						l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
				if (whg_use){
					pOPN[OPN_WHG].Mix2((int32*)q, ss2,
							l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
				}
				if (thg_use) {
					pOPN[OPN_THG].Mix2((int32*)q, ss2,
							l_vol[OPN_THG][uStereo], r_vol[OPN_THG][uStereo]);
				}
				else if (fm7_ver == 1){
					pOPN[OPN_THG].psg.Mix2((int32*)q, ss2, 16, 16);
				}
			}
		}
//      	  CopySoundBufferGeneric((DWORD *)q, (WORD *)p, (int)(ss2 * channels));
		/*
		 * ここにレンダリング関数ハンドリング
		 */
	}
	SDL_SemPost(RenderSem);
	return ss2;
}

void SndDrvOpn::Play(int ch,  int slot, int samples)
{
		if(slot >= bufSlot) return;
		//if(chunk[slot].abuf == NULL) return;
		//if(chunk[slot].alen <= 0) return;
		if(!enable) return;
		if(RenderSem == NULL) return;
		SDL_SemWait(RenderSem);
        if(samples >0) {
      	  CopySoundBufferGeneric((DWORD *)buf32[slot], (WORD *)buf[slot], (int)(samples * channels));
      	  chunk[slot].abuf = buf[slot];
      	  chunk[slot].alen = (Uint32)(samples * channels * sizeof(Sint16));
      	  chunk[slot].allocated = 1;
      	  chunk[slot].volume = volume;
      	  Mix_PlayChannel(ch, &chunk[slot], 0);
        }
		SDL_SemPost(RenderSem);
}

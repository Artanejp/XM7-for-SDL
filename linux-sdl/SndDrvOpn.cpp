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
        int         i,j;
        Sint32       *p = (Sint32 *) from;
        Sint16       *t = (Sint16 *) to;
        Sint32       tmp1;

        if (p == NULL) {
                return;
        }
        if (t == NULL) {
                return;
        }
        i = (size / 4) * 4;
#if 1
        for (j = 0; j < i; j += 4) {
                tmp1 = p[j];
                t[j] = (Sint16) tmp1 ;
                tmp1 = p[j + 1];
                t[j + 1] = (Sint16) tmp1;
                tmp1 = p[j + 2];
                t[j + 2] = (Sint16) tmp1 ;
                tmp1 = p[j + 3];
                t[j + 3] = (Sint16) tmp1;
        }
       i = size - i;
        i = size;
        for (j = 0; j < i; j++) {
                tmp1 = p[j];
                t[j] = (int16)tmp1;
        }
#endif
}


void SndDrvOpn::InitOpn(void)
{
	if(!pOPN) return;
	pOPN->Reset();
	pOPN->Init(OPN_CLOCK * 100, srate, bFMHQmode, NULL);
    pOPN->SetReg(0x27, 0);
    uCh3Mode = 0xff;
}

BYTE SndDrvOpn::GetCh3Mode(void)
{
	return uCh3Mode;
}

void SndDrvOpn::SetCh3Mode(Uint8 dat)
{
	uCh3Mode = dat;
}


void SndDrvOpn::SetReg(BYTE reg, BYTE dat)
{
	pOPN->SetReg((uint) reg,(uint) dat);
}

BYTE SndDrvOpn::GetReg(BYTE reg)
{

	return 	opn_reg[opn_number][reg];

}


void SndDrvOpn::SetReg(BYTE *reg)
{
    int       i;

    /*
     * PSG
     */
    for (i = 0; i < 16; i++) {
    	pOPN->SetReg((BYTE) i, reg[i]);
    }

    /*
     * FM音源キーオフ
     */
    for (i = 0; i < 3; i++) {
    	pOPN->SetReg(0x28, (BYTE) i);
    }

    /*
     * FM音源レジスタ
     */
    for (i = 0x30; i < 0xb4; i++) {
    	pOPN->SetReg((BYTE) i, reg[i]);
    }

    /*
     * FM音源動作モード
     */
    pOPN->SetReg(0x27, reg[0x27] & 0xc0);
}



SndDrvOpn::SndDrvOpn(void) {
	// TODO Auto-generated constructor stub
	int i;
	bufSlot = DEFAULT_SLOT;
	bufSize = (ms * srate * channels *sizeof(Sint16)) / 1000;
	for(i = 0; i<bufSlot; i++) {
		buf32[i] = NULL;
	}


	enable = TRUE;
	counter = 0;
	uChanSep = uChSeparation;
	uCh3Mode = 0;
	opn_number = 0;
	volume = MIX_MAX_VOLUME;
	if(RenderSem == NULL) RenderSem = SDL_CreateSemaphore(1);
	SDL_SemPost(RenderSem);
	pOPN = new FM::OPN;
	InitOpn();
}

void SndDrvOpn::DeleteOpn(void)
{
	delete pOPN;
}
SndDrvOpn::~SndDrvOpn() {
	// TODO Auto-generated destructor stub
	if(RenderSem) SDL_SemWait(RenderSem);
	enable = FALSE;
	DeleteOpn();

}


void SndDrvOpn::SetOpNo(int num)
{
	opn_number = num;
}

/*
 * ボリューム設定: XM7/Win32 v3.4L30より
 */
void SndDrvOpn::SetRenderVolume(void)
{
	SetRenderVolume((int)nFMVolume, (int)nPSGVolume);
}

void SndDrvOpn::SetRenderVolume(int level)
{
	/* FM音源/PSGボリューム設定 */
	SetRenderVolume(level, level);
}

void SndDrvOpn::SetRenderVolume(int fm, int psg)
{
	/* FM音源/PSGボリューム設定 */
		if (pOPN) {
			pOPN->SetVolumeFM(fm * 2);
			pOPN->psg.SetVolume(psg * 2);
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

int SndDrvOpn::GetLevelSnd(int ch)
{
	        FM::OPN *p;
	        int     i;
	        double  s;
	        double  t;
	        int     *buf;

	        ASSERT((ch >= 0) && (ch < 18));

	/*
	 * OPN,WHGの区別
	 */
	        p = pOPN;

	/*
	 * 存在チェック
	 */
	        if (!p) {
	                return 0;
	        }

	/*
	 * FM,PSGの区別
	 */
	        if (ch < 3) {
	                /*
	                 * FM:512サンプルの2乗和を計算
	                 */
	                buf = p->rbuf[ch];
	                s = 0;
	                for (i = 0; i < 512; i++) {
	                        t = (double) *buf++;
	                        t *= t;
	                        s += t;
	                }
	                s /= 512;

	                /*
	                 * ゼロチェック
	                 */
	                if (s == 0) {
	                        return 0;
	                }

	                /*
	                 * log10を取る
	                 */
	                s = log10(s);

	                /*
	                 * FM音源補正
	                 */
	                s *= 40.0;
	        } else {

	                /*
	                 * PSG:512サンプルの2乗和を計算
	                 */
	                buf = p->psg.rbuf[ch - 3];
	                s = 0;
	                for (i = 0; i < 512; i++) {
	                        t = (double) *buf++;
	                        t*= t;
	                        s += t;
	                }
	                s /= 512;

	                /*
	                 * ゼロチェック
	                 */
	                if (s == 0) {
	                        return 0;
	                }

	                /*
	                 * log10を取る
	                 */
	                s = log10(s);

	                /*
	                 * PSG音源補正
	                 */
	                s *= 60.0;
	        }
	        return (int) s;
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
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
	if(RenderSem == NULL) {
		return NULL;
	}
	SDL_SemWait(RenderSem);
    channels = uChannels;

	buf[slot] = (Uint8 *)malloc(bufSize);
	if(buf[slot] == NULL) return NULL; /* バッファ取得に失敗 */
	buf32[slot] = (Uint32 *)malloc(bufSize * 2);
	if(buf32[slot] == NULL) {
		free(buf[slot]);
		return NULL;
	}
	memset(buf[slot], 0x00, bufSize); /* 初期化 */
	memset(buf32[slot], 0x00, bufSize * 2); /* 初期化 */
	chunk[slot].abuf = buf[slot];
	chunk[slot].alen = bufSize;
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



Uint8 *SndDrvOpn::Setup(int tick)
{
	int opno = opn_number;
	return Setup(tick, opno);
}

Uint8  *SndDrvOpn::Setup(int tick, int opno)
{
	UINT uChannels;
	int i;

	uStereo = nStereoOut %4;
    if ((uStereo > 0) || bForceStereo) {
    	uChannels = 2;
    } else {
    	uChannels = 1;
    }
    channels = uChannels;
	   if(tick > 0) {
		   ms = (UINT)tick;
	   } else {
		   ms = nSoundBuffer;
	   }
	   srate = nSampleRate;
		bufSize = (ms * srate * channels * sizeof(Sint16)) / 1000 ;
		uChanSep = uChSeparation;

	   SetOpNo(opno);
	   InitOpn();
	   for(i = 0; i < bufSlot; i++) {
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
	Uint32 *q;

	s = (ms * srate)/1000;
	if(slot > bufSlot) return 0;
	if(start > s) return 0; /* 開始点にデータなし */
	if(sSamples > s) sSamples = s;

	q = buf32[slot];
	q = &q[start * channels];

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
#ifdef ROMEO
                        if ((pOPN) && !bUseRomeo) {
#else
                                if (pOPN) {
#endif
                                        pOPN->Mix((int32*)q, ss2);
                                }
                                if ((!thg_use) && (fm7_ver == 1) && (opn_number == OPN_THG)) {
                                        pOPN->psg.Mix((int32*)q, ss2);
                                }
                        }
                        else {
                                /* ステレオ */
                                if (!whg_use && !thg_use) {
                                        /* WHG/THGを使用していない(強制モノラル) */
#ifdef ROMEO
                                        if (!bUseRomeo) {
                                                pOPN[OPN_STD]->Mix2((int32*)q, ss2, 16, 16);
                                        }
#else
                                        if(opn_number == OPN_STD) {
                                        	pOPN->Mix2((int32*)q, ss2, 16, 16);
                                        }
#endif
                                        if ((fm7_ver == 1) && (opn_number == OPN_THG)) {
                                                pOPN->psg.Mix2((int32*)q, ss2, 16, 16);
                                        }
                                }
                                else {
                                        /* WHGまたはTHGを使用中 */
#ifdef ROMEO
                                        if (!bUseRomeo) {
                                                pOPN[OPN_STD]->Mix2((int32*)q, ss2,
                                                                    l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
                                        }
#else
                                        if(opn_number == OPN_STD) {
                                        	pOPN->Mix2((int32*)q, ss2,
                                        			l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
                                        }
#endif
                                        if ((whg_use)  && (opn_number == OPN_WHG)){
#ifdef ROMEO
                                                if (bUseRomeo) {
                                                        pOPN[OPN_WHG]->psg.Mix2((int32*)q, samples,
                                                                                l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
                                                }
                                                else {
                                                        pOPN[OPN_WHG]->Mix2((int32*)q, samples,
                                                                            l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
                                                }
#else
                                                pOPN->Mix2((int32*)q, ss2,
                                                                    l_vol[OPN_WHG][uStereo], r_vol[OPN_WHG][uStereo]);
#endif
                                        }
                                        if ((thg_use) && (opn_number == OPN_THG)) {
                                                pOPN->Mix2((int32*)q, ss2,
                                                                    l_vol[OPN_THG][uStereo], r_vol[OPN_THG][uStereo]);
                                        }
                                        else if ((fm7_ver == 1) && (opn_number == OPN_THG)){
                                                pOPN->psg.Mix2((int32*)q, ss2, 16, 16);
                                        }
                                }
                        }

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
		if(chunk[slot].abuf == NULL) return;
		if(chunk[slot].alen <= 0) return;
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

/*
 * SndDrvOpn.cpp
 *
 *  Created on: 2010/09/28
 *      Author: whatisthis
 */

#include "SndDrvOpn.h"
#include "api_snd.h"
#include "xm7_types.h"

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

static inline Sint16 _clamp(Sint32 b)
{
    if(b < -0x7fff) return -0x7fff;
    if(b > 0x7fff) return 0x7fff;
    return (Sint16) b;
}


void SndDrvOpn::CopySoundBufferGeneric(DWORD * from, WORD * to, int size)
{
    int         i, j;
    Sint32       *p = (Sint32 *) from;
    Sint16       *t = (Sint16 *) to;
    register Sint32       tmp1;
    v4hi   *l;
    v8hi *h;
    register v8hi tmp2;
    register v4hi tmp3;
    

    if (p == NULL) {
        return;
    }
    if (t == NULL) {
        return;
    }
    h = (v8hi *)p;
    l = (v4hi *)t;
    i = (size >> 3) << 3;
    
    for (j = 0; j < i; j += 8) {
        tmp2 = *h++;
        tmp3.ss[0] =_clamp(tmp2.si[0]);
        tmp3.ss[1] =_clamp(tmp2.si[1]);
        tmp3.ss[2] =_clamp(tmp2.si[2]);
        tmp3.ss[3] =_clamp(tmp2.si[3]);
        tmp3.ss[4] =_clamp(tmp2.si[4]);
        tmp3.ss[5] =_clamp(tmp2.si[5]);
        tmp3.ss[6] =_clamp(tmp2.si[6]);
        tmp3.ss[7] =_clamp(tmp2.si[7]);
        *l++ = tmp3;
        }
        p = (Sint32 *)h;
        t = (Sint16 *)l;
        if(i >= size) return;
        for (j = 0; j < (size - i); j++) {
            tmp1 = *p++;
	    *t++ = _clamp(tmp1);
        }
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
	uStereo = nStereoOut %4;
    channels = 2;
	ms = nSoundBuffer;
    srate = nSampleRate;

	enable = TRUE;
	counter = 0;
	uChanSep = uChSeparation;
	nLevel = 32767;

	for(i = 0; i<3; i++) {
		uCh3Mode[i] = 0;
	}
	RenderSem = SDL_CreateSemaphore(1);
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
	if(RenderSem) {
		SDL_SemWait(RenderSem);
		SDL_DestroySemaphore(RenderSem);
		RenderSem = NULL;
	}
	enable = FALSE;
	DeleteOpn();
}


void SndDrvOpn::SetChannels(int c)
{
	channels = c;
}

void SndDrvOpn::SetRate(int rate)
{
    srate = rate;
    SetRate(0, OPN_CLOCK * 100, rate, bFMHQmode);
    SetRate(1, OPN_CLOCK * 100, rate, bFMHQmode);
    SetRate(2, OPN_CLOCK * 100, rate, bFMHQmode);
}



void SndDrvOpn::SetRate(int ch, unsigned int clk, int rate, BOOL hq)
{
   if(pOPN != NULL) {
	   pOPN[ch].SetRate(clk, rate, hq);
   }
}


void SndDrvOpn::SetRenderVolume(int level)
{
	nLevel = (int)(32767.0 * pow(10.0, level / 20.0));
}

void SndDrvOpn::Enable(BOOL flag)
{
	enable = flag;
}

/*
 * 互換のためのダミー関数
 */
void SndDrvOpn::SetState(BOOL state)
{

}

/*
 * 互換のためのダミー関数
 */
void SndDrvOpn::SetFreq(int f)
{

}

/*
 * 互換のためのダミー関数
 */
void SndDrvOpn::ResetCounter(BOOL flag)
{

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
			pOPN[ch].SetVolumePSG(psg);
		}
		SetLRVolume();
}

void SndDrvOpn::SetLRVolume(void)
{
    l_vol[0][0] = l_vol[0][1] = l_vol[1][2] = l_vol[2][3] =
    r_vol[1][1] = r_vol[0][2] = r_vol[1][3] = 16 + uChanSep;
    r_vol[0][0] = r_vol[0][1] = r_vol[1][2] = r_vol[2][3] =
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



void SndDrvOpn::Setup(int tick)
{
	UINT uChannels;

	uStereo = nStereoOut %4;
	uChannels = 2;
    channels = uChannels;
	enable = FALSE;
	counter = 0;

    if(tick > 0) {
	   ms = (UINT)tick;
	} else {
	   ms = nSoundBuffer;
	}
    uChanSep = uChSeparation;
    InitOpn();
    return;
}

void SndDrvOpn::Setup(int tick, int opno)
{
    Setup(tick);
}



/*
 * レンダラ(32bitバッファ)
 */

int SndDrvOpn::Render32(Sint32 *pBuf32, int start, int sSamples, BOOL clear,BOOL bZero)
{

	int ss2;
	Uint32 *q;

	if(pBuf32 == NULL) return 0;

	q = (Uint32 *)pBuf32;
	q = &q[start * channels];
	ss2 = sSamples;
	if(ss2 <= 0) return 0;
	if(RenderSem == NULL) {
		return 0;
	}
	SDL_SemWait(RenderSem);
        memset(q, 0x00, sizeof(DWORD) * ss2 * channels);

	if(enable) {
		if(bZero) {
			memset(q, 0x00, sizeof(DWORD) * ss2 * channels);
			SDL_SemPost(RenderSem);
		        RenderCounter += ss2;
			return ss2;
		}

			/* ステレオ */
			if (!whg_use && !thg_use) {
			   pOPN[OPN_STD].Mix2((int32*)q, ss2, l_vol[OPN_STD][uStereo], r_vol[OPN_STD][uStereo]);
				if (fm7_ver == 1) { // PSG
					pOPN[OPN_THG].psg.Mix2((int32*)q, ss2, 16, 16);
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
				else if (fm7_ver == 1){  // PSG
					pOPN[OPN_THG].psg.Mix2((int32*)q, ss2, 16, 16);
				}
			}
//      	  CopySoundBufferGeneric((DWORD *)q, (WORD *)p, (int)(ss2 * channels));
		/*
		 * ここにレンダリング関数ハンドリング
		 */
	}
	SDL_SemPost(RenderSem);
	RenderCounter += ss2;
	return ss2;
}




int SndDrvOpn::Render(Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero)
{
	int r;
	Sint32 *pBuf32;

	if(pBuf == NULL) return 0;
	pBuf32 =(Sint32 *)malloc(sSamples * channels * sizeof(Sint32));
	if(pBuf32 == NULL) return 0;
	r = Render32(pBuf32, start, sSamples, clear, bZero);
	if(r> 0){
		Copy32(pBuf32, pBuf, start, r);
	} else {
		r = 0;
	}

	free(pBuf32);
	return r;
}

int SndDrvOpn::Render(Sint32 *pBuf32, Sint16 *pBuf, int start, int sSamples, BOOL clear,BOOL bZero)
{
	int r;
	if(pBuf32 == NULL) return 0;
	if(pBuf == NULL) return 0;

        r = Render32(pBuf32, start, sSamples, clear, bZero);
//	if(r > 0) Copy32(pBuf32, pBuf, start, r);
	Copy32(pBuf32, pBuf, start, sSamples);
	return r;
}

void SndDrvOpn::Copy32(Sint32 *src, Sint16 *dst, int ofset, int samples)
{
	DWORD *p;
	WORD *q;

	if((samples <= 0) || (ofset < 0))return;
    if(RenderSem == NULL) return;
    SDL_SemWait(RenderSem);

	p = (DWORD *)src;
	p = &(p[ofset * channels]);
	q = (WORD *)dst;
	q = &(q[ofset * channels]);
//	if(ofset <= 0){
//		memset(dst, 0x00, (ms * srate * channels * sizeof(Sint16)) / 1000);
//	}

//	memset(q, 0x00 , samples * channels * sizeof(Sint16));
	CopySoundBufferGeneric(p, q, (int)(samples * channels));
    SDL_SemPost(RenderSem);
}

int SndDrvOpn::GetRenderCounter(void)
{
    return RenderCounter;
}

void SndDrvOpn::ResetRenderCounter(void)
{
    RenderCounter = 0;
}

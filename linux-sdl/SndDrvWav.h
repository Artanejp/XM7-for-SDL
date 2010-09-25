/*
 * SndDrvWav.h
 *
 *  Created on: 2010/09/25
 *      Author: whatisthis
 */

#ifndef SNDDRVWAV_H_
#define SNDDRVWAV_H_

class SndDrvWav: public SndDrvTmpl {
public:
	SndDrvWav();
	virtual ~SndDrvWav();
	void Render(int msec, BOOL clear);
	Uint8 *NewBuffer(void);
	void DeleteBuffer(void);
private:
	Uint8 *buf;
	int bufSize;
	int ms;
	int channels;
	int playCh;
	int srate;
	int howlong; /* 実際の演奏秒数 */
	Mix_Chunk chunk;
	Mix_Chunk *chunkP;
	BOOL enable;
	int counter;
	struct _WAVDATA {
	    short          *p;  /* 波形データポインタ */
	    DWORD       size;   /* データサイズ(サンプル数) */

	    DWORD       freq;   /* サンプリング周波数 */
	} Wav[3];
	struct _WAVPLAY {
	    BOOL        bPlay;          /* WAV再生フラグ */
	    DWORD       dwWaveNo;       /* WAVでーたなんばー */
	    DWORD       dwCount1;       /* WAVでーたかうんた(整数部) */
	    DWORD       dwCount2;       /* WAVでーたかうんた(小数部) */
	    DWORD       dwCount3;       /* WAV再生かうんた */
	} WavP[SNDBUF];
	static char     *WavName[] = {
	 /* WAVファイル名 */
	    "RELAY_ON.WAV",
	    "RELAY_OFF.WAV",
	    "FDDSEEK.WAV",
	    NULL,
	    NULL
	#if 0
	    "HEADUP.WAV",
	    "HEADDOWN.WAV"
	#endif  /* */
	};


};

#endif /* SNDDRVWAV_H_ */

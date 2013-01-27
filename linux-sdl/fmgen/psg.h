// ---------------------------------------------------------------------------
//	PSG-like sound generator
//	Copyright (C) cisc 1997, 1999.
// ---------------------------------------------------------------------------
//	$Id: psg.h,v 1.6 2000/09/08 13:45:57 cisc Exp $

#ifndef PSG_H
#define PSG_H

#include "cisc.h"

#define PSG_SAMPLETYPE		int32		// int32 or int16
//#define PSG_SAMPLETYPE		int16		// int32 or int16
#define PSG_IPSCALE			16384
#define PSG_INTERPOLATE(y, x)	\
	(((((((-y[0]+3*y[1]-3*y[2]+y[3]) * x + PSG_IPSCALE/2) / PSG_IPSCALE \
	+ 3 * (y[0]-2*y[1]+y[2])) * x + PSG_IPSCALE/2) / PSG_IPSCALE \
	- 2*y[0]-3*y[1]+6*y[2]-y[3]) * x + 3*PSG_IPSCALE) / (6*PSG_IPSCALE) + y[1])

// ---------------------------------------------------------------------------
//	class PSG
//	PSG に良く似た音を生成する音源ユニット
//	
//	interface:
//	bool SetClock(uint clock, uint rate, bool ipflag)
//		初期化．このクラスを使用する前にかならず呼んでおくこと．
//		PSG のクロックや PCM レートを設定する
//		(オリジナルfmgenに対してipflagが追加されているのに注意)
//
//		clock:	PSG の動作クロック
//		rate:	生成する PCM のレート
//		ipflag:	線形補間ON/OFF
//		retval	初期化に成功すれば true
//
//	void Mix(Sample* dest, int nsamples)
//		PCM を nsamples 分合成し， dest で始まる配列に加える(加算する)
//		あくまで加算なので，最初に配列をゼロクリアする必要がある
//	
//	void Reset()
//		リセットする
//
//	void SetReg(uint reg, uint8 data)
//		レジスタ reg に data を書き込む
//	
//	uint GetReg(uint reg)
//		レジスタ reg の内容を読み出す
//	
//	void SetVolume(int db)
//		各音源の音量を調節する
//		単位は約 1/2 dB
//
class PSG
{
public:
	typedef PSG_SAMPLETYPE Sample;
	
	enum
	{
		noisetablesize = 1 << 11,	// ←メモリ使用量を減らしたいなら減らして
		toneshift = 24,
		envshift = 22,
		noiseshift = 14,
		oversampling = 2,		// ← 音質より速度が優先なら減らすといいかも
	};

public:
	PSG();
	~PSG();

	void Mix(Sample* dest, int nsamples);
	void Mix2(Sample* dest, int nsamples, int vol_l, int vol_r);
	void SetClock(int clock, int rate, bool ipflag);

	void SetVolume(int vol);
	void SetChannelMask(int c);

	void Reset();
	void SetReg(uint regnum, uint8 data);
	uint GetReg(uint regnum) { return reg[regnum & 0x0f]; }

	int rcnt;
	int32 rbuf[3][512];

protected:
	void MakeNoiseTable();
	void MakeEnvelopTable();
	static void StoreSample(Sample& dest, int32 data);
	
	uint8 reg[16];

	const uint* envelop;
	uint olevel[3];
	uint32 scount[3], speriod[3];
	uint32 ecount, eperiod;
	uint32 ncount, nperiod;
	uint32 tperiodbase;
	uint32 eperiodbase;
	uint32 nperiodbase;
	int volume;
	int mask;

	static uint enveloptable[16][64];
	static uint noisetable[noisetablesize];
	static int EmitTable[32];
	static bool table_initialize;

	uint	psgrate;

	// 補間モード用
	bool	interpolation;
	int32	mixdelta;
	int		mpratio;
	int32	mb[4];
};

#endif // PSG_H

// JULIET.CPP
// Programmed by ROMEOユーザー数名 / and GORRY.

// 2004.08.13		OPN/WHG/THGのopn.cへの統合に合わせた変更をおこなう
//					ROMEO出力時にWHG部(OPN3 ch.4-6)のプリスケーラがOPN部
//					(OPN3 ch.1-3)の設定に引きずられる問題を修正

#ifdef ROMEO

#include	<windows.h>
#include	<math.h>
#include	"romeo.h"
#include	"juliet.h"

/* XM7 header file by RHG */
#include	"xm7.h"
#include	"opn.h"
#include	"w32.h"
#include	"w32_sch.h"

#define ROMEO_CLK	(1000000.0 * 4.0)	/* ROMEO マスタークロック */
#define	CLKCONV		1					/* 音程変換使用フラグ */
#define	USEOPM		0					/* OPM使用フラグ */


#ifndef SUCCESS
#define	SUCCESS		0
#endif

#ifndef FAILURE
#define	FAILURE		(!SUCCESS)
#endif

enum {
	ROMEO_AVAIL			= 0x01,
	ROMEO_YMF288		= 0x02,			// 必ず存在する筈？
	ROMEO_YM2151		= 0x04
};

typedef struct {
	HMODULE			mod;

	PCIFINDDEV		finddev;
	PCICFGREAD32	read32;
	PCIMEMWR8		out8;
	PCIMEMWR16		out16;
	PCIMEMWR32		out32;
	PCIMEMRD8		in8;
	PCIMEMRD16		in16;
	PCIMEMRD32		in32;

	ULONG			addr;
	ULONG			irq;
	ULONG			avail;
	ULONG			snoopcount;

#if USEOPM
	BYTE			YM2151_outop[8];
	BYTE			YM2151_ttl[8*4];
#endif

	BYTE			YMF288_outop[8];
	BYTE			YMF288_ttl[8*4];
	BYTE			YMF288_PSG;
} _ROMEO;


#define	ROMEO_TPTR(member)	(int)&(((_ROMEO *)NULL)->member)


typedef struct {
	char	*symbol;
	int		addr;
} DLLPROCESS;


static const DLLPROCESS	dllproc[] = {
				{FN_PCIFINDDEV,		ROMEO_TPTR(finddev)},
				{FN_PCICFGREAD32,	ROMEO_TPTR(read32)},
				{FN_PCIMEMWR8,		ROMEO_TPTR(out8)},
				{FN_PCIMEMWR16,		ROMEO_TPTR(out16)},
				{FN_PCIMEMWR32,		ROMEO_TPTR(out32)},
				{FN_PCIMEMRD8,		ROMEO_TPTR(in8)},
				{FN_PCIMEMRD16,		ROMEO_TPTR(in16)},
				{FN_PCIMEMRD32,		ROMEO_TPTR(in32)}
};

static	_ROMEO		romeo = {NULL};

static	const	BYTE	FMoutop[] = {
	0x08, 0x08, 0x08, 0x08, 0x0c, 0x0e, 0x0e, 0x0f
};
#if USEOPM
static	char	YM2151vol = 0;
#endif

typedef struct {
	DWORD	Time;
	WORD	Addr;
	BYTE	Data;
} _CmnBuf;

//
//	OPMコマンドバッファ関係 byうさ
//
#if USEOPM
#define	OPMCMNDBUFSIZE	65535
static	_CmnBuf			OpmCmnBuf[OPMCMNDBUFSIZE+1];
static	volatile int	OpmNumCmnd;
static	int				OpmCmndReadIdx,OpmCmndWriteIdx;
#endif

//
//	OPN3Lコマンドバッファ関係 byうさ
//
#define	OPNCMNDBUFSIZE	65535
static	_CmnBuf			OpnCmnBuf[OPNCMNDBUFSIZE+1];
static	volatile int	OpnNumCmnd;
static	int				OpnCmndReadIdx,OpnCmndWriteIdx;

//
//	MD 音程変換関係
//
#if CLKCONV

//						        SLT FLG/DATA
static	unsigned char	fnum_buff[32];
static	unsigned char	psg_buff[16];
static	BYTE			fnum_tbl[2048 * 2];
static	WORD			psgfreq_tbl[80530];
static	int				Prescaler[2];

/* 音声合成モードサポート by RHG */
static	BOOL			CSMmode;
#endif


BOOL juliet_load(void)
{
		int			i;
const	DLLPROCESS	*dp;
		BOOL		r = SUCCESS;

	juliet_unload();

	romeo.mod = LoadLibrary(PCIDEBUG_DLL);
	if (romeo.mod == NULL) {
		return(FAILURE);
	}
	for (i=0, dp=dllproc; i<sizeof(dllproc)/sizeof(DLLPROCESS); i++, dp++) {
		FARPROC proc;
		proc = GetProcAddress(romeo.mod, dp->symbol);
		if (proc == NULL) {
			r = FAILURE;
			break;
		}
		*(DWORD *)(((BYTE *)&romeo) + (dp->addr)) = (DWORD)proc;
	}
	if (r) {
		juliet_unload();
	}
	return(r);
}


void juliet_unload(void)
{
	if (romeo.mod) {
		FreeLibrary(romeo.mod);
	}
	ZeroMemory(&romeo, sizeof(romeo));
#if USEOPM
	FillMemory(romeo.YM2151_ttl, 8*4, 0x7f);
#endif
	FillMemory(romeo.YMF288_ttl, 8*4, 0x7f);
	romeo.YMF288_PSG = 0x3f;
}


// ----

// pciFindPciDevice使うと、OS起動後一発目に見つけられないことが多いので、
// 自前で検索する
static ULONG searchRomeo(void)
{
	int bus, dev, func;

	for (bus=0; bus<256; bus++) {
		for (dev=0; dev<32; dev++) {
			for (func=0; func<8; func++) {
				ULONG addr = pciBusDevFunc(bus, dev, func);
				ULONG dev_vend = romeo.read32(addr, 0x0000);

				if ( (dev_vend&0xffff)!=ROMEO_VENDORID ) {
					continue;
				}

				dev_vend >>= 16;
				if ( (dev_vend==ROMEO_DEVICEID) ||
					 (dev_vend==ROMEO_DEVICEID2) ) {
					return addr;
				}
			}
		}
	}
	return ((ULONG)0xffffffff);
}


BOOL juliet_prepare(void)
{
	ULONG	pciaddr;

	if (romeo.mod == NULL) {
		return(FAILURE);
	}

	pciaddr = searchRomeo();
	if ( pciaddr!=(ULONG)0xffffffff ) {
		romeo.addr = romeo.read32(pciaddr, ROMEO_BASEADDRESS1);
		romeo.irq  = romeo.read32(pciaddr, ROMEO_PCIINTERRUPT) & 0xff;
		if (romeo.addr) {
			romeo.avail = ROMEO_AVAIL | ROMEO_YMF288;
#if USEOPM
			juliet_YM2151Reset();
#endif
			juliet_YMF288Reset();
		}
		return (SUCCESS);
	}
	return (FAILURE);
}


// ---- YM2151部

#if USEOPM
// リセットと同時に、OPMチップの有無も確認
void juliet_YM2151Reset(void)
{
	BYTE flag;

	if (romeo.avail & ROMEO_AVAIL) {
		juliet_YM2151Mute(TRUE);

		romeo.out32(romeo.addr + ROMEO_YM2151CTRL, 0x00);
		// 44.1kHz x 192 clk = 4.35ms 以上ないと、DACのリセットかからない
		Sleep(10);
		flag = (BYTE)(romeo.in8(romeo.addr + ROMEO_YM2151DATA) + 1);

		romeo.out32(romeo.addr + ROMEO_YM2151CTRL, 0x80);
		// リセット解除後、一応安定するまでちょっと待つ
		Sleep(10);
		flag |= romeo.in8(romeo.addr + ROMEO_YM2151DATA);

		// flag!=0 だと OPM チップがない
		if ( !flag ) {
			romeo.avail |= ROMEO_YM2151;

			// Busy検出用にSnoopカウンタを使う
			romeo.out32(romeo.addr + ROMEO_SNOOPCTRL, (unsigned int)0x80000000);
			romeo.snoopcount = 0xffffffff;
		}
	}

	// OPMコマンドバッファを初期化 byうさ
	OpmNumCmnd = 0;
	OpmCmndReadIdx = 0;
	OpmCmndWriteIdx = 0;

}

int juliet_YM2151IsEnable(void)
{
	return (( romeo.avail&ROMEO_YM2151 )?TRUE:FALSE);
}

int juliet_YM2151IsBusy(void)
{
	int ret = FALSE;
	if ( romeo.avail & ROMEO_YM2151 ) {
		if ( (romeo.snoopcount==romeo.in32(romeo.addr + ROMEO_SNOOPCTRL)) ||
		     (romeo.in8(romeo.addr + ROMEO_YM2151DATA)&0x80 ) ) {
			ret = TRUE;
		}
	}
	return ret;
}

static void YM2151W(BYTE addr, BYTE data) {

	// 書き込み直後だと、ROMEOチップでの遅延のため、まだ書き込みが起こって
	// いない（＝Busyが立っていない）可能性がある。
	// ので、Snoopカウンタで書き込み発生を見張る
	while ( romeo.snoopcount==romeo.in32(romeo.addr + ROMEO_SNOOPCTRL) ) {
		Sleep(0);
	}
	romeo.snoopcount = romeo.in32(romeo.addr + ROMEO_SNOOPCTRL);

	// カウンタ増えた時点ではまだBusyの可能性があるので、OPMのBusyも見張る
	while ( romeo.in8(romeo.addr + ROMEO_YM2151DATA)&0x80 ) {
		Sleep(0);
	}

	romeo.out8(romeo.addr + ROMEO_YM2151ADDR, addr);

	// dummy(?) wait byうさ
	romeo.in8(romeo.addr + ROMEO_YM2151DATA);
	romeo.out8(romeo.addr + ROMEO_YM2151DATA, data);
}

static void YM2151volset(BYTE ch, BYTE mask, char vol)
{
	BYTE	data;
	BYTE	out;

	ch &= (BYTE)7;
	out = (BYTE)romeo.YM2151_outop[ch];
	ch += (BYTE)0x60;
	do {
		if (mask & 1) {
			data = (BYTE)romeo.YM2151_ttl[ch & 0x1f];
			if (out & 1) {
				data -= vol;
				if (data & 0x80) {
					data = (BYTE)((vol < 0)?0x7f:0);
				}
			}
			YM2151W(ch, data);
		}
		ch += (BYTE)0x08;
		out >>= 1;
		mask >>= 1;
	} while(mask);
}

void juliet_YM2151Mute(BOOL mute)
{
	BYTE	ch;
	char	vol;

	if (romeo.avail & ROMEO_YM2151) {
		vol = (BYTE)(mute?-127:YM2151vol);
		for (ch=0; ch<8; ch++) {
			YM2151volset(ch, romeo.YM2151_outop[ch & 7], vol);
		}
	}
}

void juliet_YM2151W(BYTE addr, BYTE data)
{
	if (romeo.avail & ROMEO_YM2151) {
		if ((addr & 0xe0) == 0x60) {				// ttl
			data &= 0x7f;
			romeo.YM2151_ttl[addr & 0x1f] = data;
			if (romeo.YM2151_outop[addr & 7] & (1 << ((addr >> 3) & 3))) {
				data -= YM2151vol;
				if (data & 0x80) {
					data = (BYTE)((YM2151vol < 0)?0x7f:0);
				}
			}
		}
		YM2151W(addr, data);
		if ((addr & 0xf8) == 0x20) {				// algorithm
			BYTE op;
			op = (BYTE)(romeo.YM2151_outop[addr & 7] ^ FMoutop[data & 7]);
			if (op) {
				romeo.YM2151_outop[addr & 7] = FMoutop[data & 7];
				YM2151volset((BYTE)(addr & 7), op, YM2151vol);
			}
		}
	}
}

//	OPMバッファ書き込み byうさ
void juliet_YM2151BW(BYTE addr, BYTE data)
{
	if (OpmNumCmnd < OPMCMNDBUFSIZE) {
#if 1
		OpmCmnBuf[OpmCmndWriteIdx].Time = dwNowTime;
#else
		OpmCmnBuf[OpmCmndWriteIdx].Time = timeGetTime();
#endif
		OpmCmnBuf[OpmCmndWriteIdx].Addr = (WORD)addr;
		OpmCmnBuf[OpmCmndWriteIdx].Data = (BYTE)data;
		++OpmCmndWriteIdx;
		OpmCmndWriteIdx &= OPMCMNDBUFSIZE;
		++OpmNumCmnd;
	}
}

//
//	OPMバッファ処理 byうさ
//
void juliet_YM2151EXEC( DWORD Wait )
{
	DWORD t1, t2;
	unsigned char reg,data;

	while ( OpmNumCmnd ) {
#if 1
		t1 = dwNowTime;
#else
		t1 = timeGetTime();
#endif
		if (t1 >= OpmCmnBuf[OpmCmndReadIdx].Time) {
			t1 -= OpmCmnBuf[OpmCmndReadIdx].Time;
			if ( t1 < Wait ) break;
		}
		reg = (BYTE)OpmCmnBuf[OpmCmndReadIdx].Addr;
		data = (BYTE)OpmCmnBuf[OpmCmndReadIdx].Data;
		++OpmCmndReadIdx;
		OpmCmndReadIdx &= OPMCMNDBUFSIZE;
		--OpmNumCmnd;

		juliet_YM2151W( (BYTE)reg, (BYTE)data );
	}
}
#endif


// ---- YMF288部

static void YMF288W(BYTE a1, BYTE addr, BYTE data);

void juliet_YMF288Reset(void)
{
	if (romeo.avail & ROMEO_YMF288) {
		juliet_YMF288Mute(TRUE);
		romeo.out32(romeo.addr + ROMEO_YMF288CTRL, 0x00);
		Sleep(100);
		romeo.out32(romeo.addr + ROMEO_YMF288CTRL, 0x80);
		Sleep(100);
	}

	// OPN3Lコマンドバッファを初期化 byうさ
	OpnNumCmnd = 0;
	OpnCmndReadIdx = 0;
	OpnCmndWriteIdx = 0;

	/* プリスケーラ/音声合成フラグを初期化 */
	Prescaler[0] = -1;
	Prescaler[1] = -1;
	CSMmode = FALSE;

#if CLKCONV
	// 再生周波数変換テーブル初期化(LENA互換)
	int i;
	int fnum;
	int blk;

	/* FM */
	for (i=0; i<2048; i++) {
		blk = 0;
		fnum = (int)((double)i * 0.9216);
		while (fnum >= 0x0800) {
			blk ++;
			fnum >>= 1;
		}
		fnum_tbl[i]			= (BYTE)(fnum & 0xff);
		fnum_tbl[i + 2048]	= (BYTE)(((fnum >> 8) & 0x07) | (blk << 3));
	}

	/* PSG */
	for( i=0; i<80530; i++ ) {
		fnum = (int)((double)i * (10000.0 / 12288.0));
		if (fnum > 65535) {
			fnum = 65535;
		}
		psgfreq_tbl[i] = (WORD)fnum;
	}

	memset(fnum_buff, 0, sizeof(fnum_buff));
	memset(psg_buff, 0xff, sizeof(psg_buff));

//	YMF288W((BYTE)0, (BYTE)0x20, (BYTE)0x00);	// YM2608 mode
	YMF288W((BYTE)0, (BYTE)0x29, (BYTE)0x80);	// OPNA mode set (for MD)
#endif
}

int juliet_YMF288IsEnable(void)
{
	return(TRUE);
}

int juliet_YMF288IsBusy(void)
{
	return((!(romeo.avail&ROMEO_YMF288)) ||
			((romeo.in8(romeo.addr + ROMEO_YMF288ADDR1) & 0x80) != 0));
}


static void YMF288W(BYTE a1, BYTE addr, BYTE data)
{
	while(romeo.in8(romeo.addr + ROMEO_YMF288ADDR1) & 0x80) {
		Sleep(0);
	}
	romeo.out8(romeo.addr + (a1?ROMEO_YMF288ADDR2:ROMEO_YMF288ADDR1), addr);

	while(romeo.in8(romeo.addr + ROMEO_YMF288ADDR1) & 0x80) {
		Sleep(0);
	}
	romeo.out8(romeo.addr + (a1?ROMEO_YMF288DATA2:ROMEO_YMF288DATA1), data);
}


static void YMF288volset(BYTE ch, BYTE mask, char vol)
{
	BYTE	data;
	BYTE	out;
	BYTE	a1;
	BYTE	*datp;

	a1 = (BYTE)(ch & 4);
	out = (BYTE)(romeo.YMF288_outop[ch & 7]);
	ch &= (BYTE)3;
	datp = romeo.YMF288_ttl + (a1 << 2);
	ch += (BYTE)0x40;
	do {
		if (mask & 1) {
			data = (BYTE)datp[ch & 0x0f];
			if (out & 1) {
				data -= vol;
				if (data & 0x80) {
					data = (BYTE)((vol < 0)?0x7f:0);
				}
			}
			YMF288W(a1, ch, data);
		}
		ch += (BYTE)0x04;
		out >>= 1;
		mask >>= 1;
	} while(mask);
}


void juliet_YMF288Mute(BOOL mute)
{
	BYTE	ch;
	char	vol;

	if (romeo.avail & ROMEO_YMF288) {
		YMF288W(0, 0x07, (char)(mute?0x3f:romeo.YMF288_PSG));

		vol = (char)(mute?-127:0);
		for (ch=0; ch<3; ch++) {
			YMF288volset((BYTE)(ch+0), romeo.YMF288_outop[ch+0], vol);
			YMF288volset((BYTE)(ch+4), romeo.YMF288_outop[ch+4], vol);
		}
	}
}

void juliet_YMF288A(BYTE addr, BYTE data)
{
	if (romeo.avail & ROMEO_YMF288) {
		if (addr == 0x07) {							// psg mix
			romeo.YMF288_PSG = data;
		}
		else if ((addr & 0xf0) == 0x40) {			// ttl
			romeo.YMF288_ttl[addr & 0x0f] = (BYTE)(data & 0x7f);
		}
		else if ((addr & 0xfc) == 0xb0) {			// algorithm
			romeo.YMF288_outop[addr & 3] = FMoutop[data & 7];
		}
		YMF288W(0, addr, data);
	}
}

void juliet_YMF288B(BYTE addr, BYTE data)
{
	if (romeo.avail & ROMEO_YMF288) {
		if ((addr & 0xf0) == 0x40) {				// ttl
			romeo.YMF288_ttl[0x10 + addr & 0x0f] = (BYTE)(data & 0x7f);
		}
		else if ((addr & 0xfc) == 0xb0) {			// algorithm
			romeo.YMF288_outop[4 + addr & 3] = FMoutop[data & 7];
		}
		YMF288W(1, addr, data);
	}
}

void juliet_YMF288WriteReg(WORD addr, BYTE data)
{
	if (OpnNumCmnd < OPNCMNDBUFSIZE) {
#if 1
		OpnCmnBuf[OpnCmndWriteIdx].Time = dwNowTime;
#else
		OpnCmnBuf[OpnCmndWriteIdx].Time = timeGetTime();
#endif
		OpnCmnBuf[OpnCmndWriteIdx].Addr = (WORD)addr;
		OpnCmnBuf[OpnCmndWriteIdx].Data = (BYTE)data;
		++OpnCmndWriteIdx;
		OpnCmndWriteIdx &= OPNCMNDBUFSIZE;
		++OpnNumCmnd;
	}
}

//
//	OPN3Lバッファ書き込み byうさ
//
void juliet_YMF288A_B(BYTE addr, BYTE data)
{
#if CLKCONV
	unsigned int	fnum, oct, w;
	BYTE ad;
	const BYTE psg_scale[3] = { 1, 2, 4 };

	/* プリスケーラ設定値を内部表現に変換 by RHG */
	if (opn_scale[OPN_STD] == 6) {
		Prescaler[0] = 2;
	}
	else if (opn_scale[OPN_STD] == 2) {
		Prescaler[0] = 0;
	}
	else {
		Prescaler[0] = 1;
	}
	w = psg_scale[Prescaler[0]];

	// 音声合成モード対応 by RHG
	switch (addr) {
		case 0x27:
			if ((data & 0xc0) == 0x00) {
				/* 通常モード */
				data = 0x00;
				CSMmode = FALSE;
			}
			else {
				if (data & 0x80) {
					/* 音声合成モード */
					CSMmode = TRUE;
				}
				else {
					/* 効果音モード */
					CSMmode = FALSE;
				}
				/* YMF288を効果音モードに設定 */
				data = 0x40;
			}
			break;
		case 0xff:
			if (CSMmode) {
				/* Ch3 KeyOff→KeyOn  うわぁぁぁぁん、滅殺～ */
				juliet_YMF288WriteReg(0x28, 0x02);
				juliet_YMF288WriteReg(0x28, 0xf2);
			}
			return;
	}

	//	EGパラメータ補正 by RHG
	if ((addr >= 0x50) && (addr <= 0x7f)) {
		/* KS/AR,DR,SR */
		if (((data & 0x1f) != 0x1f) && ((data & 0x1f) >= Prescaler[0])) {
			data = (BYTE)((data & 0xe0) | ((data & 0x1f) - Prescaler[0]));
		}
	}
	if ((addr >= 0x80) && (addr <= 0x8f)) {
		/* SL/RR */
		if (((data & 0x0f) != 0x0f) && ((data & 0x0f) >= Prescaler[0])) {
			data = (BYTE)((data & 0xf0) | ((data & 0x0f) - Prescaler[0]));
		}
	}

	//	発音周波数変換処理(FM)
	if ((addr >= 0xa0) && (addr <= 0xae) && ((addr & 3) != 3)) {
		/* 謎 */
		ad = (BYTE)(addr & 0x0f);

		/* データ保存 */
		fnum_buff[ad] = data;

		/* F-Num2/Blockへの書き込みならまだ何もしない */
		if (ad & 0x04) {
			return;
		}

		ad &= (BYTE)0x1b;

		/* F-Number/Blockを計算 */
		fnum = ((fnum_buff[ad + 4] << 9) + (fnum_buff[ad + 0] << 1) & 0xffe);
		oct =   (fnum_buff[ad + 4] & 0x38);
		fnum /= opn_scale[OPN_STD];

		/* データ出力 */
		addr &= (WORD)0xfb;
		data = (BYTE)(fnum_tbl[fnum]);
		juliet_YMF288WriteReg(	(WORD)(addr + 4), 
								(BYTE)(fnum_tbl[fnum + 2048] + oct));
	}

	//	発音周波数変換処理(PSG)
	if (addr <= 5) {
		/* データ保存 */
		psg_buff[addr] = data;

		/* 周波数を変換 */
		addr &= (WORD)0x0e;
		fnum = (((psg_buff[addr + 1] << 8) | psg_buff[addr + 0]) & 0xfff);
		fnum = psgfreq_tbl[fnum * w];
		if (fnum > 4095) {
			fnum = 4095;
		}

		/* 上位4ビットの変化チェック */
		if (psg_buff[addr + 9] != (BYTE)(fnum >> 8)) {
			juliet_YMF288WriteReg((WORD)(addr + 1), (BYTE)(fnum >> 8));
			psg_buff[addr + 9] = (BYTE)(fnum >> 8);
		}

		/* 下位8ビットも変化チェック */
		data = (BYTE)(fnum & 0xff);
		if (psg_buff[addr + 8] == data) {
			return;
		}
		psg_buff[addr + 8] = data;
	}

	if (addr == 6) {
		/* ノイズ周波数 */
		fnum = psgfreq_tbl[(data & 0x1f) * w];
		if (fnum > 31) {
			fnum = 31;
		}
		data = (BYTE)fnum;
	}

	//	エンベロープ周波数変換処理(PSG)
	if ((addr >= 11) && (addr <= 12)) {
		/* データ保存 */
		psg_buff[addr - 5] = data;

		/* 周波数を変換 */
		fnum = (unsigned int)(((psg_buff[7] << 8) |
							   (psg_buff[6])) & 0xffff);
		fnum *= w;
		if (fnum >= 80530) {
			fnum = 65535;
		}
		else {
			fnum = psgfreq_tbl[fnum];
		}
		addr = 11;
		data = (BYTE)(fnum & 0xff);

		/* データ出力 */
		juliet_YMF288WriteReg((WORD)12, (BYTE)(fnum >> 8));
	}
#endif

	//	再生バッファ出力
	juliet_YMF288WriteReg((WORD)addr, (BYTE)data);
}


void juliet_YMF288B_B(BYTE addr, BYTE data)
{
#if CLKCONV
	unsigned int	fnum, oct;
	BYTE ad;

	/* プリスケーラ設定値を内部表現に変換 by RHG */
	if (opn_scale[OPN_WHG] == 6) {
		Prescaler[1] = 2;
	}
	else if (opn_scale[OPN_WHG] == 2) {
		Prescaler[1] = 0;
	}
	else {
		Prescaler[1] = 1;
	}

	/* キーオン命令変換 by RHG */
	if (addr == 0x28) {
		juliet_YMF288WriteReg(0x28, (BYTE)((data & 0xf3) | 0x04));
		return;
	}

	//	EGパラメータ補正 by RHG
	if ((addr >= 0x50) && (addr <= 0x7f)) {
		/* KS/AR,DR,SR */
		if (((data & 0x1f) != 0x1f) && ((data & 0x1f) >= Prescaler[1])) {
			data = (BYTE)((data & 0xe0) | ((data & 0x1f) - Prescaler[1]));
		}
	}
	if ((addr >= 0x80) && (addr <= 0x8f)) {
		/* SL/RR */
		if (((data & 0x0f) != 0x0f) && ((data & 0x0f) >= Prescaler[1])) {
			data = (BYTE)((data & 0xf0) | ((data & 0x0f) - Prescaler[1]));
		}
	}

	//	発音周波数変換処理(FM)
	if ((addr >= 0xa0) && (addr <= 0xae) && ((addr & 3) != 3)) {
		/* 謎 */
		ad = (BYTE)((addr & 0x0f) | 0x10);

		/* データ保存 */
		fnum_buff[ad] = data;

		/* F-Num2/Blockへの書き込みならまだ何もしない */
		if (ad & 0x04) {
			return;
		}

		ad &= (BYTE)0x1b;

		/* F-Number/Blockを計算 */
		fnum = ((fnum_buff[ad + 4] << 9) + (fnum_buff[ad + 0] << 1) & 0xffe);
		oct =   (fnum_buff[ad + 4] & 0x38);
		fnum /= opn_scale[OPN_WHG];

		/* データ出力 */
		addr &= (WORD)0xfb;
		data = (BYTE)(fnum_tbl[fnum]);
		juliet_YMF288WriteReg(	(WORD)((addr + 4) | 0x0100),
								(BYTE)(fnum_tbl[fnum + 2048] + oct));
	}
#endif

	//	再生バッファ出力
	juliet_YMF288WriteReg((WORD)(addr | 0x0100), (BYTE)data);
}

//	OPN3Lバッファ処理 byうさ
void juliet_YMF288EXEC( DWORD Wait )
{
	DWORD t1;
	unsigned char	reg, data, num;

	while ( OpnNumCmnd ) {
#if 1
		t1 = dwNowTime;
#else
		t1 = timeGetTime();
#endif
		if (t1 >= OpnCmnBuf[OpnCmndReadIdx].Time) {
			t1 -= OpnCmnBuf[OpnCmndReadIdx].Time;
			if ( t1 < Wait ) {
				break;
			}
		}
		reg  = (BYTE)(OpnCmnBuf[OpnCmndReadIdx].Addr & 0xFF);
		data = (BYTE)(OpnCmnBuf[OpnCmndReadIdx].Data);
		num  = (BYTE)((OpnCmnBuf[OpnCmndReadIdx].Addr >> 8) & 0x01);
		++OpnCmndReadIdx;
		OpnCmndReadIdx &= OPNCMNDBUFSIZE;
		--OpnNumCmnd;

		if( num == 0 ) {
			juliet_YMF288A((BYTE)reg, (BYTE)data);
		}
		else {
			juliet_YMF288B((BYTE)reg, (BYTE)data);
		}
	}
}


/* XM7では使ってないしBCCでコンパイルできないのでばっさり斬る(笑) */
#if 0
// ---- delay...

#ifndef LABEL
#define	LABEL		__declspec(naked)
#endif

#define	FM_BUFBIT		10
#define	FM_BUFFERS		(1 << FM_BUFBIT)

typedef struct {
	DWORD	clock;
	DWORD	data;
} FMRINGDATA;

typedef struct {
	DWORD		base;
	DWORD		pos;
	DWORD		avail;
	DWORD		maxclock;
	FMRINGDATA	datas[FM_BUFFERS];
} FMRING;

static	FMRING	fmr;
		DWORD	basedclk = 0;

static void (*sendfmfn[3])(BYTE addr, BYTE data) =
							{juliet_YM2151W, juliet_YMF288A, juliet_YMF288B};


LABEL static void fmr_release(void) {

	__asm {
				dec		fmr.avail
				mov		edx, fmr.pos
				mov		eax, fmr.datas.data[edx * 8]
				inc		edx
				and		edx, (FM_BUFFERS - 1);
				mov		fmr.pos, edx
				push	eax
				shr		eax, 8
				push	eax
				shr		eax, 8
				call	sendfmfn[eax*4]
				add		esp, 8
				ret
	}
}


LABEL static void fmr_send(DWORD clock, DWORD data) {

	__asm {
				mov		ecx, fmr.avail
				cmp		ecx, FM_BUFFERS
				jne		short fmdatasend
				call	fmr_release
				dec		ecx
fmdatasend:		add		ecx, fmr.pos
				and		ecx, (FM_BUFFERS - 1);
				mov		eax, fmr.base
				add		eax, [esp + 4]
				mov		fmr.datas.clock[ecx * 8], eax
				mov		eax, [esp + 8]
				mov		fmr.datas.data[ecx * 8], eax
				inc		fmr.avail
				ret
	}
}


#define	_CPUID	__asm _emit 00fh __asm _emit 0a2h
#define	_RDTSC	__asm _emit 00fh __asm _emit 031h
#define	RDTSC_SFT	6

LABEL void juliet2_reset(void) {

	__asm {
				push	ebx
				push	esi
				push	edi
				mov		ecx, (type fmr) / 4
				xor		eax, eax
				mov		edi, offset fmr
				rep stosd
				push	100
				mov		edi, dword ptr Sleep
				_CPUID
				_RDTSC
				mov		esi, edx
				xchg	edi, eax
				call	eax
				_CPUID
				_RDTSC
				sub		eax, edi
				sbb		edx, esi
				shrd	eax, edx, RDTSC_SFT
				mov		fmr.maxclock, eax
				mov		ebx, 6
				xor		edx, edx
				div		ebx
				mov		basedclk, eax
				pop		edi
				pop		esi
				pop		ebx
				ret
	}
}


LABEL void juliet2_sync(DWORD delaytick) {

	__asm {
				push	ebx
				_CPUID
				_RDTSC
				shrd	eax, edx, RDTSC_SFT
				add		eax, basedclk
				add		eax, basedclk
				mov		fmr.base, eax
				pop		ebx
				ret
	}
}


LABEL void juliet2_exec(void) {

	__asm {
				cmp		fmr.avail, 0
				jne		short jul2exe
jul2exe_exit:	ret
jul2exe:		push	ebx
				_CPUID
				_RDTSC
				pop		ebx
				shrd	eax, edx, RDTSC_SFT
				neg		eax
				mov		edx, fmr.pos
				add		eax, fmr.datas[edx * 8].clock
				cmp		eax, fmr.maxclock
				jb		jul2exe_exit
				call	fmr_release
				cmp		fmr.avail, 0
				jne		short jul2exe
				ret
	}
}


void juliet2_YM2151W(BYTE addr, BYTE data, DWORD clock) {

	if (romeo.avail & ROMEO_YM2151) {
		fmr_send(clock, (addr << 8) | data);
	}
}

void juliet2_YMF288A(BYTE addr, BYTE data, DWORD clock) {

	if (romeo.avail & ROMEO_YMF288) {
		fmr_send(clock, 0x10000 | (addr << 8) | data);
	}
}
#endif

#endif

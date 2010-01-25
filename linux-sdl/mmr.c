/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ MMR,TWR / I/O型RAMディスクカード ]
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mmr.h"
#include "subctrl.h"
#include "jcard.h"

/*
 *	グローバル ワーク
 */
BOOL mmr_flag;							/* MMR有効フラグ */
BYTE mmr_seg;							/* MMRセグメント */
BOOL mmr_modify;						/* MMR状態変更フラグ */
#if XM7_VER >= 3
BYTE mmr_reg[0x80];						/* MMRレジスタ */
BOOL twr_flag;							/* TWR有効フラグ */
BYTE twr_reg;							/* TWRレジスタ */
BOOL mmr_ext;							/* 拡張MMR有効フラグ */
BOOL mmr_fastmode;						/* MMR高速フラグ */
BOOL mmr_extram;						/* 拡張RAM有効フラグ */
BOOL mmr_fast_refresh;					/* 高速リフレッシュフラグ */
#else
BYTE mmr_reg[0x40];						/* MMRレジスタ */
BOOL twr_flag;							/* TWR有効フラグ */
BYTE twr_reg;							/* TWRレジスタ */
#endif

/* I/O型RAMディスクカード */
#if XM7_VER >= 3
#ifdef MR2
BYTE mr2_nowcnt;						/* MR2 セクタカウント */
WORD mr2_secreg;						/* MR2 セクタレジスタ */
#endif
#endif


/*
 *	MMR
 *	初期化
 */
BOOL FASTCALL mmr_init(void)
{
#if XM7_VER >= 3
	/* 768KB 拡張RAMカードをディセーブル */
	mmr_extram = FALSE;
#endif

	return TRUE;
}

/*
 *	MMR
 *	クリーンアップ
 */
void FASTCALL mmr_cleanup(void)
{
}

/*
 *	MMR
 *	リセット
 */
void FASTCALL mmr_reset(void)
{
	/* MMR/TWR */
	mmr_flag = FALSE;
	twr_flag = FALSE;
	memset(mmr_reg, 0, sizeof(mmr_reg));
	mmr_seg = 0;
	twr_reg = 0;
	mmr_modify = FALSE;

#if XM7_VER >= 3
	/* MMR(AV40拡張) */
	mmr_ext = FALSE;
	mmr_fastmode = FALSE;
	mmr_fast_refresh = FALSE;

	/* I/O型RAMディスク */
#ifdef MR2
	mr2_nowcnt = 0;
	mr2_secreg = 0;
#endif
#endif
}

/*-[ メモリマネージャ ]-----------------------------------------------------*/

/*
 *	TWRアドレス変換
 */
static BOOL FASTCALL mmr_trans_twr(WORD addr, DWORD *taddr)
{
	ASSERT(fm7_ver >= 2);

	/* TWR有効か */
	if (!twr_flag) {
		return FALSE;
	}

	/* アドレス要件チェック */
	if ((addr < 0x7c00) || (addr > 0x7fff)) {
		return FALSE;
	}

	/* TWRレジスタより変換 */
	*taddr = (DWORD)twr_reg;
	*taddr *= 256;
	*taddr += addr;
	*taddr &= 0xffff;
#if XM7_VER == 1
	/* FM-77(not AV)のテキスト空間に合わせてアドレスを補正 */
	*taddr |= 0x20000;
#endif

	return TRUE;
}

/*
 *	MMRアドレス変換
 */
static DWORD FASTCALL mmr_trans_mmr(WORD addr)
{
	DWORD maddr;
	int offset;

	ASSERT(fm7_ver >= 2);

	/* MMR有効か */
	if (!mmr_flag) {
		return (DWORD)(0x30000 | addr);
	}

	/* MMRレジスタより取得 */
	offset = (int)addr;
	offset >>= 12;

#if XM7_VER >= 3
	/* 拡張MMRがoffなら、セグメントは0〜3まで */
	if (mmr_ext) {
		offset |= (mmr_seg * 0x10);
	}
	else {
		offset |= ((mmr_seg & 0x03) * 0x10);
	}
#else
	offset |= ((mmr_seg & 0x03) * 0x10);
#endif

	/* 拡張MMRがoffなら、6bitのみ有効 */
	maddr = (DWORD)mmr_reg[offset];
#if XM7_VER >= 3
	if (!mmr_ext) {
		maddr &= 0x3f;
	}
#else
	maddr &= 0x3f;
#endif
	maddr <<= 12;

	/* 下位12ビットと合成 */
	addr &= 0xfff;
	maddr |= addr;

	return maddr;
}

/*
 *	メインCPUバス
 *	１バイト読み出し
 */
BOOL FASTCALL mmr_extrb(WORD *addr, BYTE *dat)
{
	DWORD raddr, rsegment;

#if XM7_VER >= 2
	ASSERT(fm7_ver >= 2);
#endif

	/* $FC00〜$FFFFは常駐空間 */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMRを通す */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* 標準空間 */
	if (rsegment == 0x30000) {
		/* MMRは再配置禁止 */
		if ((raddr >= 0x3fd80) && (raddr <= 0x3fd97)) {
			*dat = 0xff;
			return TRUE;
		}

		/* $30セグメント */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		*dat = extram_a[raddr & 0xffff];
		return TRUE;
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			*dat = submem_readb((WORD)(raddr & 0xffff));
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 3
		*dat = jcard_readb((WORD)(raddr & 0xffff));
#else
		*dat = 0xff;
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* 768KB 拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			*dat = extram_c[raddr - 0x40000];
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 拡張RAM */
	*dat = extram_a[raddr];
	return TRUE;
#endif
}

/*
 *	メインCPUバス
 *	１バイト読み出し(I/Oなし)
 */
BOOL FASTCALL mmr_extbnio(WORD *addr, BYTE *dat)
{
	DWORD raddr, rsegment;

#if XM7_VER >= 2
	ASSERT(fm7_ver >= 2);
#endif

	/* $FC00〜$FFFFは常駐空間 */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMRを通す */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* 標準空間 */
	if (rsegment == 0x30000) {
		/* MMRは再配置禁止 */
		if ((raddr >= 0x3fd80) && (raddr <= 0x3fd95)) {
			*dat = 0xff;
			return TRUE;
		}

		/* $30セグメント */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		*dat = extram_a[raddr & 0xffff];
		return TRUE;
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			*dat = submem_readbnio((WORD)(raddr & 0xffff));
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 3
		*dat = jcard_readb((WORD)(raddr & 0xffff));
#else
		*dat = 0xff;
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* 768KB 拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			*dat = extram_c[raddr - 0x40000];
		}
		else {
			*dat = 0xff;
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 拡張RAM */
	*dat = extram_a[raddr];
	return TRUE;
#endif
}

/*
 *	メインCPUバス
 *	１バイト書き込み
 */
BOOL FASTCALL mmr_extwb(WORD *addr, BYTE dat)
{
	DWORD raddr, rsegment;

#if XM7_VER >= 2
	ASSERT(fm7_ver >= 2);
#endif

	/* $FC00〜$FFFFは常駐空間 */
	if (*addr >= 0xfc00) {
		return FALSE;
	}

	/* TWR,MMRを通す */
	if (!mmr_trans_twr(*addr, &raddr)) {
		raddr = mmr_trans_mmr(*addr);
	}

	rsegment = (raddr & 0xf0000);

	/* 標準空間 */
	if (rsegment == 0x30000) {
		/* MMRは再配置禁止 */
		if ((raddr >= 0x3fd80) && (raddr <= 0x3fd95)) {
			return TRUE;
		}

		/* $30セグメント */
		*addr = (WORD)(raddr & 0xffff);
		return FALSE;
	}

#if XM7_VER >= 2
	/* FM77AV 拡張RAM */
	if (rsegment == 0x00000) {
		extram_a[raddr & 0xffff] = dat;
		return TRUE;
	}

	/* サブシステム */
	if (rsegment == 0x10000) {
		if (subhalt_flag) {
			submem_writeb((WORD)(raddr & 0xffff), dat);
		}
		return TRUE;
	}

	/* 日本語カード */
	if (rsegment == 0x20000) {
#if XM7_VER >= 3
		jcard_writeb((WORD)(raddr & 0xffff), dat);
#endif
		return TRUE;
	}

#if XM7_VER >= 3
	/* AV40拡張RAM */
	if (rsegment >= 0x40000) {
		if (mmr_extram) {
			extram_c[raddr - 0x40000] = dat;
		}
		return TRUE;
	}
#endif

	return FALSE;
#else
	/* FM-77 拡張RAM */
	extram_a[raddr] = dat;
	return TRUE;
#endif

}

/*-[ MR2 I/O型RAMディスク ]-------------------------------------------------*/

#if (XM7_VER >= 3) && defined(MR2)
/*
 *	MR2
 *	アドレス計算
 */
static DWORD FASTCALL mr2_address(void)
{
	DWORD tmp;

	if (mr2_secreg <= 0x0bff) {
		/* 計算方法は適当なので違っている可能性、大(^^; 2002/07/29 */
		tmp = (0xbff - mr2_secreg) << 8;
		tmp |= mr2_nowcnt;
	}
	else {
		tmp = 0;
	}

	return tmp;
}

/*
 *	MR2
 *	データリード
 */
static BYTE FASTCALL mr2_read_data(void)
{
	BYTE dat;

	/* セクタレジスタが異常の場合は読み出せない */
	if (mr2_secreg <= 0x0bff) {
		dat = extram_c[mr2_address()];
		mr2_nowcnt ++;
	}
	else {
		dat = 0xff;
	}

	return dat;
}

/*
 *	MR2
 *	データライト
 */
static void FASTCALL mr2_write_data(BYTE dat)
{
	/* セクタレジスタが異常の場合は書き込めない */
	if (mr2_secreg <= 0x0bff) {
		extram_c[mr2_address()] = dat;
		mr2_nowcnt ++;
	}
}

/*
 *	MR2
 *	セクタレジスタ更新
 */
static void FASTCALL mr2_update_sector(WORD addr, BYTE dat)
{
	/* セクタレジスタを更新 */
	if (addr == 0xfd9e) {
		mr2_secreg &= (WORD)0x00ff;
		mr2_secreg |= (WORD)((dat & 0x0f) << 8);
	}
	else {
		mr2_secreg &= (WORD)0x0f00;
		mr2_secreg |= (WORD)dat;
	}

	/* 範囲チェック */
	if (mr2_secreg >= 0x0c00) {
		ASSERT(FALSE);
		mr2_secreg = 0x0c00;
	}

	/* セクタカウンタをリセット */
	mr2_nowcnt = 0;
}
#endif

/*-[ メモリマップドI/O ]----------------------------------------------------*/

/*
 *	MMR
 *	１バイト読み出し
 */
BOOL FASTCALL mmr_readb(WORD addr, BYTE *dat)
{
	BYTE tmp;

	/* バージョンチェック */
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		return FALSE;
	}
#else
	if (fm_subtype != FMSUB_FM77) {
		return FALSE;
	}
#endif

	switch (addr) {
#if XM7_VER >= 2
		/* ブートステータス */
		case 0xfd0b:
			if (boot_mode == BOOT_BASIC) {
				*dat = 0xfe;
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* イニシエータROM */
		case 0xfd10:
			*dat = 0xff;
			return TRUE;
#endif

		/* MMRセグメント */
		case 0xfd90:
			*dat = 0xff;
			return TRUE;

		/* TWRオフセット */
		case 0xfd92:
			*dat = 0xff;
			return TRUE;

		/* モードセレクト */
		case 0xfd93:
			tmp = 0xff;
			if (!mmr_flag) {
				tmp &= (BYTE)(~0x80);
			}
			if (!twr_flag) {
				tmp &= ~0x40;
			}
			if (!bootram_rw) {
				tmp &= ~1;
			}
			*dat = tmp;
			return TRUE;

#if XM7_VER >= 3
		/* 拡張MMR/CPUスピード */
		case 0xfd94:
			*dat = 0xff;
			return TRUE;

		/* モードセレクト２ */
		case 0xfd95:
			tmp = 0xff;
			if (fm7_ver >= 3) {
				/* bit7:拡張ROMセレクト */
				if (extrom_sel) {
					tmp &= (BYTE)~0x80;
				}
				/* bit4:MMR使用時の速度低下を抑止 */
				if (!mmr_fastmode) {
					tmp &= (BYTE)~0x08;
				}
				/* bit0:400ラインタイミング出力ステータス */
				/*      XM7では1(200ライン出力)固定 */
			}

			*dat = tmp;
			return TRUE;

#ifdef MR2
		/* MR2 データレジスタ */
		case 0xfd9c:
			if (mmr_extram) {
				*dat = mr2_read_data();
			}
			else {
				*dat = 0xff;
			}
			return TRUE;

		/* MR2 セクタレジスタ(Write Only) */
		case 0xfd9e:
		case 0xfd9f:
			*dat = 0xff;
			return TRUE;
#endif

#endif
	}

	/* MMRレジスタ */
	if ((addr >= 0xfd80) && (addr <= 0xfd8f)) {
#if XM7_VER >= 3
		if (mmr_ext) {
			tmp = mmr_reg[mmr_seg * 0x10 + (addr - 0xfd80)];
		}
		else {
			tmp = mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)];
			/* tmp &= 0x3f; */
		}
#else
		tmp = mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)];
		/* tmp &= 0x3f; */
#endif
		*dat = tmp;
		return TRUE;
	}

	return FALSE;
}

/*
 *	MMR
 *	１バイト書き込み
 */
BOOL FASTCALL mmr_writeb(WORD addr, BYTE dat)
{
	/* バージョンチェック */
#if XM7_VER >= 2
	if (fm7_ver < 2) {
		return FALSE;
	}
#else
	if (fm_subtype != FMSUB_FM77) {
		return FALSE;
	}
#endif

	switch (addr) {
		/* イニシエータROM */
#if XM7_VER >= 2
		case 0xfd10:
			if (dat & 0x02) {
				initrom_en = FALSE;
			}
			else {
				initrom_en = TRUE;
			}
			return TRUE;
#endif

		/* MMRセグメント */
		case 0xfd90:
			mmr_seg = (BYTE)(dat & 0x07);
			return TRUE;

		/* TWRオフセット */
		case 0xfd92:
			twr_reg = dat;
			return TRUE;

		/* モードセレクト */
		case 0xfd93:
			if (dat & 0x80) {
				if (!mmr_flag) {
					mmr_flag = TRUE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			else {
				if (mmr_flag) {
					mmr_flag = FALSE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			if (dat & 0x40) {
				if (!twr_flag) {
					twr_flag = TRUE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			else {
				if (twr_flag) {
					twr_flag = FALSE;
#if XM7_VER >= 3
					if (!mmr_fastmode) {
						mmr_modify = TRUE;
					}
#else
					mmr_modify = TRUE;
#endif
				}
			}
			if (dat & 0x01) {
				bootram_rw = TRUE;
			}
			else {
				bootram_rw = FALSE;
			}
			return TRUE;

#if XM7_VER >= 3
		/* 拡張MMR/CPUスピード */
		case 0xfd94:
			if (fm7_ver >= 3) {
				/* bit7:拡張MMR */
				if (dat & 0x80) {
					mmr_ext = TRUE;
				}
				else {
					mmr_ext = FALSE;
				}
				/* bit2:リフレッシュスピード */
				if (dat & 0x04) {
					if (!mmr_fast_refresh) {
						mmr_fast_refresh = TRUE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				else {
					if (mmr_fast_refresh) {
						mmr_fast_refresh = FALSE;
						if (!mmr_fastmode) {
							mmr_modify = TRUE;
						}
					}
				}
				/* bit0:ウィンドウスピード XM7では未対応 */
			}
			return TRUE;

		/* モードセレクト２ */
		case 0xfd95:
			if (fm7_ver >= 3) {
				/* bit7:拡張ROMセレクト */
				if (dat & 0x80) {
					extrom_sel = TRUE;
				}
				else {
					extrom_sel = FALSE;
				}
				/* bit4:MMR使用時の速度低下を抑止 */
				if (dat & 0x08) {
					if (!mmr_fastmode) {
						mmr_fastmode = TRUE;
						mmr_modify = TRUE;
					}
				}
				else {
					if (mmr_fastmode) {
						mmr_fastmode = FALSE;
						mmr_modify = TRUE;
					}
				}
			}
			return TRUE;

#ifdef MR2
		/* MR2 データレジスタ */
		case 0xfd9c:
			if (mmr_extram) {
				mr2_write_data(dat);
			}
			return TRUE;

		/* MR2 セクタレジスタ */
		case 0xfd9e:
		case 0xfd9f:
			mr2_update_sector(addr, dat);
			return TRUE;
#endif
#endif
	}

	/* MMRレジスタ */
	if ((addr >= 0xfd80) && (addr <= 0xfd8f)) {
#if XM7_VER >= 3
		/* ここでのデータは8bitすべて記憶 */
		if (mmr_ext) {
			mmr_reg[mmr_seg * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
		}
		else {
			mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
		}
#else
		mmr_reg[(mmr_seg & 3) * 0x10 + (addr - 0xfd80)] = (BYTE)dat;
#endif
		return TRUE;
	}

	return FALSE;
}

/*-[ ファイルI/O ]----------------------------------------------------------*/

/*
 *	MMR
 *	セーブ
 */
BOOL FASTCALL mmr_save(int fileh)
{
	if (!file_bool_write(fileh, mmr_flag)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, mmr_seg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8拡張部 */
	if (!file_write(fileh, mmr_reg, 0x80)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_ext)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_fastmode)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mmr_extram)) {
		return FALSE;
	}
	if (mmr_extram) {
		if (!file_write(fileh, extram_c, 0xc0000)) {
			return FALSE;
		}
	}
#ifdef MR2
	if (!file_byte_write(fileh, mr2_nowcnt)) {
		return FALSE;
	}
	if (!file_word_write(fileh, mr2_secreg)) {
		return FALSE;
	}
#else
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, 0)) {
		return FALSE;
	}
#endif
#else
	if (!file_write(fileh, mmr_reg, 0x40)) {
		return FALSE;
	}
#endif

	if (!file_bool_write(fileh, twr_flag)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, twr_reg)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	MMR
 *	ロード
 */
BOOL FASTCALL mmr_load(int fileh, int ver)
{
#if (XM7_VER >= 3) && !defined(MR2)
	WORD tmp;
#endif

	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &mmr_flag)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &mmr_seg)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* ファイルバージョン8で拡張 */
	if (ver >= 800) {
		if (!file_read(fileh, mmr_reg, 0x80)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_ext)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_fastmode)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &mmr_extram)) {
			return FALSE;
		}
		if (mmr_extram) {
			if (!file_read(fileh, extram_c, 0xc0000)) {
				return FALSE;
			}
		}
		if (ver >= 902) {
#ifdef MR2
			if (!file_byte_read(fileh, &mr2_nowcnt)) {
				return FALSE;
			}
			if (!file_word_read(fileh, &mr2_secreg)) {
				return FALSE;
			}
#else
			if (!file_byte_read(fileh, (BYTE *)&tmp)) {
				return FALSE;
			}
			if (!file_word_read(fileh, &tmp)) {
				return FALSE;
			}
#endif
		}
#ifdef MR2
		else {
			mr2_nowcnt = 0;
			mr2_secreg = 0;
		}
#endif
	}
	else {
		/* Ver5互換 */
		if (!file_read(fileh, mmr_reg, 0x40)) {
			return FALSE;
		}
		mmr_ext = FALSE;
		mmr_fastmode = FALSE;
		mmr_extram = FALSE;
#ifdef MR2
		mr2_nowcnt = 0;
		mr2_secreg = 0;
#endif
	}
#else
	if (!file_read(fileh, mmr_reg, 0x40)) {
		return FALSE;
	}
#endif

	if (!file_bool_read(fileh, &twr_flag)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &twr_reg)) {
		return FALSE;
	}

	return TRUE;
}

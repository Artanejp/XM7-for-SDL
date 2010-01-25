/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2009 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2009 Ryu Takegami
 *
 *	[ 論理演算・直線補間(MB61VH010/011) ]
 */

#if XM7_VER >= 2

#include <string.h>
#include <stdio.h>
#include "xm7.h"
#include "aluline.h"
#include "event.h"
#include "display.h"
#include "subctrl.h"
#include "multipag.h"
#include "device.h"
/* XM7/SDL依存 */
#include <sdl.h>
#include <sdl_draw.h>

/*
 *	グローバル ワーク
 */
BYTE alu_command;						/* 論理演算 コマンド */
BYTE alu_color;							/* 論理演算 カラー */
BYTE alu_mask;							/* 論理演算 マスクビット */
BYTE alu_cmpstat;						/* 論理演算 比較ステータス */
BYTE alu_cmpdat[8];						/* 論理演算 比較データ */
BYTE alu_disable;						/* 論理演算 禁止バンク */
BYTE alu_tiledat[3];					/* 論理演算 タイルパターン */

BOOL line_busy;							/* 直線補間 BUSY */
WORD line_offset;						/* 直線補間 アドレスオフセット */
WORD line_style;						/* 直線補間 ラインスタイル */
WORD line_x0;							/* 直線補間 X0 */
WORD line_y0;							/* 直線補間 Y0 */
WORD line_x1;							/* 直線補間 X1 */
WORD line_y1;							/* 直線補間 Y1 */
BOOL line_boost;						/* 直線補間 全速力描画フラグ */

/*
 *  スタティック ワーク
 */
static BYTE alu_vram_bank;				/* 論理演算 処理中VRAMバンク */
static WORD line_addr_old;				/* 直線補間 前回処理したVRAMアドレス */
static BYTE line_mask;					/* 直線補間 マスクビット */
static WORD line_count;					/* 直線補間 カウンタ(整数部) */
static BYTE line_count_sub;				/* 直線補間 カウンタ(小数部) */

/*
 *	プロトタイプ宣言
 */
static void FASTCALL alu_cmp(WORD addr);	/* コンペア */


/*
 *	論理演算・直線補間
 *	初期化
 */
BOOL FASTCALL aluline_init(void)
{
	/* 直線補間処理中にBUSY状態とする */
	line_boost = FALSE;

	return TRUE;
}

/*
 *	論理演算・直線補間
 *	クリーンアップ
 */
void FASTCALL aluline_cleanup(void)
{
}

/*
 *	論理演算・直線補間
 *	リセット
 */
void FASTCALL aluline_reset(void)
{
	/* 全てのレジスタを初期化 */
	alu_command = 0;
	alu_color = 0;
	alu_mask = 0;
	alu_cmpstat = 0;
	memset(alu_cmpdat, 0x80, sizeof(alu_cmpdat));
	alu_disable = 0x08;		/* 実機(40EX)では0x0f? */
	memset(alu_tiledat, 0, sizeof(alu_tiledat));

	line_busy = FALSE;
	line_offset = 0;
	line_style = 0;
	line_x0 = 0;
	line_y0 = 0;
	line_x1 = 0;
	line_y1 = 0;

	alu_vram_bank = 0;
	line_addr_old = 0xffff;
	line_mask = 0xff;
	line_count = 0;
	line_count_sub = 0;
}

/*-[ 論理演算 ]-------------------------------------------------------------*/

/*
 *	論理演算
 *	VRAM読み出し
 */
static BYTE FASTCALL alu_read(WORD addr)
{
	/* アクティブプレーンでない場合、0xFFを返す */
	if (multi_page & (1 << alu_vram_bank)) {
		return 0xff;
	}

#if XM7_VER >= 3
	if (mode400l) {
		if (addr >= 0x8000) {
			/* 400ライン時の0x8000〜0xBFFFにはメモリが存在しない */
			return 0xff;
		}
		else {
			return vram_ablk[alu_vram_bank * 0x8000 + addr];
		}
	}
	else {
		return vram_aptr[alu_vram_bank * 0x8000 + addr];
	}
#else
	return vram_aptr[alu_vram_bank * 0x4000 + addr];
#endif
}

/*
 *	論理演算
 *	VRAM読み出し(プレーン指定)
 */
static BYTE FASTCALL alu_read_plane(WORD addr, int plane)
{
	/* アクティブプレーンでない場合、0xFFを返す */
	if (multi_page & (1 << plane)) {
		return 0xff;
	}

#if XM7_VER >= 3
	if (mode400l) {
		if (addr >= 0x8000) {
			/* 400ライン時の0x8000〜0xBFFFにはメモリが存在しない */
			return 0xff;
		}
		else {
			return vram_ablk[plane * 0x8000 + addr];
		}
	}
	else {
		return vram_aptr[plane * 0x8000 + addr];
	}
#else
	return vram_aptr[plane * 0x4000 + addr];
#endif
}

/*
 *	論理演算
 *	VRAM書き込み
 */
static void FASTCALL alu_write(WORD addr, BYTE dat)
{
	/* アクティブプレーンでない場合NOP */
	if (multi_page & (1 << alu_vram_bank)) {
		return;
	}

#if XM7_VER >= 3
	if (mode400l) {
		/* 400ライン時の0x8000〜0xBFFFにはメモリが存在しないのでNOP */
		if (addr < 0x8000) {
			if (vram_ablk[alu_vram_bank * 0x8000 + addr] != dat) {
				vram_ablk[alu_vram_bank * 0x8000 + addr] = dat;
				vram_notify(addr, dat);
			}
		}
	}
	else {
		if (vram_aptr[alu_vram_bank * 0x8000 + addr] != dat) {
			vram_aptr[alu_vram_bank * 0x8000 + addr] = dat;
			vram_notify(addr, dat);
		}
	}
#else
	if (vram_aptr[alu_vram_bank * 0x4000 + addr] != dat) {
		vram_aptr[alu_vram_bank * 0x4000 + addr] = dat;
		vram_notify(addr, dat);
	}
#endif
}

/*
 *	論理演算
 *	書き込みサブ(比較書き込み付)
 */
static void FASTCALL alu_writesub(WORD addr, BYTE dat)
{
	BYTE temp;

	/* 常に書き込み可能か */
	if ((alu_command & 0x40) == 0) {
		alu_write(addr, dat);
		return;
	}

	/* イコール書き込みか、NOTイコール書き込みか */
	if (alu_command & 0x20) {
		/* NOTイコールで書き込む */
		temp = alu_read(addr);
		temp &= alu_cmpstat;
		dat &= (BYTE)(~alu_cmpstat);
	}
	else {
		/* イコールで書き込む */
		temp = alu_read(addr);
		temp &= (BYTE)(~alu_cmpstat);
		dat &= alu_cmpstat;
	}

	/* データをVRAMに書き込む */
	alu_write(addr, (BYTE)(temp | dat));
}

/*
 *	論理演算
 *	PSET
 */
static void FASTCALL alu_pset(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAMオフセットを取得、ビット位置初期化 */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* 比較書き込み時は、先に比較データを取得 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* 各バンクに対して処理を行う(無効バンクはスキップ) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* 演算カラーデータより、データ作成 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* 演算なし(PSET) */
			mask = alu_read(addr);

			/* マスクビットの処理 */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* 書き込み */
			alu_writesub(addr, dat);
		}

		/* 次のバンクへ */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	論理演算
 *	コマンド1(禁止・黒色書き込み)
 */
static void FASTCALL alu_prohibit(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAMオフセットを取得、ビット位置初期化 */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* 比較書き込み時は、先に比較データを取得 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* 各バンクに対して処理を行う(無効バンクはスキップ) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* 演算なし */
			mask = alu_read(addr);

			/* マスクビットの処理 */
			dat = (mask & alu_mask);

			/* 書き込み */
			alu_writesub(addr, dat);
		}

		/* 次のバンクへ */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	論理演算
 *	OR
 */
static void FASTCALL alu_or(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAMオフセットを取得、ビット位置初期化 */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif


	/* 比較書き込み時は、先に比較データを取得 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* 各バンクに対して処理を行う(無効バンクはスキップ) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* 演算カラーデータより、データ作成 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* 演算 */
			mask = alu_read(addr);
			dat |= mask;

			/* マスクビットの処理 */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* 書き込み */
			alu_writesub(addr, dat);
		}

		/* 次のバンクへ */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	論理演算
 *	AND
 */
static void FASTCALL alu_and(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAMオフセットを取得、ビット位置初期化 */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* 比較書き込み時は、先に比較データを取得 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* 各バンクに対して処理を行う(無効バンクはスキップ) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* 演算カラーデータより、データ作成 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* 演算 */
			mask = alu_read(addr);
			dat &= mask;

			/* マスクビットの処理 */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* 書き込み */
			alu_writesub(addr, dat);
		}

		/* 次のバンクへ */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	論理演算
 *	XOR
 */
static void FASTCALL alu_xor(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAMオフセットを取得、ビット位置初期化 */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* 比較書き込み時は、先に比較データを取得 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* 各バンクに対して処理を行う(無効バンクはスキップ) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* 演算カラーデータより、データ作成 */
			if (alu_color & bit) {
				dat = 0xff;
			}
			else {
				dat = 0;
			}

			/* 演算 */
			mask = alu_read(addr);
			dat ^= mask;

			/* マスクビットの処理 */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* 書き込み */
			alu_writesub(addr, dat);
		}

		/* 次のバンクへ */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	論理演算
 *	NOT
 */
static void FASTCALL alu_not(WORD addr)
{
	BYTE dat;
	BYTE mask;
	BYTE bit;

	/* VRAMオフセットを取得、ビット位置初期化 */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* 比較書き込み時は、先に比較データを取得 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* 各バンクに対して処理を行う(無効バンクはスキップ) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* 演算(NOT) */
			mask = alu_read(addr);
			dat = (BYTE)(~mask);

			/* マスクビットの処理 */
			dat &= (BYTE)(~alu_mask);
			mask &= alu_mask;
			dat |= mask;

			/* 書き込み */
			alu_writesub(addr, dat);
		}

		/* 次のバンクへ */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	論理演算
 *	タイルペイント
 */
static void FASTCALL alu_tile(WORD addr)
{
	BYTE bit;
	BYTE mask;
	BYTE dat;

	/* VRAMオフセットを取得、ビット位置初期化 */
	alu_vram_bank = 0;
	bit = 0x01;

#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
	 	addr &= 0x3fff;
	}
#else
	 addr &= 0x3fff;
#endif

	/* 比較書き込み時は、先に比較データを取得 */
	if (alu_command & 0x40) {
		alu_cmp(addr);
	}

	/* 各バンクに対して処理を行う(無効バンクはスキップ) */
	while (alu_vram_bank < 3) {
		if (!(alu_disable & bit)) {
			/* データ作成 */
			dat = alu_tiledat[alu_vram_bank];

			/* マスクビットの処理 */
			dat &= (BYTE)(~alu_mask);
			mask = alu_read(addr);
			mask &= alu_mask;
			dat |= mask;

			/* 書き込み */
			alu_writesub(addr, dat);
		}

		/* 次のバンクへ */
		alu_vram_bank ++;
		bit <<= 1;
	}
}

/*
 *	論理演算
 *	コンペア
 */
static void FASTCALL alu_cmp(WORD addr)
{
	BYTE color;
	BYTE bit;
	int i, j;
	BOOL flag;
	BYTE dat;
	BYTE b, r, g;
	BYTE disflag;

	/* アドレスマスク */
#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* カラーデータ取得 */
	b = alu_read_plane(addr, 0);
	r = alu_read_plane(addr, 1);
	g = alu_read_plane(addr, 2);

	/* バンクディセーブルを考慮する(女神転生対策) */
	disflag = (BYTE)((~alu_disable) & 0x07);

	/* 比較が必要 */
	dat = 0;
	bit = 0x80;
	for (i=0; i<8; i++) {
		/* 色を作成 */
		color = 0;
		if (b & bit) {
			color |= 0x01;
		}
		if (r & bit) {
			color |= 0x02;
		}
		if (g & bit) {
			color |= 0x04;
		}

		/* 8つの色スロットをまわって、どれか一致するものがあるか */
		flag = FALSE;
		for (j=0; j<8; j++) {
			if ((alu_cmpdat[j] & 0x80) == 0) {
				if ((alu_cmpdat[j] & disflag) == (color & disflag)) {
					flag = TRUE;
					break;
				}
			}
		}

		/* イコールで1を設定 */
		if (flag) {
			dat |= bit;
		}

		/* 次へ */
		bit >>= 1;
	}

	/* データ設定 */
	alu_cmpstat = dat;
}

/*-[ 直線補間 ]-------------------------------------------------------------*/

/*
 *	直線補間
 *	論理演算回路起動
 */
void FASTCALL aluline_exec(WORD addr)
{
	/* アドレスチェック */
	if (addr >= 0x8000) {
		line_mask = 0xff;
		return;
	}

	/* マスクを設定 */
	alu_mask = line_mask;
	line_mask = 0xff;

	/* 論理演算 */
	switch (alu_command & 0x07) {
		/* PSET */
		case 0:
			alu_pset(addr);
			break;
		/* 禁止(黒色書き込み) */
		case 1:
			alu_prohibit(addr);
			break;
		/* OR */
		case 2:
			alu_or(addr);
			break;
		/* AND */
		case 3:
			alu_and(addr);
			break;
		/* XOR */
		case 4:
			alu_xor(addr);
			break;
		/* NOT */
		case 5:
			alu_not(addr);
			break;
		/* タイルペイント */
		case 6:
			alu_tile(addr);
			break;
		/* コンペア */
		case 7:
			alu_cmp(addr);
			break;
	}

	/* カウントアップ */
	line_count ++;
}

/*
 *	直線補間
 *	点描画
 */
static void FASTCALL aluline_pset(int x, int y)
{
	WORD addr;
	static BYTE mask[] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};

	/* 論理演算のデータバスに入るので、論理演算onが必要 */
	if (!(alu_command & 0x80)) {
		return;
	}

	/* 画面モードから、アドレス算出 */
#if XM7_VER >= 3
	if (screen_mode & SCR_ANALOG) {
		addr = (WORD)(y * 40 + (x >> 3));
	}
	else {
		addr = (WORD)(y * 80 + (x >> 3));
	}
#else
	if (mode320) {
		addr = (WORD)(y * 40 + (x >> 3));
	}
	else {
		addr = (WORD)(y * 80 + (x >> 3));
	}
#endif

	/* オフセットを加える */
	addr += line_offset;
#if XM7_VER >= 3
	if (mode400l) {
		addr &= 0x7fff;
	}
	else {
		addr &= 0x3fff;
	}
#else
	addr &= 0x3fff;
#endif

	/* 前回のアドレスと今回のアドレスが一致しない場合論理演算を行う */
	if (line_addr_old != addr) {
		/* 論理演算を動かす */
		aluline_exec(line_addr_old);
		line_addr_old = addr;
	}

	/* ラインスタイル */
	/* スタイルビットが立っている場合のみ */
	if (line_style & 0x8000) {
		/* マスクを設定 */
		line_mask &= mask[x & 0x07];
	}

	/* ラインスタイルレジスタを左ローテート */
	line_style = (WORD)((line_style << 1) | (line_style >> 15));
}

/*
 *	直線描画(DDA)
 *	参考文献:6809機械語によるグラフィック処理の技法 第6回 (◎h!FM '86/5)
 */
static void FASTCALL aluline_line(void)
{
	int x1, x2, y1, y2;
	int dx, dy, ux, uy, r;

	/* データ取得 */
	x1 = (int)line_x0;
	x2 = (int)line_x1;
	y1 = (int)line_y0;
	y2 = (int)line_y1;

	/* カウンタ初期化 */
	line_count = 0;
	line_addr_old = 0xffff;
	line_mask = 0xff;

	/* 初期値の計算 */
	dx = x2 - x1;
	dy = y2 - y1;
	if (dx < 0) {
		ux = -1;
		dx = -dx;
	}
	else {
		ux = 1;
	}
	if (dy < 0) {
		uy = -1;
		dy = -dy;
	}
	else {
		uy = 1;
	}

	if ((dx == 0) && (dy == 0)) {
		/* 単一点描画 */
		aluline_pset(x1, y1);
	}
	else if (dx == 0) {
		/* Xが同一の場合 (垂直) */
		for (;;) {
			aluline_pset(x1, y1);
			if (y1 == y2) {
				break;
			}
			y1 += uy;
		}
	}
	else if (dy == 0) {
		/* Yが同一の場合 (水平) */
		for (;;) {
			aluline_pset(x1, y1);
			if (x1 == x2) {
				break;
			}
			x1 += ux;
		}
	}
	else if (dx >= dy) {
		/* メインループ1 (DX >= DY) */
		r = dx >> 1;
		for (;;) {
			aluline_pset(x1, y1);
			if (x1 == x2) {
				break;
			}

			x1 += ux;
			r -= dy;
			if (r < 0) {
				r += dx;
				y1 += uy;
			}
		}
	}
	else {
		/* メインループ2 (DX < DY) */
		r = dy >> 1;
		for (;;) {
			aluline_pset(x1, y1);
			if (y1 == y2) {
				break;
			}

			y1 += uy;
			r -= dx;
			if (r < 0) {
				r += dy;
				x1 += ux;
			}
		}
	}

	/* 最後のバイトの論理演算 */
	aluline_exec(line_addr_old);
}


/*
 *	直線補間
 *	イベント
 */
static BOOL FASTCALL aluline_event(void)
{
	/* 直線補間をREADYにする */
	line_busy = FALSE;

	schedule_delevent(EVENT_LINE);
	return TRUE;
}

/*-[ メモリマップドI/O ]----------------------------------------------------*/

/*
 *	論理演算・直線補間
 *	１バイト読み出し
 */
BOOL FASTCALL aluline_readb(WORD addr, BYTE *dat)
{
	/* バージョンチェック */
	if (fm7_ver < 2) {
		return FALSE;
	}

	switch (addr) {
		/* 論理演算 コマンド */
		case 0xd410:
			*dat = alu_command;
			return TRUE;

		/* 論理演算 カラー */
		case 0xd411:
			*dat = alu_color;
			return TRUE;

		/* 論理演算 マスクビット */
		case 0xd412:
			*dat = alu_mask;
			return TRUE;

		/* 論理演算 比較ステータス */
		case 0xd413:
			*dat = alu_cmpstat;
			return TRUE;

		/* 論理演算 無効バンク */
		case 0xd41b:
			*dat = alu_disable;
			return TRUE;
	}

	/* 論理演算 比較データ */
	if ((addr >= 0xd413) && (addr <= 0xd41a)) {
		*dat = 0xff;
		return TRUE;
	}

	/* 論理演算 タイルパターン */
	if ((addr >= 0xd41c) && (addr <= 0xd41e)) {
		*dat = 0xff;
		return TRUE;
	}

	/* 直線補間 */
	if ((addr >= 0xd420) && (addr <= 0xd42b)) {
		*dat = 0xff;
		return TRUE;
	}

	return FALSE;
}

/*
 *	論理演算・直線補間
 *	１バイト書き込み
 */
BOOL FASTCALL aluline_writeb(WORD addr, BYTE dat)
{
	DWORD tmp;

	/* バージョンチェック */
	if (fm7_ver < 2) {
		return FALSE;
	}

	switch (addr) {
		/* 論理演算 コマンド */
		case 0xd410:
			alu_command = dat;
			return TRUE;

		/* 論理演算 カラー */
		case 0xd411:
			alu_color = dat;
			return TRUE;

		/* 論理演算 マスクビット */
		case 0xd412:
			alu_mask = dat;
			return TRUE;

		/* 論理演算 無効バンク */
		case 0xd41b:
			alu_disable = dat;
			return TRUE;

		/* 直線補間 アドレスオフセット(A1から注意) */
		case 0xd420:
			line_offset &= 0x01fe;
			line_offset |= (WORD)((dat * 512) & 0x3e00);
			return TRUE;
		case 0xd421:
			line_offset &= 0x3e00;
			line_offset |= (WORD)(dat * 2);
			return TRUE;

		/* 直線補間 ラインスタイル */
		case 0xd422:
			line_style &= 0xff;
			line_style |= (WORD)(dat * 256);
			return TRUE;
		case 0xd423:
			line_style &= 0xff00;
			line_style |= (WORD)dat;
			return TRUE;

		/* 直線補間 X0 */
		case 0xd424:
			line_x0 &= 0xff;
			line_x0 |= (WORD)(dat * 256);
			line_x0 &= 0x03ff;
			return TRUE;
		case 0xd425:
			line_x0 &= 0xff00;
			line_x0 |= (WORD)dat;
			return TRUE;

		/* 直線補間 Y0 */
		case 0xd426:
			line_y0 &= 0xff;
			line_y0 |= (WORD)(dat * 256);
			line_y0 &= 0x01ff;
			return TRUE;
		case 0xd427:
			line_y0 &= 0xff00;
			line_y0 |= (WORD)dat;
			return TRUE;

		/* 直線補間 X1 */
		case 0xd428:
			line_x1 &= 0xff;
			line_x1 |= (WORD)(dat * 256);
			line_x1 &= 0x03ff;
			return TRUE;
		case 0xd429:
			line_x1 &= 0xff00;
			line_x1 |= (WORD)dat;
			return TRUE;

		/* 直線補間 Y1 */
		case 0xd42a:
			line_y1 &= 0xff;
			line_y1 |= (WORD)(dat * 256);
			line_y1 &= 0x01ff;
			return TRUE;
		case 0xd42b:
			line_y1 &= 0xff00;
			line_y1 |= (WORD)dat;

			/* ここで直線補間スタート */
			aluline_line();

			/* 線は引いたが、しばらくBUSYにしておく */
			/* BUSY状態にする時間を算出(1バイト描画=1/16μsecと仮定) */
			tmp = (DWORD)(line_count >> 4);

			/* BUSY時間の小数部調整を行う */
			line_count_sub += (BYTE)(line_count & 15);
			if (line_count_sub >= 16) {
				tmp ++;
				line_count_sub &= (BYTE)15;
			}

			/* 計算結果が1μs以上の場合はBUSYにする */
			if (tmp > 0) {
				line_busy = TRUE;
				schedule_setevent(EVENT_LINE, tmp, aluline_event);
			}

			return TRUE;
	}

	/* 論理演算 比較データ */
	if ((addr >= 0xd413) && (addr <= 0xd41a)) {
		alu_cmpdat[addr - 0xd413] = dat;
		return TRUE;
	}

	/* 論理演算 タイルパターン */
	if ((addr >= 0xd41c) && (addr <= 0xd41e)) {
		alu_tiledat[addr - 0xd41c] = dat;
		return TRUE;
	}

	return FALSE;
}

/*
 *	論理演算・直線補間
 *	VRAMダミーリード
 */
void FASTCALL aluline_extrb(WORD addr)
{
	/* 論理演算が有効か */
	if (alu_command & 0x80) {
		/* コマンド別 */
		switch (alu_command & 0x07) {
			/* PSET */
			case 0:
				alu_pset(addr);
				break;
			/* 禁止(黒色書き込み) */
			case 1:
				alu_prohibit(addr);
				break;
			/* OR */
			case 2:
				alu_or(addr);
				break;
			/* AND */
			case 3:
				alu_and(addr);
				break;
			/* XOR */
			case 4:
				alu_xor(addr);
				break;
			/* NOT */
			case 5:
				alu_not(addr);
				break;
			/* タイルペイント */
			case 6:
				alu_tile(addr);
				break;
			/* コンペア */
			case 7:
				alu_cmp(addr);
				break;
		}
	}
}

/*-[ ファイルI/O ]----------------------------------------------------------*/

/*
 *	論理演算・直線補間
 *	セーブ
 */
BOOL FASTCALL aluline_save(int fileh)
{
	if (!file_byte_write(fileh, alu_command)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_color)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_mask)) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_cmpstat)) {
		return FALSE;
	}
	if (!file_write(fileh, alu_cmpdat, sizeof(alu_cmpdat))) {
		return FALSE;
	}
	if (!file_byte_write(fileh, alu_disable)) {
		return FALSE;
	}
	if (!file_write(fileh, alu_tiledat, sizeof(alu_tiledat))) {
		return FALSE;
	}

	if (!file_bool_write(fileh, line_busy)) {
		return FALSE;
	}

	if (!file_word_write(fileh, line_offset)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_style)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_x0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_y0)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_x1)) {
		return FALSE;
	}
	if (!file_word_write(fileh, line_y1)) {
		return FALSE;
	}

	/* 従来バージョンとの互換用ダミー */
	if (!file_byte_write(fileh, 0)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	論理演算・直線補間
 *	ロード
 */
BOOL FASTCALL aluline_load(int fileh, int ver)
{
	BYTE tmp;

	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &alu_command)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_color)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_mask)) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_cmpstat)) {
		return FALSE;
	}
	if (!file_read(fileh, alu_cmpdat, sizeof(alu_cmpdat))) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &alu_disable)) {
		return FALSE;
	}
	if (!file_read(fileh, alu_tiledat, sizeof(alu_tiledat))) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &line_busy)) {
		return FALSE;
	}

	if (!file_word_read(fileh, &line_offset)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_style)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_x0)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_y0)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_x1)) {
		return FALSE;
	}
	if (!file_word_read(fileh, &line_y1)) {
		return FALSE;
	}

	/* 従来バージョンとの互換用ダミー */
	if (!file_byte_read(fileh, &tmp)) {
		return FALSE;
	}

	/* イベント */
	schedule_handle(EVENT_LINE, aluline_event);

	return TRUE;
}

#endif /* XM7_VER >= 2 */

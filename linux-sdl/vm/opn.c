/*
 *	FM-7 EMULATOR "XM7"
 *
 *	Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *	Copyright (C) 2001-2010 Ryu Takegami
 *
 *	[ OPN/WHG/THG (YM2203) ]
 */

#include <string.h>
#include "xm7.h"
#include "opn.h"
#include "device.h"
#include "mainetc.h"
#include "event.h"
#include "mouse.h"

/*
 *	グローバル ワーク
 */
BOOL opn_enable;						/* OPN有効・無効フラグ(7 only) */
BOOL whg_enable;						/* WHG有効・無効フラグ */
BOOL whg_use;							/* WHG使用フラグ */
BOOL thg_enable;						/* THG有効・無効フラグ */
BOOL thg_use;							/* THG使用フラグ */

BYTE opn_reg[3][256];					/* OPNレジスタ */
BOOL opn_key[3][4];						/* OPNキーオンフラグ */
BOOL opn_timera[3];						/* タイマーA動作フラグ */
BOOL opn_timerb[3];						/* タイマーB動作フラグ */
DWORD opn_timera_tick[3];				/* タイマーA間隔 */
DWORD opn_timerb_tick[3];				/* タイマーB間隔 */
BYTE opn_scale[3];						/* プリスケーラ */

/*
 *	スタティック ワーク
 */
static BYTE opn_pstate[3];				/* ポート状態 */
static BYTE opn_selreg[3];				/* セレクトレジスタ */
static BYTE opn_seldat[3];				/* セレクトデータ */
static BOOL opn_timera_int[3];			/* タイマーAオーバーフロー */
static BOOL opn_timerb_int[3];			/* タイマーBオーバーフロー */
static BOOL opn_timera_en[3];			/* タイマーAイネーブル */
static BOOL opn_timerb_en[3];			/* タイマーBイネーブル */

/*
 *	プロトタイプ宣言
 */
static void FASTCALL opn_notify_no(int no, BYTE reg, BYTE dat);
static void FASTCALL opn_cleanup_common(int no);
static void FASTCALL opn_reset_common(int no);

static BOOL FASTCALL opn_timera_event(void);
static BOOL FASTCALL whg_timera_event(void);
static BOOL FASTCALL thg_timera_event(void);
static BOOL FASTCALL opn_timerb_event(void);
static BOOL FASTCALL whg_timerb_event(void);
static BOOL FASTCALL thg_timerb_event(void);
static void FASTCALL opn_timera_calc(int no);
static void FASTCALL opn_timerb_calc(int no);


/*
 *	OPN
 *	初期化
 */
BOOL FASTCALL opn_init(void)
{
	memset(opn_reg, 0, sizeof(opn_reg));

	/* デフォルトはOPN有効 */
	opn_enable = TRUE;

	return TRUE;
}

/*
 *	OPN
 *	クリーンアップ
 */
void FASTCALL opn_cleanup(void)
{
	opn_cleanup_common(OPN_STD);
}

/*
 *	OPN
 *	リセット
 */
void FASTCALL opn_reset(void)
{
	/* リセット */
	opn_reset_common(OPN_STD);
}

/*
 *	WHG
 *	初期化
 */
BOOL FASTCALL whg_init(void)
{
	/* WHG有効 */
	whg_enable = TRUE;
	whg_use = FALSE;

	return TRUE;
}

/*
 *	WHG
 *	クリーンアップ
 */
void FASTCALL whg_cleanup(void)
{
	opn_cleanup_common(OPN_WHG);
}

/*
 *	WHG
 *	リセット
 */
void FASTCALL whg_reset(void)
{
	/* 使用フラグを下げる */
	whg_use = FALSE;

	/* リセット */
	opn_reset_common(OPN_WHG);
}

/*
 *	THG
 *	初期化
 */
BOOL FASTCALL thg_init(void)
{
	/* THG有効 */
	thg_enable = FALSE;
	thg_use = FALSE;

	return TRUE;
}

/*
 *	THG
 *	クリーンアップ
 */
void FASTCALL thg_cleanup(void)
{
	opn_cleanup_common(OPN_THG);
}

/*
 *	THG
 *	リセット
 */
void FASTCALL thg_reset(void)
{
	/* 使用フラグを下げる */
	thg_use = FALSE;

	/* リセット */
	opn_reset_common(OPN_THG);
}

/*-[ 共通処理(Reset/Cleanup) ]-----------------------------------------------*/

/*
 *	OPN
 *	クリーンアップ共通処理
 */
static void FASTCALL opn_cleanup_common(int no)
{
	BYTE i;

	/* PSG */
	for (i=0; i<6; i++) {
		opn_notify_no(no, i, 0);
	}
	opn_notify_no(no, 7, 0xff);

	/* TL=$7F */
	for (i=0x40; i<0x50; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0x7f);
	}

	/* キーオフ */
	for (i=0; i<3; i++) {
		opn_notify_no(no, 0x28, i);
		opn_key[no][i] = FALSE;
	}
}

/*
 *	OPN
 *	リセット共通処理
 */
static void FASTCALL opn_reset_common(int no)
{
	BYTE i;

	/* レジスタクリア */
	memset(opn_reg[no], 0, sizeof(opn_reg[no]));

	/* タイマーOFF */
	opn_timera[no] = FALSE;
	opn_timerb[no] = FALSE;

	/* I/O初期化 */
	opn_pstate[no] = OPN_INACTIVE;
	opn_selreg[no] = 0;
	opn_seldat[no] = 0;

	/* デバイス */
	opn_timera_int[no] = FALSE;
	opn_timerb_int[no] = FALSE;
	opn_timera_tick[no] = 0;
	opn_timerb_tick[no] = 0;
	opn_timera_en[no] = FALSE;
	opn_timerb_en[no] = FALSE;
	opn_scale[no] = 3;

	/* PSG初期化 */
	for (i=0; i<14;i++) {
		if (i == 7) {
			opn_notify_no(no, i, 0xff);
			opn_reg[no][i] = 0xff;
		}
		else {
			opn_notify_no(no, i, 0);
		}
	}

	/* MUL,DT */
	for (i=0x30; i<0x40; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0);
	}

	/* TL=$7F */
	for (i=0x40; i<0x50; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0x7f);
		opn_reg[no][i] = 0x7f;
	}

	/* AR=$1F */
	for (i=0x50; i<0x60; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0x1f);
		opn_reg[no][i] = 0x1f;
	}

	/* その他 */
	for (i=0x60; i<0xb4; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0);
	}

	/* SL,RR */
	for (i=0x80; i<0x90; i++) {
		if ((i & 0x03) == 3) {
			continue;
		}
		opn_notify_no(no, i, 0xff);
		opn_reg[no][i] = 0xff;
	}

	/* キーオフ */
	for (i=0; i<3; i++) {
		opn_notify_no(no, 0x28, i);
		opn_key[no][i] = FALSE;
	}

	/* モード */
	opn_notify_no(no, 0x27, 0);
}

/*-[ 共通処理(VM I/F) ]------------------------------------------------------*/

/*
 *	OPN
 *	レジスタ書き込み通知
 */
static void FASTCALL opn_notify_no(int no, BYTE reg, BYTE dat)
{
	/* 各音源の通知関数を呼ぶだけ */
	switch (no) {
		case OPN_STD:
			opn_notify(reg, dat);
			break;
		case OPN_WHG:
			whg_notify(reg, dat);
			break;
		case OPN_THG:
			thg_notify(reg, dat);
			break;
		default:
			ASSERT(FALSE);
			break;
	}
}

/*-[ 共通処理(タイマー) ]----------------------------------------------------*/

/*
 *	OPN
 *	IRQフラグ共通処理
 */
static void FASTCALL opn_set_irq_flag(int no, BOOL flag)
{
	switch (no) {
		case OPN_STD:
			opn_irq_flag = flag;
			break;
		case OPN_WHG:
			whg_irq_flag = flag;
			break;
		case OPN_THG:
			thg_irq_flag = flag;
			break;
		default:
			ASSERT(FALSE);
			break;
	}

#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return;
	}
#endif
	maincpu_irq();
}

/*
 *	OPN
 *	タイマーAオーバフロー共通処理
 */
static BOOL FASTCALL opn_timera_event_main(int no)
{
	ASSERT ((no >= 0) && (no <= 2));

	/* イネーブルか */
	if (opn_timera_en[no]) {
		/* オーバーフローアクションが有効か */
		if (opn_timera[no]) {
			opn_timera[no] = FALSE;
			opn_timera_int[no] = TRUE;

			/* 割り込みをかける */
			opn_set_irq_flag(no, TRUE);
		}
	}

	/* CSM音声合成モードでのキーオン */
	opn_notify_no(no, 0xff, 0);

	/* タイマー再設定 */
	opn_timera_calc(no);

	/* タイマーは回し続ける */
	return TRUE;
}

/*
 *	OPN
 *	タイマーBオーバフロー共通処理
 */
static BOOL FASTCALL opn_timerb_event_main(int no)
{
	ASSERT ((no >= 0) && (no <= 2));

	/* イネーブルか */
	if (opn_timerb_en[no]) {
		/* オーバーフローアクションが有効か */
		if (opn_timerb[no]) {
			/* フラグ変更 */
			opn_timerb[no] = FALSE;
			opn_timerb_int[no] = TRUE;

			/* 割り込みをかける */
			opn_set_irq_flag(no, TRUE);
		}
	}

	/* タイマー再設定 */
	opn_timerb_calc(no);

	/* タイマーは回し続ける */
	return TRUE;
}

/*
 *	OPN
 *	タイマーAインターバル算出共通処理
 */
static void FASTCALL opn_timera_calc(int no)
{
	DWORD t;
	BYTE temp;

	ASSERT ((no >= 0) && (no <= 2));

	t = opn_reg[no][0x24];
	t *= 4;
	temp = (BYTE)(opn_reg[no][0x25] & 3);
	t |= temp;
	t &= 0x3ff;
	t = (1024 - t);
	t *= opn_scale[no];
	t *= 12;
	t *= 10000;
	t /= OPN_CLOCK;

	/* タイマー値を設定 */
	if (t != opn_timera_tick[no]) {
		opn_timera_tick[no] = t;
		switch (no) {
			case OPN_STD:
				schedule_setevent(EVENT_OPN_A, opn_timera_tick[OPN_STD],
					opn_timera_event);
				break;
			case OPN_WHG:
				schedule_setevent(EVENT_WHG_A, opn_timera_tick[OPN_WHG],
					whg_timera_event);
				break;
			case OPN_THG:
				schedule_setevent(EVENT_THG_A, opn_timera_tick[OPN_THG],
					thg_timera_event);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
}

/*
 *	OPN
 *	タイマーBインターバル算出共通処理
 */
static void FASTCALL opn_timerb_calc(int no)
{
	DWORD t;

	ASSERT ((no >= 0) && (no <= 2));

	t = opn_reg[no][0x26];
	t = (256 - t);
	t *= 192;
	t *= opn_scale[no];
	t *= 10000;
	t /= OPN_CLOCK;

	/* タイマー値を設定 */
	if (t != opn_timerb_tick[no]) {
		opn_timerb_tick[no] = t;
		switch (no) {
			case OPN_STD:
				schedule_setevent(EVENT_OPN_B, opn_timerb_tick[OPN_STD],
					opn_timerb_event);
				break;
			case OPN_WHG:
				schedule_setevent(EVENT_WHG_B, opn_timerb_tick[OPN_WHG],
					whg_timerb_event);
				break;
			case OPN_THG:
				schedule_setevent(EVENT_THG_B, opn_timerb_tick[OPN_THG],
					thg_timerb_event);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
}

/*-[ 共通処理(レジスタアレイ) ]----------------------------------------------*/

/*
 *	OPN
 *	レジスタアレイより読み出し
 */
static BYTE FASTCALL opn_readreg(int no, BYTE reg)
{
	/* レジスタビットマスクデータ(PSG) */
	static const BYTE opn_bitmask[16] = {
		0xff,	0x0f,	0xff,	0x0f,
		0xff,	0x0f,	0x1f,	0xff,
		0x1f,	0x1f,	0x1f,	0xff,
		0xff,	0x0f,	0xff,	0xff,
	};

	ASSERT ((no >= 0) && (no <= 2));

	/* FM音源部は読み出せない */
	if (reg >= 0x10) {
		return 0xff;
	}

	return (BYTE)(opn_reg[no][reg] & opn_bitmask[reg]);
}

/*
 *	OPN
 *	レジスタアレイへ書き込み
 */
static void FASTCALL opn_writereg(int no, BYTE reg, BYTE dat)
{
#ifdef MOUSE
	BOOL strobe;
	BYTE mask;
#endif

	ASSERT ((no >= 0) && (no <= 2));

	if (no == OPN_WHG) {
		/* フラグオン */
		whg_use = TRUE;
	}

	/* タイマー処理 */
	/* このレジスタは非常に難しい。良く分からないまま扱っている人が大半では？ */
	if (reg == 0x27) {
		/* オーバーフローフラグのクリア */
		if (dat & 0x10) {
			opn_timera_int[no] = FALSE;
		}
		if (dat & 0x20) {
			opn_timerb_int[no] = FALSE;
		}

		/* 両方落ちたら、割り込みを落とす */
		if (!opn_timera_int[no] && !opn_timerb_int[no]) {
			opn_set_irq_flag(no, FALSE);
		}

		/* タイマーA */
		if (dat & 0x01) {
			/* 0→1でタイマー値をロード、それ以外でもタイマーon */
			if ((opn_reg[no][0x27] & 0x01) == 0) {
				if (!opn_timera[no]) {
					/* オーバーフロー済みの場合、再設定 */
					opn_timera_tick[no] = 0;
				}
				opn_timera_calc(no);
			}
			opn_timera_en[no] = TRUE;
		}
		else {
			opn_timera_en[no] = FALSE;
		}
		if (dat & 0x04) {
			opn_timera[no] = TRUE;
		}
		else {
			opn_timera[no] = FALSE;
		}

		/* タイマーB */
		if (dat & 0x02) {
			/* 0→1でタイマー値をロード、それ以外でもタイマーon */
			if ((opn_reg[no][0x27] & 0x02) == 0) {
				if (!opn_timerb[no]) {
					/* オーバーフロー済みの場合、再設定 */
					opn_timerb_tick[no] = 0;
				}
				opn_timerb_calc(no);
			}
			opn_timerb_en[no] = TRUE;
		}
		else {
			opn_timerb_en[no] = FALSE;
		}
		if (dat & 0x08) {
			opn_timerb[no] = TRUE;
		}
		else {
			opn_timerb[no] = FALSE;
		}

		/* データ記憶 */
		opn_reg[no][reg] = dat;

		/* モードのみ出力 */
		opn_notify_no(no, 0x27, (BYTE)(dat & 0xc0));
		return;
	}

	/* データ記憶 */
	opn_reg[no][reg] = dat;

	switch (reg) {
#ifdef MOUSE
		/* 出力ポート */
		case 0x0f:
			/* マウスエミュレーションは標準OPNのみ有効 */
			if ((no == OPN_STD) && mos_capture) {
				if (mos_port == 1) {
					mask = 0x10;
				}
				else {
					mask = 0x20;
				}

				if (opn_reg[OPN_STD][15] & mask) {
					strobe = TRUE;
				}
				else {
					strobe = FALSE;
				}
				mos_strobe_signal(strobe);
			}
			return;
#endif

		/* プリスケーラ１ */
		case 0x2d:
			if (opn_scale[no] != 3) {
				opn_scale[no] = 6;
				opn_timera_calc(no);
				opn_timerb_calc(no);
			}
			return;

		/* プリスケーラ２ */
		case 0x2e:
			opn_scale[no] = 3;
			opn_timera_calc(no);
			opn_timerb_calc(no);
			return;

		/* プリスケーラ３ */
		case 0x2f:
			opn_scale[no] = 2;
			opn_timera_calc(no);
			opn_timerb_calc(no);
			return;

		/* タイマーA */
		case 0x24:
		case 0x25:
			opn_timera_tick[no] = 0;
			return;

		/* タイマーB */
		case 0x26:
			opn_timerb_tick[no] = 0;
			return;
	}

	/* 出力先を絞る */
	if ((reg >= 14) && (reg <= 0x26)) {
		return;
	}
	if ((reg >= 0x29) && (reg <= 0x2c)) {
		return;
	}

	/* キーオン */
	if (reg == 0x28) {
		if (dat >= 16) {
			opn_key[no][dat & 0x03] = TRUE;
		}
		else {
			opn_key[no][dat & 0x03] = FALSE;
		}
	}

	/* 出力 */
	opn_notify_no(no, reg, dat);
}

/*-[ 共通処理(I/Oポート) ]---------------------------------------------------*/

/*
 *	OPN
 *	データレジスタ読み込み共通処理
 */
static void FASTCALL opn_read_data_reg(int no, BYTE *dat)
{
#ifdef MOUSE
	BYTE port, trigger;
#endif

	ASSERT ((no >= 0) && (no <= 2));

	switch (opn_pstate[no]) {
		/* 通常コマンド */
		case OPN_INACTIVE:
		case OPN_READDAT:
		case OPN_WRITEDAT:
		case OPN_ADDRESS:
			*dat = opn_seldat[no];
			break;

		/* ステータス読み出し */
		case OPN_READSTAT:
			*dat = 0;
			if (opn_timera_int[no]) {
				*dat |= 0x01;
			}
			if (opn_timerb_int[no]) {
				*dat |= 0x02;
			}
			break;

		/* ジョイスティック読み取り */
		case OPN_JOYSTICK:
			if (opn_selreg[no] == 14) {
				/* ジョイスティックポートエミュレーションは標準OPNのみ */
				if (no == OPN_STD) {
#ifdef MOUSE
					if (mos_capture) {
						/* マウス */
						if (mos_port == 1) {
							port = 0x00;
							trigger = (BYTE)(opn_reg[no][15] & 0x03);
						}
						else {
							port = 0x40;
							trigger = (BYTE)((opn_reg[no][15] >> 2) & 0x03);
						}

						if ((opn_reg[no][15] & 0xc0) == port) {
							*dat = (BYTE)(mos_readdata(trigger) | 0xc0);
							break;
						}
					}
#endif
					if ((opn_reg[no][15] & 0xf0) == 0x20) {
						/* ジョイスティック１ */
						*dat = (BYTE)(~joy_request(0) | 0xc0);
						break;
					}
					if ((opn_reg[no][15] & 0xf0) == 0x50) {
						/* ジョイスティック２ */
						*dat = (BYTE)(~joy_request(1) | 0xc0);
						break;
					}
				}

				/* それ以外 */
				*dat = 0xff;
			}
			else {
				/* レジスタが14でなければ、FF以外を返す */
				/* HOW MANY ROBOT対策 */
				*dat = 0;
			}
			break;
	}
}

/*
 *	OPN
 *	割り込みステータス読み込み共通処理
 */
static void FASTCALL opn_read_interrupt_reg(int no, BYTE *dat)
{
	ASSERT ((no >= 0) && (no <= 2));

	if (opn_timera_int[no] || opn_timerb_int[no]) {
		*dat = 0xf7;
	}
	else {
		*dat = 0xff;
	}
}

/*
 *	OPN
 *	コマンドレジスタ書き込み共通処理
 */
static void FASTCALL opn_write_command_reg(int no, BYTE dat)
{
	ASSERT ((no >= 0) && (no <= 2));

	switch (dat & 0x0f) {
		/* インアクティブ(動作定義なし、データレジスタ書き換え可) */
		case OPN_INACTIVE:
			opn_pstate[no] = OPN_INACTIVE;
			break;
		/* データ読み出し */
		case OPN_READDAT:
			opn_pstate[no] = OPN_READDAT;
			opn_seldat[no] = opn_readreg(no, opn_selreg[no]);
			break;
		/* データ書き込み */
		case OPN_WRITEDAT:
			opn_pstate[no] = OPN_WRITEDAT;
			opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			break;
		/* ラッチアドレス */
		case OPN_ADDRESS:
			opn_pstate[no] = OPN_ADDRESS;
			opn_selreg[no] = opn_seldat[no];

			/* プリスケーラはアドレス指定のみでok */
			if ((opn_selreg[no] >= 0x2d) && (opn_selreg[no] <= 0x2f)) {
				opn_seldat[no] = 0;
				opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			}
			break;
		/* リードステータス */
		case OPN_READSTAT:
			opn_pstate[no] = OPN_READSTAT;
			break;
		/* ジョイスティック読み取り */
		case OPN_JOYSTICK:
			opn_pstate[no] = OPN_JOYSTICK;
			break;
	}
}

/*
 *	OPN
 *	データレジスタ書き込み共通処理
 */
static void FASTCALL opn_write_data_reg(int no, BYTE dat)
{
	ASSERT ((no >= 0) && (no <= 2));

	/* データを記憶 */
	opn_seldat[no] = dat;

	/* インアクティブ以外の場合は、所定の動作を行う */
	switch (opn_pstate[no]) {
		/* データ書き込み */
		case OPN_WRITEDAT:
			opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			break;
		/* ラッチアドレス */
		case OPN_ADDRESS:
			opn_selreg[no] = opn_seldat[no];

			/* プリスケーラはアドレス指定のみでok */
			if ((opn_selreg[no] >= 0x2d) && (opn_selreg[no] <= 0x2f)) {
				opn_seldat[no] = 0;
				opn_writereg(no, opn_selreg[no], opn_seldat[no]);
			}
			break;
	}
}

/*-[ 共通処理(ステート) ]----------------------------------------------------*/

/*
 *	OPN
 *	ステートセーブ共通処理
 */
static BOOL FASTCALL opn_save_common(int no, SDL_RWops *fileh)
{
	ASSERT ((no >= 0) && (no <= 2));

	if (!file_write(fileh, opn_reg[no], 256)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timera[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timerb[no])) {
		return FALSE;
	}
	if (!file_dword_write(fileh, opn_timera_tick[no])) {
		return FALSE;
	}
	if (!file_dword_write(fileh, opn_timerb_tick[no])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, opn_scale[no])) {
		return FALSE;
	}

	if (!file_byte_write(fileh, opn_pstate[no])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, opn_selreg[no])) {
		return FALSE;
	}
	if (!file_byte_write(fileh, opn_seldat[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timera_int[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timerb_int[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timera_en[no])) {
		return FALSE;
	}
	if (!file_bool_write(fileh, opn_timerb_en[no])) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	OPN
 *	ステートロード共通処理
 */
static BOOL FASTCALL opn_load_common(int no, SDL_RWops *fileh)
{
	ASSERT ((no >= 0) && (no <= 2));

	if (!file_read(fileh, opn_reg[no], 256)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timera[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timerb[no])) {
		return FALSE;
	}
	if (!file_dword_read(fileh, &opn_timera_tick[no])) {
		return FALSE;
	}
	if (!file_dword_read(fileh, &opn_timerb_tick[no])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &opn_scale[no])) {
		return FALSE;
	}

	if (!file_byte_read(fileh, &opn_pstate[no])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &opn_selreg[no])) {
		return FALSE;
	}
	if (!file_byte_read(fileh, &opn_seldat[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timera_int[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timerb_int[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timera_en[no])) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &opn_timerb_en[no])) {
		return FALSE;
	}

	return TRUE;
}

/*-[ 標準OPN ]---------------------------------------------------------------*/

/*
 *	OPN
 *	タイマーAオーバフロー
 */
static BOOL FASTCALL opn_timera_event(void)
{
	return opn_timera_event_main(OPN_STD);
}

/*
 *	OPN
 *	タイマーBオーバフロー
 */
static BOOL FASTCALL opn_timerb_event(void)
{
	return opn_timerb_event_main(OPN_STD);
}

/*
 *	OPN
 *	１バイト読み出し
 */
BOOL FASTCALL opn_readb(WORD addr, BYTE *dat)
{
	/* FM-7モード時OPN有効チェック */
#if XM7_VER >= 2
	if ((fm7_ver == 1) && (!opn_enable)) {
#else
	if (!opn_enable) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* コマンドレジスタは読み出し禁止 */
		case 0xfd0d:
			/* FM-7モードではTHG SSGを使用するため無効 */
			if (fm7_ver == 1) {
				return FALSE;
			}
		case 0xfd15:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif
			*dat = 0xff;
			return TRUE;

		/* データレジスタ */
		case 0xfd0e:
			/* FM-7モードではTHG SSGを使用するため無効 */
			if (fm7_ver == 1) {
				return FALSE;
			}
		case 0xfd16:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif
			opn_read_data_reg(OPN_STD, dat);
			return TRUE;

		/* 拡張割り込みステータス */
		case 0xfd17:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif
			opn_read_interrupt_reg(OPN_STD, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	OPN
 *	１バイト書き込み
 */
BOOL FASTCALL opn_writeb(WORD addr, BYTE dat)
{
	/* FM-7モード時OPN有効チェック */
#if XM7_VER >= 2
	if ((fm7_ver == 1) && (!opn_enable)) {
#else
	if (!opn_enable) {
#endif
		return FALSE;
	}

	switch (addr) {
		/* OPNコマンドレジスタ */
		case 0xfd0d:
			/* FM-7モードではTHG SSGを使用するため無効 */
			if (fm7_ver == 1) {
				return FALSE;
			}
			/* PSGアクセス時は下位2bitのみ有効 */
			dat &= 0x03;
		case 0xfd15:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif
			opn_write_command_reg(OPN_STD, dat);
			return TRUE;

		/* データレジスタ */
		case 0xfd0e:
			/* FM-7モードではTHG SSGを使用するため無効 */
			if (fm7_ver == 1) {
				return FALSE;
			}
		case 0xfd16:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif
			opn_write_data_reg(OPN_STD, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	OPN
 *	セーブ
 */
BOOL FASTCALL opn_save(SDL_RWops *fileh)
{
	if (!opn_save_common(OPN_STD, fileh)) {
		return FALSE;
	}
	return TRUE;
}

/*
 *	OPN
 *	ロード
 */
BOOL FASTCALL opn_load(SDL_RWops *fileh, int ver)
{
	int i;

	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!opn_load_common(OPN_STD, fileh)) {
		return FALSE;
	}

	/* OPNレジスタ復旧 */
	opn_notify(0x27, (BYTE)(opn_reg[OPN_STD][0x27] & 0xc0));

	for (i=0; i<3; i++) {
		opn_notify(0x28, (BYTE)i);
		opn_key[OPN_STD][i] = FALSE;
	}

	for (i=0; i<13; i++) {
		if ((i < 8) || (i > 10)) {
			opn_notify((BYTE)i, opn_reg[OPN_STD][i]);
		}
		else {
			opn_notify((BYTE)i, 0);
		}
	}

	for (i=0x30; i<0xb4; i++) {
		opn_notify((BYTE)i, opn_reg[OPN_STD][i]);
	}

	/* イベント */
	schedule_handle(EVENT_OPN_A, opn_timera_event);
	schedule_handle(EVENT_OPN_B, opn_timerb_event);

	return TRUE;
}

/*-[ WHG OPN ]---------------------------------------------------------------*/

/*
 *	WHG
 *	タイマーAオーバフロー
 */
static BOOL FASTCALL whg_timera_event(void)
{
	return opn_timera_event_main(OPN_WHG);
}

/*
 *	WHG
 *	タイマーBオーバフロー
 */
static BOOL FASTCALL whg_timerb_event(void)
{
	return opn_timerb_event_main(OPN_WHG);
}

/*
 *	WHG
 *	１バイト読み出し
 */
BOOL FASTCALL whg_readb(WORD addr, BYTE *dat)
{
	/* 有効フラグをチェック、無効なら何もしない */
	if (!whg_enable) {
		return FALSE;
	}
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		/* コマンドレジスタは読み出し禁止 */
		case 0xfd45:
			*dat = 0xff;
			return TRUE;

		/* データレジスタ */
		case 0xfd46:
			opn_read_data_reg(OPN_WHG, dat);
			return TRUE;

		/* 拡張割り込みステータス */
		case 0xfd47:
			opn_read_interrupt_reg(OPN_WHG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	WHG
 *	１バイト書き込み
 */
BOOL FASTCALL whg_writeb(WORD addr, BYTE dat)
{
	/* 有効フラグをチェック、無効なら何もしない */
	if (!whg_enable) {
		return FALSE;
	}
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
		return FALSE;
	}
#endif

	switch (addr) {
		/* WHG コマンドレジスタ */
		case 0xfd45:
			opn_write_command_reg(OPN_WHG, dat);
			return TRUE;

		/* WHG データレジスタ */
		case 0xfd46:
			opn_write_data_reg(OPN_WHG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	WHG
 *	セーブ
 */
BOOL FASTCALL whg_save(SDL_RWops *fileh)
{
	if (!file_bool_write(fileh, whg_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, whg_use)) {
		return FALSE;
	}
	if (!opn_save_common(OPN_WHG, fileh)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	WHG
 *	ロード
 */
BOOL FASTCALL whg_load(SDL_RWops *fileh, int ver)
{
	int i;

	/* ファイルバージョン3で追加 */
	if (ver < 300) {
		whg_use = FALSE;
		return TRUE;
	}

	if (!file_bool_read(fileh, &whg_enable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &whg_use)) {
		return FALSE;
	}
	if (!opn_load_common(OPN_WHG, fileh)) {
		return FALSE;
	}

	/* WHGレジスタ復旧 */
	whg_notify(0x27, (BYTE)(opn_reg[OPN_WHG][0x27] & 0xc0));

	for (i=0; i<3; i++) {
		whg_notify(0x28, (BYTE)i);
		opn_key[OPN_WHG][i] = FALSE;
	}

	for (i=0; i<13; i++) {
		if ((i < 8) || (i > 10)) {
			whg_notify((BYTE)i, opn_reg[OPN_WHG][i]);
		}
		else {
			whg_notify((BYTE)i, 0);
		}
	}

	for (i=0x30; i<0xb4; i++) {
		whg_notify((BYTE)i, opn_reg[OPN_WHG][i]);
	}

	/* イベント */
	schedule_handle(EVENT_WHG_A, whg_timera_event);
	schedule_handle(EVENT_WHG_B, whg_timerb_event);

	return TRUE;
}

/*-[ THG OPN ]---------------------------------------------------------------*/

/*
 *	THG
 *	タイマーAオーバフロー
 */
static BOOL FASTCALL thg_timera_event(void)
{
	return opn_timera_event_main(OPN_THG);
}

/*
 *	THG
 *	タイマーBオーバフロー
 */
static BOOL FASTCALL thg_timerb_event(void)
{
	return opn_timerb_event_main(OPN_THG);
}

/*
 *	THG
 *	１バイト読み出し
 */
BOOL FASTCALL thg_readb(WORD addr, BYTE *dat)
{
	switch (addr) {
		/* コマンドレジスタは読み出し禁止 */
		case 0xfd0d:
#if XM7_VER >= 2
			/* FM-7モードのみ有効 */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
		case 0xfd51:
			/* 有効フラグをチェック、無効なら何もしない */
			if ((!thg_enable) && (addr == 0xfd51)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd51)) {
				return FALSE;
			}
#endif

			*dat = 0xff;
			return TRUE;

		/* データレジスタ */
		case 0xfd0e:
#if XM7_VER >= 2
			/* FM-7モードのみ有効 */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
		case 0xfd52:
			/* 有効フラグをチェック、無効なら何もしない */
			if ((!thg_enable) && (addr == 0xfd52)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd52)) {
				return FALSE;
			}
#endif

			opn_read_data_reg(OPN_THG, dat);
			return TRUE;

		/* 拡張割り込みステータス */
		case 0xfd53:
#if XM7_VER == 1
			if (fm_subtype == FMSUB_FM8) {
				return FALSE;
			}
#endif
			opn_read_interrupt_reg(OPN_THG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	THG
 *	１バイト書き込み
 */
BOOL FASTCALL thg_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
		/* THGコマンドレジスタ */
		case 0xfd0d:
#if XM7_VER >= 2
			/* FM-7モードのみ有効 */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
			/* PSGアクセス時は下位2bitのみ有効 */
			dat &= 0x03;
		case 0xfd51:
			/* 有効フラグをチェック、無効なら何もしない */
			if ((!thg_enable) && (addr == 0xfd51)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd51)) {
				return FALSE;
			}
#endif

			/* THGフラグON・PSGレジスタマスク処理 */
			if ((dat & 0x0f) == OPN_WRITEDAT) {
				if (addr == 0xfd51) {
					/* THG I/Oアクセス時はフラグオン */
					thg_use = TRUE;
				}
				else if (opn_selreg[OPN_THG] >= 0x10) {
					/* PSG I/Oアクセス時、レジスタが0x10以上なら何もしない */
					return TRUE;
				}
			}

			opn_write_command_reg(OPN_THG, dat);
			return TRUE;

		/* データレジスタ */
		case 0xfd0e:
#if XM7_VER >= 2
			/* FM-7モードのみ有効 */
			if (fm7_ver != 1) {
				return FALSE;
			}
#endif
		case 0xfd52:
			/* 有効フラグをチェック、無効なら何もしない */
			if ((!thg_enable) && (addr == 0xfd52)) {
				return FALSE;
			}
#if XM7_VER == 1
			if ((fm_subtype == FMSUB_FM8) && (addr == 0xfd52)) {
				return FALSE;
			}
#endif

			/* THGフラグON・PSGレジスタマスク処理 */
			if (opn_pstate[OPN_THG] == OPN_WRITEDAT) {
				if (addr == 0xfd51) {
					/* THG I/Oアクセス時はフラグオン */
					thg_use = TRUE;
				}
				else if (opn_selreg[OPN_THG] >= 0x10) {
					/* PSG I/Oアクセス時、レジスタが0x10以上なら何もしない */
					return TRUE;
				}
			}

			opn_write_data_reg(OPN_THG, dat);
			return TRUE;
	}

	return FALSE;
}

/*
 *	THG
 *	セーブ
 */
BOOL FASTCALL thg_save(SDL_RWops *fileh)
{
	if (!file_bool_write(fileh, thg_enable)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, thg_use)) {
		return FALSE;
	}
	if (!opn_save_common(OPN_THG, fileh)) {
		return FALSE;
	}

	return TRUE;
}

/*
 *	THG
 *	ロード
 */
BOOL FASTCALL thg_load(SDL_RWops *fileh, int ver)
{
	int i;

	/* ファイルバージョン6で追加 */
	if (ver < 200) {
		return FALSE;
	}
#if XM7_VER >= 2
	if (ver < 600) {
		thg_use = FALSE;
		return TRUE;
	}
#endif

	if (!file_bool_read(fileh, &thg_enable)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &thg_use)) {
		return FALSE;
	}
	if (!opn_load_common(OPN_THG, fileh)) {
		return FALSE;
	}

	/* THGレジスタ復旧 */
	thg_notify(0x27, (BYTE)(opn_reg[OPN_THG][0x27] & 0xc0));

	for (i=0; i<3; i++) {
		thg_notify(0x28, (BYTE)i);
		opn_key[OPN_THG][i] = FALSE;
	}

	for (i=0; i<13; i++) {
		if ((i < 8) || (i > 10)) {
			thg_notify((BYTE)i, opn_reg[OPN_THG][i]);
		}
		else {
			thg_notify((BYTE)i, 0);
		}
	}

	for (i=0x30; i<0xb4; i++) {
		thg_notify((BYTE)i, opn_reg[OPN_THG][i]);
	}

	/* イベント */
	schedule_handle(EVENT_THG_A, thg_timera_event);
	schedule_handle(EVENT_THG_B, thg_timerb_event);

	return TRUE;
}

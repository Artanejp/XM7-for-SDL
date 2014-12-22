/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2013 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2013 Ryu Takegami
 *
 *      [ サブCPUコントロール ]
 */

#include <string.h>
#include "xm7.h"
#include "keyboard.h"
#include "subctrl.h"
#include "device.h"
#include "display.h"
#include "aluline.h"
#include "ttlpalet.h"
#include "multipag.h"


/*
 *	グローバル ワーク
 */
BOOL subhalt_flag;							/* サブHALTフラグ */
BOOL subbusy_flag;							/* サブBUSYフラグ */
BOOL subcancel_flag;						/* サブキャンセルフラグ */
BOOL subattn_flag;							/* サブアテンションフラグ */
BOOL subhalt_request;						/* サブHALTリクエストフラグ */
BOOL subcancel_request;					/* サブキャンセルリクエストフラグ */
BYTE shared_ram[0x80];					/* 共有RAM */
BOOL subreset_flag;							/* サブ再起動フラグ */
BYTE busy_CLR_count;						/* BUSY($D40A) CLR命令実行時カウンタ */
#if XM7_VER >= 2
BOOL mode320;										/* 320x200モード */
#if XM7_VER >= 3
BOOL mode400l;									/* 640x400モード */
BOOL mode256k;									/* 26万色モード */
BOOL subram_protect;						/* サブモニタRAMプロテクト */
BOOL subreset_halt;							/* HALT中再起動フラグ */
BYTE subif_dat;									/* サブI/Fレジスタ 画面モード保存 */
BOOL subkanji_flag;							/* 漢字ROM サブCPU接続フラグ */
#endif
#endif
#if XM7_VER == 1 && defined(L4CARD)
BOOL select_400line;						/* メイン側400ライン選択フラグ */
BOOL subkanji_flag;							/* 漢字ROM サブCPU接続フラグ */
#endif


/*
 *	サブCPUコントロール
 *	初期化
 */
BOOL FASTCALL
subctrl_init(void)
{
	return TRUE;
}

/*
 *	サブCPUコントロール
 *	クリーンアップ
 */
void FASTCALL
subctrl_cleanup(void)
{
}

/*
 *	サブCPUコントロール
 *	リセット
 */
void FASTCALL
subctrl_reset(void)
{
	subhalt_request = FALSE;
	subhalt_flag = FALSE;
	subbusy_flag = TRUE;
	subcancel_request = FALSE;
	subcancel_flag = FALSE;
	subattn_flag = FALSE;
	subreset_flag = FALSE;
	busy_CLR_count = 0;
#if XM7_VER >= 2
	mode320 = FALSE;
#if XM7_VER >= 3
	mode400l = FALSE;
	mode256k = FALSE;
	screen_mode = SCR_200LINE;
	subram_protect = TRUE;
	subreset_halt = FALSE;
	subif_dat = 0x08;
	subkanji_flag = FALSE;
#endif
#endif

#if XM7_VER == 1 && defined(L4CARD)
	select_400line = FALSE;
	subkanji_flag = FALSE;
	enable_400line = FALSE;
#endif

	memset(shared_ram, 0xff, sizeof(shared_ram));
}

/*
 *	サブCPUコントロール
 *	HALT/CANCEL アクノリッジ
 */
void FASTCALL
subctrl_halt_ack(void)
{
	subhalt_flag = subhalt_request;
	subcancel_flag = subcancel_request;

	/* HALT時にはBUSY信号をONにする */
	if (subhalt_request) {
		subbusy_flag = TRUE;
	}
}

/*
 *	サブCPUコントロール
 *	１バイト読み出し
 */
BOOL FASTCALL
subctrl_readb(WORD addr, BYTE * dat)
{
	BYTE ret;

	switch (addr) {
			/* サブCPU アテンション割り込み、Breakキー割り込み */
		case 0xfd04:
			/* BUSYフラグ */
			if (subbusy_flag) {
				ret = 0xff;
			}
			else {
				ret = 0x7f;
			}
			/* アテンションフラグ */
			if (subattn_flag) {
				ret &= ~0x01;
				subattn_flag = FALSE;
			}
			/* Breakキーフラグ */
			if (break_flag || hotreset_flag) {
				ret &= ~0x02;

				/* ホットリセット処理 */
				if (hotreset_flag) {
#if XM7_VER >= 2
					/* イニシエータROMが無効ならホットリセット解除 */
					if (!initrom_en) {
						break_flag = FALSE;
						hotreset_flag = FALSE;
					}
#else
					break_flag = FALSE;
					hotreset_flag = FALSE;
#endif
				}
			}

#if XM7_VER == 1 && defined(L4CARD)
			/* 以下、FM-77モードのみ有効 */
			if (fm_subtype == FMSUB_FM77) {
				/* 漢字ROM切り換え */
				if (subkanji_flag) {
					ret &= ~0x20;
				}

				/* 400ラインカード */
				if (enable_400linecard) {
					ret &= ~0x10;
				}

				/* 400ラインカードモード */
				if (enable_400linecard && select_400line) {
					ret &= ~0x08;
				}
			}
#endif

			*dat = ret;
			maincpu_firq();
			return TRUE;

			/* サブインタフェース */
		case 0xfd05:
			if (subbusy_flag) {
				*dat = 0xfe;
				return TRUE;
			}
			else {
				*dat = 0x7e;
				return TRUE;
			}

			/* サブモードステータス */
#if XM7_VER >= 2
		case 0xfd12:
			ret = 0xff;
			if (fm7_ver >= 2) {
				/* 320/640 */
				if (!mode320) {
					ret &= ~0x40;
				}

				/* ブランクステータス */
				if (blank_flag) {
					ret &= ~0x02;
				}
				/* VSYNCステータス */
				if (!vsync_flag) {
					ret &= ~0x01;
				}
			}
			*dat = ret;
			return TRUE;

			/* サブバンク切り替え(Write Only) */
		case 0xfd13:
			*dat = 0xff;
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *	サブCPUコントロール
 *	１バイト書き込み
 */
BOOL FASTCALL
subctrl_writeb(WORD addr, BYTE dat)
{
	switch (addr) {
#if XM7_VER >= 3
			/* サブインタフェース AV40拡張部分 */
		case 0xfd04:
			if (fm7_ver >= 3) {
				/* bit2:サブモニタプロテクト */
				if (dat & 0x04) {
					subram_protect = FALSE;
				}
				else {
					subram_protect = TRUE;
				}
				/* bit3:400ラインモード選択 */
				if (dat & 0x08) {
					mode400l = FALSE;
				}
				else {
					mode400l = TRUE;
				}
				/* bit4:26万色モード選択 */
				/*      bit3と4の両方がONだった場合、mode400lを優先 */
				if ((dat & 0x10) && !mode400l) {
					mode256k = TRUE;
				}
				else {
					mode256k = FALSE;
				}
				/* bit5:漢字ROM接続切り換え */
				if (dat & 0x20) {
					subkanji_flag = FALSE;
				}
				else {
					subkanji_flag = TRUE;
				}

				/* 画面モードが切り替わった場合だけ再描画 */
				if ((BYTE) (dat & 0x18) != subif_dat) {
					display_setpointer(TRUE);
				}
				subif_dat = (BYTE) (dat & 0x18);
			}
			return TRUE;
#endif

#if XM7_VER == 1 && defined(L4CARD)
			/* サブインタフェース FM-77拡張部分 */
		case 0xfd04:
			/* 以下、FM-77モードのみ有効 */
			if (fm_subtype == FMSUB_FM77) {
				/* bit3:400ラインカードモード */
				if (dat & 0x08) {
					select_400line = FALSE;
				}
				else {
					select_400line = TRUE;
				}

				/* bit5:漢字ROM接続切り換え */
				if (dat & 0x20) {
					subkanji_flag = FALSE;
				}
				else {
					subkanji_flag = TRUE;
				}
			}
			return TRUE;
#endif

			/* サブコントロール */
		case 0xfd05:
			if (dat & 0x80) {
				/* サブHALTリクエスト */
				subhalt_request = TRUE;
			}
			else {
				/* サブRUNリクエスト */
				subhalt_request = FALSE;

				/* バンク切り替え後は、リセットする */
#if XM7_VER >= 3

				if (subreset_halt) {
					subcpu_reset();
					subreset_halt = FALSE;
				}
#endif
			}
			if (dat & 0x40) {
				/* キャンセルIRQ */
				subcancel_request = TRUE;
			}
			subcpu_irq();
			return TRUE;

			/* サブモード切り替え */
#if XM7_VER >= 2
		case 0xfd12:
			if (fm7_ver >= 2) {
				/* 画面モードが切り替わった場合だけ再描画 */
				if (dat & 0x40) {
					if (!mode320) {
						mode320 = TRUE;
						display_setpointer(TRUE);
					}
				}
				else {
					if (mode320) {
						mode320 = FALSE;
						display_setpointer(TRUE);
					}
				}
			}
			return TRUE;

			/* サブバンク切り替え */
		case 0xfd13:
			if (fm7_ver >= 2) {
				/* バンク切り替え */
#if XM7_VER >= 3
				if ((fm7_ver >= 3) && (dat & 0x04)) {
					subrom_bank = 4;
				}
				else {
					subrom_bank = (BYTE) (dat & 0x03);
				}
#else
				subrom_bank = (BYTE) (dat & 0x03);
#endif

				/* リセット */
#if XM7_VER >= 3
				if (!subhalt_flag) {
					subcpu_reset();
					subreset_halt = FALSE;
				}
				else {
					subreset_halt = TRUE;
				}
#else
				subcpu_reset();
#endif

				/* フラグ類セット */
				subreset_flag = TRUE;
				subbusy_flag = TRUE;

				/* 表示系I/Oをリセットする (FM77AV デモプログラム) */
				aluline_reset();
				ttlpalet_reset();
				multipag_reset();

				/* CRTレジスタをリセットする */
				display_reset();
				display_notify();

				/* INS LEDを消灯させる */
				ins_flag = FALSE;
			}
			return TRUE;
#endif
	}

	return FALSE;
}

/*
 *      サブCPUコントロール
 *      セーブ
 */
BOOL FASTCALL
subctrl_save(SDL_RWops * fileh)
{
	if (!file_bool_write(fileh, subhalt_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subbusy_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subcancel_flag)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subattn_flag)) {
		return FALSE;
	}

	if (!file_write(fileh, shared_ram, 0x80)) {
		return FALSE;
	}

	if (!file_bool_write(fileh, subreset_flag)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_bool_write(fileh, mode320)) {
		return FALSE;
	}
#endif

	/* Ver7拡張 */
	if (!file_bool_write(fileh, subhalt_request)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subcancel_request)) {
		return FALSE;
	}

	/* Ver711拡張 */
	if (!file_byte_write(fileh, busy_CLR_count)) {
		return FALSE;
	}

#if XM7_VER >= 3
	/* Ver8拡張 */
	if (!file_bool_write(fileh, mode400l)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, mode256k)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subram_protect)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subreset_halt)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subkanji_flag)) {
		return FALSE;
	}
#endif

#if XM7_VER == 1 && defined(L4CARD)
	/* XM7 V1.1 / FM-77L4拡張 */
	if (!file_bool_write(fileh, select_400line)) {
		return FALSE;
	}
	if (!file_bool_write(fileh, subkanji_flag)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*
 *      サブCPUコントロール
 *      ロード
 */
BOOL FASTCALL
subctrl_load(SDL_RWops * fileh, int ver)
{
	/* バージョンチェック */
	if (ver < 200) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subhalt_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subbusy_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subcancel_flag)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subattn_flag)) {
		return FALSE;
	}

	if (!file_read(fileh, shared_ram, 0x80)) {
		return FALSE;
	}

	if (!file_bool_read(fileh, &subreset_flag)) {
		return FALSE;
	}

#if XM7_VER >= 2
	if (!file_bool_read(fileh, &mode320)) {
		return FALSE;
	}
#endif

	/* Ver7拡張 */
#if XM7_VER >= 3
	if (((ver >= 700) && (ver <= 799)) || (ver >= 900)) {
#elif XM7_VER >= 2
	if (ver >= 700) {
#else
	if (ver >= 300) {
#endif
		if (!file_bool_read(fileh, &subhalt_request)) {
			return FALSE;
		}
		if (!file_bool_read(fileh, &subcancel_request)) {
			return FALSE;
		}
	}
	else {
		subhalt_request = subhalt_flag;
		subcancel_request = subcancel_flag;
	}

	/* Ver711拡張 */
#if XM7_VER >= 3
	if (((ver >= 711) && (ver <= 799)) || (ver >= 911)) {
#elif XM7_VER >= 2
	if (ver >= 711) {
#else
	if (ver >= 300) {
#endif
		if (!file_byte_read(fileh, &busy_CLR_count)) {
			return FALSE;
		}
	}
	else {
		busy_CLR_count = 0;
	}

#if XM7_VER >= 3
	/* Ver8拡張 */
	if (ver < 800) {
		mode400l = FALSE;
		mode256k = FALSE;
		subif_dat = 0x08;
		subram_protect = FALSE;
		subreset_halt = FALSE;

		/* ポインタを構成 */
		display_setpointer(TRUE);
		return TRUE;
	}

	if (!file_bool_read(fileh, &mode400l)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &mode256k)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &subram_protect)) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &subreset_halt)) {
		return FALSE;
	}

	/* Ver9.06拡張 */
	if (ver >= 906) {
		if (!file_bool_read(fileh, &subkanji_flag)) {
			return FALSE;
		}
	}
	else {
		if (mode400l) {
			subkanji_flag = TRUE;
		}
		else {
			subkanji_flag = FALSE;
		}
	}

	/* サブI/F Reg. 画面モード復帰 */
	subif_dat = 0x00;
	if (!mode400l) {
		subif_dat |= (BYTE) 0x08;
	}
	if (mode256k) {
		subif_dat |= (BYTE) 0x10;
	}
#endif

#if XM7_VER == 1 && defined(L4CARD)
	/* XM7 V1.1 / FM-77L4拡張 */
	if (!file_bool_read(fileh, &select_400line)) {
		return FALSE;
	}
	/* 400ラインカード整合性チェック */
	if (select_400line && !detect_400linecard) {
		return FALSE;
	}
	if (!file_bool_read(fileh, &subkanji_flag)) {
		return FALSE;
	}
#endif

	/* ポインタを構成 */
	display_setpointer(TRUE);

	return TRUE;
}

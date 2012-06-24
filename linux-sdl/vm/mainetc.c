/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ メインCPU各種I/O ]
 */

#include <string.h>
#include "xm7.h"
#include "mainetc.h"
#include "keyboard.h"
#include "opn.h"
#include "device.h"
#include "event.h"

/*
 *      グローバル ワーク
 */
BOOL            key_irq_flag;	/* キーボード割り込み 要求 */
BOOL            key_irq_mask;	/* キーボード割り込み マスク */
BOOL            lp_irq_flag;	/* プリンタ割り込み 要求 */
BOOL            lp_irq_mask;	/* プリンタ割り込み マスク */
BOOL            timer_irq_flag;	/* タイマー割り込み 要求 */
BOOL            timer_irq_mask;	/* タイマー割り込み マスク */

BOOL            mfd_irq_flag;	/* FDC割り込み フラグ */
BOOL            mfd_irq_mask;	/* FDC割り込み マスク */
BOOL            txrdy_irq_flag;	/* TxRDY割り込み フラグ */
BOOL            txrdy_irq_mask;	/* TxRDY割り込み マスク */
BOOL            rxrdy_irq_flag;	/* RxRDY割り込み フラグ */
BOOL            rxrdy_irq_mask;	/* RxRDY割り込み マスク */
BOOL            syndet_irq_flag;	/* SYNDET割り込み フラグ */
BOOL            syndet_irq_mask;	/* SYNDET割り込み マスク */

BOOL            opn_irq_flag;	/* OPN割り込み フラグ */
BOOL            whg_irq_flag;	/* WHG割り込み フラグ */
BOOL            thg_irq_flag;	/* THG割り込み フラグ */
#if XM7_VER >= 3
BOOL            dma_irq_flag;	/* DMA割り込み フラグ */
#endif
BOOL            beep_flag;	/* BEEPフラグ */
BOOL            speaker_flag;	/* スピーカフラグ */
#if XM7_VER == 1
BOOL banksel_en;					/* バンク切り換えイネーブルフラグ */
#endif


/*
 *      プロトタイプ宣言
 */
BOOL FASTCALL   mainetc_event(void);	/* 2.03ms タイマーイベント
					 */

/*
 *      メインCPU I/O
 *      初期化
 */
BOOL FASTCALL mainetc_init(void)
{
#if XM7_VER == 1
	/* FM-8モードの標準はバンク切り換え無効 */
	banksel_en = FALSE;
#endif
    return TRUE;
}

/*
 *      メインCPU I/O
 *      クリーンアップ
 */
void            FASTCALL
mainetc_cleanup(void)
{
}

/*
 *      メインCPU I/O
 *      リセット
 */
void            FASTCALL
mainetc_reset(void)
{
    /*
     * 割り込みフラグ初期化
     */
    key_irq_flag = FALSE;
    key_irq_mask = TRUE;
    lp_irq_flag = FALSE;
    lp_irq_mask = TRUE;
    timer_irq_flag = FALSE;
    timer_irq_mask = TRUE;

    mfd_irq_flag = FALSE;
    mfd_irq_mask = TRUE;
    txrdy_irq_flag = FALSE;
    txrdy_irq_mask = TRUE;
    rxrdy_irq_flag = FALSE;
    rxrdy_irq_mask = TRUE;
    syndet_irq_flag = FALSE;
    syndet_irq_mask = TRUE;

    opn_irq_flag = FALSE;
    whg_irq_flag = FALSE;
    thg_irq_flag = FALSE;
#if XM7_VER >= 3
    dma_irq_flag = FALSE;
#endif

    /*
     * BEEPフラグ初期化
     */
    beep_flag = FALSE;
    speaker_flag = FALSE;

#if XM7_VER == 1
    /*
     * イベントを追加(FM-8モードを除く)
     */
    if (fm_subtype != FMSUB_FM8) {
	schedule_setevent(EVENT_MAINTIMER, 2034, mainetc_event);
    } else {
	schedule_delevent(EVENT_MAINTIMER);
    }
#else
    /*
     * イベントを追加
     */
    schedule_setevent(EVENT_MAINTIMER, 2034, mainetc_event);
#endif

    /*
     * 通知
     */
    beep_notify();
}

/*
 *      BEEP終了イベント
 */
BOOL            FASTCALL
mainetc_beep(void)
{
    /*
     * BEEP停止
     */
    beep_flag = FALSE;

    /*
     * 自己イベント削除時は、TRUEにする
     */
    schedule_delevent(EVENT_BEEP);

    /*
     * 通知
     */
    beep_notify();

    return TRUE;
}

/*
 *      タイマー割り込みイベント
 */
BOOL            FASTCALL
mainetc_event(void)
{
    /*
     * 2.03msごとのCLKで、maskの反転をDFFで入力する
     */
    timer_irq_flag = !timer_irq_mask;
    maincpu_irq();

    /*
     * CLKは4.9152MHzなので、2.0345ms単位で発生する
     */
    if (event[EVENT_MAINTIMER].reload == 2034) {
	schedule_setevent(EVENT_MAINTIMER, 2035, mainetc_event);
    } else {
	schedule_setevent(EVENT_MAINTIMER, 2034, mainetc_event);
    }

    return TRUE;
}

/*
 *      FDC割り込み
 *      (fdc.cよりコマンド正常・異常終了時、フォースインタラプトで呼ばれる)
 */
void            FASTCALL
mainetc_fdc(void)
{
#if XM7_VER == 1
    /*
     * マスクされていれば、何もしない
     */
   if ((fm_subtype == FMSUB_FM8) || mfd_irq_mask) {
	return;
    }
#else
    /*
     * マスクされていれば、何もしない
     */
    if (mfd_irq_mask) {
	return;
    }
#endif

    /*
     * メインCPUでIRQ割り込みをかける
     */
    mfd_irq_flag = TRUE;

    /*
     * 処理
     */
    maincpu_irq();
}

/*
 *      LP割り込み
 *      (tapelp.cよりプリンタデータ出力時に呼ばれる)
 */
void            FASTCALL
mainetc_lp(void)
{
#if XM7_VER == 1
    /*
     * マスクされていれば、何もしない
     */
    if ((fm_subtype == FMSUB_FM8) || lp_irq_mask) {
	return;
    }
#else
    /*
     * マスクされていれば、何もしない
     */
    if (lp_irq_mask) {
	return;
    }
#endif

    /*
     * メインCPUでIRQ割り込みをかける
     */
    lp_irq_flag = TRUE;

    /*
     * 処理
     */
    maincpu_irq();
}

/*
 *      メインCPU I/O
 *      １バイト読み出し
 */
BOOL FASTCALL mainetc_readb(WORD addr, BYTE * dat)
{
    BYTE            ret;

    switch (addr) {
	/*
	 * キーボード 上位
	 */
    case 0xfd00:
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
	    ret = 0xff;
	    if (lowspeed_mode) {
		ret &= (BYTE) 0xfe;
	    }

	    *dat = ret;
	    return TRUE;
	}
#endif

	if (key_fm7 & 0x0100) {
	    ret = 0xff;
	} else {
	    ret = 0x7f;
	}
#if XM7_VER == 1
	if (lowspeed_mode) {
	    ret &= (BYTE) 0xfe;
	}
#endif
	*dat = ret;
	return TRUE;

	/*
	 * キーボード 下位
	 */
    case 0xfd01:
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
	    return FALSE;
	}
#endif

	*dat = (BYTE) (key_fm7 & 0xff);
	key_irq_flag = FALSE;
	maincpu_irq();
	subcpu_firq();
	return TRUE;

	/*
	 * IRQ要因識別
	 */
    case 0xfd03:
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
	    return FALSE;
	}
#endif

	ret = 0xff;
	if ((key_irq_flag) && !(key_irq_mask)) {
	    ret &= ~0x01;
	}
	if (lp_irq_flag) {
	    ret &= ~0x02;
	    lp_irq_flag = FALSE;
	}
	if (timer_irq_flag) {
	    ret &= ~0x04;
	    timer_irq_flag = FALSE;
	}
	if (mfd_irq_flag ||
	    txrdy_irq_flag || rxrdy_irq_flag || syndet_irq_flag ||
#if XM7_VER >= 3
	    dma_irq_flag ||
#endif
	    opn_irq_flag || whg_irq_flag || thg_irq_flag) {
	    ret &= ~0x08;
	}
	*dat = ret;
	maincpu_irq();
	return TRUE;

	/*
	 * BASIC ROM
	 */
    case 0xfd0f:
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) && !banksel_en) {
	    return FALSE;
	}
#endif

	basicrom_en = TRUE;
	*dat = 0xff;
#if XM7_VER == 1
			/* FM-8モード時はブートモードが切り替わる仕様 */
	if (fm_subtype == FMSUB_FM8) {
		boot_mode = BOOT_BASIC;
	}
#endif
	return TRUE;
    }

    return FALSE;
}

/*
 *      メインCPU I/O
 *      １バイト書き込み
 */
BOOL            FASTCALL
mainetc_writeb(WORD addr, BYTE dat)
{
    switch (addr) {
	/*
	 * 割り込みマスク
	 */
    case 0xfd02:
#if XM7_VER == 1
	if (fm_subtype == FMSUB_FM8) {
	    return FALSE;
	}
#endif

	if (dat & 0x80) {
	    syndet_irq_mask = FALSE;
	} else {
	    syndet_irq_mask = TRUE;
	}
	if (dat & 0x40) {
	    rxrdy_irq_mask = FALSE;
	} else {
	    rxrdy_irq_mask = TRUE;
	}
	if (dat & 0x20) {
	    txrdy_irq_mask = FALSE;
	} else {
	    txrdy_irq_mask = TRUE;
	}
	if (dat & 0x10) {
	    mfd_irq_mask = FALSE;
	} else {
	    mfd_irq_mask = TRUE;
	}
	if (dat & 0x04) {
	    timer_irq_mask = FALSE;
	} else {
	    timer_irq_mask = TRUE;
	}
	if (dat & 0x02) {
	    lp_irq_mask = FALSE;
	} else {
	    lp_irq_mask = TRUE;
	}
	if (dat & 0x01) {
	    key_irq_mask = FALSE;
	} else {
	    key_irq_mask = TRUE;
	}
	maincpu_irq();
	subcpu_firq();
	return TRUE;

	/*
	 * BEEP
	 */
    case 0xfd03:
	/*
	 * スピーカフラグの処理
	 */
	if (dat & 0x01) {
	    speaker_flag = TRUE;
	} else {
	    speaker_flag = FALSE;
	}
	if (dat & 0x40) {
	    /*
	     * 単発BEEP
	     */
	    beep_flag = TRUE;
	    schedule_setevent(EVENT_BEEP, 205000, mainetc_beep);

	    /*
	     * 通知
	     */
	    beep_notify();
	} else {
	    if (dat & 0x80) {
		/*
		 * 連続BEEP
		 */
		beep_flag = TRUE;
	    } else {
		/*
		 * BEEP OFF
		 */
		beep_flag = FALSE;
	    }

	    /*
	     * 通知
	     */
	    beep_notify();
	}
	return TRUE;

	/*
	 * BASIC ROM
	 */
    case 0xfd0f:
#if XM7_VER == 1
	if ((fm_subtype == FMSUB_FM8) && !banksel_en) {
	    return FALSE;
	}
#endif
	basicrom_en = FALSE;

#if XM7_VER == 1
	/* FM-8モード時はブートモードが切り替わる仕様 */
	if (fm_subtype == FMSUB_FM8) {
		boot_mode = BOOT_DOS;
	}
#endif
	return TRUE;
    }

    return FALSE;
}

/*
 *      メインCPU I/O
 *      セーブ
 */
BOOL            FASTCALL
mainetc_save(SDL_RWops *fileh)
{
    if (!file_bool_write(fileh, key_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, key_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, lp_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, timer_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, timer_irq_mask)) {
	return FALSE;
    }

    if (!file_bool_write(fileh, mfd_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, mfd_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, txrdy_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, txrdy_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, rxrdy_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, rxrdy_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, syndet_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, syndet_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, opn_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, whg_irq_flag)) {
	return FALSE;
    }

    /*
     * Ver6,8追加
     */
    if (!file_bool_write(fileh, thg_irq_flag)) {
	return FALSE;
    }
#if XM7_VER >= 3
    /*
     * Ver8追加
     */
    if (!file_bool_write(fileh, dma_irq_flag)) {
	return FALSE;
    }
#endif

    if (!file_bool_write(fileh, beep_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, speaker_flag)) {
	return FALSE;
    }
#if XM7_VER == 1
   if (!file_bool_write(fileh, banksel_en)) {
       return FALSE;
   }
#endif
    return TRUE;
}

/*
 *      メインCPU I/O
 *      ロード
 */
BOOL            FASTCALL
mainetc_load(SDL_RWops *fileh, int ver)
{
    /*
     * バージョンチェック
     */
    if (ver < 200) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &key_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &key_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &lp_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &timer_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &timer_irq_mask)) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &mfd_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &mfd_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &txrdy_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &txrdy_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &rxrdy_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &rxrdy_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &syndet_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &syndet_irq_mask)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &opn_irq_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &whg_irq_flag)) {
	return FALSE;
    }

    /*
     * Ver6追加
     */
#if XM7_VER >= 2
    if (ver >= 600) {
#else
    if (ver >= 300) {
#endif
	if (!file_bool_read(fileh, &thg_irq_flag)) {
	    return FALSE;
	}
    }
#if XM7_VER >= 3
    /*
     * Ver8追加
     */
    if (ver >= 800) {
	if (!file_bool_read(fileh, &dma_irq_flag)) {
	    return FALSE;
	}
    }
#endif

    if (!file_bool_read(fileh, &beep_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &speaker_flag)) {
	return FALSE;
    }
#if XM7_VER == 1
    if (ver >= 303) {
	if (!file_bool_read(fileh, &banksel_en)) {
		return FALSE;
	}
    }
#endif

    /*
     * イベント
     */
    schedule_handle(EVENT_MAINTIMER, mainetc_event);
    schedule_handle(EVENT_BEEP, mainetc_beep);

    /*
     * 通知
     */
    beep_notify();

    return TRUE;
}

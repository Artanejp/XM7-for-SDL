/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ インテリジェントマウス ]
 */


#include <assert.h>
#include <string.h>
#include "xm7.h"
#include "device.h"
#include "mainetc.h"
#include "event.h"
#include "mouse.h"
#ifdef MOUSE

/*
 *      グローバル ワーク
 */
BYTE            mos_port;	/* マウス接続ポート */
BOOL            mos_capture;	/* マウスキャプチャフラグ */

/*
 *      スタティック ワーク
 */
static BYTE     mos_x;		/* Ｘ移動距離 (左方向:+
				 * 右方向:-) */
static BYTE     mos_y;		/* Ｙ移動距離 (上方向:+
				 * 下方向:-) */
static BYTE     mos_phase;	/* フェーズカウンタ */
static BOOL     mos_strobe;	/* ストローブ信号状態(保存用) */


/*
 *      インテリジェントマウス
 *      初期化
 */
BOOL            FASTCALL
mos_init(void)
{
    /*
     * マウスキャプチャを停止する
     */
    mos_port = 1;
    mos_capture = FALSE;

    return TRUE;
}

/*
 *      インテリジェントマウス
 *      クリーンアップ
 */
void            FASTCALL
mos_cleanup(void)
{
}

/*
 *      インテリジェントマウス
 *      リセット
 */
void            FASTCALL
mos_reset(void)
{
    /*
     * ワークエリア初期化
     */
    mos_x = 0;
    mos_y = 0;
    mos_phase = 0;
    mos_strobe = FALSE;
}

/*
 *      インテリジェントマウス
 *      タイムアウト処理
 */
static BOOL     FASTCALL
mos_timeout(void)
{
    /*
     * タイムアウトイベントを削除
     */
    schedule_delevent(EVENT_MOUSE);

    /*
     * ストローブ信号・フェーズカウンタをリセット
     */
    mos_phase = 0;
    mos_strobe = FALSE;

    return TRUE;
}

/*
 *      インテリジェントマウス
 *      ストローブ信号処理
 */
void            FASTCALL
mos_strobe_signal(BOOL strb)
{
    /*
     * ストローブ信号の状態が変化したかチェック
     */
    if (strb != mos_strobe) {
	/*
	 * ストローブ信号の状態を保存
	 */
	mos_strobe = strb;

	if (mos_phase == 0) {
	    /*
	     * フェーズ0の時に移動距離を取り込む
	     */
	    mospos_request(&mos_x, &mos_y);

	    /*
	     * タイムアウトイベントの登録
	     */
	    schedule_setevent(EVENT_MOUSE, 2000, mos_timeout);
	}

	/*
	 * フェーズカウンタを更新
	 */
	mos_phase = (BYTE) ((mos_phase + 1) & 0x03);
    }
}

/*
 *      インテリジェントマウス
 *      データ読み込み
 */
BYTE            FASTCALL
mos_readdata(BYTE trigger)
{
    BYTE            ret;

    /*
     * フェーズカウンタに従ってデータを作成
     */
    switch (mos_phase) {
    case 1:			/* Ｘ上位ニブル */
	ret = (BYTE) ((mos_x >> 4) & 0x0f);
	break;
    case 2:			/* Ｘ下位ニブル */
	ret = (BYTE) (mos_x & 0x0f);
	break;
    case 3:			/* Ｙ上位ニブル */
	ret = (BYTE) ((mos_y >> 4) & 0x0f);
	break;
    case 0:			/* Ｙ下位ニブル */
	ret = (BYTE) (mos_y & 0x0f);
	break;
    }

    /*
     * ボタン押下状態データを合成
     */
    ret |= (BYTE) ((mosbtn_request() & (trigger << 4)) & 0x30);

    return ret;
}

/*
 *      インテリジェントマウス
 *      セーブ
 */
BOOL            FASTCALL
mos_save(SDL_RWops *fileh)
{
    if (!file_byte_write(fileh, mos_x)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, mos_y)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, mos_phase)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, mos_strobe)) {
	return FALSE;
    }

    return TRUE;
}

/*
 *      インテリジェントマウス
 *      ロード
 */
BOOL            FASTCALL
mos_load(SDL_RWops *fileh, int ver)
{
    /*
     * バージョンチェック
     */
    if (ver < 200) {
	return FALSE;
    }

    /*
     * いったんリセットする
     */
    mos_reset();

#if XM7_VER >= 3
    if ((ver >= 900) || ((ver >= 700) && (ver <= 799))) {
#elif XM7_VER >= 2
    if (ver >= 700) {
#else
    if ((ver >= 302) && (ver <= 399)) {
#endif
	if (!file_byte_read(fileh, &mos_x)) {
	    return FALSE;
	}
	if (!file_byte_read(fileh, &mos_y)) {
	    return FALSE;
	}
	if (!file_byte_read(fileh, &mos_phase)) {
	    return FALSE;
	}
	if (!file_bool_read(fileh, &mos_strobe)) {
	    return FALSE;
	}
    }

    /*
     * イベント
     */
    schedule_handle(EVENT_MOUSE, mos_timeout);

    return TRUE;
}

#endif				/* MOUSE */

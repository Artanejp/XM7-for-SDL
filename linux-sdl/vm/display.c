/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2011 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2011 Ryu Takegami
 *
 *      [ ディスプレイ ]
 */

#include <stdlib.h>
#include <string.h>
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "ttlpalet.h"
#include "multipag.h"
#include "mainetc.h"
#include "aluline.h"
#include "keyboard.h"
#include "kanji.h"
#include "event.h"
/*
 * XM7/SDL依存
 */
#ifdef USE_AGAR
#include "agar_xm7.h"
//#include "agar_gldraw.h"
#else
#include "sdl.h"
#endif
#include "api_draw.h"

extern void LockVram(void);
extern void UnLockVram(void);

/*
 *      グローバル ワーク
 */
BOOL            crt_flag;	/* CRT ONフラグ */
BOOL            vrama_flag;	/* VRAMアクセスフラグ */
WORD            vram_offset[2];	/* VRAMオフセットレジスタ */
WORD            crtc_offset[2];	/* CRTCオフセット */
BOOL            vram_offset_flag;	/* 拡張VRAMオフセットレジスタフラグ
					 */
BOOL            vsync_flag;	/* VSYNCフラグ */
BOOL            draw_aftervsync;	/* 画面描画通知タイミング
					 */

BOOL            blank_flag;	/* ブランキングフラグ */
#if XM7_VER >= 2
BOOL            subnmi_flag;	/* サブNMIイネーブルフラグ */

BYTE            vram_active;	/* アクティブページ */
BYTE           *vram_aptr;	/* VRAMアクティプポインタ */
BYTE            vram_display;	/* 表示ページ */
BYTE           *vram_dptr;	/* VRAM表示ポインタ */
#endif


#if XM7_VER >= 3
/*
 * FM77AV40
 */
BYTE            screen_mode;	/* 画面モード */
BYTE            subram_vrambank;	/* アクティブページ(400line/26万色)
					 */

WORD            sub_kanji_addr;	/* 漢字ROMアドレス */
BOOL            sub_kanji_bank;	/* 漢字ROM選択 */

/*
 * FM77AV40EX
 */
WORD            window_x1,
                window_dx1;	/* ウィンドウ X1 */
WORD            window_y1,
                window_dy1;	/* ウィンドウ Y1 */
WORD            window_x2,
                window_dx2;	/* ウィンドウ X2 */
WORD            window_y2,
                window_dy2;	/* ウィンドウ Y2 */
BOOL            window_open;	/* ウインドウオープンフラグ */

BYTE            block_active;	/* アクティブブロック */
BYTE           *vram_ablk;	/* アクティブブロックポインタ
				 */
BYTE            block_display;	/* 表示ブロック */
BYTE           *vram_bdptr;	/* 裏表示ブロックポインタ */
BYTE           *vram_dblk;	/* 表示ブロックポインタ2 */
BYTE           *vram_bdblk;	/* 裏表示ブロックポインタ2 */
#endif

#if XM7_VER == 1 && defined(L4CARD)
BOOL            width40_flag;	/* width flag */
BOOL            cursor_lsb;	/* カーソルアドレスLSB */
BOOL            enable_400line;	/* 400ラインモードフラグ */
BOOL            workram_select;	/* ワークRAMセレクトフラグ */
WORD            sub_kanji_addr;	/* 漢字ROMアドレス */

BYTE            crtc_register[0x20];	/* CRTCレジスタ */
BYTE            crtc_regnum;	/* CRTCレジスタ番号レジスタ */

WORD            text_start_addr;	/* テキストスタートアドレス
					 */
WORD            cursor_addr;	/* カーソルアドレス */
BOOL            text_blink;	/* テキストブリンク状態 */
BOOL            cursor_blink;	/* カーソルブリンク状態 */
#endif

/*
 *      スタティック ワーク
 */
static BYTE    *vram_buf;	/* VRAMスクロールバッファ */
static BYTE     vram_offset_count[2];	/* VRAMオフセット設定カウンタ
					 */
static WORD     blank_count;	/* ブランクカウンタ */
#if XM7_VER == 1 && defined(L4CARD)
static DWORD    text_scroll_count;	/* TVRAMオフセット設定カウンタ
					 */
static DWORD    text_cursor_count;	/* TVRAMカーソル位置設定カウンタ
					 */
static BYTE     text_blink_count;	/* テキストブリンクカウンタ
					 */
static BYTE     cursor_blink_count;	/* カーソルブリンクカウンタ
					 */
#endif

/*
 *      プロトタイプ宣言
 */
static BOOL FASTCALL subcpu_event(void);	/* サブCPUタイマイベント
						 */
static BOOL FASTCALL display_vsync(void);	/* VSYNCイベント */
static BOOL FASTCALL display_blank(void);	/* VBLANK,HBLANKイベント
						 */
static void FASTCALL display_setup(void);	/* イベントセットアップ
						 */
static void FASTCALL vram_scroll(WORD offset);	/* VRAMスクロール */
#if XM7_VER == 1 && defined(L4CARD)
static BOOL FASTCALL display_text_blink(void);	/* テキストブリンクイベント
						 */
static BOOL FASTCALL display_cursor_blink(void);	/* カーソルブリンクイベント
							 */
#endif

#if (XM7_VER >= 3) && (defined(_OMF) || defined(_WIN32))
extern void     memcpy400l(BYTE *, BYTE *, int);
										/*
										 * 400ラインスクロール用メモリ転送
										 */
#endif

/*
 *      24/32bit Color用輝度テーブル
 */
#if XM7_VER >= 3
const BYTE      truecolorbrightness[64] = {
    0, 4, 8, 12, 16, 20, 24, 28,
    32, 36, 40, 45, 49, 53, 57, 61,
    65, 69, 73, 77, 81, 85, 89, 93,
    97, 101, 105, 109, 113, 117, 121, 125,
    130, 134, 138, 142, 146, 150, 154, 158,
    162, 166, 170, 174, 178, 182, 186, 190,
    194, 198, 202, 206, 210, 215, 219, 223,
    227, 231, 235, 239, 243, 247, 251, 255,
};
#endif


/*
 *      ディスプレイ
 *      初期化
 */
BOOL            FASTCALL
display_init(void)
{
    /*
     * VRAMスクロール用バッファを確保
     */
#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
    vram_buf = (BYTE *) malloc(0x8000);
#else
    vram_buf = (BYTE *) malloc(0x4000);
#endif
    if (vram_buf == NULL) {
	return FALSE;
    }
#if XM7_VER == 1 && defined(L4CARD)
    /*
     * 初期化直後は200ラインモードに設定
     */
    enable_400line = FALSE;
#endif

    /*
     * 描画通知タイミングはVSYNC後
     */
    draw_aftervsync = TRUE;

    /*
     * CRTCオフセットワークをクリア
     */
    memset(crtc_offset, 0, sizeof(crtc_offset));

    return TRUE;
}

/*
 *      ディスプレイ
 *      クリーンアップ
 */
void            FASTCALL
display_cleanup(void)
{
    ASSERT(vram_buf);
    if (vram_buf) {
	free(vram_buf);
    }
}

/*
 *      ディスプレイ
 *      リセット
 */
void            FASTCALL
display_reset(void)
{
#if XM7_VER >= 2
    int             i;
#endif

    /*
     * リセット時のスクロール位置補正
     */
#if XM7_VER >= 2
    for (i = 0; i < 2; i++) {
	vram_scroll((WORD) - crtc_offset[i]);
    }
#else
    vram_scroll((WORD) - crtc_offset[0]);
#endif

    /*
     * CRTレジスタ
     */
    crt_flag = FALSE;
    vrama_flag = FALSE;
    memset(vram_offset, 0, sizeof(vram_offset));
    memset(crtc_offset, 0, sizeof(crtc_offset));
    vram_offset_flag = FALSE;
    memset(vram_offset_count, 0, sizeof(vram_offset_count));

#if XM7_VER >= 2
    /*
     * 割り込み、イベント
     */
    subnmi_flag = TRUE;
    vsync_flag = FALSE;
    blank_flag = TRUE;

    /*
     * アクティブページ、表示ページ初期化
     */
    vram_active = 0;
    vram_aptr = vram_c;
    vram_display = 0;
    vram_dptr = vram_c;
#if XM7_VER >= 3
    subram_vrambank = 0;

    /*
     * ハードウェアウィンドウ初期化
     */
    window_x1 = window_x2 = 0;
    window_y1 = window_y2 = 0;
    window_dx1 = window_dx2 = 0;
    window_dy1 = window_dy2 = 0;
    window_open = FALSE;

    /*
     * アクティブブロック、表示ブロック初期化
     */
    block_active = 0;
    block_display = 0;
    vram_ablk = vram_c;
    vram_dblk = vram_c;
    vram_bdptr = vram_c + 0x18000;
    vram_bdblk = vram_bdptr;

    /*
     * サブ側漢字ROM初期化
     */
    sub_kanji_addr = 0;
    sub_kanji_bank = FALSE;
#endif				/* XM7_VER >= 3 */
#endif				/* XM7_VER >= 2 */

#if XM7_VER == 1 && defined(L4CARD)
    /*
     * 割り込み、イベント
     */
    vsync_flag = FALSE;
    blank_flag = TRUE;

    /*
     * TextVRAM
     */
    text_cursor_count = 0;
    text_scroll_count = 0;
    text_start_addr = 0x0000;
    text_blink = TRUE;
    cursor_addr = 0x0000;
    cursor_blink = TRUE;
    text_blink_count = 0;
    cursor_blink_count = 0;
    workram_select = FALSE;

    /*
     * CRTC
     */
    crtc_regnum = 0;
    memset(crtc_register, 0, sizeof(crtc_register));

    /*
     * サブ側漢字ROM初期化
     */
    sub_kanji_addr = 0;

    /*
     * 400ラインモード時はテキストブリンクイベントを追加
     */
    if (enable_400line && enable_400linecard) {
	schedule_setevent(EVENT_TEXT_BLINK, 160 * 1000,
			  display_text_blink);
    } else {
	schedule_delevent(EVENT_TEXT_BLINK);
    }
#endif

    /*
     * 20msごとに起こすイベントを追加
     */
    schedule_setevent(EVENT_SUBTIMER, 20000, subcpu_event);

    /*
     * VSYNC, VBLANK, HBLANKのセットアップ
     */
    display_setup();
}

#if XM7_VER == 1 && defined(L4CARD)
/*
 *      テキストブリンクイベント
 */
static BOOL     FASTCALL
display_text_blink(void)
{
    WORD            addr;
    WORD            ofsaddr;

    /*
     * 400ラインモードでなければイベントを削除して帰る
     */
    if (!(enable_400line && enable_400linecard)) {
	schedule_delevent(EVENT_TEXT_BLINK);
	return TRUE;
    }

    /*
     * テキストブリンク (320ms)
     */
    if (text_blink_count++ >= 2) {
	/*
	 * ブリンク状態を反転
	 */
	text_blink = (!text_blink);

	/*
	 * ブリンクキャラクタを検索
	 */
	for (addr = 0; addr < 4000; addr += (WORD) 2) {
	    ofsaddr = (WORD) ((addr + text_start_addr) & 0xFFE);
	    if (tvram_c[ofsaddr + 1] & 0x10) {
		tvram_notify(ofsaddr, 0);
	    }
	}

	/*
	 * カウンタをリセット
	 */
	text_blink_count = 0;
    }

    /*
     * カーソルブリンク (320/160ms)
     */
    if (crtc_register[10] & 0x40) {
	cursor_blink_count++;
	if ((cursor_blink_count >= 2) || !(crtc_register[10] & 0x20)) {
	    /*
	     * ブリンク状態を反転
	     */
	    cursor_blink = (!cursor_blink);

	    /*
	     * カーソル位置を再表示
	     */
	    tvram_notify(cursor_addr, 0);

	    /*
	     * カウンタをリセット
	     */
	    cursor_blink_count = 0;
	}
    }

    return TRUE;
}
#endif

/*
 *      VSYNCイベント
 */
static BOOL     FASTCALL
display_vsync(void)
{
    if (!vsync_flag) {
	if ((blank_count & 0xfff) > 0) {
	    /*
	     * 一巡した
	     */
	    display_setup();
	    return TRUE;
	}

	/*
	 * これから垂直同期
	 */
	vsync_flag = TRUE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
	if (blank_count & 0x1000) {
	    /*
	     * 400ライン(24kHzモード) 0.33ms
	     */
	    schedule_setevent(EVENT_VSYNC, 330, display_vsync);
	} else {
	    /*
	     * 200ライン(15kHzモード) 0.51ms
	     */
	    schedule_setevent(EVENT_VSYNC, 510, display_vsync);
	}
#else
	/*
	 * 200ライン(15kHzモード) 0.51ms
	 */
	schedule_setevent(EVENT_VSYNC, 510, display_vsync);
#endif

	/*
	 * ビデオディジタイズ
	 */
#if XM7_VER >= 2
	if (digitize_enable) {
	    if (digitize_keywait || simpose_mode == 0x03) {
		digitize_notify();
	    }
	}
#else
	blank_count++;
#endif
    } else {
	/*
	 * これから垂直表示
	 */
	vsync_flag = FALSE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
	if (blank_count & 0x1000) {
	    /*
	     * 400ライン(24kHzモード) 0.98ms + 16.4ms
	     */
	    schedule_setevent(EVENT_VSYNC, 980 + 16400, display_vsync);
	} else {
	    /*
	     * 200ライン(15kHzモード) 1.91ms + 12.7ms
	     */
	    schedule_setevent(EVENT_VSYNC, 1910 + 12700, display_vsync);
	}
#else
	/*
	 * 200ライン(15kHzモード) 1.91ms + 12.7ms
	 */
	schedule_setevent(EVENT_VSYNC, 1910 + 12700, display_vsync);
#endif

	/*
	 * VSYNC通知
	 */
	if (draw_aftervsync) {
	    vsync_notify();
	}
    }

    return TRUE;
}

/*
 *      VBLANK,HBLANKイベント
 */
static BOOL     FASTCALL
display_blank(void)
{
    if ((blank_count & 0xfff) == 0) {
	if (blank_flag) {
	    /*
	     * これから初回
	     */
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
	    if (blank_count & 0x1000) {
		/*
		 * 400ライン。11us
		 */
		schedule_setevent(EVENT_BLANK, 11, display_blank);
	    } else {
		/*
		 * 200ライン。24us
		 */
		schedule_setevent(EVENT_BLANK, 24, display_blank);
	    }
#else
	    /*
	     * 200ライン。24us
	     */
	    schedule_setevent(EVENT_BLANK, 24, display_blank);
#endif
	    blank_count++;
	    return TRUE;
	}
    }

    if (blank_flag) {
	/*
	 * これから水平表示期間
	 */
	blank_flag = FALSE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
	if (blank_count & 0x1000) {
	    /*
	     * 400ライン。30us
	     */
	    schedule_setevent(EVENT_BLANK, 30, display_blank);
	} else {
	    /*
	     * 200ライン。39usまたは40us
	     */
	    schedule_setevent(EVENT_BLANK, 39 + (blank_count & 1),
			      display_blank);
	}
#else
	/*
	 * 200ライン。39usまたは40us
	 */
	schedule_setevent(EVENT_BLANK, 39 + (blank_count & 1),
			  display_blank);
#endif
	blank_count++;
	return TRUE;
    } else {
	/*
	 * これから水平同期期間
	 */
	blank_flag = TRUE;
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
	if (blank_count & 0x1000) {
	    /*
	     * 400ライン。11us
	     */
	    schedule_setevent(EVENT_BLANK, 11, display_blank);
	} else {
	    /*
	     * 200ライン。24us
	     */
	    schedule_setevent(EVENT_BLANK, 24, display_blank);
	}
#else
	/*
	 * 200ライン。24us
	 */
	schedule_setevent(EVENT_BLANK, 24, display_blank);
#endif

	return TRUE;
    }
}

/*
 *      イベントセットアップ
 */
static void     FASTCALL
display_setup(void)
{
#if (XM7_VER >= 3) || (XM7_VER == 1 && defined(L4CARD))
    /*
     * 200ライン,400ラインの判別
     */
#if XM7_VER >= 3
    if (screen_mode == SCR_400LINE) {
#else
    if (enable_400line && enable_400linecard) {
#endif
	blank_count = 0x1000;
	schedule_setevent(EVENT_VSYNC, 340, display_vsync);
	schedule_setevent(EVENT_BLANK, 1650, display_blank);
    } else {
	blank_count = 0;
	schedule_setevent(EVENT_VSYNC, 1520, display_vsync);
	schedule_setevent(EVENT_BLANK, 3940, display_blank);
    }
#else
    blank_count = 0;
    schedule_setevent(EVENT_VSYNC, 1520, display_vsync);
    schedule_setevent(EVENT_BLANK, 3940, display_blank);

    /*
     * 垂直ブランキング期間
     */
    vsync_flag = FALSE;
    blank_flag = TRUE;
#endif

    /*
     * VSYNC通知
     */
    if (!draw_aftervsync) {
	vsync_notify();
    }
}

/*
 *      サブCPU
 *      イベント処理
 */
static BOOL     FASTCALL
subcpu_event(void)
{
#if XM7_VER >= 2
    /*
     * 念のため、チェック
     */
    if (!subnmi_flag && (fm7_ver >= 2)) {
	return FALSE;
    }
#endif

    /*
     * NMI割り込みを起こす
     */
    subcpu_nmi();
    return TRUE;
}

/*
 *      ポインタ再設定
 */
void            FASTCALL
display_setpointer(BOOL redraw)
{
#if XM7_VER >= 3
    /*
     * 画面モード番号
     */
    if (mode400l) {
	screen_mode = SCR_400LINE;
    } else if (mode256k) {
	screen_mode = SCR_262144;
    } else if (mode320) {
	screen_mode = SCR_4096;
    } else {
	screen_mode = SCR_200LINE;
    }
#endif
#ifdef USE_AGAR
    LockVram();
#endif
#if XM7_VER >= 2
    /*
     * アクティブポインタ・アクティブブロックポインタ
     */
    vram_aptr = vram_c;
#if XM7_VER >= 3
    vram_ablk = vram_c;
    switch (screen_mode) {
    case SCR_400LINE:		/* 640x400 8色 */
	vram_aptr += (subram_vrambank * 0x8000);
	if (block_active) {
	    vram_aptr += 0x18000;
	    vram_ablk += 0x18000;
	}
	break;
    case SCR_262144:		/* 320x200 262144色 */
	switch (subram_vrambank) {
	case 0:
	    break;
	case 1:
	    vram_aptr += 0x4000;
	    break;
	case 2:
	    vram_aptr += 0x18000;
	    break;
	}
	break;
    default:			/* 640x200か、320x200 4096色 */
	if (vram_active) {
	    vram_aptr += 0x4000;
	}
	if (block_active) {
	    vram_aptr += 0x18000;
	}
    }
#else
    if (vram_active) {
	vram_aptr += 0xc000;
    }
#endif

    /*
     * 表示ポインタ、裏表示ブロックポインタ
     */
    vram_dptr = vram_c;
#if XM7_VER >= 3
    vram_dblk = vram_c;
    vram_bdblk = vram_c;
    if ((screen_mode == SCR_200LINE) && vram_display) {
	vram_dptr += 0x4000;
    }
    vram_bdptr = vram_dptr;
    if (block_display) {
	vram_dptr += 0x18000;
	vram_dblk += 0x18000;
    } else {
	vram_bdptr += 0x18000;
	vram_bdblk += 0x18000;
    }
#else
    if (!mode320 && vram_display) {
	vram_dptr += 0xc000;
    }
#endif				/* XM7_VER >= 3 */
#endif				/* XM7_VER >= 2 */

    /*
     * 必要なら再描画
     */
    if (redraw) {
	display_notify();
    }
#ifdef USE_AGAR
    UnlockVram();
#endif

}

/*
 *      ハードウェアウインドウ
 *      オープンチェック・書き換え範囲決定
 */
#if XM7_VER >= 3
static void     FASTCALL
check_window_open(void)
{
    /*
     * Xs<Xe and Ys<Ye であれば、ウィンドウオープン
     */
    if ((window_x1 < window_x2) && (window_y1 < window_y2)) {
	window_open = TRUE;
    } else {
	window_open = FALSE;
	window_dx1 = 0;
	window_dx2 = 0;
	window_dy1 = 0;
	window_dy2 = 0;
    }

    /*
     * 再描画指示
     */
    window_notify();
}

/*
 *      ハードウェアウインドウ
 *      クリッピング
 */
void            FASTCALL
window_clip(int mode)
{
    static const WORD max_x[4] = { 640, 320, 640, 320 };
    static const WORD max_y[4] = { 200, 200, 400, 200 };

    ASSERT((mode >= 0) && (mode <= 2));

    /*
     * Xクリッピング
     */
    window_dx1 = window_x1;
    window_dx2 = window_x2;
    if (window_dx1 > max_x[mode]) {
	window_dx1 = max_x[mode];
    }
    if (window_dx2 > max_x[mode]) {
	window_dx2 = max_x[mode];
    }

    /*
     * Yクリッピング
     */
    window_dy1 = window_y1;
    window_dy2 = window_y2;
    if (window_dy1 > max_y[mode]) {
	window_dy1 = max_y[mode];
    }
    if (window_dy2 > max_y[mode]) {
	window_dy2 = max_y[mode];
    }
}
#endif

/*
 *      4096色/262144色モード用 VRAMスクロール
 */
#if XM7_VER >= 2
static void     FASTCALL
vram_scroll_analog(WORD offset, DWORD addr)
{
    int             i;
    BYTE           *vram;

#if XM7_VER >= 3
#ifdef USE_AGAR
    LockVram();
#endif

    for (i = 0; i < 3; i++) {
	vram = (BYTE *) ((vram_c + addr) + 0x8000 * i);

	/*
	 * テンポラリバッファへコピー
	 */
	memcpy(vram_buf, vram, offset);
	memcpy(&vram_buf[0x2000], &vram[0x2000], offset);

	/*
	 * 前へ詰める
	 */
	memcpy(vram, (vram + offset), 0x4000 - offset);

	/*
	 * テンポラリバッファより復元
	 */
	memcpy(vram + (0x2000 - offset), vram_buf, offset);
	memcpy(vram + (0x4000 - offset), &vram_buf[0x2000], offset);
    }
#else
    for (i = 0; i < 6; i++) {
	vram = (BYTE *) ((vram_c + addr) + 0x2000 * i);

	/*
	 * テンポラリバッファへコピー
	 */
	memcpy(vram_buf, vram, offset);

	/*
	 * 前へ詰める
	 */
	memcpy(vram, (vram + offset), 0x2000 - offset);

	/*
	 * テンポラリバッファより復元
	 */
	memcpy(vram + (0x2000 - offset), vram_buf, offset);
    }
#endif
#ifdef USE_AGAR
    UnlockVram();
#endif

}
#endif

/*
 *      400ラインモードVRAMスクロール用メモリ転送 (C版)
 */
#if (XM7_VER >= 3) && (!(defined(_OMF) || defined(_WIN32)))
static void     FASTCALL
memcpy400l(BYTE * dest, BYTE * src, WORD siz)
{
    siz >>= 1;
    while (siz) {
	*dest = *src;
	src += 2;
	dest += 2;
	siz--;
    }
}
#endif

/*
 *      VRAMスクロール
 */
static void     FASTCALL
vram_scroll(WORD offset)
{
    int             i;
    BYTE           *vram;

    if (offset == 0) {
	return;
    }
#if XM7_VER >= 2
#if XM7_VER >= 3
    /*
     * 400ライン
     */
    if (screen_mode == SCR_400LINE) {
	/*
	 * 400ライン時 オフセットマスク
	 */
	offset &= 0x3fff;
	offset <<= 1;
#ifdef USE_AGAR
    LockVram();
#endif

	/*
	 * ループ
	 */
	for (i = 0; i < 3; i++) {
	    vram = (BYTE *) (vram_c + 0x8000 * i + vram_active);

	    /*
	     * テンポラリバッファへコピー
	     */
	    memcpy400l(vram_buf, vram, offset);

	    /*
	     * 前へ詰める
	     */
	    memcpy400l(vram, (vram + offset), 0x8000 - offset);

	    /*
	     * テンポラリバッファより復元
	     */
	    memcpy400l(vram + (0x8000 - offset), vram_buf, offset);
	}
#ifdef USE_AGAR
    UnlockVram();
#endif

	return;
    }

    /*
     * 4096色/262144色
     */
    if (screen_mode & SCR_ANALOG) {
#else
    /*
     * 4096色
     */
    if (mode320) {
#endif
	/*
	 * 320時 オフセットマスク
	 */
	offset &= 0x1fff;

	if (vram_active == 1) {
	    /*
	     * バンク1
	     */
#if XM7_VER >= 3
	    vram_scroll_analog(offset, 0x04000);
#else
	    vram_scroll_analog(offset, 0x0c000);
#endif
	} else {
	    /*
	     * バンク0
	     */
	    vram_scroll_analog(offset, 0x00000);
#if XM7_VER >= 3
	    if (screen_mode == SCR_262144) {
		/*
		 * 26万色モード時はバンク2も同時にスクロール
		 */
		vram_scroll_analog(offset, 0x18000);
	    }
#endif
	}

	return;
    }
#endif				/* XM7_VER >= 2 */

#if XM7_VER == 1 && defined(L4CARD)
    if (enable_400line && enable_400linecard) {
	/*
	 * 400ライン単色時はレンダラ側でスクロールを行う
	 */
	/*
	 * (メイン$FD37の挙動再現の関係)
	 */
	return;
    }
#endif

    /*
     * 8色
     */
    offset &= 0x3fff;

    /*
     * ループ
     */
    for (i = 0; i < 3; i++) {
#if XM7_VER >= 3
	vram = (BYTE *) (vram_c + 0x8000 * i);
	if (vram_active) {
	    vram += 0x4000;
	}
#elif XM7_VER >= 2
	vram = (BYTE *) (vram_c + 0x4000 * i);
	if (vram_active) {
	    vram += 0xc000;
	}
#else
	vram = (BYTE *) (vram_c + 0x4000 * i);
#endif
#ifdef USE_AGAR
    LockVram();
#endif

	/*
	 * テンポラリバッファへコピー
	 */
	memcpy(vram_buf, vram, offset);

	/*
	 * 前へ詰める
	 */
	memcpy(vram, (vram + offset), 0x4000 - offset);

	/*
	 * テンポラリバッファより復元
	 */
	memcpy(vram + (0x4000 - offset), vram_buf, offset);
#ifdef USE_AGAR
    UnlockVram();
#endif
    }
}

/*
 *      400ラインモード用VRAM配置補正
 */
#if XM7_VER >= 3
BOOL            FASTCALL
fix_vram_address(void)
{
    DWORD           i;
#ifdef USE_AGAR
    LockVram();
#endif

    for (i = 0; i < 0x30000; i += 0x18000) {
	memcpy(&vram_buf[0x00000], &vram_c[0x04000 + i], 0x4000);
	memcpy(&vram_buf[0x04000], &vram_c[0x10000 + i], 0x4000);
	memcpy(&vram_c[0x04000 + i], &vram_c[0x08000 + i], 0x4000);
	memcpy(&vram_c[0x10000 + i], &vram_c[0x0c000 + i], 0x4000);
	memcpy(&vram_c[0x08000 + i], &vram_buf[0x04000], 0x4000);
	memcpy(&vram_c[0x0c000 + i], &vram_buf[0x00000], 0x4000);
    }
#ifdef USE_AGAR
    UnlockVram();
#endif

    return TRUE;
}
#endif

#if XM7_VER == 1 && defined(L4CARD)
/*
 *      カーソル 位置・ブリンク速度再設定
 */
void            FASTCALL
cursor_setup(void)
{
    WORD            cursor_addr_old;

    /*
     * カーソル移動
     */
    cursor_addr_old = cursor_addr;
    cursor_addr = (WORD) (crtc_register[14] << 10);
    cursor_addr |= (WORD) (crtc_register[15] << 2);
    if (cursor_lsb) {
	cursor_addr += (WORD) 2;
    }
    cursor_addr &= (WORD) 0xFFF;

    /*
     * 旧カーソル座標と新カーソル座標をそれぞれ再描画
     */
    tvram_notify(cursor_addr_old, 0);
    tvram_notify(cursor_addr, 0);
}

/*
 *      CRTC(HD6845) １バイト読み込み
 */
static BYTE     FASTCALL
crtc_readb(BYTE reg)
{
    if ((reg >= 0x0c) && (reg <= 0x11)) {
	/*
	 * 0x0c〜0x11は記憶している値を返す
	 */
	return crtc_register[reg];
    } else {
	/*
	 * 0x0c〜0x11以外はライトオンリー
	 */
	return 0xff;
    }
}

/*
 *      CRTC(HD6845) １バイト書き込み
 */
static void     FASTCALL
crtc_writeb(BYTE reg, BYTE dat)
{
    /*
     * 書き込んだ値を保存
     */
    crtc_register[reg] = dat;

    switch (reg) {
	/*
	 * カーソルサイズ
	 */
    case 0x0a:
    case 0x0b:
	cursor_setup();
	break;

	/*
	 * テキストスタートアドレス(上位)
	 */
    case 0x0c:
	text_start_addr &= 0x03FC;
	text_start_addr |= (WORD) ((dat & 0x03) << 10);
	text_scroll_count++;
	if ((text_scroll_count & 1) == 0) {
	    tvram_redraw_notify();
	}
	break;

	/*
	 * テキストスタートアドレス(下位)
	 */
    case 0x0d:
	text_start_addr &= 0xFC00;
	text_start_addr |= (WORD) (dat << 2);
	text_scroll_count++;
	if ((text_scroll_count & 1) == 0) {
	    tvram_redraw_notify();
	}
	break;

	/*
	 * カーソルアドレス
	 */
    case 0x0e:
    case 0x0f:
	text_cursor_count++;
	if ((text_cursor_count & 1) == 0) {
	    cursor_setup();
	}
	break;
    }
}
#endif

/*
 *      ディスプレイ
 *      １バイト読み込み
 *      ※メイン−サブインタフェース信号線を含む
 */
BOOL            FASTCALL
display_readb(WORD addr, BYTE * dat)
{
    BYTE            ret;
#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
    int             offset;
#endif				/* XM7_VER >= 3 */

    switch (addr) {
	/*
	 * キャンセルIRQ ACK
	 */
    case 0xd402:
	subcancel_flag = FALSE;
	subcancel_request = FALSE;
	subcpu_irq();
	*dat = 0xff;
	return TRUE;

	/*
	 * BEEP
	 */
    case 0xd403:
	beep_flag = TRUE;
	schedule_setevent(EVENT_BEEP, 205000, mainetc_beep);

	/*
	 * 通知
	 */
	beep_notify();
	return TRUE;

	/*
	 * アテンションIRQ ON
	 */
    case 0xd404:
	subattn_flag = TRUE;
	*dat = 0xff;
	maincpu_firq();
	return TRUE;

#if XM7_VER == 1 && defined(L4CARD)
	/*
	 * サブモードレジスタ
	 */
    case 0xd405:
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	ret = 0xff;

	/*
	 * bit7〜1は400ライン時のみ有効
	 */
	if (enable_400linecard) {
	    /*
	     * ブランキング
	     */
	    if (blank_flag) {
		ret &= (BYTE) ~ 0x80;
	    }

	    /*
	     * VSYNC
	     */
	    if (!vsync_flag) {
		ret &= (BYTE) ~ 0x40;
	    }

	    /*
	     * ライン数
	     */
	    if (select_400line) {
		ret &= ~0x20;
	    }

	    /*
	     * CURSOR LSB
	     */
	    if (!cursor_lsb) {
		ret &= (BYTE) ~ 0x10;
	    }

	    /*
	     * WIDTH
	     */
	    if (width40_flag) {
		ret &= (BYTE) ~ 0x08;
	    }

	    /*
	     * ワークRAM選択
	     */
	    if (!workram_select) {
		ret &= (BYTE) ~ 0x04;
	    }

	    /*
	     * 400ラインモード
	     */
	    if (enable_400line) {
		ret &= ~0x02;
	    }
	}

	/*
	 * サイクルスチール
	 */
	/*
	 * 設定変更には対応していないが、とりあえず値は返す
	 */
	if (cycle_steal) {
	    ret &= ~0x01;
	}

	*dat = ret;
	return TRUE;
#endif

#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
	/*
	 * サブ漢字ROM
	 */
    case 0xd406:		/* サブ漢字LEFT */
    case 0xd407:		/* サブ漢字RIGHT */
#if XM7_VER >= 3
	if ((fm7_ver >= 3) && subkanji_flag) {
	    /*
	     * アドレスはワード単位で、8bitのみ取得
	     */
	    offset = sub_kanji_addr << 1;
	    if (sub_kanji_bank) {
		*dat = kanji_rom2[offset + (addr & 1)];
	    } else {
		*dat = kanji_rom[offset + (addr & 1)];
	    }
	}
#else
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	if (subkanji_flag) {
	    /*
	     * アドレスはワード単位で、8bitのみ取得
	     */
	    offset = sub_kanji_addr << 1;
	   if ((offset >= 0x6000) && (offset < 0x8000)) {
	      /* FM-7モード時の$6000〜$7FFFは未定義領域 */
	      *dat = (BYTE)(addr & 1);
	   }
	   else {
	      /* 通常領域 */
	      *dat = kanji_rom[offset + (addr & 1)];
	   }
	}
#endif
	else {
	    *dat = 0xff;
	}
	return TRUE;
#endif

	/*
	 * CRT ON
	 */
    case 0xd408:
	if (!crt_flag) {
	    crt_flag = TRUE;
	    /*
	     * CRT OFF→ON
	     */
	    display_notify();
	}
	*dat = 0xff;
	return TRUE;

	/*
	 * VRAMアクセス ON
	 */
    case 0xd409:
	vrama_flag = TRUE;
	*dat = 0xff;
	return TRUE;

	/*
	 * BUSYフラグ OFF
	 */
    case 0xd40a:
	subbusy_flag = FALSE;
	*dat = 0xff;
	return TRUE;

#if XM7_VER == 1 && defined(L4CARD)
	/*
	 * CRTCアドレスレジスタ(400LINE CARD)
	 */
    case 0xd40b:
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	if (enable_400linecard) {
	    *dat = (BYTE) (crtc_regnum & 0x1f);
	} else {
	    *dat = 0xff;
	}
	return TRUE;

	/*
	 * CRTCデータレジスタ(400LINE CARD)
	 */
    case 0xd40c:
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	if (enable_400linecard) {
	    *dat = (BYTE) crtc_readb(crtc_regnum);
	} else {
	    *dat = 0xff;
	}
	return TRUE;
#endif

#if XM7_VER >= 3
	/*
	 * FM77AV40
	 * サブRAMバンクセレクト/サブ漢字ROMセレクト
	 */
    case 0xd42e:
	/*
	 * fm7_verに関わらず、読み出し出来ない
	 */
	*dat = 0xff;
	return TRUE;

	/*
	 * FM77AV40 400ライン/26万色用VRAMバンクセレクト
	 */
    case 0xd42f:
	if (fm7_ver >= 3) {
	    *dat = (BYTE) (0xfc | (subram_vrambank & 3));
	} else {
	    *dat = 0xff;
	}
	return TRUE;
#endif

#if XM7_VER >= 2
	/*
	 * FM77AV MISCレジスタ
	 */
    case 0xd430:
	if (fm7_ver >= 2) {
	    ret = 0xff;

	    /*
	     * ブランキング
	     */
	    if (blank_flag) {
		ret &= (BYTE) ~ 0x80;
	    }

	    /*
	     * 直線補間
	     */
	    if (line_busy) {
		ret &= (BYTE) ~ 0x10;
		/*
		 * LINE BOOST時は一回だけBUSYを見せる
		 */
		if (line_boost) {
		    line_busy = FALSE;
		    schedule_delevent(EVENT_LINE);
		}
	    }

	    /*
	     * VSYNC
	     */
	    if (!vsync_flag) {
		ret &= (BYTE) ~ 0x04;
	    }

	    /*
	     * サブRESETステータス
	     */
	    if (!subreset_flag) {
		ret &= (BYTE) ~ 0x01;
	    }

	    *dat = ret;
	    return TRUE;
	}

	return FALSE;
#endif
    }

    return FALSE;
}

/*
 *      ディスプレイ
 *      １バイト書き込み
 *      ※メイン−サブインタフェース信号線を含む
 */
BOOL            FASTCALL
display_writeb(WORD addr, BYTE dat)
{
    WORD            offset;
#if XM7_VER >= 2
    BOOL            redraw_flag;
#endif

    switch (addr) {
#if XM7_VER == 1 && defined(L4CARD)
	/*
	 * サブモードレジスタ
	 */
    case 0xd405:
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	/*
	 * bit4〜1は400ラインカード有効時のみ有効
	 */
	if (enable_400linecard) {
	    /*
	     * CURSOR LSB
	     */
	    if (dat & 0x10) {
		cursor_lsb = TRUE;
	    } else {
		cursor_lsb = FALSE;
	    }

	    /*
	     * WIDTH
	     */
	    if (dat & 0x08) {
		width40_flag = FALSE;
	    } else {
		width40_flag = TRUE;
	    }

	    /*
	     * ワークRAM選択
	     */
	    if (dat & 0x04) {
		workram_select = TRUE;
	    } else {
		workram_select = FALSE;
	    }

	    /*
	     * 400ラインカード モード切り換え
	     */
	    if (dat & 0x02) {
		if (!enable_400line) {
		    return TRUE;
		}
		enable_400line = FALSE;
	    } else {
		if (enable_400line) {
		    return TRUE;
		}
		enable_400line = TRUE;
	    }

	    /*
	     * フラグ類セット
	     */
	    subreset_flag = TRUE;
	    subbusy_flag = TRUE;

	    /*
	     * CRTレジスタをリセットする
	     */
	    display_reset();
	    display_notify();

	    /*
	     * INS LEDを消灯させる
	     */
	    ins_flag = FALSE;

	    /*
	     * サブCPUをリセット
	     */
	    subcpu_reset();
	}

	return TRUE;
#endif

#if XM7_VER >= 3 || (XM7_VER == 1 && defined(L4CARD))
	/*
	 * サブ漢字ROM アドレス上位
	 */
    case 0xd406:
#if XM7_VER >= 3
	if (fm7_ver >= 3) {
	    sub_kanji_addr &= (WORD) 0x00ff;
	    sub_kanji_addr |= (WORD) (dat << 8);
	}
#else
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	sub_kanji_addr &= (WORD) 0x00ff;
	sub_kanji_addr |= (WORD) (dat << 8);
#endif
	return TRUE;

	/*
	 * サブ漢字ROM アドレス下位
	 */
    case 0xd407:
#if XM7_VER >= 3
	if (fm7_ver >= 3) {
	    sub_kanji_addr &= (WORD) 0xff00;
	    sub_kanji_addr |= (WORD) dat;
	}
#else
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	sub_kanji_addr &= (WORD) 0xff00;
	sub_kanji_addr |= (WORD) dat;
#endif
	return TRUE;
#endif

	/*
	 * CRT OFF
	 */
    case 0xd408:
	if (crt_flag) {
	    /*
	     * CRT ON→OFF
	     */
	    crt_flag = FALSE;
	    display_notify();
	}
	crt_flag = FALSE;
	return TRUE;

	/*
	 * VRAMアクセス OFF
	 */
    case 0xd409:
	vrama_flag = FALSE;
	return TRUE;

	/*
	 * BUSYフラグ ON
	 */
    case 0xd40a:
	/*
	 * CLR命令チェック
	 */
	if ((fetch_op == 0x0f) || (fetch_op == 0x6f) || (fetch_op == 0x7f)) {
	    if (fetch_op == 0x0f) {
		busy_CLR_count = 1;
	    } else {
		busy_CLR_count = 2;
	    }
	    subbusy_flag = FALSE;
	} else {
	    subbusy_flag = TRUE;
	}
	return TRUE;

#if XM7_VER == 1 && defined(L4CARD)
	/*
	 * CRTCアドレスレジスタ(400LINE CARD)
	 */
    case 0xd40b:
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	if (enable_400linecard) {
	    crtc_regnum = (BYTE) (dat & 0x1f);
	}
	return TRUE;

	/*
	 * CRTCデータレジスタ(400LINE CARD)
	 */
    case 0xd40c:
	/*
	 * FM-77モードのみ有効
	 */
	if (fm_subtype != FMSUB_FM77) {
	    return FALSE;
	}

	if (enable_400linecard) {
	    crtc_writeb(crtc_regnum, dat);
	}
	return TRUE;
#endif

	/*
	 * VRAMオフセットアドレス
	 */
    case 0xd40e:
    case 0xd40f:
	if (addr == 0xd40e) {
	    /*
	     * VRAMオフセットアドレス 上位
	     */
#if XM7_VER == 1 && defined(L4CARD)
	    /*
	     * 画面モードによって有効ビット数が変化する
	     */
	    if (enable_400line && enable_400linecard) {
		offset = (WORD) (dat & 0x7f);
	    } else {
		offset = (WORD) (dat & 0x3f);
	    }
#else
	    offset = (WORD) (dat & 0x3f);
#endif
	    offset <<= 8;
#if XM7_VER >= 2
	    offset |= (WORD) (vram_offset[vram_active] & 0xff);
#else
	    offset |= (WORD) (vram_offset[0] & 0xff);
#endif
	} else {
	    /*
	     * VRAMオフセットアドレス 下位
	     */
#if XM7_VER >= 2
	    /*
	     * 拡張オフセットフラグがOFFなら、下位5bitは無効
	     */
	    if (!vram_offset_flag) {
		dat &= 0xe0;
	    }
	    offset = (WORD) (vram_offset[vram_active] & 0x3f00);
	    offset |= (WORD) dat;
#else
#if XM7_VER == 1 && defined(L4CARD)
	    /*
	     * 画面モードによって有効ビット数が変化する
	     */
	    if (enable_400line && enable_400linecard) {
		offset = (WORD) (vram_offset[0] & 0x7f00);
		offset |= (WORD) (dat & 0xfe);
	    } else {
		offset = (WORD) (vram_offset[0] & 0x3f00);
		offset |= (WORD) (dat & 0xe0);
	    }
#else
	    offset = (WORD) (vram_offset[0] & 0x3f00);
	    offset |= (WORD) (dat & 0xe0);
#endif
#endif
	}

#if XM7_VER >= 2
	vram_offset[vram_active] = offset;
	/*
	 * カウントアップ、スクロール
	 */
	vram_offset_count[vram_active]++;
	if ((vram_offset_count[vram_active] & 1) == 0) {
	    vram_scroll((WORD) (vram_offset[vram_active] -
				crtc_offset[vram_active]));
	    crtc_offset[vram_active] = vram_offset[vram_active];
	    display_notify();
	}
#else
	vram_offset[0] = offset;
	/*
	 * カウントアップ、スクロール
	 */
	vram_offset_count[0]++;
	if ((vram_offset_count[0] & 1) == 0) {
	    vram_scroll((WORD) (vram_offset[0] - crtc_offset[0]));
	    crtc_offset[0] = vram_offset[0];
	    display_notify();
	}
#endif
	return TRUE;

#if XM7_VER >= 3
	/*
	 * FM77AV40
	 * サブRAMバンクセレクト/サブ漢字ROMセレクト
	 */
    case 0xd42e:
	if (fm7_ver >= 3) {
	    /*
	     * bit0-2:CGRAMバンクセレクト
	     */
	    cgram_bank = (BYTE) (dat & 0x07);

	    /*
	     * bit3,4:コンソールRAMバンクセレクト
	     */
	    consram_bank = (BYTE) ((dat >> 3) & 0x03);
	    if (consram_bank == 3) {
		/*
		 * バンク3は存在しない
		 */
		ASSERT(FALSE);
		consram_bank = 0;
	    }

	    /*
	     * bit7:第1水準・第2水準セレクト
	     */
	    if (dat & 0x80) {
		/*
		 * 第2水準
		 */
		sub_kanji_bank = TRUE;
	    } else {
		/*
		 * 第1水準
		 */
		sub_kanji_bank = FALSE;
	    }
	}
	return TRUE;

	/*
	 * FM77AV40 400ライン/26万色用VRAMバンクセレクト
	 */
    case 0xd42f:
	if (fm7_ver >= 3) {
	    subram_vrambank = (BYTE) (dat & 0x03);
	    if (subram_vrambank == 3) {
		/*
		 * バンク3は存在しない
		 */
		ASSERT(FALSE);
		subram_vrambank = 0;
	    }

	    /*
	     * ポインタを再構成
	     */
	    display_setpointer(FALSE);
	}
	return TRUE;
#endif

#if XM7_VER >= 2
	/*
	 * FM77AV MISCレジスタ
	 */
    case 0xd430:
	if (fm7_ver >= 2) {
	    redraw_flag = FALSE;

	    /*
	     * NMIマスク
	     */
	    if (dat & 0x80) {
		subnmi_flag = FALSE;
		event[EVENT_SUBTIMER].flag = EVENT_DISABLED;
		subcpu.intr &= ~INTR_NMI;
	    } else {
		subnmi_flag = TRUE;
		event[EVENT_SUBTIMER].flag = EVENT_ENABLED;
	    }

	    /*
	     * アクティブページ
	     */
	    if (dat & 0x20) {
		vram_active = 1;
	    } else {
		vram_active = 0;
	    }

	    /*
	     * ディスプレイページ
	     */
	    if (dat & 0x40) {
		if (vram_display == 0) {
		    vram_display = 1;

		    /*
		     * 200ライン8色モードでは画面再描画が必要
		     */
#if XM7_VER >= 3
		    if (screen_mode == SCR_200LINE) {
#else
		    if (!mode320) {
#endif
			redraw_flag = TRUE;
		    }
		}
	    } else {
		if (vram_display == 1) {
		    vram_display = 0;

		    /*
		     * 200ライン8色モードでは画面再描画が必要
		     */
#if XM7_VER >= 3
		    if (screen_mode == SCR_200LINE) {
#else
		    if (!mode320) {
#endif
			redraw_flag = TRUE;
		    }
		}
	    }

	    /*
	     * 拡張VRAMオフセットレジスタ
	     */
	    if (dat & 0x04) {
		vram_offset_flag = TRUE;
	    } else {
		vram_offset_flag = FALSE;
	    }

	    /*
	     * CGROMバンク
	     */
	    cgrom_bank = (BYTE) (dat & 0x03);

	    /*
	     * ポインタ再構成・画面再描画
	     */
	    display_setpointer(redraw_flag);
	}

	return TRUE;
#endif

#if XM7_VER >= 3
	/*
	 * FM77AV40EX VRAMブロックセレクト
	 */
    case 0xd433:
	if (fm7_ver >= 3) {
	    redraw_flag = FALSE;

	    /*
	     * bit0:アクティブブロックセレクト
	     */
	    if (dat & 0x01) {
		block_active = 1;
	    } else {
		block_active = 0;
	    }

	    /*
	     * bit4:表示ブロックセレクト
	     */
	    if (dat & 0x10) {
		if (block_display == 0) {
		    block_display = 1;
		    if (screen_mode != SCR_262144) {
			redraw_flag = TRUE;
		    }
		}
	    } else {
		if (block_display == 1) {
		    block_display = 0;
		    if (screen_mode != SCR_262144) {
			redraw_flag = TRUE;
		    }
		}
	    }

	    /*
	     * ポインタ再構成・画面再描画
	     */
	    display_setpointer(redraw_flag);
	}
	return TRUE;

	/*
	 * FM77AV40EX ハードウェアウィンドウ
	 */
    case 0xd438:		/* Xウィンドウスタートアドレス(上位)
				 */
    case 0xd439:		/* Xウィンドウスタートアドレス(下位)
				 */
    case 0xd43a:		/* Xウィンドウエンドアドレス(上位)
				 */
    case 0xd43b:		/* Xウィンドウエンドアドレス(下位)
				 */
    case 0xd43c:		/* Yウィンドウスタートアドレス(上位)
				 */
    case 0xd43d:		/* Yウィンドウスタートアドレス(下位)
				 */
    case 0xd43e:		/* Yウィンドウエンドアドレス(上位)
				 */
    case 0xd43f:		/* Yウィンドウエンドアドレス(下位)
				 */
	if (fm7_ver <= 2) {
	    return TRUE;
	}

	switch (addr & 7) {
	case 0:		/* Xウィンドウスタートアドレス(上位)
				 */
	    window_x1 &= (WORD) 0x00f8;
	    window_x1 |= (WORD) ((dat & 0x03) << 8);
	    break;
	case 1:		/* Xウィンドウスタートアドレス(下位)
				 */
	    window_x1 &= (WORD) 0x0300;
	    window_x1 |= (WORD) (dat & 0xf8);
	    break;
	case 2:		/* Xウィンドウエンドアドレス(上位)
				 */
	    window_x2 &= (WORD) 0x00f8;
	    window_x2 |= (WORD) ((dat & 0x03) << 8);
	    break;
	case 3:		/* Xウィンドウエンドアドレス(下位)
				 */
	    window_x2 &= (WORD) 0x0300;
	    window_x2 |= (WORD) (dat & 0xf8);
	    break;
	case 4:		/* Yウィンドウスタートアドレス(上位)
				 */
	    window_y1 &= (WORD) 0x00ff;
	    window_y1 |= (WORD) ((dat & 0x01) << 8);
	    break;
	case 5:		/* Yウィンドウスタートアドレス(下位)
				 */
	    window_y1 &= (WORD) 0x0100;
	    window_y1 |= (WORD) dat;
	    break;
	case 6:		/* Yウィンドウエンドアドレス(上位)
				 */
	    window_y2 &= (WORD) 0x00ff;
	    window_y2 |= (WORD) ((dat & 0x01) << 8);
	    break;
	case 7:		/* Yウィンドウエンドアドレス(下位)
				 */
	    window_y2 &= (WORD) 0x0100;
	    window_y2 |= (WORD) dat;
	    break;
	default:
	    ASSERT(FALSE);
	}

	check_window_open();
	break;
#endif
    }

    return FALSE;
}

/*
 *      ディスプレイ
 *      セーブ
 */
BOOL            FASTCALL
display_save(int fileh)
{
    if (!file_bool_write(fileh, crt_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, vrama_flag)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_bool_write(fileh, subnmi_flag)) {
	return FALSE;
    }
#endif
    if (!file_bool_write(fileh, vsync_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, blank_flag)) {
	return FALSE;
    }

    if (!file_bool_write(fileh, vram_offset_flag)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_word_write(fileh, vram_offset[0])) {
	return FALSE;
    }
    if (!file_word_write(fileh, vram_offset[1])) {
	return FALSE;
    }
    if (!file_word_write(fileh, crtc_offset[0])) {
	return FALSE;
    }
    if (!file_word_write(fileh, crtc_offset[1])) {
	return FALSE;
    }

    if (!file_byte_write(fileh, vram_active)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, vram_display)) {
	return FALSE;
    }
#else
    if (!file_word_write(fileh, vram_offset[0])) {
	return FALSE;
    }
    if (!file_word_write(fileh, crtc_offset[0])) {
	return FALSE;
    }
#endif

    /*
     * Ver6拡張
     */
    if (!file_word_write(fileh, blank_count)) {
	return FALSE;
    }
#if XM7_VER >= 3
    /*
     * Ver8拡張
     */
    if (!file_byte_write(fileh, subram_vrambank)) {
	return FALSE;
    }
    if (!file_word_write(fileh, sub_kanji_addr)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, sub_kanji_bank)) {
	return FALSE;
    }

    if (!file_word_write(fileh, window_x1)) {
	return FALSE;
    }
    if (!file_word_write(fileh, window_dx1)) {
	return FALSE;
    }
    if (!file_word_write(fileh, window_y1)) {
	return FALSE;
    }
    if (!file_word_write(fileh, window_dy1)) {
	return FALSE;
    }
    if (!file_word_write(fileh, window_x2)) {
	return FALSE;
    }
    if (!file_word_write(fileh, window_dx2)) {
	return FALSE;
    }
    if (!file_word_write(fileh, window_y2)) {
	return FALSE;
    }
    if (!file_word_write(fileh, window_dy2)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, window_open)) {
	return FALSE;
    }

    if (!file_byte_write(fileh, block_active)) {
	return FALSE;
    }
    if (!file_byte_write(fileh, block_display)) {
	return FALSE;
    }
#endif

#if XM7_VER == 1 && defined(L4CARD)
    /*
     * XM7 V1.1 / FM-77L4テキストVRAMまわり
     */
    if (!file_bool_write(fileh, width40_flag)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, cursor_lsb)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, text_blink)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, cursor_blink)) {
	return FALSE;
    }
    if (!file_word_write(fileh, text_start_addr)) {
	return FALSE;
    }
    if (!file_word_write(fileh, cursor_addr)) {
	return FALSE;
    }

    if (!file_bool_write(fileh, enable_400line)) {
	return FALSE;
    }
    if (!file_bool_write(fileh, workram_select)) {
	return FALSE;
    }
    if (!file_word_write(fileh, sub_kanji_addr)) {
	return FALSE;
    }

    if (!file_byte_write(fileh, crtc_regnum)) {
	return FALSE;
    }
    if (!file_write(fileh, crtc_register, 0x20)) {
	return FALSE;
    }
#endif

    return TRUE;
}

/*
 *      ディスプレイ
 *      ロード
 */
BOOL            FASTCALL
display_load(int fileh, int ver)
{
    /*
     * バージョンチェック
     */
    if (ver < 200) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &crt_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &vrama_flag)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_bool_read(fileh, &subnmi_flag)) {
	return FALSE;
    }
#endif
    if (!file_bool_read(fileh, &vsync_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &blank_flag)) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &vram_offset_flag)) {
	return FALSE;
    }
#if XM7_VER >= 2
    if (!file_word_read(fileh, &vram_offset[0])) {
	return FALSE;
    }
    if (!file_word_read(fileh, &vram_offset[1])) {
	return FALSE;
    }
    if (!file_word_read(fileh, &crtc_offset[0])) {
	return FALSE;
    }
    if (!file_word_read(fileh, &crtc_offset[1])) {
	return FALSE;
    }

    if (!file_byte_read(fileh, &vram_active)) {
	return FALSE;
    }
    if (!file_byte_read(fileh, &vram_display)) {
	return FALSE;
    }

    /*
     * イベント
     */
    schedule_handle(EVENT_SUBTIMER, subcpu_event);
    schedule_handle(EVENT_VSYNC, display_vsync);
    schedule_handle(EVENT_BLANK, display_blank);
#else
    if (!file_word_read(fileh, &vram_offset[0])) {
	return FALSE;
    }
    if (!file_word_read(fileh, &crtc_offset[0])) {
	return FALSE;
    }

    /*
     * イベント
     */
    schedule_handle(EVENT_SUBTIMER, subcpu_event);
    schedule_handle(EVENT_VSYNC, display_vsync);
#endif

    /*
     * Ver6拡張
     */
#if XM7_VER >= 2
    if ((ver >= 600) || ((ver >= 300) && (ver <= 499))) {
#else
    if (ver >= 300) {
#endif
	if (!file_word_read(fileh, &blank_count)) {
	    return FALSE;
	}
    }
#if XM7_VER >= 3
    /*
     * Ver8拡張
     */
    if (ver >= 800) {
	if (!file_byte_read(fileh, &subram_vrambank)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &sub_kanji_addr)) {
	    return FALSE;
	}
	if (!file_bool_read(fileh, &sub_kanji_bank)) {
	    return FALSE;
	}

	if (!file_word_read(fileh, &window_x1)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &window_dx1)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &window_y1)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &window_dy1)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &window_x2)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &window_dx2)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &window_y2)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &window_dy2)) {
	    return FALSE;
	}
	if (!file_bool_read(fileh, &window_open)) {
	    return FALSE;
	}

	if (!file_byte_read(fileh, &block_active)) {
	    return FALSE;
	}
	if (!file_byte_read(fileh, &block_display)) {
	    return FALSE;
	}
    } else {
	subram_vrambank = 0;
	sub_kanji_addr = 0;
	sub_kanji_bank = FALSE;

	window_x1 = 0;
	window_dx1 = 0;
	window_x2 = 0;
	window_dx2 = 0;
	window_y1 = 0;
	window_dy1 = 0;
	window_y2 = 0;
	window_dy2 = 0;
	window_open = FALSE;

	block_active = 0;
	block_display = 0;
    }
#endif

#if XM7_VER == 1 && defined(L4CARD)
    /*
     * XM7 V1.1 / FM-77L4テキストVRAMまわり
     */
    if (!file_bool_read(fileh, &width40_flag)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &cursor_lsb)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &text_blink)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &cursor_blink)) {
	return FALSE;
    }
    if (!file_word_read(fileh, &text_start_addr)) {
	return FALSE;
    }
    if (!file_word_read(fileh, &cursor_addr)) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &enable_400line)) {
	return FALSE;
    }
    if (!file_bool_read(fileh, &workram_select)) {
	return FALSE;
    }
    if (!file_word_read(fileh, &sub_kanji_addr)) {
	return FALSE;
    }

    if (!file_byte_read(fileh, &crtc_regnum)) {
	return FALSE;
    }
    if (!file_read(fileh, crtc_register, 0x20)) {
	return FALSE;
    }

    /*
     * イベント
     */
    schedule_handle(EVENT_TEXT_BLINK, display_text_blink);
#endif

    /*
     * ポインタを構成
     */
    display_setpointer(TRUE);
    display_setup();

    return TRUE;
}

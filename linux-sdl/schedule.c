/*
 *      FM-7 EMULATOR "XM7"
 *
 *      Copyright (C) 1999-2010 ＰＩ．(yasushi@tanaka.net)
 *      Copyright (C) 2001-2010 Ryu Takegami
 *
 *      [ スケジューラ ]
 */

#include <string.h>
#include "xm7.h"
#include "jsubsys.h"
#include "subctrl.h"
#include "display.h"
#include "mmr.h"
#include "fdc.h"
#include "jsubsys.h"
#if XM7_VER >= 3
#include "dmac.h"
#endif
#include "device.h"
#include "event.h"
#include "tapelp.h"

/*
 *      グローバル ワーク
 */
BOOL            run_flag;	/* 動作中フラグ */
BOOL            stopreq_flag;	/* 停止要求フラグ */
event_t         event[EVENT_MAXNUM];	/* イベント データ */
breakp_t        breakp[BREAKP_MAXNUM];	/* ブレークポイント
					 * データ */
WORD            main_runadr;	/* メインCPUの前回実行アドレス 
				 */
WORD            sub_runadr;	/* サブCPUの前回実行アドレス */
DWORD           main_speed;	/* メインCPUスピード */
DWORD           mmr_speed;	/* メイン(MMR)スピード */
#if XM7_VER >= 3
DWORD           fmmr_speed;	/* メイン(高速MMR)スピード */
#endif
DWORD           sub_speed;	/* サブCPUスピード */
WORD            main_overcycles;	/* メインCPUオーバーサイクル 
					 */
WORD            sub_overcycles;	/* サブCPUオーバーサイクル */
BOOL            cycle_steal;	/* サイクルスチールフラグ */
DWORD           speed_ratio;	/* CPU動作速度(%) */
DWORD           vmtime;		/* VM仮想時間 */
#if XM7_VER == 1
DWORD           main_speed_low;	/* メインCPUスピード(低速) */
DWORD           sub_speed_low;	/* サブCPUスピード(低速) */
#ifdef JSUB
DWORD           jsub_speed;	/* 日本語サブCPUスピード */
WORD            jsub_overcycles;	/* 日本語サブCPUオーバーサイクル 
					 */
WORD            jsub_runadr;	/* 日本語サブCPUの前回実行アドレス 
				 */
#endif
#endif

/*
 *      スタティック ワーク
 */
static BOOL     break_flag;	/* ブレークポイント有効フラグ 
				 */
static int      exec0;		/* 高精度合成用実行時間補助カウンタ 
				 */

/*
 *      スケジューラ
 *      初期化
 */
BOOL            FASTCALL
schedule_init(void)
{
    run_flag = FALSE;
    stopreq_flag = FALSE;
    break_flag = FALSE;
    memset(breakp, 0, sizeof(breakp));
    memset(event, 0, sizeof(event));

    /*
     * 前回の実行アドレスを初期化 
     */
    main_runadr = 0xFFFF;
    sub_runadr = 0xFFFF;
#if (XM7_VER == 1) && defined(JSUB)
    jsub_runadr = 0xFFFF;
#endif

    /*
     * CPU速度初期設定 
     */
    main_speed = MAINCYCLES * 10;
    mmr_speed = MAINCYCLES_MMR * 10;
#if XM7_VER >= 3
    fmmr_speed = MAINCYCLES_FMMR * 10;
#endif
    sub_speed = SUBCYCLES * 10;
    cycle_steal = TRUE;
    main_overcycles = 0;
    sub_overcycles = 0;
#if XM7_VER == 1
    main_speed_low = MAINCYCLES_LOW * 10;
    sub_speed_low = SUBCYCLES_LOW * 10;
#ifdef JSUB
    jsub_speed = JSUBCYCLES * 10;
    jsub_overcycles = 0;
#endif
#endif
    exec0 = 0;

    /*
     * 仮想時間初期化 
     */
    vmtime = 0;

    return TRUE;
}

/*
 *      スケジューラ
 *      クリーンアップ
 */
void            FASTCALL
schedule_cleanup(void)
{
}

/*
 *      スケジューラ
 *      リセット
 */
void            FASTCALL
schedule_reset(void)
{
    int             i;

    /*
     * RTCイベント以外のスケジュールをクリア 
     */
    /*
     * (XM7起動直後はRTCイベントも schedule_init
     * でクリアされている) 
     */
    for (i = 0; i < EVENT_MAXNUM; i++) {
	if (i != EVENT_RTC) {
	    memset(&event[i], 0, sizeof(event_t));
	}
    }

    /*
     * カウンタクリア 
     */
    maincpu.total = 0;
    subcpu.total = 0;
    main_overcycles = 0;
    sub_overcycles = 0;
#if (XM7_VER == 1) && defined(JSUB)
    jsubcpu.total = 0;
    jsub_overcycles = 0;
#endif
    exec0 = 0;

    /*
     * 仮想時間初期化 
     */
    vmtime = 0;

    /*
     * 実行速度比率初期化 
     */
    speed_ratio = 10000;
}

/*
 *      スケジューラ
 *      実行サイクル数取得
 */
DWORD           FASTCALL
schedule_get_cycle(void)
{
    DWORD           tmp;

#if XM7_VER >= 3
    if (mmr_fastmode) {
	tmp = fmmr_speed;
    } else
#endif
#if XM7_VER == 1
    if (lowspeed_mode) {
	tmp = main_speed_low;
    } else {
#else
    {
#endif
	if (mmr_flag || twr_flag) {
	    tmp = mmr_speed;
	} else {
	    tmp = main_speed;
	}
    }

#if XM7_VER >= 3
    /*
     * 高速リフレッシュモードのサイクル数を生成 
     */
    if (!mmr_fastmode && mmr_fast_refresh) {
	if (mmr_flag || twr_flag) {
	    /*
	     * MMR/TWR有効時 約8.9%アップ 
	     */
	    tmp = (DWORD) ((tmp * 4461) >> 12);
	} else {
	    /*
	     * MMR/TWR無効時 約8.6%アップ 
	     */
	    tmp = (DWORD) ((tmp * 4447) >> 12);
	}
    }
#endif

    /*
     * CPU速度比率 
     */
    if (speed_ratio != 10000) {
	tmp = (tmp * speed_ratio) / 10000;
	if (tmp < 1) {
	    tmp = 1;
	}
    }

    return tmp;
}

/*-[ イベント ]-------------------------------------------------------------*/

/*
 *      スケジューラ
 *      イベント設定
 */
BOOL            FASTCALL
schedule_setevent(int id, DWORD microsec, BOOL(FASTCALL * func) (void))
{
    DWORD           exec;

    ASSERT((id >= 0) && (id < EVENT_MAXNUM));
    ASSERT(func);

    if ((id < 0) || (id >= EVENT_MAXNUM)) {
	return FALSE;
    }
    if (microsec == 0) {
	event[id].flag = EVENT_NOTUSE;
	return FALSE;
    }

    /*
     * 登録 
     */
    event[id].current = microsec;
    event[id].reload = microsec;
    event[id].callback = func;
    event[id].flag = EVENT_ENABLED;

    /*
     * 実行中なら、時間を足しておく必要がある(後で引くため) 
     */
    if (run_flag) {
	exec = (DWORD) maincpu.total;
	exec *= 10000;
	exec /= schedule_get_cycle();
	event[id].current += exec;
    }

    return TRUE;
}

/*
 *      スケジューラ
 *      イベント削除
 */
BOOL            FASTCALL
schedule_delevent(int id)
{
    ASSERT((id >= 0) && (id < EVENT_MAXNUM));

    if ((id < 0) || (id >= EVENT_MAXNUM)) {
	return FALSE;
    }

    /*
     * 未使用に 
     */
    event[id].flag = EVENT_NOTUSE;

    return TRUE;
}

/*
 *      スケジューラ
 *      イベントハンドラ設定
 */
void            FASTCALL
schedule_handle(int id, BOOL(FASTCALL * func) (void))
{
    ASSERT((id >= 0) && (id < EVENT_MAXNUM));
    ASSERT(func);

    /*
     * コールバック関数を登録するのみ。それ以外は触らない 
     */
    event[id].callback = func;
}

/*
 *      スケジューラ
 *      最短実行時間調査
 */
static DWORD    FASTCALL
schedule_chkevent(DWORD microsec)
{
    DWORD           exectime;
    int             i;

    /*
     * 初期設定 
     */
    exectime = microsec;

    /*
     * イベントを回って調査 
     */
    for (i = 0; i < EVENT_MAXNUM; i++) {
	if (event[i].flag == EVENT_NOTUSE) {
	    continue;
	}

//	ASSERT(event[i].current > 0);
//	ASSERT(event[i].reload > 0);

	if (event[i].current < exectime) {
	    exectime = event[i].current;
	}
    }

    return exectime;
}

/*
 *      スケジューラ
 *      進行処理
 */
static void     FASTCALL
schedule_doevent(DWORD microsec)
{
    int             i;

    for (i = 0; i < EVENT_MAXNUM; i++) {
	if (event[i].flag == EVENT_NOTUSE) {
	    continue;
	}

	ASSERT(event[i].current > 0);
	ASSERT(event[i].reload > 0);

	/*
	 * 実行時間を引き 
	 */
	if (event[i].current < microsec) {
	    event[i].current = 0;
	} else {
	    event[i].current -= microsec;
	}

	/*
	 * カウンタが0なら 
	 */
	if (event[i].current == 0) {
	    /*
	     * 時間はENABLE,DISABLEにかかわらずリロード 
	     */
	    event[i].current = event[i].reload;
	    /*
	     * コールバック実行 
	     */
	    if (event[i].flag == EVENT_ENABLED) {
		if (!event[i].callback()) {
		    event[i].flag = EVENT_DISABLED;
		}
	    }
	}
    }
}

/*-[ ブレークポイント ]-----------------------------------------------------*/

/*
 *      スケジューラ
 *      ブレークポイントセット(すでにセットしてあれば消去)
 */
BOOL            FASTCALL
schedule_setbreak(int cpu, WORD addr)
{
    int             i;

    /*
     * まず、全てのブレークポイントを検索し、見つかるか 
     */
    for (i = 0; i < BREAKP_MAXNUM; i++) {
	if (breakp[i].flag != BREAKP_NOTUSE) {
	    if ((breakp[i].cpu == cpu) && (breakp[i].addr == addr)) {
		break;
	    }
	}
    }
    /*
     * 見つかれば、削除 
     */
    if (i != BREAKP_MAXNUM) {
	breakp[i].flag = BREAKP_NOTUSE;
	/*
	 * ブレーク有効フラグをチェック 
	 */
	break_flag = FALSE;
	for (i = 0; i < BREAKP_MAXNUM; i++) {
	    if (breakp[i].flag != BREAKP_NOTUSE) {
		break_flag = TRUE;
	    }
	}
	return TRUE;
    }

    /*
     * 空きを調査 
     */
    for (i = 0; i < BREAKP_MAXNUM; i++) {
	if (breakp[i].flag == BREAKP_NOTUSE) {
	    break;
	}
    }
    /*
     * すべて埋まっているか 
     */
    if (i == BREAKP_MAXNUM) {
	return FALSE;
    }

    /*
     * セット 
     */
    breakp[i].flag = BREAKP_ENABLED;
    breakp[i].cpu = cpu;
    breakp[i].addr = addr;

    /*
     * ブレーク有効 
     */
    break_flag = TRUE;

    return TRUE;
}

/*
 *      スケジューラ
 *      ブレークポイントセット(位置指定)
 */
BOOL            FASTCALL
schedule_setbreak2(int num, int cpu, WORD addr)
{
    /*
     * セット 
     */
    if (breakp[num].flag != BREAKP_DISABLED) {
	breakp[num].flag = BREAKP_ENABLED;
    }
    breakp[num].cpu = cpu;
    breakp[num].addr = addr;

    /*
     * ブレーク有効 
     */
    break_flag = TRUE;

    return TRUE;
}

/*
 *      スケジューラ
 *      ブレークチェック
 */
static BOOL     FASTCALL
schedule_chkbreak(void)
{
    int             i;
    WORD            main_prevrunadr;
    WORD            sub_prevrunadr;
#if (XM7_VER == 1) && defined(JSUB)
    WORD            jsub_prevrunadr;
#endif

    ASSERT(break_flag);

    main_prevrunadr = main_runadr;
    sub_prevrunadr = sub_runadr;
    main_runadr = maincpu.pc;
    sub_runadr = subcpu.pc;
#if (XM7_VER == 1) && defined(JSUB)
    jsub_prevrunadr = jsub_runadr;
    jsub_runadr = jsubcpu.pc;
#endif

    for (i = 0; i < BREAKP_MAXNUM; i++) {
	if (breakp[i].flag == BREAKP_ENABLED) {
	    if (breakp[i].cpu == MAINCPU) {
		if (breakp[i].addr == maincpu.pc) {
		    if (maincpu.pc != main_prevrunadr) {
			return TRUE;
		    }
		}
	    } else if (breakp[i].cpu == SUBCPU) {
		if (breakp[i].addr == subcpu.pc) {
		    if (subcpu.pc != sub_prevrunadr) {
			return TRUE;
		    }
		}
	    }
#if (XM7_VER == 1) && defined(JSUB)
	    else if (breakp[i].cpu == JSUBCPU) {
		if (breakp[i].addr == jsubcpu.pc) {
		    if (jsubcpu.pc != jsub_prevrunadr) {
			return TRUE;
		    }
		}
	    }
#endif
	}
    }

    return FALSE;
}

/*-[ 実行部 ]---------------------------------------------------------------*/

/*
 *      トレース
 */
void            FASTCALL
schedule_trace(void)
{
    /*
     * １命令実行 
     */
#if XM7_VER >= 3
    if (!dma_burst_transfer || !dma_flag) {
	maincpu_execline();
    }
#else
    maincpu_execline();
#endif

#if XM7_VER >= 3
    /*
     * DMA転送 
     */
    if (dma_flag) {
	dmac_exec();
    }
#endif

#if XM7_VER >= 2
    if ((!subhalt_flag || (subcpu.intr & INTR_HALT)) &&
	(cycle_steal || !(vrama_flag && !blank_flag))) {
#else
    if (!subhalt_flag || (subcpu.intr & INTR_HALT)) {
#endif
	subcpu_execline();
#if XM7_VER == 1
	/*
	 * VRAMアクセスフラグONの場合,所要クロックを3倍にする 
	 */
	if (vrama_flag) {
	    subcpu.total += (WORD) (subcpu.cycle * 2);
	}
#endif
    }
#if (XM7_VER == 1) && defined(JSUB)
    if (jsub_available && jsub_enable && !jsub_haltflag &&
	(fm_subtype != FMSUB_FM8)) {
	jsubcpu_execline();
    }
#endif

    /*
     * HALT要求に応答 (V3.1) 
     */
    subctrl_halt_ack();

    /*
     * オーバーサイクル数をクリア 
     */
    main_overcycles = 0;
    sub_overcycles = 0;
#if (XM7_VER == 1) && defined(JSUB)
    jsub_overcycles = 0;
#endif
}

/*
 *      メインだけ全力駆動
 */
void            FASTCALL
schedule_main_fullspeed(void)
{
    ASSERT(run_flag);
    ASSERT(!stopreq_flag);

#if XM7_VER >= 3
    if (dma_burst_transfer) {
	/*
	 * バースト転送実行中。メインCPUは動けない 
	 */
	return;
    }
#endif

    if (break_flag) {
	/*
	 * ブレークポイントあり 
	 */
	if (schedule_chkbreak()) {
	    stopreq_flag = TRUE;
	}
	if (stopreq_flag) {
	    return;
	}
    }

    /*
     * メインCPU実行 
     */
#ifdef FDDSND
    if (!fdc_waitmode || !(fdc_status & FDC_ST_BUSY)) {
	maincpu_exec();
    }
#else
    maincpu_exec();
#endif

    /*
     * HALT要求に応答 (V3.1) 
     */
    subctrl_halt_ack();

    /*
     * オーバーサイクル数をクリア 
     */
    main_overcycles = 0;
}

/*
 *      全力駆動
 */
void            FASTCALL
schedule_fullspeed(void)
{
    ASSERT(run_flag);
    ASSERT(!stopreq_flag);

    if (break_flag) {
	/*
	 * ブレークポイントあり 
	 */
	if (schedule_chkbreak()) {
	    stopreq_flag = TRUE;
	}
	if (stopreq_flag) {
	    return;
	}
    }

    /*
     * メインCPU実行 
     */
#if XM7_VER >= 3
    if (!dma_burst_transfer) {
#ifdef FDDSND
	if (!fdc_waitmode || !(fdc_status & FDC_ST_BUSY)) {
	    maincpu_exec();
	}
#else
	maincpu_exec();
#endif
    }
#else
#ifdef FDDSND
    if (!fdc_waitmode || !(fdc_status & FDC_ST_BUSY)) {
	maincpu_exec();
    }
#else
    maincpu_exec();
#endif
#endif

    /*
     * サブCPU実行 
     */
#if XM7_VER >= 2
    if ((!subhalt_flag || (subcpu.intr & INTR_HALT)) &&
	(cycle_steal || !(vrama_flag && !blank_flag))) {
#else
    if (!subhalt_flag || (subcpu.intr & INTR_HALT)) {
#endif
	subcpu_exec();
#if XM7_VER == 1
	/*
	 * VRAMアクセスフラグONの場合,所要クロックを3倍にする 
	 */
	if (vrama_flag) {
	    subcpu.total += (WORD) (subcpu.cycle * 2);
	}
#endif
    }
#if (XM7_VER == 1) && defined(JSUB)
    /*
     * 日本語サブCPU実行 
     */
    if (jsub_available && jsub_enable && !jsub_haltflag &&
	(fm_subtype != FMSUB_FM8)) {
	jsubcpu_execline();
    }
#endif

    /*
     * HALT要求に応答 (V3.1) 
     */
    subctrl_halt_ack();

    /*
     * オーバーサイクル数をクリア 
     */
    main_overcycles = 0;
    sub_overcycles = 0;
#if (XM7_VER == 1) && defined(JSUB)
    jsub_overcycles = 0;
#endif
}

/*
 *      実行
 */
DWORD           FASTCALL
schedule_exec(DWORD microsec)
{
    DWORD           exec;
    DWORD           exec2;
    DWORD           count;
    DWORD           cycle;
    DWORD           ratio;
    WORD            maincnt;
    WORD            sub;
    DWORD           limit;
    DWORD           tmp;
#if (XM7_VER == 1) && defined(JSUB)
    DWORD           ratio_js;
    WORD            jsub;
#endif

    ASSERT(run_flag);
    ASSERT(!stopreq_flag);

    /*
     * 最短の実行時間を得る 
     */
    exec = schedule_chkevent(microsec);
    exec2 = 0;

    do {
	/*
	 * メインCPUとサブCPUの動作速度比率を求める 
	 */
	cycle = schedule_get_cycle();
#if XM7_VER == 1
	if (lowspeed_mode) {
	    tmp = (sub_speed_low * speed_ratio) / 100000;
	} else {
	    tmp = (sub_speed * speed_ratio) / 100000;
	}
#else
	tmp = (sub_speed * speed_ratio) / 100000;
#endif
	if (tmp < 1) {
	    tmp = 1;
	}
	ratio = (tmp << 12);
	ratio /= (cycle / 10);
#if (XM7_VER == 1) && defined(JSUB)
	tmp = (jsub_speed * speed_ratio) / 100000;
	if (tmp < 1) {
	    tmp = 1;
	}
	ratio_js = (tmp << 12);
	ratio_js /= (cycle / 10);
#endif

	/*
	 * CPU時間に換算 
	 */
	count = cycle;
	count *= (exec - exec2);
	count /= 10000;
	maincnt = (WORD) count;
	sub = (WORD) ((maincnt * ratio) >> 12);
#if (XM7_VER == 1) && defined(JSUB)
	jsub = (WORD) ((maincnt * ratio_js) >> 12);
#endif

	/*
	 * カウンタ・フラグ初期化 
	 */
	maincpu.total = main_overcycles;
	subcpu.total = sub_overcycles;
#if (XM7_VER == 1) && defined(JSUB)
	jsubcpu.total = jsub_overcycles;
#endif
	mmr_modify = FALSE;

	if (cycle_steal) {
	    if (break_flag) {
		/*
		 * サイクルスチールあり、ブレークポイントあり 
		 */

		/*
		 * 実行 
		 */
		while ((maincpu.total < maincnt) && !mmr_modify) {
		    if (schedule_chkbreak()) {
			stopreq_flag = TRUE;
		    }
		    if (stopreq_flag) {
			break;
		    }
#if XM7_VER >= 3
		    /*
		     * メインCPU実行 
		     */
		    if (!dma_burst_transfer) {
			maincpu_exec();
		    }

		    /*
		     * DMA転送 
		     */
		    if (dma_flag) {
			dmac_exec();
		    }
#else
		    /*
		     * メインCPU実行 
		     */
		    maincpu_exec();
#endif

		    /*
		     * サブ側でCLR命令を使用してBUSYフラグを操作した場合の 
		     */
		    /*
		     * 後処理(MAGUS) 
		     */
		    if (busy_CLR_count) {
			busy_CLR_count--;
			if (busy_CLR_count == 0) {
			    subbusy_flag = TRUE;
			}
		    }

		    /*
		     * サブCPU実行 
		     */
		    limit = maincpu.total * ratio;
		    if (subhalt_flag && !(subcpu.intr & INTR_HALT)) {
			/*
			 * メインCPUと強制同期 
			 */
			subcpu.total = (WORD) (limit >> 12);
		    } else {
			while (((DWORD) subcpu.total << 12) < limit) {
			    if (schedule_chkbreak()) {
				stopreq_flag = TRUE;
			    }
			    if (stopreq_flag) {
				break;
			    }
			    subcpu_exec();
			}
		    }

#if (XM7_VER == 1) && defined(JSUB)
		    /*
		     * 日本語サブCPU実行 
		     */
		    limit = maincpu.total * ratio_js;
		    while ((((DWORD) jsubcpu.total << 12) < limit) &&
			   jsub_available && jsub_enable && !jsub_haltflag
			   && (fm_subtype != FMSUB_FM8)) {
			if (schedule_chkbreak()) {
			    stopreq_flag = TRUE;
			}
			if (stopreq_flag) {
			    break;
			}
			jsubcpu_exec();
		    }
#endif

		    /*
		     * HALT要求に応答 (V3.1) 
		     */
		    subctrl_halt_ack();
		}

		/*
		 * ブレークした場合の処理 
		 */
		if (stopreq_flag) {
		    /*
		     * exec分時間を進める(あえて補正しない) 
		     */
		    run_flag = FALSE;
		}
	    } else {
		/*
		 * サイクルスチールあり、ブレークポイントなし 
		 */

		/*
		 * 実行 
		 */
		while ((maincpu.total < maincnt) && !mmr_modify) {
#if XM7_VER >= 3
		    /*
		     * メインCPU実行 
		     */
		    if (!dma_burst_transfer) {
			maincpu_exec();
		    }

		    /*
		     * DMA転送 
		     */
		    if (dma_flag) {
			dmac_exec();
		    }
#else
		    /*
		     * メインCPU実行 
		     */
		    maincpu_exec();
#endif

		    /*
		     * サブ側でCLR命令を使用してBUSYフラグを操作した場合の 
		     */
		    /*
		     * 後処理(MAGUS) 
		     */
		    if (busy_CLR_count) {
			busy_CLR_count--;
			if (busy_CLR_count == 0) {
			    subbusy_flag = TRUE;
			}
		    }

		    /*
		     * サブCPU実行 
		     */
		    limit = maincpu.total * ratio;
		    if (subhalt_flag && !(subcpu.intr & INTR_HALT)) {
			/*
			 * メインCPUと強制同期 
			 */
			subcpu.total = (WORD) (limit >> 12);
		    } else {
			while (((DWORD) subcpu.total << 12) < limit) {
			    subcpu_exec();
			}
		    }

#if (XM7_VER == 1) && defined(JSUB)
		    /*
		     * 日本語サブCPU実行 
		     */
		    limit = maincpu.total * ratio_js;
		    while ((((DWORD) jsubcpu.total << 12) < limit) &&
			   jsub_available && jsub_enable && !jsub_haltflag
			   && (fm_subtype != FMSUB_FM8)) {
			jsubcpu_exec();
		    }
#endif

		    /*
		     * HALT要求に応答 (V3.1) 
		     */
		    subctrl_halt_ack();
		}
	    }
	} else {
	    if (break_flag) {
		/*
		 * サイクルスチールなし、ブレークポイントあり 
		 */

		/*
		 * 実行 
		 */
		while ((maincpu.total < maincnt) && !mmr_modify) {
		    if (schedule_chkbreak()) {
			stopreq_flag = TRUE;
		    }
		    if (stopreq_flag) {
			break;
		    }
#if XM7_VER >= 3
		    /*
		     * メインCPU実行 
		     */
		    if (!dma_burst_transfer) {
			maincpu_exec();
		    }

		    /*
		     * DMA転送 
		     */
		    if (dma_flag) {
			dmac_exec();
		    }
#else
		    /*
		     * メインCPU実行 
		     */
		    maincpu_exec();
#endif

		    /*
		     * サブ側でCLR命令を使用してBUSYフラグを操作した場合の 
		     */
		    /*
		     * 後処理(MAGUS) 
		     */
		    if (busy_CLR_count) {
			busy_CLR_count--;
			if (busy_CLR_count == 0) {
			    subbusy_flag = TRUE;
			}
		    }

		    /*
		     * サブCPU実行 
		     */
		    limit = maincpu.total * ratio;
#if XM7_VER >= 2
		    if ((subhalt_flag && !(subcpu.intr & INTR_HALT)) ||
			(vrama_flag && blank_flag)) {
#else
		    if (subhalt_flag && !(subcpu.intr & INTR_HALT)) {
#endif
			/*
			 * メインCPUと強制同期 
			 */
			subcpu.total = (WORD) (limit >> 12);
		    } else {
			while (((DWORD) subcpu.total << 12) < limit) {
			    if (schedule_chkbreak()) {
				stopreq_flag = TRUE;
			    }
			    if (stopreq_flag) {
				break;
			    }

			    subcpu_exec();
#if XM7_VER == 1
			    /*
			     * VRAMアクセスフラグONの場合 
			     */
			    /*
			     * 所要クロックを3倍にする 
			     */
			    if (vrama_flag) {
				subcpu.total += (WORD) (subcpu.cycle * 2);
			    }
#endif
			}
		    }

#if (XM7_VER == 1) && defined(JSUB)
		    /*
		     * 日本語サブCPU実行 
		     */
		    limit = maincpu.total * ratio_js;
		    while ((((DWORD) jsubcpu.total << 12) < limit) &&
			   jsub_available && jsub_enable && !jsub_haltflag
			   && (fm_subtype != FMSUB_FM8)) {
			if (schedule_chkbreak()) {
			    stopreq_flag = TRUE;
			}
			if (stopreq_flag) {
			    break;
			}
			jsubcpu_exec();
		    }
#endif

		    /*
		     * HALT要求に応答 (V3.1) 
		     */
		    subctrl_halt_ack();
		}

		/*
		 * ブレークした場合の処理 
		 */
		if (stopreq_flag) {
		    /*
		     * exec分時間を進める(あえて補正しない) 
		     */
		    run_flag = FALSE;
		}
	    } else {
		/*
		 * サイクルスチールなし、ブレークポイントなし 
		 */

		/*
		 * 実行 
		 */
		while ((maincpu.total < maincnt) && !mmr_modify) {
#if XM7_VER >= 3
		    /*
		     * メインCPU実行 
		     */
		    if (!dma_burst_transfer) {
			maincpu_exec();
		    }

		    /*
		     * DMA転送 
		     */
		    if (dma_flag) {
			dmac_exec();
		    }
#else
		    /*
		     * メインCPU実行 
		     */
		    maincpu_exec();
#endif

		    /*
		     * サブ側でCLR命令を使用してBUSYフラグを操作した場合の 
		     */
		    /*
		     * 後処理(MAGUS) 
		     */
		    if (busy_CLR_count) {
			busy_CLR_count--;
			if (busy_CLR_count == 0) {
			    subbusy_flag = TRUE;
			}
		    }

		    /*
		     * サブCPU実行 
		     */
		    limit = maincpu.total * ratio;
#if XM7_VER >= 2
		    if ((subhalt_flag && !(subcpu.intr & INTR_HALT)) ||
			(vrama_flag && blank_flag)) {
#else
		    if (subhalt_flag && !(subcpu.intr & INTR_HALT)) {
#endif
			/*
			 * メインCPUと強制同期 
			 */
			subcpu.total = (WORD) (limit >> 12);
		    } else {
			while (((DWORD) subcpu.total << 12) < limit) {
			    subcpu_exec();
#if XM7_VER == 1
			    /*
			     * VRAMアクセスフラグONの場合 
			     */
			    /*
			     * 所要クロックを3倍にする 
			     */
			    if (vrama_flag) {
				subcpu.total += (WORD) (subcpu.cycle * 2);
			    }
#endif
			}
		    }

#if (XM7_VER == 1) && defined(JSUB)
		    /*
		     * 日本語サブCPU実行 
		     */
		    limit = maincpu.total * ratio_js;
		    while ((((DWORD) jsubcpu.total << 12) < limit) &&
			   jsub_available && jsub_enable && !jsub_haltflag
			   && (fm_subtype != FMSUB_FM8)) {
			jsubcpu_exec();
		    }
#endif

		    /*
		     * HALT要求に応答 (V3.1) 
		     */
		    subctrl_halt_ack();
		}
	    }
	}

	/*
	 * MMR関連レジスタが変更された場合、実行時間を補正 
	 */
	if (mmr_modify) {
	    exec2 += (maincpu.total * 10000) / cycle;
	    main_overcycles = 0;
	    sub_overcycles = 0;
#if (XM7_VER == 1) && defined(JSUB)
	    jsub_overcycles = 0;
#endif
	}
    }
    while (mmr_modify && run_flag && (exec > exec2));

    /*
     * オーバーサイクル処理 
     */
    if (maincpu.total > maincnt) {
	main_overcycles = (WORD) (maincpu.total - maincnt);
    } else {
	main_overcycles = 0;
    }
    if (subcpu.total > sub) {
	sub_overcycles = (WORD) (subcpu.total - sub);
    } else {
	sub_overcycles = 0;
    }
#if (XM7_VER == 1) && defined(JSUB)
    if (jsubcpu.total > jsub) {
	jsub_overcycles = (WORD) (jsubcpu.total - jsub);
    } else {
	jsub_overcycles = 0;
    }
#endif

    /*
     * イベント処理 
     */
    maincpu.total = 0;
    subcpu.total = 0;
#if (XM7_VER == 1) && defined(JSUB)
    jsubcpu.total = 0;
#endif
    schedule_doevent(exec);
    vmtime += exec;

    return exec;
}

/*-[ ファイルI/O ]----------------------------------------------------------*/

/*
 *      スケジューラ
 *      セーブ
 */
BOOL            FASTCALL
schedule_save(int fileh)
{
    int             i;

    if (!file_bool_write(fileh, run_flag)) {
	return FALSE;
    }

    if (!file_bool_write(fileh, stopreq_flag)) {
	return FALSE;
    }

    /*
     * Ver901拡張 
     */
    if (!file_byte_write(fileh, BREAKP_MAXNUM)) {
	return FALSE;
    }

    /*
     * ブレークポイント 
     */
    for (i = 0; i < BREAKP_MAXNUM; i++) {
	if (!file_byte_write(fileh, (BYTE) breakp[i].flag)) {
	    return FALSE;
	}
	if (!file_byte_write(fileh, (BYTE) breakp[i].cpu)) {
	    return FALSE;
	}
	if (!file_word_write(fileh, breakp[i].addr)) {
	    return FALSE;
	}
    }

    /*
     * イベント 
     */

    /*
     * Ver9拡張 
     */
    if (!file_byte_write(fileh, EVENT_MAXNUM)) {
	return FALSE;
    }

    for (i = 0; i < EVENT_MAXNUM; i++) {
	/*
	 * コールバック以外を保存 
	 */
	if (!file_byte_write(fileh, (BYTE) event[i].flag)) {
	    return FALSE;
	}
	if (!file_dword_write(fileh, event[i].current)) {
	    return FALSE;
	}
	if (!file_dword_write(fileh, event[i].reload)) {
	    return FALSE;
	}
    }

    /*
     * Ver9.05/7.05拡張 
     */
    if (!file_word_write(fileh, main_overcycles)) {
	return FALSE;
    }
    if (!file_word_write(fileh, sub_overcycles)) {
	return FALSE;
    }
#if (XM7_VER == 1) && defined(JSUB)
    if (!file_word_write(fileh, jsub_overcycles)) {
	return FALSE;
    }
#endif

    return TRUE;
}

/*
 *      スケジューラ
 *      ロード
 */
BOOL            FASTCALL
schedule_load(int fileh, int ver, BOOL old)
{
    int             i;
    BYTE            tmp;
    BYTE            MAXNUM;

    /*
     * バージョンチェック 
     */
    if (ver < 200) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &run_flag)) {
	return FALSE;
    }

    if (!file_bool_read(fileh, &stopreq_flag)) {
	return FALSE;
    }

    /*
     * ブレークポイント 
     */
    /*
     * Ver9拡張 
     */
#if XM7_VER >= 3
    if ((ver >= 901) || ((ver >= 701) && (ver <= 799))) {
#elif XM7_VER >= 2
    if (ver >= 701) {
#else
    if (ver >= 300) {
#endif
	if (!file_byte_read(fileh, &MAXNUM)) {
	    return FALSE;
	}
    } else {
	MAXNUM = BREAKP_MAXNUM_OLD;
    }

    /*
     * いったん初期化する 
     */
    memset(breakp, 0, sizeof(breakp));

    for (i = 0; i < MAXNUM; i++) {
	if (!file_byte_read(fileh, &tmp)) {
	    return FALSE;
	}
	breakp[i].flag = (int) tmp;
	if (!file_byte_read(fileh, &tmp)) {
	    return FALSE;
	}
	breakp[i].cpu = (int) tmp;
	if (!file_word_read(fileh, &breakp[i].addr)) {
	    return FALSE;
	}
    }
    break_flag = FALSE;
    for (i = 0; i < MAXNUM; i++) {
	if (breakp[i].flag != BREAKP_NOTUSE) {
	    break_flag = TRUE;
	}
    }

    /*
     * イベント 
     */
    /*
     * Ver9拡張 
     */
#if XM7_VER >= 3
    if ((ver >= 900) || ((ver >= 700) && (ver <= 799))) {
#elif XM7_VER >= 2
    if (ver >= 700) {
#else
    if (ver >= 300) {
#endif
	if (!file_byte_read(fileh, &MAXNUM)) {
	    return FALSE;
	}
    } else {
	if (old) {
	    MAXNUM = EVENT_MAXNUM_L30;
	} else {
	    MAXNUM = EVENT_MAXNUM_L31;
	}
    }

    for (i = 0; i < MAXNUM; i++) {
	/*
	 * コールバック以外を設定 
	 */
	if (!file_byte_read(fileh, &tmp)) {
	    return FALSE;
	}
	event[i].flag = (int) tmp;
	if (!file_dword_read(fileh, &event[i].current)) {
	    return FALSE;
	}
	if (!file_dword_read(fileh, &event[i].reload)) {
	    return FALSE;
	}
    }

    /*
     * Ver9.05/Ver7.05拡張 
     */
#if XM7_VER >= 3
    if ((ver >= 905) || ((ver >= 705) && (ver <= 799))) {
#elif XM7_VER >= 2
    if (ver >= 705) {
#else
    if (ver >= 300) {
#endif
	if (!file_word_read(fileh, &main_overcycles)) {
	    return FALSE;
	}
	if (!file_word_read(fileh, &sub_overcycles)) {
	    return FALSE;
	}
#if (XM7_VER == 1) && defined(JSUB)
	if (!file_word_read(fileh, &jsub_overcycles)) {
	    return FALSE;
	}
#endif
    } else {
	main_overcycles = 0;
	sub_overcycles = 0;
#if (XM7_VER == 1) && defined(JSUB)
	jsub_overcycles = 0;
#endif
    }

    /*
     * 実行速度比率初期化 
     */
    speed_ratio = 10000;

    return TRUE;
}

/*
 * agar_sch.cpp
 *
 *  Created on: 2010/11/08
 *      Author: whatisthis
 */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include <SDL/SDL.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "xm7.h"
#include "tapelp.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "mouse.h"

#include "agar_xm7.h"
#include "agar_osd.h"
#include "agar_logger.h"

#include "sdl_sch.h"

#include "api_snd.h"
#include "api_kbd.h"
#include "api_js.h"
#include "api_draw.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*
 *  グローバル ワーク
 */
DWORD dwExecTotal;		/* 実行トータル時間(us) */
DWORD dwDrawTotal;		/* 描画トータル回数 */
DWORD dwSoundTotal;		/* サウンドトータル時間 */
DWORD uTimerResolution;	/* タイマー精度 */
BOOL  bTapeFullSpeed;		/* テープ高速モードフラグ */
BOOL  bFullSpeed;		/* 全力駆動フラグ */
BOOL  bAutoSpeedAdjust;		/* 速度自動調整フラグ */
DWORD dwNowTime;		/* timeGetTimeの値 */
BOOL  bTapeModeType;		/* テープ高速モードタイプ */
BOOL  bHiresTick;               /* Hi resolution 1ms timer */
DWORD nTickResUs;                /* Wait value for Hi-Resolution tick */
   
/*
 *  スタティック ワーク
 */
static DWORD    dwThResult;	/* スレッド戻り値 */
static int     nPriority;	/* スレッド実行優先順位 */
static BOOL    bDrawVsync;	/* VSYNCフラグ(描画用) */
static BOOL    bPollVsync;	/* VSYNCフラグ(ポーリング用) */
static DWORD   dwExecTime;	/* 実行時間(ms) */
static int     nFrameSkip;	/* フレームスキップ数(ms) */
static BOOL    bRunningBak;	/* 実行フラグ backup */
static BOOL    bFastMode;	/* 高速実行中フラグ backup */
static DWORD  nSpeedCheck;	/* 速度調整用カウンタ(ms) */
static DWORD   dwChkTime;	/* 速度調整基準時間(ms) */
static DWORD   dwSleepCount;	/* スリープ回数 */
   
BOOL bFullScreen = FALSE;

/*
 *  プロトタイプ宣言
 */
static void *ThreadSch(void *);	/* スレッド関数 */
static void *Thread1ms(void *);
DWORD XM7_timeGetTime(void);	/* timeGetTime互換関数 */
void  XM7_Sleep(DWORD t);	/* Sleep互換関数 */
void  XM7_Sync1ms(DWORD init);	/* 1msピリオドで待つ */

static AG_Thread SchThread;
static AG_TimeOps SchTimeOps;

/*
 *  初期化
 */
void  InitSch(void)
{

	/*
	 * ワークエリア初期化
	 */
	dwThResult = 0;
	bDrawVsync = TRUE;
	bPollVsync = TRUE;
	bRunningBak = FALSE;
	bFastMode = FALSE;

	/*
	 * グローバルワーク
	 */
	dwExecTotal = 0;
	dwDrawTotal = 0;
	dwSoundTotal = 0;
	dwNowTime = 0;
	bTapeFullSpeed = FALSE;
	bFullSpeed = FALSE;
	bAutoSpeedAdjust = FALSE;
	uTimerResolution = 1;
	bTapeModeType = FALSE;
        return;
}


/*
 *  クリーンアップ
 */
void CleanSch(void)
{
	int ret;
	bCloseReq = TRUE; // 終了要求
        SDL_Delay(10);
//	AG_ThreadJoin(&SchThread,(void *)&ret); // スケジューラが終わるのを待つ
//	AG_ThreadCancel(SchThread); // スケジューラが終わるのを待つ
        bCloseReq = FALSE;
}
/*
 *  セレクト
 */
BOOL SelectSch(void)
{

	/*
	 * スレッド生成
	 */
	AG_ThreadCreate(&SchThread, &ThreadSch, NULL);
	return TRUE;
}


/*
 *  VSYNC通知
 */
void vsync_notify(void)
{
	bDrawVsync = TRUE;
	bPollVsync = TRUE;
}
   
/*
 *  1ms実行
 */

void ExecSch(void)
{
	int dwCount;
	int dwExec;

	/*
	 * ポーリング
	 */
	PollKbd();
	if (bPollVsync) {
		PollJoy();

#ifdef MOUSE
//		PollMos();

#endif				/*  */
		bPollVsync = FALSE;
	}

	/*
	 * サウンド
	 */
	ProcessSnd(FALSE);
	dwCount = 1000;
	while (dwCount > 0) {

		/*
		 * 中止要求が上がっていれば、即座にリターン
		 */
		if (stopreq_flag) {
			run_flag = FALSE;
			break;
		}

		/*
		 * ここで実行
		 */
		dwExec = schedule_exec(dwCount);
		dwCount -= dwExec;
		dwSoundTotal += dwExec;
	}

	/*
	 * トータルタイム増加
	 */
	dwExecTotal += (1000 - dwCount);
}


/*
 *  描画
 */
static void  DrawSch(void)
{
	OnDraw();
	DrawStatus();

	/*
	 * カウンタアップ
	 */
	dwDrawTotal++;
}
/*
 *  実行リセット
 * ※VMのロックは行っていないので注意
 */
void ResetSch(void)
{
	nFrameSkip = 0;
	dwExecTime = XM7_timeGetTime();
}
/*
 *  速度調整リセット
 */
void ResetSpeedAdjuster(void)
{
	nSpeedCheck = 0;
	dwSleepCount = 0;
	dwChkTime = XM7_timeGetTime();
}

/*
 *  スレッド関数
 */
static void *ThreadSch(void *param)
{
	DWORD dwTempTime;
	BOOL fast_mode;
	int tmp;
	int retval;

	/*
	 * 初期化
	 */
	ResetSch();
	ResetSpeedAdjuster();
        XM7_DebugLog(XM7_LOG_INFO, "Scheduler Started.");
	/*
	 * SDLイベントハンドラ登録
	 */

	/*
	 * 無限ループ(クローズ指示があれば終了)
	 */
	while (!bCloseReq) {

		/*
		 * いきなりロック
		 */
		LockVM();

		/*
		 * 実行指示が変化したかチェック
		 */
		if (__builtin_expect((bRunningBak != run_flag), 0)) {
			bRunningBak = run_flag;

#ifdef ROMEO
			/*
			 * YMF288をミュート
			 */
			ROMEO_Mute(!run_flag);

#endif				/*  */
		}

		/*
		 * 実行指示がなければ、スリープ
		 */
		if (__builtin_expect((!run_flag), 0)) {

			/*
			 * 無音を作ってスリープ
			 */
			ProcessSnd(TRUE);
			UnlockVM();
		        
		        XM7_Sleep(10);
			ResetSch();
			ResetSpeedAdjuster();
			continue;
		}

		/*
		 * リセット時はカウンタ類を初期化
		 */
		if (reset_flag) {
			ResetSpeedAdjuster();
			reset_flag = FALSE;
		}

		/*
		 * 時間を取得(49日でのループを考慮)
		 */
		dwNowTime = XM7_timeGetTime();
		dwTempTime = dwNowTime;
		if (dwTempTime < dwExecTime) {
			dwExecTime = 0;
		}

		/*
		 * 時間を比較
		 */
		if (dwTempTime <= dwExecTime) {

			/*
			 * 時間が余っているが、描画できるか
			 */
			if (bDrawVsync) {
				DrawSch();
				nFrameSkip = 0;
				bDrawVsync = FALSE;
			}

			/*
			 * 再度、時間を取得(49日でのループを考慮)
			 */
			dwNowTime = XM7_timeGetTime();
			dwTempTime = dwNowTime;
			if (dwTempTime < dwExecTime) {
				dwExecTime = 0;
			}
			if (dwTempTime > dwExecTime) {
				UnlockVM();
				continue;
			}

			/*
			 * 時間に余裕があるので、テープ高速モード判定
			 */
			if (__builtin_expect(((!tape_motor || !bTapeFullSpeed) || !bTapeModeType), 0)) {
				dwSleepCount++;
				if (bFullSpeed
						|| (tape_motor && bTapeFullSpeed && !bTapeModeType)) {

					/*
					 * 全力駆動モード
					 * あまった時間もCPUを動かす
					 */
					UnlockVM();
					while (!stopreq_flag) {
						if (dwTempTime != XM7_timeGetTime()) {
							break;
						}

						/*
						 * テープ高速モード時はメインのみ全力駆動
						 */
						if (tape_motor && bTapeFullSpeed
								&& !bFullSpeed) {
							schedule_main_fullspeed();
						}

						else {
							schedule_fullspeed();
						}
					}
					continue;
				}
				else {
				   UnlockVM();
				   XM7_Sync1ms(dwNowTime);
				   continue;
				}
			}
		   
			/*
			 * テープ高速モード
			 */
			dwExecTime = dwTempTime - 1;
			if (dwExecTime > dwTempTime) {
				dwExecTime++;
			}
		}

		/*
		 * 実行
		 */
		ExecSch();
		nFrameSkip++;
		nSpeedCheck++;
		dwExecTime++;

		/*
		 * 自動速度調整
		 */
		if (__builtin_expect((nSpeedCheck >= 200), 0)) {
			if (bAutoSpeedAdjust) {

				/*
				 * 時間を取得(49日でのループを考慮)
				 */
				dwTempTime = XM7_timeGetTime();
				if (dwTempTime < dwChkTime) {
					dwChkTime = 0;
				}

				/*
				 * 速度調整間隔+スリープ時間と実際の実行時間の比率を求める
				 */
				speed_ratio =
						(nSpeedCheck + dwSleepCount) * speed_ratio;
				speed_ratio /= (dwTempTime - dwChkTime);

				/*
				 * CPU速度比率を100%から5%の間に制限する
				 */
				if (speed_ratio > 10000) {
					speed_ratio = 10000;
				}

				else if (speed_ratio < 500) {
					speed_ratio = 500;
				}
			}
			else {

				/*
				 * 速度調整無効時は100%固定
				 */
				speed_ratio = 10000;
			}

			/*
			 * カウンタ類を初期化
			 */
			ResetSpeedAdjuster();
		}

		/*
		 * 終了対策で、ここで抜ける
		 */
		if (bCloseReq) {
			UnlockVM();
			break;
		}

		/*
		 * Break対策
		 */
		if (__builtin_expect((!run_flag), 0)) {
			DrawSch();
			bDrawVsync = FALSE;
			nFrameSkip = 0;
			UnlockVM();
			continue;
		}

		/*
		 * スキップカウンタが規定値以下なら、続けて実行
		 */
		if (__builtin_expect((bAutoSpeedAdjust), 1)) {
			tmp = (10000 - speed_ratio) / 10;

			/*
			 * 2fps?15fps/30fpsの間に制限
			 */
#if XM7_VER >= 3
			if (screen_mode & SCR_ANALOG) {

#else				/*  */
				if (mode320) {

#endif				/*  */
					/*
					 * 4096色/26万色モードでは最高75fps
					 */
					if (tmp < 13) {
						tmp = 13;
					}
				}

				else {

					/*
					 * 8色モードでは最高75fps
					 */
					if (tmp < 13) {
						tmp = 13;
					}
				}
				if (tmp > 500) {
					tmp = 500;
				}
			}
			else {
				tmp = 500;
			}
			if (nFrameSkip >= tmp) {

				/*
				 * 無描画が続いているので、ここで一回描画
				 */
				DrawSch();
				ResetSch();
				bDrawVsync = FALSE;
			}
			UnlockVM();
		}
	   

		/*
		 * 終了を明示するため、要求フラグを降ろす
		 */
		bCloseReq = FALSE;
//		AG_QuitGUI();
//	        XM7_DebugLog(XM7_LOG_INFO, "Scheduler quits gui.");
		retval = 0;
	        XM7_DebugLog(XM7_LOG_INFO, "Scheduler end.");
		AG_ThreadExit((void *)&retval);
}


	/*
	 *  timeGetTime互換関数
	 */
DWORD XM7_timeGetTime(void)
{
  //   return (DWORD)AG_GetTicks();
   return (DWORD)SDL_GetTicks();
}


/*
 *  Sleep互換関数
 */
void XM7_Sleep(DWORD t)
{
  //   AG_Delay(t);
   SDL_Delay(t);
}

void XM7_Sync1ms(DWORD init)
{
#ifdef HAVE_NANOSLEEP
   struct timespec req, remain;
   int tick;
#endif   
   if(!bHiresTick) { // Wait 1ms, not use nanosleep, but reduce cpu usage.
	XM7_Sleep(1);
        return;
   }
#ifdef HAVE_NANOSLEEP   
   tick = nTickResUs;
   if(tick < 20) tick = 20; // uSec
   if(tick > 500) tick = 500; // uSec
   req.tv_sec = 0;
   req.tv_nsec = tick * 1000; 
   do {
      if(__builtin_expect((XM7_timeGetTime() != init), 0)) break;
      nanosleep(&req, &remain); // Okay, per tick uS.
   } while(1);
#endif
}
   
#ifdef __cplusplus
}
#endif

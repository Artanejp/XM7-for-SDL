/*
 *  FM-7 EMULATOR "XM7"
 * Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2003 Ryu Takegami
 * Copyright (C) 2004 GIMONS 
 * Copyright (C) 2010 K.Ohta  [XWIN スケジューラ / SDL ] 
 */  
    
#ifdef _XWIN
    
#include<gtk/gtk.h>
#include "xm7.h"
#include "tapelp.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "mouse.h"
#include "sdl.h"
#include "sdl_bar.h"
#include "sdl_sch.h"
#include "sdl_kbd.h"
#include "sdl_draw.h"
    
    /*
     *  グローバル ワーク 
     */ 
    DWORD dwExecTotal;		/* 実行トータル時間(us) */
DWORD dwDrawTotal;		/* 描画トータル回数 */
DWORD dwSoundTotal;		/* サウンドトータル時間 */
DWORD uTimerResolution;	/* タイマー精度 */
BOOL bTapeFullSpeed;		/* テープ高速モードフラグ */
BOOL bFullSpeed;		/* 全力駆動フラグ */
BOOL bAutoSpeedAdjust;		/* 速度自動調整フラグ */
DWORD dwNowTime;		/* timeGetTimeの値 */
BOOL bTapeModeType;		/* テープ高速モードタイプ */

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
static void    *ThreadSch(void *);	/* スレッド関数 */
static DWORD FASTCALL timeGetTime(void);	/* timeGetTime互換関数 
						 */
static void FASTCALL Sleep(DWORD t);	/* Sleep互換関数 */

    /*
     *  初期化 
     */ 
void            FASTCALL
InitSch(void) 
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
void            FASTCALL
CleanSch(void) 
{
} 
    /*
     *  セレクト 
     */ 
    BOOL FASTCALL SelectSch(void) 
{
    GError * error = NULL;
    
	/*
	 * スレッド生成 
	 */ 
	if (!g_thread_create(ThreadSch, NULL, FALSE, &error))
	 {
	return FALSE;
	}
    return TRUE;
}


    /*
     *  VSYNC通知 
     */ 
void            FASTCALL
vsync_notify(void) 
{
    bDrawVsync = TRUE;
    bPollVsync = TRUE;
} 
    /*
     *  1ms実行 
     */ 
void            FASTCALL
ExecSch(void) 
{
    DWORD dwCount;
    DWORD dwExec;
    
#if 1
	SDL_Event eventQueue;
    
	/*
	 * SDL POLLING 
	 */ 
	while (SDL_PollEvent(&eventQueue))
	 {
	switch (eventQueue.type)
	     {
	case SDL_KEYDOWN:	/* キーボードはSDL */
	    OnKeyPress(&eventQueue);
	    break;
	case SDL_KEYUP:	/* キーボードはSDL */
	    OnKeyRelease(&eventQueue);
	    break;
	case SDL_JOYAXISMOTION:	/* JS動く */
	    OnMoveJoy(&eventQueue);
	    break;
	case SDL_JOYBUTTONDOWN:
	    OnPressJoy(&eventQueue);
	    break;
	case SDL_JOYBUTTONUP:
	    OnReleaseJoy(&eventQueue);
	    break;
	default:
	    break;
	    }
	}
    
#endif				/*  */
	
	/*
	 * ポーリング 
	 */ 
	PollKbd();
    if (bPollVsync) {
	PollJoy();
	
#ifdef MOUSE
	    PollMos();
	
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
static void     FASTCALL
DrawSch(void) 
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
void            FASTCALL
ResetSch(void) 
{
    nFrameSkip = 0;
    dwExecTime = timeGetTime();
} 
    /*
     *  速度調整リセット 
     */ 
void            FASTCALL
ResetSpeedAdjuster(void) 
{
    nSpeedCheck = 0;
    dwSleepCount = 0;
    dwChkTime = timeGetTime();
} 
    /*
     *  スレッド関数 
     */ 
static void    *
ThreadSch(void *param) 
{
    DWORD dwTempTime;
    BOOL fast_mode;
    int            tmp;
    
	/*
	 * 初期化 
	 */ 
	ResetSch();
    ResetSpeedAdjuster();
    
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
	    if (bRunningBak != run_flag) {
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
	    if (!run_flag) {
	    
		/*
		 * 無音を作ってスリープ 
		 */ 
		ProcessSnd(TRUE);
	    UnlockVM();
	    
		// Sleep(10);
		SDL_Delay(10);
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
	    dwNowTime = timeGetTime();
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
		dwNowTime = timeGetTime();
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
		if ((!tape_motor || !bTapeFullSpeed) || !bTapeModeType) {
		dwSleepCount++;
		if (bFullSpeed
		     || (tape_motor && bTapeFullSpeed && !bTapeModeType)) {
		    
			/*
			 * 全力駆動モード
			 * あまった時間もCPUを動かす 
			 */ 
			UnlockVM();
		    while (!stopreq_flag) {
			if (dwTempTime != timeGetTime()) {
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
		    SDL_Delay(1);
		    UnlockVM();
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
	    if (nSpeedCheck >= 200) {
	    if (bAutoSpeedAdjust) {
		
		    /*
		     * 時間を取得(49日でのループを考慮) 
		     */ 
		    dwTempTime = timeGetTime();
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
	    if (!run_flag) {
	    DrawSch();
	    bDrawVsync = FALSE;
	    nFrameSkip = 0;
	    UnlockVM();
	    continue;
	}
	
	    /*
	     * スキップカウンタが規定値以下なら、続けて実行 
	     */ 
	    if (bAutoSpeedAdjust) {
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
		     * 4096色/26万色モードでは最高15fps 
		     */ 
		    if (tmp < 66) {
		    tmp = 66;
		}
	    }
	    
	    else {
		
		    /*
		     * 8色モードでは最高30fps 
		     */ 
		    if (tmp < 33) {
		    tmp = 33;
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
}


    /*
     *  timeGetTime互換関数 
     */ 
static DWORD    FASTCALL
timeGetTime(void)
{
    
	// struct timeval t;
	// gettimeofday(&t, 0);
	// return (t.tv_sec*1000000 + t.tv_usec)/1000;
	return (DWORD) SDL_GetTicks();
}


    /*
     *  Sleep互換関数 
     */ 
static void     FASTCALL
Sleep(DWORD t)
{
    usleep(t * 1000);
} 
#endif	/* _XWIN */

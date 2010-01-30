/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN スケジューラ ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_sch_h_
#define _xw_sch_h_
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
    
	/*
	 *  主要エントリ 
	 */ 
    void FASTCALL   InitSch(void);
                   
	/*
	 * 初期化 
	 */            
    void FASTCALL   CleanSch(void);
                   
	/*
	 * クリーンアップ 
	 */            
                    BOOL FASTCALL SelectSch(void);
                   
	/*
	 * セレクト 
	 */            
    void FASTCALL   ResetSch(void);
                   
	/*
	 * 実行リセット 
	 */            
                   
	/*
	 *  主要ワーク 
	 */            
    extern DWORD    dwExecTotal;
                   
	/*
	 * 実行時間トータル 
	 */            
    extern DWORD    dwDrawTotal;
                   
	/*
	 * 描画回数トータル 
	 */            
    extern DWORD    dwSoundTotal;
                   
	/*
	 * サウンド時間トータル 
	 */            
    extern DWORD    uTimerResolution;
                   
	/*
	 * タイマー精度 
	 */            
    extern BOOL     bTapeFullSpeed;
                   
	/*
	 * テープ高速モード 
	 */            
    extern BOOL     bFullSpeed;
                   
	/*
	 * 全力駆動 
	 */            
    extern BOOL     bAutoSpeedAdjust;
                   
	/*
	 * 速度自動調整フラグ 
	 */            
    extern DWORD    dwNowTime;
                   
	/*
	 * timeGetTimeの値 
	 */            
    extern BOOL     bTapeModeType;
                   
	/*
	 * テープ高速モードタイプ 
	 */            
    extern BYTE     nTimePeriod;
                   
	/*
	 * タイマ精度設定指示 
	 */            
#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_sch_h_ */
#endif	/* _XWIN */

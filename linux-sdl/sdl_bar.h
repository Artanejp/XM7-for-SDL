/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN コントロールバー
 * ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_bar_h_
#define _xw_bar_h_
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */

        /*
         * Global変数
         */
        extern char StatusFont[MAXPATHLEN];
	/*
	 *  主要エントリ 
	 */ 
        extern void CreateStatus(void);
               
	/*
	 * ステータスバーの生成 
	 */            
        extern void DrawStatus(void);
                   
	/*
	 * 描画 
	 */            
        extern void  PaintStatus(void);
                   
	/*
	 * 再描画 
	 */            
#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_bar_h_ */
#endif	/* _XWIN */

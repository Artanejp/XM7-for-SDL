/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN
 * コンフィギュレーション ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_cmd_h_
#define _xw_cmd_h_
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
    
	/*
	 *  定数、型定義 
	 */ 
#define stricmp	strcasecmp
#define COPI	"Copyright(C) 1999-2003 ＰＩ．"
#define HTPI	"http://www.ipc-tokai.or.jp/~ytanaka/"
#define CORYU	"Copyright(C) 2001-2003 Ryu Takegami"
#define HTRYU	"http://retropc.net/~ryu/xm7/"
#define COFMGEN	"FM Sound Generator. PSG Implementation\nTechnology from M88, 1998-2003 by cisc"
#define	COGIMO	"Copyright(C) 2004 GIMONS"
#define	HTGIMO	"http://www.geocities.jp/kugimoto0715/"
#define VERSTR "FM-7 EMULATOR XM7\n"VERSION" "LEVEL"（"DATE"）\nFor Linux GTK"
#define AUTSTR COPI"\n"HTPI"\n\n"CORYU"\n"HTRYU"\n\n"COGIMO"\n"HTGIMO"\n\n"COFMGEN
	typedef struct {
	int            drive;
	               int media;
                   } Disk;	/* D77ディスクメディア */
                  
	/*
	 *  主要エント 
	 */            
    void FASTCALL   CreateMenu(GtkWidget * parent);	/* メニュー生成 
							 */

    void FASTCALL   OnDropFiles(void);	/* ファイルドロップ */
                   void FASTCALL OnCmdLine(char *arg);	/* コマンドライン処理 
							 */
                  
#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_cmd_h_ */
#endif	/* _XWIN */

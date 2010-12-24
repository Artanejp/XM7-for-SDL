/*
 *   FM-7 EMULATOR "XM7"
 *   Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp) 
 *   Copyright (C) 2001-2003 Ryu Takegami 
 *   Copyright (C) 2004 GIMONS
 *   Copyright (C) 2010 K.Ohta (whatisthis.sowhat@gmail.com) 
 *   [SDL Toolbox ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_cmd_h_
#define _xw_cmd_h_
    
#ifdef __cplusplus
extern          "C" {
    
#endif				/*  */
    
	/*
	 *  定数、型定義 
	 */ 
#define stricmp	strcasecmp
#define COPI	"Copyright(C) 1999-2003 ＰＩ．"
#define HTPI	"http://www.ipc-tokai.or.jp/~ytanaka/"
#define CORYU	"Copyright(C) 2001-2003 Ryu Takegami"
#define HTRYU	"http://retropc.net/~ryu/xm7/"
#define COFMGEN "FM Sound Generator. PSG Implementation\nTechnology from M88, 1998-2003 by cisc"
#define COGIMO	"Copyright(C) 2004 GIMONS"
#define HTGIMO	"http://www.geocities.jp/kugimoto0715/"
#define COART   "Copyright(C) 2010 K.Ohta porting to SDL & AGAR."
#define HTART   "http://sky.geocities.jp/artanejp/"
#ifdef LOCALVER
#define VERSTR "For SDL/AGAR\n"VERSION" "LEVEL"\n（"LOCALVER"/"DATE"）\n"
#else
#define VERSTR "FM-7 EMULATOR XM7\n"VERSION" "LEVEL"（"DATE"）\n"
#endif
#define AUTSTR COPI"\n"HTPI"\n\n"CORYU"\n"HTRYU"\n\n"COGIMO"\n"HTGIMO"\n\n"COART"\n"HTART"\n\n"COFMGEN
typedef struct {
        int drive;
        int media;
} Disk;	/* D77ディスクメディア */
                  
/*
 *  主要エントリ 
 */            

        extern void OnDropFiles(void);	/* ファイルドロップ */
        extern void OnCmdLine(char *arg);	/* コマンドライン処理 
							 */
        extern void OnExec(void);
        extern void OnBreak(void);
        extern void OnReset(AG_Event *);
        extern void OnHotReset(AG_Event *);
        extern void OnBasic(AG_Event *);
        extern void OnDos(AG_Event *);
        extern void OnExit(AG_Event *);
        extern void    OnDiskOpen(AG_Event *);
        extern void    OnDiskEject(AG_Event *);
        extern void    OnDiskTemp(AG_Event *);
        extern void    OnDiskProtect(AG_Event *);
        extern void    OnMediaChange(AG_Event *);

        extern void    OnRec(AG_Event *);
        extern void    OnRew(AG_Event *);
        extern void    OnFF(AG_Event *);
        extern void    OnTapeEject(AG_Event *);
        extern void    OnMouseMode(AG_Event *);
        extern void    OnTimeAdjust(AG_Event *);
        extern void    OnGrpCapture(AG_Event *);
        extern void    OnGrpCapture2(AG_Event *);
        extern void    OnWavCapture(AG_Event *);
        extern void    OnChgSound(AG_Event *event);

#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_cmd_h_ */
#endif	/* _XWIN */

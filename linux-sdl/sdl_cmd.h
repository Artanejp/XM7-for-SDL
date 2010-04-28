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
#define COART   "Copyright(C) 2010 K.Ohta porting to SDL."
#define HTART   "http://sky.geocities.jp/artanejp/"
#ifdef LOCALVER
#define VERSTR "FM-7 EMULATOR XM7 For SDL\n"VERSION" "LEVEL"（"LOCALVER"/"DATE"）\n"
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


#ifdef USE_GTK

        extern void    OnExit(GtkWidget * widget, gpointer data);
        extern void    OnDiskOpen(GtkWidget * widget, gpointer data);
        extern void    OnReset(GtkWidget * widget, gpointer data); 
        extern void    OnHotReset(GtkWidget * widget, gpointer data); 
        extern void    OnBasic(GtkWidget * widget, gpointer data); 
        extern void    OnDos(GtkWidget * widget, gpointer data); 
        extern void    OnDiskEject(GtkWidget * widget, gpointer data); 
        extern void    OnDiskTemp(GtkWidget * widget, gpointer data);
        extern void    OnDiskProtect(GtkWidget * widget, gpointer data);
        extern void    OnMediaChange(GtkWidget * widget, gpointer data);

        extern void    OnRec(GtkWidget * widget, gpointer data);
        extern void    OnRew(GtkWidget * widget, gpointer data);
        extern void    OnFF(GtkWidget * widget, gpointer data);
        extern void    OnTapeEject(GtkWidget * widget, gpointer data);

        extern void    OnMouseMode(GtkWidget * widget, gpointer data);
        extern void    OnTimeAdjust(GtkWidget * widget, gpointer data);
        extern void    OnGrpCapture(GtkWidget * widget, gpointer data);
        extern void    OnGrpCapture2(GtkWidget * widget, gpointer data);
        extern void    OnWavCapture(GtkWidget * widget, gpointer data);
#endif

#ifdef __cplusplus
}              
#endif				/*  */
               
#endif	/* _xw_cmd_h_ */
#endif	/* _XWIN */

/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN 各種ダイアログ ] 
 */  
    
#ifdef _XWIN
    
#ifndef _xw_dlg_h_
#define _xw_dlg_h_
    
    /*
     *  定数、型定義 
     */ 
#define DLG_OK 1					/* ダイアログＯＫ */
#define DLG_CANCEL 2				/* ダイアログＣＡＮＣＥＬ */
#define DLG_NONE 0					/* ダイアログ未確定 */
    typedef struct {
GtkWidget * dlg;
    char           sFilename[256];
                   BYTE bResult;
               } FileSelectDialog;	/* ファイルセレクターダイアログ 
					 */
typedef struct {
    GtkWidget * dlg;
    GtkWidget * entTitle;
#if XM7_VER >= 3
    GtkWidget      *radMediaType2D;
                   GtkWidget * radMediaType2DD;
#endif				/*  */
    GtkWidget      *chkUserDisk;
                   char sTitle[256];
#if XM7_VER >= 3
    BOOL            b2DDDisk;
#endif				/*  */
    BOOL            bUserDisk;
                   BOOL bResult;
               } DiskImageDialog;	/* ディスクイメージダイアログ 
					 */
typedef struct {
    GtkWidget * dlg;
    GtkWidget * entTitle;
    char           sTitle[256];
                   BOOL bResult;
               } DiskTitleDialog;	/* ディスクタイトルダイアログ 
					 */

    /*
     *  主要エントリ 
     */ 
void            OpenErrorMessageDialog(char *strTitle, char *strMessage);
												/*
												 * エラーメッセージダイアログ 
												 */ 
    FileSelectDialog OpenFileSelectDialog(char *dir);
												/*
												 * ファイル選択ダイアログ 
												 */ 
    DiskImageDialog OpenDiskImageDialog(void);	/* ディスクイメージダイアログ 
						 */
DiskTitleDialog OpenDiskTitleDialog(void);	/* ディスクタイトルダイアログ 
						 */

#endif	/* _xw_dlg_h_ */
#endif	/* _XWIN */

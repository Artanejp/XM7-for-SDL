/*
 *  FM-7 EMULATOR "XM7"
 *  Copyright (C) 1999-2003 ＰＩ．(ytanaka@ipc-tokai.or.jp)
 *  Copyright (C) 2001-2010 Ryu Takegami
 *  Copyright (C) 2004 GIMONS
 *  Copyright (C) 2010 K.Ohta  
 *[ GTK メニューコマンド] 
 */  
    
    
#include <gtk/gtk.h>
#include "xm7.h"
#include "fdc.h"
#include "tapelp.h"
#include "tools.h"
#include "mouse.h"
#include "rtc.h"
#include "sdl.h"
#include "sdl_cmd.h"
#include "sdl_cfg.h"
#include "sdl_gtkdlg.h"
#include "sdl_prop.h"
#include "sdl_snd.h"
#include "sdl_sch.h"
#include "sdl_kbd.h"
#ifdef USE_GTK
#include "gtk_toolbox.h"
#endif
    /*
     *  グローバル ワーク 
     */ 
char            InitialDir[5][MAXPATHLEN];

    /*
     *  スタティック ワーク 
     */ 


/*-[ ファイルメニュー ]-----------------------------------------------------*/ 
    
/*
 *  リセット(R) 
 */ 
void
OnReset(GtkWidget * widget, gpointer data) 
{
    LockVM();
    system_reset();
    UnlockVM();
    
	/*
	 * 再描画 
	 */ 
	// OnRefresh();
} 
/*
 *  ホットリセット(H) 
 */ 
void
OnHotReset(GtkWidget * widget, gpointer data) 
{
    LockVM();
    system_hotreset();
    UnlockVM();
    
	/*
	 * 再描画 
	 */ 
	// OnRefresh();
} 
    /*
     *  BASICモード(B) 
     */ 
void
OnBasic(GtkWidget * widget, gpointer data) 
{

    LockVM();
    boot_mode = BOOT_BASIC;
    if (fm7_ver < 2) {
	mainmem_transfer_boot();
    }
    
    else {
	system_reset();
    }
    UnlockVM();
}


    /*
     *  DOSモード(D) 
     */ 
void
OnDos(GtkWidget * widget, gpointer data) 
{

    LockVM();
    boot_mode = BOOT_DOS;
    if (fm7_ver < 2) {
	mainmem_transfer_boot();
    }
    
    else {
	system_reset();
    }
    UnlockVM();
}


    /*
     *  終了(X) 
     */ 
void
OnExit(GtkWidget * widget, gpointer data) 
{
/*
 * フラグアップ 
 */ 
    LockVM();
    bCloseReq = TRUE;
    UnlockVM();
    //gtk_widget_destroy(GTK_WIDGET(data));
    gtk_widget_destroy(GTK_WIDGET(data));
} 



/*
 *  ディスクイジェクト 
 */ 
void
OnDiskEject(GtkWidget * widget, gpointer data) 
{
    int            Drive = ((Disk *) data)->drive;
    
	/*
	 * イジェクト 
	 */ 
	LockVM();
    fdc_setdisk(Drive, NULL);
    UnlockVM();
} 
    /*
     *  ディスク一時取り出し 
     */ 
void
OnDiskTemp(GtkWidget * widget, gpointer data) 
{
    int            Drive = ((Disk *) data)->drive;
    
	/*
	 * 書き込み禁止切り替え 
	 */ 
    LockVM();
    if (fdc_teject[Drive]) {
	fdc_teject[Drive] = FALSE;
    }
    
    else {
	fdc_teject[Drive] = TRUE;
    }
    UnlockVM();
}


/*
 *  ディスク書き込み禁止 
 */ 
void
OnDiskProtect(GtkWidget * widget, gpointer data) 
{
    int            Drive = ((Disk *) data)->drive;
    
	/*
	 * 書き込み禁止切り替え 
	 */ 
	LockVM();
    if (fdc_writep[Drive]) {
	fdc_setwritep(Drive, FALSE);
    }
    
    else {
	fdc_setwritep(Drive, TRUE);
    }
    ResetSch();
    UnlockVM();
}


/*
 *  メディア切り替え 
 */ 
void
OnMediaChange(GtkWidget * widget, gpointer data) 
{
    int            Drive = ((Disk *) data)->drive;
    int            Media = ((Disk *) data)->media;
    
	/*
	 * 書き込み禁止切り替え 
	 */ 
    LockVM();
    fdc_setmedia(Drive, Media);
    ResetSch();
    UnlockVM();
} 



/*
 *  テープイジェクト 
 */ 
void
OnTapeEject(GtkWidget * widget, gpointer data) 
{
    
/*
 * イジェクト 
 */ 
        LockVM();
        tape_setfile(NULL);
        UnlockVM();
} 
/*
 *  巻き戻し 
 */ 
void
OnRew(GtkWidget * widget, gpointer data) 
{
    
/*
 * 巻き戻し 
 */ 
        LockVM();
        StopSnd();
        tape_rew();
        PlaySnd();
        ResetSch();
        UnlockVM();
} 
/*
 *  早送り 
 */ 
void
OnFF(GtkWidget * widget, gpointer data) 
{
    
/*
 * 巻き戻し 
 */ 
    LockVM();
    StopSnd();
    tape_ff();
    PlaySnd();
    ResetSch();
    UnlockVM();
} 

/*
 *  録音 
 */ 
void
OnRec(GtkWidget * widget, gpointer data) 
{
    
/*
 * 録音 
 */ 
        LockVM();
        if (tape_rec) {
                tape_setrec(FALSE);
        } else {
                tape_setrec(TRUE);
        }
        UnlockVM();
}



    /*
     *  実行(X) 
     */ 
void
OnExec(void) 
{
    
/*
 * 既に実行中なら、何もしない 
 */ 
        if (run_flag) {
                return;
        }
/*
 * スタート 
 */ 
        LockVM();
        stopreq_flag = FALSE;
        run_flag = TRUE;
        UnlockVM();
}


/*
 *  停止(B) 
 */ 
void
OnBreak(void) 
{
        /*
         * 既に停止状態なら、何もしない 
         */ 
        if (!run_flag) {
                return;
        }
    
/*
 * 停止 
 */ 
        LockVM();
        stopreq_flag = TRUE;
        UnlockVM();
}




    /*
     *  時刻アジャスト 
     */ 
void
OnTimeAdjust(GtkWidget * widget, gpointer data) 
{
    
	/*
	 * 時刻を再設定する 
	 */ 
	rtc_time_adjust();
    
	/*
	 * 念のためスケジュールを初期化 
	 */ 
	rtc_reset();
} 
/*
 *  マウスモード切り換え(M) 
 */ 
#ifdef MOUSE
void
OnMouseMode(GtkWidget * widget, gpointer data) 
{
    
/*
 * マウスキャプチャフラグを反転させてモード切り替え 
 */ 
        mos_capture = (!mos_capture);
        SetMouseCapture(bActivate);
} 
#endif				/*  */
    
/*
 *  画面キャプチャ(C) 
 */ 
void
OnGrpCapture(GtkWidget * widget, gpointer data) 
{
    char          *p;
    
	/*
	 * ファイル選択 
	 */ 
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[3]);
    if (dlg.bResult != DLG_OK) {
	return;
    }
    
	/*
	 * キャプチャ 
	 */ 
	LockVM();
    StopSnd();
    capture_to_bmp(dlg.sFilename, FALSE);
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(dlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[3], dlg.sFilename);
    }
}


/*
 *  画面キャプチャ2 
 */ 
void
OnGrpCapture2(GtkWidget * widget, gpointer data) 
{
    char          *p;
    
	/*
	 * ファイル選択 
	 */ 
	FileSelectDialog dlg = OpenFileSelectDialog(InitialDir[3]);
    if (dlg.bResult != DLG_OK) {
	return;
    }
    
	/*
	 * キャプチャ 
	 */ 
	LockVM();
    StopSnd();
    capture_to_bmp2(dlg.sFilename);
    PlaySnd();
    ResetSch();
    UnlockVM();
    p = strrchr(dlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[3], dlg.sFilename);
    }
}


    /*
     *  WAVキャプチャ(W) 
     */ 
void
OnWavCapture(GtkWidget * widget, gpointer data) 
{
    char          *p;
    FileSelectDialog dlg;
    
	/*
	 * 既にキャプチャ中なら、クローズ 
	 */ 
	if (hWavCapture >= 0) {
	LockVM();
	CloseCaptureSnd();
	UnlockVM();
	return;
    }
    
	/*
	 * ファイル選択 
	 */ 
	dlg = OpenFileSelectDialog(InitialDir[4]);
    if (dlg.bResult != DLG_OK) {
	return;
    }
    
	/*
	 * キャプチャ 
	 */ 
	LockVM();
    OpenCaptureSnd(dlg.sFilename);
    UnlockVM();
    
	/*
	 * 条件判定 
	 */ 
	if (hWavCapture < 0) {
	LockVM();
	StopSnd();
	PlaySnd();
	ResetSch();
	UnlockVM();
    }
    p = strrchr(dlg.sFilename, '/');
    if (p != NULL) {
	p[1] = '\0';
	strcpy(InitialDir[4], dlg.sFilename);
    }
}



    /*
     *  サウンド出力モード切り替え 
     */ 
static void
OnChgSound(GtkWidget * widget, gpointer data) 
{
    LockVM();
    
	/*
	 * サウンドモード変更 
	 */ 
	nStereoOut = (nStereoOut + 1) % 4;
    
	/*
	 * 適用 
	 */ 
	ApplySnd();
    UnlockVM();
} 


    /*
     *  ファイルドロップサブ 
     */ 
void
OnDropSub(char *path) 
{
    char           InitDir[MAXPATHLEN];
    char          *ext = NULL;
    char          *p = NULL;
    
    /*
     * 拡張子だけ分離 
     */ 
    p = strrchr(path, '.');
    if (p != NULL) {
	ext = p;
    }
    strcpy(InitDir, path);
    p = strrchr(InitDir, '/');
    if (p != NULL) {
	p[1] = '\0';
    } else {
	InitDir[0] = '\0';
    }
    if (ext != NULL) {
	
	    /*
	     * D77 
	     */ 
	    if (stricmp(ext, ".D77") == 0) {
	    strcpy(InitialDir[0], InitDir);
	    LockVM();
	    StopSnd();
	    fdc_setdisk(0, path);
	    fdc_setdisk(1, NULL);
	    if ((fdc_ready[0] != FDC_TYPE_NOTREADY)
		 && (fdc_medias[0] >= 2)) {
		fdc_setdisk(1, path);
		fdc_setmedia(1, 1);
	    }
	    system_reset();
	    PlaySnd();
	    ResetSch();
	    UnlockVM();
	}
	
	    /*
	     * 2D/VFD 
	     */ 
	    if ((stricmp(ext, ".2D") == 0) || (stricmp(ext, ".VFD") == 0)) {
	    strcpy(InitialDir[0], InitDir);
	    LockVM();
	    StopSnd();
	    fdc_setdisk(0, path);
	    fdc_setdisk(1, NULL);
	    system_reset();
	    PlaySnd();
	    ResetSch();
	    UnlockVM();
	}
	
	    /*
	     * T77 
	     */ 
	    if (stricmp(ext, ".T77") == 0) {
	    strcpy(InitialDir[1], InitDir);
	    LockVM();
	    tape_setfile(path);
	    UnlockVM();
	}
	
	    /*
	     * XM7 
	     */ 
	    if (stricmp(ext, ".XM7") == 0) {
	    strcpy(InitialDir[2], InitDir);
	    LockVM();
	    StopSnd();
	    StateLoad(path);
	    PlaySnd();
	    ResetSch();
	    UnlockVM();
	}
    }
}


/*
 *  ファイルドロップ 
 */ 
void
OnDropFiles(void) 
{
    char           path[MAXPATHLEN];
    
	/*
	 * 処理 
	 */ 
	OnDropSub(path);
} 
    /*
     *  コマンドライン処理 
     */ 
void
OnCmdLine(char *arg) 
{
/*
 * 処理 
 */ 
        OnDropSub(arg);
} 


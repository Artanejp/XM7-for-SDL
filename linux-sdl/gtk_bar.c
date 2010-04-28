/*
 *  FM-7 EMULATOR "XM7"  Copyright (C) 1999-2003
 * ＰＩ．(ytanaka@ipc-tokai.or.jp) Copyright (C) 2001-2003 Ryu
 * Takegami Copyright (C) 2004 GIMONS  [ XWIN コントロールバー
 * ] 
 */  
    
#ifdef _XWIN
    
#include <gtk/gtk.h>
#include "xm7.h"
#include "keyboard.h"
#include "tapelp.h"
#include "display.h"
#include "ttlpalet.h"
#include "apalet.h"
#include "subctrl.h"
#include "fdc.h"
#include "sdl.h"
#include "sdl_bar.h"
#include "sdl_sch.h"
    
    /*
     *  スタティック ワーク 
     */ 
static char     szCaption[128];	/* キャプション */
static int     nCAP;		/* CAPキー */
static int     nKANA;		/* かなキー */
static int     nINS;		/* INSキー */
static int     nDrive[2];	/* フロッピードライブ */
static char    szDrive[2][16 + 1];	/* フロッピードライブ */
static int     nTape;		/* テープ */

    /*
     * Status Bar Widhet 
     */ 
static GtkWidget *hbox3;
static GtkWidget *hbox4;
static GtkWidget *hbox5;
static GtkWidget *frmflp[2];
static GtkWidget *frmtape;
static GtkWidget *frmcaps;
static GtkWidget *frmkana;
static GtkWidget *frmins;
static GtkWidget *evtflp[2];
static GtkWidget *evttape;
static GtkWidget *evtcaps;
static GtkWidget *evtkana;
static GtkWidget *evtins;
static GtkWidget *lblstat;
static GtkWidget *lblflp[2];
static GtkWidget *lbltape;
static GtkWidget *lblcaps;
static GtkWidget *lblkana;
static GtkWidget *lblins;

    /*
     * Colors 
     */ 
static GdkColor colBLACK;	// = {0, 0, 0, 0};
static GdkColor colWHITE;	// = {1, 0xffff, 0xffff, 0xffff};
static GdkColor colRED;		// = {2, 0xffff, 0, 0};
static GdkColor colDRED;	// = {3, 0xbf00, 0, 0};
static GdkColor colDBLUE;	// = {4, 0, 0, 0xbf00};
static GdkColor colNORM;

/*-[ ステータスバー ]-------------------------------------------------------*/ 
    /*
     *  ステータスバーの生成 
     */ 
void            FASTCALL
CreateStatus(GtkWidget * parent) 
{
        GtkWidget *hbox_Status;
        GtkWidget *hbox_Files;
        GtkWidget *hbox4, *hbox5 , *hbox6;
//        hbox_Files = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_Sts_Files"));
//        hbox_Files = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_Sts_Status"));
        //hbox3 = GTK_WIDGET(gtk_builder_get_object(gbuilderMain, "hbox_Sts_Files"));

    colNORM = gtk_widget_get_style(parent)->bg[GTK_STATE_NORMAL];
    gdk_color_parse("black", &colBLACK);
    gdk_color_parse("white", &colWHITE);
    gdk_color_parse("red", &colRED);
    gdk_color_parse("red3", &colDRED);
    gdk_color_parse("blue3", &colDBLUE);
    
    /*
     * ステータスバーの土台となる水平ボックス 
     */ 

} 
    /*
     *  キャプション描画 
     */ 
static void     FASTCALL
DrawMainCaption(void) 
{
    char           string[256];
    char           tmp[128];
    char          *p;
    
	/*
	 * 動作状況に応じて、コピー 
	 */ 
    if (run_flag) {
            strcpy(string, "XM7[実行]");
    }
    
    else {
            strcpy(string, "XM7[停止]");
    }
    strcat(string, " ");
    
    /*
     * CPU速度比率 
     */ 
    if (bAutoSpeedAdjust) {
            sprintf(tmp, "(%3d%%) ", speed_ratio / 100);
            strcat(string, tmp);
    }
    
    /*
     * フロッピーディスクドライブ 0 
     */ 
    if (fdc_ready[0] != FDC_TYPE_NOTREADY) {
            
	    /*
	     * ファイルネーム＋拡張子のみ取り出す 
	     */ 
	    p = strrchr(fdc_fname[0], '/');
	if (p == NULL) {
	    p = fdc_fname[0];
	} else {
	    p++;
	}
	sprintf(tmp, "- %s ", p);
	strcat(string, tmp);
    }
    
	/*
	 * フロッピーディスクドライブ 1 
	 */ 
	if (fdc_ready[1] != FDC_TYPE_NOTREADY) {
	if ((strcmp(fdc_fname[0], fdc_fname[1]) != 0) || 
	     (fdc_ready[0] == FDC_TYPE_NOTREADY)) {
	    
		/*
		 * ファイルネーム＋拡張子のみ取り出す 
		 */ 
		p = strrchr(fdc_fname[1], '/');
	    if (p == NULL) {
		p = fdc_fname[1];
	    } else {
		p++;
	    }
	    sprintf(tmp, "(%s) ", p);
	    strcat(string, tmp);
	}
    }
    
	/*
	 * テープ 
	 */ 
	if (tape_fileh != -1) {
	
	    /*
	     * ファイルネーム＋拡張子のみ取り出す 
	     */ 
	    p = strrchr(tape_fname, '/');
	if (p == NULL) {
	    p = tape_fname;
	} else {
	    p++;
	}
	sprintf(tmp, "- %s ", p);
	strcat(string, tmp);
    }
    
	/*
	 * 比較描画 
	 */ 
	string[127] = '\0';
    if (memcmp(szCaption, string, strlen(string) + 1) != 0) {
	strcpy(szCaption, string);
	gdk_threads_enter();
	gtk_window_set_title(GTK_WINDOW(wndMain), szCaption);
	gdk_threads_leave();
    }
}


/*
 *  CAPキー描画 
 */ 
static void     FASTCALL
DrawCAP(void) 
{
    int            num;
    
    /*
     * 番号決定 
     */ 
    if (caps_flag) {
            num = 1;
    } else {
            num = 0;
    }
    
    /*
     * 同じなら何もしない 
     */ 
    if (nCAP == num) {
            return;
    }
    
    /*
     * 描画、ワーク更新 
     */ 
    nCAP = num;
    gdk_threads_enter();
    if (nCAP) {
	gtk_widget_modify_bg(evtcaps, GTK_STATE_NORMAL, &colRED);
    }
    
    else {
	gtk_widget_modify_bg(evtcaps, GTK_STATE_NORMAL, &colBLACK);
    }
    gdk_threads_leave();
}


    /*
     *  かなキー描画 
     */ 
static void     FASTCALL
DrawKANA(void) 
{
    int            num;
    
	/*
	 * 番号決定 
	 */ 
	if (kana_flag) {
	num = 1;
    }
    
    else {
	num = 0;
    }
    
	/*
	 * 同じなら何もしない 
	 */ 
	if (nKANA == num) {
	return;
    }
    
	/*
	 * 描画、ワーク更新 
	 */ 
	nKANA = num;
    gdk_threads_enter();
    if (nKANA) {
	gtk_widget_modify_bg(evtkana, GTK_STATE_NORMAL, &colRED);
    }
    
    else {
	gtk_widget_modify_bg(evtkana, GTK_STATE_NORMAL, &colBLACK);
    }
    gdk_threads_leave();
}


    /*
     *  INSキー描画 
     */ 
static void     FASTCALL
DrawINS(void) 
{
    int            num;
    
	/*
	 * 番号決定 
	 */ 
	if (ins_flag) {
	num = 1;
    }
    
    else {
	num = 0;
    }
    
	/*
	 * 同じなら何もしない 
	 */ 
	if (nINS == num) {
	return;
    }
    
	/*
	 * 描画、ワーク更新 
	 */ 
	nINS = num;
    gdk_threads_enter();
    if (nINS) {
	gtk_widget_modify_bg(evtins, GTK_STATE_NORMAL, &colRED);
    }
    
    else {
	gtk_widget_modify_bg(evtins, GTK_STATE_NORMAL, &colBLACK);
    }
    gdk_threads_leave();
}


    /*
     *  ドライブ描画 
     */ 
static void     FASTCALL
DrawDrive(int drive) 
{
    int            num;
    char          *name;
    char           string[128];
    gchar * utf8;
    ASSERT((drive >= 0) && (drive <= 1));
    
	/*
	 * 番号セット 
	 */ 
	if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
	num = 255;
    }
    
    else {
	num = fdc_access[drive];
	if (num == FDC_ACCESS_SEEK) {
	    num = FDC_ACCESS_READY;
	}
    }
    
	/*
	 * 名前取得 
	 */ 
	name = "";
    if (fdc_ready[drive] == FDC_TYPE_D77) {
	name = fdc_name[drive][fdc_media[drive]];
    }
    if (fdc_ready[drive] == FDC_TYPE_2D) {
	name = "2D DISK";
    }
    if (fdc_ready[drive] == FDC_TYPE_VFD) {
	name = "VFD DISK";
    }
    
	/*
	 * 番号比較 
	 */ 
	if (nDrive[drive] == num) {
	if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
	    return;
	}
	if (strcmp(szDrive[drive], name) == 0) {
	    return;
	}
    }
    
	/*
	 * 描画 
	 */ 
	nDrive[drive] = num;
    strcpy(szDrive[drive], name);
    if (nDrive[drive] == 255) {
	strcpy(string, "");
    }
    
    else {
	strcpy(string, szDrive[drive]);
    }
    gdk_threads_enter();
    utf8 =
	g_convert(string, strlen(string), "UTF-8", "CP932", NULL, NULL,
		  NULL);
    gtk_label_set_text(GTK_LABEL(lblflp[drive]), utf8);
    g_free(utf8);
    if (nDrive[drive] == FDC_ACCESS_READ) {
	gtk_widget_modify_fg(lblflp[drive], GTK_STATE_NORMAL, &colWHITE);
	gtk_widget_modify_bg(evtflp[drive], GTK_STATE_NORMAL, &colDRED);
    } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
	gtk_widget_modify_fg(lblflp[drive], GTK_STATE_NORMAL, &colWHITE);
	gtk_widget_modify_bg(evtflp[drive], GTK_STATE_NORMAL, &colDBLUE);
    } else {
	gtk_widget_modify_fg(lblflp[drive], GTK_STATE_NORMAL, &colBLACK);
	gtk_widget_modify_bg(evtflp[drive], GTK_STATE_NORMAL, &colNORM);
    }
    gdk_threads_leave();
}


    /*
     *  テープ描画 
     */ 
static void     FASTCALL
DrawTape(void) 
{
    int            num;
    char           string[128];
    
	/*
	 * ナンバー計算 
	 */ 
	num = 30000;
    if (tape_fileh != -1) {
	num = (int) ((tape_offset >> 8) % 10000);
	if (tape_motor) {
	    if (tape_rec) {
		num += 20000;
	    }
	    
	    else {
		num += 10000;
	    }
	}
    }
    
	/*
	 * 番号比較 
	 */ 
	if (nTape == num) {
	return;
    }
    
	/*
	 * 描画 
	 */ 
	nTape = num;
    if (nTape >= 30000) {
	string[0] = '\0';
    }
    
    else {
	sprintf(string, "%04d", nTape % 10000);
    }
    gdk_threads_enter();
    gtk_label_set_text(GTK_LABEL(lbltape), string);
    if ((nTape >= 10000) && (nTape < 30000)) {
	if (nTape >= 20000) {
	    gtk_widget_modify_bg(evttape, GTK_STATE_NORMAL, &colDBLUE);
	}
	
	else {
	    gtk_widget_modify_bg(evttape, GTK_STATE_NORMAL, &colDRED);
	}
    } else {
	gtk_widget_modify_bg(evttape, GTK_STATE_NORMAL, &colNORM);
    }
    gdk_threads_leave();
}


    /*
     *  描画 
     */ 
void            FASTCALL
DrawStatus(void) 
{
    DrawMainCaption();
    DrawCAP();
    DrawKANA();
    DrawINS();
    DrawDrive(0);
    DrawDrive(1);
    DrawTape();
} 
    /*
     *  再描画 
     */ 
void            FASTCALL
PaintStatus(void) 
{
    
	/*
	 * 記憶ワークをすべてクリアする 
	 */ 
	szCaption[0] = '\0';
    nCAP = -1;
    nKANA = -1;
    nINS = -1;
    nDrive[0] = -1;
    nDrive[1] = -1;
    szDrive[0][0] = '\0';
    szDrive[1][0] = '\0';
    nTape = -1;
    
	/*
	 * 描画 
	 */ 
	DrawStatus();
} 
#endif	/* _XWIN */

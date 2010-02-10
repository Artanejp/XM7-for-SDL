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
#include "sdl_draw.h"

#define COL_BLACK 0x00000000
#define COL_RED   0x00ff0000
#define COL_BLUE  0x0000ff00
#define COL_NORM  0xffffffff

    
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


/*-[ ステータスバー ]-------------------------------------------------------*/ 
    /*
     *  ステータスバーの生成 
     */ 
void
CreateStatus(void) 
{
        SDL_Surface *p;
        SDL_Rect rec;
        p = SDL_GetVideoSurface();
        SDL_LockSurface(p);
        /*
         * RECT INS
         */
        rec.x = nDrawWidth - (60 * 3);
        rec.y = nDrawHeight + 0;
        rec.w = 50;
        rec.h = 20;
        SDL_FillRect(p, &rec, COL_BLACK);

        /*
         * RECT CAPS
         */
        rec.x = nDrawWidth - (60 * 2);
        rec.y = nDrawHeight + 0;
        rec.w = 50;
        rec.h = 20;
        SDL_FillRect(p, &rec, COL_BLACK);

        /*
         * RECT KANA
         */
        rec.x = nDrawWidth - (60 * 1);
        rec.y = nDrawHeight + 0;
        rec.w = 50;
        rec.h = 20;
        SDL_FillRect(p, &rec, COL_BLACK);


        /*
         * RECT Drive1
         */
        rec.x = nDrawWidth - (100 * 3);
        rec.y = nDrawHeight + 20;
        rec.w = 100;
        rec.h = 20;
        SDL_FillRect(p, &rec, COL_NORM);

        /*
         * RECT Drive0
         */
        rec.x = nDrawWidth - (100 * 2);
        rec.y = nDrawHeight + 20;
        rec.w = 100;
        rec.h = 20;
        SDL_FillRect(p, &rec, COL_NORM);

        /*
         * RECT Tape
         */
        rec.x = nDrawWidth - (100 * 1);
        rec.y = nDrawHeight + 20;
        rec.w = 100;
        rec.h = 20;
        SDL_FillRect(p, &rec, COL_NORM);


        /*
         * Draw
         */
        SDL_UpdateRect(p, 0, 0, p->w, p->h);
        SDL_UnlockSurface(p);

} 
    /*
     *  キャプション描画 
     */ 
static void
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
//    if (memcmp(szCaption, string, strlen(string) + 1) != 0) {
//	strcpy(szCaption, string);
//	gdk_threads_enter();
//	gtk_window_set_title(GTK_WINDOW(wndMain), szCaption);
//	gdk_threads_leave();
//    }
}


    /*
     *  CAPキー描画 
     */ 
static void
DrawCAP(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec;


    
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
        rec.x = nDrawWidth - (60 * 2);
        rec.y = nDrawHeight + 0;
        rec.w = 50;
        rec.h = 20;


//        gdk_threads_enter();
        p = SDL_GetVideoSurface();
        SDL_LockSurface(p);

        if (nCAP) {
                SDL_FillRect(p, &rec, COL_RED);
        } else {
                SDL_FillRect(p, &rec, COL_BLACK);
        }
        SDL_UpdateRect(p, rec.x, rec.y, rec.w, rec.h);
        SDL_UnlockSurface(p);
//        gdk_threads_leave();
}


    /*
     *  かなキー描画 
     */ 
static void
DrawKANA(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec;
        p = SDL_GetVideoSurface();
    
        /*
         * 番号決定 
         */ 
        if (kana_flag) {
                num = 1;
        } else {
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
        rec.x = nDrawWidth - (60 * 1);
        rec.y = nDrawHeight + 0;
        rec.w = 50;
        rec.h = 20;

//        gdk_threads_enter();
        p = SDL_GetVideoSurface();
        SDL_LockSurface(p);
        if (nKANA) {
                SDL_FillRect(p, &rec, COL_RED);
        } else {
                SDL_FillRect(p, &rec, COL_BLACK);
        }

        SDL_UpdateRect(p, rec.x, rec.y, rec.w, rec.h);
        SDL_UnlockSurface(p);
//        gdk_threads_leave();
}


    /*
     *  INSキー描画 
     */ 
static void     FASTCALL
DrawINS(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec;
        p = SDL_GetVideoSurface();

        /*
         * 番号決定 
         */ 
        if (ins_flag) {
                num = 1;
        }  else {
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
//        gdk_threads_enter();
        rec.x = nDrawWidth - (60 * 3);
        rec.y = nDrawHeight + 0;
        rec.w = 50;
        rec.h = 20;

//        gdk_threads_enter();
        p = SDL_GetVideoSurface();
        SDL_LockSurface(p);
        if (nINS) {
                SDL_FillRect(p, &rec, COL_RED);
        } else {
                SDL_FillRect(p, &rec, COL_BLACK);
        }
        SDL_UpdateRect(p, rec.x, rec.y, rec.w, rec.h);

        SDL_UnlockSurface(p);
//        gdk_threads_leave();
}


    /*
     *  ドライブ描画 
     */ 
static void 
DrawDrive(int drive) 
{
    int            num;
    char          *name;
    char           string[128];
    gchar * utf8;
    SDL_Rect rec;
    SDL_Surface *p;

    ASSERT((drive >= 0) && (drive <= 1));
  

    /*
     * 番号セット 
     */ 
    if (fdc_ready[drive] == FDC_TYPE_NOTREADY) {
            num = 255;
    }  else {
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
//    gdk_threads_enter();
    p = SDL_GetVideoSurface();  
    SDL_LockSurface(p);
    //  gtk_label_set_text(GTK_LABEL(lblflp[drive]), utf8);
//    g_free(utf8);
    rec.x = nDrawWidth - (100 * (2+drive));
    rec.y = nDrawHeight + 20;
    rec.w = 100;
    rec.h = 20;


    if (nDrive[drive] == FDC_ACCESS_READ) {
            /*
             * READ
             */
            SDL_FillRect(p, &rec, COL_RED);
//	gtk_widget_modify_fg(lblflp[drive], GTK_STATE_NORMAL, &colWHITE);
//	gtk_widget_modify_bg(evtflp[drive], GTK_STATE_NORMAL, &colDRED);
    } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
            SDL_FillRect(p, &rec, COL_BLUE);
//	gtk_widget_modify_fg(lblflp[drive], GTK_STATE_NORMAL, &colWHITE);
//	gtk_widget_modify_bg(evtflp[drive], GTK_STATE_NORMAL, &colDBLUE);
    } else {
            SDL_FillRect(p, &rec, COL_NORM);
//	gtk_widget_modify_fg(lblflp[drive], GTK_STATE_NORMAL, &colBLACK);
//	gtk_widget_modify_bg(evtflp[drive], GTK_STATE_NORMAL, &colNORM);
    }
//    utf8 =
//	g_convert(string, strlen(string), "UTF-8", "CP932", NULL, NULL,
//		  NULL);
    SDL_UpdateRect(p, rec.x, rec.y, rec.w, rec.h);
    SDL_UnlockSurface(p);
//    gdk_threads_leave();
}


    /*
     *  テープ描画 
     */ 
static void
DrawTape(void) 
{
    int            num;
    char           string[128];
    SDL_Surface *p;
    SDL_Rect rec;
    
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
//    gdk_threads_enter();
    rec.x = nDrawWidth - (100 * 1);
    rec.y = nDrawHeight + 20;
    rec.w = 100;
    rec.h = 20;
    p = SDL_GetVideoSurface();
//    gtk_label_set_text(GTK_LABEL(lbltape), string);
    if ((nTape >= 10000) && (nTape < 30000)) {
            if (nTape >= 20000) {
                    SDL_FillRect(p, &rec, COL_BLUE);
                    //gtk_widget_modify_bg(evttape, GTK_STATE_NORMAL, &colDBLUE);
            }   else {
                    SDL_FillRect(p, &rec, COL_RED);
                    //gtk_widget_modify_bg(evttape, GTK_STATE_NORMAL, &colDRED);
            }
    } else {
            SDL_FillRect(p, &rec, COL_NORM);
            //gtk_widget_modify_bg(evttape, GTK_STATE_NORMAL, &colNORM);
    }
//    gdk_threads_leave();
}

/*
 *  描画 
 */ 
void  
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
void
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

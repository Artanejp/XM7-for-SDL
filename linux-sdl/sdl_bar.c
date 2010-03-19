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
#include "SDL/SDL_ttf.h"
#include <iconv.h>

#define COL_BLACK 0xff000000
#define COL_RED   0xff0000ff
#define COL_BLUE  0xffff0000
#define COL_GREEN  0xff00ff00
#define COL_NORM  0xffffffff
//#define COL_BLACK 0xffffffff
//#define COL_RED   0xffffffff
//#define COL_BLUE  0xffffffff
//#define COL_NORM  0xffffffff

/* フォントとしてIPAゴシックを使う */
#define FUNC_FONT "./ipagui.ttf" 
#define STAT_FONT "./ipagui.ttf" 
#define FUNC_PT 16
#define STAT_PT 16
    
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
static SDL_Surface      *pInsOn; /* INSキープリレンダリング(ON) */
static SDL_Surface      *pInsOff; /* INSキープリレンダリング(OFF) */
static SDL_Surface      *pKanaOn; /* カナキープリレンダリング(ON) */
static SDL_Surface      *pKanaOff; /* カナキープリレンダリング(OFF) */
static SDL_Surface      *pCapsOn; /* Capsキープリレンダリング(ON) */
static SDL_Surface      *pCapsOff; /* Capsキープリレンダリング(OFF) */
static SDL_Surface      *pFDRead[2]; /* Drive0 Read */
static SDL_Surface      *pFDWrite[2]; /* Drive0 Write */
static SDL_Surface      *pFDNorm[2]; /* Drive0 Normal */
static SDL_Surface      *pCMTRead; /* Tape Read */
static SDL_Surface      *pCMTWrite; /* Tape Write */
static SDL_Surface      *pCMTNorm; /* Tape Normal */

static SDL_Surface      *pStatusBar; /* ステータス表示バー */


/*-[ ステータスバー ]-------------------------------------------------------*/ 
    /*
     *  ステータスバーの生成 
     */ 
void
CreateStatus(void) 
{
        SDL_Surface *p;
        SDL_Surface *tmp;
        SDL_Rect rec;
        SDL_Rect drec;
        TTF_Font *f;
        SDL_Color r, b, n , black;
        Uint32 rmask, gmask, bmask, amask;

        /*
         * RECT INS
         */
        r.r = 255;
        r.g = 0;
        r.b = 0;

        b.r = 0;
        b.g = 0;
        b.b = 255;

        n.r = 255;
        n.g = 255;
        n.b = 255;

        black.r = 0;
        black.g = 0;
        black.b = 0;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
//        amask = 0x000000ff;
        amask = 0x000000ff;
#else
//        amask = 0;
//        gmask = 0;
//        rmask = 0;
//        bmask = 0;
        amask = 0xff000000;
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
#endif


        TTF_Init();
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;
        f = TTF_OpenFont(STAT_FONT, STAT_PT);


//        tmp  = TTF_RenderUTF8_Shaded(f, "INS", black , r);
//        printf("pInsOff: %dbpp\n",pInsOff->format->BitsPerPixel);
        pInsOn = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);

        pInsOff = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        SDL_FillRect(pInsOn, &rec, COL_RED);
        tmp = TTF_RenderUTF8_Blended(f, "INS", n );
        SDL_BlitSurface(tmp,NULL,pInsOn,NULL);
        SDL_FreeSurface(tmp);

        SDL_FillRect(pInsOff, &rec, COL_BLACK);
        tmp = TTF_RenderUTF8_Blended(f, "INS", r );
        SDL_BlitSurface(tmp,NULL,pInsOff,NULL);
        SDL_FreeSurface(tmp);
        /*
         * RECT CAPS
         */

        pCapsOn = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        pCapsOff = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        SDL_FillRect(pCapsOn, &rec, COL_RED);
        tmp  = TTF_RenderUTF8_Blended(f, "CAPS", n);
        SDL_BlitSurface(tmp,NULL,pCapsOn,NULL);
        SDL_FreeSurface(tmp);


        SDL_FillRect(pCapsOff, &rec, COL_BLACK);
        tmp  = TTF_RenderUTF8_Blended(f, "CAPS" , r);
        SDL_BlitSurface(tmp,NULL,pCapsOff,NULL);
        SDL_FreeSurface(tmp);


        /*
         * RECT KANA
         */
        pKanaOn = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        pKanaOff = SDL_CreateRGBSurface(SDL_SWSURFACE, 50, 20,
                                       32, rmask, gmask, bmask, amask);
        SDL_FillRect(pKanaOn, &rec, COL_RED);
        tmp  = TTF_RenderUTF8_Blended(f, "かな", n);
        SDL_BlitSurface(tmp,NULL,pKanaOn,NULL);
        SDL_FreeSurface(tmp);

        SDL_FillRect(pKanaOff, &rec, COL_BLACK);
        tmp  = TTF_RenderUTF8_Blended(f, "かな", r);
        SDL_BlitSurface(tmp,NULL,pKanaOff,NULL);
        SDL_FreeSurface(tmp);


        TTF_CloseFont(f);
        f = NULL;

        /*
         * RECT Drive1
         */
        rec.x = 0;
        rec.y = 0;
        rec.w = 160;
        rec.h = 20;
        pFDRead[0] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pFDRead[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pFDWrite[0] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pFDWrite[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pFDNorm[0] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pFDNorm[1] = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);

        SDL_FillRect(pFDRead[1], &rec, COL_RED);
        SDL_FillRect(pFDWrite[1], &rec, COL_BLUE);
        SDL_FillRect(pFDNorm[1], &rec, COL_NORM);
        /*
         * RECT Drive0
         */
        SDL_FillRect(pFDRead[0], &rec, COL_RED);
        SDL_FillRect(pFDWrite[0], &rec, COL_BLUE);
        SDL_FillRect(pFDNorm[0], &rec, COL_NORM);

        /*
         * RECT Tape
         */
        pCMTRead = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pCMTWrite = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);
        pCMTNorm = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 20,
                                       32, rmask, gmask, bmask, amask);


        SDL_FillRect(pCMTRead, &rec, COL_RED);
        SDL_FillRect(pCMTWrite, &rec, COL_BLUE);
        SDL_FillRect(pCMTNorm, &rec, COL_NORM);

        /*
         * Draw
         */
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        DrawStatus();
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;
        SDL_UpdateRect(p, 0, 0, p->w, p->h);
} 

void
DestroyDraw(void)
{
        SDL_FreeSurface(pInsOn);
        SDL_FreeSurface(pInsOff);
        SDL_FreeSurface(pCapsOn);
        SDL_FreeSurface(pCapsOff);
        SDL_FreeSurface(pKanaOn);
        SDL_FreeSurface(pKanaOff);
        SDL_FreeSurface(pFDRead[0]);
        SDL_FreeSurface(pFDWrite[0]);
        SDL_FreeSurface(pFDNorm[0]);
        SDL_FreeSurface(pFDRead[1]);
        SDL_FreeSurface(pFDWrite[1]);
        SDL_FreeSurface(pFDNorm[1]);
        SDL_FreeSurface(pCMTRead);
        SDL_FreeSurface(pCMTWrite);
        SDL_FreeSurface(pCMTNorm);
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
}


    /*
     *  CAPキー描画 
     */ 
static void
DrawCAP(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec,drec;


    
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
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;

        drec.x = nDrawWidth - (60 * 2);
        drec.y = nDrawHeight + 0;
        drec.w = 50;
        drec.h = 20;

        p = SDL_GetVideoSurface();
        if(p == NULL) return;

        if (nCAP) {
                SDL_BlitSurface(pCapsOn, NULL, p, &drec);
        } else {
                SDL_BlitSurface(pCapsOff, NULL, p, &drec); 
        }
        SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);
        SDL_Flip(p);
}


    /*
     *  かなキー描画 
     */ 
static void
DrawKANA(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec, drec;
    
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

        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;


        drec.x = nDrawWidth - (60 * 1);
        drec.y = nDrawHeight + 0;
        drec.w = 50;
        drec.h = 20;
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        if (nKANA) {
                SDL_BlitSurface(pKanaOn, &rec, p, &drec);                 
        } else {
                SDL_BlitSurface(pKanaOff, &rec, p, &drec); 
        }
        SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);

}


    /*
     *  INSキー描画 
     */ 
static void     FASTCALL
DrawINS(void) 
{
        int            num;
        SDL_Surface *p;
        SDL_Rect rec, drec;
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
        rec.x = 0;
        rec.y = 0;
        rec.w = 50;
        rec.h = 20;

        drec.x = nDrawWidth - (60 * 3);
        drec.y = nDrawHeight + 0;
        drec.w = 50;
        drec.h = 20;
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        if (nINS) {
                SDL_BlitSurface(pInsOn, &rec, p, &drec);
        } else {
                SDL_BlitSurface(pInsOff, &rec, p, &drec);
        }
        SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);
        SDL_Flip(p);
}


    /*
     *  ドライブ描画 
     */ 
static void 
DrawDrive(int drive) 
{
    int            num;
    char          *name;
    char          string[128];
    char          utf8[256];
    iconv_t       hd;
    size_t        in, out;

    SDL_Rect rec,drec;
    SDL_Surface *p, *tmp;
    SDL_Color r, b, n , black;
    TTF_Font *f;

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

    p = SDL_GetVideoSurface();  
//    hd = iconv_open("utf8", "cp932");
//    if(hd < 0) return;
//    in = strlen(string);
//    out = 256;
//    memset(utf8, 0x00, 255);
//    iconv(hd, &string, &in, &utf8, &out);
//    iconv_close(hd);
    //  gtk_label_set_text(GTK_LABEL(lblflp[drive]), utf8);
//    g_free(utf8);
//    utf8 =
//	g_convert(string, strlen(string), "UTF-8", "CP932", NULL, NULL,
//		  NULL);

//    if(p == NULL) return;
    rec.x = 0;
    rec.y = 0;
    rec.w = 160;
    rec.h = 20;
    drec.x = nDrawWidth - (160 * (drive + 2));
    drec.y = nDrawHeight + 20;
    drec.w = 160;
    drec.h = 20;

#if 0
    f = TTF_OpenFont(STAT_FONT, STAT_PT);
    SDL_FillRect(pFDRead[drive], &rec, COL_RED);
    SDL_FillRect(pFDWrite[drive], &rec, COL_BLUE);

    tmp  = TTF_RenderUTF8_Blended(f, utf8, n);
    if(tmp->w < rec.w) rec.w = tmp->w;
    if(tmp->h < rec.h) rec.h = tmp->h;
    SDL_BlitSurface(tmp,&rec,pFDRead[drive],NULL);


    SDL_BlitSurface(tmp,&rec,pFDWrite[drive],NULL);
    SDL_FreeSurface(tmp);
    rec.x = 0;
    rec.y = 0;
    rec.w = 160;
    rec.h = 20;

    SDL_FillRect(pFDNorm[drive], &rec, COL_NORM);
    tmp  = TTF_RenderUTF8_Blended(f, utf8, black);
    if(tmp->w < rec.w) rec.w = tmp->w;
    if(tmp->h < rec.h) rec.h = tmp->h;
    SDL_BlitSurface(tmp,&rec,pFDNorm[drive],NULL);
    SDL_FreeSurface(tmp);

    TTF_CloseFont(f);
#endif

    if (nDrive[drive] == FDC_ACCESS_READ) {
            /*
             * READ
             */
            SDL_BlitSurface(pFDRead[drive], &rec, p, &drec);
    } else if (nDrive[drive] == FDC_ACCESS_WRITE) {
            SDL_BlitSurface(pFDWrite[drive], &rec, p, &drec);
    } else {
            SDL_BlitSurface(pFDNorm[drive], &rec, p, &drec);
    }
    SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);

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
    SDL_Rect rec,drec;
    
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

    drec.x = nDrawWidth - (160 * 1);
    drec.y = nDrawHeight + 20;
    drec.w = 160;
    drec.h = 20;

    rec.x = 0;
    rec.y = 0;
    rec.w = 160;
    rec.h = 20;

    p = SDL_GetVideoSurface();
    if(p == NULL) return;

    if ((nTape >= 10000) && (nTape < 30000)) {
            if (nTape >= 20000) {
                    SDL_BlitSurface(pCMTWrite, &rec, p, &drec);
            }   else {
                    SDL_BlitSurface(pCMTRead, &rec, p, &drec);
            }
    } else {
                    SDL_BlitSurface(pCMTNorm, &rec, p, &drec);
    }
    SDL_UpdateRect(p, drec.x, drec.y, drec.w, drec.h);
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
        SDL_Surface *p;
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
        p = SDL_GetVideoSurface();
        if(p == NULL) return;
        DrawStatus();
} 
#endif	/* _XWIN */
